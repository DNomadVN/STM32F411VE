#include "TCA9548A_Driver.h"

TCA9548A_STATUS TCA9548A_Init(TCA9548A_HandleTypeDef *htca9548a, I2C_HandleTypeDef *hi2c, uint16_t address) {
    htca9548a->I2C_channel = hi2c;
    htca9548a->Address = address;

    if (TCA9548A_OK == TCA9548A_DisableAllChannel(htca9548a)) {
        return TCA9548A_OK;
    }
    return TCA9548A_ERROR;
}


TCA9548A_STATUS TCA9548A_SelectSingleChannel(TCA9548A_HandleTypeDef *htca9548a, uint8_t channel) {
    if (channel > 7) {
        return TCA9548A_ERROR;
    }
    uint8_t tmp = (1 << channel);
    if (HAL_OK == HAL_I2C_Master_Transmit(htca9548a->I2C_channel, htca9548a->Address, &tmp, 1, 10)) {
        return TCA9548A_OK;
    }
    return TCA9548A_ERROR;
}


TCA9548A_STATUS TCA9548A_SelectMultiChannel(TCA9548A_HandleTypeDef *htca9548a, uint8_t channel) {
    if (HAL_OK == HAL_I2C_Master_Transmit(htca9548a->I2C_channel, htca9548a->Address, &channel, 1, 10)) {
        return TCA9548A_OK;
    }
    return TCA9548A_ERROR;
}


uint8_t TCA9548A_ReadStatus(TCA9548A_HandleTypeDef *htca9548a) {
    uint8_t tmp;
    if (HAL_OK == HAL_I2C_Master_Receive(htca9548a->I2C_channel, htca9548a->Address, &tmp, 1, 10)) {
        return tmp;
    }
    return 0;
}


TCA9548A_STATUS TCA9548A_DisableAllChannel(TCA9548A_HandleTypeDef *htca9548a) {
    uint8_t tmp = 0x00;
    if (HAL_OK == HAL_I2C_Master_Transmit(htca9548a->I2C_channel, htca9548a->Address, &tmp, 1, 10)) {
        return TCA9548A_OK;
    }
    return TCA9548A_ERROR;
}
