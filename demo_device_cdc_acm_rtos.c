/***************************************************************************
 * Copyright (c) 2025-present Eclipse ThreadX Contributors
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** Overview                                                              */
/**                                                                       */
/**                                                                       */
/**                                                                       */
/** Note                                                                  */
/**                                                                       */
/**  This demonstration is not optimized, to optimize application user    */
/**  sould configuer related class flag in ux_user.h and adjust           */
/**  UX_DEVICE_MEMORY_STACK_SIZE                                          */
/**                                                                       */
/**                                                                       */
/**  AUTHOR                                                               */
/**                                                                       */
/**   Mohamed AYED                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tx_api.h"
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

#ifndef UX_DEVICE_SIDE_ONLY
#error UX_DEVICE_SIDE_ONLY must be defined
#endif

/**************************************************/
/**  Define constants                             */
/**************************************************/
#define UX_DEVICE_MEMORY_STACK_SIZE     (7*1024)
#define UX_DEMO_THREAD_STACK_SIZE       (1*1024)

#define UX_DEMO_CDC_ACM_DEVICE_VID      0x090A
#define UX_DEMO_CDC_ACM_DEVICE_PID      0x4036

#define UX_DEMO_MAX_EP0_SIZE            0x40U
#define UX_DEMO_CONFIG_DESC_SIZE        0x22U
#define UX_DEMO_BCD_USB                 0x0200

/**************************************************/
/**  usbx device cdc acm demo callbacks           */
/**************************************************/
VOID ux_demo_device_cdc_acm_instance_activate(VOID *cdc_acm_instance);
VOID ux_demo_device_cdc_acm_instance_deactivate(VOID *cdc_acm_instance);
VOID ux_demo_device_cdc_acm_instance_parameter_chage(VOID *cdc_acm_instance);

/**************************************************/
/**  usbx application initialization with RTOS    */
/**************************************************/
VOID tx_application_define(VOID *first_unused_memory);

/**************************************************/
/**  usbx device cdc acm demo thread              */
/**************************************************/
VOID ux_demo_device_cdc_acm_thread_entry(ULONG thread_input);

/**************************************************/
/**  usbx device cdc acm demo                    */
/**************************************************/

/**************************************************/
/**  usbx device cdc acm instance                 */
/**************************************************/
UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

/**************************************************/
/**  thread object                                */
/**************************************************/
static TX_THREAD ux_cdc_acm_thread;
static ULONG ux_cdc_acm_thread_stack[UX_DEMO_THREAD_STACK_SIZE / sizeof(ULONG)];

/**************************************************/
/**  usbx callback error                          */
/**************************************************/
static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code);

static CHAR ux_system_memory_pool[UX_DEVICE_MEMORY_STACK_SIZE];

#ifndef EXTERNAL_MAIN
extern int board_setup(void);
#endif /* EXTERNAL_MAIN */
extern int usb_device_dcd_initialize(void *param);

/**************************************************/
/**  USB Report descriptor                        */
/**************************************************/

#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED sizeof(ux_demo_device_framework_full_speed)

