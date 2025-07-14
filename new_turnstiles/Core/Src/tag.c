#include <tag.h>
#include <lcd_i2c.h>
#include <stdlib.h> // Necessario per malloc, free, strdup
#include <string.h> // Necessario per memcmp, strdup

// --- Global variables (defined and initialized here) ---
int numRegisteredTags = 4;
TagData registeredTags[MAX_REGISTERED_TAGS] = {
    {"Giuseppe","Castaldo",{0x92, 0x60, 0x92, 0xab, 0xcb}, 100, "Napoli SG"},
    {"Cristina","Carleo",{0x13, 0x0f, 0x28, 0x21, 0x14}, 50, "Bagnoli"}, // Pozzuoli: accesso libero
    {"Antonio","Emmanuele",{0xe3, 0x28, 0x11, 0x22, 0xf8}, 500, "Napoli SG"},//carta Cris
    {"Vincenzo", "Bruno",{0x12, 0xc9, 0x92, 0xab, 0xe2}, 500, "Bagnoli"} //tag Cris
//    {"Antonio","Emmanuele",{0xe3, 0x28, 0x11, 0x22, 0xf8}, 500, "Pozzuoli"},//carta Cris
//    {"Vincenzo", "Bruno",{0x12, 0xc9, 0x92, 0xab, 0xe2}, 5000, null } //tag Cris
//    {"Antonio","Emmanuele",{0xe3, 0x28, 0x11, 0x22, 0xf8}, 50, "Napoli san giovanni"},//carta Cris
//    {"Vincenzo", "Bruno",{0x12, 0xc9, 0x92, 0xab, 0xe2}, 50, "Bagnoli"} //tag Cris

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

bool getNamesBySerial(unsigned char* serNum, char* nameBuffer, size_t nameBufferSize, char* surnameBuffer, size_t surnameBufferSize) {
    TagData* foundTag = findTag(serNum); // Usa la tua funzione findTag esistente

    // Assicurati che i buffer siano sempre terminati da null in caso di errore o tag non trovato
    if (nameBuffer != NULL && nameBufferSize > 0) nameBuffer[0] = '\0';
    if (surnameBuffer != NULL && surnameBufferSize > 0) surnameBuffer[0] = '\0';

    if (foundTag != NULL) {
        // Copia il nome nel buffer, assicurandoti di non superare la dimensione
        if (nameBuffer != NULL && nameBufferSize > 0) {
            strncpy(nameBuffer, foundTag->name, nameBufferSize - 1);
            nameBuffer[nameBufferSize - 1] = '\0'; // Assicura la terminazione della stringa
        }

        // Copia il cognome nel buffer, assicurandoti di non superare la dimensione
        if (surnameBuffer != NULL && surnameBufferSize > 0) {
            strncpy(surnameBuffer, foundTag->surname, surnameBufferSize - 1);
            surnameBuffer[surnameBufferSize - 1] = '\0'; // Assicura la terminazione della stringa
        }
        return true; // Tag trovato e dati copiati
    } else {
        // Tag non trovato. I buffer sono già stati azzerati sopra.
        return false;
    }
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

// Function to update Departure Station for a given tag
bool updateDepartureStation(uchar* serNum, const char* newStation) {
    TagData* tag = findTag(serNum);
    if (tag != NULL) {
        // Copia la nuova stazione nel campo departureStation
        strncpy(tag->departureStation, newStation, MAX_STRING_LENGTH - 1);
        tag->departureStation[MAX_STRING_LENGTH - 1] = '\0'; // Assicura la terminazione della stringa
        return true;
    }
    return false; // Tag non trovato
}


TicketProcessResult processTicketPayment(TagData* tag) {
    TicketProcessResult result;
    result.accessType = ACCESS_DENIED;
    result.showRedLed = false;

    if (tag == NULL) {
        snprintf(result.messageLine1, 17, "ERRORE TAG"); // Max 16 + null
        snprintf(result.messageLine2, 17, "NON VALIDO!");
        return result;
    }

    char tempBuffer[MAX_STRING_LENGTH]; // Buffer temporaneo per sprintf

    if (strcmp(tag->departureStation, "Pozzuoli") == 0) {
        snprintf(result.messageLine1, 17, "USCITA Pozzuoli");
        snprintf(result.messageLine2, 17, "TORNELLO APERTO");
        result.accessType = ACCESS_EXIT;
        updateDepartureStation(tag->serialNum, "");
    } else if (strcmp(tag->departureStation, "Bagnoli") == 0) {
        int costoBagnoli = 130;
        if (updateCoins(tag->serialNum, -costoBagnoli)) {
            int euroScalati = costoBagnoli / 100;
            int centesimiScalati = costoBagnoli % 100;
            snprintf(tempBuffer, MAX_STRING_LENGTH, "SCALATI: %d.%02dE", euroScalati, centesimiScalati);
            snprintf(result.messageLine1, 17, "%s", tempBuffer); // Tronca a 16

            snprintf(tempBuffer, MAX_STRING_LENGTH, "CREDITO: %d.%02dE", tag->coins / 100, tag->coins % 100);
            snprintf(result.messageLine2, 17, "%s", tempBuffer); // Tronca a 16

            // Questi messaggi verranno visualizzati dal main dopo aver chiamato questa funzione
            result.accessType = ACCESS_EXIT;
            updateDepartureStation(tag->serialNum, "");
        } else {
            snprintf(result.messageLine1, 17, "ACCESSO NEGATO");
            snprintf(result.messageLine2, 17, "CREDITO INSUFF.");
            result.showRedLed = true;
            result.accessType = ACCESS_DENIED;
        }
    } else if (strcmp(tag->departureStation, "Napoli SG") == 0) {
        int costoSanGiovanni = 310;
        if (updateCoins(tag->serialNum, -costoSanGiovanni)) {
            int euroScalati = costoSanGiovanni / 100;
            int centesimiScalati = costoSanGiovanni % 100;
            snprintf(tempBuffer, MAX_STRING_LENGTH, "SCALATI: %d.%02dE", euroScalati, centesimiScalati);
            snprintf(result.messageLine1, 17, "%s", tempBuffer);

            snprintf(tempBuffer, MAX_STRING_LENGTH, "CREDITO: %d.%02dE", tag->coins / 100, tag->coins % 100);
            snprintf(result.messageLine2, 17, "%s", tempBuffer);

            result.accessType = ACCESS_EXIT;
            updateDepartureStation(tag->serialNum, "");
        } else {
            snprintf(result.messageLine1, 17, "ACCESSO NEGATO");
            snprintf(result.messageLine2, 17, "CREDITO INSUFF.");
            result.showRedLed = true;
            result.accessType = ACCESS_DENIED;
        }
    }
    // Logica di entrata: se la stazione di partenza è vuota o sconosciuta
    else if (strlen(tag->departureStation) == 0 || strcmp(tag->departureStation, "Sconosciuta") == 0) {
        snprintf(result.messageLine1, 17, "ARRIVEDERCI");
        snprintf(result.messageLine2, 17, "DA POZZUOLI"); // Stazione di entrata predefinita
        result.accessType = ACCESS_ENTER;
        updateDepartureStation(tag->serialNum, "Pozzuoli"); // Aggiorna la stazione del tag
    }
    else {
        snprintf(result.messageLine1, 17, "STAZ. NON GEST.");
        snprintf(result.messageLine2, 17, "ACCESSO NEGATO!");
        result.showRedLed = true;
        result.accessType = ACCESS_DENIED;
    }

    return result;
}
