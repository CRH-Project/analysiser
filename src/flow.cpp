#include "flow.h"
#include "utils.h"
#include "domain_stat.h"
#include "dns_trace.h"
#include "roll.h"
#include "headers.h"
#include <pcap/pcap.h>
#include <cassert>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#define ISUSR(ip) (((ip) & 0xffff0000) == 0xc0a80000 && ((ip)!=0xc0a80a01) && ((ip)!=0xc0a80a02))
#define ISHTTPS(port)	((port) == 443)
#define ISHTTP(port)	((port) == 80)
#define MAXHOUR		20

std::map<SocketStat,std::set<PacketInfo>,Less> skt_map;
std::vector<FlowType> flowVec;
std::map<std::string, DomainStat> httpsMap,httpMap;
typedef std::map<std::string, DomainStat> _MAP_T;
std::vector<DomainStat> httpsVec,httpVec;
typedef std::vector<DomainStat> _VEC_T;
std::vector<FlowType>  flowPerHour[MAXHOUR];
int flowNumPerHour[MAXHOUR];
struct timeval beginTime;

/*bool operator<(const struct timeval & l, const struct timeval & r){
	if(l.tv_sec == r.tv_sec) return l.tv_usec<r.tv_usec;
	return l.tv_sec<r.tv_sec;
}*/
bool operator==(const struct timeval & l,const struct timeval & r)
{
	return l.tv_sec==r.tv_sec && l.tv_usec==r.tv_usec;
}
bool operator<(const PacketInfo & l, const PacketInfo & r)
{
	return l.tv<r.tv;
}
/*struct timeval operator-(const struct timeval & l,const struct timeval & r)
{
	struct timeval ans;
	__suseconds_t pp=0;
	if (l.tv_sec<r.tv_sec){std::cerr<<"GOT NEGATIVE TIME"<<std::endl; assert(NULL);}
	ans.tv_sec=l.tv_sec-r.tv_sec;	
	if(l.tv_usec<r.tv_usec){pp=1000000;ans.tv_sec-=1;}
	ans.tv_usec=pp+l.tv_usec-r.tv_usec;
	return ans;
}*/
std::ostream & operator<<(std::ostream & o, PacketInfo & p)
{
	o<<"( SEQ:"<<(p.seq%65535)<<", LEN:"<<p.tot_len<<", NSEQ:"<<(p.seq+p.tot_len)%65535<<", ISEQ:"<<p.seq<<", TIME:"<<p.tv.tv_sec<<"."<<p.tv.tv_usec<<")";
	return o;
}
std::ostream & operator<<(std::ostream & o, SocketStat & s)
{
	struct in_addr src;
	struct in_addr dst;
	src.s_addr=s.srcip;
	dst.s_addr=s.dstip;
	auto ssrc = inet_ntoa(src);
	o<<ssrc<<":"<<s.srcport<<" -> ";
	auto sdst = inet_ntoa(dst);
	o<<sdst<<":"<<s.dstport;
	return o;
}
int total;
void flowHandler(u_char *user, const struct pcap_pkthdr *h, const u_char *pkt)
{
	total++;
	if(total == 1) beginTime = h->ts;
	const struct Ethernet *link = (struct Ethernet *)pkt;
//	printf("eth type is: %x\n",link->type);
	const struct Ipv4 *net = (struct Ipv4 *)(pkt + sizeof(struct Ethernet));
	const struct Tcp *trans = (struct Tcp *)((u_char *)net + 4 * net->ihl);
	if (net->protocol!=6) return;
	uint16_t _len;		//LENGTH TO COMPUTE NEXT SEQ
	_len=ntohs(net->tot_len);
	_len = _len - 4*net->ihl -4*trans->doff;
	SocketStat skt(net->srcip,net->dstip,ntohs(trans->srcport),ntohs(trans->dstport));
	PacketInfo pkt_info(_len,ntohl(trans->seq),h->ts);
	skt_map[skt].insert(pkt_info);
}

void printFlow()
{
	FlowType tempFlow;
	for(auto pskt : skt_map)
	{
		SocketStat skt=pskt.first;
		std::cout<<skt<<std::endl;
		uint32_t prevseq=0;
		uint32_t flowcnt=0;
		uint32_t flowsize=0;
		uint32_t pkt_count=0;
		timeval start,end,duration;
		for(auto pkt : pskt.second)
		{
			uint32_t diff;
			if(prevseq>pkt.seq) diff=prevseq-pkt.seq;
			else diff=pkt.seq-prevseq;
			if(flowcnt==0){
			   	std::cout<<"flow 0:\n";
				start=pkt.tv;
				flowcnt++;
			}
			else if(diff>THRESHOLD)
			{
				std::cout<<"flow "<<flowcnt<<std::endl;
				flowcnt++;
				duration=end-start;
				FlowType::dir_t dir;
				if(ISUSR(ntohl(skt.srcip))) dir=FlowType::UPLOAD;
				else dir=FlowType::DOWNLOAD;
				if(flowsize!=0 && duration.tv_usec+duration.tv_sec>0 && pkt_count>=10) 
					flowVec.push_back(tempFlow=FlowType(skt,flowsize,duration,dir,start.tv_sec));
				std::cout<<"Size: "<<flowsize<<
					" duration (sec:usec): "<<duration.tv_sec<<":"<<duration.tv_usec<<
					" rate: "<<tempFlow.calRate()<<std::endl;
				flowsize=0;start=pkt.tv;
				pkt_count=0;
			}
			std::cout<<"\t"<<pkt<<"\n";
			//REFRESH PREVIOUS 
			prevseq=pkt.seq;
			flowsize+=pkt.tot_len;
			pkt_count++;
			end=pkt.tv;
		}
		//ADD FLOW
		std::cout<<"end: "<<end.tv_sec<<":"<<end.tv_usec<<" start (sec:usec): "<<start.tv_sec<<":"<<start.tv_usec<<std::endl;
		duration=end-start;
		FlowType::dir_t dir;
		if(ISUSR(ntohl(skt.srcip))) dir=FlowType::UPLOAD;
		else dir=FlowType::DOWNLOAD;
		if(flowsize!=0 && duration.tv_usec+duration.tv_sec>0 && pkt_count>=10) 
			flowVec.push_back(tempFlow=FlowType(skt,flowsize,duration,dir,start.tv_sec));
		std::cout<<"Size: "<<flowsize<<
			" duration (sec:usec): "<<duration.tv_sec<<":"<<duration.tv_usec<<
			" rate: "<<tempFlow.calRate()<<std::endl;
		
		std::cout<<"==================================="<<std::endl;
		std::cout<<std::endl;
	}
}

