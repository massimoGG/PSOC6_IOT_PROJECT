#include "bt.h"
#include "app_bt_utils.h"
#include "state.h"

#include "GeneratedSource/cycfg_bt_settings.h"
#include "GeneratedSource/cycfg_gap.h"
#include "GeneratedSource/cycfg_gatt_db.h"
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cybt_platform_trace.h"
#include "cyhal.h"
#include "stdio.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_stack.h"
#include "wiced_bt_types.h"
#include "wiced_data_types.h"
#include "wiced_memory.h"
#include <FreeRTOS.h>
#include <inttypes.h>
#include <queue.h>
#include <string.h>
#include <task.h>

/*******************************************************************
 * Variable Definitions
 ******************************************************************/
typedef void (*pfn_free_buffer_t)(uint8_t *);
static uint16_t connection_id = 0;
static wiced_bt_device_address_t connected_bda;
static wiced_bt_ble_advert_mode_t *p_adv_mode = NULL;

/* If true we will go into bonding mode. This will be set false if pre-existing
 * bonding info is available */
static wiced_bool_t bond_mode = WICED_TRUE;

typedef struct {
  wiced_bt_device_link_keys_t link_keys;
  wiced_bt_ble_privacy_mode_t privacy_mode;
} tBondInfo;

tBondInfo bondinfo;
/*Variable to store CCCD Data*/
uint16_t peer_cccd_data = {0};
/*Local Indentity Key*/
wiced_bt_local_identity_keys_t identity_keys = {0};

/*******************************************************************
 * Function Prototypes
 ******************************************************************/
/* Bluetooth LE functions*/

static wiced_bt_gatt_status_t
ble_app_gatt_event_handler(wiced_bt_gatt_evt_t event,
                           wiced_bt_gatt_event_data_t *p_event_data);
static void ble_app_init(void);
wiced_bt_gatt_status_t
ble_app_connect_handler(wiced_bt_gatt_connection_status_t *p_conn_status);

State newState;

/****************************************************************************
 *                              FUNCTION DEFINITIONS
 ***************************************************************************/

/**
 * Function Name:
 * ble_app_init
 *
 * Function Description:
 * @brief  Initialize the Application
 *
 * @param  None.
 *
 * @return None.
 */
void ble_app_init(void) {
  /* Allow peer to pair */
  wiced_bt_set_pairable_mode(WICED_TRUE, 0);

  /* Set Advertisement Data */
  wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                          cy_bt_adv_packet_data);

  /* Register with stack to receive GATT callback */
  wiced_bt_gatt_register(ble_app_gatt_event_handler);

  /* Initialize GATT Database */
  wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);

  /* Allow new devices to bond and disable security */
  bond_mode = TRUE;
  printf("Starting Undirected Advertisement \r\n\r\n");
  /* Start Undirected LE Advertisements on device startup. */
  wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
}

/**
 * Function Name:
 * app_bt_management_callback
 *
 * Function Description:
 * @brief This is a Bluetooth stack management event handler function to receive
 events
 *        from BLE stack and process as per the application.
 *
 * @param  wiced_bt_management_evt_t event : BLE event code of one byte length
 wiced_bt_management_evt_data_t *p_event_data: Pointer to BLE management event
 *         structures
 *
 * @return wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
 *
 */

