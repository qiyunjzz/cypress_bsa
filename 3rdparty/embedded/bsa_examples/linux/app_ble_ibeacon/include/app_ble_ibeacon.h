/*****************************************************************************
**
**  Name:           app_ble_ibeacon.h
**
**  Description:     Bluetooth BLE ibeacon advertiser application include file
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef APP_BLE_IBEACON_H
#define APP_BLE_IBEACON_H

#include "bsa_api.h"
#include "app_ble.h"
#include "app_ble_client_db.h"
#include "app_ble_server.h"
#include "app_ble_client.h"


/*
 * BLE common functions
 */
/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_start_ibeacon
 **
 ** Description     start ibeacon advertisement
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_start_ibeacon(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_start_ibeacon_adv
 **
 ** Description     start ibeacon advertisement
 **
 ** Parameters      inst_id : instance id for multi adv
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_start_ibeacon_adv(UINT8 inst_id);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_stop_ibeacon_adv
 **
 ** Description     stop ibeacon advertisement
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_stop_ibeacon_adv(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_stop_ibeacon
 **
 ** Description     stop ibeacon
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_stop_ibeacon(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_start_le_client
 **
 ** Description     start LE client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_start_le_client(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_stop_le_client
 **
 ** Description     stop LE client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_stop_le_client(void);

/*******************************************************************************
 **
 ** Function        app_ble_wifi_create_service
 **
 ** Description     create service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_create_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_add_char
 **
 ** Description     Add character to service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_add_char(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_start_service
 **
 ** Description     Start Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_start_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_deregister
 **
 ** Description     Deregister server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_deregister(void);

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_send_data_to_ibeacon_server
 **
 ** Description     Write data to Ibeacon Server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_send_data_to_ibeacon_server(void);

/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_init_disc_cback
 **
 ** Description      Discovery callback
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_ibeacon_init_disc_cback(tBSA_DISC_EVT event, tBSA_DISC_MSG *p_data);

#endif