UCHAR ux_demo_device_framework_full_speed[] = {
    /* Device descriptor */
    0x12,                       /* bLength */
    0x01,                       /* bDescriptorType */
    UX_W0(UX_DEMO_BCD_USB), UX_W1(UX_DEMO_BCD_USB), /* bcdUSB */
    0x02,                       /* bDeviceClass : 0x02 : CDC-Control */
    0x02,                       /* bDeviceSubClass : 0x02 */
    0x00,                       /* bDeviceProtocol : 0x00 : Reset */
    UX_DEMO_MAX_EP0_SIZE,       /* bMaxPacketSize0 */
    UX_W0(UX_DEMO_CDC_ACM_DEVICE_VID), UX_W1(UX_DEMO_CDC_ACM_DEVICE_VID), /* idVendor : ... */
    UX_W0(UX_DEMO_CDC_ACM_DEVICE_PID), UX_W1(UX_DEMO_CDC_ACM_DEVICE_PID), /* idProduct */
    0x00, 0x01,                 /* bcdDevice */
    0x01,                       /* iManufacturer */
    0x02,                       /* iProduct */
    0x03,                       /* iSerialNumber */
    0x01,                       /* bNumConfigurations */

    /* Configuration descriptor, total 75 */
    0x09,                       /* bLength */
    0x02,                       /* bDescriptorType */
    UX_W0(UX_DEMO_CONFIG_DESC_SIZE), UX_W1(UX_DEMO_CONFIG_DESC_SIZE), /* wTotalLength */
    0x02,                       /* bNumInterfaces */
    0x01,                       /* bConfigurationValue */
    0x00,                       /* iConfiguration */
    0x40,                       /* bmAttributes */
                                    /* D6 : 0x1 : Self-powered */
                                    /* D5, Remote Wakeup : 0x0 : Not supported */
    0x00,                       /* bMaxPower : 0 : 0mA */

    /* Interface association descriptor */
    0x08,                       /* bLength */
    0x0b,                       /* bDescriptorType */
    0x00,                       /* bFirstInterface */
    0x02,                       /* bInterfaceCount */
    0x02,                       /* bFunctionClass : 0x02 */
    0x02,                       /* bFunctionSubClass : 0x02 */
    0x00,                       /* bFunctionProtocol : 0x00 : Reset */
    0x00,                       /* iFunction */

    /* Interface descriptor */
    0x09,                       /* bLength */
    0x04,                       /* bDescriptorType */
    0x00,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x01,                       /* bNumEndpoints */
    0x02,                       /* bInterfaceClass : 0x02 : CDC Control */
    0x02,                       /* bInterfaceSubClass : 0x02 */
    0x00,                       /* bInterfaceProtocol : 0x00 : Undefined */
    0x00,                       /* iInterface */

    /* CDC Header Functional Descriptor */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x00,                       /* bDescriptorSubtype */
    0x10, 0x01,                 /* bcdCDC : 0x0110 : CDC 1.1 */

    /* CDC Call Management Functional Descriptor */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x01,                       /* bDescriptorSubtype */
    0x03,                       /* bmCapabilities */
                                    /* D1 : 0x1 : Send/receive call management over Data Class interface */
                                    /* D0 : 0x1 : Handle call management itself */
    0x01,                       /* bDataInterface */

    /* CDC Abstract Control Management Functional Descriptor */
    0x04,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x02,                       /* bDescriptorSubtype */
    0x0f,                       /* bmCapabilities */
                                    /* D3, notification Network_Connection : 0x1 : Supported */
                                    /* D2, request Send_Break : 0x1 : Supported */
                                    /* D1, request Set_Line_Coding
                                                   Set_Control_Line_State
                                                   Get_Line_Coding and
                                                   notification Serial_State : 0x1 : Supported */
                                    /* D0, request Set_Comm_Feature
                                                   Clear_Comm_Feature
                                                   Get_Comm_Feature : 0x1 : Supported */

    /* CDC Union Functional Descriptor (1 slave interface) */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x06,                       /* bDescriptorSubtype */
    0x00,                       /* bMasterInterface */
    0x01,                       /* bSlaveInterface0 */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x83,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x01 */
                                    /* D3..0, Endpoint number : 3 */
    0x03,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x3 : Interrupt */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x08, 0x00,                 /* wMaxPacketSize : 8 */
                                    /* D10..0, Max Packet Size : 8 */
                                    /* D12..11, Additional transactions : 0x00 */
    0xff,                       /* bInterval : 255 */

    /* Interface Descriptor */
    0x09,                       /* bLength */
    0x04,                       /* bDescriptorType */
    0x01,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x02,                       /* bNumEndpoints */
    0x0a,                       /* bInterfaceClass : 0x0A : CDC Data */
    0x00,                       /* bInterfaceSubClass : 0x00 : Undefined */
    0x00,                       /* bInterfaceProtocol : 0x00 : Undefined */
    0x00,                       /* iInterface */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x02,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x00 */
                                    /* D3..0, Endpoint number : 2 */
    0x02,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x2 : Bulk */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x40, 0x00,                 /* wMaxPacketSize : 64 */
                                    /* D10..0, Max Packet Size : 64 */
                                    /* D12..11, Additional transactions : 0x00 */
    0x00,                       /* bInterval : 0 : 0 */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x81,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x01 */
                                    /* D3..0, Endpoint number : 1 */
    0x02,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x2 : Bulk */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x40, 0x00,                 /* wMaxPacketSize : 64 */
                                    /* D10..0, Max Packet Size : 64 */
                                    /* D12..11, Additional transactions : 0x00 */
    0x00,                       /* bInterval : 0 : 0 */
};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED sizeof(ux_demo_device_framework_high_speed)
UCHAR ux_demo_device_framework_high_speed[] = {
    /* Device descriptor */
    0x12,                       /* bLength */
    0x01,                       /* bDescriptorType */
    0x00, 0x02,                 /* bcdUSB : 0x0200 : USB 2.0 */
    0x02,                       /* bDeviceClass : 0x02 : CDC-Control */
    0x02,                       /* bDeviceSubClass : 0x02 */
    0x00,                       /* bDeviceProtocol : 0x00 : Reset */
    0x40,                       /* bMaxPacketSize0 : 64 : 64 */
    0xC9, 0x1F,                 /* idVendor : 0x1FC9 */
    0x94, 0x00,                 /* idProduct */
    0x00, 0x01,                 /* bcdDevice */
    0x01,                       /* iManufacturer */
    0x02,                       /* iProduct */
    0x03,                       /* iSerialNumber */
    0x01,                       /* bNumConfigurations */

    /* Device qualifier descriptor */
    0x0A,                       /* bLength */
    0x06,                       /* bDescriptorType */
    0x00, 0x02,                 /* bcdUSB : 0x0200 : USB 2.0 */
    0x02,                       /* bDeviceClass : 0x02 */
    0x00,                       /* bDeviceSubClass : 0x00 : Reset */
    0x00,                       /* bDeviceProtocol : 0x00 : Reset */
    0x40,                       /* bMaxPacketSize0 : 64 : 64 */
    0x01,                       /* bNumConfigurations */
    0x00,                       /* bReserved */

    /* Configuration descriptor */
    0x09,                       /* bLength */
    0x02,                       /* bDescriptorType */
    0x4b, 0x00,                 /* wTotalLength : 75 */
    0x02,                       /* bNumInterfaces */
    0x01,                       /* bConfigurationValue */
    0x00,                       /* iConfiguration */
    0x40,                       /* bmAttributes */
                                    /* D6 : 0x1 : Self-powered */
                                    /* D5, Remote Wakeup : 0x0 : Not supported */
    0x32,                       /* bMaxPower : 50 : 100mA */

    /* Interface Association Descriptor */
    0x08,                       /* bLength */
    0x0b,                       /* bDescriptorType */
    0x00,                       /* bFirstInterface */
    0x02,                       /* bInterfaceCount */
    0x02,                       /* bFunctionClass : 0x02 */
    0x02,                       /* bFunctionSubClass : 0x02 */
    0x00,                       /* bFunctionProtocol : 0x00 : Reset */
    0x00,                       /* iFunction */

    /* Interface Descriptor */
    0x09,                       /* bLength */
    0x04,                       /* bDescriptorType */
    0x00,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x01,                       /* bNumEndpoints */
    0x02,                       /* bInterfaceClass : 0x02 : CDC Control */
    0x02,                       /* bInterfaceSubClass : 0x02 */
    0x00,                       /* bInterfaceProtocol : 0x00 : Undefined */
    0x00,                       /* iInterface */

    /* CDC Header Functional Descriptor */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x00,                       /* bDescriptorSubtype */
    0x10, 0x01,                 /* bcdCDC : 0x0110 : CDC 1.1 */

    /* CDC Call Management Functional Descriptor */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x01,                       /* bDescriptorSubtype */
    0x00,                       /* bmCapabilities */
                                    /* D1 : 0x0 : Send/receive call management over COMM Class interface */
                                    /* D0 : 0x0 : Do not handle call management itself */
    0x01,                       /* bDataInterface */

    /* CDC Abstract Control Management Functional Descriptor */
    0x04,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x02,                       /* bDescriptorSubtype */
    0x06,                       /* bmCapabilities */
                                    /* D3, notification Network_Connection : 0x0 : Not supported */
                                    /* D2, request Send_Break : 0x1 : Supported */
                                    /* D1, request Set_Line_Coding
                                                   Set_Control_Line_State
                                                   Get_Line_Coding
                                                   notification Serial_State : 0x1 : Supported */
                                    /* D0, request Set_Comm_Feature
                                                   Clear_Comm_Feature
                                                   Get_Comm_Feature : 0x0 : Not supported */

    /* CDC Union Functional Descriptor (1 slave interface) */
    0x05,                       /* bLength */
    0x24,                       /* bDescriptorType */
    0x06,                       /* bDescriptorSubtype */
    0x00,                       /* bMasterInterface */
    0x01,                       /* bSlaveInterface0 */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x83,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x01 */
                                    /* D3..0, Endpoint number : 3 */
    0x03,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x3 : Interrupt */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x08, 0x00,                 /* wMaxPacketSize : 8 */
                                    /* D10..0, Max Packet Size : 8 */
                                    /* D12..11, Additional transactions : 0x00 */
    0x0f,                       /* bInterval : 15 : 15ms / x16384 (FS 16384ms/HS 2048ms) */

    /* Interface Descriptor */
    0x09,                       /* bLength */
    0x04,                       /* bDescriptorType */
    0x01,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x02,                       /* bNumEndpoints */
    0x0a,                       /* bInterfaceClass : 0x0A : CDC Data */
    0x00,                       /* bInterfaceSubClass : 0x00 : Undefined */
    0x00,                       /* bInterfaceProtocol : 0x00 : Undefined */
    0x00,                       /* iInterface */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x02,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x00 */
                                    /* D3..0, Endpoint number : 2 */
    0x02,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x2 : Bulk */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x00, 0x02,                 /* wMaxPacketSize : 512 */
                                    /* D10..0, Max Packet Size : 512 */
                                    /* D12..11, Additional transactions : 0x00 */
    0x00,                       /* bInterval : 0 : 0 */

    /* Endpoint Descriptor */
    0x07,                       /* bLength */
    0x05,                       /* bDescriptorType */
    0x81,                       /* bEndpointAddress */
                                    /* D7, Direction : 0x01 */
                                    /* D3..0, Endpoint number : 1 */
    0x02,                       /* bmAttributes */
                                    /* D1..0, Trasfer Type : 0x2 : Bulk */
                                    /* D3..2, Synchronization Type : 0x0 : No Synchronization */
                                    /* D5..4, Usage Type : 0x0 : Data endpoint */
    0x00, 0x02,                 /* wMaxPacketSize : 512 */
                                    /* D10..0, Max Packet Size : 512 */
                                    /* D12..11, Additional transactions : 0x00 */
    0x00,                       /* bInterval : 0 : 0 */
};


