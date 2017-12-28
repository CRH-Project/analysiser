#ifndef _SPLITER_HH_
#define _SPLITER_HH_ 

#include "headers.h"
#include <stdio.h>
#include <pcap/pcap.h>

struct my_pcap_pkthdr
{
	bpf_u_int32 tv_sec;
	bpf_u_int32 tv_usec; 
	bpf_u_int32 caplen;
	bpf_u_int32 len;	
}__attribute__((packed));
#define SPLIT_SUCCESS	0x0
#define INVALID_FILE	0x1
#define BAD_ALLOC		0x2
//int record_num;
// FUNCTION : split
// Split a specified pcap file into several pieces,
// Each piece has a specified number of records
// @params: 
//	fileName:	the pointer to the file
//	rec_num:	the specified number of records
//	prefix:		the prefix of output files' name
// @return:
//
// @author: Yihua

#ifdef __cplusplus
extern "C" {
#endif
int split(const char * fileName, int rec_num,const char * prefix);

#ifdef __cplusplus
}
#endif

#endif

