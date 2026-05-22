/*
 * producer_task.c
 *
 *  Created on: May 18, 2026
 *      Author: matthias
 */
#include "producer_task.h"



void StartProducerTask(void* argument)
{
	struct producer_msg value_struct			= {	.producer_ID 		=	((struct producer_settings*) argument)-> producer_ID,
													.producer_value 	= 	0};
	uint16_t 			delay 											=	((struct producer_settings*) argument)-> production_delay;
	osMessageQueueId_t	queue											=	((struct producer_settings*) argument)-> queue_ID;

	for(;;)
	{

		xQueueSend(queue,&value_struct,0);
		value_struct.producer_value++;
		osDelay(delay);
	}
	osThreadTerminate(NULL);
}
