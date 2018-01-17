/**
 * sim.h
 * 
 * Defines functions used in sim.cpp
 * and scheduler type
 */

#ifndef _SIM_HH_
#define _SIM_HH_

/*===================*/
/*INCLUDES AND MACROS*/
/*===================*/

#include <time.h>

/*============================*/
/*GLB TYPE AND VAR DECLARAIONS*/
/*============================*/

/**
 * TYPEDEF scheduler_t
 * 
 * The interface which the schedule algorithm
 * is expected to use
 */
typedef int				(*scheduler_t)(void);

/**
 * TYPEDEF result_t
 *
 * The result of the simulation
 */
typedef struct timeval	result_t;

/*====================*/
/*FUNCTION DEFINITIONS*/
/*====================*/
#ifdef __cplusplus		//FOR c++
extern "C"
{
#endif 

/**
 * FUNCTION set_scheduler(scheduler)
 *
 * set the scheduler
 *
 * @param scheduler		the specified schedule algorithm
 */
void 
set_scheduler(scheduler_t);

/**
 * FUNCTION roll_pcap(filename)
 * 
 * simulate using the specified pcap file
 *
 * @param filename	the specified pcap file
 *
 * @returns 0 on success
 *			error code otherwise
 */
int 
roll_pcap(const char *filename);


/**
 * FUNCTION get_result()
 *
 * Simply get the result
 *
 * @returns an variable of reult_t indicates the finished time
 */
result_t 
get_result(void);

/**
 * FUNCTION print_result()
 *
 * print the result;
 */
void print_result(void);

#ifdef __cplusplus		//for c++
}
#endif


#endif 
