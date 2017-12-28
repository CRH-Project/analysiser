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
	struct in_addr in;
	in.s_addr = ntohl(this->ip);

	char buf[300];
	sprintf(buf,"ip = %s, domain name = %s, hit times = %d" \
				", average flow size = %d",
			inet_ntoa(in), findAddr(this->ip).c_str(),
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

bool operator<(const DomainStat & l, const DomainStat & r)
{
	return l.ip<r.ip;
}

bool more_hit(const DomainStat & l, const DomainStat & r)
{
	return l.hit_times>r.hit_times;
}