/* Bluetooth Management Event Handler */
wiced_result_t
app_bt_management_callback(wiced_bt_management_evt_t event,
                           wiced_bt_management_evt_data_t *p_event_data) {
  wiced_bt_dev_status_t status = WICED_BT_SUCCESS;
  wiced_bt_device_address_t bda = {0};
  wiced_bt_dev_ble_pairing_info_t *p_ble_info = NULL;

  switch (event) {
  case BTM_ENABLED_EVT:
    /* Bluetooth Controller and Host Stack Enabled */
    if (WICED_BT_SUCCESS == p_event_data->enabled.status) {
      wiced_bt_dev_read_local_addr(bda);
      printf("Local Bluetooth Address: ");
      print_bd_address(bda);
      /* Perform application-specific initialization */
      ble_app_init();
    } else {
      printf("Bluetooth Disabled \n");
    }
    break;

  case BTM_DISABLED_EVT:
    /* Bluetooth Controller and Host Stack Disabled */
    printf("Bluetooth Disabled \r\n");
    break;

  case BTM_PASSKEY_NOTIFICATION_EVT:
    /* Print passkey to the screen so that the user can enter it. */
    printf("*******************************************************************"
           "***\n");
    printf("Passkey Notification\n");
    printf("PassKey: %" PRIu32 " \n",
           p_event_data->user_passkey_notification.passkey);
    printf("*******************************************************************"
           "***\n");
    /*for simplicity we are confirming the passkey, end users may want to
     * implement their own input method*/
    wiced_bt_dev_confirm_req_reply(
        WICED_BT_SUCCESS, p_event_data->user_passkey_notification.bd_addr);

    /* Send to MQTT */
    newState.state = SEC_PAIRING;
    newState.meta.passkey = p_event_data->user_passkey_notification.passkey;
    if (xStateQueue != NULL)
      xQueueSend(xStateQueue, &newState, (TickType_t)10);
    break;

  case BTM_SECURITY_REQUEST_EVT:
    /* Security Request */
    /* Only grant if we are in bonding mode */
    if (TRUE == bond_mode) {
      printf("Security Request Granted \n");
      wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,
                                  WICED_SUCCESS);
    } else {
      printf("Security Request Denied - not in bonding mode \n");
    }
    break;

  case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
    /* Request for Pairing IO Capabilities (BLE) */
    printf("BLE Pairing IO Capabilities Request\n");
    /* IO Capabilities on this Platform */
    p_event_data->pairing_io_capabilities_ble_request.local_io_cap =
        BTM_IO_CAPABILITIES_DISPLAY_AND_YES_NO_INPUT;
    p_event_data->pairing_io_capabilities_ble_request.auth_req =
        BTM_LE_AUTH_REQ_SC_MITM_BOND;
    p_event_data->pairing_io_capabilities_ble_request.init_keys =
        BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
    break;

  case BTM_BLE_CONNECTION_PARAM_UPDATE:
    printf("Connection parameter update status:%d, Connection Interval: %d, "
           "Connection Latency: %d, Connection Timeout: %d\n",
           p_event_data->ble_connection_param_update.status,
           p_event_data->ble_connection_param_update.conn_interval,
           p_event_data->ble_connection_param_update.conn_latency,
           p_event_data->ble_connection_param_update.supervision_timeout);
    break;

  case BTM_PAIRING_COMPLETE_EVT:

    /* Pairing is Complete */
    p_ble_info = &p_event_data->pairing_complete.pairing_complete_info.ble;
    printf("Pairing Status %s \n", get_bt_smp_status_name(p_ble_info->reason));

    if (WICED_BT_SUCCESS == p_ble_info->reason) /* Bonding successful */
    {
      printf("Successfully Bonded to: ");
      print_bd_address(p_event_data->pairing_complete.bd_addr);

      bond_mode = FALSE; /* remember that the device is now bonded, so disable
                        bonding */
    } else {
      printf("Bonding failed! \n");
    }
    break;

  case BTM_ENCRYPTION_STATUS_EVT:
    /* Encryption Status Change */
    printf("Encryption Status event for: ");
    print_bd_address(p_event_data->encryption_status.bd_addr);
    printf("Encryption Status event result: %d \n",
           p_event_data->encryption_status.result);

    if (memcmp(&(bondinfo.link_keys.bd_addr),
               p_event_data->encryption_status.bd_addr,
               sizeof(wiced_bt_device_address_t)) == 0) {
      app_wicedbutton_mb1_client_char_config[0] = peer_cccd_data;
      /* Bonded */
      printf("Bond info present.\n");
    } else {
      printf("No Bond info present.\n");
    }
    break;

  case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
    /* save device keys to Flash */
    printf("Paired Device Key Update \r\n");
    wiced_bt_device_link_keys_t *link_key =
        &(p_event_data->paired_device_link_keys_update);
    memcpy(&bondinfo.link_keys, (uint8_t *)link_key,
           sizeof(wiced_bt_device_link_keys_t));

    printf("Successfully Bonded to AAAAAAAAAAAAAAAAAAA ");
    print_bd_address(p_event_data->paired_device_link_keys_update.bd_addr);

    /** REAL BT MAC */
    newState.state = SEC_CONNECTED;
    memcpy(newState.meta.bdadr,
           p_event_data->paired_device_link_keys_update.bd_addr,
           sizeof(p_event_data->paired_device_link_keys_update.bd_addr));
    /* Send new state */
    if (xStateQueue != NULL)
      xQueueSend(xStateQueue, (void*)&newState, (TickType_t)10);
    break;

  case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
    /* Paired Device Link Keys Request */
    printf("Paired Device Link keys Request Event for device ");
    print_bd_address(
        (uint8_t *)(p_event_data->paired_device_link_keys_request.bd_addr));
    /* Need to search to see if the BD_ADDR we are looking for is in Flash. If
     * not, we return WICED_BT_ERROR and the stack */
    /* will generate keys and will then call
     * BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT so that they can be stored */
    status = WICED_BT_ERROR; /* Assume the device won't be found. If it is, we
                                will set this back to WICED_BT_SUCCESS */

    printf("MEMCMP - ");
    print_bd_address(bondinfo.link_keys.bd_addr);
    if (memcmp(&(bondinfo.link_keys.bd_addr),
               p_event_data->paired_device_link_keys_request.bd_addr,
               sizeof(wiced_bt_device_address_t)) == 0) {
      /* Copy the keys to where the stack wants it */
      memcpy(&(p_event_data->paired_device_link_keys_request),
             &(bondinfo.link_keys), sizeof(wiced_bt_device_link_keys_t));
      status = WICED_BT_SUCCESS;
    } else {
      printf("Device Link Keys not found in the database! \n");
    }

    break;

  case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT: /* Update of local privacy keys -
                                              save to NVSRAM */
    /* Update of local privacy keys - save to Flash */
    printf("Local Identity Key Update\n");
    memcpy(&identity_keys,
           (uint8_t *)&(p_event_data->local_identity_keys_update),
           sizeof(wiced_bt_local_identity_keys_t));

    break;

  case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT: /* Request for local privacy keys -
                                               read from Flash */

    printf("Local Identity Key Request\r\n");
    /*Read Local Identity Resolution Keys*/
    memcpy(&(p_event_data->local_identity_keys_request), &(identity_keys),
           sizeof(wiced_bt_local_identity_keys_t));
    print_array(&identity_keys, sizeof(wiced_bt_local_identity_keys_t));
    status = WICED_BT_SUCCESS;

    break;

  case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
    /* Advertisement State Changed */
    p_adv_mode = &p_event_data->ble_advert_state_changed;
    // led_task_communicator(*p_adv_mode);
    printf("Advertisement State Change: %d\r\n", *p_adv_mode);
    break;

  default:
    printf("Unhandled Bluetooth Management Event: 0x%x %s\n", event,
           get_btm_event_name(event));
    break;
  }

  return status;
}

