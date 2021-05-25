/*
 * StepperDriver.c
 *
 *  Created on: Apr 25, 2021
 *      Author: Ice Cream
 *
 *============================================================
 *      HOW TO USE THIS DRIVER
 *============================================================
 *   #1: Khoi tao dong co bang ham @ref StepperInit()
 *   #2: Dat vi tri ban dau bang ham @ref setCurrentPos()
 *   #3: Dat vi tri muon di toi bang ham @ref setTargetPos()
 *   #4: Them ham @ref runToTarget() vao vong lap de dong co se tu dong chay toi vi
 *       da chon.
 *
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
	stepper->Port           = port;
	stepper->GPIO_Pin_Dir   = pin_Dir;
	stepper->GPIO_Pin_Pulse = pin_Pulse;
	stepper->Microstep      = microstep;
	stepper->CurrentPulse   = currentPos * FACTOR;
	stepper->TargetPulse    = currentPos * FACTOR;

	return STEPPER_OK;
}


STEPPER_STATUS setCurrentPos(Stepper_HandleTypeDef *stepper, uint16_t current) {
	stepper->CurrentPulse = current * FACTOR;
	stepper->TargetPulse = current * FACTOR;
	return STEPPER_OK;
}

STEPPER_STATUS setTargetPos(Stepper_HandleTypeDef *stepper, uint16_t target) {
	stepper->TargetPulse = target * FACTOR;
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
	 *  don't forget to add this below USER CODE BEGIN 2
   *  HAL_TIM_Base_Start(&htim2);
   *  Code:
   *  __HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
   *  while ((uint16_t)__HAL_TIM_GET_COUNTER(&htim2) < us);
   */
}

STEPPER_STATUS runToTarget(Stepper_HandleTypeDef *stepper) {

	if (stepper->TargetPulse != stepper->CurrentPulse) {
		// convert Position in millimeters to pulses base on microstep
		
		if (stepper->TargetPulse > stepper->CurrentPulse) {
			setDirCCW(stepper);
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_SET);
			delay_us(TdelayON);
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_RESET);
			delay_us(TdelayOFF);
			stepper->CurrentPulse++;
		}
		else {
			setDirCW(stepper);
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_SET);
			delay_us(TdelayON);
			HAL_GPIO_WritePin(stepper->Port, stepper->GPIO_Pin_Pulse, GPIO_PIN_RESET);
			delay_us(TdelayOFF);
			stepper->CurrentPulse--;
		}
		return STEPPER_ERROR;
	}
	return STEPPER_OK;
}
