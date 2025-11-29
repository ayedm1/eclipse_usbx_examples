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
/**  AUTHOR                                                               */
/**                                                                       */
/**   Mohamed AYED                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "ux_demo_device_hid_mouse_keyboard.h"

/**************************************************/
/**  usbx device hid demo callbacks               */
/**************************************************/
VOID ux_demo_device_hid_instance_activate(VOID *hid_instance);
VOID ux_demo_device_hid_instance_deactivate(VOID *hid_instance);
UINT ux_demo_device_hid_callback(UX_SLAVE_CLASS_HID *hid_instance, UX_SLAVE_CLASS_HID_EVENT *hid_event);
UINT ux_demo_device_hid_get_callback(UX_SLAVE_CLASS_HID *hid_instance, UX_SLAVE_CLASS_HID_EVENT *hid_event);

/**************************************************/
/**  usbx device hid demo thread                  */
/**************************************************/
VOID ux_demo_device_hid_thread_entry(ULONG thread_input);

/**************************************************/
/**  usbx application initialization with RTOS    */
/**************************************************/
VOID tx_application_define(VOID *first_unused_memory);

/**************************************************/
/**  usbx device hid demo mouse                   */
/**************************************************/
UINT ux_demo_hid_mouse_cursor_move(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_mouse_buttons(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_mouse_scroll_wheel(UX_SLAVE_CLASS_HID *device_hid);

UINT ux_demo_hid_keyboard_send_lowercase_character(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_keyboard_send_uppercase_character(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_keyboard_send_number(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_keyboard_send_special_character(UX_SLAVE_CLASS_HID *device_hid);
UINT ux_demo_hid_keyboard_send_enter(UX_SLAVE_CLASS_HID *device_hid);

static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code);

/**************************************************/
/**  usbx device hid mouse instance               */
/**************************************************/
UX_SLAVE_CLASS_HID *hid;

/**************************************************/
/**  thread object                                */
/**************************************************/
static TX_THREAD ux_hid_thread;

/**************************************************/
/**  usbx device hid keyboard                     */
/**************************************************/
ULONG num_lock_flag  = UX_FALSE;
ULONG caps_lock_flag = UX_FALSE;

//int main(void)
//{
//    /* Initialize the board.  */
//    board_setup();
//
//    /* Enter the ThreadX kernel.  */
//    tx_kernel_enter();
//
//    while (1)
//    {
//    }
//
//}

VOID tx_application_define(VOID *first_unused_memory)
{
CHAR                            *stack_pointer;
CHAR                            *memory_pointer;
UINT                            status;
UX_SLAVE_CLASS_HID_PARAMETER    hid_parameter;

    /* Initialize the free memory pointer.  */
    stack_pointer =  (CHAR *) first_unused_memory;

    /* Initialize the RAM disk memory. */
    memory_pointer =  stack_pointer +  DEMO_STACK_SIZE;

    /* Initialize USBX Memory */
    status = ux_system_initialize(memory_pointer, UX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0);

    if(status != UX_SUCCESS)
        return;

    printf("USBX System initialized successfully\r\n");

    /* Install the device portion of USBX */
    status = ux_device_stack_initialize(ux_demo_get_high_speed_framework(),
                                        ux_demo_get_high_speed_framework_length(),
                                        ux_demo_get_full_speed_framework(),
                                        ux_demo_get_full_speed_framework_length(),
                                        ux_demo_get_string_framework(),
                                        ux_demo_get_string_framework_length(),
                                        ux_demo_get_language_framework(),
                                        ux_demo_get_language_framework_length(),
                                        UX_NULL);

    if(status != UX_SUCCESS)
        return;

    printf("USBX Device stack initialized successfully\r\n");

    /* Initialize the hid mouse class parameters for the device */
    hid_parameter.ux_slave_class_hid_instance_activate         = ux_demo_device_hid_instance_activate;
    hid_parameter.ux_slave_class_hid_instance_deactivate       = ux_demo_device_hid_instance_deactivate;
    hid_parameter.ux_device_class_hid_parameter_report_address = ux_demo_device_hid_get_report();
    hid_parameter.ux_device_class_hid_parameter_report_length  = ux_demo_device_hid_get_report_length();
    hid_parameter.ux_device_class_hid_parameter_report_id      = UX_TRUE;
    hid_parameter.ux_device_class_hid_parameter_callback       = ux_demo_device_hid_callback;
    hid_parameter.ux_device_class_hid_parameter_get_callback   = ux_demo_device_hid_get_callback;

    /* Initialize the device storage class. The class is connected with interface 0 on configuration 1. */
    status = ux_device_stack_class_register(_ux_system_slave_class_hid_name, ux_device_class_hid_entry,
                                            1, 0, (VOID *)&hid_parameter);

    if(status != UX_SUCCESS)
        return;

    /* Create the main demo thread.  */
    status = ux_utility_thread_create(&ux_hid_thread, "hid_usbx_app_thread_entry",
                                      ux_demo_device_hid_thread_entry, 0, stack_pointer,
                                      1024, 20, 20, 1, TX_AUTO_START);

    if(status != UX_SUCCESS)
        return;

    /* error callback */
    ux_utility_error_callback_register(ux_demo_error_callback);
}

/********************************************************************/
/**  ux_demo_device_hid_instance_activate                           */
/********************************************************************/
VOID ux_demo_device_hid_instance_activate(VOID *hid_instance)
{
    if (hid == UX_NULL)
        hid = (UX_SLAVE_CLASS_HID*) hid_instance;
}

/********************************************************************/
/**  ux_demo_device_hid_instance_deactivate                         */
/********************************************************************/
VOID ux_demo_device_hid_instance_deactivate(VOID *hid_instance)
{
    if (hid_instance == (VOID *)hid)
        hid = UX_NULL;
}

/********************************************************************/
/**  ux_demo_device_hid_callback                                    */
/********************************************************************/
UINT ux_demo_device_hid_callback(UX_SLAVE_CLASS_HID *hid_instance, UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
    UX_PARAMETER_NOT_USED(hid_instance);

    /* There was an event.  Analyze it.  Is it NUM LOCK ? */
    if ((hid_event -> ux_device_class_hid_event_buffer[0] & UX_HID_NUM_LOCK_MASK) &&
        (hid_event -> ux_device_class_hid_event_report_type == UX_DEVICE_CLASS_HID_REPORT_TYPE_OUTPUT))

        /* Set the Num lock flag.  */
        num_lock_flag = UX_TRUE;
    else
        /* Reset the Num lock flag.  */
        num_lock_flag = UX_FALSE;

    /* There was an event.  Analyze it.  Is it CAPS LOCK ? */
    if ((hid_event -> ux_device_class_hid_event_buffer[0] & UX_HID_CAPS_LOCK_MASK) &&
        (hid_event -> ux_device_class_hid_event_report_type == UX_DEVICE_CLASS_HID_REPORT_TYPE_OUTPUT))
        /* Set the Caps lock flag.  */
        caps_lock_flag = UX_TRUE;
    else
        /* Reset the Caps lock flag.  */
        caps_lock_flag = UX_FALSE;

    return UX_SUCCESS;
}

/********************************************************************/
/**  ux_demo_device_hid_get_callback                                */
/********************************************************************/
UINT ux_demo_device_hid_get_callback(UX_SLAVE_CLASS_HID *hid_instance, UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
    UX_PARAMETER_NOT_USED(hid_instance);
    UX_PARAMETER_NOT_USED(hid_event);

    return UX_SUCCESS;
}

/********************************************************************/
/**  ux_demo_device_hid_thread_entry: hid demo thread               */
/********************************************************************/
VOID ux_demo_device_hid_thread_entry(ULONG thread_input)
{
UINT            status;
UINT            demo_state = UX_DEMO_MOUSE_CURSOR;

    UX_PARAMETER_NOT_USED(thread_input);

    MX_USB_OTG_HS_PCD_Init();

    ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

    HAL_PCD_Start(&hpcd_USB_OTG_HS);

    while (1)
    {
      /* Check if the device state already configured.  */
      if ((UX_SLAVE_DEVICE_CHECK_STATE(UX_DEVICE_CONFIGURED)) && (hid != UX_NULL))
      {

        switch(demo_state)
        {

        case UX_DEMO_MOUSE_CURSOR:

          /* Move cursor */
          status = ux_demo_hid_mouse_cursor_move(hid);

          if (status == UX_MOUSE_CURSOR_MOVE_DONE)
              demo_state = UX_DEMO_MOUSE_WHEEL;

          break;

        case UX_DEMO_MOUSE_WHEEL:

          /* Scroll wheel */
          status = ux_demo_hid_mouse_scroll_wheel(hid);

          if (status == UX_MOUSE_WHEEL_MOVE_DONE)
              demo_state = UX_DEMO_HID_KEYBOARD_SEND_LOWCASE_CHARATER;

          break;

        case UX_DEMO_HID_KEYBOARD_SEND_LOWCASE_CHARATER:

          /* keyboard send lowercase character */
          status = ux_demo_hid_keyboard_send_lowercase_character(hid);

          if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
          {
              status = ux_demo_hid_keyboard_send_enter(hid);

              if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
                  demo_state = UX_DEMO_HID_KEYBOARD_SEND_UPPERCASE_CHARATER;
          }

          break;


        case UX_DEMO_HID_KEYBOARD_SEND_UPPERCASE_CHARATER:

          /* keyboard send uppercase character */
          status = ux_demo_hid_keyboard_send_uppercase_character(hid);

          if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
          {
              status = ux_demo_hid_keyboard_send_enter(hid);

              if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
                  demo_state = UX_DEMO_HID_KEYBOARD_SEND_NUMBER;
          }

          break;


        case UX_DEMO_HID_KEYBOARD_SEND_NUMBER:

          /* keyboard send number */
          status = ux_demo_hid_keyboard_send_number(hid);

          if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
          {
              status = ux_demo_hid_keyboard_send_enter(hid);

              if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
                  demo_state = UX_DEMO_HID_KEYBOARD_SENLD_SPECIAL_CHARATER;
          }

          break;


        case UX_DEMO_HID_KEYBOARD_SENLD_SPECIAL_CHARATER:

          /* keyboard send special character */
          status = ux_demo_hid_keyboard_send_special_character(hid);

          if (status == UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE)
              demo_state = UX_DEMO_HID_KEYBOARD_SEND_EVENT_FINISH;

          break;

        default:

          ux_utility_delay_ms(MS_TO_TICK(10));

          break;
        }
      }
      else
      {
        /* Sleep thread for 10ms.  */
        ux_utility_delay_ms(MS_TO_TICK(10));
      }
    }
}

/********************************************************************/
/**  ux_demo_hid_mouse_cursor_move: show how to move mouse cursor   */
/********************************************************************/
UINT ux_demo_hid_mouse_cursor_move(UX_SLAVE_CLASS_HID *device_hid)
{
UINT                        status;
UX_SLAVE_CLASS_HID_EVENT    device_hid_event;
static UCHAR                mouse_x;
static UCHAR                mouse_y;
static UCHAR                mouse_move_dir;
static UCHAR                mouse_move_count;

    /* Sleep thread for 10ms.  */
    ux_utility_delay_ms(MS_TO_TICK(10));

    /* Initialize mouse event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_MOUSE;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 4;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;           /* ...R|M|L  */
    device_hid_event.ux_device_class_hid_event_buffer[1] = mouse_x;     /* X         */
    device_hid_event.ux_device_class_hid_event_buffer[2] = mouse_y;     /* Y         */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;           /* Wheel     */

    /* Move cursor.  */
    switch(mouse_move_dir)
    {
    case UX_MOUSE_CURSOR_MOVE_RIGHT:  /* +x.  */

        mouse_x = UX_DEMO_HID_MOUSE_CURSOR_MOVE;
        mouse_y = 0;
        mouse_move_count ++;

        if (mouse_move_count >= UX_DEMO_HID_MOUSE_CURSOR_MOVE_N)
        {
            mouse_move_count = 0;
            mouse_move_dir = UX_MOUSE_CURSOR_MOVE_DOWN;
        }

        break;

    case UX_MOUSE_CURSOR_MOVE_DOWN:  /* +y.  */

        mouse_x = 0;
        mouse_y = UX_DEMO_HID_MOUSE_CURSOR_MOVE;
        mouse_move_count ++;

        if (mouse_move_count >= UX_DEMO_HID_MOUSE_CURSOR_MOVE_N)
        {
            mouse_move_count = 0;
            mouse_move_dir = UX_MOUSE_CURSOR_MOVE_LEFT;
        }

        break;

    case UX_MOUSE_CURSOR_MOVE_LEFT:  /* -x. */

        mouse_x = (CHAR)(-UX_DEMO_HID_MOUSE_CURSOR_MOVE);
        mouse_y = 0;
        mouse_move_count ++;

        if (mouse_move_count >= UX_DEMO_HID_MOUSE_CURSOR_MOVE_N)
        {
            mouse_move_count = 0;
            mouse_move_dir = UX_MOUSE_CURSOR_MOVE_UP;
        }

        break;

    case UX_MOUSE_CURSOR_MOVE_UP:  /* -y. */

        mouse_x = 0;
        mouse_y = (UCHAR)(-UX_DEMO_HID_MOUSE_CURSOR_MOVE);
        mouse_move_count ++;

        if (mouse_move_count >= UX_DEMO_HID_MOUSE_CURSOR_MOVE_N)
        {
            mouse_move_count = 0;
            mouse_move_dir = UX_MOUSE_CURSOR_MOVE_DONE;
        }

        break;

    default:

        mouse_x = 0;
        mouse_y = 0;

        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        break;
    }

    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if(status != UX_SUCCESS)
        return UX_ERROR;

    return mouse_move_dir;
}

/*****************************************************************/
/**  ux_demo_hid_mouse_buttons: show how to click mouse button   */
/*****************************************************************/
UINT ux_demo_hid_mouse_buttons(UX_SLAVE_CLASS_HID *device_hid)
{
UINT                      status;
UX_SLAVE_CLASS_HID_EVENT  device_hid_event;
static UCHAR              button_status;

    /* Sleep thread for 10ms.  */
    ux_utility_delay_ms(MS_TO_TICK(10));

    /* Initialize mouse event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_MOUSE;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 4;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0; /* ...M|R|L  */
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0; /* X         */
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0; /* Y         */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0; /* Wheel     */

    switch(button_status)
    {
    case UX_MOUSE_BUTTON_PRESS_LEFT:

      /* Press L button.  */
      device_hid_event.ux_device_class_hid_event_buffer[0] = 0x01;

      button_status =  UX_MOUSE_BUTTON_PRESS_RIGHT;

      break;

     case UX_MOUSE_BUTTON_PRESS_RIGHT:

       /* Press R button.  */
       device_hid_event.ux_device_class_hid_event_buffer[0] = 0x02;

       button_status =  UX_MOUSE_BUTTON_PRESS_MIDDLE;

       break;

    case UX_MOUSE_BUTTON_PRESS_MIDDLE:

      /* Press M button.  */
      device_hid_event.ux_device_class_hid_event_buffer[0] = 0x04;

      button_status =  UX_MOUSE_BUTTON_PRESS_DONE;

      break;

    default:

        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        break;
    }

    /* Set the mouse event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if(status != UX_SUCCESS)
        return UX_ERROR;

    /* Sleep thread for 10ms.  */
    ux_utility_delay_ms(MS_TO_TICK(10));

    /* Release button.  */
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0x00;
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if(status != UX_SUCCESS)
        return UX_ERROR;

    return button_status;
}

/**************************************************************/
/**  ux_demo_hid_mouse_scroll_wheel: show how to move wheel   */
/**************************************************************/
UINT ux_demo_hid_mouse_scroll_wheel(UX_SLAVE_CLASS_HID *device_hid)
{
UINT                        status;
UX_SLAVE_CLASS_HID_EVENT    device_hid_event;
static UCHAR                mouse_wheel;
static UCHAR                wheel_move_dir;
static UCHAR                wheel_move_count;

    /* Sleep thread for 10ms.  */
    ux_utility_delay_ms(MS_TO_TICK(10));

    /* Initialize mouse event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_MOUSE;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 4;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0; /* ...M|R|L  */
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0; /* X         */
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0; /* Y         */
    device_hid_event.ux_device_class_hid_event_buffer[3] = mouse_wheel; /* Wheel     */

    /* Move wheel.  */
    switch(wheel_move_dir)
    {
    case UX_MOUSE_WHEEL_MOVE_DOWN:

        mouse_wheel = (UCHAR)(-UX_DEMO_HID_MOUSE_WHEEL_MOVE);

        wheel_move_count ++;

        if (wheel_move_count >= UX_DEMO_HID_MOUSE_WHEEL_MOVE_N)
        {
            wheel_move_count = 0;
            wheel_move_dir = UX_MOUSE_WHEEL_MOVE_UP;
        }

        break;

    case UX_MOUSE_WHEEL_MOVE_UP:

        mouse_wheel = UX_DEMO_HID_MOUSE_WHEEL_MOVE;

        wheel_move_count ++;

        if (wheel_move_count >= UX_DEMO_HID_MOUSE_WHEEL_MOVE_N)
        {
            wheel_move_count = 0;
            wheel_move_dir = UX_MOUSE_WHEEL_MOVE_DONE;
        }

        break;

    default:

        mouse_wheel = 0;

        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        break;
    }

    /* Set the mouse event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if(status != UX_SUCCESS)
      return UX_ERROR;

    return wheel_move_dir;
}

/********************************************************************/
/**  ux_demo_hid_keyboard_send_lowercaser_character:                */
/**          send lowercase character (a -> z)                      */
/********************************************************************/
UINT ux_demo_hid_keyboard_send_lowercase_character(UX_SLAVE_CLASS_HID *device_hid)
{
UCHAR                    status;
UCHAR                    key;
UX_SLAVE_CLASS_HID_EVENT device_hid_event;
static UCHAR             index;

    /* Wait for 10 seconds. */
    ux_utility_thread_sleep(MS_TO_TICK(50));

    key = keys_alphanumeric[index];

    /* Insert a key into the keyboard event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_KEYBOARD;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 8;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = key;   /* key */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[4] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[5] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[6] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[7] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Next event has the key depressed.  */
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Are we at the end of alphabet ?  */
    if (index < sizeof(keys_alphanumeric))
        /* Next key.  */
        index++;
    else
    {
        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        status = UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE;
    }

    return status;
}

/********************************************************************/
/**  ux_demo_hid_keyboard_send_uppercase_character:                 */
/**          send uppercase character (A -> Z)                      */
/********************************************************************/
UINT ux_demo_hid_keyboard_send_uppercase_character(UX_SLAVE_CLASS_HID *device_hid)
{
UCHAR                    status;
UCHAR                    key;
UX_SLAVE_CLASS_HID_EVENT device_hid_event;
static UCHAR             index;


    /* Wait for 20 seconds. */
    ux_utility_thread_sleep(MS_TO_TICK(50));

    key = keys_alphanumeric[index];

    /* Insert a key into the keyboard event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_KEYBOARD;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 8;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0x02;  /* Left Shift modifier */
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = key;   /* key */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[4] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[5] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[6] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[7] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Next event has the key depressed.  */
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Are we at the end of alphabet ?  */
    if (index < sizeof(keys_alphanumeric))
        /* Next key.  */
        index++;
    else
    {
        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        status = UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE;
    }

   return status;
}

/********************************************************************/
/**  ux_demo_hid_keyboard_send_number:                              */
/**          send number (0 -> 9)                                   */
/********************************************************************/
UINT ux_demo_hid_keyboard_send_number(UX_SLAVE_CLASS_HID *device_hid)
{
UCHAR                    status;
UCHAR                    number;
UX_SLAVE_CLASS_HID_EVENT device_hid_event;
static UCHAR             index;


    /* Wait for 20 seconds. */
    ux_utility_thread_sleep(MS_TO_TICK(50));

    number = keys_numeric[index];

    /* Insert a key into the keyboard event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_KEYBOARD;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 8;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = number;   /* key */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[4] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[5] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[6] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[7] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Next event has the key depressed.  */
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Are we at the end of numeric ?  */
    if (index < sizeof(keys_numeric))
        /* Next key.  */
        index++;
    else
    {
        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        status = UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE;
    }

   return status;
}

/********************************************************************/
/**  ux_demo_hid_keyboard_send_special_character:                   */
/**          send number (!, @, #, $, %, ^, &, *, (, ))             */
/********************************************************************/
UINT ux_demo_hid_keyboard_send_special_character(UX_SLAVE_CLASS_HID *device_hid)
{
UCHAR                    status;
UCHAR                    key;
UX_SLAVE_CLASS_HID_EVENT device_hid_event;
static UCHAR             index;


    /* Wait for 20 seconds. */
    ux_utility_thread_sleep(MS_TO_TICK(50));

    key = keys_numeric[index];

    /* Insert a key into the keyboard event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_KEYBOARD;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 8;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0x02;  /* Left Shift modifier */
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = key;   /* key */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[4] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[5] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[6] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[7] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Next event has the key depressed.  */
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Are we at the end of alphabet ?  */
    if (index < sizeof(keys_numeric))
        /* Next key.  */
        index++;
    else
    {
        ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

        status = UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE;
    }

   return status;
}

/********************************************************************/
/**  ux_demo_hid_keyboard_send_enter:                               */
/**          send enter (\r)                                        */
/********************************************************************/
UINT ux_demo_hid_keyboard_send_enter(UX_SLAVE_CLASS_HID *device_hid)
{
UCHAR                    status;
UX_SLAVE_CLASS_HID_EVENT device_hid_event;


    /* Wait for 20 seconds. */
    ux_utility_thread_sleep(MS_TO_TICK(50));

    /* Insert a key into the keyboard event.  */
    device_hid_event.ux_device_class_hid_event_report_id = HID_REPORT_ID_KEYBOARD;
    device_hid_event.ux_device_class_hid_event_report_type = UX_DEVICE_CLASS_HID_REPORT_TYPE_INPUT;
    device_hid_event.ux_device_class_hid_event_length = 8;
    device_hid_event.ux_device_class_hid_event_buffer[0] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[1] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0x28; /* ENTER key */
    device_hid_event.ux_device_class_hid_event_buffer[3] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[4] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[5] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[6] = 0;
    device_hid_event.ux_device_class_hid_event_buffer[7] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    /* Next event has the key depressed.  */
    device_hid_event.ux_device_class_hid_event_buffer[2] = 0;

    /* Set the keyboard event.  */
    status = ux_device_class_hid_event_set(device_hid, &device_hid_event);

    if (status != UX_SUCCESS)
        return status;

    ux_utility_memory_set(&device_hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

    status = UX_DEMO_HID_KEYBOARD_SEND_EVENT_DONE;

    return status;
}

static VOID ux_demo_error_callback(UINT system_level, UINT system_context, UINT error_code)
{
    /*
     * Refer to ux_api.h. For example,
     * UX_SYSTEM_LEVEL_INTERRUPT, UX_SYSTEM_CONTEXT_DCD, UX_DEVICE_HANDLE_UNKNOWN
     */
    printf("USBX error: system level(%d), context(%d), error code(0x%x)\r\n", system_level, system_context, error_code);
}
