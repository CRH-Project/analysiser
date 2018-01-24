#include "sim.h"
#include "scheduler.h"
#include "simlib.h"
#include "sample.h"
#include <cstdio>
#include <iostream>

using namespace std;

int cal_schedule(scheduler_t sch,const char * filename,
		const char * sched_name)
{
	set_scheduler(sch);
	roll_pcap(filename);
	printf("-----------------------------------\n");
	printf("scheduler : %s\n",sched_name);
	print_result();
	printf("-----------------------------------\n");
	clean_simulator();
	clean_buffer();

}
int main(int argc, char *argv[])
{
	if(argc!=4)
	{
		fprintf(stderr,"Usage %s <pcap_file> <bdw_source> <card_cnt>\n",
				argv[0]);
		exit(-1);
	}

	int card_cnt = atoi(argv[3]);


	/* init the buffer, don't change */
	buffer_init(card_cnt);		//defined in "simlib.h"

	/* init bdw source file, don't change */
	/* need to call 'buffer_init' first */
	init_bdw_source(argv[2]);

	/* init the sample scheduler */
	init_simple_greddy(card_cnt);
	
	cal_schedule(simple_greddy, argv[1],"simple greddy");
	

	buffer_init(card_cnt);
	init_bdw_source(argv[2]);
	init_sample(card_cnt);		//defined in "sample.h"
	cal_schedule(simple_rr,argv[1],"simple round robin");
	

	return 0;
}
