/**
 * flow_percent.hh
 * 
 * calculate for HTTP HTTPS UDP TCP size percentage
 */

#ifndef _FLOW_PERCENT_HH_
#define _FLOW_PERCENT_HH_

/* INCLUDES AND MACRO */
#include <pcap.h>

namespace fpc{
	const int TYPE_CNT = 5;
	const int EFFECTIVE_CNT = 2;
	enum Type{TCP = 0, UDP = 1, HTTP = 2, HTTPS = 3, OTHER_TCP = 4};
};

/* GLB TYPE AND VAR DECLARATION */

/* FUNCTION DECLARATION */
void roll_percent(u_char * user, const struct pcap_pkthdr * h, const u_char * pkt);

size_t getSize(fpc::Type tp);
int getAllSizes(int ** pArray);
double getPercentage(fpc::Type tp);
int getAllPercentages(double ** pArray);

#endif
