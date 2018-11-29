/*
 * SPIv2.cpp
 *
 *  Created on: Nov 24, 2018
 *      Author: Martin
 */

#include "SPI_ADC.h"


//extern "C" {
//void RIT_IRQHandler(void) {
//
//	InitiateConversion();
//
//	if (--RIT_count == 0) {
//		Chip_RIT_Disable(LPC_RITIMER); // disable timer
//		semBool = 1;
//	}
//
//	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
//
//}
//void SPI0_IRQHandler(void) {
//	uint16_t adc_sample = myspiv2->ReadSample();
//	ar[ar_index] = adc_sample - 1638;
//	ar_index++;
//}
//}


SPI_ADC::SPI_ADC() {

	/*
	 * SPI configuration
	 */
	Chip_SPI_Init(LPC_SPI0);

	// Pin configuration
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	Chip_SWM_MovablePinAssign(SWM_SPI0_SSELSN_0_IO, PORT_CS * 32 + PIN_CS);							// Chip/slave select
	Chip_IOCON_PinMuxSet(LPC_IOCON, PORT_CS, PIN_CS, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));

	Chip_IOCON_PinMuxSet(LPC_IOCON, PORT_CLK, PIN_CLK, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));		// Clock
	Chip_SWM_MovablePinAssign(SWM_SPI0_SCK_IO, PORT_CLK * 32 + PIN_CLK);

	Chip_IOCON_PinMuxSet(LPC_IOCON, PORT_MISO, PIN_MISO, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));	// Master in, slave out
	Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, PORT_MISO * 32 + PIN_MISO);

	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	// SPI configuration
	LPC_SPI0->CFG = SPI_CFG_SPI_EN |				// Enable SPI
					SPI_CFG_MASTER_EN |				// Master mode
					SPI_CFG_MSB_FIRST_EN |			// Data is transmitted and received in MSB first order
					SPI_CFG_CPHA_FIRST |			// CPHA = 0; data is captured on the first edge, changed on the following edge of the clock
					SPI_CFG_CPOL_LO |				// CPOL = 0; clock default(rest) state is low
					SPI_CFG_SPOL0_LO;				// Chip(slave) select 0 is active low

	LPC_SPI0->DIV = 45-1;							// CLK frequency 1.6 MHz

	LPC_SPI0->DLY = 0x0 << 0 |						// PRE_DELAY: delay added between SSEL assertion and first CLK edge (1 clock cycle delay added by default)
					0x0 << 4 |						// POST_DELAY: delay added between the end of data transfer and SSEL deassertion
					0x0 << 8 |						// FRAME_DELAY
					0x0 << 12;						// TRANSFER_DELAY

	LPC_SPI0->TXCTRL = SPI_TXCTL_ASSERT_SSEL0 | 	// Select slave 0
					   SPI_TXCTL_DEASSERT_SSEL1 | 	//
					   SPI_TXCTL_DEASSERT_SSEL2 | 	//
					   SPI_TXCTL_DEASSERT_SSEL3 | 	//
					   SPI_TXCTL_EOT | 				// Deassert after each transmission
					   SPI_TXCTL_LEN(15-1);			// Data length 15


	// SPI interrupt
	LPC_SPI0->INTENSET = SPI_INTENSET_RXRDYEN;	// Enable interrupt when receiver data is available
	NVIC_EnableIRQ(SPI0_IRQn);					// Enable interrupt in NVIC
	NVIC_SetPriority(SPI0_IRQn, 5);

//	/*
//	 * RIT configuration
//	 */
//	Chip_RIT_Init(LPC_RITIMER);					// initialize RIT (= enable clocking etc.)
//	Chip_RIT_SetCounter(LPC_RITIMER, 0);		// reset the counter
//	Chip_RIT_EnableCompClear(LPC_RITIMER);		// Enable automatic clear on when compare value==timer value, this makes interrupts trigger periodically
//	Chip_RIT_SetCompareValue(LPC_RITIMER,
//			(uint64_t) Chip_Clock_GetSystemClockRate() / SAMPLING_RATE);
//	NVIC_EnableIRQ(RITIMER_IRQn);				// Enable the interrupt signal in NVIC
//	NVIC_SetPriority( RITIMER_IRQn, 5 );		// set the priority level of the interrupt

}


//void SPI_ADC::StartSamplingBlocking() {
//
//	semBool = 0;
//	ar_index = 0;
//	RIT_count = NUM_SAMPLES;
//
//	Chip_RIT_Enable(LPC_RITIMER);				// Start counting
//
//	while(semBool == 0) {						// Wait for ISR to tell that we're done
//
//	}
//
//}

/*
 * @brief		Initiates an ADC conversion by enabling the clock signal; interrupt is generated once the data is ready
 */
void SPI_ADC::InitiateConversion() {

	LPC_SPI0->TXDAT = 0x0;		// Writing to transmitter data register initiates transmission

}

/*
 * @brief		Reads the most recent sample received from the ADC and clears the interrupt flag
 * @return		12-bit result of the A/D conversion
 */
uint16_t SPI_ADC::ReadSample() {

	return (LPC_SPI0->RXDAT & 0xFFF);

}

