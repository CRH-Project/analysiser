#include "headers.h"
#include <algorithm>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include "domain_stat.h"

#define DATA_FILE	"raw_data.txt"
using namespace std;

string prefix = "total/";

std::map<std::string, DomainStat> Map;
typedef std::map<std::string, DomainStat> _MAP_T;
std::vector<DomainStat> Vec;
typedef std::vector<DomainStat> _VEC_T;

void getRaw(istream & _i)
{
	string url;
	int hit_times,size;
	while(_i>>url)
	{
		auto & dms = Map[url];
		_i>>hit_times;
		cerr<< url<<" "<<hit_times<<endl;
		dms.hit_times = hit_times;
		dms.url = url;
		for(int i=0;i<hit_times;i++)
		{
			_i>>size;
			dms.flowSize.push_back(size);
		}
	}
}

void getVec()
{
	for(auto ent : Map)
	{
		Vec.push_back(ent.second);
	}
}

void Mkdir(const string & s)
{
	int a = mkdir(s.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	if(a == -1)
	{
		cerr<<strerror(errno)<<endl;
	}

}

int printStat()
{
	const int N = 15;
	int m = (N<Vec.size()?N:Vec.size());
	string s2 = prefix + "/";
	ofstream fout,foutBrief(prefix + "brief.txt");
	for(int i=0,j=0;i<m;i++)
	{
		fout.open(s2+"_RANK"+std::to_string(j)+".txt",
				std::ios::out);
		if(Vec[i].url == ADDR_NOT_FOUND)
		{
			m++;continue;
		}	
		std::cerr<<Vec[i].getBasicInfo()<<std::endl;
		Vec[i].printToFile(fout);
		fout.close();
		j++;
	}
	for(auto ent : Vec)
		foutBrief<<ent.getBasicInfo()<<endl;
	foutBrief.close();
}

int main(int argc, char *argv[])
{
	if(argc!=3)
	{
		fprintf(stderr,"Usage : %s <filename> <http | https>\n"
				,argv[0]);
		exit(-1);
	}

	Mkdir(prefix);
	if(string(argv[2]) == "http" || string(argv[2]) == "https")
	{
		prefix += argv[2];
	}
	else
	{
		fprintf(stderr,"Usage : %s <filename> <http | https>\n"
				,argv[0]);
	}

	Mkdir(prefix);

	ifstream fin(argv[1]), datafin;
	if(!fin)
	{
		fprintf(stderr,"cannot open file %s!\n",argv[1]);
		exit(-1);
	}

	string dir;
	while(fin>>dir)
	{
		if(dir.back()!='/')
			dir.push_back('/');
		
		datafin.open(dir+DATA_FILE);
		
		if(!datafin)
		{
			fprintf(stderr,"cannot open file %s!\n",
					(dir+DATA_FILE).c_str());
		}

		getRaw(datafin);
		datafin.close();
	}

	getVec();
	std::sort(Vec.begin(),Vec.end(),MoreHit());

	printStat();
	fin.close();
}
