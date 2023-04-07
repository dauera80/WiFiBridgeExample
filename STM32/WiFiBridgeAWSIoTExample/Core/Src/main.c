/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "hdc1080\hdc1080.h"
#include "clcd\clcd.h"

#include "rb.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//TODO: Set AP / AWSIoT info
#define AP_SSID           " "
#define AP_PWD            " "
#define AP_SECTYPE        "WPA"

#define AWSIoT_ENDPOINT   " "
#define AWSIoT_CLIENTID   " "


//TODO: Enable/Disable Debug Message and Bypass WiFiBridge to PC
#define WIFIBRIDGE_DEBUG      0
#define WIFIBRIDGE_BYPASS     0

#if WIFIBRIDGE_DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

#define AT_RECV_LINE_MAX  30

// Publish Topic List
#define PUB_TOPIC_VAR     "EVBDemo/var"
#define PUB_TOPIC_PHOTO   "EVBDemo/photo"
#define PUB_TOPIC_TEMP    "EVBDemo/temp"
#define PUB_TOPIC_ACC     "EVBDemo/3axis"
#define PUB_TOPIC_ENC     "EVBDemo/encoder"
#define PUB_TOPIC_BH002   "EVBDemo/bh002"
#define PUB_TOPIC_SW      "EVBDemo/switch"

// Subscribe Topic List
#define SUB_TOPIC_LED     "EVBDemo/LED"
#define SUB_TOPIC_RGB     "EVBDemo/RGB"
#define SUB_TOPIC_DAC     "EVBDemo/DAC"
#define SUB_TOPIC_SERVO   "EVBDemo/ServoMotor"
#define SUB_TOPIC_CLCD    "EVBDemo/CLCD"

// Raw data ranges from -3g to 3g, sensitivity 330mV/g
#define ACC_RAW_MIN       819
#define ACC_RAW_MAX       3276
// Gravitational acceleration ranges [g]
#define ACC_GRAVITY_MIN   -3
#define ACC_GRAVITY_MAX   3

