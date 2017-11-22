#include "flow.h"
#include "headers.h"
#include <pcap/pcap.h>
#include <cassert>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#define ISUSR(ip) (((ip) & 0xffff0000) == 0xc0a80000 && ((ip)!=0xc0a80a01) && ((ip)!=0xc0a80a02))
std::map<SocketStat,std::set<PacketInfo>,Less> skt_map;
std::vector<FlowType> flowVec;
bool operator<(const struct timeval & l, const struct timeval & r){
	if(l.tv_sec == r.tv_sec) return l.tv_usec<r.tv_usec;
	return l.tv_sec<r.tv_sec;
}
bool operator==(const struct timeval & l,const struct timeval & r)
{
	return l.tv_sec==r.tv_sec && l.tv_usec==r.tv_usec;
}
bool operator<(const PacketInfo & l, const PacketInfo & r)
{
	return l.tv<r.tv;
}
struct timeval operator-(const struct timeval & l,const struct timeval & r)
{
	struct timeval ans;
	__suseconds_t pp=0;
	if (l.tv_sec<r.tv_sec){std::cerr<<"GOT NEGATIVE TIME"<<std::endl; assert(NULL);}
	ans.tv_sec=l.tv_sec-r.tv_sec;	
	if(l.tv_usec<r.tv_usec){pp=1000000;ans.tv_sec-=1;}
	ans.tv_usec=pp+l.tv_usec-r.tv_usec;
	return ans;
}
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
	for(auto pskt : skt_map)
	{
		SocketStat skt=pskt.first;
		std::cout<<skt<<std::endl;
		uint32_t prevseq=0;
		uint32_t flowcnt=0;
		uint32_t flowsize=0;
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
				if(flowsize!=0 && duration.tv_usec+duration.tv_sec>0) flowVec.push_back(FlowType(skt,flowsize,duration,dir));
				std::cout<<"Size: "<<flowsize<<" duration (sec:usec): "<<duration.tv_sec<<":"<<duration.tv_usec<<std::endl;
				flowsize=0;start=pkt.tv;
			}
			std::cout<<"\t"<<pkt<<"\n";
			//REFRESH PREVIOUS 
			prevseq=pkt.seq;
			flowsize+=pkt.tot_len;
			end=pkt.tv;
		}
		//ADD FLOW
		std::cout<<"end: "<<end.tv_sec<<":"<<end.tv_usec<<" start (sec:usec): "<<start.tv_sec<<":"<<start.tv_usec<<std::endl;
		duration=end-start;
		FlowType::dir_t dir;
		if(ISUSR(ntohl(skt.srcip))) dir=FlowType::UPLOAD;
		else dir=FlowType::DOWNLOAD;
		if(flowsize!=0 && duration.tv_usec+duration.tv_sec>0) flowVec.push_back(FlowType(skt,flowsize,duration,dir));
		std::cout<<"Size: "<<flowsize<<" duration (sec:usec): "<<duration.tv_sec<<":"<<duration.tv_usec<<std::endl;
		
		std::cout<<"==================================="<<std::endl;
		std::cout<<std::endl;
	}
}
int runFlow(char * fileName)
{
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
	for(auto flow : flowVec)
	{
		flow.print(std::cout);
		std::cout<<std::endl;
	}
	return 0;
}	

//UPLOAD 0 DOWNLOAD 1
void printSize(std::ostream & o)
{
	o<<flowVec.size()<<std::endl;
	for(auto flow : flowVec)
	{
		o<<flow.size<<" "<<flow.direction<<std::endl;
	}
}
void printDuration(std::ostream & out)
{
	out<<flowVec.size()<<std::endl;
	for(auto flow : flowVec)
	{
		out<<flow.calDuration()<<" "<<flow.direction<<std::endl;
	}
}
void printRate(std::ostream & out)
{
	out<<flowVec.size()<<std::endl;
	for(auto flow : flowVec)
	{
		out<<flow.calRate()<<" "<<flow.direction<<std::endl;
	}
}
