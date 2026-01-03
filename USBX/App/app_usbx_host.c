/* USBX host applicative file adapted for STM32F4 (FS) */

/* Includes ------------------------------------------------------------------*/
#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

static TX_THREAD ux_host_app_thread;

/* USER CODE BEGIN PV */
TX_THREAD                   keyboard_app_thread;
TX_THREAD                   mouse_app_thread;
TX_THREAD                   msc_app_thread;
UX_HOST_CLASS_HID           *hid_instance;
UX_HOST_CLASS_HID_MOUSE     *mouse;
UX_HOST_CLASS_HID_KEYBOARD  *keyboard;
UX_HOST_CLASS_STORAGE       *storage;
UX_HOST_CLASS_STORAGE_MEDIA *storage_media;
FX_MEDIA                    *media;
TX_EVENT_FLAGS_GROUP        ux_app_EventFlag;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID app_ux_host_thread_entry(ULONG thread_input);
static UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *current_class, VOID *current_instance);
static VOID ux_host_error_callback(UINT system_level, UINT system_context, UINT error_code);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Forward declarations for F4 port adapter */
void MX_USB_OTG_FS_HCD_Init(void);
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;

/**
  * @brief  Application USBX Host Initialization.
  * @param  memory_ptr: memory pointer
  * @retval status
  */
