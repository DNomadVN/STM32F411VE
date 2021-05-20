/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "StepperDriver.h"
#include "TCA9548A_Driver.h"
#include "BH1750_Driver.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// Variables for Step
Stepper_HandleTypeDef Step1;
Stepper_HandleTypeDef Step2;
Stepper_HandleTypeDef Step0;

uint8_t data[11];
uint16_t info0, info1, info2;

// Variables for Sensor
TCA9548A_HandleTypeDef i2cHub;
BH1750_HandleTypeDef sensor[4];
uint8_t TCAAddress = (0x70 << 1);
float result[4];
char message[10];
uint8_t ret;

typedef enum {
	IDLE = 0,
	RUN_STEPPER = 1,
	READ_SENSOR = 2
} MODE;

MODE program = IDLE;

uint8_t CurCompleted[] = "\nSet Current Position for 3 Stepper Motor completed!\n";
uint8_t TarCompleted[] = "\nSet Target Position for 3 Stepper Motor completed!\n";
uint8_t RunStepper[] = "\nSteppers are running!\n";
uint8_t Done[] = "\nDone!!!\n";


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Additional functions 
void delay_us(uint16_t us)
{
   __HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
   while ((uint16_t)__HAL_TIM_GET_COUNTER(&htim2) < us);
}

/* =========================================================== */
void decryption(uint8_t *data) {
	info0 = (data[2] - '0') * 100 + (data[3] - '0') * 10 + (data[4] - '0');
	info1 = (data[5] - '0') * 100 + (data[6] - '0') * 10 + (data[7] - '0');
	info2 = (data[8] - '0') * 100 + (data[9] - '0') * 10 + (data[10] - '0');
	
}

// Keep It Safe
void keepMotorSafe() {
	uint32_t X0, X1, X2;
	X0 = Step0.TargetPulse;
	X1 = Step1.TargetPulse;
	X2 = Step2.TargetPulse;
	
	// Step 1
	if (X1 > 359 * FACTOR) 				X1 = 359 * FACTOR;
	if (X1 < (110 * FACTOR)) 			X1 = 110 * FACTOR;
	
	// Step 2
	if (X2 > 462 * FACTOR) 				X2 = 462 * FACTOR;
	if (X2 < 212 * FACTOR) 				X2 = 212 * FACTOR;
	if (X2 < (X1 + 103 * FACTOR)) X2 = X1 + 103 * FACTOR;

	// Step0
	if (X0 > 259 * FACTOR) 				X0 = 259 * FACTOR;
	if (X0 < (8 * FACTOR)) 				X0 = 8 * FACTOR;
	if (X0 > (X1 - 103 * FACTOR)) X0 = X1 - 103 * FACTOR;
	
	Step0.TargetPulse = X0;
	Step1.TargetPulse = X1;
	Step2.TargetPulse = X2;
}

// Transmision Data
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == huart2.Instance) {
		// Convert data to information
		switch (data[0]) {
			case 'S': // Stepper
			{
				switch (data[1]){
					case 'C': // Set current position
						decryption(data);
						// Set both Cur and Tar to prevent motor running after set Current Pos
						setCurrentPos(&Step0, info0);
						setCurrentPos(&Step1, info1);
						setCurrentPos(&Step2, info2);
						HAL_UART_Transmit_IT(&huart2, (uint8_t *)CurCompleted, sizeof(CurCompleted));
						break;
					case 'T': // Set target position
						decryption(data);
						setTargetPos(&Step0, info0);
						setTargetPos(&Step1, info1);
						setTargetPos(&Step2, info2);
						keepMotorSafe();
						HAL_UART_Transmit_IT(&huart2, (uint8_t *)TarCompleted, sizeof(TarCompleted));
						program = RUN_STEPPER;
						break;
				}
				break;
			}
			case 'R': // Read Sensor
			{
				program = READ_SENSOR;
				break;
			}
						
		}
		
		HAL_UART_Receive_IT(&huart2, (uint8_t *)data, 11);
	}
	
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
	
	// Init STEP
	StepperInit(&Step0, GPIOA, GPIO_PIN_9, GPIO_PIN_8, 32, 0);
	StepperInit(&Step1, GPIOC, GPIO_PIN_9, GPIO_PIN_8, 32, 0);
	StepperInit(&Step2, GPIOC, GPIO_PIN_7, GPIO_PIN_6, 32, 0);
	
	// Init UART
	HAL_UART_Receive_IT(&huart2, (uint8_t *)data, 11);
	
	
	// Init Sensor
	TCA9548A_Init(&i2cHub, &hi2c1, TCAAddress);
	
	for (uint8_t i = 0; i < 4; i++) {
		TCA9548A_SelectSingleChannel(&i2cHub, i);
		BH1750_Init(&sensor[i], &hi2c1, BH1750_ADDRESS_LOW);
		BH1750_PowerState(&sensor[i], 1);
		BH1750_SetMode(&sensor[i], CONTINUOUS_H_RES_MODE);
	}



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (program == RUN_STEPPER) {
			runToTarget(&Step0);
			runToTarget(&Step1);
			runToTarget(&Step2);
		} 
		else if (program == READ_SENSOR) {
				for (uint8_t i = 0; i < 4; i++) {
				TCA9548A_SelectSingleChannel(&i2cHub, i);
				BH1750_ReadLight(&sensor[i], &result[i]);
				
				ret = snprintf(message, sizeof(message), "%f", result[i]);
				if (i < 3) message[9] = ' ';
				else message[9] = '\n';
				HAL_UART_Transmit(&huart2, (uint8_t *)message, 10, 10);
				program = IDLE;
			}
		}

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 50-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xffff-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
