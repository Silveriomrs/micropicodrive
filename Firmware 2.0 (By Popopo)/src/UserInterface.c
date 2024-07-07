/**
 * This module define and structure the GUI and controls of the
 * device throught the cartridge with the user.
 * Also it contains funtions that are part of the control module.
 *
 * @Author: Dr. Gusman
 * @Author: Modified by Popopo
 * @Version: 1.2.1
 * @date: 04/03/2024
 */

#include <string.h>
#include <stdio.h>
//Si quito no se queja
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "EventMachine.h"
//
#include "UserInterface.h"
#include "SharedBuffers.h"
#include "SharedEvents.h"
#include "View.h"
#include "IO_Cart.h"
//

#define LED_ON(LED) gpio_put(LED, true)
#define LED_OFF(LED) gpio_put(LED, false)
#define CART_OUT() gpio_get(PIN_UI_DETECT)								//Detect Cartridge disconected. TRUE if disconected, FALSE in other case
#define BTN_PRESSED(BUTTON) (!gpio_get(BUTTON))							//Detect Button pressed. TRUE if pressed, FALSE in other case

USER_INTERFACE_STATE uiState = IDLE;
USER_INTERFACE_STATE uiNextState;

uint64_t delayEnd;

// bool mdInUse = false;
// bool firstFolderEntry = false;

//Process events from the MD control
void process_md_to_ui_event(void* event) {
    mtuevent_t* evt = (mtuevent_t*)event;

    switch(evt->event) {
        case MTU_MD_DESELECTED:
            mdInUse = false;
            setInFormat(false);
            LED_OFF(PIN_LED_SELECT);
            LED_OFF(PIN_LED_READ);
            LED_OFF(PIN_LED_WRITE);
            break;
        case MTU_MD_SELECTED:
            mdInUse = true;
            LED_ON(PIN_LED_SELECT);
            break;
        case MTU_MD_READING:
            LED_ON(PIN_LED_READ);
            LED_OFF(PIN_LED_WRITE);
            break;
        case MTU_MD_WRITTING:
            LED_ON(PIN_LED_WRITE);
            LED_OFF(PIN_LED_READ);
            break;
        case MTU_BUFFERSET_READ:
            process_md_read(evt->arg);
            break;
        case MTU_BUFFERSET_WRITTEN:
            process_md_write(evt->arg);
            break;
    }
}

//Debounce a button press
void debounce_button(uint button) {
    while(BTN_PRESSED(button)){ sleep_ms(20); }
    sleep_ms(200);
}

//Check if cancel was requested
void check_cancel(){
    if(BTN_PRESSED(PIN_BTN_BACK)) {
        debounce_button(PIN_BTN_BACK);
        rewind_path();
        uiState = OPEN_FOLDER;
        crt_type = NONE;
        utmevent_t removeEvt;
        setCurrectSector(0);
        removeEvt.event = UTM_CARTRIDGE_REMOVED;
        event_push(&uiToMdEventQueue, &removeEvt);
    }
}

void program_delay(uint64_t ms_delay, USER_INTERFACE_STATE nextState){
    delayEnd = time_us_64() + (ms_delay * 1000);
    uiNextState = nextState;
    uiState = DELAY;
}

void check_delay(){
    uint64_t currentTime = time_us_64();
    if(currentTime >= delayEnd) uiState = uiNextState;
}

