#ifndef TCA9548_H_
#define TCA9548_H_

#include "stm32f4xx_hal.h"

#define TCA_ADDRESS (0x70 << 1)

typedef enum {
    TCA9548A_OK = 0,
    TCA9548A_ERROR = 1
} TCA9548A_STATUS;

typedef struct TCA9548_Driver {
    I2C_HandleTypeDef *I2C_channel;
    uint8_t Address;
} TCA9548A_HandleTypeDef;

TCA9548A_STATUS TCA9548A_Init(TCA9548A_HandleTypeDef *htca9548a, I2C_HandleTypeDef *hi2c, uint16_t address);
TCA9548A_STATUS TCA9548A_SelectSingleChannel(TCA9548A_HandleTypeDef *htca9548a, uint8_t channel);
TCA9548A_STATUS TCA9548A_SelectMultiChannel(TCA9548A_HandleTypeDef *htca9548a, uint8_t channel);
uint8_t TCA9548A_ReadStatus(TCA9548A_HandleTypeDef *htca9548a);
TCA9548A_STATUS TCA9548A_DisableAllChannel(TCA9548A_HandleTypeDef *htca9548a);

#endif
