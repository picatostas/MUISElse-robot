/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "ring_buf.h"
#include "fsm.h"
#include <string.h>
#include "stm32f411e_discovery_gyroscope.h"
#include "stm32f411e_discovery_accelerometer.h"
#include "gyro.h"
#include "accelero.h"
#include "magneto.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t test[]= "HOLA\r\n";
uint8_t uart_rx[20];

static rb_struct rb_usb,rb_uart;
#define MSG_SIZE 6
#define UART_MSG_SIZE 6
#define MAX_MSG 3


// transition flags


typedef uint8_t flag;


flag usb_flag = 0, uart_flag = 0;
uint32_t usb_count = 0, uart_count = 0, A_count = 0, G_count = 0;

typedef struct{
	flag A;
	flag C;
	flag G;
	flag T;
	flag M;
	flag L;
	flag S;
}flags;
flags cmd,sensor_timer = (flags){0,0,0,0,0,0,0};

typedef struct{
	unsigned char buffer[MAX_MSG][MSG_SIZE];
	uint8_t count;
}Received_msgs;

Received_msgs  msgs;

typedef struct{
	uint32_t   count;
	uint32_t  period;
	flag 	 	 usb;
	flag 		uart;
}Sensor;

Sensor sensorA, sensorG,sensorC,sensorT;// = (Sensor){0,800,0,0};

typedef struct {
		int16_t x;
		int16_t y;
		int16_t z;
}DATA_ACCELERO;
typedef struct {
		float x;
		float y;
		float z;
}DATA_GYRO;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void parseMsg(rb_struct *);

// USB
static int usb_timer(fsm_t* this);
static int usb_parse_finished(fsm_t* this);
static void usb_parse(fsm_t* this);
static int return_true(fsm_t* this){return 1;}

// UART



static int uart_timer(fsm_t* this);
static int uart_parse_finished(fsm_t* this);
static void uart_parse(fsm_t* this);


// PARSER
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);

static int check_command_count(fsm_t * this);
static int cmd_A(fsm_t* this);
static int cmd_C(fsm_t* this);
static int cmd_G(fsm_t* this);
static int cmd_T(fsm_t* this);
static int cmd_M(fsm_t* this);
static int cmd_L(fsm_t* this);
static int cmd_S(fsm_t* this);


static void do_A(fsm_t* this);
static void do_C(fsm_t* this);
static void do_G(fsm_t* this);
static void do_T(fsm_t* this);
static void do_M(fsm_t* this);
static void do_L(fsm_t* this);
static void do_S(fsm_t* this);

static void select_device(fsm_t * this);


// SENSOR
static int timer_A(fsm_t* this);
static int timer_G(fsm_t* this);
static int timer_finished_A(fsm_t* this);
static int timer_finished_G(fsm_t* this);

static void send_A(fsm_t* this);
static void send_G(fsm_t* this);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

enum usb_state{
	USB_IDLE,
	USB_PARSE
};
enum sensor_state{
	SENSOR_IDLE,
	SENSOR_SEND
};
enum uart_state{
	UART_IDLE,
	UART_PARSE
};

