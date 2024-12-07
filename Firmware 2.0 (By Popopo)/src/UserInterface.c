/**
 * This module define and structure the GUI and controls of the
 * device throught the cartridge with the user.
 * Also it contains funtions that are part of the control module.
 *
 * @Author: Dr. Gusman
 * @Author: Modified by Popopo
 * @Version: 1.4.2
 * @date: 05/12/2024
 */

#include <string.h>
#include <stdio.h>
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
#define IS_CART_OUT() gpio_get(PIN_UI_DETECT)				//Detect Cartridge disconected. TRUE if disconected, FALSE in other case
#define BTN_PRESSED(BUTTON) (!gpio_get(BUTTON))				//Detect Button pressed. TRUE if pressed, FALSE in other case

USER_INTERFACE_STATE uiState = IDLE;
USER_INTERFACE_STATE uiNextState;

uint64_t delayEnd;
bool FirstBoot = true;										//Flag to detect if the MDP is booted by first time or not. That is important to define the behaviour.

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
        case MTU_MD_WRITTING: //TODO: Alternative point that could be a nice place to direct writting function
            LED_ON(PIN_LED_WRITE);
            LED_OFF(PIN_LED_READ);
            break;
        case MTU_BUFFERSET_READ:
            process_md_read(evt->arg);
            break;
        case MTU_BUFFERSET_WRITTEN: //TODO: Here could be a nice place to direct writting function
            process_md_write(evt->arg);
            break;
    }
}

/**
 * This function go to the next file on the FS that is valid to browse.
 * It means, directories, MDV and MDP images.
 * For that purpose it moves the pointer to the next FS entry, ignoring the non-selectable files.
 * @return FRESULT res with the result of the operation.
 */
FRESULT nextEntry(){
	FRESULT res = nextFSEntry();
	//Firsly check if there is a entry and if it is not null (what means there is not another file).
	if( res == FR_OK && fno.fname[0] == 0) {
		//Not valid one, it is null reference due to last file in the table.
		//So do nothing.
		showMSG(NO_MORE_FILES);
	}else if (res == FR_OK && isSelectable()){
		//If it is an valid file then show the name too.
		show_file_name(fno.fname,IN_FOLDER);
	}
	
	return res;
}

/**
 * Debounce a button pressed.
 * It gives some sleeping process time to ignore extra holding time when pressing a button.
 * When the button is the button 'NEXT' instead of debounce completealy, it adds some latency
 *  and the behavior of read the new FS entry. Allowing a softer file browsing.
 * @param button The button to apply debounce.
 */
