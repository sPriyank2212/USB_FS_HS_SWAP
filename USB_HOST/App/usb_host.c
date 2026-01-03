/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
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

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_hid.h"

/* USER CODE BEGIN Includes */
#include "../../Core/Src/File_Handling.h"
#include"main.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t msc_connected = 0;
uint8_t hid_connected = 0;

/* External UART handles for separate outputs */
extern UART_HandleTypeDef huart1;  // For MSC (Pendrive) on USB_OTG_HS
extern UART_HandleTypeDef huart2;  // For HID (Keyboard/Mouse) on USB_OTG_FS
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess1(USBH_HandleTypeDef *phost, uint8_t id);
static void USBH_UserProcess2(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostHS, USBH_UserProcess1, HOST_HS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* Add delay between USB host initializations to prevent enumeration conflicts */
  HAL_Delay(100);

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess2, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_HID_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostHS);
  USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess1  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_2 */
  char Uart_Buf[100];
  int len;

  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    Appli_state = APPLICATION_DISCONNECT;
    len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Disconnected\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

    // Unmount USB when disconnected
    if (msc_connected) {
      Unmount_USB();
      msc_connected = 0;
    }
    break;

  case HOST_USER_CLASS_ACTIVE:
    Appli_state = APPLICATION_READY;
    len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Ready\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

    // Print MSC device info
//    USBH_MSC_GetLUNInfo(phost, 0);

    // Mount USB and perform file operations
    Mount_USB();
    Check_USB_Details();

    // Remove existing files from USB
    len = sprintf(Uart_Buf, "[UART1-MSC] Cleaning USB - removing existing files...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);
    Format_USB();

    // Turn off LED before starting benchmark
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    // Run benchmark: Write 5MB file and measure speed
    Benchmark_Write_Test();

    // Run benchmark: Read the same file and measure speed
    Benchmark_Read_Test();

    // Set LED solid ON when benchmark complete
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    len = sprintf(Uart_Buf, "[UART1-MSC] Benchmark Complete - LED ON\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

    msc_connected = 1;
    break;

  case HOST_USER_CONNECTION:
    Appli_state = APPLICATION_START;
    len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Connected\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);
    break;

  default:
    break;
  }
  /* USER CODE END CALL_BACK_2 */
}

static void USBH_UserProcess2  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_21 */
  char Uart_Buf[100];
  int len;

  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
    break;

  case HOST_USER_DISCONNECTION:
    Appli_state = APPLICATION_DISCONNECT;
    len = sprintf(Uart_Buf, "[UART2-HID] HID Device Disconnected\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    break;

  case HOST_USER_CLASS_ACTIVE:
    Appli_state = APPLICATION_READY;

    if (USBH_HID_GetDeviceType(phost) == HID_MOUSE)
    {
      len = sprintf(Uart_Buf, "[UART2-HID] Mouse Connected\r\n");
      HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);

      HID_MOUSE_Info_TypeDef *Mouse_Info;
      Mouse_Info = USBH_HID_GetMouseInfo(phost);
      int X_VAL = Mouse_Info->x;
      int Y_VAL = Mouse_Info->y;
      if (X_VAL > 127) X_VAL -= 255;
      if (Y_VAL > 127) Y_VAL -= 255;

      len = sprintf(Uart_Buf, "[UART2-HID] Mouse: X=%d, Y=%d, Btn1=%d, Btn2=%d, Btn3=%d\r\n",
                    X_VAL, Y_VAL, Mouse_Info->buttons[0], Mouse_Info->buttons[1], Mouse_Info->buttons[2]);
      HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    }

    if (USBH_HID_GetDeviceType(phost) == HID_KEYBOARD)
    {
      len = sprintf(Uart_Buf, "[UART2-HID] Keyboard Connected\r\n");
      HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);

      HID_KEYBD_Info_TypeDef *Keyboard_Info;
      Keyboard_Info = USBH_HID_GetKeybdInfo(phost);
      char key = USBH_HID_GetASCIICode(Keyboard_Info);

      len = sprintf(Uart_Buf, "[UART2-HID] Key Pressed = %c\r\n", key);
      HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    }
    break;

  case HOST_USER_CONNECTION:
    Appli_state = APPLICATION_START;
    len = sprintf(Uart_Buf, "[UART2-HID] HID Device Connected\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    break;

  default:
    break;
  }
  /* USER CODE END CALL_BACK_21 */
}

/**
  * @}
  */

/**
  * @}
  */

