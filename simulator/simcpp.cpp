#include "sim.h"
#include "simlib.h"
#include "headers.h"
#include "utils.h"
#include <stdexcept>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <pcap.h>

//#define DEBUG
#ifdef DEBUG
#include "../src/headers.h"
#include "../src/utils.h"
#endif
/*=============================*/
/*GLB TYPE AND VARS DEFINITIONS*/
/*=============================*/

static int total = 0;
static const struct timeval interval = {0l,100l};
static scheduler_t scheduler;	/*the scheduler used*/
static result_t curr_ans;		/*the result*/
static struct timeval prev = {0l,0l},curr = {0l,0l},start;

/*============================*/
/*HELPER FUNCTIONS DEFINITIONS*/
/*============================*/
void roller(u_char *, const struct pcap_pkthdr*,
				const u_char *);
void update_result(const struct timeval & effnum);
void flush_buffer();


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
	const struct Udp_t *trans = (struct Udp_t *)((u_char *)net + 4 * net->ihl);
	const char *app = (char *)((u_char *)trans + sizeof(struct Udp_t));

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

	int num = scheduler();
	
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

