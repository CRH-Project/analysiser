#include "spliter.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
	//test for split
	int b=atoi(argv[2]);
	split(argv[1],b,"test");
	return 0;
}
