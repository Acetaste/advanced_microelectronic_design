/*
 * Monitor.h
 *
 *  Created on: May 20, 2026
 *      Author: matthias
 */

#ifndef INC_MONITOR_TASK_H_
#define INC_MONITOR_TASK_H_

#include "stm32l4xx_hal.h"
#include "cmsis_os.h"


struct monitor_msg{
	uint8_t 			msg_ID;				//msg_id: enum monitor_mode with information for all (A) Tasks or a specific (S) Task
	uint8_t 			data1;				//for A: number of task, 			for S: thread_ID
	uint16_t 			data2;				//for A: combined production rate	for S: task specific production_rate
	uint32_t 			data3;				//for A: remaining heap space		for S: time the task is/was alive
};

struct monitor_task_info{
	uint8_t 		status; 			// enum live_status
	uint16_t 		production_rate;	//is set to production rate if alive and 0 if dead
	uint32_t 		time_alive;			//here the systick of creation is saved, when the task is killed the time it was alive is sent
	osThreadId_t	thread_ID;			//Thread ID to delete later
};

enum monitor_mode{
	 ALL_TASKS, SPECIFIC_TASK,
};

enum live_status{
	DEAD_TASK, ALIVE_TASK
};




#endif /* INC_MONITOR_TASK_H_ */
