#include "spliter.h"
#include "roll.h"
#include <iostream>
using namespace std;
int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		cerr<<"Usage : %s <filename> <size>"<<endl;
	}
	split(argv[1],atoi(argv[2]),"test");
	return 0;
}
