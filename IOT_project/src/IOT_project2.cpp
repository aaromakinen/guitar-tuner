/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "DigitalIoPin.h"
#include "LiquidCrystal.h"
#include "string.h"
#include "math.h"
#include "user_vcom.h"
#include "event_groups.h"
#include "Fmutex.h"
#include "notes.h"
#include <algorithm>
#include "SPI_ADC.h"
#include "RIT.h"
#include <atomic>

extern "C" {
#include "Yin.h"
}

#define manual	( 1 << 0 )
#define automatic	( 1 << 1 )
#define tuning	( 1 << 2 )
#define temp_manual	(1 << 3)

Fmutex *mutex;
Fmutex *ritmutex;
SemaphoreHandle_t pidSem;
EventGroupHandle_t xEventGroup;
DigitalIoPin *out, *sw1, *sw2, *sw3, *sw4, *tighten, *loosen, *rs, *en, *d4, *d5, *d6, *d7,*red_led,*green_led;
LiquidCrystal *lcd;

std::atomic<int> string{0};
std::atomic<int> viewed_tuning {0};



float detected_frequency = 0;
// TODO: insert other include files here
struct tunes{
	std::string names[6];
	float strings[6];
};

tunes pre_tunings[3] = {
		{"","","","","","",82.4,110.0,146.83,196.0,246.9,329.6},
		{"E2","A2","D3","G3","B3","E4",82.4,110.0,146.83,196.0,246.9,329.6},
		{"D2","A2","D3","G3","B3","E4",73.4,110.0,146.83,196.0,246.9,329.6}
};
tunes desired_tuning = pre_tunings[1];
enum strings{
	E,
	a,
	d,
	g,
	b,
	e
};

SPI_ADC* spiadc0;

static int16_t ar[NUM_SAMPLES];
int ar_index = 0;
bool semBool = 0;
volatile uint32_t RIT_count;
bool rit_spi = true;

extern "C"{
void RIT_IRQHandler(void) {

	if (rit_spi){
		spiadc0->InitiateConversion();
	}

	if (--RIT_count == 0) {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		semBool = 1;
	}

	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag

}

void SPI0_IRQHandler(void) {
	uint16_t adc_sample = spiadc0->ReadSample();
	ar[ar_index] = adc_sample - 2048;
	ar_index++;
}
}


void SCT_Init(void)
{
	LPC_SCT0->CONFIG = SCT_CONFIG_16BIT_COUNTER | SCT_CONFIG_AUTOLIMIT_L; // two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT0->MATCHREL[0].L = 2550-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12);

	LPC_SCT0->MATCHREL[1].L = 2550; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12);
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0

	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

static void vDisplay(void *param) {
	char buffer[41];
	std::string manual_str = "     MANUAL MODE!B1=loosen B4=tighten";
	const std::string automatic_str = "AUTO  6 5 4 3 2 1!";
	std::string temp;
	std::string half_names;
	int counter = 0;
	EventBits_t bits;
	while(1){
		bits = xEventGroupGetBits(xEventGroup);

		if((bits & manual) != 0){
			ritmutex->lock();
			rit_spi = false;
			lcd->print(manual_str);
			ritmutex->unlock();
		}

		else if((bits & automatic) != 0){
			mutex->lock();
			char* note = getNote(detected_frequency);
			sprintf(buffer,"%s %.1f->%.1f %s",note,detected_frequency,desired_tuning.strings[string],desired_tuning.names[string].c_str());
			mutex->unlock();
			temp = automatic_str + buffer;
			if(counter%2==0 && counter %4!=0){

				temp[(string*2)+6] = ' ';
			}
			ritmutex->lock();
			rit_spi = false;
			lcd->print(temp);
			ritmutex->unlock();
			counter++;
		}

		else if((bits & tuning) != 0){
			if(viewed_tuning==0){
				temp = "     CUSTOM!B2=cancel B3=select";
				ritmutex->lock();
				rit_spi = false;
				lcd->print(temp);
				ritmutex->unlock();
			}
			else{
				mutex->lock();
				sprintf(buffer,"%s %s %s %s %s %s!B2=cancel B3=select",pre_tunings[viewed_tuning].names[0].c_str(),pre_tunings[viewed_tuning].names[1].c_str(),pre_tunings[viewed_tuning].names[2].c_str(),
						pre_tunings[viewed_tuning].names[3].c_str(),pre_tunings[viewed_tuning].names[4].c_str(),pre_tunings[viewed_tuning].names[5].c_str());
				mutex->unlock();
				temp=buffer;
				ritmutex->lock();
				rit_spi = false;
				lcd->print(temp);
				ritmutex->unlock();
			}
		}


		vTaskDelay(100);
	}
}

