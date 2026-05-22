/*
 * UART_task.c
 *
 *  Created on: May 21, 2026
 *      Author: matthias
 */

#include 	"UART_task.h"
#define BITMASK_LSB 0b11111111

void monitor_struct_to_uint8(struct monitor_msg monitor_struct, uint8_t* uint8_pointer)
{
	*(uint8_pointer+0) = (uint8_t) 		monitor_struct.msg_ID;
	*(uint8_pointer+1) = (uint8_t) 		monitor_struct.data1;
	*(uint8_pointer+2) = (uint8_t) ((	monitor_struct.data2>>8)	& BITMASK_LSB);
	*(uint8_pointer+3) = (uint8_t) (	monitor_struct.data2		& BITMASK_LSB);
	*(uint8_pointer+4) = (uint8_t) ((	monitor_struct.data3>>24)	& BITMASK_LSB);
	*(uint8_pointer+5) = (uint8_t) ((	monitor_struct.data3>>16)	& BITMASK_LSB);
	*(uint8_pointer+6) = (uint8_t) ((	monitor_struct.data3>>8) 	& BITMASK_LSB);
	*(uint8_pointer+7) = (uint8_t) (	monitor_struct.data3		& BITMASK_LSB);
}

void producer_struct_to_uint8(struct producer_msg producer_struct, uint8_t* uint8_pointer)
{
	*(uint8_pointer+0) = PRODUCER_INFO;
	*(uint8_pointer+1) = (uint8_t) 		producer_struct.producer_ID;
	*(uint8_pointer+2) = (uint8_t) ((	producer_struct.producer_value>>8)	& BITMASK_LSB);
	*(uint8_pointer+3) = (uint8_t) (	producer_struct.producer_value		& BITMASK_LSB);
}
