
# Sistema di Controllo Accessi per Tornelli

Questo progetto implementa un sistema di controllo accessi intelligente per tornelli, basato su microcontrollore STM32F3DISCOVERY. Il sistema permette l'apertura del tornello tramite l'autenticazione di tag RFID registrati o attraverso un comando IR da remoto, gestendo contestualmente il credito associato agli utenti per il calcolo del costo del biglietto.

## Obiettivi

* **Implementare un sistema di controllo accessi** per un tornello utilizzando un microcontrollore STM32F3 Discovery.
* **Gestire l'autenticazione RFID** tramite un modulo RC522 per il riconoscimento di carte pre-registrate.
* **Integrare il controllo IR** per l'apertura manuale del tornello tramite telecomando.
* **Controllare un motore stepper** per la movimentazione del tornello.
* **Visualizzare lo stato del sistema** e le informazioni utente su un display LCD I2C.
* **(In Implementazione)** Aggiornare il saldo utente e calcolare il costo del biglietto in base alla stazione di partenza/arrivo.

## Tecnologie Utilizzate

* **Microcontrollore:** STM32F303VCTx (Arm Cortex-M4) su scheda STM32F3DISCOVERY
* **Linguaggio di Programmazione:** C
* **Ambiente di Sviluppo:** STM32CubeIDE (Generato con STM32CubeMX 6.15.0)
* **Periferiche Esterne:**
    * Modulo RFID-RC522
    * Ricevitore IR (Infrarossi)
    * Motore Stepper 28BYJ-48 con driver ULN2003
    * Display LCD 16x2 I2C
* **Librerie Custom:** `rc522.h`, `Tag.h`, `lcd_i2c.h`

## Architettura e Design

Il sistema è basato sul microcontrollore STM32F303VCTx e integra diverse periferiche per la gestione del tornello. La logica principale risiede nel file `main.c`, che orchestra le interazioni tra i vari componenti.

* **Modulo RFID (RC522):** Utilizzato per leggere il Serial Number (UID) dei tag RFID. L'autenticazione avviene confrontando l'UID letto con un database di tag registrati (`registeredTags` in `tag.c`).
* **Ricevitore IR:** Intercetta i segnali infrarossi da un telecomando. Un'interruzione sul pin del ricevitore attiva una routine per decodificare il segnale e, se valido, attivare l'apertura del tornello.
* **Motore Stepper (28BYJ-48):** Controlla il meccanismo di apertura/chiusura del tornello. La rotazione del motore è basata su un numero predefinito di "steps" (4096 per rivoluzione completa), gestendo l'apertura e la chiusura con rotazioni parziali.
* **Display LCD (I2C):** Fornisce feedback all'utente e allo sviluppatore, mostrando messaggi di benvenuto, stato del sistema (es. "Passaggio Autorizzato", "Accesso Negato"), nome/cognome dell'utente e credito residuo.
* **Gestione Tag (`Tag.c`):** Un array globale `registeredTags` memorizza i dati degli utenti (nome, cognome, Serial Number RFID, credito). Funzioni come `findTag`, `isTagRegistered`, `getTagInfo` e `updateCoins` gestiscono la logica di ricerca, verifica e aggiornamento del credito.
* **Logica di Controllo:** Il sistema è in grado di decodificare i segnali dal telecomando IR (utilizzando un approccio basato sul rilevamento della durata dei bit per il protocollo NEC) e di gestire l'input del modulo RFID. Il tornello si apre se il tag è riconosciuto o se viene ricevuto il comando IR corretto.

### Diagramma a Blocchi del Sistema

```

\+-------------------+
|                   |
|    STM32F303VCTx  |
|   (Cortex-M4)     |
|                   |
\+--------+----------+
|
|  GPIO, I2C, SPI, Timers
|
\+--------+--------------------------+
|                                   |
|      +----------+   +----------+  |
|      | Modulo   |   | Ricevitore |  |
|      | RFID-RC522 |   |    IR    |  |
|      +----------+   +----------+  |
|          |             |           |
|          | (SPI)       | (GPIO/EXTI) |
|          |             |           |
|      +---v------+   +--v---------+  |
|      | Lettura  |   | Decodifica |  |
|      | Tag      |   | Segnale IR |  |
|      +----------+   +------------+  |
|           |                 |         |
|           |                 |         |
|      +----v-----------------v----+   |
|      | Logica di Autenticazione  |   |
|      | e Controllo Tornello      |   |
|      +---------------------------+   |
|           |                 |         |
|           |                 |         |
|      +----v------+   +-----v-----+   |
|      | Driver    |   | Display   |   |
|      | ULN2003   |   | LCD I2C   |   |
|      +-----------+   +-----------+   |
|            |                 |         |
|            | (GPIO)          | (I2C)     |
|            |                 |         |
|      +-----v-----+           +-----v-----+
|      | Motore    |           | Visualizz.  |
|      | Stepper   |           | Stato     |
|      | 28BYJ-48  |           | e Dati    |
|      +-----------+           +-----------+
|                                   |
\+-----------------------------------+

```

### Pinout dei Componenti



* **RC522 (SPI):**
    * SDA (NSS) -> PA4
    * SCK -> PA5
    * MOSI -> PA7
    * MISO -> PA6
    * RST -> PB0 (o altro pin GPIO)
    * GND -> GND
    * 3.3V -> 3.3V



