/*
 * commandTask.h
 *
 *  Created on: May 21, 2026
 *      Author: Joachim
 */

#ifndef INC_COMMANDTASK_H_
#define INC_COMMANDTASK_H_

typedef struct cmd_struct{
    uint8_t     UART_command_ID;
    uint8_t     thread_ID;
    uint16_t    delay;
} cmd_struct;

enum UART_command_ID{
    CMD_SHOW_ALL,
	CMD_SHOW_ONE,
	CMD_CREATE,
	CMD_DELETE,
	CMD_INVALID = 0xFF
};

#endif /* INC_COMMANDTASK_H_ */
