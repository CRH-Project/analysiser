#ifndef __FLOW_HH__
#define __FLOW_HH__
#include <list>
#include <vector>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <algorithm>

const int THRESHOLD =1000000;
class PacketInfo{
public:
	uint16_t tot_len;
	uint32_t seq;
	struct timeval tv;
	PacketInfo():tot_len(0),seq(0)
	{
		tv.tv_sec=0;
		tv.tv_usec=0;
	}
	PacketInfo(uint16_t tlen,uint32_t s,struct timeval t)
	{
		tot_len=tlen;
		seq=s;
		tv=t;
	}
};
class SocketStat
{
public:
	uint32_t srcip;
	uint32_t dstip;
	uint16_t srcport;
	uint16_t dstport;
	SocketStat():srcip(0),dstip(0),srcport(0),dstport(0){};
	SocketStat(uint32_t sip,uint32_t dip,uint16_t sport,uint16_t dport)
	{
		srcip=sip;
		dstip=dip;
		srcport=sport;
		dstport=dport;
	}
};
std::ostream & operator<<(std::ostream & o, SocketStat & s);
class FlowType
{
public:
	SocketStat st;
	uint32_t size;
	timeval tv;
	time_t start;
	enum dir_t{UPLOAD,DOWNLOAD}direction;
	FlowType():st(),size(0)
	{tv.tv_sec=0;tv.tv_usec=0;direction=DOWNLOAD;}
	FlowType(const SocketStat & s,uint32_t si,timeval t,enum dir_t d)
		:st(s),size(si),direction(d)
	{
		tv.tv_sec=t.tv_sec;
		tv.tv_usec=t.tv_usec;
	}
	FlowType(const SocketStat & s,uint32_t si,timeval t,enum dir_t d,time_t star):FlowType(s,si,t,d)
	{
		start = star;
	}
	double calRate()
	{
		return size/((double)(tv.tv_sec)+0.000001*tv.tv_usec);
	}
	double calDuration()
	{
		return (double)(tv.tv_sec)+0.000001*tv.tv_usec;
	}
	void print(std::ostream & o)
	{
		char name[2][14]={"UPLOAD","DOWNLOAD"};
		o<<st<<"; size: "<<size<<" bits; duration: "<<calDuration()<<" secs; Rate: "<<calRate()<<" bits/s; Direction: "<<name[direction];

	}

};

extern std::vector<FlowType> flowVec;

struct Less{
	bool operator()(const SocketStat & l,const SocketStat & r)
	{
		
		if(l.srcip==r.srcip)
		{
			if(l.srcport==r.srcport)
			{
				if(l.dstip==r.dstip)
				{
					return l.dstport<r.dstport;
				}
				return l.dstip<r.dstip;
			}
			return l.srcport<r.srcport;
		}
		return l.srcip<r.srcip;
	}
};
bool operator<(const struct timeval & l, const struct timeval & r);
bool operator==(const struct timeval & l,const struct timeval & r);
bool operator<(const PacketInfo & l, const PacketInfo & r);
struct timeval operator-(const timeval & l,const timeval & r);
std::ostream & operator<<(std::ostream & o, PacketInfo & p);
int runFlow(char * fileName);

void printSize(std::ostream&,FlowType::dir_t);
void printDuration(std::ostream&,FlowType::dir_t);
void printRate(std::ostream&,FlowType::dir_t);
//void printAll(std::ostream&,FlowType::dir_t);
template <typename T> void analysis(T  obj);
double getMaxDura();
double getMaxRate();
typedef bool (*PCMP)(FlowType &,FlowType &);
template<class T>
void sortPacket(T cmp)
{
	std::sort(flowVec.begin(),flowVec.end(),cmp);
}
#endif