#define CONVERT_SCALE(x,inMin,inMax,outMin,outMax) ((((float)x - inMin) * ((float)outMax - outMin)) / ((float)inMax - inMin) + outMin)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//TODO: Fill certificate and key of own device(client)
const char certificate[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDWTCCAkGgAwIBAgIUGk10t7Jo3jVUA7wolTqCCnE42ZQwDQYJKoZIhvcNAQEL\r\n"
    " ... "
    "-----END CERTIFICATE-----\r\n";

const char privateKey[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIIEowIBAAKCAQEAycuVFiGODVtDHo8kd7lxSyPUTaKM4+nxDZWKtFRzVJWO4m4m\r\n"
    " ... "
    "-----END RSA PRIVATE KEY-----\r\n";


RingFifo_t gUart1Fifo, gUart2Fifo;
volatile int gEnCoderCnt;
volatile uint16_t gADCxConvertedValue[6];
volatile uint8_t gRGBPulse[3];
volatile int gfPub, gfSoftReset;

static RingFifo_t *WiFi_RingBuf_Handle;
static volatile uint32_t interval_count, interval_count_start;
static char sStr[2048] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the UART2 and Loop until the end of transmission */
  if (ch == '\n')
    HAL_UART_Transmit (&huart2, (uint8_t *) "\r", 1, 0xFFFF);
  HAL_UART_Transmit (&huart2, (uint8_t *) & ch, 1, 0xFFFF);

  return ch;
}

/**
 * @brief   DAC7512 Write through SPI
 * @param   data   unsigned short data for send through SPI
 * @retval  0 is success to send the data, 1 is fail
 */
uint8_t
DAC7512_write (unsigned short data)
{
  uint8_t buff[2], ret;

  HAL_GPIO_WritePin (SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);

  buff[0] = (uint8_t) (data >> 8);
  buff[1] = (uint8_t) data;

  ret = HAL_SPI_Transmit (&hspi2, buff, 2, 1000);

  if (ret)
    return ret;

  HAL_GPIO_WritePin (SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);

  return 0;
}

/* internal function */
static void Countdown(int ms) {
  interval_count = (uint32_t)ms;
  interval_count_start = HAL_GetTick();
}

static uint8_t Expired(void) {
  return ((HAL_GetTick() - interval_count_start) >= interval_count);
}

static void Serial_clearbuf(void) {
  RB_clear(WiFi_RingBuf_Handle);
}

static uint8_t ReadStringUntil(char *str, char until) {
  uint8_t i = 0;

  do {
      if (RB_isempty(WiFi_RingBuf_Handle))
        return i;  // if not received yet, return read count
      str[i++] = RB_read(WiFi_RingBuf_Handle);
  } while((str[i - 1] != until));
  str[i] = 0;

  return 0;
}

// ReadATresponseLine
// Receive until timeout or 'OK\r' or 'ERRxx\r'
int ReadATresponseLine(char *rvLine, int nLineBufSize,
                       const char *responseFilter,
                       unsigned long delayMs) {
  int nbLine = 0, idx = 0, rvLineLen = 0;
  char *aLine[AT_RECV_LINE_MAX] = {0};
  char *pszSubstr = NULL;
  int i, ret = 3;

  Countdown(delayMs);
  if (rvLine)
    memset(rvLine, 0, nLineBufSize);

  do {
    if (!RB_isempty(WiFi_RingBuf_Handle)) {
      HAL_Delay(1);

      idx = ReadStringUntil(&sStr[idx], '\r');
      if (idx)
          continue; // if can't read until '\r', continue

      int nLen = strlen(sStr);

      if (nLen > 1 ) {
          if (strncmp(sStr, "\n\r", 2) == 0)
            continue;
        aLine[nbLine] = (char *)malloc(nLen + 1);
        if (aLine[nbLine] == NULL)
          DPRINTF ("Malloc Fail!\n");

        strncpy(aLine[nbLine], sStr, nLen);
        aLine[nbLine][nLen] = 0;

        pszSubstr = strstr(aLine[nbLine], "OK\n");
        if (pszSubstr != NULL) {
          DPRINTF ("Found OK\n");
          nbLine++;
          ret = 0;
          break;
        }
        pszSubstr = strstr(aLine[nbLine], "ERR");
        if (pszSubstr != NULL) {
          if (*(pszSubstr + 5) == '\n') // parse to "ERR%02\n" format
            {
              DPRINTF ("Found ERR\n");
              nbLine++;
              ret = 1;
              break;
            }
        }
        nbLine++;
      }
    }
    if (nbLine >= AT_RECV_LINE_MAX) {
      break;
    }
  } while (!Expired());

  DPRINTF ("Total Line %d\n", nbLine);
  for (i = 0; i < nbLine; i++) {
    DPRINTF ("line[%d]: %s", i, aLine[i]);

    if (rvLine) {
        int aLineLen = strlen(aLine[i]);
        if (responseFilter == NULL) {
            // Not filtering response
            if (nLineBufSize > rvLineLen + aLineLen) {
              strcat(rvLine, aLine[i]);
              rvLineLen += aLineLen;
            }
            else
              DPRINTF ("Out of rvLineBuf\n");
        } else if (strlen(responseFilter) > 0) {
            // Filtering Response
            char *pszSubstr = NULL;

            pszSubstr = strstr(aLine[i], responseFilter);
            if (pszSubstr != NULL) {
                pszSubstr += strlen(responseFilter);
                while (isspace((int)*pszSubstr)) // trim heading
                  {
                    pszSubstr++;
                  }
                char *pTemp = pszSubstr;
                while (pTemp < (aLine[i] + strlen(aLine[i]))) // trim ending
                  {
                    if (*pTemp == '\n' || *pTemp == '\r') // remove cariage return and line feed
                      {
                        *pTemp = 0; // zero terminate string
                        break;
                      }
                    pTemp++;
                  }

                DPRINTF ("Filtered response: %s\n", pszSubstr);
                if (nLineBufSize > aLineLen)
                  strcpy(rvLine, pszSubstr);
                else
                  DPRINTF ("Out of rvLineBuf\n");
            }
        } else {
            // Not filtering response
            if (nLineBufSize > rvLineLen + aLineLen) {
              strcat(rvLine, aLine[i]);
              rvLineLen += aLineLen;
            }
            else
              DPRINTF ("Out of rvLineBuf\n");
        }
    }

    free(aLine[i]);
  }

  return ret;
}
// sendATCmd
/**
 * @brief   Send AT command and Receive response
 *
 * @details
 * @note
 *
 * @param   pCmd            Reference to the Command Buffer
 * @param   pResponse       Reference to the Response Buffer, If not use can set NULL
 * @param   responseBufSize
 * @param   pResponseFilter Filter string in response, If use response buffer copy from pointer after filter string to CR/LF.
 * @param   waitDelayMs     wait Delay for Response, If set 0, It immediate return(Non-Block)
 *
 * @return  Success 0, Fail 1 or 3. If not zero wait delay, it receive response data.
 *          When receive response,
 *          it's 'OK' return 0,
 *          it's 'ERR' return 1,
 *          when timeout return 3
 * */
int SendATcmd(const char *pCmd, char *pResponse, int responseBufSize,
              const char *pResponseFilter,
              unsigned long waitDelayMs) {
  int ret = 0;

  memset(pResponse, 0, responseBufSize);

  DPRINTF ("sendATcmd: %s", pCmd);

  ret = HAL_UART_Transmit(&huart1, (uint8_t *)pCmd, strlen(pCmd), waitDelayMs + 500);

  if (waitDelayMs && !ret)
    {
      ret = ReadATresponseLine(pResponse, responseBufSize, pResponseFilter, waitDelayMs);

      if (ret == 0) {
        DPRINTF ("...ATcmd OK\n");
      } else if (ret == 1) {
        DPRINTF ("...ATcmd Fails\n");
      } else {
        DPRINTF ("...ATcmd not response\n");
      }
    }

  return ret;
}

#define WIFI_RESET_PORT       WiFi_Reset_GPIO_Port
#define WIFI_RESET_PIN        WiFi_Reset_Pin
void WiFiBridge_SoftReset()
{
  HAL_GPIO_WritePin(WIFI_RESET_PORT, WIFI_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(WIFI_RESET_PORT, WIFI_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
}

int WiFiBridge_Init(void)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "at\n");
  do {
      ret = SendATcmd(cmd, NULL, 0, NULL, 1000);
  }while (ret);

  Serial_clearbuf();
  sprintf(cmd, "echo -n 0\n");
  ret = SendATcmd(cmd, NULL, 0, NULL, 1000);

  return ret;
}

int WiFiBridge_WLANConn(char *pSSID, char *pPWD, char *pSecType, char *pSubBuf, int subBufSize)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "wlancfg -s \"%s\" -p \"%s\" -t %s\n", pSSID, pPWD, pSecType);
  ret = SendATcmd(cmd, pSubBuf, subBufSize, NULL, 1000);

  sprintf(cmd, "wlanconnect\n");
  ret = SendATcmd(cmd, &pSubBuf[strlen(pSubBuf)], subBufSize - strlen(pSubBuf), NULL, 10000);

  return ret;
}

