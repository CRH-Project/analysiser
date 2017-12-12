#include "httpTrace.h"
#include "utils.h"
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

#define LOG_TIMES 100000
#define ISHTTP(port) ((port) == 80)
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

class CanAdd
{
	public:
		TargetShooter & ts;
		CanAdd(TargetShooter & t):ts(t){}
		bool operator()(const HttpPacket & htp)
		{
			if(htp.seq>ts.start && htp.seq<=ts.end)	//true!
			{
				ts.size+=htp.tot_len;
				contentSize[ts.type]+=htp.tot_len;
//				std::cout<<contentSize.size()<<std::endl;
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
		return t.end-t.start <= t.size+THRESHOLD;
	}
};
/*
 * FUNCTION void flush()
 *
 * calculate stats and add them to contentSize
 */
inline void flush()
{
	int before,after;
	before=targetSet.size();
	std::remove_if(targetSet.begin(),targetSet.end(),CleanTarget());
	after=targetSet.size();
	if(before<after)
		std::cout<<"(before,after) : ("<<before<<","<<after<<")"<<std::endl;
	for(auto tgt : targetSet)
	{
		//std::cout<<"removing..."<<std::endl;
		std::remove_if(buffer.begin(),buffer.end(),CanAdd(tgt));
	}
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
	if((r = getField(len, app, "Content-Length: ")) < 0) 
		return;
	else 
		_len = atoi(len);
	//We have length now, we should have type in this packet;
	if((r = getField(type, app, "Content-Type: ")) < 0)
	   	return;
	else				//contains Content-Type field
	{
		int st = pack.seq, en = pack.seq + _len; 
		targetSet.insert(TargetShooter(
					Channel(pack.ch),st,en,
					std::string(type),TargetShooter::DOWNLINK)
				);
		//flush();
	}
	return;
}
static size_t total;
void http_roller(u_char * user, const struct pcap_pkthdr * h, const u_char * pkt)
{
	if(total % LOG_TIMES == 0) std::cout<<total<<"packets done"<<std::endl; 
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

void print()
{
	flush();
	std::cout<<"total packets : "<<total<<std::endl;
	std::cout<<"targetSet size : "<<targetSet.size()
		<<" buffer size : "<<buffer.size()<<std::endl;
	for(auto e : contentSize)
	{
		std::cout<<e.first<<" : "<<e.second<<std::endl;
	}
}
