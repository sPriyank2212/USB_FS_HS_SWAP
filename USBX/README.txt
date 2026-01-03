USBX integration notes for USB_FS_HS_SWAP project

Files added by the port helper:
 - USBX/Target/ux_stm32_config.h     (STM32F4 specific HCD channels)
 - USBX/Target/usbx_stm32_port.c      (adapter: MX_USB_OTG_FS_HCD_Init, USBX host init)
 - USBX/App/app_usbx_host.c           (minimal USBX host init wrapper)
 - USBX/App/app_usbx_host.h

Manual steps to enable USBX + ThreadX in CubeIDE:
1. In CubeIDE, open the project `.ioc` and enable Azure RTOS (ThreadX and USBX) under Middleware if available. Regenerate code.
2. If Azure RTOS packages are not installed in CubeIDE, either install them via Help -> Manage Embedded Software Packages, or manually add the sources:
   - Add `Middlewares/ST/usbx` to the project's include path and add the relevant sources (host controller files under `common/usbx_stm32_host_controllers`).
   - Add `Middlewares/ST/threadx` and `Middlewares/Third_Party/FATFS` as required.
3. Add the new `USBX` folder files into the project (right click project -> Add -> Existing Files) so they compile with the project.
4. Ensure the project's include paths contain the root of the repo `Middlewares/ST/usbx` and `Middlewares/Third_Party` so headers resolve.
5. In application code, call `MX_USBX_Host_Init(byte_pool_ptr)` from your ThreadX init to start USBX host.

Notes:
- The adapter uses `USB_OTG_FS_PERIPH_BASE` and the `hhcd_USB_OTG_FS` HCD handle from `USB_HOST/Target/usbh_conf.c`.
- VBUS control is a weak function `USBH_DriverVBUS` which does nothing by default; override it in board code if you need software VBUS control.

If you want, I can also update the CubeIDE project files to add the new sources automatically — say "also add to project" and I'll edit the `.cproject` entries.