int WiFiBridge_AWSIoTCertUpload(const char *pCert)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "awsiotupload -c\n");
  ret = SendATcmd(cmd, NULL, 0, NULL, 0);
  ret = SendATcmd(pCert, NULL, 0, NULL, 5000);

  return ret;
}

int WiFiBridge_AWSIoTKeyUpload(const char *pKey)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "awsiotupload -k\n");
  ret = SendATcmd(cmd, NULL, 0, NULL, 0);
  ret = SendATcmd(pKey, NULL, 0, NULL, 5000);

  return ret;
}

int WiFiBridge_AWSIoTStart(char *pEndPoint, char *pClientID, char *pSubBuf, int subBufSize)
{
  int ret;
  char cmd[256];

  Serial_clearbuf();
  sprintf(cmd, "awsiotcfg -e %s -c %s\n", pEndPoint, pClientID);
  ret = SendATcmd(cmd, pSubBuf, subBufSize, NULL, 1000);

  sprintf(cmd, "awsiotconnect\n");
  ret = SendATcmd(cmd, &pSubBuf[strlen(pSubBuf)], subBufSize - strlen(pSubBuf), NULL, 10000);

  return ret;
}

int WiFiBridge_AWSIoTRegSub(char *pTopic, char *pSubBuf, int subBufSize)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "awsiotcmd -t \"%s\" -s\n", pTopic);
  ret = SendATcmd(cmd, pSubBuf, subBufSize, NULL, 1000);

  return ret;
}

