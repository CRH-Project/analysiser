/**
 * simlib.h
 *
 * functions used to in simulator
 */

#ifndef _SIMLIB_HH_
#define _SIMLIB_HH_

/*===================*/
/*INCLUDES AND MACROS*/
/*===================*/

#include <stdlib.h>
#include <list>

/*=============================*/
/*GLB TYPE AND VAR DECLARATIONS*/
/*=============================*/

#ifdef __cplusplus
typedef struct buffer_t
{
	double c_len;
	size_t c_cnt;
	std::list<double> pkts;
} Buffer;
#endif

/*=====================*/
/*FUNCTION DECLARATIONS*/
/*=====================*/

#ifdef __cplusplus
/* C++ FUNCTIONS */
Buffer			get_buffer_info(int index);
extern "C"
{
#endif


/**
 * FUNCTIONS BUFFER OPERATIONS
 *
 * int		buffer_init(count)
 * timeval	update_buffer(delta,interval)
 * void		buffer_add(index,len)
 * size_t	buffer_dec(index,len)
 * Buffer	get_buffer_info(index)
 * 
 */

int 			buffer_init(size_t count);
struct timeval	update_buffer(struct timeval delta, 
							const struct timeval & interval);
void			buffer_add(int index, size_t len);
double			buffer_dec(int index, double len);


/**
 * FUNCTIONS INFO GETTERS
 *
 * timeval	get_start_time()
 * int		get_bandwidth(index, timestamp)
 */
struct timeval get_start_time();
double get_bandwidth(int ind, struct timeval timestamp);


#ifdef __cplusplus
}
#endif 
#endif
