#ifndef __TAG_H
#define __TAG_H

#define MAX_REGISTERED_TAGS 10
#define MAX_STRING_LENGTH 50
#define SERIAL_DIM 5

#include <stdbool.h> // **ADD THIS for 'bool', 'true', 'false'**
#include "rc522.h"    // **ADD THIS for 'uchar' type definition**

// ---TagData structure ---
typedef struct {
	char name[MAX_STRING_LENGTH];
	char surname[MAX_STRING_LENGTH];
    uchar serialNum[SERIAL_DIM]; // Stores the 5-byte serial number of the RFID tag
    int coins;          // Represents the "coins" associated with this tag
} TagData;


TagData* findTag(uchar* serNum);
bool isTagRegistered(uchar* serNum);
bool updateCoins(uchar* serNum, int amount);
char** returnNameSurname(TagData* registeredTag);
bool getNamesBySerial(unsigned char* serNum, char* nameBuffer, size_t nameBufferSize, char* surnameBuffer, size_t surnameBufferSize);

#endif
