/**
 * FlowInfo.h
 *
 * defined class FlowInfo
 */

#ifndef _FLOWINFO_HH_
#define _FLOWINFO_HH_

/* INCLUDES AND MACROS */
#include <stdint.h>
#include <stdlib.h>
#include <utility>
class FlowInfo 
{
	/* public constants and typedefs */
	public:
		const static int32_t THRESHOLD = 1000000;
		typedef std::pair<uint32_t,uint16_t> _IP_PORT_PAIR;
		typedef std::pair<_IP_PORT_PAIR,_IP_PORT_PAIR>
						_END_PAIR;

	/* private fields */
	private:
		uint16_t srcport,dstport;
		uint32_t srcip,dstip;
		uint32_t seq_start,seq_max;	
		size_t size;

	
	/* private functions */
	private:
		const _IP_PORT_PAIR 
				getSrcPair() const
				{
					return std::make_pair(srcip,srcport);
				}

		const _IP_PORT_PAIR  
				getDstPair() const
				{
					return std::make_pair(dstip,dstport);
				}

		const _END_PAIR 
				getEndPair() const
				{
					return std::make_pair(getSrcPair(),getDstPair());
				}
	

	/* public methods */
	public:
		/* constructors */
		FlowInfo();
		FlowInfo(uint32_t srcip, uint32_t dstip,
				uint16_t srcport, uint16_t dstporti,
				uint32_t seq_start);

		/* setters */
		void setStartSeq(uint32_t start);
		void setMaxSeq(uint32_t seq);

		/* getters */
		uint16_t		getSrcPort()	const;
		uint32_t		getSrcIp()		const;
		uint32_t		getMaxSeq()		const;
		uint32_t		getStartSeq()	const;
		size_t			getSize()	const;
		
		/* operations */
		void addSize(size_t add);
		
		friend bool operator<(const FlowInfo & l, const FlowInfo & r);
};

/* FUNCTION DECLARATIONS */
bool operator<(const FlowInfo & l, const FlowInfo & r);


#endif 
