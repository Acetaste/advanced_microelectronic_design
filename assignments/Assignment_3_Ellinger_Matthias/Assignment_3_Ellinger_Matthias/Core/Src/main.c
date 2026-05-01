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
#include "semphr.h"
#include "prod_struct.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_IT_BLOCKED_MSG "Uart buffer currently blocked, input not read\n"
#define BLOCKED_MSG_LEN sizeof(UART_IT_BLOCKED_MSG)
#define UART_RECEIVE_BUFFER_SIZE 15
#define NUMB_PROD_TASK 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Definitions for monitorTask */
osThreadId_t monitorTaskHandle;
uint32_t monitorTaskBuffer[ 512 ];
osStaticThreadDef_t monitorTaskControlBlock;
const osThreadAttr_t monitorTask_attributes = {
  .name = "monitorTask",
  .cb_mem = &monitorTaskControlBlock,
  .cb_size = sizeof(monitorTaskControlBlock),
  .stack_mem = &monitorTaskBuffer[0],
  .stack_size = sizeof(monitorTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for producerTask1 */
osThreadId_t producerTask1Handle;
uint32_t producerTask1Buffer[ 128 ];
osStaticThreadDef_t producerTask1ControlBlock;
const osThreadAttr_t producerTask1_attributes = {
  .name = "producerTask1",
  .cb_mem = &producerTask1ControlBlock,
  .cb_size = sizeof(producerTask1ControlBlock),
  .stack_mem = &producerTask1Buffer[0],
  .stack_size = sizeof(producerTask1Buffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for producerTask2 */
osThreadId_t producerTask2Handle;
uint32_t producerTask2Buffer[ 128 ];
osStaticThreadDef_t producerTask2ControlBlock;
const osThreadAttr_t producerTask2_attributes = {
  .name = "producerTask2",
  .cb_mem = &producerTask2ControlBlock,
  .cb_size = sizeof(producerTask2ControlBlock),
  .stack_mem = &producerTask2Buffer[0],
  .stack_size = sizeof(producerTask2Buffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for producerTask3 */
osThreadId_t producerTask3Handle;
uint32_t producerTask3Buffer[ 128 ];
osStaticThreadDef_t producerTask3ControlBlock;
const osThreadAttr_t producerTask3_attributes = {
  .name = "producerTask3",
  .cb_mem = &producerTask3ControlBlock,
  .cb_size = sizeof(producerTask3ControlBlock),
  .stack_mem = &producerTask3Buffer[0],
  .stack_size = sizeof(producerTask3Buffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for producerQueue */
osMessageQueueId_t producerQueueHandle;
uint8_t producerQueue1Buffer[ 16 * sizeof( uint16_t ) ];
osStaticMessageQDef_t producerQueue1ControlBlock;
const osMessageQueueAttr_t producerQueue_attributes = {
  .name = "producerQueue",
  .cb_mem = &producerQueue1ControlBlock,
  .cb_size = sizeof(producerQueue1ControlBlock),
  .mq_mem = &producerQueue1Buffer,
  .mq_size = sizeof(producerQueue1Buffer)
};
/* Definitions for TaskSettingsSem */
osSemaphoreId_t TaskSettingsSemHandle;
osStaticSemaphoreDef_t uartSemControlBlock;
const osSemaphoreAttr_t TaskSettingsSem_attributes = {
  .name = "TaskSettingsSem",
  .cb_mem = &uartSemControlBlock,
  .cb_size = sizeof(uartSemControlBlock),
};
/* USER CODE BEGIN PV */
static int 					uart_receive_index;
static volatile uint8_t 	uart_receive_char								= '\0';
static volatile uint8_t 	uart_receive_buffer[UART_RECEIVE_BUFFER_SIZE]	= {0};
static uint8_t				new_uart_input_flag 							= 0;
static uint8_t				uart_overflow_flag 								= 0;
static unsigned int 		task_delay_array[NUMB_PROD_TASK] 				={1000,5000,10000};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartmonitorTask(void *argument);
void StartproducerTask1(void *argument);
void StartproducerTask2(void *argument);
void StartproducerTask3(void *argument);

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of TaskSettingsSem */
  TaskSettingsSemHandle = osSemaphoreNew(1, 1, &TaskSettingsSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of producerQueue */
  producerQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &producerQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of monitorTask */
  monitorTaskHandle = osThreadNew(StartmonitorTask, NULL, &monitorTask_attributes);

  /* creation of producerTask1 */
  producerTask1Handle = osThreadNew(StartproducerTask1, NULL, &producerTask1_attributes);

  /* creation of producerTask2 */
  producerTask2Handle = osThreadNew(StartproducerTask2, NULL, &producerTask2_attributes);

  /* creation of producerTask3 */
  producerTask3Handle = osThreadNew(StartproducerTask3, NULL, &producerTask3_attributes);

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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart-> Instance == USART2)
	{
		if((new_uart_input_flag == 0) && (uart_overflow_flag == 0))
		{
			if(uart_receive_char == '\0')
			{
				uart_receive_buffer[uart_receive_index] = uart_receive_char;
				new_uart_input_flag = 1;
			}
			else if(uart_receive_index < UART_RECEIVE_BUFFER_SIZE-1)
			{
				uart_receive_buffer[uart_receive_index] = uart_receive_char;
				uart_receive_index++;
			}
			else
			{
				uart_overflow_flag = 1;

			}
		}
		HAL_UART_Receive_IT(&huart2,(uint8_t*) &uart_receive_char, 1);
	}



}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartmonitorTask */
/**
  * @brief  Function implementing the monitorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartmonitorTask */
void StartmonitorTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	uint8_t uart_transmit_buffer[150] = {0};
	int buffer_len = 0;
	unsigned int task_index = 0;
	unsigned int task_delay = 0;
	prod_struct buffer_struct = {.index = 0, .value = 0};
	buffer_len = sprintf((char*) uart_transmit_buffer,"Monitor Task started\n");
	HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);
	HAL_UART_Receive_IT(&huart2, (uint8_t*) &uart_receive_char, 1);
	/* Infinite loop */
	for(;;)
	{
		if(uart_overflow_flag == 1)
		{
			  uart_overflow_flag = 0;
			  uart_receive_index = 0;
			  buffer_len = sprintf((char*) uart_transmit_buffer,"Uart message was bigger than buffer(%d chars), message not interpreted. Try again!\n", UART_RECEIVE_BUFFER_SIZE);
			  HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);

		}


		if(new_uart_input_flag == 1)
		{
			new_uart_input_flag = 0;
			uart_receive_index = 0;

			buffer_len = sprintf((char*) uart_transmit_buffer,"Read in: \"%s\". Ready for new input\n",(char*) uart_receive_buffer);
			HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, HAL_MAX_DELAY);


			if(sscanf((char*) uart_receive_buffer, "T%u %ums", &task_index, &task_delay)==2)
			{
				buffer_len = sprintf((char*)uart_transmit_buffer,"task number: %u, task delay: %u\n", task_index, task_delay);
				HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, 100);
				xSemaphoreTake(TaskSettingsSemHandle, HAL_MAX_DELAY);
				task_delay_array[task_index-1] = task_delay;
				xSemaphoreGive(TaskSettingsSemHandle);

			}
			else
			{
				buffer_len = sprintf((char*)uart_transmit_buffer,"wrong format, expected task number and task delay space separated\n");
				HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, 100);
			}
		}
		while(xQueueReceive(producerQueueHandle, &buffer_struct, 0)!=errQUEUE_EMPTY)
		{
			buffer_len = sprintf((char*)uart_transmit_buffer,"New Value : %hu received by task: %hu\n",buffer_struct.value,buffer_struct.index+1);
			HAL_UART_Transmit(&huart2, uart_transmit_buffer, buffer_len, 100);
		}
		osDelay(1);
	}
	osThreadTerminate(NULL);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartproducerTask1 */
/**
* @brief Function implementing the producerTask1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartproducerTask1 */
void StartproducerTask1(void *argument)
{
  /* USER CODE BEGIN StartproducerTask1 */
	unsigned int task1delay = 100;
	prod_struct task1_struct = {.index = 0, .value = 0};
  /* Infinite loop */
  for(;;)
  {
	  xSemaphoreTake(TaskSettingsSemHandle, HAL_MAX_DELAY);
	  task1delay = task_delay_array[0];
	  xSemaphoreGive(TaskSettingsSemHandle);
	  xQueueSend(producerQueueHandle,&task1_struct,0);
	  task1_struct.value++;
	  osDelay(task1delay);
  }
  /* USER CODE END StartproducerTask1 */
}

/* USER CODE BEGIN Header_StartproducerTask2 */
/**
* @brief Function implementing the producerTask2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartproducerTask2 */
void StartproducerTask2(void *argument)
{
  /* USER CODE BEGIN StartproducerTask2 */
	unsigned int task2delay = 100;
	prod_struct task2_struct = {.index = 1, .value = 0};
  /* Infinite loop */
  for(;;)
  {
	  xSemaphoreTake(TaskSettingsSemHandle, HAL_MAX_DELAY);
	  task2delay = task_delay_array[1];
	  xSemaphoreGive(TaskSettingsSemHandle);
	  xQueueSend(producerQueueHandle,&task2_struct,0);
	  task2_struct.value++;
	  osDelay(task2delay);
  }
  /* USER CODE END StartproducerTask2 */
}

/* USER CODE BEGIN Header_StartproducerTask3 */
/**
* @brief Function implementing the producerTask3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartproducerTask3 */
void StartproducerTask3(void *argument)
{
  /* USER CODE BEGIN StartproducerTask3 */
	unsigned int task3delay = 100;
	prod_struct task3_struct = {.index = 2, .value = 0};
  /* Infinite loop */
  for(;;)
  {
	  xSemaphoreTake(TaskSettingsSemHandle, HAL_MAX_DELAY);
	  task3delay = task_delay_array[2];
	  xSemaphoreGive(TaskSettingsSemHandle);
	  xQueueSend(producerQueueHandle,&task3_struct,0);
	  task3_struct.value++;
	  osDelay(task3delay);
  }
  /* USER CODE END StartproducerTask3 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
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
