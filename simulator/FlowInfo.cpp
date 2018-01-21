#include "FlowInfo.h"

/* PRIVATE FUNCTION IMPLEMENTATION */


/* PUBLIC METHOD IMPLEMENTATION */
FlowInfo::FlowInfo()	/* default contructor */
{
	srcport = dstport = 0;
	srcip = dstip = 0;
	seq_start = seq_max = 0;
	size = 0;
}

FlowInfo::FlowInfo(uint32_t sip, uint32_t dip,
				uint16_t spt, uint16_t dpt, uint32_t sst)
				:srcport(spt),dstport(dpt),srcip(sip),dstip(dip),
				 seq_start(sst)
{
	this->seq_max = sst;
	this->size = 0;
}

void 
FlowInfo::setStartSeq(uint32_t start)
{
	this->seq_start = start;
}

void
FlowInfo::setMaxSeq(uint32_t seq)
{
	this->seq_max = seq;
}

uint16_t
FlowInfo::getSrcPort() const
{
	return this->srcport;
}

uint32_t 
FlowInfo::getSrcIp() const
{
	return this->srcip; 
}

uint32_t
FlowInfo::getMaxSeq() const
{
	return this->seq_max;
}

uint32_t
FlowInfo::getStartSeq() const
{
	return this->seq_start;
}

size_t
FlowInfo::getSize() const
{
	return this->size;
}

void
FlowInfo::addSize(size_t a)
{
	this->size+=a;
}

/* OTHER FUNCTIONS */
bool
operator<(const FlowInfo & l, const FlowInfo & r)
{
	return l.getEndPair()<r.getEndPair();
}
