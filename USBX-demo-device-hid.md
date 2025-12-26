# USBX HID Demos (Keyboard + Mouse)

## Index
- [HID Keyboard](#hid-keyboard-rtos--standalone)
- [HID Mouse](#hid-mouse-rtos--standalone)
- [HID Mouse + Keyboard](#hid-mouse--keyboard-rtos--standalone)
- [HID Custom](#hid-custom-rtos--standalone)

# HID Keyboard (RTOS & Standalone)
- **Purpose:** Demonstrates a USB HID Keyboard device implemented with Eclipse ThreadX USBX.
- **Variants:** [demo_device_hid_keyboard_rtos.c](../demo_device_hid_keyboard_rtos.c) and [demo_device_hid_keyboard_standalone.c](../demo_device_hid_keyboard_standalone.c).
- **Targets:** MCU platforms with a USB Device Controller (e.g., STM32H7). Platform glue is provided by your board layer via `board_setup()` and `usb_device_dcd_initialize()`.

  - [Overview](#hid-keyboard-overview)
  - [Build Prerequisites](#hid-keyboard-build-prerequisites)
  - [Descriptors](#hid-keyboard-descriptors)
  - [Initialization Flow](#hid-keyboard-init)
  - [Troubleshooting](#hid-keyboard-troubleshooting)

<a id="hid-keyboard-overview"></a>
## Overview
- **Device role:** Enumerates as a HID Keyboard on a host PC.
- **Typing demo:** Periodically sends key codes (6-key rollover array) to the host.
- **LED feedback:** Host can control LEDs (Num Lock, Caps Lock, etc.) via Output reports handled in callbacks.
- **Boot subclass:** Optional (`DEMO_HID_BOOT_DEVICE`); enable for BIOS/UEFI compatibility.

## Source Layout
- **RTOS:** [demo_device_hid_keyboard_rtos.c](../demo_device_hid_keyboard_rtos.c) — uses ThreadX (`tx_kernel_enter`, `tx_application_define`, `ux_utility_thread_create`).
- **Standalone:** [demo_device_hid_keyboard_standalone.c](../demo_device_hid_keyboard_standalone.c) — uses USBX standalone scheduler (`ux_system_tasks_run`) and `ux_demo_device_hid_task()` in the main loop.

<a id="hid-keyboard-build-prerequisites"></a>
## Build Prerequisites

### Common:
- Define `UX_DEVICE_SIDE_ONLY`.
- Provide platform functions: `board_setup()` and `usb_device_dcd_initialize(void*)` (DCD/PHY clocks, GPIO, interrupts, VBUS, etc.).
- Link against Eclipse ThreadX USBX device stack and the device controller driver for your MCU.

### RTOS variant:
- ThreadX kernel port for your MCU; call `tx_kernel_enter()` in `main()` and implement `tx_application_define()` to initialize USBX and start the HID thread.

### Standalone variant:
- Define `UX_STANDALONE`.
- Set `UX_PERIODIC_RATE` to 1000 (1ms tick) in your configuration if not already.
- In `main()`, run `ux_system_tasks_run()` repeatedly and call `ux_demo_device_hid_task()`.

<a id="hid-keyboard-descriptors"></a>
## Descriptors (Detailed)

### USB Descriptor Summary

- **VID/PID:** 0x070A / 0x4090 (sample values; adjust for your product).
- **EP0:** 64 bytes (`UX_DEMO_MAX_EP0_SIZE = 0x40`).
- **Configuration:** 1 interface, self-powered, max power 100 mA.
- **Interface:** HID, Subclass Boot (optional), Protocol 0x01 (Keyboard).
- **Endpoint:** Interrupt IN `0x81`, packet size 8, `bInterval = 8` (FS 8ms/HS 16ms effective).
- **HID Report:** 8-bit modifiers, 1 reserved byte, 6-key rollover array; LED Output bits with padding.

### HID Report Descriptor
- Modifiers: 8 bits (Left/Right Ctrl/Shift/Alt/GUI), logical 0..1.
- Reserved: 1 byte.
- Key Array: 6 bytes (6-key rollover), key codes in Usage Page Keyboard (0..101).
- LED Output: 5 bits (Num, Caps, Scroll, Compose, Kana), then 3 bits padding to byte align.
- Resulting report layouts:
    - Input (to host): [Byte0] Modifiers, [Byte1] Reserved, [Byte2..7] 6-key array.
    - Output (from host): [Byte0] LED bits (Num=bit0, Caps=bit1, …), [Byte1] padding.

<details><summary><b>Keyboard HID report bytes</b></summary>

```c
/* hid_keyboard_report[] */
0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
0x09, 0x06,       // USAGE (Keyboard)
0xA1, 0x01,       // COLLECTION (Application)
  /* Modifiers */
  0x05, 0x07,     //   USAGE_PAGE (Key Codes)
  0x19, 0xE0,     //   USAGE_MINIMUM (Left Control)
  0x29, 0xE7,     //   USAGE_MAXIMUM (Right GUI)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,     //   REPORT_SIZE (1)
  0x95, 0x08,     //   REPORT_COUNT (8) -> 8 modifiers
  0x81, 0x02,     //   INPUT (Data,Var,Abs)
  /* Reserved byte */
  0x75, 0x08,     //   REPORT_SIZE (8)
  0x95, 0x01,     //   REPORT_COUNT (1)
  0x81, 0x01,     //   INPUT (Constant)
  /* Key array (6-key rollover) */
  0x05, 0x07,     //   USAGE_PAGE (Key Codes)
  0x19, 0x00,     //   USAGE_MINIMUM (0)
  0x29, 0x65,     //   USAGE_MAXIMUM (101)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x25, 0x65,     //   LOGICAL_MAXIMUM (101)
  0x75, 0x08,     //   REPORT_SIZE (8)
  0x95, 0x06,     //   REPORT_COUNT (6)
  0x81, 0x00,     //   INPUT (Data, Array)
  /* LED outputs */
  0x05, 0x08,     //   USAGE_PAGE (LEDs)
  0x19, 0x01,     //   USAGE_MINIMUM (Num Lock)
  0x29, 0x05,     //   USAGE_MAXIMUM (Kana)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,     //   REPORT_SIZE (1)
  0x95, 0x05,     //   REPORT_COUNT (5)
  0x91, 0x02,     //   OUTPUT (Data,Var,Abs)
  /* LED padding */
  0x75, 0x03,     //   REPORT_SIZE (3)
  0x95, 0x01,     //   REPORT_COUNT (1)
  0x91, 0x01,     //   OUTPUT (Constant)
0xC0              // END_COLLECTION
```

</details>

### USB Device/Configuration Descriptors
- Device descriptor:
    - `bcdUSB = 0x0200` (USB 2.0), class/subclass/protocol = 0 (interface-defined).
    - `bMaxPacketSize0 = 64`.
    - `idVendor = 0x070A`, `idProduct = 0x4090` (demo values), `iManufacturer = 1`, `iProduct = 2`, `iSerialNumber = 3`.
    - `bNumConfigurations = 1`.
- Device qualifier descriptor (HS build): mirrors device characteristics for the other speed.
- Configuration descriptor:
    - `wTotalLength = 0x22` (34 bytes across config + interface + HID + endpoint).
    - `bmAttributes = 0xC0` (self-powered), `bMaxPower = 0x32` (100 mA units of 2 mA).
    - `bNumInterfaces = 1`, `bConfigurationValue = 1`.
- Interface descriptor:
    - `bInterfaceClass = 0x03` (HID), `bInterfaceSubClass` = Boot if `DEMO_HID_BOOT_DEVICE`, `bInterfaceProtocol = 0x01` (Keyboard).
    - `bNumEndpoints = 1` (INT IN).
- HID descriptor:
    - `bcdHID = 0x0110` (HID 1.11), `bCountryCode = 33` (US), `bNumDescriptors = 1`.
    - Report descriptor type `0x22`, length matches `sizeof(hid_keyboard_report)`.
- Endpoint descriptor (Interrupt IN):
    - `bEndpointAddress = 0x81` (IN, EP1), `bmAttributes = 0x03` (Interrupt).
    - `wMaxPacketSize = 8` bytes.
    - `bInterval = 8` → FS: 8 ms; HS: 16 ms.

<details><summary><b>USB Device descriptor framework (FS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x07,       // idVendor  (0x070A)
0x90, 0x40,       // idProduct (0x4090)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x04,             // iConfiguration
0xC0,             // bmAttributes (self-powered)
0x32,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x01,             // bInterfaceSubClass (Boot) if enabled
0x01,             // bInterfaceProtocol (Keyboard)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted here: LSB,MSB at build time */
0x00, 0x00,

/* Endpoint (IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<details><summary><b>USB Device descriptor framework (HS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x07,       // idVendor  (0x070A)
0x90, 0x40,       // idProduct (0x4090)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Device Qualifier */
0x0A,             // bLength
0x06,             // bDescriptorType (Device Qualifier)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x01,             // bNumConfigurations
0x00,             // bReserved

/* HS Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x05,             // iConfiguration (sample HS index)
0xC0,             // bmAttributes (self-powered)
0x19,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x01,             // bInterfaceSubClass (Boot) if enabled
0x01,             // bInterfaceProtocol (Keyboard)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted by build: LSB,MSB */
0x00, 0x00,

/* Endpoint (IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<a id="hid-keyboard-init"></a>
## Initialization Flow
### RTOS
- `board_setup()` → clocks, pins, cache, UART (optional), USB power.
- `tx_kernel_enter()` → ThreadX start.
- `tx_application_define()`:
    - `ux_system_initialize()` with a static pool (`UX_DEVICE_MEMORY_STACK_SIZE`, default 7 KB).
    - `ux_device_stack_initialize()` with HS/FS device + string frameworks.
    - HID class register: `_ux_system_slave_class_hid_name` → `ux_device_class_hid_entry` with callbacks.
    - Create demo thread → `ux_demo_device_hid_thread_entry()`.
- In thread:
    - `usb_device_dcd_initialize(UX_NULL)` to register the DCD.
    - When configured (`UX_DEVICE_CONFIGURED`) and instance ready, send keyboard input reports.

### Standalone
- `board_setup()`.
- `ux_application_define()`:
    - Same USBX init + HID registration as RTOS variant.
    - `usb_device_dcd_initialize(UX_NULL)`.
- `main()` loop:
    - `ux_system_tasks_run()`; call `ux_demo_device_hid_task()` to emit keyboard input reports when configured.

## User Callback Handlers

- **`ux_demo_device_hid_instance_activate`:** Stores the class instance pointer on activation so the app can send input reports via `ux_device_class_hid_event_set()` once the device is configured.
- **`ux_demo_device_hid_instance_deactivate`:** Clears the stored instance on deactivation to prevent sending after disconnect or configuration change.
- **`ux_demo_device_hid_callback`:** Handles host-to-device HID Output/Feature reports. Parse LED bits in `event->ux_device_class_hid_event_buffer[0]` and update flags (e.g., `caps_lock_flag`, `num_lock_flag`).
- **`ux_demo_device_hid_get_callback`:** Provides device-to-host data for GET_REPORT if needed; usually not required for boot keyboards.
- **`ux_demo_error_callback`:** Registered via `ux_utility_error_callback_register()` for diagnostics.

Notes:
- The sample sets ux_device_class_hid_parameter_report_id = UX_FALSE (single report, no Report ID byte). If you enable report IDs, ensure buffers account for the extra ID prefix.
- Keep callbacks non-blocking; offload work to a thread/task or main loop.

## Demo configuration
- `UX_DEVICE_MEMORY_STACK_SIZE` (default 7*1024).
- `DEMO_HID_BOOT_DEVICE`: enable for BIOS/UEFI compatibility.
- Key cadence/delay: adjust demo timing (e.g., periodic delay) to control typing rate.

## Demo configuration optimization
- To optimize your application, user can flow this defines config in `ux_user.h`

```c
#define UX_DEVICE_ENDPOINT_BUFFER_OWNER    1
#define UX_DEVICE_CLASS_HID_ZERO_COPY

#define UX_DEVICE_CLASS_HID_EVENT_BUFFER_LENGTH         8
#define UX_DEVICE_CLASS_HID_MAX_EVENTS_QUEUE            2

#define UX_DEVICE_ALTERNATE_SETTING_SUPPORT_DISABLE

#define UX_DEVICE_INITIALIZE_FRAMEWORK_SCAN_DISABLE
#define UX_MAX_DEVICE_ENDPOINTS                         1 /* Interrupt endpoint.  */
#define UX_MAX_DEVICE_INTERFACES                        1 /* HID interface.  */

#define UX_MAX_SLAVE_INTERFACES                         1
#define UX_MAX_SLAVE_CLASS_DRIVER                       1

#define UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH             64 /* > 62 for descriptors enumeration.  */
#define UX_SLAVE_REQUEST_DATA_MAX_LENGTH                8

#define UX_NAME_REFERENCED_BY_POINTER
```

## Expected Behavior
- On connection to a PC, the device enumerates as “HID Keyboard Demo”.
- Periodically sends key codes (e.g., 'a' key code `0x04`) in the input report; host displays characters accordingly.
- Caps/Num Lock LEDs update on the device when the host toggles them via Output report.

## Porting Notes
- Ensure your DCD driver and low-level BSP match your MCU and USB instance (FS/HS, ULPI/Embedded PHY).
- Provide proper NVIC priorities and ISR bindings for the DCD driver.
- For HS with external PHY, verify 60 MHz ULPI clock and VBUS sensing.

## Build & Run (Typical STM32 example)
- Toolchain/IDE: IAR EWARM, Keil uVision, or Arm GCC + CMake/Ninja.
- Include ThreadX and USBX sources/ports for your Cortex-M core.
- Add one of the demo sources to your application and ensure:
    - `UX_DEVICE_SIDE_ONLY` (and `UX_STANDALONE` for the standalone variant).
    - Board layer implements `board_setup()` and `usb_device_dcd_initialize()`.

Example minimal main (RTOS variant is self-contained in the sample): see [demo_device_hid_keyboard_rtos.c](../demo_device_hid_keyboard_rtos.c).

<a id="hid-keyboard-troubleshooting"></a>
## Troubleshooting
- Not enumerating:
    - Check D+ (FS) or HS PHY signals and power; verify `usb_device_dcd_initialize()` registers the controller.
    - Confirm `UX_DEVICE_CONFIGURED` becomes true; inspect `ux_demo_error_callback()` logs.
- No characters or incorrect keys:
    - Verify key codes in the 6-byte array and modifier bits; ensure correct HID Usage IDs.
    - Ensure report length and endpoint configuration match the host expectations.
- LED not toggling:
    - Confirm Output report handling in `ux_demo_device_hid_callback()` updates LED flags and any GPIOs.

# HID Mouse (RTOS & Standalone)
- **Purpose:** Demonstrates a USB HID Mouse device implemented with Eclipse ThreadX USBX.
- **Variants:** [demo_device_hid_mouse_rtos.c](https://github.com/ayedm1/eclipse_usbx_examples/blob/9d5fcb504ac511afcbc5346b6e8ff2ca5e20393c/demo_device_hid_mouse_rtos.c) and [demo_device_hid_mouse_standalone.c](https://github.com/ayedm1/eclipse_usbx_examples/blob/9d5fcb504ac511afcbc5346b6e8ff2ca5e20393c/demo_device_hid_mouse_standalone.c).
- **Targets:** MCU platforms with a USB Device Controller (e.g., STM32H7). Platform glue is provided by your board layer via `board_setup()` and `usb_device_dcd_initialize()`.

  - [Overview](#hid-mouse-overview)
  - [Descriptors](#hid-mouse-descriptors)
  - [Initialization Flow](#hid-mouse-init)
  - [Troubleshooting](#hid-mouse-troubleshooting)
  - [Run Demo](#hid-mouse-run-demo)

<a id="hid-mouse-overview"></a>
## Overview
- **Device role:** Enumerates as a HID Mouse on a host PC.
- **Movement demo:** Generates cursor movement automatically in a square pattern.
- **Optional absolute mode:** Define `UX_DEMO_MOUSE_ABSOLUTE` to switch from relative to absolute XY reporting.
- **Boot subclass:** Enabled by default (`DEMO_HID_BOOT_DEVICE`).

## Source Layout
- **RTOS:** [demo_device_hid_mouse_rtos.c](https://github.com/ayedm1/eclipse_usbx_examples/blob/9d5fcb504ac511afcbc5346b6e8ff2ca5e20393c/demo_device_hid_mouse_rtos.c) — uses ThreadX (`tx_kernel_enter`, `tx_application_define`, `ux_utility_thread_create`).
- **Standalone:** [demo_device_hid_mouse_standalone.c](https://github.com/ayedm1/eclipse_usbx_examples/blob/9d5fcb504ac511afcbc5346b6e8ff2ca5e20393c/demo_device_hid_mouse_standalone.c) — uses USBX standalone scheduler (`ux_system_tasks_run`) and `ux_demo_device_hid_task()` in the main loop.

## Build Prerequisites

### Common:
  - Define `UX_DEVICE_SIDE_ONLY`.
  - Provide platform functions: `board_setup()` and `usb_device_dcd_initialize(void*)` (DCD/PHY clocks, GPIO, interrupts, VBUS, etc.).
  - Link against Eclipse ThreadX USBX device stack and the device controller driver for your MCU.

### RTOS variant:
  - ThreadX kernel port for your MCU; call `tx_kernel_enter()` in `main()` and implement `tx_application_define()` to initialize USBX and start the HID thread.

### Standalone variant:
  - Define `UX_STANDALONE`.
  - Set `UX_PERIODIC_RATE` to 1000 (1ms tick) in your configuration if not already.
  - In `main()`, run `ux_system_tasks_run()` repeatedly and call `ux_demo_device_hid_task()`.

## USB Descriptor Summary

- **VID/PID:** 0x090A / 0x4036 (sample values; adjust for your product).
- **EP0:** 64 bytes (`UX_DEMO_MAX_EP0_SIZE = 0x40`).
- **Configuration:** 1 interface, self-powered, max power 100 mA.
- **Interface:** HID, Subclass Boot (default), Protocol 0x02 (Mouse).
- **Endpoint:** Interrupt IN `0x81`, packet size 8, `bInterval = 8` (FS 8ms/HS 1ms effective with HS microframes).
- **HID Report:** 3 buttons + X/Y + wheel. Relative by default; absolute if `UX_DEMO_MOUSE_ABSOLUTE` is defined.

<a id="hid-mouse-descriptors"></a>
## Descriptors (Detailed)

- ### HID Report Descriptor:
  - Usage page/collection: Generic Desktop → Mouse → Pointer (Application + Physical collections).
  - Buttons: Usage min/max 1..3, logical 0..1, report size 1, count 3 → 3 button bits, followed by 5 padding bits.
  - X/Y axes:
    - Relative mode (default): logical −127..127, report size 8, count 2 → 1 byte X + 1 byte Y, both Relative.
    - Absolute mode (`UX_DEMO_MOUSE_ABSOLUTE`): logical 0..32767, report size 16, count 2 → 2 bytes X (LSB/MSB) + 2 bytes Y, Absolute.
  - Wheel: logical −127..127, report size 8, count 1 → 1 byte Wheel, Relative.
  - Resulting input report layout:
    - Relative: [Byte0] Buttons (3 bits) + 5 pad, [Byte1] X, [Byte2] Y, [Byte3] Wheel.
    - Absolute: [Byte0] Buttons (3 bits) + 5 pad, [Byte1..2] X (LSB,MSB), [Byte3..4] Y (LSB,MSB), [Byte5] Wheel.

<details><summary><b>Relative mode (default)</b></summary>

```c
/* hid_mouse_report[] (relative XY) */
0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
0x09, 0x02,       // USAGE (Mouse)
0xA1, 0x01,       // COLLECTION (Application)
  0x09, 0x01,     //   USAGE (Pointer)
  0xA1, 0x00,     //   COLLECTION (Physical)
    /* 3 buttons */
    0x05, 0x09,   //     USAGE_PAGE (Button)
    0x19, 0x01,   //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,   //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,   //     LOGICAL_MINIMUM (0)
    0x25, 0x01,   //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,   //     REPORT_SIZE (1)
    0x95, 0x03,   //     REPORT_COUNT (3) -> 3 buttons
    0x81, 0x02,   //     INPUT (Data,Var,Abs) -> Buttons
    /* padding to next byte */
    0x75, 0x05,   //     REPORT_SIZE (5)
    0x95, 0x01,   //     REPORT_COUNT (1)
    0x81, 0x03,   //     INPUT (Const,Var,Abs) -> Padding bits
    /* X, Y relative */
    0x05, 0x01,   //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,   //     USAGE (X)
    0x09, 0x31,   //     USAGE (Y)
    0x15, 0x81,   //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,   //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,   //     REPORT_SIZE (8)
    0x95, 0x02,   //     REPORT_COUNT (2) -> X,Y
    0x81, 0x06,   //     INPUT (Data,Var,Rel) -> X,Y are relative
    /* Wheel */
    0x09, 0x38,   //     USAGE (Wheel)
    0x15, 0x81,   //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,   //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,   //     REPORT_SIZE (8)
    0x95, 0x01,   //     REPORT_COUNT (1) -> Wheel
    0x81, 0x06,   //     INPUT (Data,Var,Rel) -> Wheel
  0xC0,           //   END_COLLECTION
0xC0              // END_COLLECTION
```
   </details>

<details><summary><b>Absolute mode (`UX_DEMO_MOUSE_ABSOLUTE` defined)</b></summary>

```c
/* hid_mouse_report[] (absolute XY) */
0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
0x09, 0x02,       // USAGE (Mouse)
0xA1, 0x01,       // COLLECTION (Application)
  0x09, 0x01,     //   USAGE (Pointer)
  0xA1, 0x00,     //   COLLECTION (Physical)
    /* 3 buttons */
    0x05, 0x09,   //     USAGE_PAGE (Button)
    0x19, 0x01,   //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,   //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,   //     LOGICAL MINIMUM (0)
    0x25, 0x01,   //     LOGICAL MAXIMUM (1)
    0x75, 0x01,   //     REPORT_SIZE (1)
    0x95, 0x03,   //     REPORT_COUNT (3)
    0x81, 0x02,   //     INPUT (Data,Var,Abs)
    /* padding */
    0x75, 0x05,   //     REPORT_SIZE (5)
    0x95, 0x01,   //     REPORT COUNT (1)
    0x81, 0x03,   //     INPUT (Const,Var,Abs)
    /* X, Y absolute (16-bit each) */
    0x05, 0x01,   //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,   //     USAGE (X)
    0x09, 0x31,   //     USAGE (Y)
    0x16, 0x00, 0x00, //  LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x7F, //  LOGICAL_MAXIMUM (32767)
    0x75, 0x10,   //     REPORT_SIZE (16)
    0x95, 0x02,   //     REPORT_COUNT (2) -> X,Y
    0x81, 0x02,   //     INPUT (Data,Var,Abs) -> Absolute X,Y
    /* Wheel */
    0x09, 0x38,   //     USAGE (Wheel)
    0x15, 0x81,   //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,   //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,   //     REPORT_SIZE (8)
    0x95, 0x01,   //     REPORT_COUNT (1)
    0x81, 0x06,   //     INPUT (Data,Var,Rel)
  0xC0,           //   END_COLLECTION
0xC0              // END_COLLECTION
```
   </details>


## USB Device/Configuration Descriptors

  - Device descriptor:
    - `bcdUSB = 0x0200` (USB 2.0), class/subclass/protocol = 0 (interface-defined).
    - `bMaxPacketSize0 = 64`.
    - `idVendor = 0x090A`, `idProduct = 0x4036` (demo values), `iManufacturer = 1`, `iProduct = 2`, `iSerialNumber = 3`.
    - `bNumConfigurations = 1`.
  - Device qualifier descriptor (HS build): mirrors device characteristics for the other speed.
  - Configuration descriptor:
    - `wTotalLength = 0x22` (34 bytes across config + interface + HID + endpoint).
    - `bmAttributes = 0xC0` (self-powered), `bMaxPower = 0x32` (100 mA units of 2 mA).
    - `bNumInterfaces = 1`, `bConfigurationValue = 1`.
  - Interface descriptor:
    - `bInterfaceClass = 0x03` (HID), `bInterfaceSubClass = 0x01` (Boot if `DEMO_HID_BOOT DEVICE`), `bInterfaceProtocol = 0x02` (Mouse).
    - `bNumEndpoints = 1` (INT IN).
  - HID descriptor:
    - `bcdHID = 0x0110` (HID 1.11), `bCountryCode = 33` (US), `bNumDescriptors = 1`.
    - Report descriptor type `0x22`, length matches `sizeof(hid_mouse_report)`.
  - Endpoint descriptor (Interrupt IN):
    - `bEndpointAddress = 0x81` (IN, EP1), `bmAttributes = 0x03` (Interrupt).
    - `wMaxPacketSize = 8` bytes.
    - `bInterval = 8` → FS: 8 ms; HS: 2^(8−1) × 125 µs = 16 ms.

<details><summary><b>USB Device descriptor framework (FS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x09,       // idVendor  (0x090A)
0x36, 0x40,       // idProduct (0x4036)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x04,             // iConfiguration
0xC0,             // bmAttributes (self-powered)
0x32,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x01,             // bInterfaceSubClass (Boot) if enabled
0x02,             // bInterfaceProtocol (Mouse)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted here: LSB,MSB at build time */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```
   </details>

<details><summary><b>USB Device descriptor framework (HS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x09,       // idVendor  (0x090A)
0x36, 0x40,       // idProduct (0x4036)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Device Qualifier */
0x0A,             // bLength
0x06,             // bDescriptorType (Device Qualifier)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x01,             // bNumConfigurations
0x00,             // bReserved

/* HS Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x05,             // iConfiguration (sample HS index)
0xC0,             // bmAttributes (self-powered)
0x19,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x01,             // bInterfaceSubClass (Boot) if enabled
0x02,             // bInterfaceProtocol (Mouse)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted by build: LSB,MSB */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<a id="hid-mouse-init"></a>
## Initialization Flow
### RTOS
- `board_setup()` → clocks, pins, cache, UART (optional), USB power.
- `tx_kernel_enter()` → ThreadX start.
- `tx_application_define()`:
  - `ux_system_initialize()` with a static pool (`UX_DEVICE_MEMORY STACK_SIZE`, default 7 KB).
  - `ux_device_stack_initialize()` with HS/FS device + string frameworks.
  - HID class register: `_ux_system_slave_class_hid_name` → `ux_device_class_hid_entry` with callbacks.
  - Create demo thread → `ux_demo_device_hid_thread_entry()`.
- In thread:
  - `usb_device_dcd_initialize(UX_NULL)` to register the DCD.
  - When configured (`UX_DEVICE_CONFIGURED`) and instance ready, send events via `ux_device_class_hid_event_set()`.

### Standalone
- `board_setup()`.
- `ux_application_define()`:
  - Same USBX init + HID registration as RTOS variant.
  - `usb_device_dcd_initialize(UX_NULL)`.
- `main()` loop:
  - `ux_system_tasks_run()`; call `ux_demo_device_hid_task()` to emit HID events when configured.

## User Callback Handlers

- **`ux_demo_device_hid_instance_activate`:** Stores the class instance pointer on activation so the app can send input reports via `ux_device_class_hid_event_set()` once the device is configured.
- **`ux_demo_device_hid_instance_deactivate`:** Clears the stored instance on deactivation to prevent sending after disconnect or configuration change.
- **`ux_demo_device_hid_callback`:** Handles host-to-device HID transfers (e.g., SET_REPORT for Output/Feature reports). The sample returns `UX_SUCCESS` without handling; adapt to parse `event->ux_device_class_hid_event_buffer` for custom outputs (LEDs, settings).
- **`ux_demo_device_hid_get_callback`:** Provides device-to-host data for GET_REPORT requests if your device supports Feature/Input fetches outside interrupt IN. Sample returns `UX_SUCCESS` without populating; implement to fill `event->ux_device_class_hid_event_buffer` and set `event->ux_device_class_hid_event_length` appropriately.
- **`ux_demo_error_callback`:** Registered via `ux_utility_error_callback_register()`. Use to log and diagnose stack/DCD errors; correlate `level/context/code` with constants in `ux_api.h`.

Notes:
- The sample sets `ux_device_class_hid_parameter_report_id = UX_FALSE` (single report, no Report ID byte). If you enable report IDs, ensure buffers account for the extra ID prefix.
- Keep callbacks non-blocking. For longer work, signal a worker thread/task (RTOS) or defer to your main loop (standalone).

## Demo configuration
- `UX_DEVICE_MEMORY_STACK_SIZE` (default 7*1024).
- `DEMO_HID_BOOT_DEVICE`: keep enabled for BIOS/UEFI compatibility.
- `UX_DEMO_HID_MOUSE_CURSOR_MOVE` and `_N`: tune speed/pattern.
- `UX_DEMO_MOUSE_ABSOLUTE`: switch to absolute XY; demo draws a rectangle.

## Demo configuration optimization
- To optimize your application, user can flow this defines config in `ux_user.h`

```c
#define UX_DEVICE_ENDPOINT_BUFFER_OWNER    1
#define UX_DEVICE_CLASS_HID_ZERO_COPY

#define UX_DEVICE CLASS_HID_EVENT_BUFFER_LENGTH         8
#define UX_DEVICE CLASS_HID_MAX_EVENTS_QUEUE            2

#define UX_DEVICE_ALTERNATE_SETTING_SUPPORT_DISABLE

#define UX_DEVICE_INITIALIZE_FRAMEWORK_SCAN_DISABLE
#define UX_MAX_DEVICE_ENDPOINTS                         1 /* Interrupt endpoint.  */
#define UX_MAX_DEVICE_INTERFACES                        1 /* HID interface.  */

#define UX_MAX_SLAVE_INTERFACES                         1
#define UX_MAX_SLAVE_CLASS_DRIVER                       1

#define UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH             64 /* > 62 for descriptors enumeration.  */
#define UX_SLAVE_REQUEST_DATA_MAX_LENGTH                8

#define UX_NAME_REFERENCED_BY_POINTER
```

## Expected Behavior
- On connection to a PC, the device enumerates as “HID Mouse Demo”.
- Cursor moves automatically. In absolute mode, it traces a rectangle.
- Wheel and buttons are included in the report; the sample does not press them by default.

## Porting Notes
- Ensure your DCD driver and low-level BSP match your MCU and USB instance (FS/HS, ULPI/Embedded PHY).
- Provide proper NVIC priorities and ISR bindings for the DCD driver.
- For HS with external PHY, verify 60 MHz ULPI clock and VBUS sensing.

## Build & Run (Typical STM32 example)
- Toolchain/IDE: IAR EWARM, Keil uVision, or Arm GCC + CMake/Ninja.
- Include ThreadX and USBX sources/ports for your Cortex-M core.
- Add one of the demo sources to your application and ensure:
  - `UX_DEVICE_SIDE_ONLY` (and `UX_STANDALONE` for the standalone variant).
  - Board layer implements `board_setup()` and `usb_device_dcd_initialize()`.

Example minimal main (RTOS variant is already self-contained in the sample): see [demo_device_hid_mouse_rtos.c](https://github.com/ayedm1/eclipse_usbx_examples/blob/9d5fcb504ac511afcbc5346b6e8ff2ca5e20393c/demo_device_hid_mouse_rtos.c).

<a id="hid-mouse-troubleshooting"></a>
## Troubleshooting
- Not enumerating:
  - Check D+ (FS) or HS PHY signals and power; verify `usb_device_dcd_initialize()` actually registers the controller.
  - Confirm `UX_DEVICE_CONFIGURED` becomes true; otherwise inspect `ux_demo_error_callback()` logs.
- Slow/jerky movement:
  - Confirm `bInterval` and that the device is on a HS/FS port as expected.
  - Adjust `UX_DEMO_HID_MOUSE_CURSOR_MOVE` and `_N`.
- Standalone timing:
  - Ensure `UX_PERIODIC_RATE == 1000` and that `ux_system_tasks_run()` is called frequently (e.g., every main loop iteration).

<a id="hid-mouse-run-demo"></a>
## Running the Demo Application

The mouse demo does not require anything extra on the PC. You just need to plug the HID device running the mouse demo to the PC and see the screen cursor moving.

# HID Mouse + Keyboard (RTOS & Standalone)
- Shortcuts: [Back to Index](#index) • [Overview](#hid-combo-overview) • [Descriptors](#hid-combo-descriptors) • [Init (RTOS)](#hid-combo-init-rtos) • [Init (Standalone)](#hid-combo-init-standalone) • [Troubleshooting](#hid-combo-troubleshooting)
- **Purpose:** Demonstrates a single USB HID device exposing both Mouse and Keyboard functions using multiple Report IDs.
- **Variants:** [demo_device_hid_mouse_keyboard_rtos.c](../demo_device_hid_mouse_keyboard_rtos.c) and [demo_device_hid_mouse_keyboard_standalone.c](../demo_device_hid_mouse_keyboard_standalone.c).
- **Targets:** MCU platforms with a USB Device Controller (e.g., STM32H7). Platform glue is provided by your board layer via `board_setup()` and `usb_device_dcd_initialize()`.

<a id="hid-combo-overview"></a>
## Overview
- **Device role:** Enumerates as one HID interface with two reports: Keyboard (Report ID 1) and Mouse (Report ID 2).
- **Keyboard demo:** Periodically sends lowercase key strokes (6-key rollover array).
- **Mouse demo:** Moves cursor in a square pattern with relative X/Y and wheel.
- **Report IDs:** Required and used; ensure HID class is configured to handle report IDs.
- **Boot subclass:** Optional (`DEMO_HID_BOOT_DEVICE`); protocol is `0x00` (none) in this combined sample.

## Source Layout
- **RTOS:** [demo_device_hid_mouse_keyboard_rtos.c](../demo_device_hid_mouse_keyboard_rtos.c) — uses ThreadX (`tx_kernel_enter`, `tx_application_define`, HID thread).
- **Standalone:** [demo_device_hid_mouse_keyboard_standalone.c](../demo_device_hid_mouse_keyboard_standalone.c) — uses USBX standalone scheduler (`ux_system_tasks_run`) and `ux_demo_device_hid_task()`.

## Build Prerequisites

### Common:
- Define `UX_DEVICE_SIDE_ONLY`.
- Provide platform functions: `board_setup()` and `usb_device_dcd_initialize(void*)` (DCD/PHY clocks, GPIO, interrupts, VBUS, etc.).
- Link against Eclipse ThreadX USBX device stack and the device controller driver for your MCU.
- Ensure `UX_DEVICE_CLASS_HID_EVENT_BUFFER_LENGTH >= 9` (Report ID + keyboard reserved + keys).

### RTOS variant:
- ThreadX kernel port for your MCU; call `tx_kernel_enter()` in `main()` and implement `tx_application_define()` to initialize USBX and start the HID thread.

### Standalone variant:
- Define `UX_STANDALONE`.
- Set `UX_PERIODIC_RATE` to 1000 (1ms tick) in your configuration.
- In `main()`, run `ux_system_tasks_run()` repeatedly and call `ux_demo_device_hid_task()`.

## USB Descriptor Summary

- **VID/PID:** 0x090A / 0x4036 (sample values; adjust for your product).
- **EP0:** 64 bytes (`UX_DEMO_MAX_EP0_SIZE = 0x40`).
- **Configuration:** 1 interface, self-powered, max power 100 mA.
- **Interface:** HID, Subclass Boot optional, Protocol 0x00 (None; multiple report IDs).
- **Endpoint:** Interrupt IN `0x81`, packet size 8, `bInterval = 8` (FS 8ms/HS 16ms effective).
- **HID Report:** Two reports:
  - Report ID 1: Keyboard — 8-bit modifiers, 1 reserved byte, 6-key rollover array.
  - Report ID 2: Mouse — 3 buttons + X/Y + wheel (relative).

<a id="hid-combo-descriptors"></a>
## Descriptors (Detailed)

### HID Report Descriptor (combined)
- Application collections for Keyboard and Mouse, each prefixed with a `REPORT_ID` item.
- Keyboard: Usage Page Keyboard, modifiers (8 bits), reserved (8 bits), 6-key array (Usage IDs 0..101).
- Mouse: Buttons (3 bits + 5 pad), X/Y/Wheel (−127..127, relative).
- Resulting input report layouts:
  - Keyboard (ID=1): [ID][Modifiers][Reserved][Key0..Key5] → 1 + 1 + 1 + 6 = 9 bytes.
  - Mouse (ID=2): [ID][Buttons/pad][X][Y][Wheel] → 1 + 1 + 1 + 1 + 1 = 5 bytes.

<details><summary><b>Combined HID report bytes</b></summary>

```c
/* hid_report[] */
// Report ID 1: Keyboard
0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
0x09, 0x06,       // USAGE (Keyboard)
0xA1, 0x01,       // COLLECTION (Application)
  0x85, 0x01,     //   REPORT_ID (1)
  0x05, 0x07,     //   USAGE_PAGE (Keyboard)
  0x19, 0xE0,     //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xE7,     //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,     //   REPORT_SIZE (1)
  0x95, 0x08,     //   REPORT_COUNT (8)
  0x81, 0x02,     //   INPUT (Data,Var,Abs)
  0x95, 0x01,     //   REPORT_COUNT (1)
  0x75, 0x08,     //   REPORT_SIZE (8)
  0x81, 0x03,     //   INPUT (Const,Var,Abs)
  0x95, 0x06,     //   REPORT_COUNT (6)
  0x75, 0x08,     //   REPORT_SIZE (8)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x25, 0x65,     //   LOGICAL_MAXIMUM (101)
  0x05, 0x07,     //   USAGE_PAGE (Keyboard)
  0x19, 0x00,     //   USAGE_MINIMUM (0)
  0x29, 0x65,     //   USAGE_MAXIMUM (101)
  0x81, 0x00,     //   INPUT (Data,Array)
0xC0,             // END_COLLECTION

// Report ID 2: Mouse
0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
0x09, 0x02,       // USAGE (Mouse)
0xA1, 0x01,       // COLLECTION (Application)
  0x85, 0x02,     //   REPORT_ID (2)
  0x09, 0x01,     //   USAGE (Pointer)
  0xA1, 0x00,     //   COLLECTION (Physical)
    0x05, 0x09,   //     USAGE_PAGE (Button)
    0x19, 0x01,   //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,   //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,   //     LOGICAL_MINIMUM (0)
    0x25, 0x01,   //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,   //     REPORT_COUNT (3)
    0x75, 0x01,   //     REPORT_SIZE (1)
    0x81, 0x02,   //     INPUT (Data,Var,Abs)
    0x95, 0x01,   //     REPORT_COUNT (1)
    0x75, 0x05,   //     REPORT_SIZE (5)
    0x81, 0x03,   //     INPUT (Const,Var,Abs)
    0x05, 0x01,   //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,   //     USAGE (X)
    0x09, 0x31,   //     USAGE (Y)
    0x09, 0x38,   //     USAGE (Wheel)
    0x15, 0x81,   //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,   //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,   //     REPORT_SIZE (8)
    0x95, 0x03,   //     REPORT_COUNT (3)
    0x81, 0x06,   //     INPUT (Data,Var,Rel)
  0xC0,           //   END_COLLECTION
0xC0              // END_COLLECTION
```

</details>

### USB Device/Configuration Descriptors
- Device descriptor:
  - `bcdUSB = 0x0200`, class/subclass/protocol = 0.
  - `bMaxPacketSize0 = 64`.
  - `idVendor = 0x090A`, `idProduct = 0x4036`.
  - `bNumConfigurations = 1`.
- Device qualifier descriptor (HS build): mirrors device characteristics for the other speed.
- Configuration descriptor:
  - `wTotalLength = 0x22` (34 bytes across config + interface + HID + endpoint).
  - `bmAttributes = 0xC0` (self-powered), `bMaxPower = 0x32` (100 mA).
  - `bNumInterfaces = 1`, `bConfigurationValue = 1`.
- Interface descriptor:
  - `bInterfaceClass = 0x03` (HID), `bInterfaceSubClass` Boot if enabled, `bInterfaceProtocol = 0x00` (None).
  - `bNumEndpoints = 1` (INT IN).
- HID descriptor:
  - `bcdHID = 0x0110`, `bCountryCode = 33` (US), `bNumDescriptors = 1`.
  - Report descriptor type `0x22`, length matches `sizeof(hid_report)`.
- Endpoint descriptor (Interrupt IN):
  - `bEndpointAddress = 0x81`, `bmAttributes = 0x03` (Interrupt), `wMaxPacketSize = 8`, `bInterval = 8`.

<details><summary><b>USB Device descriptor framework (FS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x09,       // idVendor  (0x090A)
0x36, 0x40,       // idProduct (0x4036)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x04,             // iConfiguration
0xC0,             // bmAttributes (self-powered)
0x32,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x00,             // bInterfaceSubClass (Boot if enabled -> set to 1)
0x00,             // bInterfaceProtocol (None)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted here: LSB,MSB at build time */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<details><summary><b>USB Device descriptor framework (HS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x09,       // idVendor  (0x090A)
0x36, 0x40,       // idProduct (0x4036)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Device Qualifier */
0x0A,             // bLength
0x06,             // bDescriptorType (Device Qualifier)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x01,             // bNumConfigurations
0x00,             // bReserved

/* HS Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x05,             // iConfiguration (HS index)
0xC0,             // bmAttributes (self-powered)
0x19,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x00,             // bInterfaceSubClass (Boot if enabled -> set to 1)
0x00,             // bInterfaceProtocol (None)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted by build: LSB,MSB */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<a id="hid-combo-init-rtos"></a>
## Initialization Flow (RTOS)
- `board_setup()` → clocks, pins, cache, UART (optional), USB power.
- `tx_kernel_enter()` → ThreadX start.
- `tx_application_define()`:
  - `ux_system_initialize()` with a static pool (`UX_DEVICE_MEMORY_STACK_SIZE`, default 7 KB).
  - `ux_device_stack_initialize()` with HS/FS device + string frameworks.
  - HID class register: `_ux_system_slave_class_hid_name` → `ux_device_class_hid_entry` with callbacks; set `report_id` handling.
  - Create demo thread → `ux_demo_device_hid_thread_entry()`.
- In thread:
  - `usb_device_dcd_initialize(UX_NULL)` to register the DCD.
  - When configured (`UX_DEVICE_CONFIGURED`) and instance ready, alternate between keyboard and mouse reports.

<a id="hid-combo-init-standalone"></a>
## Initialization Flow (Standalone)
- `board_setup()`.
- `ux_application_define()`:
  - Same USBX init + HID registration as RTOS variant.
  - `usb_device_dcd_initialize(UX_NULL)`.
- `main()` loop:
  - `ux_system_tasks_run()`; call `ux_demo_device_hid_task()` to emit keyboard then mouse reports when configured.

## Expected Behavior
- Device enumerates as a single HID interface.
- Periodically types characters and moves the cursor in a square pattern.
- Reports include a leading Report ID byte distinguishing Keyboard (1) vs Mouse (2).

<a id="hid-combo-troubleshooting"></a>
## Troubleshooting
- Not enumerating:
  - Check D+ (FS) or HS PHY signals and power; verify `usb_device_dcd_initialize()` registers the controller.
  - Confirm `UX_DEVICE_CONFIGURED` becomes true; inspect `ux_demo_error_callback()` logs.
- Wrong report parsing on host:
  - Ensure your HID report descriptor uses `REPORT_ID` and buffers include the ID byte.
  - Confirm endpoint max packet size supports your largest report (Keyboard = 9 bytes fits in 8+ID streamed across separate IN frames if needed; demo uses small bursts).
- Standalone timing:
  - Ensure `UX_PERIODIC_RATE == 1000` and call `ux_system_tasks_run()` sufficiently often.

# HID Custom (RTOS & Standalone)
- **Purpose:** Demonstrates using the USBX HID device class to implement a custom Consumer Control device (media + brightness).
- **Variants:** [demo_device_hid_custom_rtos.c](../demo_device_hid_custom_rtos.c) and [demo_device_hid_custom_standalone.c](../demo_device_hid_custom_standalone.c).
- **Targets:** MCU platforms with a USB Device Controller (e.g., STM32H7). Platform glue is provided by your board layer via `board_setup()` and `usb_device_dcd_initialize()`.

## Overview
- **Device role:** Enumerates as a HID Consumer Control device on a host PC.
- **Events demo:** Sends one 16-bit event at a time to control media (volume up/down, mute/unmute) and screen brightness up/down.
- **Report format:** HID Input report is a 16-bit value (single item) representing the selected usage; sample cycles through actions in the demo thread/task.
- **Subclass/Protocol:** Non-boot (`bInterfaceSubClass = 0x00`), protocol undefined (`bInterfaceProtocol = 0x00`).

## Source Layout
- **RTOS:** [demo_device_hid_custom_rtos.c](../demo_device_hid_custom_rtos.c) — initializes USBX in `tx_application_define`, registers HID class, and runs a demo thread.
- **Standalone:** [demo_device_hid_custom_standalone.c](../demo_device_hid_custom_standalone.c) — initializes USBX in `ux_application_define` and drives events from `ux_demo_device_hid_task()` with `ux_system_tasks_run()`.

## Build Prerequisites

### Common:
- Define `UX_DEVICE_SIDE_ONLY`.
- Provide platform functions: `board_setup()` and `usb_device_dcd_initialize(void*)` (DCD/PHY clocks, GPIO, interrupts, VBUS, etc.).
- Link against Eclipse ThreadX USBX device stack and the device controller driver for your MCU.

### RTOS variant:
- ThreadX kernel port for your MCU; call `tx_kernel_enter()` in `main()` and implement `tx_application_define()` to initialize USBX and start the HID thread.

### Standalone variant:
- Define `UX_STANDALONE`.
- Set `UX_PERIODIC_RATE` to 1000 (1ms tick) in your configuration if not already.
- In `main()`, run `ux_system_tasks_run()` repeatedly and call `ux_demo_device_hid_task()`.

## USB Descriptor Summary

- **VID/PID:** 0x070A / 0x4027 (sample values; adjust for your product).
- **EP0:** 64 bytes (`UX_DEMO_MAX_EP0_SIZE = 0x40`).
- **Configuration:** 1 interface, self-powered, max power 100 mA.
- **Interface:** HID, Subclass 0x00 (non-boot), Protocol 0x00 (Undefined).
- **Endpoint:** Interrupt IN `0x81`, packet size 8, `bInterval = 8` (FS 8ms/HS 16ms effective).
- **HID Report:** Consumer Page (0x0C) Application collection; logical min 0, max 0x03FF; `REPORT_SIZE = 16`, `REPORT_COUNT = 1`; `INPUT (Data, Array, Abs)`.

## Descriptors (Detailed)

### HID Consumer Report Descriptor
- Usage page: Consumer (0x0C), Application collection: Consumer.
- Inputs: single 16-bit array item; supported usages include Brightness Up/Down, Volume Up/Down, Mute, Play/Pause, Next/Previous Track.
- Resulting input report layout: [Byte0..1] 16-bit event value (LSB,MSB).

<details><summary><b>Consumer HID report bytes</b></summary>

```c
/* hid_consumer_report[] */
0x05, 0x0C,       // USAGE_PAGE (Consumer Devices)
0x09, 0x01,       // USAGE (Consumer)
0xA1, 0x01,       // COLLECTION (Application)
  0x15, 0x00,     //   LOGICAL_MINIMUM (0)
  0x26, 0xFF, 0x03,//  LOGICAL_MAXIMUM (0x03FF)
  0x75, 0x10,     //   REPORT_SIZE (16)
  0x95, 0x01,     //   REPORT_COUNT (1)
  // Brightness
  0x09, 0x6F,     //   USAGE (Brightness Up)
  0x09, 0x70,     //   USAGE (Brightness Down)
  // Media controls
  0x09, 0xE9,     //   USAGE (Volume Up)
  0x09, 0xEA,     //   USAGE (Volume Down)
  0x09, 0xE2,     //   USAGE (Mute)
  0x09, 0xCD,     //   USAGE (Play/Pause)
  0x09, 0xB5,     //   USAGE (Next Track)
  0x09, 0xB6,     //   USAGE (Previous Track)
  0x81, 0x00,     //   INPUT (Data, Array, Abs)
0xC0              // END_COLLECTION
```

</details>

### USB Device/Configuration Descriptors
- Device descriptor:
  - `bcdUSB = 0x0200` (USB 2.0), class/subclass/protocol = 0 (interface-defined).
  - `bMaxPacketSize0 = 64`.
  - `idVendor = 0x070A`, `idProduct = 0x4027`, demo strings at indices 1..6.
  - `bNumConfigurations = 1`.
- Device qualifier descriptor (HS build): mirrors device characteristics for the other speed.
- Configuration descriptor:
  - `wTotalLength = 0x22` (34 bytes across config + interface + HID + endpoint).
  - `bmAttributes = 0xC0` (self-powered), `bMaxPower = 0x32` (100 mA).
  - `bNumInterfaces = 1`, `bConfigurationValue = 1`.
- Interface descriptor:
  - `bInterfaceClass = 0x03` (HID), `bInterfaceSubClass = 0x00` (non-boot), `bInterfaceProtocol = 0x00` (Undefined).
  - `bNumEndpoints = 1` (INT IN).
- HID descriptor:
  - `bcdHID = 0x0110`, `bCountryCode = 33` (US), `bNumDescriptors = 1`.
  - Report descriptor type `0x22`, length matches `sizeof(hid_consumer_report)`.
- Endpoint descriptor (Interrupt IN):
  - `bEndpointAddress = 0x81` (IN, EP1), `bmAttributes = 0x03` (Interrupt).
  - `wMaxPacketSize = 8` bytes.
  - `bInterval = 8` → FS: 8 ms; HS: 16 ms.

<details><summary><b>USB Device descriptor framework (FS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x07,       // idVendor  (0x070A)
0x27, 0x40,       // idProduct (0x4027)
0x00, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x04,             // iConfiguration
0xC0,             // bmAttributes (self-powered)
0x32,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x00,             // bInterfaceSubClass (non-boot)
0x00,             // bInterfaceProtocol (Undefined)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted here: LSB,MSB at build time */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
0x08,             // bInterval (8)
```

</details>

<details><summary><b>USB Device descriptor framework (HS)</b></summary>

```c
/* Device descriptor */
0x12,             // bLength
0x01,             // bDescriptorType (Device)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x0A, 0x07,       // idVendor  (0x070A)
0x27, 0x40,       // idProduct (0x4027)
0x01, 0x00,       // bcdDevice
0x01,             // iManufacturer
0x02,             // iProduct
0x03,             // iSerialNumber
0x01,             // bNumConfigurations

/* Device Qualifier */
0x0A,             // bLength
0x06,             // bDescriptorType (Device Qualifier)
0x00, 0x02,       // bcdUSB (2.00)
0x00,             // bDeviceClass (interface-defined)
0x00,             // bDeviceSubClass
0x00,             // bDeviceProtocol
0x40,             // bMaxPacketSize0 (64)
0x01,             // bNumConfigurations
0x00,             // bReserved

/* HS Configuration */
0x09,             // bLength
0x02,             // bDescriptorType (Configuration)
0x22, 0x00,       // wTotalLength (34)
0x01,             // bNumInterfaces
0x01,             // bConfigurationValue
0x05,             // iConfiguration (HS index)
0xC0,             // bmAttributes (self-powered)
0x19,             // bMaxPower (100 mA)

/* Interface */
0x09,             // bLength
0x04,             // bDescriptorType (Interface)
0x00,             // bInterfaceNumber
0x00,             // bAlternateSetting
0x01,             // bNumEndpoints
0x03,             // bInterfaceClass (HID)
0x00,             // bInterfaceSubClass (non-boot)
0x00,             // bInterfaceProtocol (Undefined)
0x06,             // iInterface

/* HID */
0x09,             // bLength
0x21,             // bDescriptorType (HID)
0x10, 0x01,       // bcdHID (1.10)
0x21,             // bCountryCode (US)
0x01,             // bNumDescriptors
0x22,             // bReportDescriptorType
/* wDescriptorLength (report size) inserted by build: LSB,MSB */
0x00, 0x00,

/* Endpoint (INT IN) */
0x07,             // bLength
0x05,             // bDescriptorType (Endpoint)
0x81,             // bEndpointAddress (IN, EP1)
0x03,             // bmAttributes (Interrupt)
0x08, 0x00,       // wMaxPacketSize (8)
/* bInterval provided by FS/HS timing; typical 8 */
```

</details>

## Initialization Flow (RTOS)
- `board_setup()` → clocks, pins, cache, UART (optional), USB power.
- `tx_kernel_enter()` → ThreadX start.
- `tx_application_define()`:
  - `ux_system_initialize()` with a static pool (`UX_DEVICE_MEMORY_STACK_SIZE`, default 7 KB).
  - `ux_device_stack_initialize()` with HS/FS device + string frameworks.
  - HID class register: `_ux_system_slave_class_hid_name` → `ux_device_class_hid_entry` with callbacks.
  - Create demo thread → `ux_demo_device_hid_thread_entry()`.
- In thread:
  - `usb_device_dcd_initialize(UX_NULL)` to register the DCD.
  - When configured (`UX_DEVICE_CONFIGURED`) and instance ready, send Consumer events: volume down/up → mute → unmute, then brightness down/up.

## Initialization Flow (Standalone)
- `board_setup()`.
- `ux_application_define()`:
  - Same USBX init + HID registration as RTOS variant.
  - `usb_device_dcd_initialize(UX_NULL)`.
- `main()` loop:
  - `ux_system_tasks_run()`; call `ux_demo_device_hid_task()` to emit Consumer events when configured.

## Expected Behavior
- On connection to a PC, the device enumerates as “HID Consumer Demo”.
- Media controls: volume decreases to 0, increases to 100, toggles mute/unmute.
- Brightness controls: decreases to 0 then increases to 100.
- Each action sends a press and release (two 16-bit reports).

## Troubleshooting
- Not enumerating:
  - Check D+ (FS) or HS PHY signals and power; verify `usb_device_dcd_initialize()` registers the controller.
  - Confirm `UX_DEVICE_CONFIGURED` becomes true; inspect `ux_demo_error_callback()` logs.
- Host ignores events:
  - Verify the HID Consumer page is supported by the OS and active window.
  - Ensure report length is 2 bytes and you send release after press.
- Standalone timing:
  - Ensure `UX_PERIODIC_RATE == 1000` and that `ux_system_tasks_run()` is called frequently.

## References
- USBX code repository: https://github.com/eclipse-threadx/usbx/
- USBX Documentation: https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/usbx/overview-usbx.md