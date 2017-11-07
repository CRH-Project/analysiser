#include "spliter.h"
#include "utils.h"
#include <pcap/pcap.h>
#include <string.h>

#define BUFFER_SIZE 10000000		//10^7 == 10MB buffer size

typedef int Long;					//defined Long as int; may change here...

const char POSTFIX[6]=".pcap";		//postfix of pcap file
int handler_err_code;				//err code...
Long max_recs,cnt;					//# of records per file and current count;
char file_prefix[20];				//the specified file prefix
									//@see split(...) in spliter.h
struct pcap_file_header file_header;//File header in every splited file
u_char * buffer;					//Output buffer
int buffer_len;						//Buffer pointer
FILE * fout;						//output file

void beginPrint(int cnt)
{
	if (fout) fclose(fout);				//close old file;
	const char * postfix=itoa(cnt);		//get postfix using current count
	char fname[30];						
	strcpy(fname,file_prefix);
	strcat(fname,postfix);				//add postfix 'cnt'
	strcat(fname,POSTFIX);				//add postfix '.pcap'
	fout=fopen(fname,"w");				//open this file
	if(!fout)
	{
		fprintf(stderr,"CANNOT CREATE NEW FILE CALLED %s!\n",fname);
		handler_err_code = INVALID_FILE;
	}
}

void flushFile()
{
	if(!fout) return;
	fwrite(buffer,buffer_len,1,fout);
	buffer_len=0;
	memset(buffer,0,sizeof(u_char)*BUFFER_SIZE);
}
//loop... split the packet
void handle(u_char * user, const struct pcap_pkthdr *h, const u_char *pkt)
{
	const size_t S_UCHAR=sizeof(u_char);
	const size_t S_PKTHDR=16;			//sizeof(struct my_pcap_pkthdr);
										//size of pkthdr in the pcap file
										//is 16 bytes
										//However, different systems has 
										//different size of 'struct timeval'
										//which may lead to a wrong copy...
	if(cnt % max_recs == 0)				
	{
		fprintf(stderr,"printing start at %d\n",cnt);
		flushFile();					//flush the buffer into previous file
		beginPrint(cnt);				//create a new file
		buffer_len=0;					//initialize for buffer pointer
		memcpy(buffer,&file_header,sizeof(file_header));
										//copy file header into buffer
		buffer_len+=(sizeof(file_header)/S_UCHAR);
										//update buffer pointer
	}

	++cnt;								
	
	if(buffer_len + (S_PKTHDR + h->caplen)/S_UCHAR
				> BUFFER_SIZE)			//buffer overflow?
										//buffer is full
	{									//clear buffer
		fwrite(buffer,buffer_len,1,fout);
										//write to file
		memset(buffer,0,sizeof(u_char)*BUFFER_SIZE);
										//reset buffer
		buffer_len=0;					//reset buffer pointer
	}

	struct my_pcap_pkthdr mp;			//copy pkthdr into my_pkthdr
	mp.len=h->len;mp.caplen=h->caplen;
	mp.tv_sec=h->ts.tv_sec;
	mp.tv_usec=h->ts.tv_usec;

	memcpy(buffer+buffer_len, &mp, S_PKTHDR);
										//write it into buffer
	buffer_len += S_PKTHDR/S_UCHAR;

	memcpy(buffer+buffer_len, pkt, h->caplen/S_UCHAR);
										//copy pkt into buffer
	buffer_len += h->caplen/S_UCHAR;

}

int split(const char *fileName,int rec_num, const char * prefix)
{
	handler_err_code=SPLIT_SUCCESS;		//initialize the errorcode
	buffer=(u_char*)malloc(BUFFER_SIZE*sizeof(u_char));
										//malloc the buffer
	if(buffer==NULL)					//malloc failed?
	{
		fprintf(stderr,"CANNOT ALLOC MEMORY!");
		return BAD_ALLOC;
	}
	cnt=0;max_recs=rec_num;				//init variables...
	strcpy(file_prefix,prefix);			
	FILE * f=fopen(fileName,"r");		//open input file
	if(!f)
	{
		fprintf(stderr,"INVALID FILE TO SPLIT!\n");
		return INVALID_FILE;
	}
	fread(&file_header,sizeof(file_header),1,f);
										//read file header
	fseek(f,0,SEEK_SET);				//reset file pointer to 0

	pcap_t * pcap = pcap_fopen_offline(f,NULL);
	u_char err[100];					//for error msg
	pcap_loop(pcap,0,handle,err);
	flushFile();						//flush the buffer
	free(buffer);						
	fclose(fout);
	return handler_err_code;
}