int WiFiBridge_AWSIoTPublish(char *pTopic, char *pMessage, char *pSubBuf, int subBufSize)
{
  int ret;
  char cmd[64];

  Serial_clearbuf();
  sprintf(cmd, "awsiotcmd -t \"%s\" -p \"%s\"\n", pTopic, pMessage);
  ret = SendATcmd(cmd, pSubBuf, subBufSize, NULL, 1000);

  return ret;
}

int WiFiBridge_AWSIoTPublish_NonBlock(char *pTopic, char *pMessage)
{
  int ret;
  char cmd[64];

  sprintf(cmd, "awsiotcmd -t \"%s\" -p \"%s\"\n", pTopic, pMessage);
  ret = SendATcmd(cmd, NULL, 0, NULL, 0);

  return ret;
}

int Parse_Subscribe_Message(char *pMsg)
{
  int ret = 1;
  char *subTopic, *subMsg;

  int servo_angle, servo_pulse;
  char *clcd_str;

  // subscribe callback msg format - "<Topic>\t<Message>\n\r"
  subTopic = strtok(pMsg, "\t");
  subMsg = strtok(NULL, "\n\r");

  if (subTopic && subMsg)
    {
      printf("Subscribe [%s]: %s\n", subTopic, subMsg);

      //SUB_TOPIC_LED
      if (!strncmp(subTopic, SUB_TOPIC_LED, strlen(SUB_TOPIC_LED)))
        {
          HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, atoi(strtok(subMsg, ",")));
          HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, atoi(strtok(NULL, ",")));
          HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, atoi(strtok(NULL, ",")));
          HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, atoi(strtok(NULL, ",")));
        }

      //SUB_TOPIC_RGB
      if (!strncmp(subTopic, SUB_TOPIC_RGB, strlen(SUB_TOPIC_RGB)))
        {
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, atoi(strtok(subMsg, ","))); // Red
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, atoi(strtok(NULL, ","))); // Green
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, atoi(strtok(NULL, ","))); // Blue

        }

      //SUB_TOPIC_DAC
      if (!strncmp(subTopic, SUB_TOPIC_DAC, strlen(SUB_TOPIC_DAC)))
        {
          DAC7512_write(atoi(subMsg));
        }

      //SUB_TOPIC_SERVO
      if (!strncmp(subTopic, SUB_TOPIC_SERVO, strlen(SUB_TOPIC_SERVO)))
        {
          servo_angle = atoi(subMsg);
          if (servo_angle > 90)
            servo_angle = 90;
          else if (servo_angle < -90)
            servo_angle = -90;

          servo_pulse = ((int) ((servo_angle + 90) * 1000 / 180)) + 1000;
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, servo_pulse);
        }

      //SUB_TOPIC_CLCD
      if (!strncmp(subTopic, SUB_TOPIC_CLCD, strlen(SUB_TOPIC_CLCD)))
        {
          CLCD_Clear();

          strsep(&subMsg, "\"");
          clcd_str = strsep(&subMsg, "\"");
          CLCD_Puts(0, 0, clcd_str);

          strsep(&subMsg, "\"");
          clcd_str = strsep(&subMsg, "\"");
          CLCD_Puts(0, 1, clcd_str);
        }
      ret = 0;
    }
  else
    {
      DPRINTF("sub token fail!\n");
      DPRINTF("%s", pMsg);
      ret = 1;
    }

  return ret;
}