/* String Device Framework :
   Byte 0 and 1 : Word containing the language ID : 0x0904 for US
   Byte 2       : Byte containing the index of the descriptor
   Byte 3       : Byte containing the length of the descriptor string
*/
#define STRING_FRAMEWORK_LENGTH sizeof(ux_demo_string_framework)
UCHAR ux_demo_string_framework[] = {

    /* iManufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 12,
    'U', 'S', 'B', 'X', ' ', 'e', 'c', 'l', 'i', 'p', 's', 'e',

    /* iProduct string descriptor : Index 2 */
    0x09, 0x04, 0x02, 12,
    'C', 'D', 'C', ' ', 'A', 'C', 'M', ' ', 'D', 'e', 'm', 'o',

    /* iSerialNumber Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 13,
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1',

    /* iConfiguration string descriptor : Index 4 */
    0x09, 0x04, 0x04, 10,
    'F', 'U', 'L', 'L', ' ', 'S', 'P', 'E', 'E', 'D',

    /* iConfiguration string descriptor : Index 5 */
    0x09, 0x04, 0x05, 10,
    'H', 'I', 'G', 'H', ' ', 'S', 'P', 'E', 'E', 'D',

    /* iInterface string descriptor : Index 6 */
    0x09, 0x04, 0x06, 5,
    'M', 'o', 'u', 's', 'e'
};


