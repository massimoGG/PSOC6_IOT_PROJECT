/**
 * This file is the entry point for the state changes and button handling.
 * It also prepares the message to be sent over MQTT.
 */

#include "cy_result.h"
#include "cybsp_types.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stdio.h"
#include "stdlib.h"

/* FreeRTOS header files */
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <stdbool.h>
#include "queue.h"
#include "task.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "cy_wcm.h"
#include "cybsp.h"
#include "cyhal.h"

#include "mqtt_task.h"
#include "state.h"
#include "app_bt_utils.h"

/* Configuration file for MQTT client */
#include "mqtt_client_config.h"

/* Middleware libraries */
#include "cy_mqtt_api.h"
#include "cy_retarget_io.h"
#include "wiced_bt_ble.h"

/******************************************************************************
 * Macros
 ******************************************************************************/

#define TRIP_ALARM_DELAY_MS 500

/* The maximum number of times each PUBLISH in this example will be retried. */
#define PUBLISH_RETRY_LIMIT (10)

/* A PUBLISH message is retried if no response is received within this
 * time (in milliseconds).
 */
#define PUBLISH_RETRY_MS (1000)

/* Queue length of a message queue that is used to communicate with the
 * publisher task.
 */
#define PUBLISHER_TASK_QUEUE_LENGTH (5u)

#define GPIO_INTERRUPT_PRIORITY (7u)

/* Maximum number of retries for MQTT subscribe operation */
#define MAX_SUBSCRIBE_RETRIES (3u)

/* Time interval in milliseconds between MQTT subscribe retries. */
#define MQTT_SUBSCRIBE_RETRY_INTERVAL_MS (1000)

/* The number of MQTT topics to be subscribed to. */
#define SUBSCRIPTION_COUNT (1)

/* Queue length of a message queue that is used to communicate with the
 * subscriber task.
 */
#define SUBSCRIBER_TASK_QUEUE_LENGTH (1u)

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
cyhal_gpio_callback_data_t gpio_btn_callback_data;

/* The real state of the alarm system */
State state;

TaskHandle_t state_task_handle;

/* Queue tasks will use to set new state */
QueueHandle_t xStateQueue;

/* Structure to store publish message information. */
cy_mqtt_publish_info_t publish_info = {.qos = (cy_mqtt_qos_t)MQTT_MESSAGES_QOS,
                                       .topic = MQTT_PUB_TOPIC,
                                       .topic_len =
                                           (sizeof(MQTT_PUB_TOPIC) - 1),
                                       .retain = false,
                                       .dup = false};

/* Configure the subscription information structure. */
cy_mqtt_subscribe_info_t subscribe_info = {
    .qos = (cy_mqtt_qos_t)MQTT_MESSAGES_QOS,
    .topic = MQTT_SUB_TOPIC,
    .topic_len = (sizeof(MQTT_SUB_TOPIC) - 1)};

int led_state = CYBSP_LED_STATE_OFF;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
static void subscribe_to_topic(void);
static void unsubscribe_from_topic(void);

void init_state() {
  cy_rslt_t result;

  /* Initialize the user LED */
  result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                           CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
  /* User LED init failed. Stop program execution */
  if (result != CY_RSLT_SUCCESS) {
    printf("ERROR - Failed to init CYBSP_USER_LED\n");
  }

  /* Initialize the user button */
  result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                           CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
  /* User button init failed. Stop program execution */
  if (result != CY_RSLT_SUCCESS) {
    printf("ERROR - Failed to init CYBSP_USER_BTN\n");
  }

  /* Configure GPIO interrupt */
  gpio_btn_callback_data.callback = gpio_interrupt_handler;
  cyhal_gpio_register_callback(CYBSP_USER_BTN, &gpio_btn_callback_data);
  cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
                          GPIO_INTERRUPT_PRIORITY, true);

  /* Enable global interrupts */
  //__enable_irq();

  state.state = SEC_UNACTIVE;

  /*
   * MQTT Subscription init
   */
  subscribe_to_topic();
}

