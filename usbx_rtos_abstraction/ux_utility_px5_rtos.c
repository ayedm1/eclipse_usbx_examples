/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"


#if defined(UX_PX5_RTOS_BIND)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_create                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a thread for USBX.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*    name                                  Pointer to thread name string */
/*    entry_function                        Entry function of the thread  */
/*    entry_input                           32-bit input value to thread  */
/*    stack_start                           Pointer to start of stack     */
/*    stack_size                            Stack size in bytes           */
/*    priority                              Priority of thread (0-31)     */
/*    preempt_threshold                     Preemption threshold          */
/*    time_slice                            Thread time-slice value       */
/*    auto_start                            Automatic start selection     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_create(UX_THREAD *thread_ptr, CHAR *name,
                VOID (*entry_function)(ULONG), ULONG entry_input,
                VOID *stack_start, ULONG stack_size,
                UINT priority, UINT preempt_threshold,
                ULONG time_slice, UINT auto_start)
{

UINT    status;


    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_delete                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a thread for USBX.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_cancel                           PX5 terminate thread       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_delete(UX_THREAD *thread_ptr)
{

UINT    status;


    /* Cancel the thread, effectively terminating it.  */
    status =  (UINT)pthread_cancel(thread_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_identify                         PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function return a pointer to the calling thread.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UX_THREAD *_ux_utility_thread_identify(VOID)
{


    /* If we're under interrupt, the thread returned by tx_thread_identify
        is the thread running prior to the ISR. Instead, we set it to null.  */
    return(UX_THREAD_GET_SYSTEM_STATE() ? UX_NULL : px5_globals.executing_thread[PX5_EXECUTING_CORE]);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_relinquish                       PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function helps the thread relinquish its control.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    sched_yield                            PX5 thread yield function    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
VOID  _ux_utility_thread_relinquish(VOID)
{

    /* Yield the thread.  */
    sched_yield();

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_resume                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resumes a thread for USBX.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_resume                      PX5 resume thread function  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_resume(UX_THREAD *thread_ptr)
{

UINT    status;


    /* Resume the thread.  */
    status =  (UINT)px5_pthread_resume(thread_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_schedule_other                   PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function force the scheduling of all other threads.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    caller_priority                        Priority to restore.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_priority_change           PX5 priority change           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_schedule_other(UINT caller_priority)
{

UINT        status;
UINT        old_priority;
UX_THREAD   *my_thread;

    UX_PARAMETER_NOT_USED(caller_priority);

    /* Call TX to know my own tread.  */
    my_thread = px5_globals.executing_thread[PX5_EXECUTING_CORE];

    /* Change the thread's priority.  */
    status =  (UINT)px5_pthread_priority_change(my_thread, (int)_ux_system -> ux_system_thread_lowest_priority, (int *)&old_priority);


    /* Check for error.  */
    if (status == UX_SUCCESS)
    {

        /* Wait until all other threads passed into the scheduler. */
        _ux_utility_thread_relinquish();

        /* And now return the priority of the thread to normal.  */
        status =  (UINT)px5_pthread_priority_change(my_thread, (int)_ux_system -> ux_system_thread_lowest_priority, (int *)&old_priority);


    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_sleep                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function causes the calling thread to sleep for the            */
/*    specified number of ticks.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ticks                                 Number of ticks to sleep      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_tick_sleep               PX5 sleep function             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_sleep(ULONG ticks)
{

UINT    status;

    /* Call PX5 sleep function.  */
    status =  (UINT)px5_pthread_tick_sleep((tick_t)ticks);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_thread_suspend                          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends thread for USBX.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Thread control block pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_suspend                  PX5 suspend thread service     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_suspend(UX_THREAD *thread_ptr)
{

UINT    status;


    /* Call PX5 to suspend USBX thread.  */
    status =  (UINT)px5_pthread_suspend(thread_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_mutex_create                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a protection mutex.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex                                 Pointer to mutex              */
/*    mutex_name                            Name of mutex                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_mutex_init                        PX5 mutex create          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_mutex_create(UX_MUTEX *mutex, CHAR *mutex_name)
{

UINT                status;
pthread_mutex_t     mutex_handle;
pthread_mutexattr_t mutex_attributes;


    /* Setup the mutex attribtues.  */
    pthread_mutexattr_init(&mutex_attributes);
    px5_pthread_mutexattr_setcontroladdr(&mutex_attributes, mutex, sizeof(px5_mutex_control));
    px5_pthread_mutexattr_setname(&mutex_attributes, mutex_name);

    /* Check if priority inheritance is desired.  */
    if (priority_inherit == 1)
    {

        /* Set priority inheritance.  */
        pthread_mutexattr_setprotocol(&mutex_attributes, PTHREAD_PRIO_INHERIT);
    }

    /* Mark the mutex as invalid.  */
    mutex_handle.mutex_handle_id =  0;

    /* Create the mutex.  */
    status =  (UINT)pthread_mutex_init(&mutex_handle, &mutex_attributes);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_mutex_create                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a protection mutex.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex                                 Pointer to mutex              */
/*    mutex_name                            Name of mutex                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_mutex_init                        PX5 mutex create          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_thread_preemption_change(UX_THREAD *thread_ptr, UINT new_threshold, UINT *old_threshold)
{

PX5_PROTECTION_SAVE

UINT status;
UINT new_priority;


    /* Are we adding a new threshold?  */
    if (!(new_threshold & 0x8000))
    {

        /* Get protection.  */
        PX5_PROTECTION_START

        /* Determine if we are disabling all preemptions.  */
        if (new_threshold == 0)
        {

            /* Set the priority to the maximum.  */
            new_priority =  PX5_MAXIMUM_PRIORITIES - 1;
        }
        else
        {

            /* We are not disabling all preemptions.  */

            /* Determine if the new threshold is higher than the thread's priority. */
            if (new_threshold > thread_ptr -> priority)
            {

                /* Set the priority to the threshold.  */
                new_priority =  new_threshold;
            }
            else
            {

                /* Set the priority to the thread's priority.  */
                new_priority =  thread_ptr -> priority;
            }
        }

        /* Increment the priority; we do this because upon restoring a previous threshold, this thread
           will be placed at the end of the ready list for that priority level causing threads ahead
           of it to run first, which is not the desired behavior.  */
        new_priority =  new_priority + 1;

        /* Determine if the new priority exceeds the maximum priorities.  */
        if (new_priority == PX5_MAXIMUM_PRIORITIES)
        {

            /* Set it to the maximum.  */
            new_priority =  PX5_MAXIMUM_PRIORITIES - 1;
        }

        /* Determine if this is the first threshold.  */
        if (thread_ptr -> preemption_change_nested_count == 0)
        {

            /* This is the first threshold.  We need to disable time-slicing for this thread to ensure it can't
               be preempted by threads of the same priority; note that this is only necessary when disabling
               all priorities (where the threshold is maximum), but we still do it in the other case as well for
               simplicity.  */

            /* Save the remaining time slice.  */
            thread_ptr -> preemption_change_original_time_slice_remaining =  thread_ptr -> time_slice_remaining;

            /* Disable time-slicing.  */
            thread_ptr -> time_slice_remaining =  0;
        }

        /* Change the priority.  */
        status = px5_pthread_priority_change(thread_ptr, new_priority, (int *)old_threshold);

        /* End and restore prior protection posture.  */
        PX5_PROTECTION_END

        /* Determine if the request succeeded.  */
        if (status == PX5_SUCCESS)
        {

            /* Flag the old threshold so we'll know later if we're restoring a previous one.  */
            *old_threshold |=  0x8000;

            /* Increment the nested count.  */
            thread_ptr -> preemption_change_nested_count++;
        }
    }
    else
    {

        /* We are restoring a previous threshold.  */

        /* Remove the flag.  */
        new_threshold &=  (~(0x8000));

        /* Decrement the nested count.  */
        thread_ptr -> preemption_change_nested_count--;

        /* Get protection.  */
        PX5_PROTECTION_START

        /* Determine if this is the last added threshold.  */
        if (thread_ptr -> preemption_change_nested_count == 0)
        {

            /* Re-enable time-slicing.  */
            thread_ptr -> time_slice_remaining =  thread_ptr -> preemption_change_original_time_slice_remaining;
        }

        /* Change the priority.  */
        status = px5_pthread_priority_change(thread_ptr, new_threshold, (int *)old_threshold);

        /* End and restore prior protection posture.  */
        PX5_PROTECTION_END
    }

    /* Return status to the caller.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_mutex_delete                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a protection mutex.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex                                 Pointer to mutex              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_create                       ThreadX mutex create          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_mutex_delete(UX_MUTEX *mutex)
{

UINT            status;
pthread_mutex_t mutex_handle;
UINT            old_threshold;


    /* Setup the handle.  */
    mutex_handle.mutex_handle_id =  PX5_MUTEX_HANDLE_ID;
    mutex_handle.internal_mutex_control =  mutex;

    /* Disable preemptions so we can atomically unlock and destroy the mutex.  */
    _ux_utility_thread_preemption_change(px5_globals.executing_thread[PX5_EXECUTING_CORE], 0, &old_threshold);

    /* Ensure the mutex has no owners before destroying, a condition for pthread_mutex_destroy().  To make this
       more robust, we would need to take into account nested locks and unlock it that number of times, however,
       for the middleware the mutex is only ever locked once whenever tx_mutex_delete is called.  */
    pthread_mutex_unlock(&mutex_handle);

    /* Destroy the mutex.  */
    status =  (UINT)pthread_mutex_destroy(&mutex_handle);

    /* Enable preemptions.  */
    _ux_utility_thread_preemption_change(px5_globals.executing_thread[PX5_EXECUTING_CORE], old_threshold, &old_threshold);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_mutex_off                               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases system protection.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Mutex                                                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_mutex_unlock                  ThreadX mutex put             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
VOID  _ux_utility_mutex_off(UX_MUTEX *mutex_ptr)
{

pthread_mutex_t mutex_handle;


    /* Setup the handle.  */
    mutex_handle.mutex_handle_id =  PX5_MUTEX_HANDLE_ID;
    mutex_handle.internal_mutex_control =  mutex_ptr;

    /* Unlock the mutex.  */
    result =  (u_int)pthread_mutex_unlock(&mutex_handle);

    /* Return to caller.  */
    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_mutex_on                                PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets system protection.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Mutex                                                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_mutex_lock                    PX5 mutex lock                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
VOID  _ux_utility_mutex_on(UX_MUTEX *mutex)
{

UINT            status;
pthread_mutex_t mutex_handle;


    /* Setup the handle.  */
    mutex_handle.mutex_handle_id =  PX5_MUTEX_HANDLE_ID;
    mutex_handle.internal_mutex_control =  mutex;

    /* Check the wait option.  */
    if (wait_option != UX_WAIT_FOREVER)
    {

        /* Only UX_WAIT_FOREVER is supported.  */

        /* Return status to the caller.  */
        return(UX_WAIT_ERROR);
    }

    /* Lock the mutex.  */
    status =  (UINT)pthread_mutex_lock(&mutex_handle);

    /* Check for status.  */
    if (status != UX_SUCCESS)
    {

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_UTILITY, status);
    }

    /* Return to caller.  */
    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_semaphore_create                        PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a semaphore.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore                             Semaphore to create           */
/*    semaphore_name                        Semaphore name                */
/*    initial_count                         Initial semaphore count       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_sem_extend_init                   PX5 semaphore create          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_semaphore_create(UX_SEMAPHORE *semaphore, CHAR *semaphore_name, UINT initial_count)
{

UINT        status;
sem_t       semaphore_handle;
semattr_t   semaphore_attributes;


    /* Setup the attributes for the semaphore.  */
    px5_semattr_init(&semaphore_attributes);
    px5_semattr_setcontroladdr(&semaphore_attributes, semaphore, sizeof(UX_SEMAPHORE));
    px5_semattr_setname(&semaphore_attributes, semaphore_name);

    /* Mark the semaphore as uninitialized.  */
    semaphore_handle.semaphore_handle_id =  0;

    /* Create the semaphore.  */
    status =  (UINT)px5_sem_extend_init(&semaphore_handle, 0, initial_count, &semaphore_attributes);

    /* Check for status.  */
    if (status != UX_SUCCESS)
    {

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_UTILITY, status);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_semaphore_delete                        PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the specified semaphore.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore                             Semaphore to delete           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    sem_destroy                   PX5 semaphore destroy                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_semaphore_delete(UX_SEMAPHORE *semaphore)
{

UINT    status;
sem_t   semaphore_handle;


    /* Setup the handle.  */
    semaphore_handle.semaphore_handle_id = PX5_SEMAPHORE_HANDLE_ID;
    semaphore_handle.internal_semaphore_control = semaphore;

    /* Destroy the semaphore.  */
    status =  (UINT)sem_destroy(&semaphore_handle);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_semaphore_get                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets a semaphore signal.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore                             Semaphore to get signal from  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_sem_wait                         PX5 semaphore wait             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_semaphore_get(UX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{

UINT        status;
UX_THREAD   *my_thread;
UINT        priority;
sem_t       semaphore_handle;

    /* Call TX to know my own tread.  */
    my_thread = px5_globals.executing_thread[PX5_EXECUTING_CORE];

    /* Is this the lowest priority thread in the system trying to use TX services ? */
    if (my_thread -> priority > _ux_system -> ux_system_thread_lowest_priority)
    {

        /* We need to remember this thread priority.  */
        _ux_system -> ux_system_thread_lowest_priority = priority;

    }

    /* Setup the handle.  */
    semaphore_handle.semaphore_handle_id = PX5_SEMAPHORE_HANDLE_ID;
    semaphore_handle.internal_semaphore_control = semaphore_ptr;

    /* Check the wait option.  */
    if (wait_option == 0xFFFFFFFF)
    {

        /* Wait for the semaphore.  */
        result =  (u_int)sem_wait(&semaphore_handle);
    }
    else
    {

        /* Wait for the semaphore.  */
        result =  (u_int)px5_sem_tickwait(&semaphore_handle, wait_option);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_semaphore_put                           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets a semaphore signal.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore                             Semaphore to signal           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    sem_post                        PX5 semaphore put                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_semaphore_put(UX_SEMAPHORE *semaphore)
{

UINT    status;
sem_t   semaphore_handle;


    /* Setup the handle.  */
    semaphore_handle.semaphore_handle_id = PX5_SEMAPHORE_HANDLE_ID;
    semaphore_handle.internal_semaphore_control = semaphore;

    /* Release the semaphore.  */
    status =  (UINT)sem_post(&semaphore_handle);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_event_flags_create                      PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a threadx group of flags                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                             Event flag control group      */
/*    name                                  Pointer to thread name string */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_event_flags_create            PX5 create event flag     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_event_flags_create(UX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr)
{

UINT    status;
pthread_event_flags_t       event_flags_handle;
pthread_event_flagsattr_t   event_flags_attributes;


    /* Setup the event flags attributes.  */
    px5_pthread_event_flagsattr_init(&event_flags_attributes);
    px5_pthread_event_flagsattr_setcontroladdr(&event_flags_attributes, group_ptr, sizeof(px5_event_flags_control));
    px5_pthread_event_flagsattr_setname(&event_flags_attributes, name_ptr);

    /* Mark the handle as uninitialized.  */
    event_flags_handle.event_flags_handle_id =  0;

    /* Create the event flags.  */
    status =  (UINT)px5_pthread_event_flags_create(&event_flags_handle, &event_flags_attributes);

    /* Check for status.  */
    if (status != UX_SUCCESS)
    {

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_UTILITY, UX_EVENT_ERROR);
    }
    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_event_flags_delete                      PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a threadx group of flags                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                             Event flag control group      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_event_flags_destroy           PX5 delete event flag     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_event_flags_delete(UX_EVENT_FLAGS_GROUP *group_ptr)
{

UINT                  status;
pthread_event_flags_t event_flags_handle;


    /* Setup the handle.  */
    event_flags_handle.event_flags_handle_id = PX5_EVENT_FLAGS_HANDLE_ID;
    event_flags_handle.internal_event_flags_control = group_ptr;

    /* Destroy the event flags.  */
    status =  (UINT)px5_pthread_event_flags_destroy(&event_flags_handle);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_event_flags_get                         PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function get event flags from event flag group                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                             Event flag control group      */
/*    requested_flags                       32 bits variable event flags  */
/*    get_option                            AND/OR/CLEAR ... options      */
/*    actual_flag_ptr                       where the flags are placed    */
/*    wait_option                           waiting option                */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_event_flags_wait              PX5 get event flag        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_event_flags_get(UX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                                        UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option)
{

UINT                  status;
pthread_event_flags_t event_flags_handle;


    /* Setup the handle.  */
    event_flags_handle.event_flags_handle_id = PX5_EVENT_FLAGS_HANDLE_ID;
    event_flags_handle.internal_event_flags_control = group_ptr;

    /* Check the wait option.  */
    if (wait_option != UX_WAIT_FOREVER)
    {

        /* Only UX_WAIT_FOREVER is supported.  */

        /* Return error. */
        return(UX_WAIT_ERROR);
    }

    /* Check the get option.  */
    if (get_option != UX_AND_CLEAR &&
        get_option != UX_OR_CLEAR)
    {

        /* Only UX_AND_CLEAR and UX_OR_CLEAR are supported.  */

        /* Return error.  */
        return(UX_OPTION_ERROR);
    }

    /* Wait for the event flags.  */
    result =  (u_int)px5_pthread_event_flags_wait(&event_flags_handle, requested_flags,
                                                  get_option == UX_AND_CLEAR ? PTHREAD_ALL_EVENTS : PTHREAD_ANY_EVENT,
                                                  actual_flags_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_event_flags_set                         PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function set event flags from event flag group                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                             Event flag control group      */
/*    flags_to_set                          32 bits variable event flags  */
/*    set_option                            set option                    */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_event_flags_set               PX5 set event flag        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_event_flags_set(UX_EVENT_FLAGS_GROUP *group_ptr, ULONG flags_to_set,
                                  UINT set_option)
{

UINT                  status;
pthread_event_flags_t event_flags_handle;


    /* Setup the handle.  */
    event_flags_handle.event_flags_handle_id = PX5_EVENT_FLAGS_HANDLE_ID;
    event_flags_handle.internal_event_flags_control = group_ptr;

    /* Check the set option.  */
    if (set_option != UX_OR)
    {

        /* Only UX_OR is supported.  */
        return(UX_OPTION_ERROR);
    }

    /* Set the event flags.  */
    status =  (UINT)px5_pthread_event_flags_set(&event_flags_handle, flags_to_set);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_timer_create                            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a timer.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer                                 Pointer to timer              */
/*    timer_name                            Name of timer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_ticktimer_create              PX5 timer create          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_timer_create(UX_TIMER *timer_ptr, CHAR *name_ptr, VOID (*expiration_function) (ULONG),
                                ULONG expiration_input, ULONG initial_ticks, ULONG reschedule_ticks,
                                UINT auto_activate)
{

UINT                    status;
pthread_ticktimer_t     ticktimer_handle;
pthread_ticktimerattr_t ticktimer_attributes;


    /* Setup the ticktimer attributes.  */
    px5_pthread_ticktimerattr_init(&ticktimer_attributes);
    px5_pthread_ticktimerattr_setcontroladdr(&ticktimer_attributes, timer_ptr, sizeof(px5_ticktimer_control));
    px5_pthread_ticktimerattr_setname(&ticktimer_attributes, name_ptr);

    /* Mark the ticktimer as uninitialized.  */
    ticktimer_handle.ticktimer_handle_id = 0;

    /* Create the ticktimer.  */
    status = (UINT)px5_pthread_ticktimer_create(&ticktimer_handle, &ticktimer_attributes, _px5_threadx_timer_expiration_function,
                                               (VOID *)expiration_input, initial_ticks, reschedule_ticks);

    /* Save the expiration function and input; this must be done after the create call so the internal
       ticktimer has been allocated.  */
    timer_ptr -> threadx_timer_expiration_function = expiration_function;
    timer_ptr -> threadx_timer_expiration_function_input = expiration_input;

    /* Check auto activation.  */
    if (auto_activate == UX_AUTO_ACTIVATE)
    {

        /* Start the ticktimer.  */
        px5_pthread_ticktimer_start(&ticktimer_handle);
    }


    /* Check status.  */
    if (status != UX_SUCCESS)

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_UTILITY, status);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_utility_timer_delete                            PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a timer.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer                                 Pointer to timer              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    px5_pthread_ticktimer_destroy             PX5 timer delete          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Components                                                     */
/*                                                                        */
/**************************************************************************/
UINT  _ux_utility_timer_delete(UX_TIMER *timer_ptr)
{

UINT                status;
pthread_ticktimer_t ticktimer_handle;


    /* Setup the handle.  */
    ticktimer_handle.ticktimer_handle_id = PX5_TICKTIMER_HANDLE_ID;
    ticktimer_handle.internal_ticktimer_control = timer_ptr;

    /* Destroy the ticktimer.  */
    status =  (UINT)px5_pthread_ticktimer_destroy(&ticktimer_handle);

    /* Return completion status.  */
    return(status);
}

#endif
