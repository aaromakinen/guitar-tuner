/*
 * RIT.cpp
 *
 *  Created on: 29.11.2018
 *      Author: aaro
 */

#include "RIT.h"

void RIT_start(uint32_t count, uint32_t us) {
	uint64_t cmp_value;

	semBool = 0;
	ar_index = 0;

	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;

	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);

	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);

	// wait for ISR to tell that we're done
	while(semBool == 0) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
	}

	NVIC_DisableIRQ(RITIMER_IRQn);
}