//Process the user interface state machine
void process_user_interface(){
	//Firstly check out if Cartridge is not inserted (OUT).
	if(CART_OUT()) uiState = IDLE;
	//States of the machine
    switch(uiState){
        case IDLE:
            if(!CART_OUT()) {program_delay(2000, INIT_SCREEN);} break;
        case DELAY: check_delay(); break;
        case INIT_SCREEN:												//Initialize the display and check if mSD is inserted.
            if(init_screen()){showMSG(WELCOME);}
			if(crt_type == NONE) memset(currentPath, 0, PATH_BUFFER_SIZE);
			program_delay(2000, WAITING_SD_CARD);
            break;
        case WAITING_SD_CARD:
			if(mountFS() == FR_OK) {   									//Checks if the FS is mounted.
				if(crt_type == NONE) uiState = OPEN_FOLDER;				//Is a cartridge formated inserted?, then open the directory (root or sub-directory)
				else {uiState = CARTRIDGE_READY;}
				//Try to load default image.
				loadDefault();
			}else {showMSG(SD_WAIT);}
            break;
        case OPEN_FOLDER:
			if( openDIR() != FR_OK) {
				showMSG(FOLDER_ERR_OPEN);
				uiState = WAITING_SD_CARD;
			} else {
				firstFolderEntry = true;
				uiState = READ_FOLDER_ENTRY;
			}
            break;
        case READ_FOLDER_ENTRY:
			if(nextFSEntry()) {
				showMSG(FOLDER_ERR_READ);
				uiState = WAITING_SD_CARD;
			} else {
				if(fno.fname[0] == 0) {												//TODO: aquí hay una parte de acoplamiento.
					if(firstFolderEntry) {
						showMSG(FOLDER_EPTY);
						while(!BTN_PRESSED(PIN_BTN_BACK)) {;}
						debounce_button(PIN_BTN_BACK);
						rewind_path();
						firstFolderEntry = false;
						show_file_name(fno.fname,IN_FOLDER);
						uiState = SELECT_FILE;
					}else { uiState = OPEN_FOLDER; }
				} else {
					firstFolderEntry = false;
					show_file_name(fno.fname,IN_FOLDER);
					uiState = SELECT_FILE;
				}
			}
            break;
        case SELECT_FILE:
			if(BTN_PRESSED(PIN_BTN_SELECT) && !firstFolderEntry) {
				debounce_button(PIN_BTN_SELECT);
				if(IN_FOLDER) {
					updatePath();
					uiState = OPEN_FOLDER;
				} else uiState = FILE_SELECTED;
			} else if(BTN_PRESSED(PIN_BTN_NEXT) && !firstFolderEntry) {
				debounce_button(PIN_BTN_NEXT);
				uiState = READ_FOLDER_ENTRY;
			} else if(BTN_PRESSED(PIN_BTN_BACK)) {
				debounce_button(PIN_BTN_BACK);
				rewind_path();
				uiState = OPEN_FOLDER;
			}
            break;
        case FILE_SELECTED:
			switch(fno.fsize){
				case CART_MDV_SIZE:
					showMSG(LDING_MDV);
					crt_type = MDV;
					uiState = FILE_LOAD;
					break;
				case CART_MPD_SIZE:
					showMSG(LDING_MDP);
					crt_type = MPD;
					uiState = FILE_LOAD;
					break;
				default:
					showMSG(CART_FORMAT_UNK);
					show_file_name(fno.fname,IN_FOLDER);
					uiState = SELECT_FILE;
					break;
			}
            break;
        case FILE_LOAD:
			updatePath();
			bool res = false;
			switch(crt_type) {
				case MDV: res = load_mdv_cartridge(currentPath); break;
				case MPD: res = load_mpd_cartridge(currentPath); break;
                default:
                    break;
			}
				   
			if(!res) {
				showMSG(CART_ERR_LDING);
				rewind_path();
				show_file_name(fno.fname,IN_FOLDER);
				crt_type = NONE;
				uiState = SELECT_FILE;
			} else {
				uiState = CARTRIDGE_READY;
				utmevent_t insertEvt;
				insertEvt.event = UTM_CARTRIDGE_INSERTED;
				event_push(&uiToMdEventQueue, &insertEvt);
			}
            break;
        case CARTRIDGE_READY:
			showMSG(CART_RDY);
			if(BTN_PRESSED(PIN_BTN_BACK)) {
				debounce_button(PIN_BTN_BACK);
				rewind_path();
				uiState = OPEN_FOLDER;
				crt_type = NONE;
				utmevent_t removeEvt;
				removeEvt.event = UTM_CARTRIDGE_REMOVED;
				event_push(&uiToMdEventQueue, &removeEvt);
			} else if (BTN_PRESSED(PIN_BTN_SELECT)) {
				showMSG(CART_SAVING);
				bool res = false;
				switch(crt_type) {
					case MDV: res = save_mdv_cartridge(currentPath); break;
					//case IMG: res = save_img_cartridge(); break;
					case MPD: res = save_mpd_cartridge(currentPath); break;
                    default:
                        break;
				}
				//Finally shows messages for save result & cart ready (whatever the result was)
				if(res){ showMSG(CART_SAVED);}
				else { showMSG(CART_ERR_SAVING);}
				showMSG(CART_RDY);
			}
            break;
    }
}

