#ifndef _UTILS_HH_
#define _UTILS_HH_

//FUNCTION: itoa(int)
//transfer an integer to string...
const char * itoa(int cnt);

/*
 * FUNCTION int getField(char *,const char *,char *)
 *
 * Find a specific field called <targetName> in the given source
 * and copy it to <dst>
 *
 * @params
 *		targetName : name of the target field
 *		src :		 the given source content
 *		dst :		 copy destination
 *
 */
int getField(char * dst, const char * src, char * targetName);


#endif
