/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "commandTask.h"
#include "prodManTask.h"
#include "oled.h"
#include "string.h"
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define UART_BUF_SIZE 256
#define BIG_UART_BUF_SIZE 128
#define MAX_COMMAND_SIZE 20
#define MAX_DATA_SIZE 128
#define MAX_PROD_MSG_SIZE 8

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for prodManTask */
osThreadId_t prodManTaskHandle;
const osThreadAttr_t prodManTask_attributes = {
  .name = "prodManTask",
  .stack_size = 3000 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for commandTask */
osThreadId_t commandTaskHandle;
const osThreadAttr_t commandTask_attributes = {
  .name = "commandTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for cmdToProdManQ */
osMessageQueueId_t cmdToProdManQHandle;
const osMessageQueueAttr_t cmdToProdManQ_attributes = {
  .name = "cmdToProdManQ"
};
/* Definitions for prodManToOledQ */
osMessageQueueId_t prodManToOledQHandle;
const osMessageQueueAttr_t prodManToOledQ_attributes = {
  .name = "prodManToOledQ"
};
/* USER CODE BEGIN PV */

char uart_msg[UART_BUF_SIZE];
static uint8_t uart1_rxBuf[UART_BUF_SIZE];
static uint8_t uart2_rxBuf[UART_BUF_SIZE];
static volatile uint8_t uart_receive_char;

uint8_t data[MAX_PROD_MSG_SIZE];
int data_pos = 0;
uint16_t prod_values[256] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
void StartDisplayTask(void *argument);
void StartProdManTask(void *argument);
void StartCommandTask(void *argument);

/* USER CODE BEGIN PFP */

void processProdDataByte(uint8_t rxChar, uint8_t *data, int *data_pos, uint16_t *prod_values);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of cmdToProdManQ */
  cmdToProdManQHandle = osMessageQueueNew (4, sizeof(cmd_struct), &cmdToProdManQ_attributes);

  /* creation of prodManToOledQ */
  prodManToOledQHandle = osMessageQueueNew (1, sizeof(oled_struct), &prodManToOledQ_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of displayTask */
  displayTaskHandle = osThreadNew(StartDisplayTask, NULL, &displayTask_attributes);

  /* creation of prodManTask */
  prodManTaskHandle = osThreadNew(StartProdManTask, NULL, &prodManTask_attributes);

  /* creation of commandTask */
  commandTaskHandle = osThreadNew(StartCommandTask, NULL, &commandTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart-> Instance == USART1)
	{
		processProdDataByte(uart_receive_char, data, &data_pos, prod_values);

		/*sprintf(uart_msg, "in callback");
		HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);*/

		HAL_UART_Receive_IT(&huart1,(uint8_t*) &uart_receive_char, 1);
	}
}

uint32_t getTotalProdValue(uint16_t *prod_values){
	uint32_t totalProdValue = 0;

	for (int i = 0; i < 256; i++) {
		totalProdValue += (uint32_t) (prod_values[i]);
	}

	return totalProdValue;
}

void processProdDataByte(uint8_t rxChar, uint8_t *data, int *data_pos, uint16_t *prod_values){
	oled_struct oled_stats = {0};

	//sprintf(uart_msg, "nb");
	//HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

	if(*data_pos >= MAX_PROD_MSG_SIZE){
		//error, should never reach here as every msg size must be <= MAX_PROD_DATA_SIZE

		sprintf(uart_msg, "\nmsg long\n\n");
		HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

		*data_pos = 0;
		return;
	}
	else{
		data[*data_pos] = rxChar;
	}

	if(data[0] == MSG_PROD){
		if(*data_pos >= 3){
			//end of producer task msg reached
			prod_values[data[1]] = ((uint16_t)(data[2]) << 8) | (((uint16_t)data[3])&0xFF);

			*data_pos = 0;
		}
		else{
			*data_pos = *data_pos + 1;
		}
	}
	else if(data[0] < MSG_INVALID){
		if(*data_pos == 7){
			//end of info message reached

			if(data[0] == MSG_INFO_ALL){
				oled_stats.is_single_view = 0;
				oled_stats.b = getTotalProdValue(prod_values);
			}
			else if(data[0] == MSG_INFO_ONE){
				oled_stats.is_single_view = 1;
				oled_stats.b = prod_values[data[1]];
			}

			oled_stats.a = data[1];
			oled_stats.d =	(((uint32_t)data[2]) << 8)  |
							((uint32_t)data[3]);
			oled_stats.c =	(((uint32_t)data[4]) << 24) |
							(((uint32_t)data[5]) << 16) |
							(((uint32_t)data[6]) << 8)  |
							((uint32_t)data[7]);

			xQueueOverwrite(prodManToOledQHandle, &oled_stats);

			sprintf(uart_msg, "is single?: %u, a: %u, b: %lu, c: %lu, d: %u\n", oled_stats.is_single_view, oled_stats.a, oled_stats.b, oled_stats.c, oled_stats.d);
			HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

			*data_pos = 0;
		}
		else{
			*data_pos = *data_pos + 1;
		}
	}
	else{
		//error, the msg_id sent by the producer MCU is wrong

		sprintf(uart_msg, "\nwrong msg_id\n\n");
		HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

		*data_pos = 0;
	}
}

void processCmdByte(char rxChar, char *command, int *com_pos){
    char uart_msg_cmd[UART_BUF_SIZE];

    cmd_struct parsed_cmd = {0};
    parsed_cmd.UART_command_ID = CMD_INVALID;

    if(*com_pos < MAX_COMMAND_SIZE){
        command[*com_pos] = rxChar;

        //found end of a command, parse it and change the delay value of the corresponding task
        if(command[*com_pos] == '\0'){

        	unsigned int value = 0;

            char *arg = strtok(command, " ");

            if(strcmp(arg, "delete") == 0){
            	arg = strtok(NULL, " ");
				if(sscanf(arg, "%u", &value) == 1){
					parsed_cmd.UART_command_ID = CMD_DELETE;
					parsed_cmd.thread_ID = value;
				}
            }
            else if(strcmp(arg, "create") == 0){
            	arg = strtok(NULL, " ");
				if(sscanf(arg, "%u", &value) == 1){
					parsed_cmd.UART_command_ID = CMD_CREATE;
					parsed_cmd.delay = value;
				}
            }
            else if(strcmp(arg, "show") == 0){
            	arg = strtok(NULL, " ");
            	if(strcmp(arg, "all") == 0){
            		parsed_cmd.UART_command_ID = CMD_SHOW_ALL;
            	}
            	else if(sscanf(arg, "%u", &value) == 1){
					parsed_cmd.UART_command_ID = CMD_SHOW_ONE;
					parsed_cmd.thread_ID = value;
            	}
            }

            if(parsed_cmd.UART_command_ID == CMD_INVALID){
            	sprintf(uart_msg_cmd, "Invalid command!\n");
            	HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg_cmd, strlen(uart_msg_cmd), portMAX_DELAY);
            }
            else if(xQueueSend(cmdToProdManQHandle, &parsed_cmd, 0) != pdPASS){
            	sprintf(uart_msg_cmd, "Queue full, lost cmd: %u, %u, %u\n", parsed_cmd.UART_command_ID, parsed_cmd.thread_ID, parsed_cmd.delay);
            	HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg_cmd, strlen(uart_msg_cmd), portMAX_DELAY);
            }
            else{
            	sprintf(uart_msg_cmd, "Sent cmd %u, task ID %u, delay %u\n", parsed_cmd.UART_command_ID, parsed_cmd.thread_ID, parsed_cmd.delay);
            	HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg_cmd, strlen(uart_msg_cmd), portMAX_DELAY);
            }

            *com_pos = 0;
        }
        else{
            (*com_pos)++;
        }
    }
}

