#include "spliter.h"
#include "flow.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
bool sortBySize(FlowType & l, FlowType & r)
{
	return l.size<r.size;
}
string s;
void stat1()
{
	FlowType::dir_t up=FlowType::UPLOAD;
	FlowType::dir_t down=FlowType::DOWNLOAD;
	ofstream fout_us(s+"-UPLOAD_SIZE.txt",ios::out);
	ofstream fout_ds(s+"-DOWNLOAD_SIZE.txt",ios::out);
	ofstream fout_ud(s+"-UPLOAD_DURATION.txt",ios::out);
	ofstream fout_dd(s+"-DOWNLOAD_DURATION.txt",ios::out);
	ofstream fout_ur(s+"-UPLOAD_RATE.txt",ios::out);
	ofstream fout_dr(s+"-DOWNLOAD_RATE.txt",ios::out);
	printSize(fout_us,FlowType::UPLOAD);
	printSize(fout_ds,FlowType::DOWNLOAD);
	printDuration(fout_ud,FlowType::UPLOAD);
	printDuration(fout_dd,FlowType::DOWNLOAD);
	printRate(fout_ur,FlowType::UPLOAD);
	printRate(fout_dr,FlowType::DOWNLOAD);
}
void stat2()
{
	sortPacket(sortBySize);
	int s1=100000,s2=1000000,s3=10000000;
	ofstream fout_01u(s+"UPLOAD_0.1-1MB.txt"),
			 fout_0u(s+"UPLOAD_0-0.1MB.txt"),
			 fout_1u(s+"UPLOAD_1-10MB.txt"),
			 fout_10u(s+"UPLOAD-10+MB.txt");
	ofstream fout_01d(s+"DWLOAD_0.1-1MB.txt"),
			 fout_0d(s+"DWLOAD_0-0.1MB.txt"),
			 fout_1d(s+"DWLOAD_1-10MB.txt"),
			 fout_10d(s+"DWLOAD-10+MB.txt");
	ofstream * pf;
	for(auto flow : flowVec)
	{
		if(flow.direction==FlowType::UPLOAD)
		{
			if(flow.size<s1) 
				pf=&fout_0u;
			else if (flow.size<s2)
				pf=&fout_01u;
			else if (flow.size<s3)
				pf=&fout_1u;
			else pf=&fout_10u;
		}	
		else
		{
			if(flow.size<s1) 
				pf=&fout_0d;
			else if (flow.size<s2)
				pf=&fout_01d;
			else if (flow.size<s3)
				pf=&fout_1d;
			else pf=&fout_10d;
		}
		(*pf)<<flow.calRate()<<endl;
	}

}
int main(int argc, char * argv[])
{
	//test for split
	if(argc!=2) cerr<<"USAGE ./main <filename>"<<endl;
	int b=atoi(argv[1]);
	runFlow(argv[1]);	
	s=string(argv[1]);
	cout<<"max rate: "<<getMaxRate()<<endl
		<<"max duration: "<<getMaxDura()<<endl;
	//sortPacket(sortBySize);
	//1e5 1e6 1e7
	stat1();
	stat2();
	return 0;
}
