#include "sim.h"
#include "simlib.h"
#include "headers.h"
#include "utils.h"
#include <stdexcept>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <pcap.h>
#include <set>
#include <cmath>
#include <map>

//#define DEBUG
#ifdef DEBUG
#include "../src/headers.h"
#include "../src/utils.h"
#endif
/*=============================*/
/*GLB TYPE AND VARS DEFINITIONS*/
/*=============================*/

typedef std::pair<bool,FlowInfo> __Flow_PAIR;

static int total = 0;
static const struct timeval interval = {0l,100l};
static scheduler_t scheduler;	/*the scheduler used*/
static result_t curr_ans;		/*the result*/
static struct timeval prev = {0l,0l},curr = {0l,0l},
					  /*these two are the distance to 'start'*/
					  start;
static FlowInfo currFlow;
static std::set<FlowInfo> flowSet;
static std::map<FlowInfo,int> flowMap;

/*============================*/
/*HELPER FUNCTIONS DEFINITIONS*/
/*============================*/
void roller(u_char *, const struct pcap_pkthdr*,
				const u_char *);
void update_result(const struct timeval & effnum);
void flush_buffer();
__Flow_PAIR add_flow(const FlowInfo &);
void update_flow_map(const FlowInfo &, int);
int get_flow_card(const FlowInfo &);


bool operator<(const struct timeval & l,
							const struct timeval & r);
struct timeval operator+(const struct timeval & l,
							const struct timeval & r);
struct timeval operator-(const struct timeval & l,
							const struct timeval & r);
struct timeval operator*(const struct timeval & l,
							const int r);


/*========================*/
/*FUNCTION IMPLEMENTATIONS*/
/*========================*/


/**
 * HELPER FUNCTION update_flow_map(flow,card_num)
 *
 * update the flow map
 */
void update_flow_map(const FlowInfo & f, int num)
{
	flowMap[f] = num;
}


/**
 * HELPER FUNCTION get_flow_card
 *
 * get card number of the flow
 */
int get_flow_card(const FlowInfo & f)
{
	return flowMap[f];
}



/**
 * HELPER FUNCTION update_result(efftime)
 *
 * update the final result using the specified timeval object
 *
 * @param efftime	the specified timeval object
 */
void update_result(const struct timeval & efftime)
{
	curr_ans = curr_ans + efftime;
}


/**
 * HELPER FUNCTION flush_buffer()
 *
 * flush the buffer after finishing the roll
 */
void flush_buffer()
{
	struct timeval idle_time;
	while(1)
	{
		idle_time = update_buffer(interval,interval);

		/* if have idle time, break out of the loop */
		if(idle_time.tv_sec || idle_time.tv_usec)
		{
			break;
		}

		/* else add time to ans */
		update_result(interval);
	}

	/* add "effect time" into ans */
	update_result(interval - idle_time);
}



/**
 * HELPER FUNCTION add_flow(flow)
 *
 * Add flow into a set, check if this flow exists
 *
 * @param flow	the flow to add
 *
 * @returns the matched flow in the flow set
 *			first : a boolean indicates if it is a new flow
 *			second : a variable of FlowInfo
 */
__Flow_PAIR	 add_flow(const FlowInfo & f)
{
	FlowInfo ret;
	bool flag = false;
	auto pTarget = flowSet.find(f);

	/* the specified (ip,port) pair dosen't existed */
	if(pTarget == flowSet.end())
	{
		flowSet.insert(f);
		return std::make_pair(true,f);
	}

	/* the specified (ip,port) pair exists */
	/* check if f is the same flow with it */
	/* using sequence number of both side  */
	const FlowInfo & target = *pTarget;
	auto tseq = target.getMaxSeq();
	auto fseq = f.getMaxSeq();
	int diff = std::abs(int(tseq - fseq));

	/* new sequence number too big/small , new flow */
	if(diff > FlowInfo::THRESHOLD)
	{
		flowSet.erase(pTarget);		/* remove old target */
		flowSet.insert(f);			/* insert new flow	 */
		flag = true;				/* new flow */
		ret = f;
	}
	else if (fseq < target.getStartSeq())
		/* smaller than start, update start and size */
	{
		FlowInfo newF = target;
		newF.setStartSeq(f.getStartSeq());
		newF.addSize(f.getSize());
		flowSet.erase(pTarget);
		flowSet.insert(newF);
		ret = newF;
	}
	else if (fseq > tseq)
		/* update curr and size */
	{
		FlowInfo newF = target;
		newF.setMaxSeq(f.getMaxSeq());
		newF.addSize(f.getSize());
		flowSet.erase(pTarget);
		flowSet.insert(newF);
		ret = newF;
	}
	else /* only update size */
	{
		FlowInfo newF = target;
		newF.addSize(f.getSize());
		flowSet.erase(pTarget);
		flowSet.insert(newF);
		ret = newF;
	}

	return std::make_pair(flag,ret);

}