UINT MX_USBX_Host_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_HOST_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_HOST_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Install the host portion of USBX */
  if (ux_host_stack_initialize(ux_host_event_callback) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Register a callback error function */
  ux_utility_error_callback_register(&ux_host_error_callback);

  /* Initialize the host hid class */
  if (ux_host_stack_class_register(_ux_system_host_class_hid_name,
                                   ux_host_class_hid_entry) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Initialize the host hid mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_mouse_name,
                                        ux_host_class_hid_mouse_entry) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Initialize the host hid keyboard client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_keyboard_name,
                                        ux_host_class_hid_keyboard_entry) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Initialize the host storage class */
  if (ux_host_stack_class_register(_ux_system_host_class_storage_name,
                                   ux_host_class_storage_entry) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Allocate the stack for host application main thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, UX_HOST_APP_THREAD_STACK_SIZE,
                       TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the host application main thread */
  if (tx_thread_create(&ux_host_app_thread, UX_HOST_APP_THREAD_NAME, app_ux_host_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, UX_HOST_APP_THREAD_PRIO,
                       UX_HOST_APP_THREAD_PREEMPTION_THRESHOLD, UX_HOST_APP_THREAD_TIME_SLICE,
                       UX_HOST_APP_THREAD_START_OPTION) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* USER CODE BEGIN MX_USBX_Host_Init1 */

  /* Allocate the stack for HID mouse App thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the HID mouse App thread */
  if (tx_thread_create(&mouse_app_thread, "HID mouse App thread", hid_mouse_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 1, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for HID Keyboard App thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the HID Keyboard App thread */
  if (tx_thread_create(&keyboard_app_thread, "HID Keyboard App thread", hid_keyboard_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 1, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

/* Allocate the stack for storage app thread  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the storage applicative process thread */
  if (tx_thread_create(&msc_app_thread, "MSC App thread", msc_process_thread_entry,
                       0, pointer, (UX_HOST_APP_THREAD_STACK_SIZE * 2), 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Create the event flags group */
  if (tx_event_flags_create(&ux_app_EventFlag, "Event Flag") != TX_SUCCESS)
  {
    return TX_GROUP_ERROR;
  }

  /* USER CODE END MX_USBX_Host_Init1 */

  return ret;
}

/**
  * @brief  Function implementing app_ux_host_thread_entry.
  * @param  thread_input: User thread input parameter.
  * @retval none
  */
static VOID app_ux_host_thread_entry(ULONG thread_input)
{
  /* Initialization of USB host */
  USBX_APP_Host_Init();
}

UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *current_class, VOID *current_instance)
{
  UINT status = UX_SUCCESS;

  /* Get current Hid Client */
  UX_HOST_CLASS_HID_CLIENT *client  = (UX_HOST_CLASS_HID_CLIENT *)current_instance;

  switch (event)
  {
    case UX_DEVICE_INSERTION:

      /* Get current Hid Class */
      if (current_class -> ux_host_class_entry_function == ux_host_class_hid_entry)
      {
        if (hid_instance == UX_NULL)
        {
          /* Get current Hid Instance */
          hid_instance = (UX_HOST_CLASS_HID *)current_instance;
        }
      }

      /* Get current Storage Class */
      if (current_class -> ux_host_class_entry_function == ux_host_class_storage_entry)
      {
        if (storage == UX_NULL)
        {
          /* Get current Storage Instance */
          storage = (UX_HOST_CLASS_STORAGE *)current_instance;

          USBH_UsrLog("\nUSB Mass Storage Device Plugged");
          USBH_UsrLog("PID: %#x ", (UINT)storage -> ux_host_class_storage_device -> ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)storage -> ux_host_class_storage_device -> ux_device_descriptor.idVendor);

          /* Get the storage media */
          storage_media = (UX_HOST_CLASS_STORAGE_MEDIA *)current_class -> ux_host_class_media;

          if (storage_media -> ux_host_class_storage_media_lun != 0)
          {
            storage_media = UX_NULL;
          }
          else
          {
            /* Get the media file */
            media = &storage_media->ux_host_class_storage_media;
          }

          /* Check the storage class state */
          if (storage -> ux_host_class_storage_state ==  UX_HOST_CLASS_INSTANCE_LIVE)
          {
            /* Set STORAGE_MEDIA flag */
            if (tx_event_flags_set(&ux_app_EventFlag, STORAGE_MEDIA, TX_OR) != TX_SUCCESS)
            {
              Error_Handler();
            }
          }
        }
      }

      break;

    case UX_DEVICE_REMOVAL:

      /* Free HID Instance */
      if ((VOID*)hid_instance == current_instance)
      {
        hid_instance = UX_NULL;
      }

      if ((VOID*)storage == current_instance)
      {
        /* Clear storage media instance & media file */
        storage = UX_NULL;
        storage_media = UX_NULL;
        media = UX_NULL;

        USBH_UsrLog("\nUSB Mass Storage Device Unplugged");
      }

      break;

    case UX_HID_CLIENT_INSERTION:

      USBH_UsrLog("\nHID Client Plugged");

      /* Check the HID_client if this is a HID keyboard device */
      if (client -> ux_host_class_hid_client_handler == ux_host_class_hid_keyboard_entry)
      {
        if (keyboard == UX_NULL)
        {
          keyboard = client -> ux_host_class_hid_client_local_instance;

          USBH_UsrLog("HID_Keyboard_Device");
          USBH_UsrLog("PID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
          USBH_UsrLog("USB HID Host Keyboard App...");
          USBH_UsrLog("keyboard is ready...\n");
        }
      }

      /* Check the HID_client if this is a HID mouse device */
      if (client -> ux_host_class_hid_client_handler == ux_host_class_hid_mouse_entry)
      {
        if (mouse == UX_NULL)
        {
          mouse = client -> ux_host_class_hid_client_local_instance;

          USBH_UsrLog("HID_Mouse_Device");
          USBH_UsrLog("PID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
          USBH_UsrLog("USB HID Host Mouse App...");
          USBH_UsrLog("Mouse is ready...\n");
        }
      }

      break;

    case UX_HID_CLIENT_REMOVAL:

      if ((VOID*)keyboard == client -> ux_host_class_hid_client_local_instance)
      {
        keyboard = UX_NULL;
        USBH_UsrLog("\nHID Client Keyboard Unplugged");
      }

      if ((VOID*)mouse == client -> ux_host_class_hid_client_local_instance)
      {
        mouse = UX_NULL;
        USBH_UsrLog("\nHID Client Mouse Unplugged");
      }

      break;

    default:
      break;
  }

  return status;
}

/* USBX_APP_Host_Init adapted to FS */
VOID USBX_APP_Host_Init(VOID)
{
  /* Initialize the LL driver (FS port) */
  MX_USB_OTG_FS_HCD_Init();

  /* Initialize the host controller driver and register F4 HCD handle */
  ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                             _ux_hcd_stm32_initialize, USB_OTG_FS_PERIPH_BASE,
                             (ULONG)&hhcd_USB_OTG_FS);

  /* Drive vbus */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt */
  HAL_HCD_Start(&hhcd_USB_OTG_FS);

  USBH_UsrLog(" **** USB OTG FS Dual_Class Host **** \n");
  USBH_UsrLog("USB Host library started.\n");
  USBH_UsrLog("Starting Application");
  USBH_UsrLog("Connect your HID or MSC Device");
}

/* Drive VBUS - application provides actual implementation for board */
void USBH_DriverVBUS(uint8_t state)
{
  /* Default implementation: application can override this function */
  if (state == USB_VBUS_TRUE)
  {
    /* USER: implement board-specific VBUS enable if needed */
  }
  else
  {
    /* USER: implement board-specific VBUS disable if needed */
  }
  HAL_Delay(200);
}