* **Ricevitore IR:**
    * Output -> PB1 (GPIO con EXTI)
    * VCC -> 3.3V (o 5V se richiesto dal sensore specifico)
    * GND -> GND



* **Motore Stepper (connessioni al driver ULN2003):**
    * IN1 -> PC8
    * IN2 -> PC9
    * IN3 -> PC10
    * IN4 -> PC11
    * (Altre connessioni del driver: VCC, GND al 5V e GND dell'STM32)



* **Display LCD I2C:**
    * SDA -> PB7 (I2C1_SDA)
    * SCL -> PB6 (I2C1_SCL)
    * VCC -> 5V
    * GND -> GND

## Come Compilare ed Eseguire

### Prerequisiti

* **STM32CubeIDE:** Ambiente di sviluppo integrato di STMicroelectronics.
* **Driver ST-LINK:** Necessari per la programmazione e il debug della scheda STM32F3DISCOVERY.

### Compilazione

1.  Apri il progetto "new_turnstiles" in STM32CubeIDE.
2.  Assicurati che tutti i file sorgente (`main.c`, `rc522.c`, `Tag.c`, `lcd_i2c.c`, ecc.) siano inclusi correttamente nel progetto.
3.  Pulisci e ricompila il progetto. Puoi farlo da "Project" -> "Clean" e poi "Project" -> "Build All".

### Esecuzione

1.  Connetti la tua scheda STM32F3DISCOVERY al PC tramite cavo USB.
2.  In STM32CubeIDE, clicca su "Run" (icona a forma di "play" verde) o "Debug" per caricare il firmware sulla scheda.
3.  Il programma si avvierà automaticamente dopo il caricamento.

## Utilizzo

Il sistema è progettato per operare autonomamente una volta alimentato e programmato.

* **Passaggio con RFID:**
    1.  Avvicina un tag RFID al modulo RC522.
    2.  Se il tag è registrato:
        * Il display LCD mostrerà "Passaggio Autorizzato", il nome e cognome dell'utente e il credito residuo.
        * Il tornello si aprirà (rotazione del motore stepper) e poi si richiuderà automaticamente dopo un breve ritardo.
        * **(In Implementazione)** Il credito verrà detratto in base al costo del biglietto e alla stazione di partenza/arrivo (simulata/predefinita per ora).
    3.  Se il tag non è registrato:
        * Il display LCD mostrerà "Accesso Negato".
* **Passaggio con Telecomando IR:**
    1.  Punta un telecomando IR verso il ricevitore IR del sistema.
    2.  Premi il tasto designato per l'apertura del tornello (es. il tasto "Power" o un altro tasto configurato per generare il codice NEC `0x00FF02FD`).
    3.  Se il segnale è riconosciuto:
        * Il display LCD mostrerà "IR Sblocco" o un messaggio simile.
        * Il tornello si aprirà e poi si richiuderà automaticamente.

## Risultati e Analisi

Il progetto ha dimostrato con successo l'integrazione di diverse periferiche e la gestione di interruzioni per un sistema di controllo in tempo reale.

* **Affidabilità RFID:** Il modulo RC522 è in grado di leggere i tag con buona precisione. La gestione dei tag registrati in `tag.c` consente una facile aggiunta/rimozione di utenti.
* **Risposta IR:** La decodifica dei segnali IR basata su timer e input capture si è rivelata robusta per il protocollo NEC, consentendo un'apertura remota affidabile.
* **Controllo Motore Stepper:** Il motore 28BYJ-48, controllato tramite il driver ULN2003 e le uscite GPIO dell'STM32, fornisce una movimentazione precisa e controllata del tornello.
* **Interfaccia Utente (LCD):** Il display LCD offre un feedback immediato e intuitivo sullo stato del sistema e sulle interazioni con gli utenti.

### Screenshot Esemplificativi





## Funzionalità Future / In Implementazione

* **Gestione Stazione di Partenza/Arrivo:** Implementazione della logica per permettere all'utente di selezionare una stazione di partenza (tramite un ulteriore input, es. pulsanti o una tastiera) al momento del passaggio. Questo dato sarà memorizzato nella struttura `TagData` o in una struttura correlata.
* **Calcolo Costo Biglietto:** Definizione di una tabella delle tariffe tra le diverse stazioni. Al passaggio di uscita, verrà calcolato il costo del biglietto in base alla stazione di partenza (registrata al primo passaggio) e alla stazione di arrivo (attuale).
* **Detrazione Credito:** Automazione della detrazione del costo del biglietto dal credito dell'utente memorizzato nella struttura `TagData`.
* **Funzionalità di Ricarica Credito:** Possibile aggiunta di una modalità per ricaricare il credito sui tag RFID.
* **Registrazione Dinamica Tag:** Implementazione di una funzionalità per aggiungere o rimuovere tag RFID dal sistema senza la necessità di ricompilare il firmware.

## Contributi

* Cristina Carleo
* Giuseppe Castaldo

## Riferimenti

* **RFID RC522 Tutorial:** <https://lastminuteengineers.com/how-rfid-works-rc522-arduino-tutorial/>
* **Stepper Motor 28BYJ-48 Tutorial:** <https://lastminuteengineers.com/28byj48-stepper-motor-arduino-tutorial/>
* **IR Remote Receiver Tutorial:** <https://www.circuitbasics.com/arduino-ir-remote-receiver-tutorial/>
* **Documentazione STM32F3DISCOVERY:** Puoi trovare risorse aggiuntive sul sito STMicroelectronics.
