#include "spliter.h"
#include "flow.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
bool sortBySize(FlowType & l, FlowType & r)
{
	return l.size<r.size;
}
string s;
extern vector<FlowType> flowVec;

//PRINT THE BASIC INFORMATION OF FLOW...
void stat1()
{
	FlowType::dir_t up=FlowType::UPLOAD;
	FlowType::dir_t down=FlowType::DOWNLOAD;
	ofstream fout_us(s+"UPLOAD_SIZE.txt",ios::out);
	ofstream fout_ds(s+"DOWNLOAD_SIZE.txt",ios::out);
	ofstream fout_ud(s+"UPLOAD_DURATION.txt",ios::out);
	ofstream fout_dd(s+"DOWNLOAD_DURATION.txt",ios::out);
	ofstream fout_ur(s+"UPLOAD_RATE.txt",ios::out);
	ofstream fout_dr(s+"DOWNLOAD_RATE.txt",ios::out);
	printSize(fout_us,FlowType::UPLOAD);
	printSize(fout_ds,FlowType::DOWNLOAD);
	printDuration(fout_ud,FlowType::UPLOAD);
	printDuration(fout_dd,FlowType::DOWNLOAD);
	printRate(fout_ur,FlowType::UPLOAD);
	printRate(fout_dr,FlowType::DOWNLOAD);
	fout_us.close();fout_ds.close();fout_ud.close();
	fout_dd.close();fout_ur.close();fout_dr.close();
}

//PRINT THE RATE OF 4 KINDS CLASSIFIED BY SIZE, INTO 8 FILES,(UPLOAD/DOWNLOAD) * 4 = 8
void stat2()
{
	sortPacket(sortBySize);
	int s1=100000,s2=1000000,s3=10000000;
	ofstream fout_01u(s+"UPLOAD_0.1-1MB.txt"),
			 fout_0u(s+"UPLOAD_0-0.1MB.txt"),
			 fout_1u(s+"UPLOAD_1-10MB.txt"),
			 fout_10u(s+"UPLOAD_10+MB.txt");
	ofstream fout_01d(s+"DWLOAD_0.1-1MB.txt"),
			 fout_0d(s+"DWLOAD_0-0.1MB.txt"),
			 fout_1d(s+"DWLOAD_1-10MB.txt"),
			 fout_10d(s+"DWLOAD_10+MB.txt");
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

//PRINT THE (TIMESTAMP,DURATION) AND (TIMESTAMP,SIZE) TUPLE INTO 2 FILES
//MAYBE 4? (UPLOAD/DOWNLOAD);
void stat3()
{
	int duration = 0, size = 1;
	ofstream fouts[2];
	fouts[duration].open(s+"(TIMESTP,DURATION).txt",ios::out);
	fouts[size].open(s+"(TIMESTP,SIZE).txt",ios::out);
	for(auto flow : flowVec)
	{
		auto ps = &flow.start;
		string ss = std::ctime(ps);
		ss[ss.size()-1]=' ';
		ss[3]=ss[7]=ss[10]=ss[19]='-';
		fouts[duration]<<ss<<" "<<flow.calDuration()<<endl;
		fouts[size]<<ss<<" "<<flow.size<<endl;
	}
	for(int i=0;i<2;i++) fouts[i].close();
}

#include <sys/stat.h>
#include <sys/types.h>
int main(int argc, char * argv[])
{
	//test for split
	if(argc!=2) cerr<<"USAGE ./main <filename>"<<endl;
	int b=atoi(argv[1]);
	runFlow(argv[1]);	
	s=string(argv[1]);
	s=s.substr(0,s.find("."));
	int a=mkdir(s.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	s+="/";
	cout<<"max rate: "<<getMaxRate()<<endl
		<<"max duration: "<<getMaxDura()<<endl;
	//sortPacket(sortBySize);
	//1e5 1e6 1e7
	stat1();
	stat2();
	stat3();
	return 0;
}
