/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_i2c.c
  * @brief          : LCD I2C driver file.
  * This file provides functions to control a 16x2 LCD via I2C.
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
#include "lcd_i2c.h"
#include <string.h> // Required for strlen

extern I2C_HandleTypeDef hi2c1; // External declaration for your I2C handle

#define RS_COMMAND      0x00
#define RS_DATA         0x01
#define LCD_BACKLIGHT   0x08 // Backlight bit on PCF8574

// Function to send a byte to the PCF8574
void i2c_pcf8574_write(uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, &data, 1, 100);
}

// Function to send a command or data nibble to the LCD
void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data_t;

    // Construct the byte for PCF8574:
    // D7 D6 D5 D4 BL EN RW RS
    // nibble (high 4 bits) + Backlight + Enable + Read/Write (Write=0) + Register Select
    data_t = (nibble & 0xF0) | LCD_BACKLIGHT | rs;

    // Send high nibble with Enable high, then Enable low
    i2c_pcf8574_write(data_t | 0x04);  // EN = 1 (0x04)
    HAL_Delay(1);
    i2c_pcf8574_write(data_t & ~0x04); // EN = 0 (~0x04)
    HAL_Delay(1);
}

// Function to send a command to the LCD
void lcd_send_cmd(char cmd)
{
    // Send high nibble
    lcd_send_nibble(cmd & 0xF0, RS_COMMAND);
    // Send low nibble
    lcd_send_nibble((cmd & 0x0F) << 4, RS_COMMAND);
}

// Function to send data to the LCD
void lcd_send_data(char data)
{
    // Send high nibble
    lcd_send_nibble(data & 0xF0, RS_DATA);
    // Send low nibble
    lcd_send_nibble((data & 0x0F) << 4, RS_DATA);
}

// Function to initialize the LCD
void lcd_init(void)
{
    // 4-bit mode initialization sequence (datasheet dependent, common for HD44780)
    HAL_Delay(50);  // Power-on delay
    lcd_send_nibble(0x30, RS_COMMAND); // Function Set: 8-bit interface
    HAL_Delay(5);   // Wait for more than 4.1ms
    lcd_send_nibble(0x30, RS_COMMAND); // Function Set: 8-bit interface
    HAL_Delay(1);   // Wait for more than 100us
    lcd_send_nibble(0x30, RS_COMMAND); // Function Set: 8-bit interface
    HAL_Delay(10);

    lcd_send_nibble(0x20, RS_COMMAND); // Function Set: 4-bit interface
    HAL_Delay(10);

    // Now in 4-bit mode
    lcd_send_cmd(0x28); // Function Set: 4-bit, 2 lines, 5x8 dots
    HAL_Delay(1);
    lcd_send_cmd(0x08); // Display OFF
    HAL_Delay(1);
    lcd_send_cmd(0x01); // Clear Display
    HAL_Delay(2);   // Clear display takes longer
    lcd_send_cmd(0x06); // Entry Mode Set: Increment cursor, no display shift
    HAL_Delay(1);
    lcd_send_cmd(0x0C); // Display ON, Cursor OFF, Blink OFF
    HAL_Delay(1);
}

// Function to clear the LCD
void lcd_clear(void)
{
    lcd_send_cmd(0x01); // Clear display
    HAL_Delay(2);       // Delay for clear operation
}

// Function to set cursor position
void lcd_put_cur(int row, int col)
{
    uint8_t address;
    switch(row)
    {
        case 0:
            address = 0x80 + col; // 0x80 is DDRAM address for line 1
            break;
        case 1:
            address = 0xC0 + col; // 0xC0 is DDRAM address for line 2
            break;
        default:
            return; // Invalid row
    }
    lcd_send_cmd(address);
}

// Function to send a string to the LCD
void lcd_send_string(char *str)
{
    while (*str)
    {
        lcd_send_data(*str++);
    }
}

// Funzione per visualizzare due linee di testo sull'LCD
void lcd_display_message_lines(const char* line1, const char* line2)
{
    lcd_clear();

    if (line1 != NULL) {
        lcd_put_cur(0, 0);
        // lcd_send_string invia l'intera stringa fino al null terminator
        // Per 16x2, assicurati che le stringhe passate non superino 16 caratteri per riga.
        lcd_send_string((char*)line1); // Cast a char* necessario per lcd_send_string
    }

    if (line2 != NULL) {
        lcd_put_cur(1, 0);
        lcd_send_string((char*)line2); // Cast a char* necessario per lcd_send_string
    }
}
