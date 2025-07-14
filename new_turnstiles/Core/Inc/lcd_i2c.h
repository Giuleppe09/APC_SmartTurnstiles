/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_i2c.h
  * @brief          : Header for lcd_i2c.c file.
  * This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h" // Include main.h to get access to HAL functions and hi2c1

/* Private defines -----------------------------------------------------------*/
#define LCD_ADDRESS     (0x27 << 1) // I2C address of the PCF8574 (common: 0x27 or 0x3F). Shift left by 1 for 8-bit address.

/* Private function prototypes -----------------------------------------------*/
void lcd_init (void);
void lcd_send_cmd (char cmd);
void lcd_send_data (char data);
void lcd_send_string (char *str);
void lcd_clear (void);
void lcd_put_cur(int row, int col);
void lcd_display_message_lines(const char* line1, const char* line2);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_I2C_H */
