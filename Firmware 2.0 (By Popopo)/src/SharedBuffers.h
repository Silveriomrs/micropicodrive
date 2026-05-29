/**
 * Modified by Popopo
 */

#ifndef __SHAREDBUFFERS__
#define __SHAREDBUFFERS__

#include "pico/stdlib.h"
#include "EventMachine.h"
//for lock access simult to MDV image.
#include <stdbool.h>
#include <stdint.h>
//

#define CART_SIZE 160140

#define HEADER_BUFFER_SIZE 128
#define SECTOR_BUFFER_SIZE 2980

#define HEADER_TRACK_DATA_SIZE 8
#define SECTOR_TRACK_DATA_SIZE 306

#define PREAMBLE_ZERO_BITS (5 * 8)
#define PREAMBLE_ONE_BITS 8

#define PREAMBLE_ZERO_BYTES 10
#define PREAMBLE_ONE_BYTES 2

//Locker for access to MDVs in RAM.
typedef enum {
    MDV_UNLOCKED = 0,
    MDV_LOCKED = 1
} mdv_lock_state_t;

extern volatile mdv_lock_state_t mdv_lock_state;        //Blocked or not
extern volatile bool mdv_IO_active;                     //Active IO to mdv or not

//

extern uint8_t cartridge_image[CART_SIZE];

extern uint8_t header_1_track_1[HEADER_BUFFER_SIZE];
extern uint8_t header_1_track_2[HEADER_BUFFER_SIZE];

extern uint8_t sector_1_track_1[SECTOR_BUFFER_SIZE];
extern uint8_t sector_1_track_2[SECTOR_BUFFER_SIZE];

extern uint8_t bufferset_1_sector_number;

extern uint8_t header_2_track_1[HEADER_BUFFER_SIZE];
extern uint8_t header_2_track_2[HEADER_BUFFER_SIZE];

extern uint8_t sector_2_track_1[SECTOR_BUFFER_SIZE];
extern uint8_t sector_2_track_2[SECTOR_BUFFER_SIZE];

extern uint8_t bufferset_2_sector_number;

extern evtmachine_t mdToUiEventQueue;
extern evtmachine_t uiToMdEventQueue;

#endif