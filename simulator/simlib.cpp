#include "simlib.h"
#include "utils.h"
#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>
//#define DEBUG
#ifdef DEBUG
#include "../src/utils.h"
#endif
/*=============================*/
/*GLB TYPE AND VARS DEFINITIONS*/
/*=============================*/

static Buffer * buffers;
static int		buffercnt;

/*========================*/
/*FUNCTION IMPLEMENTATIONS*/
/*========================*/

static std::ostream & operator<<(std::ostream & o,
								const struct timeval & t)
{
	o<<"("<<t.tv_sec<<","<<t.tv_usec<<")";
	return o;
}
/*-------------------*/
/* BUFFER OPERATIONS */
/*-------------------*/

/**
 * FUNCTION buffer_init
 *
 * init "count" buffers, one buffer for one card
 *
 * @params count	specified count
 * @returns 0 on success
 *			-1 otherwise
 */
int buffer_init(size_t count)
{
	buffers = new Buffer[count+1];
	if(!buffers)
	{
		fprintf(stderr,"Cannot allocate memory for buffers!\n");
		return -1;
	}
	buffercnt = count;
	return 0;
}



/**
 * FUNCTION buffer_add(index, len)
 *
 * Add a new packet of "len" size into specified buffer
 *
 * @param index		to specify buffer
 *		  len		size of new packet
 */
void buffer_add(int index, size_t len)
{
	if(index >= buffercnt)
	{
		fprintf(stderr,"cannot add into buffer %d,\
				cause it dosent exist\n", index);
		return;
	}
	Buffer & buf = buffers[index];
	buf.c_len+=len;
	buf.c_cnt++;
	buf.pkts.push_back(len);
}



/**
 * FUNCTION buffer_dec(index, len)
 *
 * decrease "len" size bytes from specified buffer
 *
 * @param index		to specify buffer
 *		  len		size to decrease
 *
 * @returns	0 if there is still some content remain
 *			in the buffer,
 *			else return the "idle length"
 *			-1 indicates error occurs
 */
double buffer_dec(int index, double len)
{
	if(index >=  buffercnt)
	{
		fprintf(stderr,"cannot decrease buffer %d,cause it dosent exist\n",
			   	index);
		return -1;
	}
	
	Buffer & buf = buffers[index];
	double ret = 0.0;
	if(buf.c_len <= len)
	{
		ret = len - buf.c_len;	/*store the return value*/
		buf.c_len = 0;
		buf.c_cnt = 0;
		buf.pkts.clear();
		return ret;
	}
	else						/*still some contents after dec*/
	{
		buf.c_len -= len;
		while(len > 0 && (*buf.pkts.begin())<=len)
		{
			/*a packet is sent*/
			len -= (*buf.pkts.begin());
			buf.pkts.pop_front();
			buf.c_cnt--;
		}

		/* sent remain length */
		*buf.pkts.begin() -= len;	
	}
	return 0;
}




/**
 * FUNCTION update_buffer(delta, interval)
 *
 * update every buffer, decrease the c_len of each
 *
 * @param delta		the length of time piece
 *		  interval	the interval of bandwidth picking
 *
 * @returns idle time length in which every buffer DON'T
 *			have ANY contents unsent
 */
struct timeval update_buffer(struct timeval delta,
						const struct timeval & interval)
{
	struct timeval start = get_start_time(),
				   duration,
				   curr,	
				   idle_time = {0l,0l},
				   delta_cp = delta;
	curr = start;
	
	bool loop = true;
	while(loop)
	{
		/* try to decrease time */
		if(interval < delta)
		{
			delta = delta - interval;
			duration = interval;
		}
		else
		{
			duration = delta;
			loop = false;
		}

		/* foreach buffer, decrease and get idle len */

		double minidle = 1e8;int minpos = -1;

		double bdws[buffercnt+1];
		for(int i=0;i<buffercnt;i++)	
		{
			double bdw = get_bandwidth(i,curr);
			bdws[i] = bdw;
			double size = bdw * duration.tv_sec +
						bdw * duration.tv_usec / 1000000.0;
			double idle_size = buffer_dec(i,size);

			/*Update min*/
			if(idle_size<minidle)
			{
				minidle = idle_size;
				minpos = i;
			}
		}

		/* min idle time is not 0, which means it comes idle */
		if(std::fabs(minidle) > 1e-8)
		{
			loop = false;
			double cal_time = minidle/bdws[minpos];
			idle_time.tv_sec = std::floor(cal_time);
			cal_time -= std::floor(cal_time);
			idle_time.tv_usec = (__suseconds_t)(cal_time * 1000000);
		}

		curr = curr + duration;
	}

	curr = curr - start;
	if(curr.tv_sec == -1 || curr.tv_usec == -1)
		std::cerr<<"here!"<<std::endl;
	struct timeval remain_time = delta_cp - curr; 
	idle_time = idle_time + remain_time;
	return idle_time;
}



/*--------------*/
/* INFO GETTERS */
/*--------------*/

double get_bandwidth(int ind, struct timeval timestamp)
{
	//TODO: change here!
	return 1500000.0;	/* 1.5 MiB per sec */
}
