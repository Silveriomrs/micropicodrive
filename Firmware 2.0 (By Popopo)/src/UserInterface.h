
#ifndef __USERINTERFACE__
#define __USERINTERFACE__


//Buttoms Pins inputs
#define PIN_BTN_BACK 12
#define PIN_BTN_NEXT 13
#define PIN_BTN_SELECT 14
//GND bucle detection
#define PIN_UI_DETECT 15

//#define PATH_BUFFER_SIZE 300

//States defined
typedef enum{
    IDLE,
    DELAY,
    INIT_SCREEN,
    WAITING_SD_CARD,
    OPEN_FOLDER,
    READ_FOLDER_ENTRY,
    SELECT_FILE,
    FILE_SELECTED,
    FILE_LOAD,
    CARTRIDGE_READY
} USER_INTERFACE_STATE;


bool loadDefault();
void RunUserInterface();

#endif