/**
 * HELPER FUNCTION roller(name, h, pkt)
 *
 * the main routine of simulation
 */
void roller(u_char * name, const struct pcap_pkthdr *h,
			const u_char * pkt)
{
	total++;
	if(total == 1) start = h->ts;
	/*INITIALIZATION*/
	const struct Ethernet *link = (struct Ethernet *)pkt;
	const struct Ipv4 *net = (struct Ipv4 *)(pkt + sizeof(struct Ethernet));
	const struct Tcp_t *trans = (struct Tcp_t *)((u_char *)net + 4 * net->ihl);

	/* not tcp -> don't simulate */
	if(net->protocol != 6) return;
	/* not download -> don't simulate */
	if(!ISUSR(ntohl(net->dstip))) return;

	/*GET TIME INFO*/
	curr = h->ts - start;
	struct timeval delta = curr - prev;
	if(delta.tv_sec == -1)
	{
		int num = scheduler();
		buffer_add(num,h->len);
		return;
	}

	/*UPDATE BUFFER INFO UNTIL THIS PACKET ARRIVES*/
	struct timeval idle_time = update_buffer(delta,interval);
	update_result(delta - idle_time);

	/*UPDATE FLOW INFO*/
	uint16_t srcport = ntohs(trans->srcport),
			 dstport = ntohs(trans->dstport);
	uint32_t srcip = ntohl(net->srcip),
			 dstip = ntohl(net->dstip);
	uint32_t seq = ntohl(trans->seq);
	size_t len = h->len;
	
	FlowInfo temp(srcip,dstip,srcport,dstport,seq);
	__Flow_PAIR pair = add_flow(temp);
	currFlow = pair.second; 

	int num = 0;
	if(pair.first)	/* FLOW doesn't exist */
	{
		/*GET AND UPDATE CARD NUMBER*/
		num = scheduler();
		update_flow_map(currFlow,num);
	}	
	else			/* Flow exists */
	{
		num = get_flow_card(currFlow);
	}

	fprintf(stderr,"%d",num);
	/*INSERT IT INTO BUFFER*/
	buffer_add(num,h->len);

	prev = curr;
}

/**
 * FUNCTION set_scheduler
 *
 * @see sim.h
 */
void set_scheduler(scheduler_t sch)
{
	scheduler = sch;
}


/**
 * FUNCTION roll_pcap(filename)
 *
 * @see sim.h
 */
int roll_pcap(const char * filename)
{
	/*OPEN FILE*/
	FILE * f=fopen(filename,"r");
	if(!f)
	{
		fprintf(stderr,"INVALID FILE TO READ\n");
		return -1;
	}
	pcap_t *pcap = pcap_fopen_offline(f,NULL);
	u_char err[100];

	/*CALL ROLLER HERE!*/
	int errcode = pcap_loop(pcap,0,roller,err);

	fclose(f);
	if(errcode)
		fprintf(stderr,"ERROR OCCURED: %s\n",err);

	/* FLUSH THE BUFFER */
	flush_buffer();
	return errcode;
	
}

/**
 * FUNCTION get_result
 *
 * @see sim.h
 */
result_t get_result()
{
	return curr_ans;
}

/**
 * FUNCTION print_result()
 *
 * @see sim.h
 */
void print_result()
{
	result_t res = get_result();
	fprintf(stdout,"total packet length is %ld.%ld secs\n",
			curr.tv_sec,curr.tv_usec);
	fprintf(stdout,"final result is: %ld.%ld secs\n",
			res.tv_sec, res.tv_usec);
}


/**
 * FUNCTION get_start_time()
 *
 * @returns the start time of this pcap file
 */
struct timeval get_start_time()
{
	return start;
}


/**
 * FUNCTION get_curr_time
 *
 * Get current time, i.e. the distance to the start
 * 
 * @returns current time [DISTANCE]
 *
 * @example start = (100,123)
 *			curr  = (201,456)
 *			then ret = (101,111)
 */
struct timeval get_curr_time()
{
	return curr;
}


/**
 * FUNCTION get_curr_flow
 *
 * Get current flow, i.e. the flow which current
 * packet belongs to
 *
 * @returns current packet
 *
 * @see FlowInfo.h
 */
const FlowInfo& get_current_flow()
{
	return currFlow;
}
