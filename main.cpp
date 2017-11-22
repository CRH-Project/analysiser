#include "spliter.h"
#include "flow.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
int main(int argc, char * argv[])
{
	//test for split
	if(argc!=2) cerr<<"USAGE ./main <filename>"<<endl;
	int b=atoi(argv[1]);
	runFlow(argv[1]);	
	string s(argv[1]);
	ofstream fout_size(s+"-size.txt",ios::out);
	ofstream fout_dura(s+"-duration.txt",ios::out);
	ofstream fout_rate(s+"-rate.txt",ios::out);
	printSize(fout_size);
	printDuration(fout_dura);
	printRate(fout_rate);
	return 0;
}