void decodeCommand(cmd_struct cmd, uint8_t *decoded_cmd){
	decoded_cmd[0] = cmd.UART_command_ID;
	decoded_cmd[1] = cmd.thread_ID;
	decoded_cmd[2] = (uint8_t) (cmd.delay >> 8);
	decoded_cmd[3] = (uint8_t) cmd.delay;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDisplayTask */
/**
  * @brief  Function implementing the displayTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN 5 */

	oled_struct old_stats;
	oled_struct cur_stats;

	old_stats.is_single_view = 0;

	setup_oled(&hspi1, &huart2);
	setup_screen_text(0); //do this whenever view changes

  /* Infinite loop */
  for(;;)
  {
	  if(xQueueReceive(prodManToOledQHandle, &cur_stats, 0) == pdPASS){
		  if(old_stats.is_single_view != cur_stats.is_single_view){
			  setup_screen_text(cur_stats.is_single_view);
			  old_stats.is_single_view = cur_stats.is_single_view;
		  }
		  output_to_oled(cur_stats.a, cur_stats.b, cur_stats.c, cur_stats.d);
	  }

	  osDelay(200);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartProdManTask */
/**
* @brief Function implementing the prodManTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProdManTask */
void StartProdManTask(void *argument)
{
  /* USER CODE BEGIN StartProdManTask */

	cmd_struct cmd;
	uint8_t cmd_raw[4];

	/*uint8_t data[MAX_PROD_MSG_SIZE];
	int data_pos = 0;*/

	uint16_t old_pos = 0;
	uint16_t pos = 0;

	if(HAL_UART_Receive_DMA(&huart1, uart1_rxBuf, UART_BUF_SIZE) != HAL_OK){
		sprintf(uart_msg, "dma fail");
		HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);
	}
	else{
		sprintf(uart_msg, "dma started");
		HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);
	}

	 /*if(HAL_UART_Receive_IT(&huart1,(uint8_t*) &uart_receive_char, 1) == HAL_OK){
		 sprintf(uart_msg, "uart it started");
		 HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);
	 }
	 else{
		 sprintf(uart_msg, "uart it failed to start");
		 HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);
	 }*/

  /* Infinite loop */
  for(;;)
  {
	  pos = UART_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);

	  /*if(HAL_UART_Receive(&huart1, uart1_rxBuf, 8, 100) == HAL_OK){
		  sprintf(uart_msg, "msg: %u,%u,%u,%u,%u,%u,%u,%u\n", uart1_rxBuf[0], uart1_rxBuf[1], uart1_rxBuf[2], uart1_rxBuf[3], uart1_rxBuf[4], uart1_rxBuf[5], uart1_rxBuf[6], uart1_rxBuf[7]);
		  HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);
	  }*/

	  //sprintf(uart_msg, "p: %u, op: %u ", pos, old_pos);
	  //HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

	  //receive data from production MCU
	  if ((pos != old_pos) && (pos < UART_BUF_SIZE))
	  {
		  if (pos > old_pos)
		  {
			  //normal case
			  for (uint16_t i = old_pos; i < pos; i++)
			  {
				  processProdDataByte(uart1_rxBuf[i], data, &data_pos, prod_values);
			  }
		  }
		  else
		  {
			  sprintf(uart_msg, "pos: %u", pos);
			  HAL_UART_Transmit(&huart2, (uint8_t *)uart_msg, strlen(uart_msg), portMAX_DELAY);

			  //wrapped around
			  for (uint16_t i = old_pos; i < UART_BUF_SIZE; i++)
			  {
				  processProdDataByte(uart1_rxBuf[i], data, &data_pos, prod_values);
			  }
			  for (uint16_t i = 0; i < pos; i++)
			  {
				  processProdDataByte(uart1_rxBuf[i], data, &data_pos, prod_values);
			  }
		  }

		  old_pos = pos;
	  }


	  //forward commands from command task to other production MCU
	  while(xQueueReceive(cmdToProdManQHandle, &cmd, 0) == pdPASS){
		  decodeCommand(cmd, cmd_raw);
		  HAL_UART_Transmit(&huart1, cmd_raw, 4, 100);
	  }

	  osDelay(1);

  }
  /* USER CODE END StartProdManTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
* @brief Function implementing the commandTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(void *argument)
{
  /* USER CODE BEGIN StartCommandTask */

	//char uart_msg[UART_BUF_SIZE];
	char command[MAX_COMMAND_SIZE];
	int com_pos = 0;

	uint16_t old_pos = 0;
	uint16_t pos = 0;

	HAL_UART_Receive_DMA(&huart2, uart2_rxBuf, UART_BUF_SIZE);

  /* Infinite loop */
  for(;;)
  {

	  pos = UART_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

	  //this entire thing checks if new data has been received via UART and processes the data
	  if (pos != old_pos)
	  {
		  if (pos > old_pos)
		  {
			  //normal case
			  for (uint16_t i = old_pos; i < pos; i++)
			  {
				  processCmdByte(uart2_rxBuf[i], command, &com_pos);
			  }
		  }
		  else
		  {
			  //wrapped around
			  for (uint16_t i = old_pos; i < UART_BUF_SIZE; i++)
			  {
				  processCmdByte(uart2_rxBuf[i], command, &com_pos);
			  }
			  for (uint16_t i = 0; i < pos; i++)
			  {
				  processCmdByte(uart2_rxBuf[i], command, &com_pos);
			  }
		  }

		  old_pos = pos;
	  }

    osDelay(100);
  }
  /* USER CODE END StartCommandTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
