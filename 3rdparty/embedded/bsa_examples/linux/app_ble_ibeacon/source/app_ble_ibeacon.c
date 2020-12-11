/*****************************************************************************
**
**  Name:           app_ble_ibeacon.c
**
**  Description:    Bluetooth BLE ibeacon advertiser application
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#include <stdlib.h>
#include "app_xml_utils.h"
#include "app_utils.h"
#include "app_mgt.h"
#include "app_disc.h"
#include "app_tm_vse.h"
#include "app_dm.h"
#include "app_ble_ibeacon.h"

/*
* Defines
*/

/* max adv data length */
#define APP_BLE_IBEACON_MAX_ADV_DATA_LEN        25
/* BLE IBEACON Helper UUID Definitions */
#define APP_BLE_IBEACON_APP_IBEACON_UUID          0x9890
#define APP_BLE_IBEACON_APP_CLIENT_UUID          0x9891
#define APP_BLE_IBEACON_APP_SERV_UUID          0xC158
#define APP_BLE_IBEACON_APP_CHAR_UUID          0xC188

#define APP_BLE_IBEACON_APP_NUM_OF_HANDLES     10
#define APP_BLE_IBEACON_APP_ATTR_NUM           1
#define APP_BLE_IBEACON_APP_NUM_OF_SERVER      1
#define APP_BLE_IBEACON_APP_NUM_OF_CLIENT      1
#define APP_BLE_IBEACON_CLIENT_INFO_DATA_LEN   100

/*
 * Global Variables
 */

char app_ble_ibeacon_write_data[APP_BLE_IBEACON_CLIENT_INFO_DATA_LEN] = "https://www.youtube.com/user/BroadcomCorporation";
char app_ble_ibeacon_received_data[APP_BLE_IBEACON_CLIENT_INFO_DATA_LEN]; //Todo: This received data/URL needs to be passed to STB

UINT8 ibeacon_adv_data[APP_BLE_IBEACON_MAX_ADV_DATA_LEN] =
    {0x4C, 0x00, 0x02, 0x15,0xe2, 0xc5, 0x6d, 0xb5, 0xdf, 0xfb, 0x48, 0xd2, 0xb0, 0x60, 0xd0, 0xf5, 0xa7, 0x10, 0x96, 0xe0, 0x00, 0x00, 0x00, 0x00,0xC5};

tAPP_DISCOVERY_CB app_ble_ibeacon_init_disc_cb;

/*
 * Local functions
 */
void app_ble_ibeacon_profile_cback(tBSA_BLE_EVT event, tBSA_BLE_MSG *p_data);
BOOLEAN app_ble_ibeacon_find_vs_eir(UINT8 *p_eir);
int app_ble_ibeacon_le_client_close(void);
int app_ble_ibeacon_le_client_deregister(void);



