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
#include "st7565s_8080.h"
#include "st7565s_gfx.h"
#include <string.h>
#include <stdio.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
//uint8_t framebuf[LCD_WIDTH * LCD_PAGES]; // 128*8 = 1024 байта
static uint8_t framebuf[LCD_WIDTH * (LCD_HEIGHT/8)];
extern uint8_t framebuf[]; // ваш буфер 128*64/8
static const char* WDN[8] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"}; // RTC_WeekDay 1..7
static const char* DOW_RU2[8] = {"", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"}; // 1..7
static const char* DOW_RU_FULL_UP[8] = {"", "ПОНЕДЕЛЬНИК", "ВТОРНИК", "СРЕДА", "ЧЕТВЕРГ", "ПЯТНИЦА", "СУББОТА", "ВОСКРЕСЕНЬЕ"};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void fb_clear(uint8_t v){
  memset(framebuf, v ? 0xFF : 0x00, sizeof(framebuf));
}

static void draw_time_date(const RTC_TimeTypeDef* t, const RTC_DateTypeDef* d){
  // Очистка экрана
  memset(framebuf, 0x00, LCD_WIDTH * (LCD_HEIGHT/8));

  // Форматируем строки
  char time_str[12];
  char date_str[24];

  // Мигающая двоеточия (по желанию):
//   char csep = (t->Seconds & 1) ? ':' : ' ';
//   snprintf(time_str, sizeof(time_str), "%02u%c%02u%c%02u", t->Hours, csep, t->Minutes, csep, t->Seconds);

  snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", t->Hours, t->Minutes, t->Seconds);
  uint16_t year = 2000u + d->Year; // если RTC.Year хранит 0..99

  // "DD.MM.YYYY, DOW"
  snprintf(date_str, sizeof(date_str), "%02u.%02u.%04u, %s", d->Date, d->Month, year,
           (d->WeekDay >=1 && d->WeekDay <=7) ? WDN[d->WeekDay] : "---");

  // Позиционирование по центру
  uint8_t scale_time = 2; // крупнее
  int w_time = GFX_TextWidth(time_str, scale_time);
  int x_time = (LCD_WIDTH - w_time) / 2;
  int y_time = 8; // отступ сверху

  int w_date = GFX_TextWidth(date_str, 1);
  int x_date = (LCD_WIDTH - w_date) / 2;
  int y_date = 40; // ниже

  // Рисуем
  GFX_DrawStringScaled(framebuf, x_time, y_time, time_str, 1, scale_time);
  GFX_DrawString(framebuf,       x_date, y_date, date_str, 1);
}


static void draw_time_date_ru(const RTC_TimeTypeDef* t, const RTC_DateTypeDef* d){
  memset(framebuf, 0x00, LCD_WIDTH * (LCD_HEIGHT/8));

  char time_str[12];
  snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", t->Hours, t->Minutes, t->Seconds);

  char date_str[32];
  uint16_t year = 2000u + d->Year; // если Year=0..99
  const char* dow = (d->WeekDay>=1 && d->WeekDay<=7) ? DOW_RU2[d->WeekDay] : "--";
  // "ДД.ММ.ГГГГ, ПН"
  snprintf(date_str, sizeof(date_str), "%02u.%02u.%04u, %s", d->Date, d->Month, year, dow);

  // Центровка
  uint8_t scale_time = 2;
  int w_time = GFX_TextWidth(time_str, scale_time);
  int x_time = (LCD_WIDTH - w_time)/2;
  int y_time = 8;

  // Для UTF-8 ширину оцениваем приближенно через ASCII ширину:
  // каждая буква/цифра ~6px, scale=1
  int w_date = (int)strlen(date_str) * 6;
  int x_date = (LCD_WIDTH - w_date)/2;
  int y_date = 40;

  // Часы крупно (ASCII)
  GFX_DrawStringScaled(framebuf, x_time, y_time, time_str, 1, scale_time);
  // Дата и день недели — UTF-8 (рус)
  GFX_DrawStringUTF8_RU(framebuf, x_date, y_date, date_str, 1, 1);
}

static void draw_time_date_ru_full(const RTC_TimeTypeDef* t, const RTC_DateTypeDef* d){
  memset(framebuf, 0x00, LCD_WIDTH * (LCD_HEIGHT/8));

  // Время HH:MM:SS крупно (×2)
  char time_str[12];
  snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", t->Hours, t->Minutes, t->Seconds);
  uint8_t scale_time = 2;
  int w_time = GFX_TextWidth(time_str, scale_time);
  int x_time = (LCD_WIDTH - w_time)/2;
  int y_time = 4; // чуть ближе к верху
  GFX_DrawStringScaled(framebuf, x_time, y_time, time_str, 1, scale_time);

  // Дата "DD.MM.YYYY"
  char date_str[20];
  uint16_t year = 2000u + d->Year; // если Year=0..99
  snprintf(date_str, sizeof(date_str), "%02u.%02u.%04u", d->Date, d->Month, year);
  int w_date = GFX_TextWidth(date_str, 1);
  int x_date = (LCD_WIDTH - w_date)/2;
  int y_date = 36; // под временем
  GFX_DrawString(framebuf, x_date, y_date, date_str, 1);

  // День недели — ВЕРХНИМ РЕГИСТРОМ, под датой
  const char* dow = (d->WeekDay>=1 && d->WeekDay<=7) ? DOW_RU_FULL_UP[d->WeekDay] : "---";
  int w_dow = GFX_TextWidthUTF8_RU(dow, 1);
  int x_dow = (LCD_WIDTH - w_dow)/2;
  int y_dow = y_date + 10; // на 1 строку ниже
  GFX_DrawStringUTF8_RU(framebuf, x_dow, y_dow, dow, 1, 1);
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
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  ST7565S_Init();

  // Было в инициализации: EV ~ 0x18. Уменьшаем:
  ST7565S_SetContrast(0x10); // мягче
  // Если всё ещё слишком тёмно:
//  ST7565S_SetContrast(0x0C);
//  ST7565S_SetContrast(0x08);

  fb_clear(0); // гасим фон
//    GFX_DrawString(framebuf, 8,  0, "HELLO, ST7565S!", 1);
//    GFX_DrawString(framebuf, 8, 16, "STM32F401 + HAL", 1);
//    GFX_DrawString(framebuf, 8, 32, "CubeIDE OK :)",   1);

//    ST7565S_DrawBuffer(framebuf);
  static uint8_t prevSeconds = 255;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Чтение времени
	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // обязательно после GetTime

	    if (sTime.Seconds != prevSeconds)
	    {
//	      draw_time_date(&sTime, &sDate);
//	      draw_time_date_ru(&sTime, &sDate);
	    	draw_time_date_ru_full(&sTime, &sDate);
	      ST7565S_DrawBuffer(framebuf);
	      prevSeconds = sTime.Seconds;
	    }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x21;
  sTime.Minutes = 0x16;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_NOVEMBER;
  sDate.Date = 0x24;
  sDate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable Calibrartion
  */
  if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DB0_Pin|DB1_Pin|DB2_Pin|DB3_Pin
                          |DB4_Pin|DB5_Pin|DB6_Pin|DB7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, WR_Pin|RD_Pin|CS_Pin|RES_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : DB0_Pin DB1_Pin DB2_Pin DB4_Pin
                           DB5_Pin DB6_Pin DB7_Pin */
  GPIO_InitStruct.Pin = DB0_Pin|DB1_Pin|DB2_Pin|DB4_Pin
                          |DB5_Pin|DB6_Pin|DB7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RS_Pin WR_Pin RD_Pin CS_Pin
                           RES_Pin */
  GPIO_InitStruct.Pin = RS_Pin|WR_Pin|RD_Pin|CS_Pin
                          |RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DB3_Pin */
  GPIO_InitStruct.Pin = DB3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DB3_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
