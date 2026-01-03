/* USBX STM32 port adapter for STM32F4 (FS) */

#include "ux_api.h"
#include "ux_hcd_stm32.h"
#include "ux_system.h"

#include "usbh_core.h"
#include "usbh_def.h"

extern HCD_HandleTypeDef hhcd_USB_OTG_FS;

/* Weak VBUS drive stub - override in board code if needed */
__weak void USBH_DriverVBUS(uint8_t state)
{
    (void)state;
}

void MX_USB_OTG_FS_HCD_Init(void)
{
  /* If HAL init exists in usbh_conf.c, calling HAL_HCD_Init here is safe */
  if (HAL_HCD_Init(&hhcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
}

void USBX_APP_Host_Init(void)
{
  /* Initialize the low level HCD */
  MX_USB_OTG_FS_HCD_Init();

  /* Register the stm32 HCD with USBX using FS peripheral base and the hhcd handle */
  ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                             _ux_hcd_stm32_initialize,
                             (ULONG)USB_OTG_FS_PERIPH_BASE,
                             (ULONG)&hhcd_USB_OTG_FS);

  /* Drive VBUS (no-op by default) */
  USBH_DriverVBUS(1);

  /* Start the HCD */
  HAL_HCD_Start(&hhcd_USB_OTG_FS);
}
