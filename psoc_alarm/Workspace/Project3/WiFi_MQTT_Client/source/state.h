#ifndef STATE_TASK_H_
#define STATE_TASK_H_

#include "queue.h"
#include "cy_mqtt_api.h"
#include "wiced_bt_dev.h"

/* Task parameters for Button TaTasksk. */
#define STATE_TASK_PRIORITY (1)
#define STATE_TASK_STACK_SIZE (1024 * 1)
#define BUFFER_SIZE (128)

/* enum for state machine */
enum States {
  SEC_ACTIVE,
  SEC_UNACTIVE,
  SEC_TRIPPED,

  SEC_CONNECTED,
  SEC_DISCONNETED,
  SEC_PAIRING,
  SEC_BUTTON,

  SEC_GETSTATE,
  SEC_INIT,
};

typedef union {
  wiced_bt_device_address_t bdadr;
  uint32_t passkey;
}StateMeta;

typedef struct {
  enum States state;
  StateMeta meta;
}State;

/* FreeRTOS task handle for this task. */
extern TaskHandle_t state_task_handle;

/* Queue tasks will use to set new state */
extern QueueHandle_t xStateQueue;

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/
void state_task(void *pvParameters);
static void subscribe_to_topic(void);
static void unsubscribe_from_topic(void);
void mqtt_subscription_callback(cy_mqtt_publish_info_t *received_msg_info);


#endif /* PUBLISHER_TASK_H_ */

/* [] END OF FILE */
