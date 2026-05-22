/*
 * task_creation.c
 *
 *  Created on: May 18, 2026
 *      Author: matthias
 */
#include "task_creation.h"



void StartProducerTask(void* argument)
{
	uint8_t 	ID 					= (producer_attr_strct*) argument-> producer_id;
	uint16_t 	delay 				= (producer_attr_strct*) argument-> production_delay;
	uint16_t	production_value 	= 0;

	for(;;)
	{

		production_value++;
		osDelay(delay);
	}
	osThreadTerminate(NULL);
}