void debounce_button(uint button) {
	FRESULT res;
    while(BTN_PRESSED(button)){ 
		sleep_ms(20); 
		//When the button is 'Next', it skip the entry and goes to the next one after a holding time (retard).
		if(button == PIN_BTN_NEXT ){
			sleep_ms(200);
			//If it continues pressed then read the next FS entry.
			if(BTN_PRESSED(button) && res != FR_NO_FILE) {
				//Just in case there are not more files, ignore next reading till debounce it.
				res = nextEntry();
			}
		} 
	}
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

/**
 * The function checks if the delay time was reached.
 * While it awaits, make the power led flash continuously.
 * @return TRUE if the time was reached. Otherwise FALSE.
 */
bool check_delay(){
	bool done = false;
    uint64_t currentTime = time_us_64();
    if(currentTime >= delayEnd)	{
		uiState = uiNextState;
		done = true;
	}
	return done;
}

//Process the user interface state machine
void process_user_interface(){
	//Firstly check out if Cartridge is not inserted (OUT).
	if(IS_CART_OUT()) uiState = IDLE;
	//States of the machine
    switch(uiState){
        case IDLE:
            if(!IS_CART_OUT()) {program_delay(2000, INIT_SCREEN);}
			else {
				sleep_ms(500);
				gpio_put(PIN_LED_PWR, !gpio_get(PIN_LED_PWR));
			}
			break;
        case DELAY:
			if(check_delay()) LED_ON(PIN_LED_PWR);
		    break;
        case INIT_SCREEN:												//Initialize the display and check if mSD is inserted.	
            if(init_screen()){
				showMSG(WELCOME);
				program_delay(2000, WAITING_SD_CARD);
				if(crt_type == NONE) memset(currentPath, 0, PATH_BUFFER_SIZE);
			} 
            break;
        case WAITING_SD_CARD:
			if(mountFS() == FR_OK) {   									//Checks if the FS is mounted.
				if(crt_type == NONE) uiState = OPEN_FOLDER;				//Is a cartridge formated inserted?, then open the directory (root or sub-directory)
				else {uiState = CARTRIDGE_READY;}
			}else {showMSG(SD_WAIT);}
            break;
        case OPEN_FOLDER:
			if( openDIR() != FR_OK) {
				showMSG(FOLDER_ERR_OPEN);
				uiState = WAITING_SD_CARD;
			} else if (FirstBoot) {
				loadDefault();  										//Try to load default image.
				FirstBoot = false;
			} else {
				firstFolderEntry = true;
				uiState = READ_FOLDER_ENTRY;
			} 
            break;
        case READ_FOLDER_ENTRY:
			if(IS_CART_OUT()){
				uiState = IDLE;
			} else if (nextEntry() != FR_OK){
				showMSG(FOLDER_ERR_READ);
				uiState = WAITING_SD_CARD;
			} else if(fno.fname[0] == 0) {
				if(firstFolderEntry) {
					showMSG(FOLDER_EPTY);
					uiState = SELECT_FILE;
				} else { uiState = OPEN_FOLDER; }
			} else if(isSelectable()){
				firstFolderEntry = false;
				uiState = SELECT_FILE;
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
			//Default initial case.
			crt_type = getFSType();
			uiState = FILE_LOAD;
			switch(crt_type){
				case MDV:
					showMSG(LDING_MDV);
					break;
				case MPD:
					showMSG(LDING_MDP);
					break;
				default:
					showMSG(CART_FORMAT_UNK);
					show_file_name(fno.fname,IN_FOLDER);
					uiState = SELECT_FILE;
					break;
			}
            break;
        case FILE_LOAD:
			if(!loadMDx()) {
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
				//Finally shows messages for save result & cart ready (whatever the result was)
				if(saveMDx()){ showMSG(CART_SAVED);}
				else { showMSG(CART_ERR_SAVING);}
			} else if(BTN_PRESSED(PIN_BTN_NEXT)){		//Now if pressed, it shows the selected file.
				show_file_name(fno.fname,IN_FOLDER);
				sleep_ms(1100);
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
	fileName = spliter((char*)ctext,"FILE");
	//Copy again the buffer & Find value for operator SCRM (Screen mode)
	strcpy(ctext,(char *)buffData);
    scrm = spliter((char*)ctext,"SCRM");
	setSCRM(scrm);
	//Try to load the file if it is the first load
	//TODO: pasar esto a IO_Cart
	//done = (FirstBoot == true)? autoLoadFile(fileName) : false;
	if(!isFilePresent(fileName)) {return false;}
    //Start searching.
    while(!done){
        if(nextFSEntry() != FR_OK || fno.fname[0] == 0){break;}
        //Comparing names of file pointed in FAT table and file name to load.
		if(strcmp(fno.fname,fileName) == 0){
			uiState = FILE_SELECTED;
            done = true;
        }
    }

	//TODO: Esta parte, adelgazarla. No es necesaria tan grande. Basta de mensajes tan redundantes.
	//Status & event setting for loaded file (selected file).
	if(done) {
		showMSG(LDING_DEFAULT);
		show_file_name(fno.fname,IN_FOLDER);
		sleep_ms(1500);
	}else {
		//Otherwise there is a mistake in CONFIG.CFG file.
		showMSG(ERR_CFG);
		uiState = OPEN_FOLDER;
	}

	return done;
}

/**
 * It initializes each button and its purpose is to save some 
 * memory, by set them upp with a default initial value.
 */
void setUpButton(uint btn){
	gpio_init(btn);
	gpio_set_dir(btn, false);
	gpio_pull_up(btn);
}

/**
 * Init the buttons.
 * It's use the auxiliar function setUpButton.
 */
void init_buttons(){
	setUpButton(PIN_BTN_BACK);
	setUpButton(PIN_BTN_NEXT);
	setUpButton(PIN_BTN_SELECT);
	setUpButton(PIN_UI_DETECT);
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