/*
 * oled.h
 *
 *  Created on: May 18, 2026
 *      Author: Joachim
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "oledc_font.h"
#include <stdio.h>

typedef struct{
  const uint8_t*   font_obj;
  uint8_t          font_r;
  uint8_t          font_g;
  uint8_t          font_b;
  uint8_t          font_orientation;
  uint16_t         font_first_char;
  uint16_t         font_last_char;
  uint16_t         font_height;
  uint16_t         x_cord;
  uint16_t         y_cord;
} oledc_t;

typedef struct oled_struct{
	uint8_t is_single_view;

	//a = task nr/id; b = prod total/value; c = free heap/time alive; d = prod rate
	uint8_t a;
	uint32_t b;
	uint32_t c;
	uint16_t d;
} oled_struct;

void comm_out(uint8_t command);
void data_out(uint8_t data);
void setup_oled(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);
void setup_screen_text(uint8_t is_single_view);
void output_to_oled(int a, int b, int c, int d);
void clearDDR(void);
void data_out_colour(uint8_t red, uint8_t green, uint8_t blue);
void draw_area(uint8_t red, uint8_t green, uint8_t blue, uint8_t row_start, uint8_t row_end,
						uint8_t column_start, uint8_t column_end, uint8_t isVertical);
void draw_pixel( uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue);
void oledc_set_font( oledc_t *ctx, const uint8_t *font_s, uint8_t red, uint8_t green, uint8_t blue );
void oledc_text( oledc_t *ctx, uint8_t *text, uint16_t x, uint16_t y );
void character( oledc_t *ctx, uint16_t ch );

#endif /* INC_OLED_H_ */
