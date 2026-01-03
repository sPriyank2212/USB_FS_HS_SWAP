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

/* Timing variables for dual USB host processing */
uint32_t last_hid_process = 0;
uint32_t last_msc_process = 0;

/* Connection debouncing structure */
typedef struct {
  uint8_t connected;
  uint8_t stable;
  uint32_t connect_time;
} USB_Device_State_t;

USB_Device_State_t msc_state = {0};
USB_Device_State_t hid_state = {0};

#define DEBOUNCE_TIME_MS  100  // 100ms debounce for connection stability

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
  char Uart_Buf[100];
  int len;
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
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* Sequential start with proper delays for dual USB host stability */
  len = sprintf(Uart_Buf, "[INIT] Starting USB_OTG_HS (MSC)...\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

  /* USER CODE END USB_HOST_Init_PreTreatment */

  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* CRITICAL: Wait for HS to stabilize before starting FS */
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
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  len = sprintf(Uart_Buf, "[INIT] Starting USB_OTG_FS (HID)...\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);

  /* USER CODE END USB_HOST_Init_PostTreatment */

  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* Additional delay for FS enumeration stability */
  HAL_Delay(50);

  len = sprintf(Uart_Buf, "[INIT] USB Dual Host Initialized\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task with timing control for dual USB host
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task with timing optimization */
  uint32_t current_tick = HAL_GetTick();

  // Process HID every 1ms (critical for mouse/keyboard smoothness - 1ms polling)
  if(current_tick - last_hid_process >= 1)
  {
    USBH_Process(&hUsbHostFS);  // HID on FS - frequent polling
    last_hid_process = current_tick;
  }

  // Process MSC every 10ms (file I/O tolerates this delay)
  if(current_tick - last_msc_process >= 10)
  {
    USBH_Process(&hUsbHostHS);  // MSC on HS - less frequent polling
    last_msc_process = current_tick;
  }
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

  case HOST_USER_CONNECTION:
    msc_state.connected = 1;
    msc_state.connect_time = HAL_GetTick();
    msc_state.stable = 0;
    Appli_state = APPLICATION_START;
    len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Connecting...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);
    break;

  case HOST_USER_CLASS_ACTIVE:
    // Only proceed if connection is stable (debounced)
    if(HAL_GetTick() - msc_state.connect_time > DEBOUNCE_TIME_MS)
    {
      if(!msc_state.stable) {
        msc_state.stable = 1;
        Appli_state = APPLICATION_READY;
        len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Connected and Stable!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

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
      }
    }
    break;

  case HOST_USER_DISCONNECTION:
    // Only log if was previously stable (prevents spurious disconnect messages)
    if(msc_state.stable) {
      Appli_state = APPLICATION_DISCONNECT;
      len = sprintf(Uart_Buf, "[UART1-MSC] USB Mass Storage Device Disconnected\r\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)Uart_Buf, len, 1000);

      // Unmount USB when disconnected
      if (msc_connected) {
        Unmount_USB();
        msc_connected = 0;
      }
    }
    msc_state.connected = 0;
    msc_state.stable = 0;
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

  case HOST_USER_CONNECTION:
    hid_state.connected = 1;
    hid_state.connect_time = HAL_GetTick();
    hid_state.stable = 0;
    Appli_state = APPLICATION_START;
    len = sprintf(Uart_Buf, "[UART2-HID] HID Device Connecting...\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    break;

  case HOST_USER_CLASS_ACTIVE:
    // Mark as stable after debounce period
    if(!hid_state.stable)
    {
      uint32_t elapsed = HAL_GetTick() - hid_state.connect_time;
      if(elapsed > DEBOUNCE_TIME_MS)
      {
        hid_state.stable = 1;
        Appli_state = APPLICATION_READY;

        if (USBH_HID_GetDeviceType(phost) == HID_MOUSE)
        {
          len = sprintf(Uart_Buf, "[UART2-HID] Mouse Connected and Stable! (Debounced after %lums)\r\n", elapsed);
          HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
        }
        else if (USBH_HID_GetDeviceType(phost) == HID_KEYBOARD)
        {
          len = sprintf(Uart_Buf, "[UART2-HID] Keyboard Connected and Stable! (Debounced after %lums)\r\n", elapsed);
          HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
        }
      }
      else
      {
        // Still waiting for debounce - log this for debugging
        static uint32_t last_debug_msg = 0;
        if(HAL_GetTick() - last_debug_msg > 50) {
          len = sprintf(Uart_Buf, "[UART2-HID] Waiting for debounce... %lums/%dms\r\n", elapsed, DEBOUNCE_TIME_MS);
          HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
          last_debug_msg = HAL_GetTick();
        }
      }
    }
    break;

  case HOST_USER_DISCONNECTION:
    // Only log if was previously stable (prevents spurious disconnect messages)
    if(hid_state.stable) {
      Appli_state = APPLICATION_DISCONNECT;
      len = sprintf(Uart_Buf, "[UART2-HID] HID Device Disconnected\r\n");
      HAL_UART_Transmit(&huart2, (uint8_t *)Uart_Buf, len, 1000);
    }
    hid_state.connected = 0;
    hid_state.stable = 0;
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