void state_task(void *pvParameters) {
  /* Status variable */
  cy_rslt_t result;

  State newState;

  /* Command to the MQTT client task */
  mqtt_task_cmd_t mqtt_task_cmd;

  /* To avoid compiler warnings */
  (void)pvParameters;

  char *buffer = malloc(BUFFER_SIZE);
  if (buffer == NULL) {
    printf("FATAL - Could not allocate buffer memory\n");
    return;
  }

  xStateQueue = xQueueCreate(2, sizeof(State));
  if (xStateQueue == NULL) {
    printf("FATAL - Could not create State Queue!\n");
    CY_ASSERT(0);
  }

  init_state();

  for (;;) {
    if (xQueueReceive(xStateQueue, &(newState), (TickType_t)10) == pdPASS) {
      result = CY_RSLT_SUCCESS;

      printf("Received: %d\n", newState.state);

      switch (newState.state) {
      case SEC_PAIRING:
        /* Pairing, Display code to MQTT */
        state.state = newState.state;
        sprintf(buffer, "{\"state\":\"PAIRING\",code:%d}", newState.meta.passkey);
        break;
      case SEC_ACTIVE:
      case SEC_DISCONNETED:
        /* Set new state */
        state.state = newState.state;
        sprintf(buffer, "{\"state\":\"ACTIVE\"}");

        /* Turn on LED */
        led_state = CYBSP_LED_STATE_ON;
        break;
      case SEC_UNACTIVE:
      case SEC_CONNECTED:
      
        /* Copy new state */
        memcpy(&state, &newState, sizeof(State));
        /* Pain but lazy */
        sprintf(buffer,
                "{\"state\":\"UNACTIVE\", bdaddr:%02X:%02X:%02X:%02X:%02X:%02X}",
                newState.meta.bdadr[0], newState.meta.bdadr[1], newState.meta.bdadr[2],
                newState.meta.bdadr[3], newState.meta.bdadr[4], newState.meta.bdadr[5]);
        xQueueSend(xStateQueue, &state, (TickType_t)10);
        /* Clear queue if the alarm was tripped */
        xQueueReset(xStateQueue);

        /* Turn off LED */
        led_state = CYBSP_LED_STATE_OFF;
        break;
      case SEC_BUTTON:
        /* Clear queue for potential trip */
        xQueueReset(xStateQueue);
        printf("Alarm disabled\n");
        result = CY_RSLT_TYPE_WARNING; /* Gewoon iets anders */
        State newState;
        newState.state = SEC_UNACTIVE; 
        xQueueSend(xStateQueue, &newState, (TickType_t)10);
        break;
      case SEC_TRIPPED:
        /* Set new state */
        state.state = SEC_TRIPPED;
        sprintf(buffer, "{\"state\":\"TRIPPED\"}");

        /* Toggle LED */
        led_state = led_state == CYBSP_LED_STATE_OFF ? CYBSP_LED_STATE_ON
                                                     : CYBSP_LED_STATE_OFF;
        vTaskDelay(pdMS_TO_TICKS(TRIP_ALARM_DELAY_MS));
        /* Report only if LED is on to limit MQTT spam */
        if (led_state == CYBSP_LED_STATE_ON)
          result = CY_RSLT_SUCCESS;

        /* Blink LED by sending this queue itself a message every x ms */
        xQueueSend(xStateQueue, &state, (TickType_t)10);
        break;
      case SEC_INIT:
        sprintf(buffer, "{\"state\":\"REINITIALIZING\"}");
        subscribe_to_topic();
        break;
      case SEC_GETSTATE:
        /* Do nothing excep run the if-statement below /*/
        break;
      default:
        sprintf(buffer, "{\"state\":\"ERROR\"}");
      }

      /* Update LED state */
      cyhal_gpio_write(CYBSP_USER_LED, led_state);

      if (result == CY_RSLT_SUCCESS) {
        /* Publish the data received over the message queue. */
        publish_info.payload = buffer;
        publish_info.payload_len = strlen(publish_info.payload);
        printf("  Publisher: Publishing '%s' on the topic '%s'\n\n",
               (char *)publish_info.payload, publish_info.topic);

        result = cy_mqtt_publish(mqtt_connection, &publish_info);

        if (result != CY_RSLT_SUCCESS) {
          printf("  Publisher: MQTT Publish failed with error 0x%0X.\n\n",
                 (int)result);

          /* Communicate the publish failure with the the MQTT
           * client task.
           */
          mqtt_task_cmd = HANDLE_MQTT_PUBLISH_FAILURE;
          xQueueSend(mqtt_task_q, &mqtt_task_cmd, portMAX_DELAY);
        }
      }
    }
  }

  /* Clean up */
  free(buffer);
  vTaskDelete(NULL);
}