/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_start_ibeacon
 **
 ** Description     start ibeacon
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_start_ibeacon(void)
{
    /* register BLE server app */
    app_ble_server_register(APP_BLE_IBEACON_APP_IBEACON_UUID, app_ble_ibeacon_profile_cback);
    GKI_delay(1000);

    /* create a BLE service */
    app_ble_ibeacon_create_service();
    GKI_delay(1000);

    /* add a characteristic */
    app_ble_ibeacon_add_char();
    GKI_delay(1000);

    /* start service */
    app_ble_ibeacon_start_service();
    GKI_delay(1000);

    /* Set BLE Visibility */
    app_dm_set_ble_visibility(TRUE, TRUE);

    /* start advertising */
    app_ble_ibeacon_start_ibeacon_adv(0);

    return 0;
}


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
int app_ble_ibeacon_start_ibeacon_adv(UINT8 inst_id)
{
    tBSA_DM_BLE_AD_MASK adv_data_mask = BSA_DM_BLE_AD_BIT_MANU | BSA_DM_BLE_AD_BIT_FLAGS;
    /* Below Ibeacon format is refered from the WEB example*/
    UINT8 flag_value = 0x1a;
    UINT8 companyIdentifier[2] = {0x4C,0x00};
    UINT8 ibeacon_adv_ind[2]={0x02, 0x15};
    UINT8 ibeacon_uuid[16]={0xe2, 0xc5, 0x6d, 0xb5, 0xdf, 0xfb, 0x48, 0xd2, 0xb0, 0x60, 0xd0, 0xf5, 0xa7, 0x10, 0x96, 0xe0};
    UINT8 major_num[2] = {0x00, 0x00};
    UINT8 minor_num[2] = {0x00, 0x00};
    UINT8 tx_pwr_2s_compl = 0xC5;
    int len = APP_BLE_IBEACON_MAX_ADV_DATA_LEN;
    int i=0,j=0;
    tBSA_DM_BLE_ADV_CONFIG adv_conf;

    /* start advertising */
    memset(&adv_conf, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
    adv_conf.flag = flag_value;
    adv_conf.len = len;

    adv_conf.p_val[i++] = companyIdentifier[0];
    adv_conf.p_val[i++] = companyIdentifier[1];
    adv_conf.p_val[i++] = ibeacon_adv_ind[0];
    adv_conf.p_val[i++] = ibeacon_adv_ind[1];
    for (j=0; j<16;j++)
    {
       adv_conf.p_val[i++] = ibeacon_uuid[j];
    }
    adv_conf.p_val[i++] = major_num[0];
    adv_conf.p_val[i++] = major_num[1];
    adv_conf.p_val[i++] = minor_num[0];
    adv_conf.p_val[i++] = minor_num[1];
    adv_conf.p_val[i++] = tx_pwr_2s_compl;
    adv_conf.adv_data_mask = adv_data_mask;
    adv_conf.inst_id = inst_id;

    app_dm_set_ble_adv_data(&adv_conf);
    app_dm_set_ble_visibility(TRUE, TRUE);

    return 0;
}

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
int app_ble_ibeacon_stop_ibeacon_adv(void)
{
    /* make undiscoverable & unconnectable */
    app_dm_set_ble_visibility(FALSE, FALSE);

    return 0;
}

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
int app_ble_ibeacon_stop_ibeacon(void)
{
    app_ble_ibeacon_deregister();

    return 0;
}

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
int app_ble_ibeacon_deregister(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_DEREGISTER ble_deregister_param;

    APP_INFO0("Bluetooth BLE deregister menu:");
    if(app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].enabled != TRUE)
    {
        APP_ERROR1("Server was not registered! = %d", APP_BLE_IBEACON_APP_NUM_OF_SERVER-1);
        return -1;
    }

    status = BSA_BleSeAppDeregisterInit(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregisterInit failed status = %d", status);
        return -1;
    }

    ble_deregister_param.server_if = app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].server_if;

    status = BSA_BleSeAppDeregister(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregister failed status = %d", status);
        return -1;
    }

    return 0;
}

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
int app_ble_ibeacon_create_service(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_CREATE ble_create_param;

    status = BSA_BleSeCreateServiceInit(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateServiceInit failed status = %d", status);
        return -1;
    }

    ble_create_param.service_uuid.uu.uuid16 = APP_BLE_IBEACON_APP_SERV_UUID;
    ble_create_param.service_uuid.len = 2;
    ble_create_param.server_if = app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].server_if;
    ble_create_param.num_handle = APP_BLE_IBEACON_APP_NUM_OF_HANDLES;
    ble_create_param.is_primary = TRUE;

    app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].wait_flag = TRUE;

    status = BSA_BleSeCreateService(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateService failed status = %d", status);
        app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].wait_flag = FALSE;
        return -1;
    }

    /* store information on control block */
    app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].attr_UUID.len = 2;
    app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].attr_UUID.uu.uuid16 = APP_BLE_IBEACON_APP_SERV_UUID;
    app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].is_pri = ble_create_param.is_primary;

    return 0;
}

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
int app_ble_ibeacon_add_char(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_ADDCHAR ble_addchar_param;


    status = BSA_BleSeAddCharInit(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddCharInit failed status = %d", status);
        return -1;
    }

    ble_addchar_param.service_id = app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].service_id;
    ble_addchar_param.is_descr = FALSE;
    ble_addchar_param.char_uuid.len = 2;
    ble_addchar_param.char_uuid.uu.uuid16 = APP_BLE_IBEACON_APP_CHAR_UUID;
    ble_addchar_param.perm = BSA_GATT_PERM_READ|BSA_GATT_PERM_WRITE;
    ble_addchar_param.property = BSA_GATT_CHAR_PROP_BIT_NOTIFY |
                                 BSA_GATT_CHAR_PROP_BIT_WRITE |
                                 BSA_GATT_CHAR_PROP_BIT_INDICATE;

    APP_INFO1("app_ble_wifi_add_char service_id:%d", ble_addchar_param.service_id);

    status = BSA_BleSeAddChar(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddChar failed status = %d", status);
        return -1;
    }
    return 0;
}

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
int app_ble_ibeacon_start_service(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_START ble_start_param;

    status = BSA_BleSeStartServiceInit(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartServiceInit failed status = %d", status);
        return -1;
    }

    ble_start_param.service_id = app_ble_cb.ble_server[APP_BLE_IBEACON_APP_NUM_OF_SERVER-1].attr[APP_BLE_IBEACON_APP_ATTR_NUM-1].service_id;
    ble_start_param.sup_transport = BSA_BLE_GATT_TRANSPORT_LE_BR_EDR;

    APP_INFO1("service_id:%d, num:%d", ble_start_param.service_id, APP_BLE_IBEACON_APP_NUM_OF_SERVER-1);

    status = BSA_BleSeStartService(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartService failed status = %d", status);
        return -1;
    }
    return 0;
}

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
int app_ble_ibeacon_start_le_client(void)
{
    int ret_value = 0;

    /* register LE Client app */
    ret_value = app_ble_client_register(APP_BLE_IBEACON_APP_CLIENT_UUID);
    if (ret_value < 0)
    {
        APP_ERROR1("app_ble_client_register failed ret_value = %d", ret_value);
        return ret_value;
    }

    /* connect to IBEACON Server */
    ret_value = app_ble_client_open();
    if (ret_value < 0)
    {
        APP_ERROR1("app_ble_client_open failed ret_value = %d", ret_value);
        return ret_value;
    }

    return ret_value;
}

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
int app_ble_ibeacon_stop_le_client(void)
{
    /* close connection */
    app_ble_ibeacon_le_client_close();

    /* Deregister application */
    app_ble_ibeacon_le_client_deregister();

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_le_client_close
 **
 ** Description     This closes connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_le_client_close(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_CLOSE ble_close_param;

    if (app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].enabled == FALSE)
    {
        APP_ERROR1("Wrong client number! = %d", APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1);
        return -1;
    }
    status = BSA_BleClCloseInit(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClCLoseInit failed status = %d", status);
        return -1;
    }
    ble_close_param.conn_id = app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].conn_id;
    status = BSA_BleClClose(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClClose failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_ibeacon_le_client_deregister
 **
 ** Description     This deregister client app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_ibeacon_le_client_deregister(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_DEREGISTER ble_appdereg_param;

    if (app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].enabled == FALSE)
    {
        APP_ERROR1("Wrong client number! = %d", APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1);
        return -1;
    }
    status = BSA_BleClAppDeregisterInit(&ble_appdereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClAppDeregisterInit failed status = %d", status);
        return -1;
    }
    ble_appdereg_param.client_if = app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].client_if;
    status = BSA_BleClAppDeregister(&ble_appdereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleAppDeregister failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_init_disc_cback
 **
 ** Description      Discovery callback
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_ibeacon_init_disc_cback(tBSA_DISC_EVT event, tBSA_DISC_MSG *p_data)
{
    UINT16 index;

    switch (event)
    {
    /* a New Device has been discovered */
    case BSA_DISC_NEW_EVT:
        APP_INFO1("\tBdaddr:%02x:%02x:%02x:%02x:%02x:%02x",
                p_data->disc_new.bd_addr[0],
                p_data->disc_new.bd_addr[1],
                p_data->disc_new.bd_addr[2],
                p_data->disc_new.bd_addr[3],
                p_data->disc_new.bd_addr[4],
                p_data->disc_new.bd_addr[5]);
        APP_INFO1("\tName:%s", p_data->disc_new.name);

        if (p_data->disc_new.eir_data[0])
        {
            /* discover EIR to find a specific ADV message */
            if (app_ble_ibeacon_find_vs_eir(p_data->disc_new.eir_data))
            {
                /* check if this device has already been received (update) */
                for (index = 0; index < APP_DISC_NB_DEVICES; index++)
                {
                    if ((app_ble_ibeacon_init_disc_cb.devs[index].in_use == TRUE) &&
                        (!bdcmp(app_ble_ibeacon_init_disc_cb.devs[index].device.bd_addr, p_data->disc_new.bd_addr)))
                    {
                        /* Update device */
                        APP_INFO1("Update device:%d", index);
                        app_ble_ibeacon_init_disc_cb.devs[index].device = p_data->disc_new;
                        break;
                    }
                }

                /* if this is new discovered device */
                if (index >= APP_DISC_NB_DEVICES)
                {
                    /* Look for a free place to store dev info */
                    for (index = 0; index < APP_DISC_NB_DEVICES; index++)
                    {
                        if (app_ble_ibeacon_init_disc_cb.devs[index].in_use == FALSE)
                        {
                            APP_INFO1("New Discovered device:%d", index);
                            app_ble_ibeacon_init_disc_cb.devs[index].in_use = TRUE;
                            memcpy(&app_ble_ibeacon_init_disc_cb.devs[index].device, &p_data->disc_new,
                                sizeof(tBSA_DISC_REMOTE_DEV));
                            break;
                        }
                    }
                }
            }
        }
        break;

    case BSA_DISC_CMPL_EVT: /* Discovery complete. */
        break;

    case BSA_DISC_DEV_INFO_EVT: /* Discovery Device Info */
        break;

    default:
        break;
    }
}


/*******************************************************************************
 **
 ** Function         app_ble_ibeacon_find_vs_eir
 **
 ** Description      This function is used to find
 **                   a specific ADV content in EIR data
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN app_ble_ibeacon_find_vs_eir(UINT8 *p_eir)
{
    UINT8 *p = p_eir;
    UINT8 eir_length;
    UINT8 eir_tag;
    BOOLEAN discovered = FALSE;

    while(1)
    {
        /* Read Tag's length */
        STREAM_TO_UINT8(eir_length, p);

        if (eir_length == 0)
        {
            break;    /* Last Tag */
        }
        eir_length--;

        /* Read Tag Id */
        STREAM_TO_UINT8(eir_tag, p);
        APP_DEBUG1("app_ble_ibeacon_find_vs_eir, eir_tag:%d", eir_tag);

        switch(eir_tag)
        {
        case HCI_EIR_MANUFACTURER_SPECIFIC_TYPE:
            if (eir_length == APP_BLE_IBEACON_MAX_ADV_DATA_LEN)
            {
                if (!memcmp(p, &ibeacon_adv_data, APP_BLE_IBEACON_MAX_ADV_DATA_LEN))
                {
                    APP_INFO0("Discovered IBeacon Advertiser!");
                    discovered = TRUE;
                }
            }
            APP_DUMP("MANUFACTURER_SPECIFIC_TYPE", p, eir_length);
            break;
        default:
            break;
        }
        p += eir_length;
    }

    return discovered;
}

