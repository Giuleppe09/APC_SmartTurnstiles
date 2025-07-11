#ifndef __TAG_H
#define __TAG_H

#define MAX_REGISTERED_TAGS 10

#include <stdbool.h> // **ADD THIS for 'bool', 'true', 'false'**
#include "rc522.h"    // **ADD THIS for 'uchar' type definition**

// ---TagData structure ---
typedef struct {
    uchar serialNum[5]; // Stores the 5-byte serial number of the RFID tag
    int coins;          // Represents the "coins" associated with this tag
} TagData;


TagData* findTag(uchar* serNum);
bool isTagRegistered(uchar* serNum);
bool updateCoins(uchar* serNum, int amount);

#endif
