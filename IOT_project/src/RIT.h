/*
 * RIT.h
 *
 *  Created on: 29.11.2018
 *      Author: aaro
 */

#ifndef RIT_H_
#define RIT_H_
#include "chip.h"

extern int ar_index;
extern bool semBool;
extern volatile uint32_t RIT_count;
void RIT_start(uint32_t count, uint32_t us);




#endif /* RIT_H_ */