/* Multiple languages are supported on the device, to add  a language besides english,
   the unicode language code must be appended to the ux_demo_language_id_framework array and the length
   adjusted accordingly.
*/
#define LANGUAGE_ID_FRAMEWORK_LENGTH sizeof(ux_demo_language_id_framework)
UCHAR ux_demo_language_id_framework[] = {
    /* English. */
    0x09, 0x04
};

#ifndef EXTERNAL_MAIN
int main(void)
{
    /* Initialize the board.  */
    board_setup();

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}
#endif /* EXTERNAL_MAIN */

VOID tx_application_define(VOID *first_unused_memory)
{
CHAR                               *memory_pointer;
UINT                               status;
UX_SLAVE_CLASS_CDC_ACM_PARAMETER   cdc_acm_parameter;

    UX_PARAMETER_NOT_USED(first_unused_memory);

    /* Use static memory block.  */
    memory_pointer = ux_system_memory_pool;

    /* Initialize USBX Memory */
    status = ux_system_initialize(memory_pointer, UX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0);

    if(status != UX_SUCCESS)
        return;

    /* Install the device portion of USBX.  */
    status =  ux_device_stack_initialize(ux_demo_device_framework_high_speed, DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED,
                                         ux_demo_device_framework_full_speed, DEVICE_FRAMEWORK_LENGTH_FULL_SPEED,
                                         ux_demo_string_framework, STRING_FRAMEWORK_LENGTH,
                                         ux_demo_language_id_framework, LANGUAGE_ID_FRAMEWORK_LENGTH,
                                         UX_NULL);

    if(status != UX_SUCCESS)
        return;

    /* Initialize the cdc acm class parameters for the device */
    cdc_acm_parameter.ux_slave_class_cdc_acm_instance_activate = ux_demo_device_cdc_acm_instance_activate;
    cdc_acm_parameter.ux_slave_class_cdc_acm_instance_deactivate = ux_demo_device_cdc_acm_instance_deactivate;
    cdc_acm_parameter.ux_slave_class_cdc_acm_parameter_change = ux_demo_device_cdc_acm_instance_parameter_chage;

    /* Initialize the device cdc acm class. The class is connected with interface 0 on configuration 1. */
    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry,
                                            1, 0, (VOID *)&cdc_acm_parameter);

    if(status != UX_SUCCESS)
        return;

    /* Create the main demo thread.  */
    status = ux_utility_thread_create(&ux_cdc_acm_thread, "cdc_acm_usbx_app_thread_entry",
                                      ux_demo_device_cdc_acm_thread_entry, 0, ux_cdc_acm_thread_stack,
                                      UX_DEMO_THREAD_STACK_SIZE, 20, 20, 1, TX_AUTO_START);

    if(status != UX_SUCCESS)
        return;

    /* error callback */
    ux_utility_error_callback_register(ux_demo_error_callback);
}

