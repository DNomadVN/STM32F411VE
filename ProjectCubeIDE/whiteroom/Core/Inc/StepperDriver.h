#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#include "stm32f4xx_hal.h"

// Time delay write HIGH and LOW to PIN PULSE in microsecond
#define TdelayON    50
#define TdelayOFF  300

// diameter of pulley (millimeters)
#define DIAMETER 13.2
#define PI 		  3.14159

// Chuyen muc fake so
// PulTranfer = 200 * stepper->Microstep / Diameter / PI    = 160
#define FACTOR 160

typedef enum {
	STEPPER_OK 	  = 0,
	STEPPER_ERROR = 1
} STEPPER_STATUS;

typedef struct StepperDriver {
	GPIO_TypeDef* Port;
	uint16_t GPIO_Pin_Dir;
	uint16_t GPIO_Pin_Pulse;
	// uint16_t GPIO_Pin_Enable;
	uint8_t Microstep;
	uint32_t CurrentPulse;
	uint32_t TargetPulse;
} Stepper_HandleTypeDef;

STEPPER_STATUS StepperInit(Stepper_HandleTypeDef *stepper,
		                   GPIO_TypeDef* port,
						   uint16_t pin_Dir,
						   uint16_t pin_Pulse,
						   uint8_t microstep,
						   uint16_t currentPos);
STEPPER_STATUS setCurrentPos(Stepper_HandleTypeDef *stepper, uint16_t current);
STEPPER_STATUS setTargetPos(Stepper_HandleTypeDef *stepper, uint16_t target);
STEPPER_STATUS setDirCCW(Stepper_HandleTypeDef *stepper);
STEPPER_STATUS setDirCW(Stepper_HandleTypeDef *stepper);
STEPPER_STATUS runToTarget(Stepper_HandleTypeDef *stepper);

#endif /* STEPPERDRIVER_H_ */
