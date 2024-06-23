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
 * Shows the current file name on the screen.
 * The function will take the name and split it if needed.
 * Also it indicate if the file is a directory or an archive.
 * @param fname Name of the file/directory
 * @param IN_FOLDER It indicates if the pointer is aiming to a folder or not.
 */
void show_file_name(const char* fname, bool IN_FOLDER){
	char auxName[12];														//Temporal string to host the name and manipulate it.
	char type[3];
	int sz = strlen(fname);
	sprintf(type, "> %s", &fname[sz-3]);
	char *head = IN_FOLDER ? "# Dir" : type;
    CLR_SCR();
    PRINT_STR(head, 0, 0, 1);
	RENDER_SCR();
	//Check if the name is too long for one line.
    if(mode == 1 && strlen(fname) > 11) {									//If name with extension is longer than 11 chars, split it in two lines
        memset(auxName, 0, 12);
        memcpy(auxName, fname, 8);											//Copy the name into the temporal string.
        PRINT_STR(auxName, 0, 2, mode);										//Print the name at 2nd line.
        sprintf(auxName, "       %s", &fname[8]);							//Compose the last line overwritting with spaces the name and leaving the extension.
        PRINT_STR(auxName, 0, 3, mode);										//Print the extension at 3rd line of the screen.
    } else if(mode == 1) {PRINT_STR(fname, 0, 2, mode);						//Otherwise it is printed compleately in line number 2 (screen).
	} else {
		printMSGScroll(head,fname,140);
		CLR_SCR();
		PRINT_STR(head,0,0,mode);
		PRINT_STR(fname,0,2,mode);
	}

    RENDER_SCR();
}

/**
 * Shows the most common messages, after cleaning the screen.
 * @param MSG_TYPE any valid value from it.
 * @param ms Delay hold time (ms) the message before give back the control.
 */
void showMSG(MSG_TYPE m,int ms){
	CLR_SCR();

	switch(m){
		case WELCOME:
			PRINT_STR(" MicroPico ", 0, 0, 1);
			PRINT_STR("   Drive   ", 0, 1, 1);
			PRINT_STR("1.3", 3/mode, 2, 2);								//Development version RC1.3 16:43H 20/MARCH/2024
			break;
		case SD_WAIT:
			PRINT_STR("No SD", 0, 0, 2);
			PRINT_STR("In", 3/mode, 2, 2);
			break;
		case CART_RDY:
			PRINT_STR("Cartridge", 0, 0, mode);
			PRINT_STR("Ready", 0, 2, mode);
			break;
		case FOLDER_ERR_OPEN:											//Here when there was an error opening the folder.
			PRINT_STR("Error", 0, 0, mode);
			PRINT_STR("opening", 0, 2, mode);
			PRINT_STR("folder.", 0, 3, mode);
			break;
		case FOLDER_ERR_READ:											//Here when there was an error reading the folder.
			PRINT_STR("Error", 0, 0, mode);
			PRINT_STR("reading", 0, 2, mode);
			PRINT_STR("folder.", 0, 3, mode);
			break;
		case FOLDER_EPTY:
			PRINT_STR("Empty", 0, 0, mode);
			PRINT_STR("folder", 0, 2, mode);
			break;
		case CART_FORMAT_UNK:
			PRINT_STR("Unknown", 0, 1, mode);
			PRINT_STR("cartridge", 0, 2, mode);
			PRINT_STR("format.", 0, 3, mode);
			break;
		case CART_ERR_LDING:
			PRINT_STR("Error", 0, 0, mode);
			PRINT_STR("loading", 0, 2, mode);
			PRINT_STR("cartridge.", 0, 3, mode);
			break;
		case CART_ERR_SAVING:
			PRINT_STR("Error", 0, 0, mode);
			PRINT_STR("saving", 0, 2, mode);
			PRINT_STR("cartridge. ", 0, 3, mode);
			break;
		case CART_SAVING:
			PRINT_STR("Saving", 0, 1, mode);
			PRINT_STR("cartridge..", 0, 2, mode);
			break;
		case CART_SAVED:
			PRINT_STR("Cartridge", 0, 1, mode);
			PRINT_STR("saved", 0, 2, mode);
			break;
		case LDING_MDV:
			PRINT_STR("Loading MDV", 0, 1, mode);
			PRINT_STR("cartridge..", 0, 2, mode);
			break;
		case LDING_MDP:
			PRINT_STR("Loading MPD", 0, 1, mode);
			PRINT_STR("cartridge..", 0, 2, mode);
			break;
		case LDING_DEFAULT: 
			printMSGScroll("","...Default file,",140);
			break;
	}

	RENDER_SCR();
	sleep_ms(ms);
}

/**
 * It prints on the display a double message, separate by lines and holding it for a defined time.
 * @param msg Message to show in the first line.
 * @param value Second message to show, it will be displayed in the second line. Usually a value or resault that need to be shown.
 * @param time Time in milliseconds to hold the message on the screen before return the control.
*/
void printMSG(const char* msg1, const char* msg2, int time){
	CLR_SCR();
	PRINT_STR(msg1,0,0,mode);
	PRINT_STR(msg2,0,2,mode);
	RENDER_SCR();
	sleep_ms(time);
}

//TODO: Function for testing purpose. Delete when no need
void printNumber(const char* msg, int number, int time){
	CLR_SCR();
	char scount[12];
	sprintf(scount, "%d", number);					// convert the count number into a string
	PRINT_STR(msg,0,1,mode);
	PRINT_STR(scount,0,2,mode);
	RENDER_SCR();
	sleep_ms(time);
}

/**
 * It shows a message with lateral scroll. Specially conveniente for big fonts.
 * @param msg Message to print.
 * @param time A good time set is 500ms but users can define others.
*/
void printMSGScroll(const char*msg1, const char*msg2, const int time){
	int size = strlen(msg2);
	for(int i = 0;i<size; i++){
		printMSG(msg1,(char*)&msg2[i],time);
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
    gpio_init(PIN_LED_ON);
    gpio_init(PIN_LED_SELECT);
    gpio_init(PIN_LED_READ);
    gpio_init(PIN_LED_WRITE);

    gpio_set_dir(PIN_LED_ON, true);
    gpio_set_dir(PIN_LED_SELECT, true);
    gpio_set_dir(PIN_LED_READ, true);
    gpio_set_dir(PIN_LED_WRITE, true);

    gpio_put(PIN_LED_ON, 1);
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
	//TODO Decidir si usamos strtol o atoi. El primero es más robusto, pero requiere mayor código.
	//Converting it into int
	//char *endptr;
	//int m = strtol(scrm, &endptr, 10);
	// if (*endptr != '\0') {return -1;}
	int m = atoi(scrm);
	//Check if it is a valid value
	switch(m){
		case(1):
		case(2):
		case(3):
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