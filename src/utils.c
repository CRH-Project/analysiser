#include "utils.h"
#include <string.h>
#include <stdlib.h>

const char * itoa(int cnt)
{
	const int BUFFER_SIZE=11;
	char * buffer=(char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer,0,sizeof(char)*BUFFER_SIZE);
	if(cnt==0) 
	{
		buffer[0]='0';
		return buffer;
	}
	int pos=BUFFER_SIZE-1;
	while(cnt!=0)
	{
		buffer[--pos]=(char)('0'+(cnt%10));
		cnt/=10;
	}
	const char * ret=buffer+pos;
	return ret;
}