static wiced_bt_gatt_status_t
ble_app_gatt_event_handler(wiced_bt_gatt_evt_t event,
                           wiced_bt_gatt_event_data_t *p_event_data) {
  wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
  if (event == GATT_CONNECTION_STATUS_EVT) {
    status = ble_app_connect_handler(&p_event_data->connection_status);
  }
  return status;
}

/**
 * Function Name:
 * ble_app_connect_handler
 *
 * Function Description:
 * @brief  This function handles the connection and disconnection events. It
 * also stores the currently connected device information in hostinfo structure.
 *
 * @param   p_conn_status  contains information related to the
 * connection/disconnection event.
 *
 * @return  wiced_bt_gatt_status_t: See possible status codes in
 * wiced_bt_gatt_status_e in wiced_bt_gatt.h
 *
 */
wiced_bt_gatt_status_t
ble_app_connect_handler(wiced_bt_gatt_connection_status_t *p_conn_status) {
  wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

  if (NULL != p_conn_status) {
    if (p_conn_status->connected) {
      /* Device has connected */
      printf("Connected : BD Addr: ");
      print_bd_address(p_conn_status->bd_addr);
      printf("Connection ID '%d'\n", p_conn_status->conn_id);

      /* Handling the connection by updating connection ID */
      connection_id = p_conn_status->conn_id;
    } else {
      /* Device has disconnected */
      printf("\nDisconnected : BD Addr: ");
      print_bd_address(p_conn_status->bd_addr);
      printf("Connection ID '%d', Reason '%s'\n", p_conn_status->conn_id,
             get_bt_gatt_disconn_reason_name(p_conn_status->reason));

      /* Handling the disconnection */
      connection_id = 0;
      /* Reset the CCCD value so that on a reconnect CCCD will be off */
      app_wicedbutton_mb1_client_char_config[0] = 0;

      bond_mode = WICED_TRUE;
      /**
       * When Disconnected -> Start scanning for the previously bonded device
       * again
       */
      printf("\r\nStarting directed Advertisement to: ");
      print_bd_address(bondinfo.link_keys.bd_addr);
      print_bd_address(bondinfo.link_keys.conn_addr);
      printf("Enter 'e' for starting undirected Advertisement to add new "
             "device\r\n");
      wiced_bt_start_advertisements(BTM_BLE_ADVERT_DIRECTED_HIGH,
                                    bondinfo.link_keys.key_data.ble_addr_type,
                                    bondinfo.link_keys.bd_addr);

      newState.state = SEC_DISCONNETED;
      /* Send new state */
      if (xStateQueue != NULL)
        xQueueSend(xStateQueue, &newState, (TickType_t)10);
    }
    status = WICED_BT_GATT_SUCCESS;
  }

  return status;
}
