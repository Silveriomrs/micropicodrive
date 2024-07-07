/**
* This module manage the View/GUI between RBPi Pico and the oLED Screen of 0.49" and LEDs Behavior.
* @Author: Original work from Dr. Gusman project.
* @Author: Silverio MRS (Popopo)
* @Version: 1.0
*/

#ifndef _VIEW_
#define _VIEW_
#include "pff/pff.h"
#include <string.h>
#include <stdio.h>
#include "ssd1306/ssd1306.h"

//LEDs Pins output
#define PIN_LED_ON 25
#define PIN_LED_SELECT 9
#define PIN_LED_READ 11
#define PIN_LED_WRITE 10
//Screen and SDCart pins.
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21

//Types of messages
typedef enum{
	WELCOME,
	SD_WAIT,
	//CART Messages mean to MDVs images, NOT TO SD cart.
	CART_RDY,
	CART_FORMAT_UNK,
	CART_ERR_LDING,
	CART_ERR_SAVING,
	CART_SAVING,
	CART_SAVED,
	//Folders messages
	FOLDER_ERR_OPEN,
	FOLDER_ERR_READ,
	FOLDER_EPTY,
	LDING_MDV,
	LDING_MDP,
	LDING_DEFAULT,
	ERR_CFG
} MSG_TYPE;

bool init_screen();
bool init_gui();
void setSCRM(char* scrm);
void show_file_name(const char* fname, bool IN_FOLDER);
void showMSG(MSG_TYPE m);
void printMSG(const char* msg1, const char* msg2, const char* msg3, int time);
void printHorizontalScroll(const char*msg1, const char*msg2, int row2, const int time);
void printVerticalScroll(const char*msg1, const char*msg2, const char* msg3, const int time);

#endif