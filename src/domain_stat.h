#ifndef _DOMAIN_STAT_HH_
#define _DOMAIN_STAT_HH_

/* MACROS AND INCLUDE FILES */
#include "headers.h"
#include <stdint.h>
#include <vector>
#include <string>

/* GLB TYPE AND VARS DEFINITIONS */
struct DomainStat
{
	std::string url;
	int hit_times;
	std::vector<int> flowSize;
public:
	DomainStat():hit_times(0){url=ADDR_NOT_FOUND;}
	int getTotalSize();
	std::string getBasicInfo();
	void printToFile(std::ostream &_f);
	void dumpTo(std::ostream & _f);
};

/* CLASS HELPER FUNCTION DEFINITIONS */

bool operator<(const DomainStat & l, const DomainStat & r);
struct MoreHit
{
	bool operator()(const DomainStat & l, const DomainStat & r)
	{
		return l.hit_times>r.hit_times;
	}
};
bool more_hit(const DomainStat & l, const DomainStat & r);
/* FUNCTION DEFINITIONS */
#endif
