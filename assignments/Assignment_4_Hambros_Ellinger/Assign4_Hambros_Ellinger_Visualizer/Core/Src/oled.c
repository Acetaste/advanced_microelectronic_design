/*
 * oled.c
 *
 *  Created on: May 18, 2026
 *      Author: Joachim
 */
#include "oled.h"

#define X_TITLE 4
#define X_DESCRIPTIONS 4
#define X_VALUES 65
#define Y_TITLE 4
#define Y_1 28
#define Y_2 46
#define Y_3 64
#define Y_4 82

static oledc_t oledc;
static SPI_HandleTypeDef *spi_handle;
static UART_HandleTypeDef *uart_handle;
char msg_buf[32];

/**
 * common commands for the OLED controller
 */
const uint8_t SELECT_COLUMN_BORDERS = 0x15;
const uint8_t SELECT_ROW_BORDERS = 0x75;
const uint8_t ENABLE_WRITE_RAM = 0x5C;

/**
 * options for the gradient, combine these together with a starting color using bitwise OR
 */
const uint16_t GRAD_HORIZONTAL = 0x8000;
const uint16_t GRAD_VERTICAL = 0;
const uint16_t GRAD_DECREASING = 0x4000;
const uint16_t GRAD_TIME_FAST = 0x0400;
const uint16_t GRAD_TIME_MEDIUM = 0x0800;
const uint16_t GRAD_TIME_SLOW = 0x0C00;
const uint16_t GRAD_TIME_SLOWER = 0x1000;
const uint16_t GRAD_STEPS_SMALL = 0x0100;
const uint16_t GRAD_STEPS_MEDIUM = 0x0200;
const uint16_t GRAD_STEPS_BIG = 0x0300;

const uint8_t DRAW_HORIZONTAL = 0x0;						//write into DDRAM row by row (bottom up)
const uint8_t DRAW_VERTICAL = 0x1;							//write into DDRAM column by column (left to right)

/**
 * @brief send command (1 byte) to the OLED controller
 * @param command
 */
