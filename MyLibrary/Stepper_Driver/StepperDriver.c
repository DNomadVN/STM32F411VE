/*
 * StepperDriver.c
 *
 *  Created on: Apr 25, 2021
 *      Author: Ice Cream
 */

#include "StepperDriver.h"

/*
 * @brief Initialize
 * @param stepper Pointer to a Stepper Motor
 * @param Port is GPIOx, x is the port (A ... E) you connect the 
 * 		   direction and pulse pins from the driver to stm32
 * @param microstep specifies the microstep value that in use. (usually 1, 2, 4, 8, 16, 32)
 * @param currentPos specifies the motor's current position 
 * @retval Stepper Status
 */
STEPPER_STATUS StepperInit(Stepper_HandleTypeDef *stepper,
		                   GPIO_TypeDef* port,
						   uint16_t pin_Dir,
						   uint16_t pin_Pulse,
						   uint8_t microstep,
						   uint16_t currentPos) {
	stepper->Port         = port;
	stepper->GPIO_Pin_Dir   = pin_Dir;
	stepper->GPIO_Pin_Pulse = pin_Pulse;
	stepper->Microstep      = microstep;
	stepper->CurrentPos     = currentPos;
	stepper->TargetPos      = currentPos;

	return STEPPER_OK;
}


STEPPER_STATUS setCurrentPos(Stepper_HandleTypeDef *stepper, uint16_t current) {
	stepper->CurrentPos = current;
	return STEPPER_OK;
}

STEPPER_STATUS setTargetPos(Stepper_HandleTypeDef *stepper, uint16_t target) {
	stepper->TargetPos = target;
	return STEPPER_OK;
}

STEPPER_STATUS setDirCCW(Stepper_HandleTypeDef *stepper) {
	HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Dir, GPIO_PIN_SET);
	return STEPPER_OK;
}

STEPPER_STATUS setDirCW(Stepper_HandleTypeDef *stepper) {
	HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Dir, GPIO_PIN_RESET);
	return STEPPER_OK;
}

__weak void delay_us(uint16_t us)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(us);
  /*  TIM2 configure
   *  Clock Source: Internal Clock
   *  Prescaler: 50 - 1
   *  Counter Period: 0xFFFF - 1
   *  Clock Configure: 50MHz
   *  Code:
   *  __HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
   *  while ((uint16_t)__HAL_TIM_GET_COUNTER(&htim2) < us);
   */
}
void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
	while ((uint16_t)__HAL_TIM_GET_COUNTER(&htim2) < us);
}
STEPPER_STATUS runToTarget(Stepper_HandleTypeDef *stepper, uint16_t target) {
	uint32_t current = stepper->CurrentPos;
	if (target != current) {
		// convert Position in millimeters to pulses base on microstep
		uint32_t waitingPul;
		if (target > current) {
			setDirCCW(stepper);
			waitingPul = target - current;
		}
		else {
			setDirCW(stepper);
			waitingPul = current - target;
		}
		waitingPul = waitingPul * 200 * stepper->Microstep / Diameter / PI;

		// output pulse
		while (waitingPul > 0) {
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_SET);
			delay_us(TdelayON);
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_RESET);
			delay_us(TdelayOFF);
			waitingPul--;
		}
		stepper->CurrentPos = target;
	}
	return STEPPER_OK;
}
