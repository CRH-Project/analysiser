#include "spliter.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "domain_stat.h"
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
extern vector<DomainStat> httpsVec;
extern vector<DomainStat> httpVec;

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
	int duration = 0, size = 1, ds = 2;
	ofstream fouts[3];
	fouts[duration].open(s+"(TIMESTP,DURATION).txt",ios::out);
	fouts[size].open(s+"(TIMESTP,SIZE).txt",ios::out);
	fouts[ds].open(s+"(DURATION,SIZE).txt",ios::out);
	for(auto flow : flowVec)
	{
		auto ps = &flow.start;
		string ss = std::ctime(ps);
		ss[ss.size()-1]=' ';
		ss[3]=ss[7]=ss[10]=ss[19]='-';
		fouts[duration]<<ss<<" "<<flow.calDuration()<<endl;
		fouts[size]<<ss<<" "<<flow.size<<endl;
		fouts[ds]<<flow.calDuration()<<" "<<flow.size<<endl;
	}
	for(int i=0;i<3;i++) fouts[i].close();
}


void HTTPStat(vector<DomainStat> & vec, string folder)
{
	const int N = 15;
	fprintf(stderr,"%s data\n",folder.c_str());
	int m = (N<vec.size()?N:vec.size());
	string s2 = s + folder + "/";
	int a=mkdir(s2.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	ofstream fout,foutBrief;
	foutBrief.open(s2+"brief.txt",std::ios::out);
	for(int i=0,j=0;i<m;i++)
	{
		fout.open(s2+folder+"_RANK"+std::to_string(j)+".txt",
				std::ios::out);
		if(vec[i].url == ADDR_NOT_FOUND)
		{
			m++;continue;
		}	
		std::cerr<<vec[i].getBasicInfo()<<std::endl;
		vec[i].printToFile(fout);
		fout.close();
		j++;
	}
	for(auto end : vec)
	{
		foutBrief<<end.getBasicInfo()<<std::endl;
	}
	foutBrief.close();
	
	/* dump file */
	ofstream dump(s2+"raw_data.txt",std::ios::out);
	for(auto ent : vec)
		ent.dumpTo(dump);
	dump.close();
}

void flowPerHour()
{
	string s2 = s+"per_hour/";
	int a=mkdir(s2.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	std::cerr<<strerror(errno)<<" in flowPerHour"<<std::endl;
	printPerHour(s2);
}

int main(int argc, char * argv[])
{
	//test for split
	if(argc!=3){ cerr<<"USAGE :"<<argv[0]<<" <filename> <output_folder_prefix>"<<endl; exit(-1);}
	int b=atoi(argv[1]);
	runFlow(argv[1]);	
	
	s=string(argv[2]);
	while(s.back() == '/') s.pop_back();
	s+="-anal/";

	int a=mkdir(s.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	fprintf(stderr,"%s in main() [making dir of %s]\n",
		   strerror(errno), s.c_str());	   
	cout<<"max rate: "<<getMaxRate()<<endl
		<<"max duration: "<<getMaxDura()<<endl;
	//sortPacket(sortBySize);
	//1e5 1e6 1e7
	stat1();
	stat2();
	stat3();
	HTTPStat(httpsVec,"https");
	HTTPStat(httpVec,"http");
	flowPerHour();
	return 0;
}
