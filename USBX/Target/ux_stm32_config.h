/* USBX STM32 config header for STM32F4 */
#ifndef __UX_STM32_CONFIG_H__
#define __UX_STM32_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private defines -----------------------------------------------------------*/
/* FS controller uses 8 host channels on this board */
#define UX_HCD_STM32_MAX_NB_CHANNELS          8

#ifdef __cplusplus
}
#endif
#endif  /* __UX_STM32_CONFIG_H__ */
