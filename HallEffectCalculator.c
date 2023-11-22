//Block of Code That Calculates The RPM and Vehicle Speed of an Electric Vehicle (Noise-proof)
//with a HALL Effect and a Nucleo-F446RE.
//You have to initialize two timers as a Counter and Timeout Timer. As a Counter Timer i use htim2
//that has 2^32 -1 Max Period. with a Prescaler of 1280 -1 (And the HCLK == 128). That way my Timer
//clicks every 10us
//The other Timer is such that it produces a Timeout when RPM are stable for a period of time. Im
//both Timers , Enable Interrupts and preload while setting Trigger Evet Selection to "Update Event"
//The Pins that I have connected the Hall Effects sensors are both internally and externally Pulled-Up
//and are registered as external Interrupts with falling edge trigger.

#include "stm32f4xx_hal.h"
include "main.h"


TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);

//Structure For the Hall Effect Data
struct hallEffectSensor{
	TIM_HandleTypeDef* counter; // 32-bit Timer
	TIM_HandleTypeDef* timeOutTimer; //Generic Timer
	uint32_t time_now;
	uint32_t time_before;
	uint16_t rpm; //measured in rpms
	float vehicle_speed;//measured in m/min
	float radius;//measured in m
	float tuner;//Variable to tune the rpms. Set Default to 1
	uint8_t isReady;//Flag that is updated when the signal of the hall Effect Sensor is pulled Down

};

//Main Function
void hf_calculator(struct hallEffectSensor hf){
	  if(hf.isReady==1){
		  hf.isReady=0;
		  hf.time_now = __HAL_TIM_GET_COUNTER(hf.counter);
		  //hf.rpm = pi*100000/(hf.time_now-hf.time_before); // Enable this if HclkFrequency/TimerPreScaler = 100000
		  hf.rpm = pi*(HAL_RCC_GetHCLKFreq()/(hf.counter->Init.Prescaler+1))/(hf.time_now-hf.time_before);//Generic but slower

		  hf.vehicle_speed = 2*pi*hf.radius*hf.rpm;
		  hf.rpm = gear_ratio*(rpm_conv*hf.rpm)*hf.tuner;
		  hf.time_before = hf.time_now;
		  hf.timeOutTimer->Instance->CNT = 0;
	  }
	  return;
}
//Some Variables and the required Functions
struct hallEffectSensor hf1 = {.isReady = 0, .radius = 0.12, .counter = &htim2, .timeOutTimer = &htim6, .tuner = 1.01};
struct hallEffectSensor hf2 = {.isReady = 0, .radius = 0.12, .counter = &htim2, .timeOutTimer = &htim7, .tuner = 1.01};
float gear_ratio = 6.3;
float rpm_conv = 9.5492;
float pi = 3.141592;
HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==GPIO_PIN_6) hf1.isReady = 1;
	else hf2.isReady = 1;
	return;

}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	if(htim==&htim6) hf1.rpm = 0;
	else if(htim==&htim7) hf2.rpm = 0;
}


int main(void){
  //Functions that are generated from Stm32CubeMx
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  //Functions that Enables our Timers
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);
	while(1){
		hf_calculator(hf1);
	  	hf_calculator(hf2);
	}

	return 0;
}
void SystemClock_Config(void){
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 128;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK){
    Error_Handler();
  }
}
static void MX_TIM2_Init(void){

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1280-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void){
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 64000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 6000-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK){
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK){
    Error_Handler();
  }
}
static void MX_TIM7_Init(void){
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 64000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 6000-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK){
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}
void Error_Handler(void)
{
  __disable_irq();
  while (1);
}