/********************************************************************/
/**  ux_demo_device_cdc_acm_instance_activate                       */
/********************************************************************/
VOID ux_demo_device_cdc_acm_instance_activate(VOID *cdc_acm_instance)
{
    if (cdc_acm == UX_NULL)
        cdc_acm = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;
}

/********************************************************************/
/**  ux_demo_device_cdc_acm_instance_deactivate                     */
/********************************************************************/
VOID ux_demo_device_cdc_acm_instance_deactivate(VOID *cdc_acm_instance)
{
    if (cdc_acm_instance == (VOID *)cdc_acm)
        cdc_acm = UX_NULL;
}

/********************************************************************/
/**  ux_demo_device_cdc_acm_instance_parameter_chage                */
/********************************************************************/
VOID ux_demo_device_cdc_acm_instance_parameter_chage(VOID *cdc_acm_instance)
{

}

/********************************************************************/
/**  ux_demo_device_cdc_acm_thread_entry: cdc acm thread            */
/********************************************************************/
VOID ux_demo_device_cdc_acm_thread_entry(ULONG thread_input)
{

    UX_PARAMETER_NOT_USED(thread_input);

    /* Register the USB device controllers available in this system.  */
    usb_device_dcd_initialize(UX_NULL);

}

static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code)
{
    /*
     * Refer to ux_api.h. For example,
     * UX_SYSTEM_LEVEL_INTERRUPT, UX_SYSTEM_CONTEXT_DCD, UX_DEVICE_HANDLE_UNKNOWN
     */
    printf("USBX error: system level(%d), context(%d), error code(0x%x)\r\n", system_level, system_context, error_code);
}
