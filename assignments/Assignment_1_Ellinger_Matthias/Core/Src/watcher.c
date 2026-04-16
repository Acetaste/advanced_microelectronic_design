/*
 * function.c
 *
 *  Created on: Apr 13, 2026
 *      Author: matth
 */
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

/**
* @brief Function writing remaining stack of given thread id into

* @param[in] thread_id Array of thread id to get stack size for
* @param[out] stack_size Array of remaining stack sizes
* @param[in] cnt Number of threads information is given for
* @retval None
*/
void get_stack_size(osThreadId_t thread_id[], uint32_t stack_size[], int cnt)
{

	 for(int i = 0; i < cnt; i++ )
	 {
		 stack_size[i] = osThreadGetStackSpace(thread_id[i]);
	 }
}



/**
* @brief Function printing stack size information of tasks to UART
* @details Given 2 input arrays with the remaining stack size, in stack_size_array, and thread attributes in, thead_attribute_array, which need to be ordered and contain amount of variables as per worker_thread count,
* the function will print send out a formated string containing each tasks stack information
* @param[in] stack_size_array Array of remaining stack sizes
* @param[in] thread_attribute_array Array of thread attributes
* @param[in] cnt Number of threads information is given for
* @param[in] uart_handle Uart handle text buffer should be send to
* @retval None
*/
void print_stack_size(uint32_t stack_size[], osThreadAttr_t thread_attribute[],int cnt, UART_HandleTypeDef uart_handle)
{
	char buf[100];
	int len = 0;

	for(int i =0; i < cnt; i++)
	{
		len = sprintf(buf, "%s:\nstack size:%u Bytes remaining, stack size: %u Bytes\n",thread_attribute[i].name ,(unsigned int) (thread_attribute[i].stack_size), (unsigned int) stack_size[i]);
		HAL_UART_Transmit(&uart_handle, (uint8_t*) buf, len, 100);
	}
	len = sprintf(buf, "\n");
	HAL_UART_Transmit(&uart_handle, (uint8_t*) buf, len, 100);

}