enum parser_state{
	PARSER_IDLE,
	PARSER_READFIRST,
	PARSER_A,
	PARSER_C,
	PARSER_G,
	PARSER_T,
	PARSER_M,
	PARSER_L,
	PARSER_S

};
static fsm_trans_t accelero_tt[] = {

  { SENSOR_IDLE, timer_A, SENSOR_SEND,send_A},
  { SENSOR_SEND,timer_finished_A, SENSOR_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};
static fsm_trans_t gyro_tt[] = {

  { SENSOR_IDLE, timer_G, SENSOR_SEND,send_G},
  { SENSOR_SEND,timer_finished_G, SENSOR_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};


static fsm_trans_t usb_tt[] = {

  { USB_IDLE, usb_timer, USB_PARSE,     usb_parse},
  { USB_PARSE,    usb_parse_finished, USB_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};

static fsm_trans_t uart_tt[] = {

  { UART_IDLE, uart_timer, UART_PARSE,     uart_parse },
  { UART_PARSE,     uart_parse_finished, UART_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};

static fsm_trans_t parser_tt[] = {

  { 	PARSER_IDLE, check_command_count, PARSER_READFIRST, select_device},
  { PARSER_READFIRST,cmd_A, PARSER_A,  do_A },
  { PARSER_READFIRST,cmd_C, PARSER_C,  do_C },
  { PARSER_READFIRST,cmd_G, PARSER_G,  do_G },
  { PARSER_READFIRST,cmd_T, PARSER_T,  do_T },
  { PARSER_READFIRST,cmd_M, PARSER_M,  do_M },
  { PARSER_READFIRST,cmd_L, PARSER_L,  do_L },
  { PARSER_READFIRST,cmd_S, PARSER_S,  do_S },
  { 		PARSER_A,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_C,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_G,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_T,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_M,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_L,return_true, PARSER_IDLE,  NULL },
  { 		PARSER_S,return_true, PARSER_IDLE,  NULL },
  {-1, NULL, -1, NULL }
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  fsm_t * usb_fsm = fsm_new(usb_tt);
  fsm_t * uart_fsm = fsm_new(uart_tt);
  fsm_t * parser_fsm = fsm_new(parser_tt);
  fsm_t * accelero_fsm = fsm_new(accelero_tt);
  fsm_t * gyro_fsm = fsm_new(gyro_tt);

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
 // MX_I2C1_Init();
  //MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  sensorA.period = 800;
  sensorG.period = 800;

 // BSP_ACCELERO_Init();
  //BSP_ACCELERO_Click_ITConfig();
  //BSP_GYRO_Init();

  HAL_UART_Receive_IT(&huart2,(uint8_t *)uart_rx,UART_MSG_SIZE);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
  BSP_ACCELERO_Init();
  BSP_GYRO_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

	 fsm_fire(usb_fsm);
	 fsm_fire(uart_fsm);
	 fsm_fire(parser_fsm);
	 fsm_fire(accelero_fsm);
	 fsm_fire(gyro_fsm);
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

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
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 479;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 4799;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 99;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

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
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PC3   ------> I2S2_SD
     PA4   ------> I2S3_WS
     PB10   ------> I2S2_CK
     PB12   ------> I2S2_WS
     PC7   ------> I2S3_MCK
     PC10   ------> I2S3_CK
     PC12   ------> I2S3_SD
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Audio_RST_GPIO_Port, Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DATA_Ready_Pin */
  GPIO_InitStruct.Pin = DATA_Ready_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DATA_Ready_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : INT1_Pin INT2_Pin MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = INT1_Pin|INT2_Pin|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CLK_IN_Pin PB12 */
  GPIO_InitStruct.Pin = CLK_IN_Pin|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_RST_Pin */
  GPIO_InitStruct.Pin = Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Audio_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/// GUARDA
static int usb_timer(fsm_t* this){
	return usb_flag;
}
static int uart_timer(fsm_t* this){
	return uart_flag;
}

static int usb_parse_finished(fsm_t* this){
	return !usb_flag;
}
static int uart_parse_finished(fsm_t* this){
	return !uart_flag;
}

/// SALIDA
static void usb_parse(fsm_t* this){
	parseMsg(&rb_usb);
	usb_flag = 0;
	usb_count = 0;
}
static void uart_parse(fsm_t* this){
	parseMsg(&rb_uart);
	uart_flag = 0;
	uart_count = 0;
}
static int check_command_count(fsm_t * this){

	if (msgs.count > 0) {
		return 1;
	}
	else
	return 0;

}
static int cmd_A(fsm_t* this){

	return cmd.A;
}
static int cmd_C(fsm_t* this){

	return cmd.C;
}
static int cmd_G(fsm_t* this){

	return cmd.G;
}
static int cmd_T(fsm_t* this){

	return cmd.T;
}
static int cmd_M(fsm_t* this){

	return cmd.M;
}
static int cmd_L(fsm_t* this){

	return cmd.L;
}
static int cmd_S(fsm_t* this){

	return cmd.S;
}
static void do_A(fsm_t* this){

	int send_rate = 0;
	unsigned char str[4];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=2;
	strncpy(str,pv,3);
	send_rate = atoi(str);
	send_rate = map(send_rate,0,999,200,999);
	sensorA.period = (uint32_t)send_rate;
	cmd.A = 0;
}
static void do_C(fsm_t* this){
	cmd.C = 0;
}
static void do_G(fsm_t* this){

	int send_rate = 0;
	unsigned char str[4];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=2;
	strncpy(str,pv,3);
	send_rate = atoi(str);
	send_rate = map(send_rate,0,999,200,999);
	sensorG.period = (uint32_t)send_rate;
	cmd.G = 0;
}
static void do_T(fsm_t* this){
	cmd.T = 0;
}
static void do_M(fsm_t* this){
	int pwm_value = 0;
	unsigned char str[4];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=2;
	strncpy(str,pv,3);
	pwm_value = atoi(str);
	pwm_value = map(pwm_value,0,180,50,250);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,pwm_value);
	cmd.M = 0;
}
static void do_L(fsm_t* this){

	int pwm_value = 0;
	unsigned char str[3];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=3;
	strncpy(str,pv,2);
	pwm_value = atoi(str);
	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count][2];
	switch (select_char) {
		case '0':
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,pwm_value);
			break;
		case '1':
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,pwm_value);
			break;
		case '2':
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_3,pwm_value);
			break;
		case '3':
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,pwm_value);
			break;
		default:
			break;
	}
	cmd.L = 0;
}
static void do_S(fsm_t* this){



	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count][2];
	unsigned char send_usb    =(unsigned char) msgs.buffer[msgs.count][3];
	unsigned char send_uart   =(unsigned char) msgs.buffer[msgs.count][4];
	switch (select_char){
		case 'A':
			if (send_usb == '1') sensorA.usb = 1;else sensorA.usb = 0;
			if (send_uart == '1')sensorA.uart= 1;else sensorA.uart = 0;
			break;
		case 'G':
			if (send_usb == '1') sensorG.usb = 1;else sensorG.usb = 0;
			if (send_uart == '1')sensorG.uart= 1;else sensorG.uart = 0;
			break;
		default:
			break;
	}



	cmd.S = 0;
}
static void select_device(fsm_t * this){

	cmd.A = 0;
	cmd.C = 0;
	cmd.G = 0;
	cmd.T = 0;
	cmd.M = 0;
	cmd.L = 0;
	cmd.S = 0;
	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count -1][1];

	switch (select_char){
		case 'A':
			cmd.A = 1;
			break;
		case 'C':
			cmd.C = 1;
			break;
		case 'G':
			cmd.G = 1;
			break;
		case 'T':
			cmd.T = 1;
			break;
		case 'M':
			cmd.M = 1;
			break;
		case 'L':
			cmd.L = 1;
			break;
		case 'S':
			cmd.S = 1;
			break;
		default:
			break;
	}
	msgs.count--;

}

