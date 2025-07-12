#include <tag.h>
#include <stdlib.h> // Necessario per malloc, free, strdup
#include <string.h> // Necessario per memcmp, strdup

// --- Global variables (defined and initialized here) ---
int numRegisteredTags = 2;
TagData registeredTags[MAX_REGISTERED_TAGS] = {
    {"Giuseppe","Castaldo",{0x92, 0x60, 0x92, 0xab, 0xcb}, 10},
    {"Cristina","Carleo",{0x13, 0x0f, 0x28, 0x21, 0x14}, 5} //Questo lo metto a caso.

};


// --- Function Implementations ---

// Function to find a tag in the registeredTags array
TagData* findTag(unsigned char* serNum){
    for (int i = 0; i < numRegisteredTags; i++) {
        if (memcmp(registeredTags[i].serialNum, serNum, 5) == 0) {
            return &registeredTags[i];
        }
    }
    return NULL;
}

//Restituisce Nome e Cognome
char** returnNameSurname(TagData* registeredTag){
	 if (registeredTag == NULL) {
	        return NULL; // Se il tag non è valido, restituisci NULL.
	 }

	 // Alloca memoria per l'array di 2 puntatori a char (uno per il nome, uno per il cognome)
	 char** generalita = (char**)malloc(2 * sizeof(char*));
	 if (generalita == NULL) {
	    // Errore di allocazione per l'array di puntatori
	    return NULL;
	 }

	 // Alloca memoria per il nome e copia la stringa
	 generalita[0] = strdup(registeredTag->name); // strdup alloca memoria e copia la stringa
	 if (generalita[0] == NULL) {
	    // Errore di allocazione per il nome. Libera l'array e restituisci NULL.
	    free(generalita);
	    return NULL;
	 }

	 // Alloca memoria per il cognome e copia la stringa
	 generalita[1] = strdup(registeredTag->surname);
	 if (generalita[1] == NULL) {
	    // Errore di allocazione per il cognome. Libera il nome già allocato, l'array e restituisci NULL.
	    free(generalita[0]); // Libera la prima stringa allocata
	    free(generalita);    // Libera l'array di puntatori
	    return NULL;
	 }

	 return generalita;
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
