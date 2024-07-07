/**
* This module manage the IO between the RBPi Pico and the cartridge.
* From original work of Dr. Gusman.
* @Author: Silverio MRS (Popopo)
* @Version: 1.0
*/

#ifndef _IO_Cart_
#define _IO_Cart_
#include "pff/pff.h"
#include <string.h>
#include <stdio.h>

#define CART_MDV_SIZE 174930
#define CART_MPD_SIZE 160140

#define MDV_PREAMBLE_SIZE 12
#define MDV_HEADER_SIZE 16
#define MDV_DATA_SIZE 646
#define MDV_PAD_SIZE 34
#define MDV_SECTOR_SIZE 686

#define MPD_HEADER_SIZE 16
#define MPD_DATA_SIZE 612

#define CARTRIDGE_HEADER_SIZE 16
#define CARTRIDGE_DATA_SIZE 612
#define CARTRIDGE_SECTOR_SIZE 628
#define CARTRIDGE_SECTOR_COUNT 255

#define PATH_BUFFER_SIZE 300
#define IN_FOLDER (fno.fattrib & AM_DIR)

/**
 * Define the format type of the image or file.
 * The types are {NONE,MDV,MPD}
*/
typedef enum{
    NONE,
    MDV,
    MPD
} CARTRIDGE_FORMAT;

extern FATFS fatfs;
extern DIR dir;
extern FILINFO fno;      // Struct with the attrs of the file.
extern CARTRIDGE_FORMAT crt_type;
extern bool mdInUse;
extern bool firstFolderEntry;
extern char currentPath[PATH_BUFFER_SIZE];

typedef struct __attribute__((__packed__)) SECTOR_HEADER{
    uint8_t HeaderData[14];
    uint16_t Checksum;
} SECTOR_HEADER_t;

typedef struct __attribute__((__packed__)) SECTOR_RECORD{
    uint8_t HeaderData[2];
    uint16_t HeaderChecksum;
    uint8_t FilePreamble[8];
    uint8_t Data[512];
    uint16_t DataChecksum;
    uint8_t ExtraBytes[84];
    uint16_t ExtraBytesChecksum;
} SECTOR_RECORD_t;

typedef struct __attribute__((__packed__)) SECTOR{
    SECTOR_HEADER_t Header;
    SECTOR_RECORD_t Record;
} SECTOR_t;

void rewind_path();
void updatePath();
FRESULT nextFSEntry();
FRESULT openDIR();
void setInFormat(bool v);
FRESULT mountFS();
void setCurrectSector(uint8_t v);
void write_buffer_set_pair(uint8_t* source, uint8_t* track1Buffer, uint8_t* track2Buffer, bool isHeader);
void write_buffer_set(uint8_t setNumber, uint8_t sector);
int8_t find_preamble_end(uint8_t* buffer);
void read_buffer_set_pair(uint8_t* destination, uint8_t* track1Buffer, uint8_t* track2Buffer, bool isHeader);
void read_buffer_set(uint8_t setNumber);
void process_md_read(uint8_t bufferSet);
void process_md_write(uint8_t bufferSet);
void fix_cartridge_checksums();
bool save_mdv_cartridge(char const *file);
bool save_mpd_cartridge(char const *file);
void loadMDx();
bool load_mdv_cartridge(char const *file);
bool load_mpd_cartridge(char const *file);
bool isFilePresent(char const *file);
bool loadFile(char const *file, BYTE *buffDataIn, const UINT block, UINT *br);
bool autoLoadFile(char const *fileName);
bool isSelectable();

#endif