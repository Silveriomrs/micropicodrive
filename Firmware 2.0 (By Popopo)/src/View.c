/**
 * Implementation of the module for View/GUI operations between the RBP Pico and the screen & Leds.
 * This module is in charge of visual representation of the information dealth by the RPBPico and the user.
 * @Author: Original work from Dr. Gusman project.
 * @Author: Silverio MRS (Popopo)
 * @Version: 1.0
 */

#include <stdlib.h> 
#include "View.h"

#define PRINT_STR(STR, COL, ROW, MODE) ssd1306_draw_string(&disp, COL * 5, ROW * 8, MODE, STR)
#define RENDER_SCR() ssd1306_show(&disp)
#define CLR_SCR() ssd1306_clear(&disp)

ssd1306_t disp;
uint8_t mode = 1;                   //Mode 1 by default


/**
 * Shows the current file name on the screen. It's the main function for browsing the SD Card.
 * The function will take the name and split it if needed.
 * Also it shows if the file is a directory or an archive.
 * @param fname Name of the file/directory
 * @param IN_FOLDER It indicates if the pointer is aiming at a folder or not.
 */
void show_file_name(const char* fname, bool IN_FOLDER){
	char auxName[12];														//Temporal string to host the name and manipulate it.
	char type[3];															//Stores the type/extension (last 3 characters).
	char *head;
	int sz = strlen(fname);
	char name[sz];
	
	if(IN_FOLDER ){
		head = "[Dir]";
	} else {
		sprintf(type, "> %s", &fname[sz-3]);									//Get the type
		head = type;
		//Get only the name when it's a file
		memset(name, 0, sz);
		memcpy(name, fname, sz - 4);											
	}

    CLR_SCR();
    PRINT_STR(head, 0, 0, 1);

	//Check if the name is too long for one line.
    if(mode == 1 && strlen(fname) > 11) {									//If name with extension is longer than 11 chars, split it in two lines
        memset(auxName, 0, 12);
        memcpy(auxName, fname, 8);											//Copy the name into the temporal string.
        PRINT_STR(auxName, 0, 2, mode);										//Print the name at 2nd line.
        sprintf(auxName, "       %s", &fname[8]);							//Compose the last line overwritting with spaces the name and leaving the extension.
        PRINT_STR(auxName, 0, 3, mode);										//Print the extension at 3rd line of the screen.
    } else if(mode == 1) {PRINT_STR(fname, 0, 2, mode);						//Otherwise it is printed compleately in line number 2 (screen).
	} else {
		if(sz > 8) printHorizontalScroll(head,name,mode*8,140);  //TODO: Puede ser que deba valer 9, para evitar scroll por punto (letras(5) + punto(1) + ext(3))
		CLR_SCR();
		PRINT_STR(head,0,0,mode);
		PRINT_STR(name,0,2,mode);
	}

    RENDER_SCR();
}

/**
 * Shows the most common messages, after cleaning the screen.
 * @param MSG_TYPE any valid value from it.
 * @param ms Delay hold time (ms) the message before give back the control.
 */
