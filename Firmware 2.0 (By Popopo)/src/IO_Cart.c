/**
 * Implementation of the module for R/W operations between the RBP Pico and the cartridge.
 * This module is in charge of data flows and control over the cartridge and image format with the QL
 * From original work of Dr. Gusman.
 * @Author: Silverio MRS (Popopo)
 * @Version: 1.2
 */


#include "IO_Cart.h"
#include "SharedBuffers.h"
//Añadidos por añadir, porque realmente no parece que hagan nada
#include "SharedEvents.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
//
#define CONCAT(DEST, SOURCE) sprintf(&DEST[strlen(DEST)],"/%s", SOURCE)

FATFS fatfs;
DIR dir;
FILINFO fno;

CARTRIDGE_FORMAT crt_type = NONE;
uint8_t currentSector = 0;
bool inFormat = false;
int skip = 0;
bool mdInUse = false;
/** FirstFolderEntry means parents directory '.'. So not a file */
bool firstFolderEntry = false;
char currentPath[PATH_BUFFER_SIZE];
// memset(currentPath, 0, PATH_BUFFER_SIZE);

//Remove from the path buffer the last entry
void rewind_path(){
    if(strlen(currentPath) == 0) return;
    char* lastPos = strrchr(currentPath, '/');
    if(lastPos == NULL) return;
    memset(lastPos, 0, (size_t)(PATH_BUFFER_SIZE - (lastPos - currentPath)));
}

/**
 * Update the path that the pointer is aiming to in the FAT table.
 *  Internally it add to the variable currentPath the name stored into fno.fname.
*/
void updatePath(){CONCAT(currentPath, fno.fname);}

/**
 * The function move pointer into the FAT FS table to the next entry.
 *  It may be a file or directory, returning the result.
 * @return FRESULT one of the option of the enum.
 * @see FRESULT
*/
FRESULT nextFSEntry() { return pf_readdir(&dir, &fno);}

/**
 * Open the directory selected in the context and returns the result of operation.
 * @return FRESULT enum value.
 * @see FRESULT
*/
FRESULT openDIR(){return pf_opendir(&dir, currentPath);}

/**
 * Sets inFormat value.
 * @param TRUE or FALSE.
*/
void setInFormat(bool v){inFormat = v;}

/**
 * Mount the FS,
 *  this function update FATS.
 * @return The resault of the operation.
 * @see {FRESULT}
*/
FRESULT mountFS() {return pf_mount(&fatfs);}

/**
 * Sets the current sector.
 * @param uint8_t value.
*/
void setCurrectSector(uint8_t v){currentSector = v;}

//Writes a pair of buffers of a buffer set (a buffer set are four buffers, two header ones and two sector ones)
void write_buffer_set_pair(uint8_t* source, uint8_t* track1Buffer, uint8_t* track2Buffer, bool isHeader) {
    track2Buffer += 4; //we skip four bits on buffer 2 to respect the skewing done by the ULA

    for(int buc = 0; buc < PREAMBLE_ZERO_BITS; buc++) {
        track1Buffer[buc] = 0;
        track2Buffer[buc] = 0;
    } 

    track1Buffer += PREAMBLE_ZERO_BITS;
    track2Buffer += PREAMBLE_ZERO_BITS;

    for(int buc = 0; buc < PREAMBLE_ONE_BITS; buc++) {
        track1Buffer[buc] = 1;
        track2Buffer[buc] = 1;
    } 

    track1Buffer += PREAMBLE_ONE_BITS;
    track2Buffer += PREAMBLE_ONE_BITS;

    uint16_t copySize = isHeader ? HEADER_TRACK_DATA_SIZE : SECTOR_TRACK_DATA_SIZE;

    for(int buc = 0; buc < copySize; buc++) {
        uint8_t t1b = *source;
        source++;
        uint8_t t2b = *source;
        source++;

        for(int buc = 0; buc < 8; buc++) {
            *track1Buffer = (t1b >> buc) & 1;
            *track2Buffer = (t2b >> buc) & 1;
            track1Buffer++;
            track2Buffer++;
        }
    }
}

