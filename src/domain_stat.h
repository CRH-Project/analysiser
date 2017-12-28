#ifndef _DOMAIN_STAT_HH_
#define _DOMAIN_STAT_HH_

/* MACROS AND INCLUDE FILES */
#include <stdint.h>
#include <vector>
#include <string>

/* GLB TYPE AND VARS DEFINITIONS */
struct DomainStat
{
	uint32_t ip;
	int hit_times;
	std::vector<int> flowSize;
public:
	DomainStat():ip(0),hit_times(0){}
	DomainStat(int _ip):ip(_ip),hit_times(0){}
	int getTotalSize();
	std::string getBasicInfo();
	void printToFile(std::ostream &_f);
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