int WiFiBridge_ReceiveHandler(char *pSubBuf)
{
  static uint16_t idx = 0;
  int ret = 1;

  if (!RB_isempty(WiFi_RingBuf_Handle))
    {
      HAL_Delay(5);
      idx = ReadStringUntil(&pSubBuf[idx], '\r');
      if (!idx)
        {
          DPRINTF("[RecvHandle] %s", pSubBuf);
          ret = 0;
        }
      else
        {
          DPRINTF("[Recved] %d\n", idx);
          for(int i=0; i<idx; i++)
            {
              DPRINTF("%c", pSubBuf[i]);
            }
          DPRINTF("\n");
        }
    }
  return ret;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  printf ("WiFiBridge AWS IoT Example P/G\n");
  printf ("SYSCLK Frequency = %9ld\n", HAL_RCC_GetSysClockFreq ());
  printf ("HCLK   Frequency = %9ld\n", HAL_RCC_GetHCLKFreq ());
  printf ("PCLK   Frequency = %9ld\n", HAL_RCC_GetPCLK1Freq ());

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t I2C_id;
  uint8_t bat_stat, hdc_humi;
  float hdc_temp, tc1047_temp;
  float grav_accel[3];

  char pubMsg[64] = { 0 };
  char subBuf[256] = { 0 };

  char *pSubMsg;
  int i;

  /* Start ADC Calibration */
  if (HAL_ADCEx_Calibration_Start (&hadc1) != HAL_OK)
    {
      Error_Handler ();
    }
  printf ("ADC Calibration Done!\n");

  /* Start ADC in DMA mode,
   *
   * ADC Rank1 - VAR,
   *     Rank2 - CDS,
   *     Rank3 - TC1047
   *     Rank4 ~ 6 - ADXL335 x/y/z
   */
  if (HAL_ADC_Start_DMA (&hadc1, (uint32_t*) &gADCxConvertedValue, 6) != HAL_OK)
    {
      Error_Handler ();
    }
  printf ("Start ADC in DMA Mode\n");

  /* Init HDC1080 */
  // HDC1080 id read to I2C
  HAL_Delay(50);
  hdc1080_read_reg (&hi2c1, HDC1080_ID_DEV, &I2C_id);
  hdc1080_init (&hi2c1, HDC1080_T_RES_14, HDC1080_RH_RES_14, 1, &bat_stat);
  printf ("I2C Initialization Done!\n");
  printf ("HDC1080's ID = %X\n", I2C_id);

  /* TIM4 PWM start for Servo */
  HAL_TIM_PWM_Start (&htim4, TIM_CHANNEL_2);

  /* TIM3 PWM start for RGB LED */
  HAL_TIM_PWM_Start (&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start (&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start (&htim3, TIM_CHANNEL_3);
  printf ("Start PWM for Servo & RGB LED\n");

  /* Init LCD */
  CLCD_Init (16, 2);
  printf ("CLCD Initialization Done!\n");

  /* Init RingBuffer */
  if (RB_init (&gUart1Fifo, 2048))
    {
      Error_Handler ();
    }
  if (RB_init (&gUart2Fifo, 2048))
    {
      Error_Handler ();
    }
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

  /* Init WiFiBridge */
  WiFi_RingBuf_Handle = &gUart1Fifo;

  // Init Boot time takes 2 sec
  WiFiBridge_SoftReset();
  HAL_Delay(2000);

  WiFiBridge_Init();
  printf ("WiFiBridge Initialization Done!\n");

  if (WiFiBridge_WLANConn(AP_SSID, AP_PWD, AP_SECTYPE, NULL, 0))
    {
      printf("WLAN Connect Fail...\n");
      Error_Handler ();
    }
  printf("\n");
  printf("WLAN Connect to AP: [%s]\n", AP_SSID);

  WiFiBridge_AWSIoTCertUpload(certificate);
  WiFiBridge_AWSIoTKeyUpload(privateKey);
  printf("Upload Certificate & Key file Done!\n");

  if (WiFiBridge_AWSIoTStart(AWSIoT_ENDPOINT, AWSIoT_CLIENTID, NULL, 0))
    {
      printf("AWSIoT Connect Fail...\n");
      Error_Handler ();
    }
  printf("AWSIoT Connect Success!\n");

  WiFiBridge_AWSIoTRegSub(SUB_TOPIC_LED, NULL, 0);
  WiFiBridge_AWSIoTRegSub(SUB_TOPIC_RGB, NULL, 0);
  WiFiBridge_AWSIoTRegSub(SUB_TOPIC_DAC, NULL, 0);
  WiFiBridge_AWSIoTRegSub(SUB_TOPIC_SERVO, NULL, 0);
  WiFiBridge_AWSIoTRegSub(SUB_TOPIC_CLCD, NULL, 0);
  printf("AWSIoT Register subscribe topic Done!\n");

  /* Start TIM2 Interrupt (100msec) */
  HAL_TIM_Base_Start_IT (&htim2);

  printf("Start AWS IoT Demo!\n");
  printf("\n");
  while (1)
  {
#if WIFIBRIDGE_BYPASS
      uint8_t ch;
      // Bypass UART data
      if (!RB_isempty (&gUart1Fifo))
        {
          ch = RB_read(&gUart1Fifo);
          HAL_UART_Transmit(&huart2, &ch, 1, 0xFFFF);
        }
      if (!RB_isempty (&gUart2Fifo))
        {
          ch = RB_read(&gUart2Fifo);
          HAL_UART_Transmit(&huart1, &ch, 1, 0xFFFF);
        }

      // Check Reset, It Flag is set when push B1 switch.
      if (gfSoftReset)
        {
          gfSoftReset = 0;

          // Software Reset Boot time takes 1 sec
          WiFiBridge_SoftReset();
          HAL_Delay(1000);

          RB_clear(&gUart1Fifo);
        }
#else
      // Check Publish flag (1000msec)
      if (gfPub == 1)
        {
          gfPub = 0;

          // data acquisition
          hdc1080_measure (&hi2c1, &hdc_temp, &hdc_humi);
          tc1047_temp = (float) (((gADCxConvertedValue[2] * 3.3 / 4096 * 1000) - 500) / 10);

          for (i = 0; i < 3; i++)
            {
              if (gADCxConvertedValue[i + 3] < ACC_RAW_MIN)
                gADCxConvertedValue[i + 3] = ACC_RAW_MIN;

              if (gADCxConvertedValue[i + 3] > ACC_RAW_MAX)
                gADCxConvertedValue[i + 3] = ACC_RAW_MAX;

              // convert to gravitation acceleration [g]
              grav_accel[i] = CONVERT_SCALE(gADCxConvertedValue[i + 3], ACC_RAW_MIN, ACC_RAW_MAX, ACC_GRAVITY_MIN, ACC_GRAVITY_MAX);
            }

          //PUB_TOPIC_VAR
          sprintf(pubMsg, "%d", gADCxConvertedValue[0]);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_VAR, pubMsg);

          //PUB_TOPIC_PHOTO
          sprintf(pubMsg, "%d", gADCxConvertedValue[1]);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_PHOTO, pubMsg);

          //PUB_TOPIC_TEMP
          sprintf(pubMsg, "%.1f", tc1047_temp);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_TEMP, pubMsg);

          //PUB_TOPIC_ACC
          sprintf(pubMsg, "%.2f,%.2f,%.2f", grav_accel[0], grav_accel[1], grav_accel[2]);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_ACC, pubMsg);

          //PUB_TOPIC_ENC
          sprintf(pubMsg, "%d", gEnCoderCnt);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_ENC, pubMsg);

          //PUB_TOPIC_BH002
          sprintf(pubMsg, "%.1f,%d", hdc_temp, hdc_humi);
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_BH002, pubMsg);

          //PUB_TOPIC_SW
          sprintf(pubMsg, "%d,%d,%d,%d",
                  !HAL_GPIO_ReadPin(SW0_GPIO_Port, SW0_Pin),
                  !HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin),
                  !HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin),
                  !HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin));
          WiFiBridge_AWSIoTPublish_NonBlock(PUB_TOPIC_SW, pubMsg);

        }

      // Check Receive data
      if (!WiFiBridge_ReceiveHandler(subBuf))
        {
          // Check '>>' for subscribe message
          pSubMsg = strstr(subBuf, ">>");
          if (pSubMsg)
            Parse_Subscribe_Message(pSubMsg + 2);
          else
            DPRINTF ("except: %s\n", subBuf);

          memset(subBuf, 0, sizeof(subBuf));
        }

