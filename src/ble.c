/* -----------------------------------------------------------------------------
 * @file   ble.c
 * @brief  Bluetooth Low Energy event handling, application programming
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#include "ble.h"

typedef struct {
  bd_addr local_addr;
  uint8_t conn_handle;
  bool is_connected;
  bool is_indications_enabled;
  bool is_indication_inflight;
} ble_context;

static ble_context ble_ctx;
static uint8_t advertising_set_handle = 0xff;

void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

//  if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal) return;

  switch (SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_boot_id:
    {
      uint8_t addr_type;
      uint8_t system_id[8];
  
      ble_ctx.is_connected = false;
      ble_ctx.is_indications_enabled = false;
      ble_ctx.is_indication_inflight = false;

      sc = sl_bt_system_get_identity_address(&(ble_ctx.local_addr), &addr_type);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_system_get_identity_address");
      }

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_gatt_server_write_attribute_value");
      }

      sc = sl_bt_connection_set_default_parameters(
             0x0190,     // min interval, time = val*1.25 ms
             0x0320,     // max interval, time = val*1.25 ms
             0x0004,     // latency
             0x001A,     // timeout - must be larger than (1+latency)*2*max_interval
             0x0000,
             0x00ff
          );

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_advertiser_create_set");
      }

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
          advertising_set_handle,
          160, // min. adv. interval (milliseconds * 1.6)
          160, // max. adv. interval (milliseconds * 1.6)
          0,   // adv. duration
          0);  // max. num. adv. events
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_advertiser_set_timing");
      }

      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
                                  advertising_set_handle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_advertiser_start");
      }

      break;
    }

    case sl_bt_evt_connection_opened_id:
    {
      LOG("Connection opened");
      ble_ctx.conn_handle = evt->data.evt_connection_opened.connection;
      ble_ctx.is_connected = true;
      sc = sl_bt_advertiser_stop(advertising_set_handle);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_advertiser_stop");
      }
      break;
    }

    case sl_bt_evt_connection_closed_id:
    {
      LOG("Connection closed");
      ble_ctx.is_connected = false;
      ble_ctx.is_indications_enabled = false;
      ble_ctx.is_indication_inflight = false;

      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
                                  advertising_set_handle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_advertiser_start");
      }
      break;
    }


    case sl_bt_evt_gatt_server_characteristic_status_id:
    {
      LOG("Characteristic status");
      if (evt->data.evt_gatt_server_characteristic_status.status_flags 
          == sl_bt_gatt_server_client_config) 
      {
        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags 
            == gatt_indication)
        {
          // indications turned on!
          // evt->data.evt_gatt_server_characteristic_status.characteristic will have 
          // the characteristic for the indication request
          ble_ctx.is_indications_enabled = true;
        }
        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags 
            == gatt_disable)
        {
          // indications turned off!
          // evt->data.evt_gatt_server_characteristic_status.characteristic will have 
          // the characteristic for the indication request
          ble_ctx.is_indications_enabled = false;
        }
      }
      else if (evt->data.evt_gatt_server_characteristic_status.status_flags 
          == sl_bt_gatt_server_confirmation)
      {
        // indication has been received
        ble_ctx.is_indication_inflight = false;
      }
      break;
    }

    case sl_bt_evt_gatt_server_indication_timeout_id:
    {
      LOG("Indication timeout");
      ble_ctx.is_indication_inflight = false; 
      break;
    }

    case sl_bt_evt_system_external_signal_id:
    {
      LOG("External signal");
      uint32_t signals = evt->data.evt_system_external_signal.extsignals;
      if (signals & evt_accel_GPIO_INT1)
      {
        uint8_t source = 0;
        accel_determine_interrupt_source(&source);
        if (source & INT_FREE_FALL)
        {
          LOG("Free fall detected, TODO: write to attribute and send indication");
        }
        // TODO: add other events / handling here?
      }
      break; 
    }

    default:
      break;

  } // end switch (SL_BT_MSG_ID(evt->header))

} // handle_ble_event();


