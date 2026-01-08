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
/** Note                                                                  */
/**                                                                       */
/**  This demonstration is not optimized, to optimize application user    */
/**  should configure related class flag in ux_user.h and adjust          */
/**  UX_DEVICE_MEMORY_STACK_SIZE                                          */
/**                                                                       */
/**                                                                       */
/**  AUTHOR                                                               */
/**                                                                       */
/**   Mohamed AYED                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "ux_api.h"
#include "ux_host_class_hid.h"
#include "ux_host_class_hid_mouse.h"

#ifndef UX_DEVICE_SIDE_ONLY
#error UX_DEVICE_SIDE_ONLY must be defined
#endif


/**************************************************/
/**  Define constants                             */
/**************************************************/
#define UX_HOST_MEMORY_STACK_SIZE               1024
#define UX_DEMO_THREAD_STACK_SIZE               1024
/**************************************************/
/**  usbx application initialization with RTOS    */
/**************************************************/
VOID tx_application_define(VOID *first_unused_memory);

/**************************************************/
/**  usbx device hid demo thread                  */
/**************************************************/
VOID ux_demo_device_hid_thread_entry(ULONG thread_input);

/**************************************************/
/**  usbx device hid demo mouse                   */
/**************************************************/
static UINT ux_demo_host_change_callback(ULONG event, UX_HOST_CLASS *host_class, VOID *instance);


UX_HOST_CLASS_HID          *hid_instance;
UX_HOST_CLASS_HID_MOUSE    *mouse;

/**************************************************/
/**  thread object                                */
/**************************************************/
static UX_THREAD ux_hid_thread;
static ULONG ux_hid_thread_stack[UX_DEMO_THREAD_STACK_SIZE / sizeof(ULONG)];

/**************************************************/
/**  usbx callback error                          */
/**************************************************/
static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code);

static CHAR ux_system_memory_pool[UX_HOST_MEMORY_STACK_SIZE];

#ifndef EXTERNAL_MAIN
extern int board_setup(void);
#endif /* EXTERNAL_MAIN */
extern int usb_device_dcd_initialize(void *param);

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
CHAR                            *memory_pointer;
UINT                            status;

    UX_PARAMETER_NOT_USED(first_unused_memory);

    /* Use static memory block.  */
    memory_pointer = ux_system_memory_pool;

    /* Initialize USBX Memory */
    status = ux_system_initialize(memory_pointer, UX_HOST_MEMORY_STACK_SIZE, UX_NULL, 0);

    if(status != UX_SUCCESS)
        return;

    /* Install the device portion of USBX.  */
    status = ux_host_stack_initialize(ux_demo_host_change_callback);

    if(status != UX_SUCCESS)
        return;

    /* Register the hid class.  */
    status = ux_host_stack_class_register(_ux_system_host_class_hid_name, ux_host_class_hid_entry);

    if(status != UX_SUCCESS)
        return;

    /* Register the HID mouse client.  */
    status = ux_host_class_hid_client_register(_ux_system_host_class_hid_client_mouse_name,
                                               ux_host_class_hid_mouse_entry);

    if(status != UX_SUCCESS)
        return;

    /* Register error callback. */
    ux_utility_error_callback_register(ux_demo_error_callback);
}

static UINT ux_demo_host_change_callback(ULONG event, UX_HOST_CLASS *current_class, VOID *current_instance)
{

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

      break;

    case UX_DEVICE_REMOVAL:

      /* Free HID Instance */
      if ((VOID*)hid_instance == current_instance)
      {
        hid_instance = UX_NULL;
      }

      break;

    case UX_HID_CLIENT_INSERTION:


      /* Check the HID_client if this is a HID mouse device */
      if (client -> ux_host_class_hid_client_handler == ux_host_class_hid_mouse_entry)
      {
        /* Get current Hid Client */
        if (mouse == UX_NULL)
        {
          mouse = client -> ux_host_class_hid_client_local_instance;
        }
      }

      /* USER CODE END UX_HID_CLIENT_INSERTION */

      break;

    case UX_HID_CLIENT_REMOVAL:


      if ((VOID*)mouse == client -> ux_host_class_hid_client_local_instance)
      {
        /* Clear hid mouse instance */
        mouse = UX_NULL;
      }

      break;

    default:
      break;
  }

  return UX_SUCCESS;
}

static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code)
{
    /*
     * Refer to ux_api.h. For example,
     * UX_SYSTEM_LEVEL_INTERRUPT, UX_SYSTEM_CONTEXT_DCD, UX_DEVICE_HANDLE_UNKNOWN
     */
    printf("USBX error: system level(%d), context(%d), error code(0x%x)\r\n", system_level, system_context, error_code);
}
