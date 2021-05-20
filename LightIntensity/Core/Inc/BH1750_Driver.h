#ifndef BH1750_H_
#define BH1750_H_

/* Include ------------ */
#include "stm32f4xx_hal.h"


#define BH1750_ADDRESS_HIGH        (0x5C<<1)
#define BH1750_ADDRESS_LOW         (0x23<<1)

#define BH1750_POWER_DOWN          0x00
#define BH1750_POWER_ON            0x01
#define BH1750_RESET               0x07

#define BH1750_DEFAULT_MTREG       69
#define BH1750_CONVERSION_FACTOR   1.2

typedef enum {
    BH1750_OK    = 0,
    BH1750_ERROR = 1
} BH1750_STATUS;

typedef enum {
    CONTINUOUS_H_RES_MODE    = 0x10,
    CONTINUOUS_H_RES_MODE2   = 0x11,
    CONTINUOUS_L_RES_MODE    = 0x13,
    ONETIME_H_RES_MODE       = 0x20,
    ONETIME_H_RES_MODE2      = 0x21,
    ONETIME_L_RES_MODE       = 0x23
} bh1750_mode;

typedef struct BH1750_Driver
{
    I2C_HandleTypeDef *I2C_channel;
    uint16_t Address;

} BH1750_HandleTypeDef;


BH1750_STATUS BH1750_Init(BH1750_HandleTypeDef *hbh1750, I2C_HandleTypeDef *hi2c, uint16_t Address);
BH1750_STATUS BH1750_Reset(BH1750_HandleTypeDef *hbh1750);
BH1750_STATUS BH1750_PowerState(BH1750_HandleTypeDef *hbh1750, uint8_t PowerOn);
BH1750_STATUS BH1750_SetMtreg(BH1750_HandleTypeDef *hbh1750, uint8_t Mtreg);
BH1750_STATUS BH1750_SetMode(BH1750_HandleTypeDef *hbh1750, bh1750_mode Mode);
BH1750_STATUS BH1750_TriggerManualConversion(BH1750_HandleTypeDef *hbh1750);
BH1750_STATUS BH1750_ReadLight(BH1750_HandleTypeDef *hbh1750, float *Result);

#endif /* BH1750_H_ */
