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

#define manual	( 1 << 0 )
#define automatic	( 1 << 1 )
#define tuning	( 1 << 2 )
#define temp_manual	(1 << 3)

Fmutex *mutex;
SemaphoreHandle_t xSemaphore;
EventGroupHandle_t xEventGroup;
DigitalIoPin *out, *sw1, *sw2, *sw3, *sw4, *tighten, *loosen, *rs, *en, *d4, *d5, *d6, *d7;
LiquidCrystal *lcd;

int string,viewed_tuning=0;

float detected_frequency = 155.6;
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
// TODO: insert other definitions and declarations here
/*extern "C" {
void PIN_INT0_IRQHandler(void){

}

void PIN_INT1_IRQHandler(void){


}
}
 */
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
			lcd->print(manual_str);
		}

		else if((bits & automatic) != 0){
			char* note = getNote(detected_frequency);
			sprintf(buffer,"%s %.1f->%.1f %s",note,detected_frequency,desired_tuning.strings[string],desired_tuning.names[string].c_str());
			temp = automatic_str + buffer;
			if(counter%2==0 && counter %4!=0){

				temp[(string*2)+6] = ' ';
			}
			lcd->print(temp);
			counter++;
		}

		else if((bits & tuning) != 0){
			if(viewed_tuning==0){
				temp = "     CUSTOM!B2=cancel B3=select";
				lcd->print(temp);
			}
			else{
				sprintf(buffer,"%s %s %s %s %s %s!B2=cancel B3=select",pre_tunings[viewed_tuning].names[0].c_str(),pre_tunings[viewed_tuning].names[1].c_str(),pre_tunings[viewed_tuning].names[2].c_str(),
						pre_tunings[viewed_tuning].names[3].c_str(),pre_tunings[viewed_tuning].names[4].c_str(),pre_tunings[viewed_tuning].names[5].c_str());
				temp=buffer;
				lcd->print(temp);
			}
		}


		vTaskDelay(100);
	}
}
static void vPID(void *param) {
	double error;

	// Proportional term
	double Pout;
	double _Kp;

	// Integral term
	double _integral;
	double Iout;
	double _Ki;

	// Derivative term
	double derivative;
	double _pre_error = 0;
	double _Kd;

	double Dout;

	// Calculate total output
	float output;
	while(1){

		if((xEventGroupGetBits(xEventGroup) & automatic) != 0){
			error = desired_tuning.strings[string] - detected_frequency;
			Pout = _Kp * error;

			_integral += error;
			Iout = _Ki * _integral;

			derivative = (error - _pre_error);
			Dout = _Kd * derivative;

			output = Pout + Iout + Dout;

			_pre_error = error;

			//use sct

			//LPC_SCT0->MATCHREL[1].L =


			/*tighten->write(false);
			loosen->write(true);
			vTaskDelay(10);
			tighten->write(false);		//test program
			loosen->write(false);
			vTaskDelay(1000);
			 */


			/*if (output<0){
				tighten->write(false);
				loosen->write(true);

			}
			else{
				tighten->write(true);
				loosen->write(false);
			}
			 */
			//vTaskDelay(output*?);
		}
		vTaskDelay(100);
	}
}
static void vFrequency(void *param) {
	while(1){
		//calculations and shit
		/*detected_frequency = ?;		//use mutex when writing the freq
		vTaskDelay(?);*/
		vTaskDelay(10);
	}
}
static void vMotor(void *param) {
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
				mutex->lock();
				string--;
				mutex->unlock();
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
				mutex->lock();
				string++;
				mutex->unlock();
				while(sw4->read()){
					vTaskDelay(1);
				}
			}
		}


		/*case automatic_string:
			if(sw1->read()){
				desired_frequency=desired_frequency-0.1;
			}
			if(sw4->read()){
				desired_frequency=desired_frequency+0.1;
			}
			break;*/

		//tuning
		if((bits & tuning) != 0){
			if(sw1->read() && viewed_tuning > 0){
				while(sw1->read()){
					vTaskDelay(1);
				}
				mutex->lock();
				viewed_tuning--;
				mutex->unlock();
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
				desired_tuning = pre_tunings[viewed_tuning];
			}

			if(sw4->read() && viewed_tuning<pre_tuning_total-1){
				while(sw4->read()){
					vTaskDelay(1);
				}
				mutex->lock();
				viewed_tuning++;
				mutex->unlock();
			}

		}



		vTaskDelay(10);
	}
}

tunes* parse(std::string sent){
	std::string temp;
	int count=0;
	tunes *received_notes = new tunes;
	int e;
	for(e=0;e<sent.length();e++){
		if(sent[e] == '='){
			e++;
			while(sent[e]!='|' && temp.length() != 3){
				temp.push_back(sent[e]);
				e++;
			}
			if(lookupNote(temp.c_str()) != -1){
				received_notes->names[count]=temp;
				received_notes->strings[count]=lookupNote(temp.c_str());
				count++;
			}
			else {
				received_notes->strings[0]=-1.0;
			}
			temp="";

		}
	}
	return received_notes;
}


static void receive_task(void *pvParameters) {
	std::string sentence;

	int cnt=0;
	char str[80];
	int a;
	int32_t len;
	vTaskDelay(10);

	while(1){
		len = USB_receive((uint8_t *)str, 79);

		for(a=0;a<len;a++){
			if(str[a] != '!' && cnt <=47){
				sentence = sentence + str[a];
				cnt++;
			}
			else
			{
				cnt=0;


				tunes *freqs =  parse(sentence);
				if(freqs->strings[0] != -1){
					mutex->lock();
					memcpy(pre_tunings,freqs, sizeof (*freqs));
					mutex->unlock();
				}
				delete freqs;
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

	// TODO: insert code here
	Chip_SCT_Init(LPC_SCT0);
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O,0,12);
	SCT_Init();

	sw1 = new DigitalIoPin(0, 9, true, true, true);
	sw2 = new DigitalIoPin(0, 10, true, true, true);
	sw3 = new DigitalIoPin(1, 3, true, true, true);
	sw4 = new DigitalIoPin(0, 29, true, true, true);

	tighten = new DigitalIoPin(1, 10, false, true, true);
	loosen = new DigitalIoPin(1, 9, false, true, true);

	out->write(true);
	loosen->write(false);
	tighten->write(false);

	rs = new DigitalIoPin(0, 8, false, true, false);
	en = new DigitalIoPin(1, 6, false, true, false);
	d4 = new DigitalIoPin(1, 8, false, true, false);
	d5 = new DigitalIoPin(0, 5, false, true, false);
	d6 = new DigitalIoPin(0, 6, false, true, false);
	d7 = new DigitalIoPin(0, 7, false, true, false);
	lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);

	lcd->begin(18,2);
	/*LPC_INMUX->PINTSEL[0] = 17;
	LPC_INMUX->PINTSEL[1] = 43;

	LPC_GPIO_PIN_INT->SIENF = 3;
	LPC_GPIO_PIN_INT->ISEL = 0;

	NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	NVIC_SetPriority(PIN_INT1_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);*/

	xSemaphore = xSemaphoreCreateBinary();

	xEventGroup = xEventGroupCreate();

	xEventGroupSetBits(xEventGroup,manual);

	mutex = new Fmutex();

	xTaskCreate(vDisplay, "vDisp",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vMotor, "vMotor",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vFrequency, "vFreq",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vPID, "vPID",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(receive_task, "Rx",
			configMINIMAL_STACK_SIZE * 8, NULL, (tskIDLE_PRIORITY + 1UL),
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
