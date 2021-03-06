#ifndef SPI_ADC_H_
#define SPI_ADC_H_

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#define PORT_CS		0
#define PIN_CS		5
#define PORT_CLK	0
#define PIN_CLK		7
#define PORT_MISO	0
#define PIN_MISO	6

#define NUM_SAMPLES		570
#define SAMPLING_RATE	20000

/*
 * SPI peripheral configured to get samples from MCP3201
 */

class SPI_ADC {
	public:
		SPI_ADC();
		//void StartSamplingBlocking(u);
		void InitiateConversion();
		uint16_t ReadSample();
		volatile int16_t samples[NUM_SAMPLES];
	private:

};

#endif /* SPI_ADC_H_ */
