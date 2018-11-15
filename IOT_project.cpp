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
QueueHandle_t xQueue;
SemaphoreHandle_t xSemaphore;
DigitalIoPin *sw1;
DigitalIoPin *sw2;
DigitalIoPin *sw3;
DigitalIoPin *sw4;
DigitalIoPin *tighten;
DigitalIoPin *loosen;
DigitalIoPin *rs;
DigitalIoPin *en;
DigitalIoPin *d4;
DigitalIoPin *d5;
DigitalIoPin *d6;
DigitalIoPin *d7;
LiquidCrystal *lcd;
int menu,string,i,mode=0;
float detected_frequency;
enum modes{
	automatic,
	manual,
	tuning
};
// TODO: insert other include files here
struct tunes{
	std::string names;
	float E_string,a_string,d_string,g_string,b_string,e_string;
};

tunes pre_tunings[1] = {"E4 a2 d3!g3 b3 e2",82.4,110.0,146.83,196.0,246.9,329.6};
tunes desired_tuning = pre_tunings[0];
enum strings{
	E,
	a,
	d,
	g,
	b,
	e
};
// TODO: insert other definitions and declarations here
extern "C" {
void PIN_INT0_IRQHandler(void){

}

void PIN_INT1_IRQHandler(void){


}
}

static void vDisplay(void *param) {
	std::string manual_str = "     MANUAL";
	const std::string automatic_str = "  E a d g b e  !"+ std::to_string(detected_frequency)+" -> ";
	std::string automatic_str_temp;
	std::string half_names;
	while(1){
		switch(menu){
		case manual:
			lcd->print(manual_str);
			break;
		case automatic:
			if (string == 0){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.E_string) ;
			}
			else if (string == 1){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.a_string) ;
			}
			else if (string == 2){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.d_string) ;
			}
			else if (string == 3){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.g_string) ;
			}
			else if (string == 4){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.b_string) ;
			}
			else if (string == 5){
				automatic_str_temp = automatic_str + std::to_string(desired_tuning.e_string) ;
			}

			automatic_str_temp[(string*2)+3] = ' ';
			lcd->print(automatic_str_temp);
			break;
		case tuning:
			lcd->print(pre_tunings[i].names);
			break;

		}
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
	double _Ki;

	double Dout;

	// Calculate total output
	double output;
	while(1){
		if(mode==automatic){
			error = desired_tuning - detected_frequency;
			Pout = _Kp * error;

			_integral += error;
			Iout = _Ki * _integral;

			derivative = (error - _pre_error);
			Dout = _Kd * derivative;

			output = Pout + Iout + Dout;

			_pre_error = error;
		}
	}
}
static void vMotor(void *param) {
	int cnt=0;

	while(1){
		switch(menu){

		case manual:
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
				while(sw1->read()){
					vTaskDelay(1);
				}
				loosen->write(false);
			}
			if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
					cnt++;
				}
				if(cnt >= 1000){
					menu = automatic;
					mode = automatic;
				}
				cnt=0;
			}
			break;
		case automatic:
			if(sw1->read() && string > 0){
				string--;
			}
			if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
					cnt++;
				}
				if(cnt >= 1000){
					menu = manual;
					mode = manual;
				}
				cnt=0;
			}
			if(sw3->read()){
				menu = tuning;
			}
			if(sw4->read() && string < 6){
				string++;
			}
			break;

			/*case automatic_string:
			if(sw1->read()){
				desired_frequency=desired_frequency-0.1;
			}
			if(sw4->read()){
				desired_frequency=desired_frequency+0.1;
			}
			break;*/

		case tuning:
			if(sw1->read() && i > 0){
				i--;
			}
			if(sw2->read()){
				menu = mode;
			}
			if(sw3->read()){
				menu = mode;
				desired_tuning = pre_tunings[i];
			}
			if(sw4->read() && i<?){
				i++;
			}
			break;
		}
		vTaskDelay(10);
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
#endif
#endif

	// TODO: insert code here
	Chip_PININT_Init(LPC_GPIO_PIN_INT);
	sw1 = new DigitalIoPin(0, 17, true, true, true);
	sw2 = new DigitalIoPin(1, 11, true, true, true);
	sw3 = new DigitalIoPin(1, 9, true, true, true);
	sw4 = new DigitalIoPin(1, 9, true, true, true);
	tighten = new DigitalIoPin(,false,false,false);
	loosen = new DigitalIoPin(,false,false,false);

	loosen->write(false);
	tighten->write(false);

	rs = new DigitalIoPin(0, 8, false, true, false);
	en = new DigitalIoPin(1, 6, false, true, false);
	d4 = new DigitalIoPin(1, 8, false, true, false);
	d5 = new DigitalIoPin(0, 5, false, true, false);
	d6 = new DigitalIoPin(0, 6, false, true, false);
	d7 = new DigitalIoPin(0, 7, false, true, false);
	lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);

	LPC_INMUX->PINTSEL[0] = 17;
	LPC_INMUX->PINTSEL[1] = 43;

	LPC_GPIO_PIN_INT->SIENF = 3;
	LPC_GPIO_PIN_INT->ISEL = 0;

	NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	NVIC_SetPriority(PIN_INT1_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);

	xSemaphore = xSemaphoreCreateBinary();
	xQueue = xQueueCreate(5,sizeof (tunes));
	xTaskCreate(vDisplay, "vDisp",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vMotor, "vMotor",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vFrequency, "vFreq",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(vPDI, "vPDI",
			configMINIMAL_STACK_SIZE *3 , NULL, (tskIDLE_PRIORITY + 1UL),
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
