#include "TCA9548A_Driver.h"


/*
 * @brief Initialize
 * @param htca9548a Pointer to a TCA9548A_HandleTypeDef
 * @param hi2c Pointer to a I2C_HandleTypeDef structure that contains
 * 			     the configuration information for the specified I2C.
 * @param address Target BH1750 device address. The device 7 bits address value in datasheet must
 *                be shifted to the left before calling by I2C function.
 * @retval TCA9548A Status
 */
TCA9548A_STATUS TCA9548A_Init(TCA9548A_HandleTypeDef *htca9548a, I2C_HandleTypeDef *hi2c, uint16_t address) {
    htca9548a->I2C_channel = hi2c;
    htca9548a->Address = address;

    if (TCA9548A_OK == TCA9548A_DisableAllChannel(htca9548a)) {
        return TCA9548A_OK;
    }
    return TCA9548A_ERROR;
}

/*
 * @brief Initialize
 * @param htca9548a Pointer to a TCA9548A_HandleTypeDef
 * @param channel From 0 to 7, is channel 0 to channel 7 on TCA9548A
 * @retval TCA9548A Status
 */
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

/*
 * @brief Initialize
 * @param htca9548a Pointer to a TCA9548A_HandleTypeDef
 * @param channel This have 8 bit to choose which channel is selected.
 *                Ex: channel 0, 3, 5 is selected, channel = 0b00101001 = 0x29 
 * @retval TCA9548A Status
 */
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