void showMSG(MSG_TYPE m){
	// Reference of space for Screen mode 1	-> "12345678901" x 4 lines
	// Rerefence of space for Screen mode 2 -> "12345" x 4 lines with superposition. Last one incomplete
	// char *msg1;
	// char *msg2;
	// char *msg3;
	// int t = 0;

	switch(m){
		case WELCOME:
			printVerticalScroll(" QL","","",110);
			CLR_SCR();
			PRINT_STR(" MPD", 0, 0, 2);
			PRINT_STR("V0.10", 0, 2, 2);								//Development version RC10 11:00H 15/07/2024
			RENDER_SCR();
			sleep_ms(0);
			break;
		case SD_WAIT:
			CLR_SCR();
			PRINT_STR("No SD", 0, 0, 2);
			PRINT_STR("In", 3/mode, 2, 2);
			RENDER_SCR();
			sleep_ms(0);
			break;
		case CART_RDY:
			if(mode == 1) printMSG("Cartridge","Ready","",0);
			else printMSG("Cart","Ready","",0);
			break;
		case FOLDER_ERR_OPEN:											//Here when there was an error opening the folder.
			if(mode == 1) printMSG("Error","opening","folder.",2000);
			else printHorizontalScroll("Error","Opening folder",8*mode,140);
			break;
		case FOLDER_ERR_READ:											//Here when there was an error reading the folder.
			if(mode==1) printMSG("Error","reading","folder.",2000);
			else printHorizontalScroll("Error","reading folder",8*mode,140);
			break;
		case FOLDER_EPTY:
			CLR_SCR();
			PRINT_STR("Empty", 0, 0, 2);
			PRINT_STR("Dir", 3/mode, 2, 2);
			RENDER_SCR();
			break;
		case CART_FORMAT_UNK:
			if(mode==1) printMSG("Unknown","cartridge","format.",4000);
			else printHorizontalScroll("UNK","cartridge format",8*mode,140);
			break;
		case CART_ERR_LDING:
			if(mode == 1) printMSG("Error","loading","cartridge.",4000);
			else printHorizontalScroll("Error","loading cartridge",8*mode,140);
			break;
		case CART_ERR_SAVING:
			if(mode ==1) printMSG("Error","saving","cartridge.",2000);
			else printHorizontalScroll("Error","saving cartridge",8*mode,140);
			break;
		case CART_SAVING:
			if(mode==1) printMSG("Saving","cartridge..","",0);
			else printHorizontalScroll("SAVE","saving cartridge",8*mode,90);
			break;
		case CART_SAVED:
			if(mode==1) printMSG("Cartridge","saved","",2000);
			else {
				CLR_SCR();
				PRINT_STR("SAVED", 0, 1, 2);
				RENDER_SCR();
				sleep_ms(2000);
			}
			break;
		case LDING_MDV:
			if(mode==1) printMSG("Loading MDV","cartridge..","",0);
			else printHorizontalScroll("LOAD","loading MDV cartridge",8*mode,120);
			break;
		case LDING_MDP:
			if(mode==1)printMSG("Loading MPD","cartridge..","",0);
			else printHorizontalScroll("LOAD","loading MPD cartridge",8*mode,120);
			break;
		case LDING_DEFAULT: 
			printHorizontalScroll("","default file",5,110);
			break;
		case ERR_CFG:
			if(mode==1)printMSG("Error","CONFIG.CFG","",3000);
			else printHorizontalScroll("ERROR","Config.cfg file",8*mode,90);
			break;
	}
}

//TODO: Function for testing purpose. Delete when no need
// void printNumber(const char* msg, int number, int time){
// 	CLR_SCR();
// 	char scount[12];
// 	sprintf(scount, "%d", number);					// convert the count number into a string
// 	PRINT_STR(msg,0,1,mode);
// 	PRINT_STR(scount,0,2,mode);
// 	RENDER_SCR();
// 	sleep_ms(time);
// }

/**
 * It prints on the display a double message, separate by lines and holding it for a defined time.
 * Depending on the screen mode, some messages would not be shown.
 * For example, in mode 2, third line won't be used.
 * @param msg1 Message to show in the first line.
 * @param msg2 Second message to show, it will be displayed in the second line. Usually a value or resault that need to be shown.
 * @param msg3 Third message to show. It will be shown in the third line.
 * @param time Time in milliseconds to hold the message on the screen before return the control.
*/
void printMSG(const char* msg1, const char* msg2, const char* msg3, int time){
	CLR_SCR();
	PRINT_STR(msg1,0,0,mode);
	PRINT_STR(msg2,0,2,mode);
	if(msg3[0] != '\0') PRINT_STR(msg3,0,3,mode);
	RENDER_SCR();
	sleep_ms(time);
}


