#include "scheduler.h"
#include "simlib.h"
#include "utils.h"
#include <cmath>
#include <ctime>

/*=======================*/
/* GLB TYPEDEFS AND VARS */
/*=======================*/

static int card_cnt;
__useconds_t * end_time;
struct timeval curr_time;

/*==============================*/
/* HELPER FUNCTION DECLARATIONS */
/*==============================*/

void update_time_info();
void update_end_info();
struct timeval cal_end_time(Buffer & b,int index);

/*================================*/
/* HELPER FUNCTION IMPLEMENTATIONS*/
/*================================*/

void update_time_info()
{
	curr_time = get_curr_time();
	Buffer bufs[card_cnt];
	for(int i=0;i<card_cnt;i++)
	{	
		bufs[i] = get_buffer_info(i);
		auto cost = cal_end_time(bufs[i], i);
		end_time[i] = cost.tv_sec;
	}
	
}


struct timeval cal_end_time(Buffer & b, int index)
{
	struct timeval ret = {0l,0l},
				   idle_time = {0l,0l},
				   interval = {1l,0l};

	double idle = 0, len = 0, bdw = 0;
	do
	{
		bdw = get_bandwidth(index, 
						get_curr_time() + get_start_time());
		len = bdw * (interval.tv_sec +
						interval.tv_usec/1000000.0);
		idle = buffer_dec(b, len);
		ret = ret + interval;
	}while ( idle == 0 );

	auto tt = idle/bdw;
	idle_time.tv_sec = std::floor(tt);
	idle_time.tv_usec = tt - idle_time.tv_sec;
	ret = ret - idle_time;

	return ret;
}
/*==========================*/
/* FUNCTION IMPLEMENTATIONS */
/*==========================*/

/**
 * FUNCTION init_simple_greddy(card_cnt)
 *
 * Init the whole scheduler
 */
void init_simple_greddy(int cc)
{
	card_cnt = cc;
	end_time = new __useconds_t[cc];
	curr_time = {0l,0l};
}



/**
 * FUNCTION clear_scheduler()
 *
 * Do clean up
 */
void clear_scheduler()
{
	delete[] end_time;
	card_cnt = 0;
	curr_time = {0l,0l};
}



/**
 * FUNCTION simple_greddy()
 *
 * Choose a card which end time is the least
 */
int simple_greddy()
{
	/* synchronize with the simulator */

	update_time_info();

	auto mint = end_time[0];
	int minpos = 0;
	for(int i=1;i<card_cnt;i++)
	{
		if(end_time[i] <= mint)
		{
			mint = end_time[i];
			minpos = i;
		}
	}

	return minpos;
}
