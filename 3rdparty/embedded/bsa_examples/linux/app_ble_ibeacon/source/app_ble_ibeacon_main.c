/*****************************************************************************
**
**  Name:           app_ble_ibeacon_main.c
**
**  Description:    Bluetooth BLE ibeacon advertiser application
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdlib.h>

#include "app_ble.h"
#include "app_utils.h"
#include "app_disc.h"
#include "app_mgt.h"
#include "app_dm.h"
#include "app_ble_client.h"
#include "app_ble_server.h"
#include "app_ble_ibeacon.h"

/*
 * Defines
 */

/* BLE IBEACON menu items */
enum
{
    APP_BLE_IBEACON_START_IBEACON_SERVER = 1,
    APP_BLE_IBEACON_START_IBEACON_ADV,
    APP_BLE_IBEACON_STOP_IBEACON_ADV,
    APP_BLE_IBEACON_STOP_IBEACON_SERVER,
    APP_BLE_IBEACON_DISCOVER_IBEACON_SERVER,
    APP_BLE_IBEACON_START_LE_CLIENT,
    APP_BLE_IBEACON_SEND_DATA2_IBEACON_SERVER,
    APP_BLE_IBEACON_STOP_LE_CLIENT,
    APP_BLE_IBEACON_MENU_QUIT = 99
};

/*
 * Global Variables
 */


/*
 * Local functions
 */
static void app_ble_ibeacon_menu(void);



/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_display_menu
 **
 ** Description      This is the IBEACON menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_ibeacon_display_menu (void)
{
    APP_INFO0("\t**** BLE IBEACON menu ***");
    APP_INFO1("\t%d => Start Ibeacon Server ", APP_BLE_IBEACON_START_IBEACON_SERVER);
    APP_INFO1("\t%d => Start Ibeacon Advertisement ", APP_BLE_IBEACON_START_IBEACON_ADV);
    APP_INFO1("\t%d => Stop Ibeacon Advertisement\n", APP_BLE_IBEACON_STOP_IBEACON_ADV);
    APP_INFO1("\t%d => Stop Ibeacon Server\n", APP_BLE_IBEACON_STOP_IBEACON_SERVER);
    APP_INFO0("\t**** BLE Client menu ***");
    APP_INFO1("\t%d => Discover Ibeacon Server", APP_BLE_IBEACON_DISCOVER_IBEACON_SERVER);
    APP_INFO1("\t%d => Start LE client", APP_BLE_IBEACON_START_LE_CLIENT);
    APP_INFO1("\t%d => Send data to Ibeacon Server", APP_BLE_IBEACON_SEND_DATA2_IBEACON_SERVER);
    APP_INFO1("\t%d => Stop LE client", APP_BLE_IBEACON_STOP_LE_CLIENT);
    APP_INFO1("\t%d => Quit", APP_BLE_IBEACON_MENU_QUIT);
}

/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_menu
 **
 ** Description      Ibeacon advertiser applicatoin sub menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_ibeacon_menu(void)
{
    int choice;

    do
    {
        app_ble_ibeacon_display_menu();

        choice = app_get_choice("Sub Menu");

        switch(choice)
        {
        case APP_BLE_IBEACON_START_IBEACON_SERVER:
            app_ble_ibeacon_start_ibeacon();
            break;

        case APP_BLE_IBEACON_START_IBEACON_ADV:
            app_ble_ibeacon_start_ibeacon_adv(0);
            break;

        case APP_BLE_IBEACON_STOP_IBEACON_ADV:
            app_ble_ibeacon_stop_ibeacon_adv();
            break;

        case APP_BLE_IBEACON_STOP_IBEACON_SERVER:
            app_ble_ibeacon_stop_ibeacon();
            break;

        case APP_BLE_IBEACON_DISCOVER_IBEACON_SERVER:
            app_disc_start_ble_regular(app_ble_ibeacon_init_disc_cback);
            break;

        case APP_BLE_IBEACON_START_LE_CLIENT:
            app_ble_ibeacon_start_le_client();
            break;

       case APP_BLE_IBEACON_SEND_DATA2_IBEACON_SERVER:
            app_ble_ibeacon_send_data_to_ibeacon_server();
            break;

        case APP_BLE_IBEACON_STOP_LE_CLIENT:
            app_ble_ibeacon_stop_le_client();
            break;

        case APP_BLE_IBEACON_MENU_QUIT:
            APP_INFO0("Quit");
            break;

        default:
            APP_ERROR1("Unknown choice:%d", choice);
            break;
        }
    } while (choice != APP_BLE_IBEACON_MENU_QUIT); /* While user don't exit sub-menu */
}

/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN app_ble_ibeacon_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        APP_DEBUG0("BSA_MGT_STATUS_EVT");
        if (p_data->status.enable)
        {
            APP_DEBUG0("Bluetooth restarted => re-initialize the application");
            app_ble_start();
        }
        break;

    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("app_management_callback BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;

     default:
        break;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function        main
 **
 ** Description     This is the main function
 **
 ** Parameters      Program's arguments
 **
 ** Returns         status
 **
 *******************************************************************************/
int main(int argc, char **argv)
{
    int status;

    /* Initialize BLE application */
    status = app_ble_init();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Initialize BLE app, exiting");
        exit(-1);
    }

    app_mgt_init();
    /* Open connection to BSA Server */
    status = app_mgt_open(NULL, app_ble_ibeacon_mgt_callback);
    if (status < 0)
    {
        APP_ERROR0("Unable to Connect to server");
        return -1;
    }

    /* Start BLE application */
    status = app_ble_start();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Start BLE app, exiting");
        exit(-1);
    }

    /* The main BLE loop */
    app_ble_ibeacon_menu();

    /* Exit BLE mode */
    app_ble_exit();

    /* Close BSA Connection before exiting (to release resources) */
    app_mgt_close();

    return 0;
}

