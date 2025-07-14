/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rc522.h"
#include <stdbool.h> // Aggiungi questa riga per il tipo bool
#include "Tag.h"
#include "lcd_i2c.h"
#include <stdio.h> // Per sprintf
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define stepsperrev 4096 //PER UNA RIVOLUZIONE

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
volatile uint8_t bitCount = 0;
volatile uint8_t isStartCaptured = 0; // 0: reset, 1: start bit (9ms pulse) captured, 2: 4.5ms pulse captured
volatile uint32_t receivedData = 0;
static volatile uint8_t isRisingCaptured = 0; // Questa variabile non sembra usata nel codice fornito, potrebbe essere un residuo
static volatile uint32_t IC_Value = 0; // Valore catturato dall'Input Capture
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void i2c_scanner(void)
{
    uint8_t i;
    HAL_StatusTypeDef ret;
    char buffer[20];
    uint8_t found_devices = 0;

    // Optional: if you have a serial port/UART configured, you can print messages there
    // Otherwise, you can use a breakpoint in debugger to check the output
    // For now, let's assume you'll use a debugger or just count LEDs

    // Toggle a LED to indicate scanning started
    HAL_GPIO_TogglePin(GPIOE, LD3_Pin); // Use one of your LEDs on GPIOE

    for(i = 1; i < 128; i++) // I2C addresses are 7-bit, from 0x01 to 0x77
    {
        // Try to transmit 1 byte to the current address
        // The last parameter (1) is the number of bytes to transmit, doesn't matter what
        // The timeout (5) is short to quickly check for ACK
        ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 2, 5); // Shift address for 8-bit format

        if (ret == HAL_OK) // Device responded (ACK received)
        {
            // Device found!
            found_devices++;
            // If you have a UART setup, you could print the address:
            // sprintf(buffer, "Found I2C device at 0x%X\r\n", i);
            // HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

            // You can also blink a specific LED or set a breakpoint here
            HAL_GPIO_TogglePin(GPIOE, LD4_Pin); // Blink another LED to confirm detection
            HAL_Delay(100); // Small delay to make LED blink visible
        }
    }

    // Toggle LED again to indicate scanning finished
    HAL_GPIO_TogglePin(GPIOE, LD3_Pin);

    // If you're debugging, you can check 'found_devices' variable here.
    // If found_devices > 0, it means at least one device responded.
    // If found_devices == 1 and you only have the LCD, then 'i' in the loop
    // when ret == HAL_OK will be your LCD address.
}

void delay (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

void stepper_set_rpm (int rpm)  // Set rpm--> max 13, min 1,,,  went to 14 rev/min
{
	delay(60000000/stepsperrev/rpm);
}

void stepper_half_drive (int step)
{
	switch (step){
		case 0:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);   // IN4
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);   // IN4
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);   // IN4
			break;
		case 3:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);   // IN4
			break;
		case 4:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);   // IN4
			break;
		case 5:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET);   // IN4
			break;
		case 6:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET);   // IN4
			break;
		case 7:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);   // IN1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);   // IN2
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // IN3
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET);   // IN4
			break;
	}
}

void stepper_step_angle (float angle, int direction, int rpm) //direction-> 0 for CK, 1 for CCK
{
	float anglepersequence = 0.703125;  // 360 = 512 sequences
	int numberofsequences = (int) (angle/anglepersequence);
	for (int seq=0; seq<numberofsequences; seq++)
	{
		if (direction == 0)  // for clockwise
		{
			for (int step=7; step>=0; step--)
				{
				stepper_half_drive(step);
				stepper_set_rpm(rpm);
				}
		}
		else if (direction == 1)  // for anti-clockwise
		{
			for (int step=0; step<=7; step++)
				{
				stepper_half_drive(step);
				stepper_set_rpm(rpm);
				}
		}
	}
}

float currentAngle = 0; // Questa va dichiarata come variabile globale, non locale a main.

