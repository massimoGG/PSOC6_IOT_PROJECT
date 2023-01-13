/*
 * bt.h
 *
 *  Created on: Jan 6, 2023
 *      Author: massimo
 */

#ifndef SOURCE_BT_H_
#define SOURCE_BT_H_

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "stdio.h"
#include "wiced_bt_dev.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "timers.h"
#include "cyhal.h"
#include "wiced_bt_ble.h"

/******************************************************************************
 *                                Constants
 ******************************************************************************/
#define CASE_RETURN_STR(enum_val)          case enum_val: return #enum_val;

#define FROM_BIT16_TO_8(val)            ((uint8_t)((val) >> 8 ))

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
/*Button interrupt configuration and handling functions*/
void   key_button_app_init         (void);
void   button_interrupt_handler    (void *handler_arg,
                                    cyhal_gpio_event_t event);

/* Callback function for Bluetooth stack management events */
wiced_bt_dev_status_t app_bt_management_callback(
		wiced_bt_management_evt_t event,
		wiced_bt_management_evt_data_t *p_event_data);

#endif /* SOURCE_BT_H_ */
