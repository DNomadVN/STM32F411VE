/*
*  ==============================================================================
*                        ##### How to use this driver #####
*  ==============================================================================
* 	Set up:
* 	#1: Run @ref PCA9685_Init()
* 	#2: Run @ref PCA9685_SetOscillatorFrequency(), 27000000
* 	#3: Run @ref PCA9685_SetPWMFreq(), 1600
*
* 	Let play: @ref PCA9685_SetPWM()
* 	#1: GPIO
* 	PCA9685_SetPWM(hpca9685, pin, 4096, 0);       // turns pin fully on
* 	delay(100);
*	PCA9685_SetPWM(hpca9685, pin, 0, 4096);       // turns pin fully off
*
*	#2: PWM
*	PCA9685_SetPWM(hpca9685, pin, on, off);
*
*
*
 */
#include "PCA9685Driver.h"
uint32_t _oscillator_freq;

void PCA9685_Init(PCA9685_HandleTypeDef *hpca9685, 
									I2C_HandleTypeDef *hi2c,
									uint8_t addr,
									uint8_t prescale) {
	hpca9685->Address = addr;
	hpca9685->hi2c = hi2c;
	PCA9685_Reset(hpca9685);
	if (prescale) {
		PCA9685_SetExtClk(hpca9685, prescale);
	} else {
		// set a default frequency
		PCA9685_SetPWMFreq(hpca9685, 1000);
	}
	// set the default internal frequency
	PCA9685_SetOscillatorFrequency(hpca9685, FREQUENCY_OSCILLATOR);
}
									
void PCA9685_Reset(PCA9685_HandleTypeDef *hpca9685) {
	PCA9685_Write8(hpca9685, PCA9685_MODE1, MODE1_RESTART);
	HAL_Delay(10);
}

void PCA9685_Sleep(PCA9685_HandleTypeDef *hpca9685) {
	uint8_t awake;
	PCA9685_Read8(hpca9685, PCA9685_MODE1, &awake);
  uint8_t sleep = awake | MODE1_SLEEP; // set sleep bit high
  PCA9685_Write8(hpca9685, PCA9685_MODE1, sleep);
  HAL_Delay(5); // wait until cycle ends for sleep to be active
}

void PCA9685_Wakeup(PCA9685_HandleTypeDef *hpca9685) {
	uint8_t sleep;
	PCA9685_Read8(hpca9685, PCA9685_MODE1, &sleep);
	uint8_t wakeup = sleep & ~MODE1_SLEEP; // set sleep bit low
	PCA9685_Write8(hpca9685, PCA9685_MODE1, wakeup);
}

void PCA9685_SetExtClk(PCA9685_HandleTypeDef *hpca9685, uint8_t prescale) {
	uint8_t oldmode;
	PCA9685_Read8(hpca9685, PCA9685_MODE1, &oldmode);
	uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
	PCA9685_Write8(hpca9685, PCA9685_MODE1, newmode); // go to sleep, turn off internal oscillator
	
	// This sets both the SLEEP and EXTCLK bits of the MODE1 register to switch to
	// use the external clock.
	PCA9685_Write8(hpca9685, PCA9685_MODE1, (newmode |= MODE1_EXTCLK));
	
	PCA9685_Write8(hpca9685, PCA9685_PRESCALE, prescale); // set the prescaler
	
	HAL_Delay(5);
	// clear the SLEEP bit to start
	PCA9685_Write8(hpca9685, PCA9685_MODE1, (newmode & ~MODE1_SLEEP) | MODE1_RESTART | MODE1_AI);
}

void PCA9685_SetPWMFreq(PCA9685_HandleTypeDef *hpca9685, float freq) {
	// Range output modulation frequency is dependant on oscillator
	if (freq < 1) 		freq = 1;
	if (freq > 3500)	freq = 3500; // Datasheet limit is 3052=50MHz/(4*4096)

	float prescaleval = ((_oscillator_freq / (freq * 4096.0)) + 0.5) - 1;
	if (prescaleval < PCA9685_PRESCALE_MIN)
    prescaleval = PCA9685_PRESCALE_MIN;
	if (prescaleval > PCA9685_PRESCALE_MAX)
    prescaleval = PCA9685_PRESCALE_MAX;
	uint8_t prescale = (uint8_t)prescaleval;
	
	uint8_t oldmode;
	PCA9685_Read8(hpca9685, PCA9685_MODE1, &oldmode);
	uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
	PCA9685_Write8(hpca9685, PCA9685_MODE1, newmode);          	// go to sleep
	PCA9685_Write8(hpca9685, PCA9685_PRESCALE, prescale); 		// set the prescaler
	PCA9685_Write8(hpca9685, PCA9685_MODE1, oldmode);
	HAL_Delay(5);
	// This sets the MODE1 register to turn on auto increment.
	PCA9685_Write8(hpca9685, PCA9685_MODE1, oldmode | MODE1_RESTART | MODE1_AI);
}

void PCA9685_SetOscillatorFrequency(PCA9685_HandleTypeDef *hpca9685, uint32_t freq) {
	_oscillator_freq = freq;
}

void PCA9685_SetPWM(PCA9685_HandleTypeDef *hpca9685, uint8_t num, uint16_t on, uint16_t off) {
	uint8_t outputBuffer[5] = {PCA9685_LED0_ON_L + 4*num, on, (on >> 8), off, (off >> 8)};
	HAL_I2C_Master_Transmit(hpca9685->hi2c, hpca9685->Address, outputBuffer, 5, 1);
}

			
/******************* Low level I2C interface */
void PCA9685_Write8(PCA9685_HandleTypeDef *hpca9685, uint8_t addr, uint8_t d) {
	HAL_I2C_Mem_Write(hpca9685->hi2c, hpca9685->Address, addr, 1, &d, 1, 1);
}

void PCA9685_Read8(PCA9685_HandleTypeDef *hpca9685, uint8_t addr, uint8_t *data) {
	HAL_I2C_Mem_Read(hpca9685->hi2c, hpca9685->Address, addr, 1, data, 1, 1);
}

