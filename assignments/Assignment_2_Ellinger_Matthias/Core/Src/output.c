/*
 * output.c
 *
 *  Created on: Apr 18, 2026
 *      Author: matth
 */
#include "output.h"

void output_data(_data* data, UART_HandleTypeDef uart_handle)
{
	char buf[350];
	int len;
	len = sprintf(buf, "Val P1: %d\nStr P1: %s\nVal P2: %d\nStr P2: %s\n\n",data->val_p1 ,data->str_p1, data->val_p2 ,data->str_p2);
	HAL_UART_Transmit(&uart_handle, (uint8_t*) buf, len, 100);
}
