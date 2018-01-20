#include <iostream>
#include "roll.h"
#include "flow_percent.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <errno.h>
using namespace std;

std::string s;

int main(int argc,char * argv[])
{
	if(argc!=3){ cerr<<"USAGE :"<<argv[0]<<" <filename> <output_folder_prefix>"<<endl; exit(-1);}

	s=string(argv[2]);
	while(s.back() == '/') s.pop_back();
	s+="-anal/";
	int a=mkdir(s.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	fprintf(stderr,"%s in main() [making dir of %s]\n",
		   strerror(errno), s.c_str());	   
	roll(argv[1],roll_percent);

	ofstream fout(s+"PERCENTAGE.txt",ios::out);
	fout<<"PROTOCOL,SIZE,PERCENTAGE"<<endl;
	fout<<"UDP, "<<getSize(fpc::UDP)<<","<<getPercentage(fpc::UDP)<<endl; 
	fout<<"TCP, "<<getSize(fpc::TCP)<<","<<getPercentage(fpc::TCP)<<endl; 
	fout<<"HTTP, "<<getSize(fpc::HTTP)<<","<<getPercentage(fpc::HTTP)<<endl; 
	fout<<"HTTPS, "<<getSize(fpc::HTTPS)<<","<<getPercentage(fpc::HTTPS)<<endl; 
	fout<<"OTHER_TCP, "<<getSize(fpc::OTHER_TCP)<<","<<getPercentage(fpc::OTHER_TCP)<<endl; 
	fout.close();
	return 0;
}
