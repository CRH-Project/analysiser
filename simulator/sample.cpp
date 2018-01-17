#include "sample.h"

static int card_cnt;
static int curr;

int init_sample(int num)
{
	card_cnt = num;
	curr = 0;
	return 0;
}

int simple_rr()
{
	curr = (curr+1)%card_cnt;
	return curr;
}


