/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for MQTT Client Example for
 *ModusToolbox.
 *
 * Related Document: See README.md
 *
 *******************************************************************************
 * Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company)
 *or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/* Header file includes */
#include "wiced_bt_stack.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "mqtt_task.h"
#include "bt.h"
#include "state.h"

#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>
#include <task.h>
#include "cycfg_bt_settings.h"
#include <queue.h>

/* Queue size for LED and UART tasks*/
#define QUEUE_SIZE (3)

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes retarget IO, sets up
 *  the MQTT client task, and then starts the RTOS scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main() {
  cy_rslt_t result;
  wiced_result_t wiced_result;

  /* Initialize the board support package. */
  result = cybsp_init();
  CY_ASSERT(CY_RSLT_SUCCESS == result);

  /* To avoid compiler warnings. */
  (void)result;

  /* Enable global interrupts. */
  __enable_irq();

  /* Initialize retarget-io to use the debug UART port. */
  cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                      CY_RETARGET_IO_BAUDRATE);

  /* Create the MQTT Client task. */
  xTaskCreate(mqtt_client_task, "MQTT Client task", MQTT_CLIENT_TASK_STACK_SIZE,
              NULL, MQTT_CLIENT_TASK_PRIORITY, NULL);

  /* Create the state task and cleanup if the operation fails. */
  /*xTaskCreate(state_task, "State task", MQTT_CLIENT_TASK_STACK_SIZE, NULL,
              MQTT_CLIENT_TASK_PRIORITY, &state_task_handle);
  */

  /* Configure platform specific settings for the BT device */
  cybt_platform_config_init(&cybsp_bt_platform_cfg);

  /* Register call back and configuration with stack */
  wiced_result =
      wiced_bt_stack_init(app_bt_management_callback, &wiced_bt_cfg_settings);

  /* Check if stack initialization was successful */
  if (WICED_BT_SUCCESS == wiced_result) {
    printf("Bluetooth Stack Initialization Successful \n");
  } else {
    printf("Bluetooth Stack Initialization failed!! \n");
    CY_ASSERT(0);
  }

  /* Start the FreeRTOS scheduler. */
  vTaskStartScheduler();

  /* Should never get here. */
  CY_ASSERT(0);
}

/* [] END OF FILE */