//Writes a buffer set with cartridge data
void write_buffer_set(uint8_t setNumber, uint8_t sector){
    if(setNumber == 0) {
        write_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * sector], header_1_track_1, header_1_track_2, true);
        write_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * sector + CARTRIDGE_HEADER_SIZE], sector_1_track_1, sector_1_track_2, false);
        bufferset_1_sector_number = sector;
    } else {
        write_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * sector], header_2_track_1, header_2_track_2, true);
        write_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * sector + CARTRIDGE_HEADER_SIZE], sector_2_track_1, sector_2_track_2, false);
        bufferset_2_sector_number = sector;
    }
}

//Find the end of a preamble from a track buffer
int8_t find_preamble_end(uint8_t* buffer){
    uint8_t zeroCount = 0;
    uint8_t oneCount = 0;

    //We need to find at least 16 zeros followed by 8 ones (0x00, 0x00, 0xFF)
    for(int buc = 0; buc < 100; buc++){
        if(buffer[buc] == 0) {
            if(oneCount != 0) //Do we came here form a one?
            {   //Reset everything
                zeroCount = 1;
                oneCount = 0;
            } else zeroCount++; //Increment count
        } else {
            if(zeroCount < 16) //Did we found a one before having eight zeros?
            {   //Reset everything
                zeroCount = 0;
                oneCount = 0;
            } else oneCount++; //Increment count
        }
        //Have we found the eight ones?
        if(oneCount == 8) return buc + 1;
    }

    //Error! We haven't found the gap end!!
    return -1;
}

//Reads a pair of buffers from a buffer set (a buffer set are four buffers, two header ones and two sector ones)
void read_buffer_set_pair(uint8_t* destination, uint8_t* track1Buffer, uint8_t* track2Buffer, bool isHeader){
    uint16_t track1Pos = find_preamble_end(track1Buffer);
    uint16_t track2Pos = find_preamble_end(track2Buffer);

    uint16_t size = isHeader ? HEADER_TRACK_DATA_SIZE : SECTOR_TRACK_DATA_SIZE;

    //Check if we're writting sector 255, if true then this is a format
    if(isHeader && !inFormat) {
        uint8_t sectorNumber = track2Buffer[track2Pos] |
            track2Buffer[track2Pos + 1] << 1 |
            track2Buffer[track2Pos + 2] << 2 |
            track2Buffer[track2Pos + 3] << 3 |
            track2Buffer[track2Pos + 4] << 4 |
            track2Buffer[track2Pos + 5] << 5 |
            track2Buffer[track2Pos + 6] << 6 |
            track2Buffer[track2Pos + 7] << 7;

        if(sectorNumber == 255) {
            inFormat = 1;
            skip = currentSector;
        }
    }

    for(uint16_t buc = 0; buc < size; buc++) {
        *destination = track1Buffer[track1Pos] |
            track1Buffer[track1Pos + 1] << 1 |
            track1Buffer[track1Pos + 2] << 2 |
            track1Buffer[track1Pos + 3] << 3 |
            track1Buffer[track1Pos + 4] << 4 |
            track1Buffer[track1Pos + 5] << 5 |
            track1Buffer[track1Pos + 6] << 6 |
            track1Buffer[track1Pos + 7] << 7;

        track1Pos += 8;
        destination++;

        *destination = track2Buffer[track2Pos] |
            track2Buffer[track2Pos + 1] << 1 |
            track2Buffer[track2Pos + 2] << 2 |
            track2Buffer[track2Pos + 3] << 3 |
            track2Buffer[track2Pos + 4] << 4 |
            track2Buffer[track2Pos + 5] << 5 |
            track2Buffer[track2Pos + 6] << 6 |
            track2Buffer[track2Pos + 7] << 7;

        track2Pos += 8;
        destination++;
    }
}

//Reads a buffer set to the cartridge buffer
void read_buffer_set(uint8_t setNumber){
    if(setNumber == 0){
        read_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * bufferset_1_sector_number], header_1_track_1, header_1_track_2, true);
        read_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * bufferset_1_sector_number + CARTRIDGE_HEADER_SIZE], sector_1_track_1, sector_1_track_2, false);
    } else {
        read_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * bufferset_2_sector_number], header_2_track_1, header_2_track_2, true);
        read_buffer_set_pair(&cartridge_image[CARTRIDGE_SECTOR_SIZE * bufferset_2_sector_number + CARTRIDGE_HEADER_SIZE], sector_2_track_1, sector_2_track_2, false);
    }
}