/*******************************************************************************
**
** Function         app_ble_ibeacon_profile_cback
**
** Description      BLE Server Profile callback.
**
** Returns          void
**
*******************************************************************************/
void app_ble_ibeacon_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data)
{
    int num, attr_index;

    APP_DEBUG1("event = %d ", event);

    switch (event)
    {
    case BSA_BLE_SE_DEREGISTER_EVT:
        APP_INFO1("BSA_BLE_SE_DEREGISTER_EVT server_if:%d status:%d",
            p_data->ser_deregister.server_if, p_data->ser_deregister.status);
        num = app_ble_server_find_index_by_interface(p_data->ser_deregister.server_if);
        if(num < 0)
        {
            APP_ERROR0("no deregister pending!!");
            break;
        }

        app_ble_cb.ble_server[num].server_if = BSA_BLE_INVALID_IF;
        app_ble_cb.ble_server[num].enabled = FALSE;
        break;

    case BSA_BLE_SE_CREATE_EVT:
        APP_INFO1("BSA_BLE_SE_CREATE_EVT server_if:%d status:%d service_id:%d",
            p_data->ser_create.server_if, p_data->ser_create.status, p_data->ser_create.service_id);

        num = app_ble_server_find_index_by_interface(p_data->ser_create.server_if);

        /* search interface number */
        if(num < 0)
        {
            APP_ERROR0("no interface!!");
            break;
        }

        /* search attribute number */
        for (attr_index = 0 ; attr_index < BSA_BLE_ATTRIBUTE_MAX ; attr_index++)
        {
            if (app_ble_cb.ble_server[num].attr[attr_index].wait_flag == TRUE)
            {
                APP_INFO1("BSA_BLE_SE_CREATE_EVT if_num:%d, attr_num:%d", num, attr_index);
                if (p_data->ser_create.status == BSA_SUCCESS)
                {
                    app_ble_cb.ble_server[num].attr[attr_index].service_id = p_data->ser_create.service_id;
                    app_ble_cb.ble_server[num].attr[attr_index].wait_flag = FALSE;
                    break;
                }
                else  /* if CREATE fail */
                {
                    memset(&app_ble_cb.ble_server[num].attr[attr_index], 0, sizeof(tAPP_BLE_ATTRIBUTE));
                    break;
                }
            }
        }
        if (attr_index >= BSA_BLE_ATTRIBUTE_MAX)
        {
            APP_ERROR0("BSA_BLE_SE_CREATE_EVT no waiting!!");
            break;
        }
        break;

    case BSA_BLE_SE_ADDCHAR_EVT:
        APP_INFO1("BSA_BLE_SE_ADDCHAR_EVT status:%d", p_data->ser_addchar.status);
        if (p_data->ser_addchar.status == BSA_SUCCESS)
        {
            APP_INFO1("attr_id:0x%x", p_data->ser_addchar.attr_id);
        }
        break;

    case BSA_BLE_SE_START_EVT:
        APP_INFO1("BSA_BLE_SE_START_EVT status:%d", p_data->ser_start.status);
        break;

    case BSA_BLE_SE_READ_EVT:
        APP_INFO1("BSA_BLE_SE_READ_EVT status:%d", p_data->ser_read.status);
        break;

    case BSA_BLE_SE_WRITE_EVT:
        APP_INFO1("BSA_BLE_SE_WRITE_EVT status:%d", p_data->ser_write.status);
        APP_DUMP("Write value", p_data->ser_write.value, p_data->ser_write.len);
        APP_INFO1("BSA_BLE_SE_WRITE_EVT trans_id:%d, conn_id:%d, handle:%d",
            p_data->ser_write.trans_id, p_data->ser_write.conn_id, p_data->ser_write.handle);
        memcpy(app_ble_ibeacon_received_data, p_data->ser_write.value, p_data->ser_write.len);
        APP_DEBUG1("Received value len:%d, val:%s", p_data->ser_write.len,app_ble_ibeacon_received_data);
        break;

    case BSA_BLE_SE_OPEN_EVT:
        APP_INFO1("BSA_BLE_SE_OPEN_EVT status:%d", p_data->ser_open.reason);
        if (p_data->ser_open.reason == BSA_SUCCESS)
        {
            APP_INFO1("conn_id:0x%x", p_data->ser_open.conn_id);
        }
        break;

    case BSA_BLE_SE_CLOSE_EVT:
        APP_INFO1("BSA_BLE_SE_CLOSE_EVT status:%d", p_data->ser_close.reason);
        break;

    default:
        break;
    }
}

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
int app_ble_ibeacon_send_data_to_ibeacon_server(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_WRITE ble_write_param;

    if (app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].write_pending)
    {
        APP_ERROR0("app_ble_wifi_write_information failed : write pending!");
        return -1;
    }

    status = BSA_BleClWriteInit(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWriteInit failed status = %d", status);
    }

    ble_write_param.char_id.char_id.uuid.len=2;
    ble_write_param.char_id.char_id.uuid.uu.uuid16 = APP_BLE_IBEACON_APP_CHAR_UUID;
    ble_write_param.char_id.srvc_id.id.uuid.len=2;
    ble_write_param.char_id.srvc_id.id.uuid.uu.uuid16 = APP_BLE_IBEACON_APP_SERV_UUID;
    ble_write_param.char_id.srvc_id.is_primary = 1;
    ble_write_param.len = strlen(app_ble_ibeacon_write_data);
    memcpy(ble_write_param.value, app_ble_ibeacon_write_data, ble_write_param.len);
    ble_write_param.descr = FALSE;
    ble_write_param.conn_id = app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].conn_id;
    ble_write_param.auth_req = BTA_GATT_AUTH_REQ_NONE;
    ble_write_param.write_type = BTA_GATTC_TYPE_WRITE_NO_RSP;

    app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].write_pending = TRUE;

    status = BSA_BleClWrite(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        app_ble_cb.ble_client[APP_BLE_IBEACON_APP_NUM_OF_CLIENT-1].write_pending = FALSE;
        APP_ERROR1("BSA_BleClWrite failed status = %d", status);
        return -1;
    }
    return 0;
}
