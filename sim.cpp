#include "sim.h"
#include "simlib.h"
#include "sample.h"
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage %s <pcap_file> <card_cnt>\n",
				argv[0]);
		exit(-1);
	}

	int card_cnt = atoi(argv[2]);

	/* init the sample scheduler */
	init_sample(card_cnt);		//defined in "sample.h"

	/* set the scheduler */
	set_scheduler(simple_rr);	//defined in "sample.h"

	/* init the buffer, don't change */
	buffer_init(card_cnt);		//defined in "simlib.h"

	/* let's ROLL! */
	roll_pcap(argv[1]);			//defined in "sim.h"

	/* get answer */
	print_result();

	return 0;
}
