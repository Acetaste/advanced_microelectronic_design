/*
 * task_creation.h
 *
 *  Created on: May 18, 2026
 *      Author: matthias
 */

#ifndef INC_PRODUCER_TASK_H_
#define INC_PRODUCER_TASK_H_

#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "queue.h"



struct producer_settings
{
	uint8_t 			producer_ID;
	uint16_t 			production_delay;
	osMessageQueueId_t	queue_ID;

};

struct producer_msg
{
	uint8_t 			producer_ID;
	uint16_t 			producer_value;
};



 void StartProducerTask(void* argument);


#endif /* INC_PRODUCER_TASK_H_ */