/**
 * It shows a message with lateral scroll. Specially conveniente for big fonts.
 * @param msg1 Message to print in the upper zone of the screen.
 * @param msg2 Message to print in the body of the screen with horizontal scroll.
 * @param time A good time set is 140ms but users can define others.
*/
void printHorizontalScroll(const char*msg1, const char*msg2, int row2, const int time){
	int size = strlen(msg2);
	int i = 0;
	while(i<size){
		//printMSG(msg1,(char*)&msg2[i],msg3,time);
		CLR_SCR();
		if(msg1[0] != '\0') ssd1306_draw_string(&disp, 0, 0, mode, msg1);
		if(msg2[0] != '\0') ssd1306_draw_string(&disp, 0, row2, mode, (char*)&msg2[i]);
		RENDER_SCR();
		sleep_ms(time);
		i++;
	}
}

/**
 * It shows a message with vertical scroll. Specially conveniente for little messages.
 * The function adjust the time related to the font size (mode) to equalize the scroll speed.
 * The lenght of a message to fit the screen:
 * 	 on Mode 1 is 11 chars.
 *   on Mode 2 is 5 chars.
 *   on Mode 3 is 3 or 4 chars.
 * For obvious reasons, first message should be not empty nor null.
 * @param msg1 Firts line message to print.
 * @param msg2 Second line message to print.
 * @param msg3 Third line message to print.
 * @param time A good time set is 140ms but users can define others.
*/
void printVerticalScroll(const char*msg1, const char*msg2, const char* msg3, const int time){
	int mode = 3;
	int row = 32;
	int countMsg = 1;
	if(msg2[0] != '\0') countMsg++;
	if(msg3[0] != '\0') countMsg++;
	int lastLine = countMsg*(mode*8);
	
	//In mode 2, a char uses 16 rows
	while(row >= -(lastLine)){
		CLR_SCR();
		ssd1306_draw_string(&disp, 0, row, mode, msg1);
		if(msg2[0] != '\0') ssd1306_draw_string(&disp, 0, row + 8*mode, mode, msg2);
		if(msg3[0] != '\0') ssd1306_draw_string(&disp, 0, row + 2*8*mode, mode, msg3);
		RENDER_SCR();
		sleep_ms(time/mode);
		row--;
	}
}

//Initialize the I2C bus
void init_i2c(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

//Initialize UI leds
void init_leds(){
    gpio_init(PIN_LED_PWR);
    gpio_init(PIN_LED_SELECT);
    gpio_init(PIN_LED_READ);
    gpio_init(PIN_LED_WRITE);

    gpio_set_dir(PIN_LED_PWR, true);
    gpio_set_dir(PIN_LED_SELECT, true);
    gpio_set_dir(PIN_LED_READ, true);
    gpio_set_dir(PIN_LED_WRITE, true);

    gpio_put(PIN_LED_PWR, 1);
}

/**
 * Initialize the I2C screen.
 * Here should be set up the fonts based on user preferences.
 */
bool init_screen(){
    disp.external_vcc=false;
    //Setup the oLED screen.
    if(!ssd1306_init(&disp, 64, 32, 0x3C, I2C_PORT)) return false;

    CLR_SCR();
    RENDER_SCR();
    return true;
}

/**
 * It sets the Screen mode to use with the cartridge.
 * In case the value is not valid it ignores it.
 * @param scrm Screen mode to use {default = 1, medium = 2, big = 3}
 */
void setSCRM(char* scrm){
	//TODO What better to use strtol o atoi?. first safer than second one, but requires more code/memory.
	//Converting it into int
	//char *endptr;
	//int m = strtol(scrm, &endptr, 10);
	// if (*endptr != '\0') {return -1;}
	int m = atoi(scrm);
	//Check if it is a valid value
	switch(m){
		case(1):
		case(2):
			mode = m;
			break;
		default:
			mode = 1;
	}
}

/**
 * Iniciate all the involved GUI elements related.
 */
bool init_gui(){
	bool done = true;
	init_i2c();
	init_leds();
	return done;
}