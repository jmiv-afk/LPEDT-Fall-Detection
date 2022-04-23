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
  bool is_indication_inflight;
} ble_context;

enum characteristic_type {
  CHR_FREEFALL = 0x01,
  CHR_ACTIVITY = 0x02,
  CHR_DOUBLETAP = 0x03
};

typedef struct {
  uint8_t buf[2];
  unsigned int characteristic;
  bool is_indication_enabled;
  bool is_indication_pending;
  enum characteristic_type type;
} characteristic_context;

static ble_context ble_ctx;
static characteristic_context freefall_ctx;
static characteristic_context activity_ctx;
static characteristic_context doubletap_ctx;

static uint8_t advertising_set_handle = 0xff;

static void send_pending_indication();
static void write_and_send_indication(characteristic_context* ctx);

static void init_characteristics()
{
  memset(freefall_ctx.buf, 0x0, 2);
  freefall_ctx.characteristic = gattdb_fall_status;
  freefall_ctx.is_indication_enabled = false;
  freefall_ctx.type = CHR_FREEFALL;

  memset(activity_ctx.buf, 0x0, 2);
  activity_ctx.characteristic = gattdb_activity_status;
  activity_ctx.is_indication_enabled = false;
  activity_ctx.type = CHR_ACTIVITY;
}

void handle_ble_event(sl_bt_msg_t *evt)
{
  unsigned int sc;

//  if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal) return;

  switch (SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_boot_id:
    {
      uint8_t addr_type;
      uint8_t system_id[8];
  
      ble_ctx.is_connected = false;
      ble_ctx.is_indication_inflight = false;
      init_characteristics();

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
             492,     // min interval, time = val*1.25 ms
             510,     // max interval, time = val*1.25 ms
             2,       // latency
             600,     // timeout - must be larger than (1+latency)*2*max_interval
             0x0,
             0xff
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
                                  sl_bt_advertiser_connectable_scannable
                                  );
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
        LOG("Error sl_bt_advertiser_stop, sc=0x%x", sc);
      }

      sc = sl_bt_connection_set_parameters(
                         ble_ctx.conn_handle,
                         492,     // min interval, time = val*1.25 ms
                         510,     // max interval, time = val*1.25 ms
                         2,       // latency
                         600,     // timeout - must be larger than (1+latency)*2*max_interval
                         0x0,
                         0xff
                         );

      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_bt_connection_set_parameters, sc=0x%x", sc);
      }
      break;
    }

    case sl_bt_evt_connection_closed_id:
    {
      LOG("Connection closed");
      ble_ctx.is_connected = false;
      ble_ctx.is_indication_inflight = false;
      freefall_ctx.is_indication_enabled = false;
      activity_ctx.is_indication_enabled = false;
      doubletap_ctx.is_indication_enabled = false;
      freefall_ctx.is_indication_pending = false;
      activity_ctx.is_indication_pending = false;
      doubletap_ctx.is_indication_pending = false;

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
          switch (evt->data.evt_gatt_server_characteristic_status.characteristic)
          {
            case gattdb_fall_status:
              freefall_ctx.is_indication_enabled = true;
              break;
            case gattdb_activity_status:
              activity_ctx.is_indication_enabled = true;
              break;
            case gattdb_doubletap_status:
              doubletap_ctx.is_indication_enabled = true;
              break;
          }
        }
        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags 
            == gatt_disable)
        {
          // indications turned off!
          switch (evt->data.evt_gatt_server_characteristic_status.characteristic)
          {
            case gattdb_fall_status:
              freefall_ctx.is_indication_enabled = false;
              freefall_ctx.is_indication_pending = false;
              break;
            case gattdb_activity_status:
              activity_ctx.is_indication_enabled = false;
              activity_ctx.is_indication_pending = false;
              break;
            case gattdb_doubletap_status:
              doubletap_ctx.is_indication_enabled = false;
              doubletap_ctx.is_indication_pending = false;
              break;
          }
        }
      }
      else if (evt->data.evt_gatt_server_characteristic_status.status_flags 
          == sl_bt_gatt_server_confirmation)
      {
        // indication has been received
        ble_ctx.is_indication_inflight = false;
        send_pending_indication();
      }
      break;
    }

    case sl_bt_evt_gatt_server_indication_timeout_id:
    {
      //LOG("Indication timeout");
      ble_ctx.is_indication_inflight = false;
      send_pending_indication();
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
          LOG("Freefall detected");
          // set flags as index 0, value as index 1
          freefall_ctx.buf[0] = 0x0; // setup the flags
          freefall_ctx.buf[1] = 0x1;
          write_and_send_indication(&freefall_ctx);
        }
        if (source & INT_ACTIVITY)
        {
          LOG("Activity detected");
          // set flags as index 0, value as index 1
          activity_ctx.buf[0] = 0x0; // setup the flags
          activity_ctx.buf[1] = 0x1;
          write_and_send_indication(&activity_ctx);
        }
        if (source & INT_INACTIVITY)
        {
          LOG("Activity detected");
          // set flags as index 0, value as index 1
          activity_ctx.buf[0] = 0x0; // setup the flags
          activity_ctx.buf[1] = 0x0;
          write_and_send_indication(&activity_ctx);
        }
        if (source & INT_DOUBLE_TAP)
        {
          LOG("Doubletap detected");
          // set flags as index 0, value as index 1
          doubletap_ctx.buf[0] = 0x0; // setup the flags
          doubletap_ctx.buf[1] = 0x1;
          write_and_send_indication(&doubletap_ctx);
        }
      }
      break; 
    }

    default:
      break;

  } // end switch (SL_BT_MSG_ID(evt->header))

} // handle_ble_event();

static void send_pending_indication()
{
  if (freefall_ctx.is_indication_pending == true)
  {
    write_and_send_indication(&freefall_ctx);
    freefall_ctx.is_indication_pending = false;
  }
  else if (activity_ctx.is_indication_pending == true)
  {
    write_and_send_indication(&activity_ctx);
    activity_ctx.is_indication_pending = false;
  }
  else if (doubletap_ctx.is_indication_pending == true)
  {
    write_and_send_indication(&doubletap_ctx);
    doubletap_ctx.is_indication_pending = false;
  }

}

static void write_and_send_indication(characteristic_context* ctx)
{
  unsigned int sc;
  if (ble_ctx.is_connected 
      && !ble_ctx.is_indication_inflight
      && ctx->is_indication_enabled)
  {
    // write the attribute value to local gattdb
    sc = sl_bt_gatt_server_write_attribute_value(
                                        ctx->characteristic,
                                        0,                    // val offset
                                        1,                    // val len
                                        &ctx->buf[1]
                                        );
    if (sc != SL_STATUS_OK) 
    {
      LOG("Error sl_bt_gatt_server_write_attribute_value, sc=0x%x", sc);
    }

    // send the indication
    sc = sl_bt_gatt_server_send_indication(
                              ble_ctx.conn_handle,  // connection handle
                              ctx->characteristic,   // the characteristic
                              2,                    // len
                              &ctx->buf[0]           // data to transmit
                              );
    if (sc != SL_STATUS_OK) 
    {
      LOG("Error sl_bt_gatt_server_send_indication, sc=0x%x", sc);
    } 
    else
    {
      ble_ctx.is_indication_inflight = true;
    }
  } 
  else if (ble_ctx.is_connected && ble_ctx.is_indication_inflight)
  {
    // set a flag reminding us to send indication upon indication timeout 
    // or on indication confirmation
    ctx->is_indication_pending = true;
  }
}
