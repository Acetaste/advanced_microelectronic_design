/*
 * output.h
 *
 *  Created on: Apr 18, 2026
 *      Author: matth
 */

#ifndef INC_OUTPUT_H_
#define INC_OUTPUT_H_

#include "stm32l4xx_hal.h"
#include <stdio.h>

typedef struct {
  int val_p1;
  char str_p1[150];
  int val_p2;
  char str_p2[150];}_data;

void output_data(_data* data, UART_HandleTypeDef uart_handle);

#endif /* INC_OUTPUT_H_ */
