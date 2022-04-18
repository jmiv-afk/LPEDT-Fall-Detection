/* -----------------------------------------------------------------------------
 * @file   ble.c
 * @brief  
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#include "ble.h"

typedef struct {
  bd_addr local_addr;
  uint8_t conn_handle;
  bool is_connected;
  bool is_indications_enabled;
} ble_context;

static ble_context ble_ctx;
static uint8_t advertising_set_handle = 0xff;

void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_boot_id:
    {
      uint8_t addr_type;
      uint8_t system_id[8];
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
        LOG("Error sl_advertiser_create_set");
      }

      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
      {
        LOG("Error sl_advertiser_create_set");
      }

      break;
    }

    case sl_bt_evt_connection_opened_id:
    {
      ble_ctx.conn_handle = evt->data.evt_connection_opened.connection;
      break;
    }

    case sl_bt_evt_connection_closed_id:
    {
      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
      advertising_set_handle,
      sl_bt_advertiser_general_discoverable,
      sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;
    }

    default:
      break;
  } 

} // handle_ble_event();


