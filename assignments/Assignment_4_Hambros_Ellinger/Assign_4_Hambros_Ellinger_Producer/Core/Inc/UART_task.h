/*
 * UART_task.h
 *
 *  Created on: May 21, 2026
 *      Author: matthias
 */

#ifndef INC_UART_TASK_H_
#define INC_UART_TASK_H_

#include "monitor_task.h"
#include "producer_task.h"

struct UART_command{
	uint8_t		UART_command_ID;
	uint8_t 	thread_ID;
	uint16_t	delay;
};

enum UART_command_ID{
	SHOW_ALL,
	SHOW_ONE,
	CREATE,
	DELETE
};

enum UART_msg_ID{
	 ALL_TASK_INFO, SPECIFIC_TASK_INFO, PRODUCER_INFO
};

void monitor_struct_to_uint8(struct monitor_msg monitor_struct, uint8_t* uint8_pointer);
void producer_struct_to_uint8(struct producer_msg producer_struct, uint8_t* uint8_pointer);
#endif /* INC_UART_TASK_H_ */
