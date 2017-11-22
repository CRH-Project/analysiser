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
	FlowType::dir_t up=FlowType::UPLOAD;
	FlowType::dir_t down=FlowType::DOWNLOAD;
	ofstream fout_us(s+"/UPLOAD_SIZE.txt",ios::out);
	ofstream fout_ds(s+"/DOWNLOAD_SIZE.txt",ios::out);
	ofstream fout_ud(s+"/UPLOAD_SIZE.txt",ios::out);
	ofstream fout_dd(s+"/DOWNLOAD_SIZE.txt",ios::out);
	ofstream fout_ur(s+"/UPLOAD_SIZE.txt",ios::out);
	ofstream fout_dr(s+"/DOWNLOAD_SIZE.txt",ios::out);
	printSize(fout_us,FlowType::UPLOAD);
	printSize(fout_ds,FlowType::DOWNLOAD);
	printSize(fout_ud,FlowType::UPLOAD);
	printSize(fout_dd,FlowType::DOWNLOAD);
	printSize(fout_ur,FlowType::UPLOAD);
	printSize(fout_dr,FlowType::DOWNLOAD);
	return 0;
}
