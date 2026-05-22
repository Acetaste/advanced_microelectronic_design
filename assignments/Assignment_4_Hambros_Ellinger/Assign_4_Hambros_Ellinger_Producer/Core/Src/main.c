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
#include "monitor_task.h"
#include "producer_task.h"
#include "UART_task.h"
#include "queue.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_RX_BUF_SIZE 50
#define UART_TX_BUF_SIZE 50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* Definitions for MonitorTask */
osThreadId_t MonitorTaskHandle;
const osThreadAttr_t MonitorTask_attributes = {
  .name = "MonitorTask",
  .stack_size = 1200 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for UARTTask */
osThreadId_t UARTTaskHandle;
const osThreadAttr_t UARTTask_attributes = {
  .name = "UARTTask",
  .stack_size = 600 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for producerQueue */
osMessageQueueId_t producerQueueHandle;
const osMessageQueueAttr_t producerQueue_attributes = {
  .name = "producerQueue"
};
/* Definitions for monitorQueue */
osMessageQueueId_t monitorQueueHandle;
const osMessageQueueAttr_t monitorQueue_attributes = {
  .name = "monitorQueue"
};
/* Definitions for UARTcommandQueue */
osMessageQueueId_t UARTcommandQueueHandle;
const osMessageQueueAttr_t UARTcommandQueue_attributes = {
  .name = "UARTcommandQueue"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
void StartMonitorTask(void *argument);
void StartUARTTask(void *argument);

/* USER CODE BEGIN PFP */

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
  /* creation of producerQueue */
  producerQueueHandle = osMessageQueueNew (32, sizeof(struct producer_msg), &producerQueue_attributes);

  /* creation of monitorQueue */
  monitorQueueHandle = osMessageQueueNew (8, sizeof(struct monitor_msg), &monitorQueue_attributes);

  /* creation of UARTcommandQueue */
  UARTcommandQueueHandle = osMessageQueueNew (8, sizeof(struct UART_command), &UARTcommandQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MonitorTask */
  MonitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &MonitorTask_attributes);

  /* creation of UARTTask */
  UARTTaskHandle = osThreadNew(StartUARTTask, NULL, &UARTTask_attributes);

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
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMonitorTask */
/**
  * @brief  Function implementing the MonitorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	struct 	monitor_msg 		msg_struct 				= {ALL_TASKS,0,0,0};
	struct 	UART_command 		command_struct			= {0,0,0};
	struct 	producer_settings	producer_struct			= {0,0,producerQueueHandle};
			uint8_t 			thread_index			= 0;
			uint8_t 			thread_full_flag		= 0;

	//by default all non explicitly initialized objects with static storage duration are initialized to 0
	struct monitor_task_info 	thread_info_array[256]	=
	{
			{0,0,0,0}
	};

	 osThreadAttr_t ProducerTask_attributes = {
	  .name = "ProducerTask",
	  .stack_size = 128 * 4,
	  .priority = (osPriority_t) osPriorityLow,
	};

	 	uint8_t uart_transmit_buffer[150] = {0};
	 	int buffer_len = 0;
		buffer_len = sprintf((char*) uart_transmit_buffer,"Monitor Task started\n");
		HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);

  /* Infinite loop */
  for(;;)
  {

	  while(xQueueReceive(UARTcommandQueueHandle, &command_struct, 0)!=errQUEUE_EMPTY)
	  {
		  buffer_len = sprintf((char*) uart_transmit_buffer,"New monitor command : %d\n", command_struct.UART_command_ID);
		  HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);
		  switch (command_struct.UART_command_ID)
		  {
		  case CREATE:

			  for(thread_index = 0; (thread_info_array[thread_index].status != DEAD_TASK) || (thread_info_array[thread_index].time_alive != 0); thread_index++)
			  {
				  if(thread_index >= 255)
				  {
					  for(thread_index = 0; thread_info_array[thread_index].status != DEAD_TASK; thread_index++)
					  {
						  if(thread_index >= 255)
						  {
							 thread_full_flag = 1;
							 break;
						  }
					  }
					  break;
				  }
			  }
			  if(thread_full_flag == 0)
			  {
				  producer_struct.producer_ID 					= thread_index;
				  producer_struct.production_delay 				= command_struct.delay;
				  thread_info_array[thread_index].thread_ID		= osThreadNew(StartProducerTask, &producer_struct, &ProducerTask_attributes);
				  buffer_len = sprintf((char*) uart_transmit_buffer,"Created Task %u\n",thread_index);
				  HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);
				  if (thread_info_array[thread_index].thread_ID != NULL)
				  {
					  thread_info_array[thread_index].status			= ALIVE_TASK;
					  thread_info_array[thread_index].production_rate	= (uint16_t) 1000/command_struct.delay;
					  thread_info_array[thread_index].time_alive		= (uint32_t)xTaskGetTickCount();
				  }

			  }
			  msg_struct.msg_ID = SPECIFIC_TASK;
			  msg_struct.data1  = thread_index;
			  break;

		  case DELETE:
			  buffer_len = sprintf((char*) uart_transmit_buffer,"Delete Task : %u\n", command_struct.thread_ID);
			  HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);
			  if(thread_info_array[command_struct.thread_ID].status == ALIVE_TASK)
			  {
				  if(osThreadTerminate(thread_info_array[command_struct.thread_ID].thread_ID)==osOK)
				  {
					  thread_info_array[command_struct.thread_ID].status			= DEAD_TASK;
					  thread_info_array[command_struct.thread_ID].production_rate	= 0;
					  thread_info_array[command_struct.thread_ID].time_alive		= ((uint32_t)xTaskGetTickCount())-thread_info_array[thread_index].time_alive;
					  thread_info_array[command_struct.thread_ID].thread_ID		= 0;
				  }

			  }
			  msg_struct.msg_ID = SPECIFIC_TASK;
			  msg_struct.data1  = command_struct.thread_ID;
			  break;

		  case SHOW_ONE:
		  	  msg_struct.msg_ID = SPECIFIC_TASK;
		  	  msg_struct.data1  = command_struct.thread_ID;
		  	  break;
		  case SHOW_ALL:
			  msg_struct.msg_ID = ALL_TASKS;

			  break;


		  }

	  }
	  if(msg_struct.msg_ID == SPECIFIC_TASK)
	  {
		  msg_struct.data2 = thread_info_array[msg_struct.data1].production_rate;
		  msg_struct.data3 = ((uint32_t) xTaskGetTickCount())-thread_info_array[msg_struct.data1].time_alive;
	  }
	  else
	  {
		  msg_struct.data1 	= 0;
		  msg_struct.data2 	= 0;
		  msg_struct.data3  = (uint32_t) xPortGetFreeHeapSize();
		  for(uint16_t i = 0; i<256; i++)
		  {
			  if(thread_info_array[i].status == ALIVE_TASK)
			  {
				  msg_struct.data1++;
				  msg_struct.data2+= thread_info_array[i].production_rate;

			  }
		  }
	  }
	  xQueueSend(monitorQueueHandle,&msg_struct,0);
	  osDelay(500);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartUARTTask */
/**
* @brief Function implementing the UARTTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUARTTask */
void StartUARTTask(void *argument)
{
  /* USER CODE BEGIN StartUARTTask */

	uint8_t rxBuf[UART_RX_BUF_SIZE] = {0};
	uint8_t txBuf[UART_TX_BUF_SIZE] = {0};
	uint16_t i = 0;
	int len =0;
	int16_t old_pos 				= -1;
	int16_t pos	 					= 0;
	HAL_UART_Receive_DMA(&huart1, rxBuf, UART_RX_BUF_SIZE);



	struct UART_command 		UART_struct 	= {0,0,0};
	struct monitor_msg			monitor_struct	= {0,0,0,0};
	struct producer_msg 	producer_struct = {0,0};


	/* Infinite loop */
	for(;;)
	{
		 pos = UART_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
		if (pos != old_pos)
		{

			if (pos >= old_pos+4)
			{
				len = sprintf((char*)txBuf,"new Data\n");
				HAL_UART_Transmit(&huart2, txBuf, len, HAL_MAX_DELAY);

				//normal case

				UART_struct.UART_command_ID = rxBuf[old_pos+1];
				UART_struct.thread_ID 		= rxBuf[old_pos+2];
				UART_struct.delay = (rxBuf[old_pos+3]<<8)|rxBuf[old_pos+4];
				old_pos += 4;
				xQueueSend(UARTcommandQueueHandle,&UART_struct,0);
				len = sprintf((char*) txBuf,"New uart command: %hu\n", UART_struct.UART_command_ID) ;
				HAL_UART_Transmit(&huart2, txBuf, len, HAL_MAX_DELAY);

			}
			else if((old_pos > pos) && (pos>=old_pos +4 - UART_RX_BUF_SIZE))
			{
				UART_struct.UART_command_ID = rxBuf[((old_pos+1)<50)?(old_pos+1):(old_pos+1-UART_RX_BUF_SIZE)];
				UART_struct.thread_ID 		= rxBuf[((old_pos+2)<50)?(old_pos+2):(old_pos+2-UART_RX_BUF_SIZE)];
				UART_struct.delay = (rxBuf[((old_pos+3)<50)?(old_pos+3):(old_pos+3-UART_RX_BUF_SIZE)]<<8)|rxBuf[((old_pos+4)<50)?(old_pos+4):(old_pos+4-UART_RX_BUF_SIZE)];
				((old_pos+4)<50)?(old_pos = old_pos + 4):(old_pos =old_pos+4-UART_RX_BUF_SIZE);
				xQueueSend(UARTcommandQueueHandle,&UART_struct,0);
			}


		}



		while(xQueueReceive(monitorQueueHandle, &monitor_struct, 0)!=errQUEUE_EMPTY)
		{
			monitor_struct_to_uint8(monitor_struct,txBuf);
			HAL_UART_Transmit(&huart1, txBuf, sizeof(struct monitor_msg), 100);

		}
		i=0;
		while((xQueueReceive(producerQueueHandle, &producer_struct, 0)!=errQUEUE_EMPTY)&& i <500)
		{
			producer_struct_to_uint8(producer_struct,txBuf);
			HAL_UART_Transmit(&huart1, txBuf, sizeof(struct producer_msg), 100);
			i++;
		}



		osDelay(1);
	 }
  /* USER CODE END StartUARTTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
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
