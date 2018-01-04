#include "domain_stat.h"
#include "dns_trace.h"
#include <iostream>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>


/* IMPLEMENTATIONS */
std::string 
DomainStat::getBasicInfo()
{
	char buf[300];
	sprintf(buf,"domain name = %s, hit times = %d" \
				", average flow size = %d",
			url.c_str(),
		   	this->hit_times,
			this->getTotalSize()/this->hit_times);
	return std::string(buf);
}

int
DomainStat::getTotalSize()
{
	int sum = 0;
	for(int a : this->flowSize) sum+=a;
	return sum;
}

void
DomainStat::printToFile(std::ostream &_f)
{
	_f<<"### "<<this->getBasicInfo()<<std::endl;
	for(int size : this->flowSize)
		_f<<size<<std::endl;
}

void
DomainStat::dumpTo(std::ostream & _f)
{
	_f<<url<<" "<<hit_times<<std::endl;
	for(int size : flowSize)
		_f<<size<<" ";
	_f<<std::endl;
}

bool operator<(const DomainStat & l, const DomainStat & r)
{
	return l.url<r.url;
}

bool more_hit(const DomainStat & l, const DomainStat & r)
{
	return l.hit_times>r.hit_times;
}