/**
 * Function to unify two fases of format process (reading & writting)
 * @param uint8_t bufferSet
*/
void process_format(uint8_t bufferSet){
    uint8_t secNum = cartridge_image[CARTRIDGE_SECTOR_SIZE * currentSector + 1];

    //If we are in the middle of a format and we're going to send sector 254, skip it to make Minerva happy...
    if(inFormat && secNum == 254){
        currentSector++;
        if(currentSector > 253) currentSector = 0;
    }

    write_buffer_set(bufferSet, currentSector);

    //If we are in the middle of a format and we're going to send sector 13, damage it to make Minerva happy...
    if(inFormat && secNum == 13) {
        cartridge_image[CARTRIDGE_SECTOR_SIZE * currentSector + 13] += 13;
        cartridge_image[CARTRIDGE_SECTOR_SIZE * currentSector + 128] += 13;
    }

    currentSector++;
    if(currentSector == 255) currentSector = 0;
}

//Process when a buffer set has been read by the ULA
void process_md_read(uint8_t bufferSet){ process_format(bufferSet);}

//Process when a buffer set has been written by the ULA
void process_md_write(uint8_t bufferSet){
    read_buffer_set(bufferSet);
    process_format(bufferSet);
}

void fix_cartridge_checksums(){
    SECTOR_t* sector = (SECTOR_t*)cartridge_image;

    for(int buc = 0; buc < 255; buc++) {
        uint16_t computedChecksum = 0;

        for(int hBuc = 0; hBuc < 14; hBuc++)
            computedChecksum += sector->Header.HeaderData[hBuc];

        computedChecksum += 0x0f0f;
        sector->Header.Checksum = computedChecksum;

        computedChecksum = 0;

        for(int hrBuc = 0; hrBuc < 2; hrBuc++)
            computedChecksum += sector->Record.HeaderData[hrBuc];

        computedChecksum += 0x0f0f;
        sector->Record.HeaderChecksum = computedChecksum;

        computedChecksum = 0;

        for(int hdBuc = 0; hdBuc < 512; hdBuc++)
            computedChecksum += sector->Record.Data[hdBuc];

        computedChecksum += 0x0f0f;
        sector->Record.DataChecksum = computedChecksum;

        for (int bExtra = 0; bExtra < 84; bExtra++)
                sector->Record.ExtraBytes[bExtra] = bExtra % 2 == 0 ? 0xAA : 0x55;

        //That is equivalent to force it anyway. Exchanging saving memory for time of processing.
        if (sector->Record.ExtraBytesChecksum != 0x3b19)
            sector->Record.ExtraBytesChecksum = 0x3b19;

        sector++;
    }
}

//Save the cartridge to a mdv image
bool save_mdv_cartridge(char const *file) {
    //if(pf_open(file) != FR_OK) return false; //TODO: Borrar una vez comprobado

    UINT writeSize;
    uint8_t tmpByte;
    int bufferPos = 0;

    uint8_t padBuffer[MDV_PAD_SIZE];
    memset(padBuffer, 'Z', MDV_PAD_SIZE);

    for(int buc = 0; buc < 255; buc++) {
        tmpByte = 0;

        for(int zeros = 0; zeros < PREAMBLE_ZERO_BYTES; zeros++) {
            if(pf_write(&tmpByte, 1, &writeSize) || (writeSize != 1) ) {
                pf_write(0, 0, &writeSize);
                return false;
            }
        }

        tmpByte = 0xff;

        for(int ones = 0; ones < PREAMBLE_ONE_BYTES; ones++) {
            if(pf_write(&tmpByte, 1, &writeSize) || (writeSize != 1) ) {
                pf_write(0, 0, &writeSize);
                return false;
            }
        }

        if(pf_write(&cartridge_image[bufferPos], CARTRIDGE_HEADER_SIZE, &writeSize) || 
        (writeSize != CARTRIDGE_HEADER_SIZE)) {
            pf_write(0, 0, &writeSize);
            return false;
        }

        bufferPos += CARTRIDGE_HEADER_SIZE;

        tmpByte = 0;

        for(int zeros = 0; zeros < PREAMBLE_ZERO_BYTES; zeros++) {
            if(pf_write(&tmpByte, 1, &writeSize)) {
                pf_write(0, 0, &writeSize);
                return false;
            }

            if(writeSize != 1) {
                pf_write(0, 0, &writeSize);
                return false;
            }
        }

        tmpByte = 0xff;

        for(int ones = 0; ones < PREAMBLE_ONE_BYTES; ones++) {
            if(pf_write(&tmpByte, 1, &writeSize)) {
                pf_write(0, 0, &writeSize);
                return false;
            }

            if(writeSize != 1) {
                pf_write(0, 0, &writeSize);
                return false;
            }
        }

        if(pf_write(&cartridge_image[bufferPos], CARTRIDGE_DATA_SIZE, &writeSize)) {
            pf_write(0, 0, &writeSize);
            return false;
        }

        if(writeSize != CARTRIDGE_DATA_SIZE) return false;

        bufferPos += CARTRIDGE_DATA_SIZE;

        if(pf_write(padBuffer, MDV_PAD_SIZE, &writeSize)) {
            pf_write(0, 0, &writeSize);
            return false;
        }else if(writeSize != MDV_PAD_SIZE) return false;
    }

    pf_write(0, 0, &writeSize);
    return true;
}

