#include <string.h> // For memcmp
#include <tag.h> // Include your own header file first

// --- Global variables (defined and initialized here) ---
TagData registeredTags[MAX_REGISTERED_TAGS] = {
    {{0x12, 0x34, 0x56, 0x78, 0x9A}, 10},
    {{0xAB, 0xCD, 0xEF, 0x10, 0x23}, 5},
};

int numRegisteredTags = 2;

// --- Function Implementations ---

// Function to find a tag in the registeredTags array
TagData* findTag(unsigned char* serNum) {
    for (int i = 0; i < numRegisteredTags; i++) {
        if (memcmp(registeredTags[i].serialNum, serNum, 5) == 0) {
            return &registeredTags[i];
        }
    }
    return NULL;
}

// Function to check if a tag is registered
bool isTagRegistered(unsigned char* serNum) {
    return findTag(serNum) != NULL;
}

// Function to update coins for a given tag
bool updateCoins(unsigned char* serNum, int amount) {
    TagData* tag = findTag(serNum);
    if (tag != NULL) {
        if ((tag->coins + amount) >= 0) { // Prevent negative coins
            tag->coins += amount;
            return true;
        } else {
            return false; // Not enough coins
        }
    }
    return false; // Tag not found
}