void comm_out(uint8_t command){
	//set CS to low (transmission)
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	//Set D/C# low (command)
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
	//transmit command
	if(HAL_SPI_Transmit(spi_handle, &command, 1, 100) != HAL_OK){
		HAL_UART_Transmit(uart_handle, (uint8_t *)"error\n", 6, 1000);
	}
	//set CS to high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

/**
 * @brief send data (1 byte) to the OLED controller
 * @param data
 */
void data_out(uint8_t data){
	//set CS to low (transmission)
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	//Set D/C# high (data)
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	//transmit data
	if(HAL_SPI_Transmit(spi_handle, &data, 1, 100) != HAL_OK){
		HAL_UART_Transmit(uart_handle, (uint8_t *)"error\n", 6, 1000);
	}
	//set CS to high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}


/**
 * @brief initializes oled registers as per datasheet
 */
void setup_oled(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart)
{
	spi_handle = hspi;
	uart_handle = huart;

    comm_out(0xfd);		//Set Command Lock
    data_out(0xb1); 	//Unlock OLED driver IC
    comm_out(0xae); 	//Display off
    comm_out(0xa0); 	//Set Re-map Color Depth
    data_out(0x62); 	//65K Color, swap top/bottom
    comm_out(0xa1); 	//Set Display Start Line
    data_out(0x00);
    comm_out(0xa2); 	//Set Display Offset
    data_out(0x60);
    comm_out(0xa6); 	//Normal display
    comm_out(0xab); 	//Function Selection
    data_out(0x01);		// 8bit-8080 interface
    comm_out(0xb1); 	//Set Reset (Phase 1) /Pre-charge (Phase 2) period
    data_out(0x53);
    comm_out(0xb3); 	//Set frame rate
    data_out(0xF0);
    comm_out(0xb4); 	//External VSL
    data_out(0xa0);
    data_out(0xb5);
    data_out(0x55);
    comm_out(0xb9); 	//Use Built-in Linear LUT
    comm_out(0xbb); 	//Set Pre-charge voltage
    data_out(0x00);
    comm_out(0xbe); 	//Set VCOMH
    data_out(0x07);
    comm_out(0xc1); 	//Set contrast level for R,G,B
    data_out(0x75); 	//Blue contrast set
    data_out(0x42); 	//Green contrast set
    data_out(0x49); 	//Red contrast set
    comm_out(0xc7); 	//Master current control
    data_out(0x0e);
    comm_out(0xca); 	//Set MUX Ratio
    data_out(0x5F);		//96 Duty
    clearDDR();			//Clear the whole DDRAM
    comm_out(0xaf); 	//Display on
}

/**
 * @brief draws initial Roll, Pitch, Yaw text on white background
 */
void setup_screen_text(uint8_t is_single_view)
{
    draw_area(31, 63, 31, 0, 95, 0, 95, DRAW_HORIZONTAL);

    oledc_set_font( &oledc, guiFont_Tahoma_10_Regular, 20, 40, 5 );

    sprintf((char *)msg_buf, is_single_view ? "Single Task" : "All Tasks");
    oledc_text(&oledc, (uint8_t *)msg_buf, X_TITLE, Y_TITLE);

    oledc_set_font( &oledc, guiFont_Tahoma_8_Regular, 5, 10, 5 );

    sprintf((char *)msg_buf, is_single_view ? "Task ID:" : "Total tasks:");
    oledc_text(&oledc, (uint8_t *)msg_buf, X_DESCRIPTIONS, Y_1);
    sprintf((char *)msg_buf, is_single_view ? "Prod. value:" : "Prod. total:");
	oledc_text(&oledc, (uint8_t *)msg_buf, X_DESCRIPTIONS, Y_2);
	sprintf((char *)msg_buf, is_single_view ? "Time alive:" : "Free heap:");
	oledc_text(&oledc, (uint8_t *)msg_buf, X_DESCRIPTIONS, Y_3);
	sprintf((char *)msg_buf, "Prod. rate:");
	oledc_text(&oledc, (uint8_t *)msg_buf, X_DESCRIPTIONS, Y_4);
}

/**
 * @brief draws values for roll pitch and yaw on screen, roll and pitch values (-90:90) get mapped to green and red font values. Turn more red the closer they are to (-)90
 * @param roll float angle
 * @param pitch float angle
 * @param yaw float angle
 */
void output_to_oled(int a, int b, int c, int d) //task nr/id; prod total/value; free heap/time alive; prod rate
{

	draw_area(31, 63, 31, X_VALUES, 95, Y_1, 95, 0);

	sprintf((char *)msg_buf,"%d", a);
	oledc_text(&oledc, (uint8_t *)msg_buf, X_VALUES, Y_1);
	sprintf((char *)msg_buf,"%d", b);
	oledc_text(&oledc, (uint8_t *)msg_buf, X_VALUES, Y_2);
	sprintf((char *)msg_buf,"%d", c);
	oledc_text(&oledc, (uint8_t *)msg_buf, X_VALUES, Y_3);
	sprintf((char *)msg_buf,"%d", d);
	oledc_text(&oledc, (uint8_t *)msg_buf, X_VALUES, Y_4);
}
/**
 * @brief clear the entire Display Data Ram (128x128 pixels) of the OLED controller
 */
void clearDDR(void)
{
	int i,j;
	comm_out(SELECT_COLUMN_BORDERS);
	data_out(0x00);
	data_out(0x7f);
	comm_out(SELECT_ROW_BORDERS);
	data_out(0x00);
	data_out(0x7f);
	comm_out(ENABLE_WRITE_RAM);
	for(i=0;i<128;i++)
	{
		 for(j=0;j<128;j++)
		 {
			 data_out(0x00);
			 data_out(0x00);
		 }
	}
}

/**
 * @brief write one pixel to DDRAM (comm_out(ENABLE_WRITE_RAM) must be called beforehand)
 *
 * @param red 5 bit value for red
 * @param green 6 bit value for green
 * @param blue 5 bit value for blue
 */
void data_out_colour(uint8_t red, uint8_t green, uint8_t blue){
	data_out((red << 3) | (green >> 3));
	data_out((green << 5) | blue);
}

void draw_area(uint8_t red, uint8_t green, uint8_t blue, uint8_t row_start, uint8_t row_end,
						uint8_t column_start, uint8_t column_end, uint8_t isVertical){

	comm_out(SELECT_COLUMN_BORDERS);
	data_out(column_start + 0x10);
	data_out(column_end + 0x10);

	comm_out(SELECT_ROW_BORDERS);
	data_out(row_start);
	data_out(row_end);

	if(isVertical){
		comm_out(0xa0);
		data_out(0x63); //change to vertical writing

		comm_out(ENABLE_WRITE_RAM);

		for (int col = 1; col <= column_end - column_start + 1; ++col) {
			for (int row = 1; row <= row_end - row_start + 1; ++row) {
				data_out_colour(red, green, blue);
			}
		}

		comm_out(0xa0);
		data_out(0x62); //change back to horizontal writing

		return;
	}

	comm_out(ENABLE_WRITE_RAM);

	for (int row = 1; row <= row_end - row_start + 1; ++row) {
		for (int col = 1; col <= column_end - column_start + 1; ++col) {
			data_out_colour(red, green, blue);
		}
	}
}

void draw_pixel( uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue){
	comm_out(SELECT_COLUMN_BORDERS);
	data_out(col + 0x10);
	data_out(col + 0x10);

	comm_out(SELECT_ROW_BORDERS);
	data_out(row);
	data_out(row);

	comm_out(ENABLE_WRITE_RAM);
	data_out_colour(red, green, blue);
}

void oledc_set_font( oledc_t *ctx, const uint8_t *font_s, uint8_t red, uint8_t green, uint8_t blue )
{
    ctx->font_obj           = font_s;
    ctx->font_first_char    = font_s[2] + (font_s[3] << 8);
    ctx->font_last_char     = font_s[4] + (font_s[5] << 8);
    ctx->font_height        = font_s[6];
    ctx->font_r         		= red;
    ctx->font_g         		= green;
    ctx->font_b         		= blue;
}

void oledc_text( oledc_t *ctx, uint8_t *text, uint16_t x, uint16_t y )
{
    uint8_t *ptr = text;

    if ( ( x >= 96 ) || ( y >= 96 ) )
    {
        return;
    }

    ctx->x_cord = x;
    ctx->y_cord = y;

    while( *ptr )
    {
        character( ctx, *ptr++ );
    }
}

void character( oledc_t *ctx, uint16_t ch )
{
    uint8_t     ch_width = 0;
    uint8_t     x_cnt;
    uint8_t     y_cnt;
    uint16_t    x = 0;
    uint16_t    y = 0;
    uint16_t    tmp;
    uint8_t     temp = 0;
    uint8_t     mask = 0;
    uint32_t    offset;
    const uint8_t *ch_table;
    const uint8_t *ch_bitmap;

    if( ch < ctx->font_first_char )
        return;

    if( ch > ctx->font_last_char )
        return;

    offset = 0;
    tmp = (ch - ctx->font_first_char) << 2;
    ch_table = ctx->font_obj + 8 + tmp;
    ch_width = *ch_table;

    offset = (uint32_t)ch_table[1] + ((uint32_t)ch_table[2] << 8) + ((uint32_t)ch_table[3] << 16);

    ch_bitmap = ctx->font_obj + offset;

    y = ctx->y_cord;
    for (y_cnt = 0; y_cnt < ctx->font_height; y_cnt++)
    {
        x = ctx->x_cord;
        mask = 0;
        for( x_cnt = 0; x_cnt < ch_width; x_cnt++ )
        {
            if( !mask )
            {
                temp = *ch_bitmap++;
                mask = 0x01;
            }
            if( temp & mask )
                 draw_pixel(x, y, ctx->font_r, ctx->font_g, ctx->font_b);

            x++;
            mask <<= 1;
        }
        y++;
    }
    ctx->x_cord = x + 1;
}
