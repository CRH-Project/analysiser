#include "httpTrace.h"
#include "domain_stat.h"
#include "utils.h"
#include <locale>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <iostream>

#define LOG_TIMES 1000000
#define ISHTTP(port) ((port) == 80)

std::string prefix;
/*
 * Store the unflushed http packets
 */
static std::list<HttpPacket> buffer;
/*
 * Map for content-type to size
 */
static std::map<std::string,int> contentSize;
/*
 * Set for TargetShooters
 */
static std::set<TargetShooter> targetSet;

/*
 * Mapping content-type to flow size
 */
static std::map<std::string, std::vector<int> > flowPerContent;

/*
 * TargetShooter array with size
 */
static std::vector<TargetShooterS> tgtArrayS;

/*
 * Domain statistics, map from ip to stat
 */
static std::map<uint32_t,DomainStat> domainStat;
static std::vector<DomainStat> domainArray;

class CanAdd
{
	public:
		const TargetShooter & ts;
		TargetShooterS & tgts;
		CanAdd(const TargetShooter & t, TargetShooterS & s)
			:ts(t),tgts(s)
		{}
		bool operator()(const HttpPacket & htp)
		{
			if(htp.seq>ts.start && htp.seq<=ts.end 
					&& htp.tot_len>0)	//true!
			{
				tgts.size += htp.tot_len;
				contentSize[ts.type]+=htp.tot_len;
//				std::cout<<contentSize.size()<<std::endl;
				int & i = flowPerContent[ts.type].back();
				i+=htp.tot_len;
				return true;
			}
			else return false;
		}
};

/*
 * UnaryPred Struct CleanTarget
 *
 * if end-start <= size + threshold then clean it 
 *	i.e. return true
 */
struct CleanTarget
{
	const int THRESHOLD = 0;
	bool operator()(const TargetShooter & t)
	{
		return false;
	}
};
/*
 * FUNCTION void flush()
 *
 * calculate stats and add them to contentSize
 */
inline void flush()
{
	int tot_flows = 0;
	for(auto tgt : targetSet)
	{
		tgtArrayS.push_back(TargetShooterS(tgt.ch,0));
		flowPerContent[tgt.type].push_back(0);
		std::remove_if(buffer.begin(),buffer.end(),
				CanAdd(tgt,tgtArrayS.back()));
		if(flowPerContent[tgt.type].back() == 0)
			flowPerContent[tgt.type].pop_back();
		else tot_flows++;
		if(tgtArrayS.back().size == 0)
			tgtArrayS.pop_back();
	}

	std::cerr<<"Flow count : "<<tgtArrayS.size()<<std::endl;
	for(auto tgts : tgtArrayS)
	{
		auto & dms = domainStat[tgts.ch.srcip];
		dms.ip = tgts.ch.srcip;
		dms.hit_times++;
		dms.flowSize.push_back(tgts.size);
	
	}

	std::cerr<<"sorting domains by hit times..."<<std::endl;
	for(auto ent : domainStat)
	{
		domainArray.push_back(ent.second);
	}
	domainStat.clear();
	std::cerr<<"domain array size :"<<domainArray.size()<<std::endl;
	std::sort(domainArray.begin(),domainArray.end(),MoreHit());
}

/*
 * FUNCTION void dealDownlink(const char *, HttpPacket &)
 *
 * get a downlink http packet and deal with it
 *
 * @param
 *		app : content in the packet
 *		pack : basic infomation of the packet
 */