void Stepper_rotate (float targetAngle, int rpm) // Accetta float per l'angolo target
{
    float changeInAngle = targetAngle - currentAngle; // Calcola la differenza tra l'angolo target e l'attuale

    if (changeInAngle > 0.001f)  // Per rotazione in senso orario (valore > 0 con una piccola tolleranza)
    {
        stepper_step_angle(changeInAngle, 0, rpm); // direction 0 per orario
    }
    else if (changeInAngle < -0.001f) // Per rotazione in senso antiorario (valore < 0 con una piccola tolleranza)
    {
        stepper_step_angle(-changeInAngle, 1, rpm); // direction 1 per antiorario, passa il valore assoluto dell'angolo
    }
    currentAngle = targetAngle; // Aggiorna l'angolo corrente
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_PCD_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  i2c_scanner();

  HAL_TIM_Base_Start(&htim1); // Questo è il timer che usi per i tuoi ritardi, lascialo
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // Avvia l'Input Capture con interruzioni


  uchar status_request=0;
  uchar TagType[2] = {0,0};
  uchar serNum[5]= {0,0,0,0,0};	//Per memorizzare il seriale del tag

  TagData* tag; // Puntatore a TagData, non allocazione qui
  MFRC522_Init(); // Inizializza il sensore RC522

  char displayBuffer[64];
  lcd_init(); // Initialize the LCD
  lcd_display_message_lines("Inserire card"," o biglietto ");

  //stop_stepper_motor(); // Assicuriamoci che il motore passo-passo sia fermo
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  // Controlla se 32 bit sono stati ricevuti (il flag bitCount lo indica)
	  if (32 <= bitCount)
	  {
		  // Qui puoi elaborare il valore di 'receivedData'
		  // Esempio: stampalo sul display LCD o su seriale, o confrontalo con codici di pulsanti noti

		  lcd_clear();
		  lcd_put_cur(0,0);
		  char ir_buffer[20];
		  sprintf(ir_buffer, "IR Data: 0x%lX", receivedData); // Stampa il dato in esadecimale
		  lcd_send_string(ir_buffer);
		  HAL_Delay(2000); // Visualizza per 2 secondi


			// Puoi premere qualsiasi tasto
			if (receivedData == receivedData) // Esempio: se il codice IR ricevuto è quello per l'apertura
			{
				lcd_clear();
				lcd_display_message_lines("Accesso", "Consentito!");
				HAL_Delay(1000); // Breve ritardo per visualizzare il messaggio

				// APERTURA TORNELLO: Rotazione completa del tornello
				for (int i = 0; i <= 360; i++) // Fa un giro completo in senso orario
				{
					Stepper_rotate((float)i, 10); // Passa l'angolo come float
				}
				// currentAngle è ora 360

				// CHIUSURA TORNELLO: Torna alla posizione iniziale
				for (int i = 360; i >= 0; i--) // Torna indietro in senso antiorario
				{
					Stepper_rotate((float)i, 10); // Passa l'angolo come float
				}
				lcd_display_message_lines("Tornello", "chiuso");
				HAL_Delay(1000);
			}
			else
			{
				lcd_clear();
				lcd_display_message_lines("IR: Codice", "Sconosciuto!");
				HAL_Delay(2000); // Mostra messaggio di negato
			}

		  // Dopo aver elaborato i dati, resetta le variabili per ricevere il prossimo segnale
		  bitCount = 0;
		  receivedData = 0;
		  isStartCaptured = 0;
		  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // Riavvia l'Input Capture per la prossima ricezione
	  }

	 if (isSensorDetected()){
			// Il sensore RC522 è stato rilevato e sta comunicando correttamente
			HAL_GPIO_WritePin(LD10_GPIO_Port, LD10_Pin, GPIO_PIN_SET);
			HAL_Delay(10); // Piccolo ritardo per stabilità
			status_request = MFRC522_Request(PICC_REQIDL, TagType); // Cerca un tag in modalità idle

			if (status_request == MI_OK)
			{
			  // Tag RFID rilevato!
			  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Accendi il LED che indica tag presente
			  lcd_display_message_lines("Tag RFID","Rilevato!");

			  status_request = MFRC522_Anticoll(serNum);
			  if (status_request == MI_OK)
			  {
				 if(isTagRegistered(&serNum)){
					 tag = findTag(&serNum);
					 // Tag Riconosciuto.
					 HAL_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, GPIO_PIN_SET); // Accendi il LED VERDE 7

					 char tagName[32];
					 char tagSurname[32];
					 bool tagFoundAndCopied = getNamesBySerial(serNum, tagName, sizeof(tagName), tagSurname, sizeof(tagSurname));

					 if (tagFoundAndCopied)
					     {
					         // Tag Riconosciuto.
					         HAL_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, GPIO_PIN_SET); // Accendi il LED VERDE 7

					         lcd_clear();
					         lcd_put_cur(0, 0);
					         sprintf(displayBuffer, "ciao %s", tagName);
					         lcd_send_string(displayBuffer);    // Stampa il nome
					         lcd_put_cur(1, 0);
					         lcd_send_string(tagSurname); // Stampa il cognome

					         //APERTURA TORNELLO: Rotazione completa del tornello
							 for (int i=0; i<=360; i++) // Fa un giro completo in senso orario
							 {
							   Stepper_rotate((float)i, 10); // Passa l'angolo come float
							   // HAL_Delay(X) qui rallenta il movimento, se vuoi un movimento più fluido,
							   // la velocità è controllata dall'RPM
							 }
							  // currentAngle è ora 360

							 // CHIUSURA TORNELLO: Torna alla posizione iniziale
							 for (int i=360; i>=0; i--) // Torna indietro in senso antiorario
							 {
							   Stepper_rotate((float)i, 10); // Passa l'angolo come float
							   // HAL_Delay(X) qui rallenta il movimento
							 }

					         HAL_Delay(2000); // Visualizza nome e cognome

					         // Poi il resto della tua logica per "Accesso Consentito"
					         lcd_display_message_lines("Accesso","Consentito!");



					         lcd_display_message_lines("Arrivederci ","e Buon viaggio");

							 HAL_Delay(2000); // Tempo per visualizzare il messaggio "Arrivederci"
					     }
					 // Torna allo stato iniziale dopo il ciclo completo
					 lcd_display_message_lines("Inserire card"," o biglietto ");

					 HAL_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, GPIO_PIN_RESET); // Spegni LED verde
					 HAL_Delay(50); // Breve delay per stabilità
				 }else{
					 // Tag NON Riconosciuto.
					 HAL_GPIO_WritePin(LD6_GPIO_Port,LD6_Pin,GPIO_PIN_SET); // Accendi il LED ROSSO 6

					 lcd_display_message_lines("Accesso ","Negato!");

					 HAL_Delay(2000); // Mostra messaggio di negato
					 HAL_GPIO_WritePin(LD6_GPIO_Port,LD6_Pin,GPIO_PIN_RESET); // Spegni LED rosso
					 HAL_Delay(50); // Breve delay per stabilità

					 lcd_display_message_lines("Inserire card"," o biglietto ");
				 }
			}
			else
			{
			  // Nessun tag rilevato o errore nella richiesta del tag
			  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Spegni il LED che indica tag presente
			}
		}
		else
		{
			// Il sensore RC522 NON è stato rilevato o non sta comunicando
			HAL_GPIO_WritePin(LD10_GPIO_Port, LD10_Pin, GPIO_PIN_RESET); // Spegni il LED "sensore ok"
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Assicurati che anche il LED del tag sia spento

			HAL_Delay(500); // Breve ritardo prima di riprovare a rilevare il sensore
		}
		// Questo delay rallenta l'intero loop, rimuovilo o riducilo se hai bisogno di risposte più veloci
		// HAL_Delay(100);
	 }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_TIM1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|SPI_1SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SP1_RST_Pin|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
                           MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = DRDY_Pin|MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT1_Pin
                          |MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
                           LD7_Pin LD9_Pin LD10_Pin LD8_Pin
                           LD6_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 SPI_1SS_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|SPI_1SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PF4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : SP1_RST_Pin PB1 */
  GPIO_InitStruct.Pin = SP1_RST_Pin|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    // Assicurati che sia il timer e il canale corretto che ha generato l'interruzione
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) // Adatta TIM2 e TIM_CHANNEL_1 se usi un altro timer/canale
    {
        if(0 == bitCount && 0 == isStartCaptured && 0 == receivedData)
        {
            // Primo fronte di discesa del bit di start (9ms)
            isStartCaptured = 1;
        }
        else if(0 == bitCount && 1 == isStartCaptured && 0 == receivedData)
        {
            // Secondo fronte di discesa del bit di start (dopo 4.5ms)
            // Resetta il contatore del timer per misurare la durata del primo bit di dati
            __HAL_TIM_SET_COUNTER(htim, 0);
            isStartCaptured = 2; // Passa allo stato di acquisizione dati
        }
        else if(32 > bitCount) // Abbiamo ancora bit da ricevere (32 bit per il protocollo NEC)
        {
            IC_Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // Legge il valore catturato
            __HAL_TIM_SET_COUNTER(htim, 0); // Resetta il contatore per la prossima misurazione

            // Decodifica il bit basandosi sulla durata dell'impulso
            // Il protocollo NEC per un '0' ha una durata di ~1.125ms (562.5us MARK + 562.5us SPACE)
            // Per un '1' ha una durata di ~2.25ms (562.5us MARK + 1.687ms SPACE)
            // I valori 1000-1300 e 2100-2400 us suggeriscono che il timer ha un tick di 1us.
            // Questi sono i "gap" tra i fronti di discesa, che rappresentano il "SPACE" del bit.

            // Received Logic '0'
            if(IC_Value > 1000 && IC_Value < 1300) // Questo è il tempo dello "SPACE" per un 0 (~1.125ms)
            {
                receivedData &= ~(1UL << bitCount); // Imposta il bit a 0
            }
            // Received Logic '1'
            else if(IC_Value > 2100 && IC_Value < 2400) // Questo è il tempo dello "SPACE" per un 1 (~2.25ms)
            {
                receivedData |= (1UL << bitCount); // Imposta il bit a 1
            }
            // ELSE: se il valore non rientra in questi range, è un errore o una tempistica non NEC

            bitCount++; // Passa al bit successivo
            if(bitCount == 32) // Tutti i 32 bit sono stati ricevuti
            {
                // Ferma l'Input Capture per evitare ulteriori interruzioni finché non viene processato il dato
                HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1); // Ho cambiato &htim2 in htim
            }
        }
    }
}
/* USER CODE END 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