bool is_https(int port){return ISHTTPS(port);}
bool is_http(int port){return ISHTTP(port);}
void getVec(_MAP_T & Map, _VEC_T &Vec, bool (*cmp)(int))
{
	for(auto flow : flowVec)
	{
		int ip;
		if(cmp(flow.st.srcport)) 
		{
			ip = flow.st.srcip;
		}
		else if(cmp(flow.st.dstport))
		{
			ip = flow.st.dstip;
			continue;
		}
		else continue;
		ip = ntohl(ip);

		std::string url = findAddr(ip);
		
		auto & dms = Map[url];
		dms.url = url;
		dms.hit_times ++;
		dms.flowSize.push_back(flow.size);
	}
	
	for(auto ent : Map)
	{
		Vec.push_back(ent.second);
	}
	std::sort(Vec.begin(),Vec.end(),MoreHit());
}
int getFlowPerHouw()
{
	bzero(flowNumPerHour,sizeof flowNumPerHour);
	std::cerr<<"start time is "<<ctime(&beginTime.tv_sec)<<std::endl;
	int stt = beginTime.tv_sec / 3600;
	for(auto flow : flowVec)
	{
		//int h = flow.start - beginTime.tv_sec;
		//int ind = h/3600;
		int ind = flow.start / 3600 - stt;
		if(ind>=0 && ind<MAXHOUR)
		{
			flowPerHour[ind].push_back(flow);
			flowNumPerHour[ind]++;
		}
		else 
		{
			fprintf(stderr,"time error, very begin = %ld, flow begin = %ld\n",
					beginTime.tv_sec, flow.start);
		}
	}
	for(int i=0;i<MAXHOUR;i++)
		if(flowNumPerHour[i]!=0)
		{
			std::cerr<<i<<" th hour : "<<flowNumPerHour[i]<<std::endl;
		}
}

int runFlow(char * fileName)
{
	roll(fileName, dns_roller);
	FILE * f=fopen(fileName,"r");
	if(!f)
	{
		fprintf(stderr,"INVALID FILE TO ANALYSIS!\n");
		return -1;
	}
	pcap_t *pcap =pcap_fopen_offline(f,NULL);
	u_char err[100];
	pcap_loop(pcap,0,flowHandler,err);
	fclose(f);
	printFlow();
	getVec(httpsMap,httpsVec,is_https);
	getVec(httpMap,httpVec,is_http);
	getFlowPerHouw();
//	for(auto flow : flowVec)
//	{
//		flow.print(std::cout);
//		std::cout<<std::endl;
//	}
	return 0;
}	

//UPLOAD 0 DOWNLOAD 1
void printSize(std::ostream & o,FlowType::dir_t d)
{
	for(auto flow : flowVec)
	{
		if(flow.direction==d)
			o<<flow.size<<std::endl;
	}
}
void printDuration(std::ostream & out,FlowType::dir_t d)
{
	for(auto flow : flowVec)
	{
		if(flow.direction==d)
			out<<flow.calDuration()<<std::endl;
	}
}
void printRate(std::ostream & out,FlowType::dir_t d)
{
	for(auto flow : flowVec)
	{
		if(flow.direction==d)
			out<<flow.calRate()<<std::endl;
	}
}

double getMaxDura()
{
	double maxDura=0.0;
	for(auto flow : flowVec)
		if(flow.calDuration()>maxDura)
			maxDura=flow.calDuration();
	return maxDura;
}
double getMaxRate()
{
	double maxRate=0.0;
	for(auto flow : flowVec)
		if(flow.calRate()>maxRate)
		   maxRate=flow.calRate();
	return maxRate;
}


void printPerHour(std::string prefix)
{

	std::ofstream fout,fout2;
	fout.open(prefix+"number_per_hour.txt",
			std::ios::out);
	fout2.open(prefix+"throughput_per_hour.txt",
			std::ios::out);
	fout<<"### starts at "<<ctime(&beginTime.tv_sec)<<std::endl;
	fout2<<"### starts at "<<ctime(&beginTime.tv_sec)<<std::endl;
	for(int i=0;i<MAXHOUR && flowNumPerHour[i];i++)
	{
		fout<<flowNumPerHour[i]<< "\t### "<<i<<"th hour"<<std::endl;
		int th = 0;
		for(auto size : flowPerHour[i])
		{
			th = size.size;
		}
		fout2<<th<< "\t### "<<i<<"th hour"<<std::endl;
	}
	fout.close();
	fout2.close();
}