//Save the cartridge to a mpd image
bool save_mpd_cartridge(char const *file) {
    //Open the file and based on return code, finish the operation or continue
    //if(pf_open(file) != FR_OK) return false; //TODO: Borrar una vez comprobado

    UINT writeSize;

    if(pf_write(cartridge_image, CART_SIZE, &writeSize) || (writeSize != CART_SIZE)) {
        pf_write(0, 0, &writeSize);
        return false;
    }

    pf_write(0, 0, &writeSize);
    return true;
}


/**
 * Save the MDV image in the right format.
 * This function filter the image type (MDP or MDV) and
 * calls to the right sub-function to do the operation.
 * @return True if the operation was successful. False otherwise.
 */
bool saveMDx(){
    bool done = false;
    //Open the file and based on return code, finish the operation or continue
    if(pf_open(currentPath) != FR_OK) return false;

    switch(crt_type) {
        case MDV: done = save_mdv_cartridge(currentPath); break;
        //case IMG: res = save_img_cartridge(); break;
        case MPD: done = save_mpd_cartridge(currentPath); break;
        default:
            break;
    }

    return done;
}


/**
 * The function loads a valid image from the storage system.
 * It detects if the pointed file is a MDV or MDP image and loads calls to the right function to load it properly.
 * @return True if the operation was successful. False otherwise.
*/
bool loadMDx(){
    bool done = false;
    //Update the path to the file.
    updatePath();
    //Open the file and based on return code, finish the operation or continue
    if (pf_open(currentPath) != FR_OK) return false;
    //Load the file considering its type.
    switch(crt_type) {
        case MDV: done = load_mdv_cartridge(currentPath); break;
        case MPD: done = load_mpd_cartridge(currentPath); break;
        default:
            break;
	}

    //Check if loading was right.
    if(!done) return done;

    //Common part of the code.
    //Put the loaded image into the RPBPico
    fix_cartridge_checksums();
    write_buffer_set(0, 0);
    write_buffer_set(1, 1);
    setCurrectSector(2);

    return done;
}

/**
 * Load a MDV image to the cartridge buffer.
 * @param char[] with the full name of the file.
 * @return TRUE if the MDV file is loaded. Otherwise FALSE.
 */
bool load_mdv_cartridge(char const *file) {
    //Open the file and based on return code, finish the operation or continue
    //if(pf_open(file) != FR_OK) return false; //TODO: Borrar una vez comprobado.

    UINT readSize = 0;
    int bufferPos = 0;
    int filePos;

    for(int buc = 0; buc < 255; buc++){
        int filePos = buc * MDV_SECTOR_SIZE + MDV_PREAMBLE_SIZE; //skip preamble
        //The order is important due to process of evaluation and exucations of functions to get they resaults to evaluate themselves.
        if(pf_lseek(filePos) ||
         (pf_read(&cartridge_image[bufferPos], MDV_HEADER_SIZE, &readSize)) ||
         (readSize != MDV_HEADER_SIZE)) return false;

        filePos += MDV_HEADER_SIZE + MDV_PREAMBLE_SIZE;
        bufferPos += MPD_HEADER_SIZE;

        if(pf_lseek(filePos) ||
        (pf_read(&cartridge_image[bufferPos], MPD_DATA_SIZE, &readSize)) || 
        (readSize != MPD_DATA_SIZE)) return false;

        bufferPos += MPD_DATA_SIZE;
    }

    return true;
}

