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
#include "user_vcom.h"
#include <move.h>
#include "parsed.h"
#include "ITM_write.h"
#include <mutex>
#include "Fmutex.h"
#include "DigitalIoPin.h"


move *mover;
parsed *par;
DigitalIoPin *motorX;
DigitalIoPin *motorY;
DigitalIoPin *sw1;
DigitalIoPin *sw2;
DigitalIoPin *sw3;
DigitalIoPin *sw4;
DigitalIoPin *limitx_1;
DigitalIoPin *limitx_2;
DigitalIoPin *limity_1;
DigitalIoPin *limity_2;
DigitalIoPin *laser;
volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT;
volatile static bool ini_x = false;
volatile static bool ini_y = false;

extern "C" {
void RIT_IRQHandler(void)
{
	static bool state = false;

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		if(RIT_count > mover->xmove || RIT_count > mover->ymove) {
			if(mover->dirx == false){
				if( (RIT_count > mover->xmove) && !limitx_1->read() ){
					// do something useful here...
					motorX->write(state);

				}
				if( (RIT_count > mover->ymove) && !limity_2->read()){
					motorY->write(state);
				}

			}
			else{
				if( (RIT_count > mover->xmove) && !limitx_2->read() ){
					motorX->write(state);

				}
				if( (RIT_count > mover->ymove) && !limity_1->read() ){
					motorY->write(state);

				}

			}


			state = !(bool)state;
			RIT_count--;

		}
	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}
void RIT_start(int count, int us)
{
	uint64_t cmp_value;
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
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}

void SCT_Init(void)
{
	LPC_SCT0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT0->MATCHREL[0].L = 20000-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT0->MATCHREL[1].L = 1000; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

static void ini(void *pvParameters) {
	vTaskDelay(100);

	int cnt = 0;
	int back_cnt = 0;
	int i;
	bool temp = false;
	mover->change_dir(0,false);
	ini_x = true;
	while ((!sw1->read()) && (!sw2->read()) && (!sw3->read()) && (!sw4->read()) ){
		motorX->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);

	}
	if (sw1->read()){
		limitx_1 = sw1;
	}
	else if (sw2->read()){
		limitx_1 = sw2;			//does the limit assigning work when limits are switched ???
	}
	else if (sw3->read()){
		limitx_1 = sw3;
	}
	else if (sw4->read()){
		limitx_1 = sw4;
	}
	mover->change_dir(0,true);
	while (sw1->read() || sw2->read() || sw3->read() || sw4->read() ){
		motorX->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		back_cnt++;
	}
	//second switch

	while (!sw1->read() && !sw2->read() && !sw3->read() && !sw4->read() ){

		motorX->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		cnt++;
	}
	if (sw1->read()){
		limitx_2 = sw1;
	}
	else if (sw2->read()){
		limitx_2 = sw2;
	}
	else if (sw3->read()){
		limitx_2 = sw3;
	}
	else if (sw4->read()){
		limitx_2 = sw4;
	}
	if (cnt %2 !=0){
		cnt++;
	}

	mover->change_dir(0,false);
	while (sw1->read() || sw2->read() || sw3->read() || sw4->read() ){
		motorX->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		back_cnt++;
	}
	mover->max_x =cnt - back_cnt;
	ini_x = false;

	// y-axle
	ini_y = true;
	cnt = 0;
	back_cnt = 0;
	mover->change_dir(1,false);

	while (!sw1->read() && !sw2->read() && !sw3->read() && !sw4->read() ){

		motorY->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
	}
	if (sw1->read()){
		limity_1 = sw1;
	}
	else if (sw2->read()){
		limity_1 = sw2;
	}
	else if (sw3->read()){
		limity_1 = sw3;
	}
	else if (sw4->read()){
		limity_1 = sw4;
	}
	mover->change_dir(1,true);
	while (sw1->read() || sw2->read() || sw3->read() || sw4->read() ){
		motorY->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		back_cnt++;
	}

	//second switch

	while (!sw1->read() && !sw2->read() && !sw3->read() && !sw4->read() ){

		motorY->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		cnt++;
	}
	if (sw1->read()){
		limity_2 = sw1;
	}
	else if (sw2->read()){
		limity_2 = sw2;
	}
	else if (sw3->read()){
		limity_2 = sw3;
	}
	else if (sw4->read()){
		limity_2 = sw4;
	}
	if (cnt %2 !=0){
		cnt++;
	}

	mover->change_dir(1,false);
	while (sw1->read() || sw2->read() || sw3->read() || sw4->read() ){
		motorY->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
		back_cnt++;
	}
	mover->max_y =cnt -back_cnt ;
	ini_y = false;

	for(i=0;i<(mover->max_x);i++){
		motorX->write(temp);
		temp = !(bool)temp;
		vTaskDelay(1);
	}
	while(1){
		vTaskDelay(1000);
	}



}

static void loop(void *pvParameters) {
	vTaskDelay(10);
	int steps;			//check speed !!!
	int base_speed = 400;
	int max_speed = 30;
	int step_cnt = 0;
	int i;
	while(1){
		par->parser();
		if (par->command=='M'){
			switch (par->command_index){
			case 10:

				USB_send((uint8_t *)"M10 XY 380 310 0.00 0.00 A0 B0 H0 S80 U160 D90\n",49);
				break;
			case 90:
				LPC_SCT0->MATCHREL[1].L = 1255;
				vTaskDelay(100);				//correct delay for serv0  and angles ???
				break;
			case 160:
				LPC_SCT0->MATCHREL[1].L = 1000;		//sometimes leaves lines???
				vTaskDelay(100);
				break;
			case 4:
				//laser
				break;
			}
		}
		if(par->command == 'G'){
			switch (par->command_index){
			case 1:
				steps = mover->desired_move(par->x, par->y);
				if (steps > 0){
					RIT_start(steps, base_speed);
					/*if(steps > 20){
						for(i=base_speed;i<=base_speed;i=i+10){
							RIT_start(10, i);
							step_cnt=step_cnt+10;
						}
						RIT_start(steps-(step_cnt*2), max_speed);

						for(i=max_speed;i>=base_speed;i=i-10){
							RIT_start(10, i);
						}					//acc and deceleration ???
					}
					else {
						RIT_start(steps, base_speed);
					}
					 */
					step_cnt = 0;
				}
				break;
			case 28:
				steps = mover->desired_move(0, 0);    //Probably not necessary
				if (steps > 0){
					RIT_start(steps,base_speed);
				}
				break;
			}
		}
	}

}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();											//something produces hard fault (one time had)
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
#endif
#endif

	// TODO: insert code here
	ITM_init();
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority( RITIMER_IRQn, 5 );
	sbRIT = xSemaphoreCreateBinary();

	Chip_SCT_Init(LPC_SCT0);
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O,0,10);
	SCT_Init();

	laser = new DigitalIoPin (0,12,false,true,false);
	laser->write(false);
	mover= new move;
	par = new parsed;
	motorX = new DigitalIoPin(0,27,false,false,false);
	motorY = new DigitalIoPin(0,24,false,false,false);


	sw1 = new DigitalIoPin(1,3,true,true,true);
	sw2 = new DigitalIoPin(0,0,true,true,true);
	sw3 = new DigitalIoPin(0,9,true,true,true);
	sw4 = new DigitalIoPin(0,29,true,true,true);




	xTaskCreate(ini, "ini",
			configMINIMAL_STACK_SIZE * 8, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(loop, "main",
			configMINIMAL_STACK_SIZE * 8, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE * 8, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	volatile static int i = 0 ;
	vTaskStartScheduler();
	// Force the counter to be placed into memory
	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
	return 0 ;
}