#endif
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 6;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 127;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 49999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 249;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 255;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 63;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CLCD_D0_Pin|CLCD_D1_Pin|CLCD_D2_Pin|CLCD_D3_Pin
                          |CLCD_EN_Pin|CLCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LED0_Pin|LED1_Pin|LED2_Pin
                          |LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WiFi_Reset_GPIO_Port, WiFi_Reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : B1_Pin EN_A_SIG_Pin */
  GPIO_InitStruct.Pin = B1_Pin|EN_A_SIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : CLCD_D0_Pin CLCD_D1_Pin CLCD_D2_Pin CLCD_D3_Pin
                           CLCD_EN_Pin CLCD_RS_Pin */
  GPIO_InitStruct.Pin = CLCD_D0_Pin|CLCD_D1_Pin|CLCD_D2_Pin|CLCD_D3_Pin
                          |CLCD_EN_Pin|CLCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LED0_Pin LED1_Pin LED2_Pin
                           LED3_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LED0_Pin|LED1_Pin|LED2_Pin
                          |LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SW0_Pin SW1_Pin SW2_Pin SW3_Pin */
  GPIO_InitStruct.Pin = SW0_Pin|SW1_Pin|SW2_Pin|SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI2_CS_Pin WiFi_Reset_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_Pin|WiFi_Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : EN_SW_Pin */
  GPIO_InitStruct.Pin = EN_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EN_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EN_B_SIG_Pin */
  GPIO_InitStruct.Pin = EN_B_SIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EN_B_SIG_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
  static uint8_t timerCnt = 0;
  if (htim->Instance == TIM2)  // 10Hz(100msec) Timer
    {
      if (++timerCnt >= 10)
        {
          timerCnt = 0;
          gfPub = 1;
        }
    }
}

void HAL_UART_RxCpltCallback (UART_HandleTypeDef *UartHandle)
{
  uint8_t rx;

  if (UartHandle->Instance == USART1)
    {
      rx = (uint8_t) (UartHandle->Instance->DR & (uint8_t) 0x00FF);
      RB_write (&gUart1Fifo, rx);
    }
  if (UartHandle->Instance == USART2)
    {
      rx = (uint8_t) (UartHandle->Instance->DR & (uint8_t) 0x00FF);
      RB_write (&gUart2Fifo, rx);
    }
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
    {
    case GPIO_PIN_8: // Encoder SW
      gEnCoderCnt = 0;
      break;

    case GPIO_PIN_12: // Encoder A_SIG
      if (!HAL_GPIO_ReadPin (EN_B_SIG_GPIO_Port, EN_B_SIG_Pin))
        gEnCoderCnt++;
      break;

    case GPIO_PIN_15: // Encoder B_SIG
      if (!HAL_GPIO_ReadPin (EN_A_SIG_GPIO_Port, EN_A_SIG_Pin))
        gEnCoderCnt--;
      break;

    case B1_Pin:
      // Set WiFiBridge Reset Flag
      gfSoftReset = 1;
      break;
    }

}
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
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      HAL_Delay(100);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
