#include "httpTrace.h"
#include "dns_trace.h"
#include "roll.h"
#include <iostream>
using namespace std;
int main(int argc,char * argv[])
{
	if(argc!=2)
	{
		cerr<<"Usage : ./http <filename>"<<endl;
	}
	roll(argv[1],dns_roller);
	roll(argv[1],http_roller);
	print();
	return 0;
}