// Motor control task
static void vPID(void *param) {
	float error;
	float abserror;

	// Maximum error from target freq. Beyond this, ignore. Used as safety feature.
	float tolerance = 20.0;

	// Determines how long motor will run.
	TickType_t tickVar = portMAX_DELAY;

	// Proportional constant
	float _Kp = 100.0;

	while(1){

		// Run motor for previously calculated interval unless new reading is received sooner
		if (xSemaphoreTake(pidSem, tickVar) == pdTRUE) {

			// Check that program is in automatic mode
			if ((xEventGroupGetBits(xEventGroup) & automatic) != 0) {
				mutex->lock();
				error = desired_tuning.strings[string] - detected_frequency;
				mutex->unlock();

				// Calculate absolute value of error
				if (error > 0) {
					abserror = error;
				} else {
					abserror = error * -1;
				}

				// Update time to run motor during next iteration
				tickVar = _Kp * abserror;

				// Status LEDs
				if(abserror > 0.3){
					red_led->write(true);
					green_led->write(false);
				}
				else{
					red_led->write(false);
					green_led->write(true);

				}

				// Determine motor direction
				if (abserror < tolerance && abserror > 0.3) {
					if (error < 0){
						tighten->write(false);
						loosen->write(true);
					}
					else{
						tighten->write(true);
						loosen->write(false);
					}
				} else {
					tighten->write(false);
					loosen->write(false);
				}

			// If mode is switched, stop motor and turn off LEDs
			} else {
				red_led->write(false);
				green_led->write(false);

				tighten->write(false);
				loosen->write(false);
			}

		// Turn motor off at end of calculate time
		} else {
			tighten->write(false);
			loosen->write(false);
		}
	}
}

// YIN fundamental frequency calculation from ADC samples
static void vFrequency(void *param) {
	spiadc0 = new SPI_ADC();
	Yin yin;
	Yin_init(&yin, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD); // 95% accuracy setting with NUM_SAMPLES samples
	float temporary;
	while(1){
		ritmutex->lock();
		rit_spi = true; // Enable ADC sampling in RIT
		RIT_start(NUM_SAMPLES, 50);
		ritmutex->unlock();
		temporary = Yin_getPitch(&yin, ar);
		if (temporary != -1.0) { // If new frequency is read (value returned is -1 otherwise)
			mutex->lock();
			detected_frequency = temporary;
			mutex->unlock();
			xSemaphoreGive(pidSem); // Trigger motor movement
		}
		vTaskDelay(10);
	}
}

// Menu/mode settings and manual mode motor control task
static void vMenu(void *param) {
	int cnt=0;
	int pre_tuning_total = 3;
	EventBits_t bits;
	EventBits_t temp_bits;
	while(1){
		bits=xEventGroupGetBits(xEventGroup);

		//manual
		if((bits & manual) != 0){
			if(sw1->read()){
				tighten->write(true);
				loosen->write(false);
				while(sw1->read()){
					vTaskDelay(1);
				}
				tighten->write(false);

			}
			if(sw4->read()){
				tighten->write(false);
				loosen->write(true);
				while(sw4->read()){
					vTaskDelay(1);
				}
				loosen->write(false);
			}
			if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
				}


				xEventGroupClearBits(xEventGroup,manual);
				xEventGroupSetBits(xEventGroup,tuning | temp_manual);
			}
			if(sw3->read()){
				while(sw3->read() && cnt <= 500){
					vTaskDelay(1);
					cnt++;
				}
				if(cnt >= 500){
					xEventGroupClearBits(xEventGroup,manual);
					xEventGroupSetBits(xEventGroup,automatic);
				}
				cnt=0;
			}
		}

		//automatic
		if((bits & automatic) != 0){
			if(sw1->read() && string > 0){
				while(sw1->read()){
					vTaskDelay(1);
				}
				string--;
			}
			if(sw3->read()){
				while(sw3->read() && cnt <= 500){
					vTaskDelay(1);
					cnt++;
				}
				if(cnt >= 500){
					xEventGroupClearBits(xEventGroup,automatic);
					xEventGroupSetBits(xEventGroup,manual);
				}
				cnt=0;
			}
			if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
				}
				xEventGroupClearBits(xEventGroup,automatic);
				xEventGroupSetBits(xEventGroup,tuning);
			}
			if(sw4->read() && string < 5){
				string++;
				while(sw4->read()){
					vTaskDelay(1);
				}
			}
		}

		//tuning
		if((bits & tuning) != 0){
			if(sw1->read() && viewed_tuning > 0){
				while(sw1->read()){
					vTaskDelay(1);
				}
				viewed_tuning--;
			}
			if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
				}
				temp_bits=xEventGroupGetBits(xEventGroup);
				xEventGroupClearBits(xEventGroup,tuning);

				if((temp_bits & temp_manual) != 0){
					xEventGroupClearBits(xEventGroup,temp_manual);
					xEventGroupSetBits(xEventGroup,manual);
				}
				else {
					xEventGroupSetBits(xEventGroup,automatic);
				}
			}
			if(sw3->read()){
				while(sw3->read()){
					vTaskDelay(1);
				}
				xEventGroupClearBits(xEventGroup,tuning);

				if((temp_bits & temp_manual) != 0){
					xEventGroupClearBits(xEventGroup,temp_manual);
					xEventGroupSetBits(xEventGroup,manual);
				}
				else {
					xEventGroupSetBits(xEventGroup,automatic);
				}
				mutex->lock();
				desired_tuning = pre_tunings[viewed_tuning];
				mutex->unlock();
			}

			if(sw4->read() && viewed_tuning<pre_tuning_total-1){
				while(sw4->read()){
					vTaskDelay(1);
				}
				viewed_tuning++;
			}

		}



		vTaskDelay(10);
	}
}