/**
 * Load a MPD image to the cartridge buffer
 * @param char[] with the full name of the file.
 */
bool load_mpd_cartridge(char const *file) {
    // if (pf_open(file) != FR_OK) return false;  //TODO: Borrar una vez comprobado
    UINT readSize = 0;
    if (pf_read(cartridge_image, CART_MPD_SIZE, &readSize) ||
       (readSize != CART_MPD_SIZE)) return false;

    return true;
}

/**
 * Indicate if a file is present in the MicroSD.
 * This function doesn't check the format of the disk, nor the format any file.
 * @param file name or directory
 * @return TRUE if it exist, Otherwise FALSE.
*/
bool isFilePresent(char const *file){return (pf_open(file) == FR_OK);}

/**
 * The function reads a file from File System directly, without setting enviroment variables.
 * Results are returned by referenced pointers. Open the file is mandatory before it.
 * @param file name of the config file set by this application to read datas.
 * @param buffDataIn is a buffer where to dump read datas.
 * @param block number of datas to read from the file.
 * @param br returned number of items read from the file.
 * @return TRUE if operation was compleated. Otherwise FALSE.
*/
bool loadFile(char const *file, BYTE *buffDataIn, const UINT block, UINT *br){ return (pf_read(buffDataIn, block, br) != FR_OK);}

/**
 * This function load a file from storage device.
 *  To do this job, it start checking if the file exist, in case that is present in the storage system,
 *  the function start to check every file in the FAT table of root directory, one by one entry on the table.
 *  Once the right entry is found, the function proceed to load it based on the extension (ie. MDP or MDV),
 *  and return the result of operation. It also sets the enviroment variable and fields to keep registered 
 *  the loaded image, in order to be able to save new datas in the right place/file.
 * @param filename Name of the file image to load.
 * @return TRUE if the operation was done, otherwise FALSE. 
*/
bool autoLoadFile(char const *fileName){
    bool done = false;
    if(!isFilePresent(fileName)) {return false;}
    pf_readdir(&dir, NULL);
    //Start searching.
    while(!done){
        if(nextFSEntry() != FR_OK || fno.fname[0] == 0){break;}
        //Comparing names of file pointed in FAT table and file name to load.
		if(strcmp(fno.fname,fileName) == 0){
            //TODO: IN_FOLDER is the State of 'select file', not sure if necessary
            //IN_FOLDER;
            //Selected file, use the case to know its format and load it properly
            switch(fno.fsize){
				case CART_MDV_SIZE:
					crt_type = MDV;
					break;
				case CART_MPD_SIZE:
					crt_type = MPD;
					break;
				default:
                    pf_readdir(&dir, NULL);
                    return false;
					break;
			}
            //Update the path with the file once it was filtered & aproved
            //updatePath(); //TODO: With last changes it seems useless. Test & remove it
            done = true;
        }
    }
    //Load the image if it was found and valid.
    if(done) done = loadMDx();
    return done;
}

/**
 * The function returns the type of the File System Entry pointed by the system at the moment of the call.
 * Available types are: MDV, MDP, NONE, FOLD.
 * @return File system entry type by the system.
 */
CARTRIDGE_FORMAT getFSType(){
    switch(fno.fsize){
        case CART_MDV_SIZE:
            return MDV;
            break;
        case CART_MPD_SIZE:
            return MPD;
            break;
        default:
            return (IN_FOLDER)? FOLD:NONE;
            break;
	}
}

/**
 * Check if the actual file pointed is a valid image format to load or not.
 * It is useful to take desitions while browsing the FS, like ignore not valid formats or file types.
 * @return TRUE if it is a supported image format. Otherwise it returns FALSE.
 */
bool isSelectable(){ return getFSType() != NONE; }