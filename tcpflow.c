// Convert the output TXT file of "tshark -qz conv,tcp -r XXX.pcap" to CSV file
// The TXT file must conform to the following format
/*********<FILE start>
================================================================================
TCP Conversations
Filter:<No Filter>
                                               |       <-      | |       ->      | |     Total     |    Relative    |   Duration   |
                                               | Frames  Bytes | | Frames  Bytes | | Frames  Bytes |      Start     |              |
192.168.16.224:51126   <-> 180.97.236.39:80      7874  11443276    6574    480916   14448  11924192   520.032142000     300.6608
192.168.24.221:50732   <-> 202.102.94.125:80     7612  10885540    4794    289212   12406  11174752   177.945869000     641.2769
192.168.14.169:52389   <-> 153.37.238.146:80     5386   7334419    4222    269802    9608   7604221   287.840428000     536.6742
......
<FILE END>*********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#define MAXFLOW 5000000
#define MAXLINE 50000
#define MAXTIME 100000
#define MAXUSR 50000
#define ISNUM(c) ((c) <= '9' && (c) >= '0')

int concur[MAXFLOW][MAXTIME];

struct Tcpflow {
	char srcip[20];
	int srcport;
	char dstip[20];
	int dstport;
	int downframe, downsize;
	int upframe, upsize;
	int totframe, totsize;
	double start, duration;
	double avg_rate;
}tcp[MAXFLOW];

struct User{
	char ip[20];
	uint8_t concur[MAXTIME];
} usr[MAXUSR];

// skip non-number characters
void skip(char **c) {while(!ISNUM(**c)) (*c)++;}

// copy src to dst one by one, until "end" appears
char *cut(char *dst, char **src, char end) {
	int i;
	for(i = 0; **src != end; (*src)++,i++)
		dst[i] = **src;
	dst[i] = '\0';
	return dst;
}

void convert_line(struct Tcpflow *tf, char *l) {
	char tmp[MAXLINE], *cur = l;

	cut(tf->srcip, &cur, ':'); cur++;
	tf->srcport = atoi(cut(tmp, &cur, ' ')); skip(&cur);

	cut(tf->dstip, &cur, ':'); cur++;
	tf->dstport = atoi(cut(tmp, &cur, ' ')); skip(&cur);

	tf->downframe = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->downsize = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->upframe = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->upsize = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->totframe = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->totsize = atoi(cut(tmp, &cur, ' ')); skip(&cur);
	tf->start = atof(cut(tmp, &cur, ' ')); skip(&cur);
	tf->duration = atof(cut(tmp, &cur, '\0'));
/********** Is this how we calculate average rate? **********/
	tf->avg_rate = (double)(tf->totsize) / tf->duration;
	return;
}

void print_tcpflow(struct Tcpflow *tf) {
	printf("%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f\n",
		tf->srcip, tf->srcport, tf->dstip, tf->dstport,
		tf->downframe, tf->downsize, tf->upframe, tf->upsize, tf->totframe, tf->totsize,
		tf->start, tf->duration, tf->avg_rate);
}

int sort_ip(const void *a, const void *b) {
	struct Tcpflow *a1, *b1;
	int r;
	a1 = (struct Tcpflow *)a;
	b1 = (struct Tcpflow *)b;
	if((r = strcmp(a1->srcip, b1->srcip)) != 0) return r;
	else return a1->srcport > b1->srcport;
}

int main(int argc, char *argv[]) {
	int flownum, usrnum, i, j;
	size_t len;
	ssize_t read;
	FILE *fp;
	char *line;
	
	if(argc != 2) {
		printf("usage: tcpflow FILENAME\n");
		exit(0);
	}

	if(!(fp = fopen(argv[1], "r"))) {
		printf("fail to open %s\n", argv[1]);
		exit(0);
	}
	// getline() must use strings allocated by malloc()
	if(!(line = (char *)malloc(MAXLINE))) {
		printf("malloc failed\n");
		exit(0);
	}

	printf("Source IP,Source Port,Destination IP,Destination Port,\
Downloaded Frames,Downloaded Bytes,Uploaded Frames,Uploaded Bytes,\
Total Frames,Total Bytes,Relative Start,Duration,Average Rate\n");

	for(flownum = 0; ((read = getline(&line, &len, fp)) != -1); flownum++) {
		if(!ISNUM(line[0])) {flownum--; continue;}

		if(flownum >= MAXFLOW) {
			printf("Too many flows.\n");
			return 0;
		}

		convert_line(&tcp[flownum], line);
		if(tcp[flownum].totframe > 2 && tcp[flownum].totsize > 100)
			print_tcpflow(&tcp[flownum]);
	}
	free(line);

	return 0;
}
