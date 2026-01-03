#ifndef APP_USBX_HOST_H
#define APP_USBX_HOST_H

#include "ux_api.h"

#define USBX_HOST_MEMORY_STACK_SIZE (64 * 1024)

UINT MX_USBX_Host_Init(VOID *memory_ptr);

#endif /* APP_USBX_HOST_H */