inline void dealDownlink(const char * app, HttpPacket & pack)
{
	buffer.push_back(pack);
	char type[100],len[100];
	int r,_len;
	//We want to have type for content shooter packet;
	if((r = getField(type, app, "Content-Type: ")) < 0)
	   	return;
	else
	{				//contains Content-Type field
		std::string type_s(type);
		std::transform(type_s.begin(),type_s.end(),
					type_s.begin(),::tolower);

		if((r = getField(len, app, "Content-Length: ")) < 0) 
		{
			//if we don't get content length, we will assume it has only 1 packet for it's type
			contentSize[type_s]+=pack.tot_len;
			if(pack.tot_len != 0)
			{
				flowPerContent[type_s].push_back(pack.tot_len);
				int st = pack.seq, en = pack.seq + _len; 
				TargetShooter temp(
						Channel(pack.ch),st,en,
						std::string(type_s),TargetShooter::DOWNLINK);
				tgtArrayS.push_back(TargetShooterS(
										pack.ch,pack.tot_len)
						);
			}

		}
		else
		{
			//we got expected length of the object, Expected Seq gap can be calculated...
			_len = atoi(len);
			using namespace std;
			int st = pack.seq, en = pack.seq + _len; 
			targetSet.insert(TargetShooter(
					Channel(pack.ch),st,en,
					std::string(type_s),TargetShooter::DOWNLINK)
				);
		//flush();
		}
	}
	return;
}
static size_t total;
void http_roller(u_char * user, const struct pcap_pkthdr * h, const u_char * pkt)
{
	if(total % LOG_TIMES == 0) std::cerr<<total<<"packets done"<<std::endl; 
	total++;
	const struct Ethernet *link = (struct Ethernet *)pkt;
//	printf("eth type is: %x\n",link->type);
	const struct Ipv4 *net = (struct Ipv4 *)(pkt + sizeof(struct Ethernet));
	const struct Tcp *trans = (struct Tcp *)((u_char *)net + 4 * net->ihl);
	const char *app = (char *)((u_char *)trans + 4 * trans->doff);
	
	//not tcp --> not http
	if(net->protocol != 6) return;

	uint32_t srcip = ntohl(net->srcip), srcport = ntohs(trans->srcport),
			dstip = ntohl(net->dstip), dstport = ntohs(trans->dstport);
	uint32_t seq = ntohl(trans->seq), tot_len = ntohs(net->tot_len);
	
	//Use the length of http payload
	tot_len = tot_len - net->ihl*4 - trans->doff*4;		

	//not http? return
//	std::cerr<<srcport<<" "<<dstport<<
	if(!ISHTTP(srcport) && !ISHTTP(dstport))
		return;

	//must be http then...
	HttpPacket pack(Channel(srcip,dstip),seq,tot_len);
	if(ISHTTP(srcport))			// download > respond
	{
		dealDownlink(app,pack);
	}
	else if (ISHTTP(dstport))	// upload  -> request
	{
		//TODO: DEAL_UPLINK
	}
}

void printBaiscInfo()
{
	std::ofstream fout;
	fout.open(prefix + "getBasicInfo.csv",std::ios::out);
	fout<<"total packets ,"<<total<<std::endl;
	fout<<"targetSet size ,"<<targetSet.size()
		<<" buffer size ,"<<buffer.size()<<std::endl;
	long long total=0;
	for(auto e : contentSize)
	{
		fout<<e.first<<" ,"<<e.second<<std::endl;
		total+=e.second;
	}
	fout<<"Total size ,"<<total<<std::endl;
}

void printFlowPerContent()
{
	std::string pre = prefix+"flow_per_contentTP";
	mkdir(pre.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	pre+='/';

	std::ofstream fout;
	int tot_flows = 0;
	for(auto content : flowPerContent)
	{
		std::string s2 = content.first;
		auto pos = s2.find("/");
		if(pos!=s2.npos)
		{
			s2.replace(pos,1,"-");
		}
		fout.open(pre+s2,std::ios::out);
		if(!fout) std::cerr<<"cannot open output file "
			<< pre+s2 <<std::endl;

		std::cerr<<"Content is : "<<content.first;
		int flow_size = 0;
		std::cerr<<'\t';
		for(auto size : content.second)
		{
			flow_size ++;
			fout<<size<<std::endl;
			//std::cerr<<size<<" ";
		}
		tot_flows += flow_size;
		std::cerr<<"flow cnt is "<<flow_size<<std::endl;
		fout.close();
	}
	std::cerr<<"total flow count is "<<tot_flows<<std::endl;
}

void printDomainStat()
{
	const int N = 20;
	int len = (N<domainArray.size()?N:domainArray.size());
	std::string pre = prefix+"domain_stat/";
	int a=mkdir(pre.c_str(), S_IRWXG | S_IRWXO | S_IRWXU);
	std::ofstream fout;
	for(int i=0;i<len;i++)
	{
		std::cerr<<domainArray[i].getBasicInfo()<<std::endl;
		fout.open(pre+"rank "+std::to_string(i)+".txt",
				std::ios::out);
		domainArray[i].printToFile(fout);
		fout.close();
	}
}

void print()
{
	std::cerr<<"flushing..."<<std::endl;
	flush();
	std::cerr<<"Flush finished! Basic Info Following"<<std::endl;
	printBaiscInfo();
	std::cerr<<"Printing flowsize per content-type"<<std::endl;
	printFlowPerContent();
	std::cerr<<"Printing domain info"<<std::endl;
	printDomainStat();
}

void setPrefix(std::string &s)
{
	prefix = s;
	mkdir(prefix.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
}