/**
 * The function returns the value asociated to an operator.
 *  Keep in mind, it modify the original text.
 * @param text Text to explore and slide it into tokes.
 * @param filter Filter that is the Operator that whose value wants to get back.
 * 		{FILE,SCRM} FILE: Default file to load. SCRM: Screen Mode to use.
 * @return token with the result requested. Otherwise Null.
*/
char* spliter(char *text, const char *filter){
	const char outer_delimiters[] = "\n";       //First delimiter is the Return Carry
	const char inner_delimiters[] = "=";        //The separator between filter and value
    const char comment = '#';                   //Comment filter.

	char* token;                                //Value to return
	char* outer_saveptr = NULL;                 //Pointer after first filter
	char* inner_saveptr = NULL;                 //Pointer after equal filter
    //First splitter applied using first delimiter
	token = strtok_r(text, outer_delimiters, &outer_saveptr);

	while (token != NULL) {
        //Filter comments
        if(token[0] != comment){
            //If it is not a comment, applied second delimiter to split the inner piece of text
            char* inner_token = strtok_r(token, inner_delimiters, &inner_saveptr);
            //Select only the Operator seeked (filter).
            if(strcmp(inner_token,filter) == 0){
                //If it is the right one, get the value, that is the second value.
                while (inner_token != NULL) {
                    //Get next item that is the seeked value.
                    inner_token = strtok_r(NULL, inner_delimiters, &inner_saveptr);
                    return inner_token;
                }
            }
        }
		token = strtok_r(NULL, outer_delimiters, &outer_saveptr);
	}
    //if not found, it returns null.
	return token;
}

/**
 * The function loads default file and return the control to the control function.
 * @return True if the operation was successful. Otherwise it returns false.
*/
bool loadDefault(){
	//Open root directory
	if(openDIR() != FR_OK) {return false;}
	//Direct checking about Config file on the root directory.
	if(!isFilePresent("CONFIG.CFG")) {return false;}
	
	//For filename
	UINT block = 512;					// Number of items to read, the whole sector. That's fine even with EOF
	UINT br = 0;						// Counter of read items.
	BYTE buffData[block];				// Buffer to store read datas.
	//Read the file.
	bool done = loadFile("CONFIG.CFG",buffData,block,&br);

	// Vars to hold data & info
	char* fileName;
	char* scrm;                
	//Create a copy of original text, for second Operator.
	char ctext[strlen((char *)buffData)+1];
    strcpy(ctext,(char *)buffData);

    //Find value for operator FILE (Default file).
    fileName = spliter((char*)buffData,"FILE");
    //Find value for operator SCRM (Screen mode)
    scrm = spliter((char*)ctext,"SCRM");
	//setSCRM(scrm);
	setSCRM("2");
	//Try to load the file.
	done = autoLoadFile(fileName);
	//Sets state machine as poinng a file in a non empty directory
	firstFolderEntry = false;
	
	//Status & event setting for loaded file (selected file).
	if(done) {
		uiState = CARTRIDGE_READY;
		utmevent_t insertEvt;
		insertEvt.event = UTM_CARTRIDGE_INSERTED;
		event_push(&uiToMdEventQueue, &insertEvt);
		showMSG(LDING_DEFAULT);
		show_file_name(fno.fname,IN_FOLDER);
		sleep_ms(3000);
	}else {
		//Otherwise there is a mistake in CONFIG.CFG file.
		showMSG(ERR_CFG);
		uiState = OPEN_FOLDER;
	}

	return done;
}


//Initialize UI buttons
void init_buttons(){
    gpio_init(PIN_BTN_BACK);
    gpio_init(PIN_BTN_NEXT);
    gpio_init(PIN_BTN_SELECT);
    gpio_init(PIN_UI_DETECT);

    gpio_set_dir(PIN_BTN_BACK, false);
    gpio_set_dir(PIN_BTN_NEXT, false);
    gpio_set_dir(PIN_BTN_SELECT, false);
    gpio_set_dir(PIN_UI_DETECT, false);

    gpio_pull_up(PIN_BTN_BACK);
    gpio_pull_up(PIN_BTN_NEXT);
    gpio_pull_up(PIN_BTN_SELECT);
    gpio_pull_up(PIN_UI_DETECT);
}

//Main user interface loop
void RunUserInterface(){
    event_machine_init(&mdToUiEventQueue, &process_md_to_ui_event, sizeof(mtuevent_t), 8);
    mtuevent_t mtuevtBuffer;
	//initiate GUI & Controls
	init_gui();
    init_buttons();

    while(true){
        event_process_queue(&mdToUiEventQueue, &mtuevtBuffer, 16);
        if(!mdInUse) process_user_interface();
        else check_cancel();
    }
}

//TODO: En un par de estados, es ineficiente estar rescribiendo el mismo mensaje en pantalla hasta evento de cambio de estado.
