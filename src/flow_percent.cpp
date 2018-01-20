#include "flow_percent.h"
#include "headers.h"

#define MOD_NUM 1000000

static size_t tot_size[fpc::TYPE_CNT];
static size_t total,tot_bytes;
static double tot_percentage[fpc::TYPE_CNT];

void roll_percent(u_char *name, const struct pcap_pkthdr * hdr,
					const u_char * pkt)
{
	total++;
	tot_bytes += hdr->len;
	if(total % MOD_NUM == 0) 
		fprintf(stderr, "%ld packets done\n",total);
	const struct Ethernet *link = (struct Ethernet *)pkt;
	const struct Ipv4 *net = (struct Ipv4 *)(pkt + sizeof(struct Ethernet));
	const struct Tcp *trans = (struct Tcp *)((u_char *)net + 4 * net->ihl);
	
	if(net->protocol == 17)	/* UDP */
		tot_size[fpc::UDP] += hdr->len;

	if(net->protocol == 6)	/* TCP */
	{
		tot_size[fpc::TCP] += hdr->len;
		
		uint16_t srcport = ntohs(trans->srcport),
				 dstport = ntohs(trans->dstport);

		if(srcport == 80 || dstport == 80)	/* http */
			tot_size[fpc::HTTP] += hdr->len;
		else if(srcport == 443 || dstport == 443)
			tot_size[fpc::HTTPS] += hdr->len;
		else tot_size[fpc::OTHER_TCP] += hdr->len;
	}
}

size_t getSize(fpc::Type tp)
{
	return tot_size[tp];
}

int getAllSizes(int ** pArray)
{
	*pArray = new int[fpc::TYPE_CNT];
	for(int i=0;i<fpc::TYPE_CNT;i++)
		*pArray[i] = tot_size[i];
	return fpc::TYPE_CNT;
}

void calPercentage()
{
	for(int i=0;i<fpc::TYPE_CNT;i++)
		tot_percentage[i] = ((double)tot_size[i])/tot_bytes;

}

double getPercentage(fpc::Type tp)
{
	calPercentage();
	return tot_percentage[tp];
}

int getAllPercentages(double ** pArray)
{
	using namespace fpc;
	calPercentage();

	*pArray = new double[TYPE_CNT];
	for(int i=0;i<TYPE_CNT;i++)
		*pArray[i] = tot_percentage[i];

	return TYPE_CNT;
}
