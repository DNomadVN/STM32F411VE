/*
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
	Set up:
  	#1	@ref BH1750_Init()
	 	Ex: BH1750_Init(&sensor[i], &hi2c1, BH1750_ADDRESS_LOW);
	#2	@ref BH1750_PowerState()
		Ex: BH1750_PowerState(&sensor[i], 1);
	#3	@ref BH1750_SetMode()
		Ex: BH1750_SetMode(&sensor[i], CONTINUOUS_H_RES_MODE);
		
	Let play: Read light @ref BH1750_ReadLight()
	BH1750_ReadLight(&sensor[i], &result[i]);
	
	
	
	*/


#include "BH1750_Driver.h"


bh1750_mode		BH1750_Mode;
uint8_t         BH1750_Mtreg;

/*
 * @brief Initialize
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @param hi2c Pointer to a I2C_HandleTypeDef structure that contains
 * 			     the configuration information for the specified I2C.
 * @param Address Target BH1750 device address. The device 7 bits address value in datasheet must
 *                be shifted to the left before calling by I2C function.
 * @retval BH1750 Status
 */
BH1750_STATUS BH1750_Init(BH1750_HandleTypeDef *hbh1750, I2C_HandleTypeDef *hi2c, uint16_t Address) {
    hbh1750->I2C_channel = hi2c;
	hbh1750->Address = Address;
	if(BH1750_OK == BH1750_Reset(hbh1750))
	{
		if(BH1750_OK == BH1750_SetMtreg(hbh1750, BH1750_DEFAULT_MTREG)) // Set default value;
			return BH1750_OK;
	}
	return BH1750_ERROR;
}

/* 
 * @brief Reset all registers to default value.
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @retval BH1750 status
 */
BH1750_STATUS BH1750_Reset(BH1750_HandleTypeDef *hbh1750) {
    uint8_t tmp = BH1750_RESET;
    if (HAL_OK == HAL_I2C_Master_Transmit(hbh1750->I2C_channel, hbh1750->Address, &tmp, 1, 10))
        return BH1750_OK;
    return BH1750_ERROR;
}

/*
 * @brief Set the power state.
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @param PowerOn
 *          @arg 0: Power down, low current, no active state.
 *          @arg 1: Ready for measurement command. 
 * @retval BH1750 status
 */
BH1750_STATUS BH1750_PowerState(BH1750_HandleTypeDef *hbh1750, uint8_t PowerOn) {
    PowerOn = (PowerOn ? 1 : 0);
    if (HAL_OK == HAL_I2C_Master_Transmit(hbh1750->I2C_channel, hbh1750->Address, &PowerOn, 1, 10))
        return BH1750_OK;
    return BH1750_ERROR;
}

/*
 * @brief Adjust measurement result for influence of optical window. (sensor sensitivity adjusting) 
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @param Mtreg The modified value of measurement time register. (31 <= Mtreg <=254) (miliseconds)
 * @retval BH1750 status
 */
BH1750_STATUS BH1750_SetMtreg(BH1750_HandleTypeDef *hbh1750, uint8_t Mtreg) {
    HAL_StatusTypeDef retCode;
	if (Mtreg < 31 || Mtreg > 254) {
		return BH1750_ERROR;
	}

	BH1750_Mtreg = Mtreg;

	uint8_t tmp[2];

	tmp[0] = (0x40 | (Mtreg >> 5));     // High bit 01000_MT[7,6,5]
	tmp[1] = (0x60 | (Mtreg & 0x1F));   // Low bit  011_MT[4,3,2,1,0] 

	retCode = HAL_I2C_Master_Transmit(hbh1750->I2C_channel, hbh1750->Address, &tmp[0], 1, 10);
	if (HAL_OK != retCode) {
		return BH1750_ERROR;
	}

	retCode = HAL_I2C_Master_Transmit(hbh1750->I2C_channel, hbh1750->Address, &tmp[1], 1, 10);
	if (HAL_OK == retCode) {
		return BH1750_OK;
	}

	return BH1750_ERROR;
}

/*
 * @brief Set the mode of converting. Look into bh1750_mode enum.
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @param Mode The mode of converting. This parameter can be one of the bh1750_mode enum values:
 *              @arg CONTINUOUS_H_RES_MODE:  Start measurement at 1lx resolution.
 *                                           Measurement Time is typically 120ms. 
 *              @arg CONTINUOUS_H_RES_MODE2: Start measurement at 0.5lx resolution.
 *                                           Measurement Time is typically 120ms. 
 *              @arg CONTINUOUS_L_RES_MODE:  Start measurement at 4lx resolution. 
 *                                           Measurement Time is typically 16ms. 
 *              @arg ONETIME_H_RES_MODE:     Start measurement at 1lx resolution.
 *                                           Measurement Time is typically 120ms. 
 *                                           It is automatically set to Power Down mode after measurement.
 *              @arg ONETIME_H_RES_MODE2:    Start measurement at 0.5lx resolution.
 *                                           Measurement Time is typically 120ms. 
 *                                           It is automatically set to Power Down mode after measurement.
 *              @arg ONETIME_L_RES_MODE:     Start measurement at 4lx resolution.
 *                                           Measurement Time is typically 16ms. 
 *                                           It is automatically set to Power Down mode after measurement.
 * @retval BH1750 status
 */
BH1750_STATUS BH1750_SetMode(BH1750_HandleTypeDef *hbh1750, bh1750_mode Mode) {
    if(!((Mode >> 4) || (Mode >> 5))) return BH1750_ERROR;
	if((Mode & 0x0F) > 3) return BH1750_ERROR;

	BH1750_Mode = Mode;
	if(HAL_OK == HAL_I2C_Master_Transmit(hbh1750->I2C_channel, hbh1750->Address, &Mode, 1, 10))
		return BH1750_OK;

	return BH1750_ERROR;
}

// BH1750_STATUS BH1750_TriggerManualConversion(uint16_t Address);

/*
 * @brief Read the converted value and calculate the result.
 * @param hbh1750 Pointer to a BH1750_HandleTypeDef
 * @param Result Pointer to your variable for getting result.
 * @retval BH1750 Status
 */
BH1750_STATUS BH1750_ReadLight(BH1750_HandleTypeDef *hbh1750, float *Result) {
 	float result;
	uint8_t tmp[2];

	if(HAL_OK == HAL_I2C_Master_Receive(hbh1750->I2C_channel, hbh1750->Address, tmp, 2, 10))
	{
		result = (tmp[0] << 8) | (tmp[1]);

		if(BH1750_Mtreg != BH1750_DEFAULT_MTREG)
		{
			result *= (float)((uint8_t)(BH1750_DEFAULT_MTREG) / (float)BH1750_Mtreg);
		}

		if(BH1750_Mode == ONETIME_H_RES_MODE2 || BH1750_Mode == CONTINUOUS_H_RES_MODE2)
		{
			result /= 2.0;
		}

		*Result = result / (float)BH1750_CONVERSION_FACTOR;
		return BH1750_OK;
	}
	return BH1750_ERROR;
}
