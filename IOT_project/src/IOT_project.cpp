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
#include "user_vcom.h"
SemaphoreHandle_t xSemaphore;
DigitalIoPin *out;
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
	manual,
	automatic,
	tuning
};
// TODO: insert other include files here
struct tunes{
	std::string names;
	float strings[6];
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

	LPC_SCT0->MATCHREL[1].L = 1000; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12);
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0

	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
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
			automatic_str_temp = automatic_str + std::to_string(desired_tuning.strings[string]) ;
			automatic_str_temp[(string*2)+3] = ' ';
			lcd->print(automatic_str_temp);
			break;
		case tuning:
			lcd->print(pre_tunings[i].names);
			break;

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
	double output;
	while(1){

		if(mode==automatic){
			error = desired_tuning.strings[string] - detected_frequency;
			Pout = _Kp * error;

			_integral += error;
			Iout = _Ki * _integral;

			derivative = (error - _pre_error);
			Dout = _Kd * derivative;

			output = Pout + Iout + Dout;

			_pre_error = error;

			if (output<0){
				tighten->write(false);
				loosen->write(true);

			}
			else{
				tighten->write(true);
				loosen->write(false);
			}
			//vTaskDelay(output*?);
		}
		vTaskDelay(100);
	}
}
static void vFrequency(void *param) {
	while(1){
		//calculations and shit
		/*detected_frequency = ?;
		vTaskDelay(?);*/
		vTaskDelay(10);
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
				while(sw4->read()){
					vTaskDelay(1);
				}
				loosen->write(false);
			}
			/*if(sw2->read()){
				while(sw2->read()){
					vTaskDelay(1);
					cnt++;
				}
				if(cnt >= 1000){
					menu = automatic;
					mode = automatic;
				}
				cnt=0;
			}*/
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
			/*
			 * if(sw4->read() && i<?){
				i++;
			}
			 */
			break;


		}
		vTaskDelay(10);
	}
}

float parse(std::string sent, char const *parser){
	std::string tmp;
	char c;
	std::size_t found;

	found = sent.find(parser);
	c = sent[(found+2)];
	int i=1;
	while (c!= '|' && i <6){
		tmp.append(1,c);
		c = sent[found+2+i];
		i++;
	}
	return std::stof(tmp);
}

static void receive_task(void *pvParameters) {
	std::string sentence;

	int cnt=0;
	char str[80];
	uint i;
	uint32_t len;
	vTaskDelay(10);

	while(1){
		len = USB_receive((uint8_t *)str, 79);

		for(i=0;i<len;i++){
			if(str[i] != '!' && cnt <=47){
				sentence = sentence + str[i];
				cnt++;
			}
			else
			{
				cnt=0;

				desired_tuning.strings[0]=parse(sentence,"E=");		// check values
				desired_tuning.strings[1]=parse(sentence,"a=");
				desired_tuning.strings[2]=parse(sentence,"d=");
				desired_tuning.strings[3]=parse(sentence,"g=");
				desired_tuning.strings[4]=parse(sentence,"b=");
				desired_tuning.strings[5]=parse(sentence,"e=");
			}


		}
	}
	vTaskDelay(1);
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

	out = new DigitalIoPin(0, 29, false, true, true);

	sw1 = new DigitalIoPin(0, 9, true, true, true);
	//sw2 = new DigitalIoPin(1, 9, true, true, true);
	//sw3 = new DigitalIoPin(1, 9, true, true, true);
	sw4 = new DigitalIoPin(0, 10, true, true, true);

	tighten = new DigitalIoPin(1, 10, false, true, true);
	loosen = new DigitalIoPin(1, 9, false, true, true);

	out->write(true);
	loosen->write(false);
	tighten->write(false);

	rs = new DigitalIoPin(0, 8, false, true, false);
	rs->write(true);
	en = new DigitalIoPin(1, 6, false, true, false);
	d4 = new DigitalIoPin(1, 8, false, true, false);
	d5 = new DigitalIoPin(0, 5, false, true, false);
	d6 = new DigitalIoPin(0, 6, false, true, false);
	d7 = new DigitalIoPin(0, 7, false, true, false);
	lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);

	/*LPC_INMUX->PINTSEL[0] = 17;
	LPC_INMUX->PINTSEL[1] = 43;

	LPC_GPIO_PIN_INT->SIENF = 3;
	LPC_GPIO_PIN_INT->ISEL = 0;

	NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	NVIC_SetPriority(PIN_INT1_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);*/

	xSemaphore = xSemaphoreCreateBinary();


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