// Parse frequencies received via USB
tunes* parse(std::string sent){
	std::string temp;
	int count=5;
	tunes *received_notes = new tunes;
	float temporary;
	int e;
	e = sent.find('=');
	if (std::string::npos == e){
		e = sent.length();
	}
	for(e=0;e<sent.length();e++){
		if(sent[e] == '='){
			e++;
			while(sent[e]!='|' && temp.length() != 6){
				temp.push_back(sent[e]);
				e++;
			}
			if (temp.length()>=4){
				temporary = stof(temp);
				if (temporary >70 && temporary < 550){
					temp = getNote(temporary);
					if(temp.length()>=2){
						received_notes->names[count] = temp;
						received_notes->strings[count] = temporary;
					}
					else{
						received_notes->strings[0] = -1;
					}
				}
				else{
					received_notes->strings[0] = -1;
				}
			}
			else{
				received_notes->strings[0] = -1;
			}

			count--;
			temp="";

		}
	}
	return received_notes;
}

// USB receive task
static void vReceive(void *pvParameters) {
	std::string sentence;

	int cnt=0;
	char str[80];
	int a;
	int32_t len;
	vTaskDelay(1000);

	while(1){

		len = USB_receive((uint8_t *)str, 80);

		for(a=0;a<len;a++){
			if(str[a] != '!' && cnt <=47){
				sentence = sentence + str[a];
				cnt++;
			}
			else
			{
				cnt=0;

				if(sentence.length() >= 42){
					tunes *freqs =  parse(sentence);
					if(freqs->strings[0] != -1){
						mutex->lock();
						pre_tunings[0] = *freqs;
						mutex->unlock();
					}
					delete freqs;
				}
				sentence = "";

			}


		}

		vTaskDelay(1);
	}

}


int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Chip_RIT_Init(LPC_RITIMER);
#endif
#endif

	Chip_SCT_Init(LPC_SCT0);
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O,0, 8);
	SCT_Init();

	sw1 = new DigitalIoPin(0, 24, true, true, true);
	sw2 = new DigitalIoPin(1, 0, true, true, true);
	sw3 = new DigitalIoPin(0, 27, true, true, true);
	sw4 = new DigitalIoPin(0, 28, true, true, true);

	tighten = new DigitalIoPin(1, 8, false, true, true);
	loosen = new DigitalIoPin(1, 6, false, true, true);

	out->write(true);
	loosen->write(false);
	tighten->write(false);
	red_led = new DigitalIoPin(0, 14, false, false, false);
	green_led = new DigitalIoPin(0, 12, false, false, false);
	red_led->write(false);
	green_led->write(false);
	rs = new DigitalIoPin(0, 29, false, true, false);
	en = new DigitalIoPin(0, 9, false, true, false);
	d4 = new DigitalIoPin(0, 10, false, true, false);
	d5 = new DigitalIoPin(1, 9, false, true, false);
	d6 = new DigitalIoPin(1, 3, false, true, false);
	d7 = new DigitalIoPin(0, 0, false, true, false);
	lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);

	lcd->begin(18,2);

	Chip_RIT_Init(LPC_RITIMER);				// initialize RIT (= enable clocking etc.)
	NVIC_SetPriority( RITIMER_IRQn, 5 );	// set the priority level of the interrupt

	pidSem = xSemaphoreCreateBinary();

	xEventGroup = xEventGroupCreate();

	xEventGroupSetBits(xEventGroup,manual);

	mutex = new Fmutex();
	ritmutex = new Fmutex();

	xTaskCreate(vDisplay, "vDisp",
			configMINIMAL_STACK_SIZE *4 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vMenu, "vMenu",
			configMINIMAL_STACK_SIZE *1 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vFrequency, "vFreq",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vPID, "vPID",
			configMINIMAL_STACK_SIZE *1 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vReceive, "vRcv",
			configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	vTaskStartScheduler();

	// Force the counter to be placed into memory
	volatile static int i = 0 ;
	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
	return 0 ;
}
