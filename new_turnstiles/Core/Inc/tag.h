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
    char departureStation[MAX_STRING_LENGTH]; // **NEW: Departure station name**
} TagData;

// Struttura per il risultato della funzione di pagamento
typedef enum {
    ACCESS_DENIED = 0, // Accesso negato
    ACCESS_ENTER  = 1, // Entrata (se la stazione è vuota o sconosciuta)
    ACCESS_EXIT   = 2  // Uscita (se la stazione è Pozzuoli, Bagnoli, S. Giovanni)
} AccessType;

typedef struct {
	AccessType accessType;
    char messageLine1[MAX_STRING_LENGTH]; // Messaggio per la prima riga LCD
    char messageLine2[MAX_STRING_LENGTH]; // Messaggio per la seconda riga LCD
    bool showRedLed;                      // Indica se accendere il LED rosso
} TicketProcessResult;


TagData* findTag(uchar* serNum);
bool isTagRegistered(uchar* serNum);
bool updateCoins(uchar* serNum, int amount);
bool updateDepartureStation(uchar* serNum, const char* newStation);
char** returnNameSurname(TagData* registeredTag);
bool getNamesBySerial(unsigned char* serNum, char* nameBuffer, size_t nameBufferSize, char* surnameBuffer, size_t surnameBufferSize);
TicketProcessResult processTicketPayment(TagData* tag);

#endif