static int timer_A(fsm_t* this){
	return sensor_timer.A;
}
static int timer_G(fsm_t* this){
	return sensor_timer.G;
}
static int timer_finished_A(fsm_t* this){
	return !sensor_timer.A;

}
static int timer_finished_G(fsm_t* this){
	return !sensor_timer.G;

}
static void send_A(fsm_t* this){

	int16_t accelero[3]= {0};

	DATA_ACCELERO data;
 	BSP_ACCELERO_GetXYZ(accelero);
	data.x = accelero[0];
	data.y = accelero[1];
	data.z = accelero[2];
	static unsigned char accelero_buf[] = "A:XXXXXX,YYYYYY,ZZZZZZ\r\n";
	sprintf(accelero_buf,"A:%06d,%06d,%06d\r\n",data.x,data.y,data.z);



	if (sensorA.uart){
		HAL_UART_Transmit(&huart2, (uint8_t *)accelero_buf,sizeof(accelero_buf)-1,10);

	}
	if (sensorA.usb){
		if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, accelero_buf, sizeof(accelero_buf)-1)==USBD_OK){
			if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
			}
		}

	}
	sensorA.count = 0;
	sensor_timer.A = 0;
}
static void send_G(fsm_t* this){

	float gyroscope[3]= {0};

	DATA_GYRO data;

	BSP_GYRO_GetXYZ(gyroscope);

	data.x = gyroscope[0];
	data.y = gyroscope[1];
	data.z = gyroscope[2];
	static unsigned char gyroscope_buf[] = "G:XXXXXXXX,YYYYYYYY,ZZZZZZZZ\r\n";
	sprintf(gyroscope_buf,"G:%08.2f,%08.2f,%08.2f\r\n",data.x,data.y,data.z);

	if (sensorG.uart) {
		HAL_UART_Transmit(&huart2, (uint8_t *)gyroscope_buf,sizeof(gyroscope_buf)-1,10);

	}
	if(sensorG.usb){
		if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, gyroscope_buf, sizeof(gyroscope_buf)-1)==USBD_OK){
			if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
			}
		}
	}
	sensorG.count = 0;
	sensor_timer.G = 0;
}


uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  return (uint16_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
void HAL_SYSTICK_Callback(void){

	if (usb_flag == 0) {
		usb_count++;
		if (usb_count >= 500) {
			usb_flag = 1;
		}
	}
	if (uart_flag == 0) {
		uart_count++;
		if (uart_count >= 500) {
			uart_flag = 1;
		}
	}
	if (sensor_timer.A == 0) {
			sensorA.count++;
			if (sensorA.count >= sensorA.period) {
				sensor_timer.A = 1;
			}

		}

	if (sensor_timer.G == 0) {
			sensorG.count++;
			if (sensorG.count >= sensorG.period) {
				sensor_timer.G = 1;
			}
		}
}

void parseMsg(rb_struct * rb){

	uint16_t size;
	static unsigned char usb_buf[] = "tx0: RA000\r\n",uart_buf[] = "tx0: RA000\r\n";
		size = rb_len(rb);

		if (size > 0) {
			while (size && rb_read(rb, 0) != 'R') {
				rb_pop(rb);
				size--;
			}
		}
		if (size > 5) {

			uint8_t i;
			for (i=0; i<5; i++) {
				msgs.buffer[msgs.count][i] = rb_read(rb,0);
				if(rb == &rb_uart) uart_buf[i+5] = rb_pop(rb);
				if(rb == &rb_usb) usb_buf[i+5] = rb_pop(rb);
			}

			msgs.count++;
			if (msgs.count >= MAX_MSG) {
				msgs.count = MAX_MSG - 1;
			}
			size -= 5;
			while (size && (rb_pop(rb) != '\r')) {
				size--;
			}

			if(rb == &rb_uart){
				if(HAL_UART_Transmit(&huart2,(uint8_t *)uart_buf,sizeof(uart_buf)-1,50) == HAL_OK){
					uart_buf[2]++;
					if (uart_buf[2] > '9'){
						uart_buf[2] = '0';
					}
				}
			}

			if (rb == &rb_usb){
				if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, usb_buf, sizeof(usb_buf)-1)==USBD_OK){
					if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
						usb_buf[2]++;
						if (usb_buf[2] > '9') {
							usb_buf[2] = '0';
							}
					}
				}
			}

		}


}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

	if (huart->Instance == USART2) {
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if (huart->Instance == USART2) {
			uint8_t Len = UART_MSG_SIZE;
			uint8_t * Buf = uart_rx;
			while(Len--){
				rb_write(&rb_uart, *Buf++);
			}
			HAL_UART_Receive_IT(&huart2,(uint8_t *)uart_rx,UART_MSG_SIZE);

		}
}
void LSE_Receive_callback(uint8_t* Buf, uint32_t Len){
	while(Len--){
		rb_write(&rb_usb, *Buf++);
	}

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