/*******************************************************************************
 * Function Name: gpio_interrupt_handler
 ********************************************************************************
 * Summary:
 *   GPIO interrupt handler.
 *
 * Parameters:
 *  void *handler_arg (unused)
 *  cyhal_gpio_event_t (unused)
 *
 *******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg,
                                   cyhal_gpio_event_t event) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  State btnState;
  btnState.state = SEC_BUTTON;

  if (xStateQueue != NULL)
    xQueueSendFromISR(xStateQueue, &btnState, &xHigherPriorityTaskWoken);
}

static void subscribe_to_topic(void) {
  /* Status variable */
  cy_rslt_t result = CY_RSLT_SUCCESS;

  /* Command to the MQTT client task */
  mqtt_task_cmd_t mqtt_task_cmd;

  /* Subscribe with the configured parameters. */
  for (uint32_t retry_count = 0; retry_count < MAX_SUBSCRIBE_RETRIES;
       retry_count++) {
    result =
        cy_mqtt_subscribe(mqtt_connection, &subscribe_info, SUBSCRIPTION_COUNT);
    if (result == CY_RSLT_SUCCESS) {
      printf("MQTT client subscribed to the topic '%.*s' successfully.\n\n",
             subscribe_info.topic_len, subscribe_info.topic);
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(MQTT_SUBSCRIBE_RETRY_INTERVAL_MS));
  }

  if (result != CY_RSLT_SUCCESS) {
    printf("MQTT Subscribe failed with error 0x%0X after %d retries...\n\n",
           (int)result, MAX_SUBSCRIBE_RETRIES);

    /* Notify the MQTT client task about the subscription failure */
    mqtt_task_cmd = HANDLE_MQTT_SUBSCRIBE_FAILURE;
    xQueueSend(mqtt_task_q, &mqtt_task_cmd, portMAX_DELAY);
  }
}

static void unsubscribe_from_topic(void) {
  cy_rslt_t result = cy_mqtt_unsubscribe(
      mqtt_connection, (cy_mqtt_unsubscribe_info_t *)&subscribe_info,
      SUBSCRIPTION_COUNT);

  if (result != CY_RSLT_SUCCESS) {
    printf("MQTT Unsubscribe operation failed with error 0x%0X!\n",
           (int)result);
  }
}

void mqtt_subscription_callback(cy_mqtt_publish_info_t *received_msg_info) {
  /* Received MQTT message */
  const char *received_msg = received_msg_info->payload;
  int received_msg_len = received_msg_info->payload_len;

  printf("  Subsciber: Incoming MQTT message received:\n"
         "    Publish topic name: %.*s\n"
         "    Publish QoS: %d\n"
         "    Publish payload: %.*s\n\n",
         received_msg_info->topic_len, received_msg_info->topic,
         (int)received_msg_info->qos, (int)received_msg_info->payload_len,
         (const char *)received_msg_info->payload);

  /*
   * Decode received MQTT message and send to state handler through queue
   * Possible commands:
   ? - GETSTATE - Forces the PSOC to retransmit its state
   ? - TRIPALARM - Trips the alarm for testing-purposes
   */
  State cmdState;
  cmdState.state = SEC_INIT;

  if (strncmp(received_msg, "GETSTATE", 8) == 0)
    cmdState.state = SEC_GETSTATE;
  if (strncmp(received_msg, "TRIPALARM", 9) == 0)
    cmdState.state = SEC_TRIPPED;
  if (strncmp(received_msg, "DEACTIVATEALARM", 15) == 0)
    cmdState.state = SEC_UNACTIVE;
  if (strncmp(received_msg, "ACTIVATEALARM", 13) == 0)
    cmdState.state = SEC_ACTIVE;

  if (cmdState.state != SEC_INIT)
    xQueueSend(xStateQueue, &cmdState, (TickType_t)10);
}
