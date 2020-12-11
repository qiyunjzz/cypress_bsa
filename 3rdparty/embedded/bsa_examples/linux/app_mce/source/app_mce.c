/*****************************************************************************
**
**  Name:           app_mce.c
**
**  Description:    Bluetooth Message Access Profile client sample application
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
**  Copyright (C) 2017 Cypress Semiconductor Corporation
**
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "gki.h"
#include "uipc.h"
#include "bsa_api.h"
#include "app_xml_param.h"
#include "app_xml_utils.h"
#include "app_disc.h"
#include "app_utils.h"
#include "app_mce.h"


#define MAX_PATH_LENGTH             512
#define DEFAULT_SEC_MASK            (BSA_SEC_AUTHENTICATION | BSA_SEC_ENCRYPTION)

#define BSA_MA_32BIT_HEX_STR_SIZE   (8+1)

static tMceCallback *s_pMceCallback = NULL;

/*
* Global Variables
*/
tAPP_MCE_CB app_mce_cb;


/*******************************************************************************
**
** Function         app_mce_open_file
**
** Description      Open the temporary file that contains the push message that
**                  needs to be sent out in bmessage format.
**
** Parameters       p_con - pointer to the connection control block
**
** Returns          return the file descriptor
**
*******************************************************************************/
static int app_mce_open_file(tAPP_MCE_CONN *p_con)
{
    static char path[260];

    memset(path, 0, sizeof(path));
    if (!getcwd(path, 260))
    {
        strcat(path, ".");
    }
    strcat(path, "/tempoutmsg.txt");

    APP_INFO0("app_mce_open_file");

    if (p_con->fd <= 0)
    {
        p_con->fd = open(path, O_RDONLY);
    }

    if (p_con->fd <= 0)
    {
        APP_ERROR1("Error could not open input file %s", path);
        return -1;
    }
    return p_con->fd;
}

/*******************************************************************************
**
** Function         app_mce_push_msg_close
**
** Description      Close the temporary file that was opened for push msg data
**
** Parameters       session_handle - MAS session handle
**
** Returns          none
**
*******************************************************************************/
static void app_mce_push_msg_close(tBSA_MCE_SESS_HANDLE session_handle)
{
    tAPP_MCE_CONN *p_conn;

    if ((p_conn = app_mce_get_conn_by_sess_hdl(session_handle)) == NULL)
        return;

    if (p_conn->fd != -1)
    {
        close(p_conn->fd);
        p_conn->fd = -1;
    }
}

/*******************************************************************************
**
** Function         app_mce_send_push_msg_data_req
**
** Description      BSA requests application to provide push_msg data that needs to be sent
**                  to peer device. Push message data needs to be a complete buffer of the outgoing
**                  message in bmessage format.
**
** Parameters       int nb_bytes  - number of bytes requested by BSA
**
** Returns          0 if success -1 if failure
**
*******************************************************************************/
int app_mce_send_push_msg_data_req(tBSA_MCE_MSG *p_data)
{
    tAPP_MCE_CONN *p_conn;
    tBSA_MCE_SESS_HANDLE session_handle = p_data->push_data_req_msg.session_handle;
    int nb_bytes = p_data->push_data_req_msg.bytes_req;
    char *p_buf;
    int num_read;

    APP_INFO1("app_mce_send_push_msg_data_req sess_id %d bytes_req %d",
        session_handle, nb_bytes);

    if ((p_conn = app_mce_get_conn_by_sess_hdl(session_handle)) == NULL)
        return -1;

    /*  Need to allocate the buffer for specfied number of bytes */
    if ((p_buf = (char *) GKI_getbuf(nb_bytes + 1)) == NULL)
    {
        APP_ERROR0("app_mce_send_push_msg_data_req - COULD NOT ALLOCATE BUFFER");
        return -1;
    }

    memset(p_buf, 0, nb_bytes + 1);

    /* Read temporary file with outgoing message content in BMessage format and fill the buffer
       upto nb_bytes

       Your implementation could be different and might not necessarily use a temporary file to
       store outgoing message in BMessage format, in which case read() operation could be eliminated
       and supply data buffer directly to UIPC_Send.

       Data supplied to UIPC_Send has to be in BMessage format.
    */

    num_read = read(p_conn->fd, p_buf, nb_bytes);

    if (num_read < nb_bytes)
    {
        app_mce_push_msg_close(session_handle);
    }

    if (num_read < 0)
    {
        APP_ERROR0(" app_mce_send_push_msg_data_req - error in reading file");
    }
    else if (TRUE != UIPC_Send(p_conn->uipc_mce_tx_channel, 0, (UINT8 *) p_buf, num_read))
    {
        APP_ERROR0(" app_mce_send_push_msg_data_req - error in UIPC send could not send data");
    }

    GKI_freebuf(p_buf);

    return 0;
}

/*******************************************************************************
**
** Function         app_mce_get_available_conn
**
** Description      Get an available connection control block
**
** Parameters       bd_addr - BD address of the device
**
** Returns          tAPP_MCE_CONN *
**
*******************************************************************************/
static tAPP_MCE_CONN *app_mce_get_available_conn(BD_ADDR bd_addr)
{
    tAPP_MCE_CONN *p_conn = NULL;
    UINT8 i;

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
    {
        for (i = 0; i < APP_MCE_MAX_CONN; i++)
        {
            if ((bdcmp(app_mce_cb.conn[i].peer_addr, bd_addr) != 0) &&
                (app_mce_cb.conn[i].is_open == FALSE))
            {
                p_conn = &app_mce_cb.conn[i];
                break;
            }
        }
    }

    if (p_conn == NULL)
    {
        APP_ERROR0("app_mce_get_available_conn could not find empty entry");
    }

    return p_conn;
}

/*******************************************************************************
**
** Function         app_mce_get_available_inst
**
** Description      Get an available instance control block
**
** Parameters       p_con   - pointer to the connection control block
**                  bd_addr - BD address of the device
**
** Returns          tAPP_MCE_INST *
**
*******************************************************************************/
static tAPP_MCE_INST *app_mce_get_available_inst(tAPP_MCE_CONN *p_conn, BD_ADDR bd_addr)
{
    tAPP_MCE_INST *p_inst = NULL;
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_INST; i++)
    {
        if (p_conn->inst[i].state == APP_MCE_CONN_ST_IDLE)
        {
            p_inst = &p_conn->inst[i];
            break;
        }
    }

    if (p_inst == NULL)
    {
        APP_INFO0("app_mce_get_available_inst could not find empty entry");
    }

    return p_inst;
}

/*******************************************************************************
**
** Function         app_mce_get_conn_by_uipc_channel_id
**
** Description      Get the connection control block by UIPC channel id
**
** Parameters       channel_id - UIPC channel ID
**
** Returns          tAPP_MCE_CONN *
**
*******************************************************************************/
static tAPP_MCE_CONN *app_mce_get_conn_by_uipc_channel_id(tUIPC_CH_ID channel_id)
{
    tAPP_MCE_CONN *p_conn = NULL;
    UINT8 i;

    for (i = 0 ; i < APP_MCE_MAX_CONN; i++)
    {
        if ((app_mce_cb.conn[i].uipc_mce_rx_channel == channel_id) ||
            (app_mce_cb.conn[i].uipc_mce_tx_channel == channel_id))
        {
            p_conn = &app_mce_cb.conn[i];
            break;
        }
    }

    if (p_conn == NULL)
    {
        APP_INFO0("app_mce_get_conn_by_uipc_channel_id could not find connection");
    }

    return p_conn;
}

/*******************************************************************************
**
** Function         app_mce_get_conn_by_bd_addr
**
** Description      Get the connection control block by BD address
**
** Parameters       bd_addr - BD address of the device
**
** Returns          tAPP_MCE_CONN *
**
*******************************************************************************/
tAPP_MCE_CONN *app_mce_get_conn_by_bd_addr(BD_ADDR bd_addr)
{
    tAPP_MCE_CONN *p_conn = NULL;
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_CONN; i++)
    {
        if (bdcmp(app_mce_cb.conn[i].peer_addr, bd_addr) == 0)
        {
            p_conn = &app_mce_cb.conn[i];
            break;
        }
    }

    if (p_conn == NULL)
    {
        APP_INFO0("app_mce_get_conn_by_bd_addr could not find connection");
    }

    return p_conn;
}

/*******************************************************************************
**
** Function         app_mce_get_inst_by_inst_id
**
** Description      Get the instance control block by instance ID
**
** Parameters       p_con       - pointer to the connection control block
**                  instance_id - MAS instance ID
**
** Returns          tAPP_MCE_INST *
**
*******************************************************************************/
tAPP_MCE_INST *app_mce_get_inst_by_inst_id(tAPP_MCE_CONN *p_conn, tBSA_MCE_INST_ID instance_id)
{
    tAPP_MCE_INST *p_inst = NULL;
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_INST; i++)
    {
        if (p_conn->inst[i].state != APP_MCE_CONN_ST_IDLE &&
            p_conn->inst[i].instance_id == instance_id)
        {
            p_inst = &p_conn->inst[i];
            break;
        }
    }

    if (p_inst == NULL)
    {
        APP_INFO0("app_mce_get_inst_by_inst_id could not find instance");
    }

    return p_inst;
}

/*******************************************************************************
**
** Function         app_mce_get_inst_by_sess_hdl
**
** Description      Get the instance control block by session handle
**
** Parameters       p_con          - pointer to the connection control block
**                  session_handle - MAS session handle
**
** Returns          tAPP_MCE_INST *
**
*******************************************************************************/
tAPP_MCE_INST *app_mce_get_inst_by_sess_hdl(tAPP_MCE_CONN *p_conn, tBSA_MCE_SESS_HANDLE session_handle)
{
    tAPP_MCE_INST *p_inst = NULL;
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_INST; i++)
    {
        if (p_conn->inst[i].state != APP_MCE_CONN_ST_IDLE &&
            p_conn->inst[i].session_handle == session_handle)
        {
            p_inst = &p_conn->inst[i];
            break;
        }
    }

    if (p_inst == NULL)
    {
        APP_INFO0("app_mce_get_inst_by_sess_hdl could not find instance");
    }

    return p_inst;
}

/*******************************************************************************
**
** Function         app_mce_get_conn_by_sess_hdl
**
** Description      Get the connection control block by session handle
**
** Parameters       session_handle - MAS session handle
**
** Returns          tAPP_MCE_CONN *
**
*******************************************************************************/
tAPP_MCE_CONN *app_mce_get_conn_by_sess_hdl(tBSA_MCE_SESS_HANDLE session_handle)
{
    tAPP_MCE_CONN *p_conn = NULL;
    UINT8 i,j;

    for (i = 0; i < APP_MCE_MAX_CONN; i++)
    {
        for (j = 0; j < APP_MCE_MAX_INST; j++)
        {
            if (app_mce_cb.conn[i].inst[j].state != APP_MCE_CONN_ST_IDLE &&
                app_mce_cb.conn[i].inst[j].session_handle == session_handle)
            {
                p_conn = &app_mce_cb.conn[i];
                break;
            }
        }
    }

    if (p_conn == NULL)
    {
        APP_INFO0("app_mce_get_conn_by_sess_hdl could not find instance");
    }

    return p_conn;
}

/*******************************************************************************
**
** Function         app_mce_find_open_pending_inst
**
** Description      Find the open pending instance
**
** Parameters       p_con - pointer to the connection control block
**
** Returns          tAPP_MCE_INST *
**
*******************************************************************************/
static tAPP_MCE_INST *app_mce_find_open_pending_inst(tAPP_MCE_CONN *p_conn)
{
    tAPP_MCE_INST *p_inst = NULL;
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_INST; i++)
    {
        if (p_conn->inst[i].state == APP_MCE_CONN_ST_OPEN_PENDING)
        {
            p_inst = &p_conn->inst[i];
            break;
        }
    }

    if (p_inst == NULL)
    {
        APP_INFO0("app_mce_find_open_pending_inst could not find open pending instance");
    }

    return p_inst;
}

/*******************************************************************************
**
** Function         app_mce_is_open
**
** Description      Check if there is an open connection
**
** Parameters       p_con - pointer to the connection control block
**
** Returns          BOOLEAN
**
*******************************************************************************/
static BOOLEAN app_mce_is_open(tAPP_MCE_CONN *p_conn)
{
    UINT8 i;

    for (i = 0; i < APP_MCE_MAX_INST; i++)
    {
        if (p_conn->inst[i].state == APP_MCE_CONN_ST_CONNECTED)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*******************************************************************************
 **
 ** Function        app_mce_uipc_cback
 **
 ** Description     uipc mce call back function.
 **
 ** Parameters      pointer on a buffer containing Message data with a BT_HDR header
 **
 ** Returns         void
 **
 *******************************************************************************/
static void app_mce_uipc_cback(BT_HDR *p_msg)
{
    tAPP_MCE_CONN   *p_conn;
    tUIPC_CH_ID     uipc_channel;
    UINT8           *p_buffer = NULL;
    static char     path[260];
    static char     file_name[100];
    int             dummy = 0;
    int             fd = -1;

    APP_INFO1("app_mce_uipc_cback response received... p_msg = %x", p_msg);

    if (p_msg == NULL)
    {
        return;
    }

    uipc_channel = p_msg->layer_specific;
    if (((p_conn = app_mce_get_conn_by_uipc_channel_id(uipc_channel)) == NULL) ||
        (uipc_channel < UIPC_CH_ID_MCE_RX_FIRST) || (uipc_channel > UIPC_CH_ID_MCE_RX_LAST))
    {
        APP_ERROR1("app_mce_uipc_cback response received...event = %d, uipc channel = %d", p_msg->event, uipc_channel);
        GKI_freebuf(p_msg);
        return;
    }

    /* Make sure we are only handling UIPC_RX_DATA_EVT and other UIPC events. Ignore UIPC_OPEN_EVT and UIPC_CLOSE_EVT */
    if (p_msg->event != UIPC_RX_DATA_EVT)
    {
        APP_ERROR1("app_mce_uipc_cback response received...event = %d",p_msg->event);
        GKI_freebuf(p_msg);
        return;
    }

    APP_INFO1("app_mce_uipc_cback response received...event = %d, msg len = %d",p_msg->event, p_msg->len);

    /* Check msg len */
    if (!(p_msg->len))
    {
        APP_ERROR0("app_mce_uipc_cback response received. DATA Len = 0");
        GKI_freebuf(p_msg);
        return;
    }

    /* Write received buffer to file */
    memset(path, 0, sizeof(path));
    memset(file_name, 0, sizeof(file_name));
    if (!getcwd(path, 260))
    {
        strcat(path, ".");
    }
    sprintf(file_name, "/get_msg_%d.txt", uipc_channel);
    strcat(path, file_name);

    APP_INFO1("app_mce_uipc_cback file path = %s", path);

    /* Get to the data buffer */
    p_buffer = ((UINT8 *)(p_msg + 1)) + p_msg->offset;

    /* Delete temporary file */
    if (p_conn->remove)
    {
        unlink(path);
        p_conn->remove = FALSE;
    }

    /* Save incoming message content in temporary file */
    fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    APP_INFO1("app_mce_uipc_cback fd = %d", fd);

    if (fd != -1)
    {
        dummy = write(fd, p_buffer, p_msg->len);
        APP_INFO1("app_mce_uipc_cback dummy = %d err = %d", dummy, errno);
        (void)dummy;
        close(fd);
    }

    /* APP_DUMP("MCE Data", p_buffer, p_msg->len); */
    GKI_freebuf(p_msg);
}

/*******************************************************************************
**
** Function         app_mce_mn_open_evt
**
** Description      Example of mn open event callback function
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_mn_open_evt(tBSA_MCE_MSG *p_data)
{
    tAPP_MCE_CONN *p_conn;

    APP_INFO1("app_mce_mn_open_evt, status : %d", p_data->mn_open.status);

    if (p_data->mn_open.status == BSA_SUCCESS)
    {
        APP_INFO1("Session handle: %d", p_data->mn_open.session_handle);
        APP_INFO1("BD ADDR: [%02x] [%02x] [%02x] [%02x] [%02x] [%02x]",
            p_data->mn_open.bd_addr[0], p_data->mn_open.bd_addr[1],
            p_data->mn_open.bd_addr[2], p_data->mn_open.bd_addr[3],
            p_data->mn_open.bd_addr[4], p_data->mn_open.bd_addr[5]);

        if ((p_conn = app_mce_get_conn_by_bd_addr(p_data->mn_open.bd_addr)) == NULL)
        {
            APP_ERROR0("Could not find connection control block");
            return;
        }

        p_conn->is_mn_open = TRUE;
        p_conn->mn_session_handle = p_data->mn_open.session_handle;
    }
}

/*******************************************************************************
**
** Function         app_mce_mn_close_evt
**
** Description      Example of mn close event callback function
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_mn_close_evt(tBSA_MCE_MSG *p_data)
{
    tAPP_MCE_CONN *p_conn;

    APP_INFO1("app_mce_mn_close_evt, status : %d", p_data->mn_close.status);

    if (p_data->mn_close.status == BSA_SUCCESS)
    {
        APP_INFO1("Session handle: %d", p_data->mn_close.session_handle);
        APP_INFO1("BD ADDR: [%02x] [%02x] [%02x] [%02x] [%02x] [%02x]",
            p_data->mn_close.bd_addr[0], p_data->mn_close.bd_addr[1],
            p_data->mn_close.bd_addr[2], p_data->mn_close.bd_addr[3],
            p_data->mn_close.bd_addr[4], p_data->mn_close.bd_addr[5]);

        if ((p_conn = app_mce_get_conn_by_bd_addr(p_data->mn_close.bd_addr)) == NULL)
        {
            APP_ERROR0("Could not find connection control block");
            return;
        }

        p_conn->is_mn_open = FALSE;
        if ((p_conn->is_open == FALSE) && (p_conn->is_mn_open == FALSE))
        {
            memset(p_conn->peer_addr, 0, BD_ADDR_LEN);
        }
    }
}

/*******************************************************************************
**
** Function         app_mce_handle_list_evt
**
** Description      Example of list event callback function
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_handle_list_evt(tBSA_MCE_MSG *p_data)
{
    UINT8 * p_buf = p_data->list_data.data;

    p_buf[p_data->list_data.len] = '\0';
    APP_INFO1("%s", (char*)p_buf);
    APP_INFO1("BSA_MCE_LIST_EVT num entry %d,is final %d, xml %d, len %d",
        p_data->list_data.num_entry, p_data->list_data.is_final, p_data->list_data.is_xml, p_data->list_data.len);
}

/*******************************************************************************
**
** Function         app_mce_handle_notif_evt
**
** Description      Example of notif event callback function
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_handle_notif_evt(tBSA_MCE_MSG *p_data)
{
    UINT8 * p_buf = p_data->notif.data;

    APP_INFO1("BSA_MCE_NOTIF_EVT - final %d, len %d", p_data->notif.final, p_data->notif.len);
    APP_INFO1("Status : %d", p_data->notif.status);
    APP_INFO1("Session handle: %d", p_data->notif.session_handle);
    APP_INFO1("MCE Instance ID: %d", p_data->notif.instance_id);
    APP_INFO1("Event Data Length: %d", p_data->notif.len);

    if (p_data->notif.len > 0)
    {
        APP_INFO0("Event Data:");
        /* Print out data buffer */
        p_buf[p_data->notif.len] = '\0';
        APP_INFO1("%s", (char*)p_buf);
    }
}

/*******************************************************************************
**
** Function         app_mce_abort_evt
**
** Description      Response events received in response to ABORT command
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_abort_evt(tBSA_MCE_MSG *p_data)
{
    APP_INFO0("app_mce_abort_evt received...");

    APP_INFO1("Status : %d", p_data->abort.status);
    APP_INFO1("Session ID: %d",  p_data->abort.session_handle);
}

/*******************************************************************************
**
** Function         app_mce_get_mas_instances_evt
**
** Description      Response to get MAS instances call
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_get_mas_instances_evt(tBSA_MCE_MSG *p_data)
{
    UINT8 cnt = p_data->mas_instances.count;
    UINT8 i;

    APP_INFO0("app_mce_get_mas_instances_evt received...");
    APP_INFO1("Number of Instances: %d", cnt);

    for (i = 0; i < cnt; i++)
    {
        APP_INFO1("Instance ID: %d", p_data->mas_instances.mas_ins[i].instance_id);
        APP_INFO1("Message Type: %d", p_data->mas_instances.mas_ins[i].msg_type);
        APP_INFO1("Instance Name: %s", p_data->mas_instances.mas_ins[i].instance_name);
    }
}

/*******************************************************************************
**
** Function         app_mce_open_evt
**
** Description      Response to get MAS instances call
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_open_evt(tBSA_MCE_MSG *p_data)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;

    APP_INFO0("app_mce_open_evt received...");

    APP_INFO1("Status : %d", p_data->open.status);
    APP_INFO1("Session ID: %d", p_data->open.session_handle);
    APP_INFO1("MCE Instance ID: %d", p_data->open.instance_id);
    APP_INFO1("Remote bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
           p_data->open.bd_addr[0], p_data->open.bd_addr[1], p_data->open.bd_addr[2],
           p_data->open.bd_addr[3], p_data->open.bd_addr[4], p_data->open.bd_addr[5]);

    if ((p_conn = app_mce_get_conn_by_bd_addr(p_data->open.bd_addr)) == NULL)
    {
        APP_ERROR0("Could not find connection control block");
        return;
    }
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, p_data->open.instance_id)) == NULL)
    {
        APP_ERROR0("Could not find instance control block");
        return;
    }

    if (p_data->open.status == BSA_SUCCESS)
    {
        p_conn->is_open = TRUE;
        p_inst->session_handle = p_data->open.session_handle;
        p_inst->state = APP_MCE_CONN_ST_CONNECTED;

        if (!p_conn->is_channel_open)
        {
            /* Once connected to PBAP Server, UIPC channel is returned. */
            /* Open the channel and register a callback to get phone book data. */
            /* Save UIPC channel */
            p_conn->uipc_mce_rx_channel = p_data->open.uipc_mce_rx_channel;
            p_conn->uipc_mce_tx_channel = p_data->open.uipc_mce_tx_channel;

            p_conn->is_channel_open = TRUE;

            /* open the UIPC channel to receive the message related data */
            if (UIPC_Open(p_conn->uipc_mce_rx_channel, app_mce_uipc_cback) == FALSE)
            {
                p_conn->is_channel_open = FALSE;
                APP_ERROR0("Unable to open UIPC RX channel");
                return;
            }

            /* open the UIPC channel to transmit push message data to BSA Server*/
            if (UIPC_Open(p_conn->uipc_mce_tx_channel, NULL) == FALSE)
            {
                p_conn->is_channel_open = FALSE;
                APP_ERROR0("Unable to open UIPC TX channel");
                return;
            }
        }
    }
    else
    {
        p_inst->state = APP_MCE_CONN_ST_IDLE;
        APP_ERROR1("MCE open failed with error:%d", p_data->open.status);
    }
}

/*******************************************************************************
**
** Function         app_mce_close_evt
**
** Description      Response events received in response to CLOSE command
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_close_evt(tBSA_MCE_MSG *p_data)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;

    APP_INFO0("app_mce_close_evt received...");

    APP_INFO1("Status : %d", p_data->close.status);
    APP_INFO1("Session ID: %d", p_data->close.session_handle);
    APP_INFO1("MCE Instance ID: %d", p_data->close.instance_id);
    APP_INFO1("Remote bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
           p_data->close.bd_addr[0], p_data->close.bd_addr[1], p_data->close.bd_addr[2],
           p_data->close.bd_addr[3], p_data->close.bd_addr[4], p_data->close.bd_addr[5]);

    if ((p_conn = app_mce_get_conn_by_bd_addr(p_data->close.bd_addr)) == NULL)
    {
        APP_ERROR0("Could not find connection control block");
        return;
    }
    if ((p_inst = app_mce_get_inst_by_sess_hdl(p_conn, p_data->close.session_handle)) == NULL)
    {
        if ((p_inst = app_mce_find_open_pending_inst(p_conn)) == NULL)
        {
            APP_ERROR0("Could not find instance control block");
            return;
        }
    }

    if (p_data->close.status == BSA_SUCCESS)
    {
        p_inst->state = APP_MCE_CONN_ST_IDLE;
        p_conn->is_open = app_mce_is_open(p_conn);

        if ((p_conn->is_open == FALSE) && (p_conn->is_channel_open == TRUE))
        {
            /* close the UIPC channel receiving data */
            UIPC_Close(p_conn->uipc_mce_rx_channel);
            UIPC_Close(p_conn->uipc_mce_tx_channel);
            p_conn->uipc_mce_rx_channel = UIPC_CH_ID_BAD;
            p_conn->uipc_mce_tx_channel = UIPC_CH_ID_BAD;
            p_conn->is_channel_open = FALSE;
        }

        if ((p_conn->is_open == FALSE) && (p_conn->is_mn_open == FALSE))
        {
            memset(p_conn->peer_addr, 0, BD_ADDR_LEN);
        }
    }
}

/*******************************************************************************
**
** Function         app_mce_start_evt
**
** Description      Response events received in response to START command
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_start_evt(tBSA_MCE_MSG *p_data)
{
    APP_INFO0("app_mce_start_evt received...");

    APP_INFO1("Status : %d", p_data->mn_start.status);
    APP_INFO1("Session ID: %d",  p_data->mn_start.session_handle);

    if (p_data->mn_start.status == BSA_SUCCESS)
    {
        app_mce_cb.is_mns_started = TRUE;
    }
}

/*******************************************************************************
**
** Function         app_mce_stop_evt
**
** Description      Response events received in response to STOP command
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_stop_evt(tBSA_MCE_MSG *p_data)
{
    UINT8 i;

    APP_INFO0("app_mce_stop_evt received...");

    APP_INFO1("Status : %d", p_data->mn_stop.status);
    APP_INFO1("Session ID: %d", p_data->mn_stop.session_handle);

    if (p_data->mn_stop.status == BSA_SUCCESS)
    {
        app_mce_cb.is_mns_started = FALSE;

        for (i = 0; i < APP_MCE_MAX_CONN; i++)
        {
            /* if MN server had active connections, it would be closed already in obex layer */
            if (app_mce_cb.conn[i].is_mn_open == TRUE)
               app_mce_cb.conn[i].is_mn_open = FALSE;
        }
    }
}

/*******************************************************************************
**
** Function         app_mce_cback
**
** Description      Example of MCE callback function to handle MCE related events
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
static void app_mce_cback(tBSA_MCE_EVT event, tBSA_MCE_MSG *p_data)
{
    switch (event)
    {
    case BSA_MCE_OPEN_EVT:
        APP_INFO0("BSA_MCE_OPEN_EVT");
        app_mce_open_evt(p_data);
        break;
    case BSA_MCE_CLOSE_EVT:
        APP_INFO0("BSA_MCE_CLOSE_EVT");
        app_mce_close_evt(p_data);
        break;
    case BSA_MCE_START_EVT:
        APP_INFO0("BSA_MCE_START_EVT");
        app_mce_start_evt(p_data);
        break;
    case BSA_MCE_STOP_EVT:
        APP_INFO0("BSA_MCE_STOP_EVT");
        app_mce_stop_evt(p_data);
        break;
    case BSA_MCE_MN_OPEN_EVT:
        APP_INFO0("BSA_MCE_MN_OPEN_EVT");
        app_mce_mn_open_evt(p_data);
        break;
    case BSA_MCE_MN_CLOSE_EVT:
        APP_INFO0("BSA_MCE_MN_CLOSE_EVT");
        app_mce_mn_close_evt(p_data);
        break;
    case BSA_MCE_NOTIF_EVT:
        APP_INFO0("BSA_MCE_NOTIF_EVT");
        app_mce_handle_notif_evt(p_data);
        break;
    case BSA_MCE_NOTIF_REG_EVT:
        APP_INFO1("BSA_MCE_NOTIF_REG_EVT status %d, sess_id %d",
            p_data->notif_reg.status, p_data->notif_reg.session_handle);
        break;
    case BSA_MCE_SET_MSG_STATUS_EVT:
        APP_INFO1("BSA_MCE_SET_MSG_STATUS_EVT status %d, sess_id %d",
            p_data->set_msg_sts.status, p_data->set_msg_sts.session_handle);
        break;
    case BSA_MCE_UPDATE_INBOX_EVT:
        APP_INFO1("BSA_MCE_UPDATE_INBOX_EVT status %d, sess_id %d",
            p_data->upd_ibx.status, p_data->upd_ibx.session_handle);
        break;
    case BSA_MCE_SET_FOLDER_EVT:
        APP_INFO1("BSA_MCE_SET_FOLDER_EVT status %d, sess_id %d",
            p_data->set_folder.status, p_data->set_folder.session_handle);
        break;
    case BSA_MCE_FOLDER_LIST_EVT:
        APP_INFO0("BSA_MCE_FOLDER_LIST_EVT");
        app_mce_handle_list_evt(p_data);
        break;
    case BSA_MCE_MSG_LIST_EVT:
        APP_INFO0("BSA_MCE_MSG_LIST_EVT");
        app_mce_handle_list_evt(p_data);
        break;
    case BSA_MCE_GET_MSG_EVT:
        APP_INFO1("BSA_MCE_SET_FOLDER_EVT status %d, sess_id %d",
            p_data->getmsg_msg.status, p_data->getmsg_msg.session_handle);
        break;
    case BSA_MCE_PUSH_MSG_DATA_REQ_EVT:
        APP_INFO0("BSA_MCE_PUSH_MSG_DATA_REQ_EVT");
        app_mce_send_push_msg_data_req(p_data);
        break;
    case BSA_MCE_PUSH_MSG_EVT:
        APP_INFO1("BSA_MCE_PUSH_MSG_EVT status %d, sess_id %d",
            p_data->pushmsg_msg.status, p_data->pushmsg_msg.session_handle);
        app_mce_push_msg_close(p_data->pushmsg_msg.session_handle);
        break;
    case BSA_MCE_MSG_PROG_EVT:
        APP_INFO1("BSA_MCE_MSG_PROG_EVT sess_id %d, read_size = %d, obj_size = %d",
            p_data->prog.session_handle, p_data->prog.read_size, p_data->prog.obj_size);
        break;
    case BSA_MCE_OBEX_PUT_RSP_EVT:
        APP_INFO1("BSA_MCE_OBEX_PUT_RSP_EVT sess_id %d, inst_id %d, rsp %d",
            p_data->ma_put_rsp.session_handle, p_data->ma_put_rsp.instance_id,
            p_data->ma_put_rsp.rsp_code);
        break;
    case BSA_MCE_OBEX_GET_RSP_EVT:
        APP_INFO1("BSA_MCE_OBEX_GET_RSP_EVT sess_id %d, inst_id %d, rsp %d",
            p_data->ma_get_rsp.session_handle, p_data->ma_get_rsp.instance_id,
            p_data->ma_get_rsp.rsp_code);
        break;
    case BSA_MCE_DISABLE_EVT:
        APP_INFO0("BSA_MCE_DISABLE_EVT");
        break;
    case BSA_MCE_INVALID_EVT:
        APP_INFO0("BSA_MCE_INVALID_EVT");
        break;
    case BSA_MCE_ABORT_EVT:
        APP_INFO0("BSA_MCE_ABORT_EVT");
        app_mce_abort_evt(p_data);
        break;
    case BSA_MCE_GET_MAS_INSTANCES_EVT:
        APP_INFO0("BSA_MCE_GET_MAS_INSTANCES_EVT");
        app_mce_get_mas_instances_evt(p_data);
        break;
    case BSA_MCE_GET_MAS_INS_INFO_EVT:
        APP_INFO1("BSA_MCE_GET_MAS_INS_INFO_EVT status %d sess_id=%d  inst_id=%d",
            p_data->get_mas_ins_info.status, p_data->get_mas_ins_info.session_handle,
            p_data->get_mas_ins_info.instance_id);
        if (p_data->get_mas_ins_info.status == BSA_SUCCESS)
        {
            APP_INFO0("BSA_MCE_GET_MAS_INS_INFO_EVT - Supported");
        }
        else
        {
            APP_INFO0("BSA_MCE_GET_MAS_INS_INFO_EVT - Failure");
        }
        break;
    default:
        APP_ERROR1("app_mce_cback unknown event:%d", event);
        break;
    }

    /* forward the callback to registered applications */
    if (s_pMceCallback)
        s_pMceCallback(event, p_data);
}


/*******************************************************************************
**
** Function         app_mce_enable
**
** Description      Example of function to Enable MCE Client application
**
** Parameters       Callback for event notification (can be NULL, if NULL default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_mce_enable(tMceCallback pcb)
{
    tBSA_STATUS status = BSA_SUCCESS;
    UINT8 i;

    tBSA_MCE_ENABLE enable_param;
    /* register callback */
    s_pMceCallback = pcb;

    APP_INFO0("app_start_mce");

    /* Initialize the control structure */
    memset(&app_mce_cb, 0, sizeof(app_mce_cb));
    for (i = 0; i < APP_MCE_MAX_CONN; i++)
    {
        app_mce_cb.conn[i].uipc_mce_rx_channel = UIPC_CH_ID_BAD;
        app_mce_cb.conn[i].uipc_mce_tx_channel = UIPC_CH_ID_BAD;
    }

    status = BSA_MceEnableInit(&enable_param);

    enable_param.p_cback = app_mce_cback;

    status = BSA_MceEnable(&enable_param);
    APP_INFO1("app_start_mce Status: %d", status);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_start_mce: Unable to enable MCE status:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_disable
**
** Description      Example of function to stop and disable MCE client module
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_disable(void)
{
    tBSA_MCE_DISABLE disable_param;
    tBSA_STATUS status;
    UINT8 i;

    APP_INFO0("app_stop_mce");

    for (i = 0; i < APP_MCE_MAX_CONN; i++)
    {
        if (app_mce_cb.conn[i].is_channel_open)
        {
            /* close the UIPC channel receiving data */
            UIPC_Close(app_mce_cb.conn[i].uipc_mce_rx_channel);
            UIPC_Close(app_mce_cb.conn[i].uipc_mce_tx_channel);
            app_mce_cb.conn[i].uipc_mce_rx_channel = UIPC_CH_ID_BAD;
            app_mce_cb.conn[i].uipc_mce_tx_channel = UIPC_CH_ID_BAD;
            app_mce_cb.conn[i].is_channel_open = FALSE;
        }
    }

    status = BSA_MceDisableInit(&disable_param);
    status = BSA_MceDisable(&disable_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_stop_mce: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_open
**
** Description      Example of function to open up MCE connection to peer device
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_open(BD_ADDR bd_addr, UINT8 instance_id)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;
    tBSA_STATUS status = BSA_SUCCESS;
    tBSA_MCE_OPEN param;

    APP_INFO0("app_mce_open");

    if ((p_conn = app_mce_get_available_conn(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_available_inst(p_conn, bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    bdcpy(p_conn->peer_addr, bd_addr);
    p_inst->instance_id = instance_id;
    p_inst->state = APP_MCE_CONN_ST_OPEN_PENDING;

    BSA_MceOpenInit(&param);
    bdcpy(param.bd_addr, bd_addr);

    param.instance_id = instance_id;
    param.sec_mask = DEFAULT_SEC_MASK;

    status = BSA_MceOpen(&param);

    if (status != BSA_SUCCESS)
    {
        p_inst->state = APP_MCE_CONN_ST_IDLE;
        APP_ERROR1("app_mce_open: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_close
**
** Description      Example of function to disconnect current connection
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_close(BD_ADDR bd_addr, UINT8 instance_id)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;
    tBSA_MCE_CLOSE close_param;
    tBSA_STATUS status;

    APP_INFO0("app_mce_close");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceCloseInit(&close_param);

    close_param.session_handle = p_inst->session_handle;
    bdcpy(close_param.bd_addr, bd_addr);

    status = BSA_MceClose(&close_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_close: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_close_all
**
** Description      Example of function to disconnect all connections
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_close_all()
{
    UINT8 i, j;
    tBSA_STATUS status = BSA_SUCCESS;

    APP_INFO0("app_mce_close_all");

    for (i = 0; i < APP_MCE_MAX_CONN; i++)
    {
        if (app_mce_cb.conn[i].is_open == FALSE)
            continue;

        for (j = 0; j < APP_MCE_MAX_INST; j++)
        {
            if (app_mce_cb.conn[i].inst[j].state != APP_MCE_CONN_ST_IDLE)
            {
                status = app_mce_close(app_mce_cb.conn[i].peer_addr, app_mce_cb.conn[i].inst[j].instance_id);
                if (status != BSA_SUCCESS)
                {
                    return status;
                }
            }
        }
    }

    return status;
}

/*******************************************************************************
**
** Function         app_mce_cancel
**
** Description      cancel connections
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_cancel(BD_ADDR bd_addr, UINT8 instance_id)
{
    tBSA_STATUS status;
    tBSA_MCE_CANCEL cancel_param;

    APP_INFO0("app_mce_cancel");

    status = BSA_MceCancelInit(&cancel_param);

    cancel_param.instance_id = instance_id;
    bdcpy(cancel_param.bd_addr, bd_addr);

    status = BSA_MceCancel(&cancel_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_cancel: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_abort
**
** Description      Example of function to abort current operation
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_abort(BD_ADDR bd_addr, UINT8 instance_id)
{
    tBSA_STATUS status;
    tBSA_MCE_ABORT abort_param;

    APP_INFO0("app_mce_abort");

    status = BSA_MceAbortInit(&abort_param);

    bdcpy(abort_param.bd_addr, bd_addr);
    abort_param.instance_id = instance_id;

    status = BSA_MceAbort(&abort_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_abort: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_get_folder_list
**
** Description      Get Folder listing at the current level on existing connection to MAS
**
** Parameters       bd_addr        - BD address of the device
**                  instance_id    - MAS instance ID
**                  max_list_count - maximum number of foldr-list objects allowed
**                  start_offset   - offset of the from the first entry of the folder-list
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_get_folder_list(BD_ADDR bd_addr, UINT8 instance_id, UINT16 max_list_count, UINT16 start_offset)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;
    tBSA_STATUS status = 0;
    tBSA_MCE_GET mceGet;

    APP_INFO0("app_mce_get_folder_list");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceGetInit(&mceGet);

    mceGet.type = BSA_MCE_GET_FOLDER_LIST;
    bdcpy(mceGet.bd_addr, bd_addr);

    mceGet.param.folderlist.session_handle = p_inst->session_handle;
    mceGet.param.folderlist.max_list_count = max_list_count;
    mceGet.param.folderlist.start_offset = start_offset;

    status = BSA_MceGet(&mceGet);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_get_folder_list: Error:%d", status);
    }

    return status;
}

/*******************************************************************************
**
** Function         bsa_mce_convert_hex_str_to_64bit_handle
**
** Description      Convert a hex string to a 64 bit message handle in Big Endian
**                  format
**
** Returns          void
**
*******************************************************************************/
void bsa_mce_convert_hex_str_to_64bit_handle(const char *p_hex_str, tBSA_MCE_MSG_HANDLE handle)
{
    UINT32 ul1, ul2;
    UINT8  *p;
    UINT8   str_len;
    char   tmp[BSA_MA_32BIT_HEX_STR_SIZE];

    APP_INFO0("bsa_mce_convert_hex_str_to_64bit_handle");

    str_len = strlen(p_hex_str);
    memset(handle,0,sizeof(tBSA_MCE_MSG_HANDLE));

    if (str_len >= 8)
    {
        /* most significant 4 bytes */
        memcpy(tmp,p_hex_str,(str_len-8));
        tmp[str_len-8]='\0';
        ul1 = strtoul(tmp,0,16);
        p=handle;
        UINT32_TO_BE_STREAM(p, ul1);

        /* least significant 4 bytes */
        memcpy(tmp,&(p_hex_str[str_len-8]),8);
        tmp[8]='\0';
        ul2 = strtoul(tmp,0,16);
        p=&handle[4];
        UINT32_TO_BE_STREAM(p, ul2);
    }
    else
    {
        /* least significant 4 bytes */
        ul1 = strtoul(p_hex_str,0,16);
        p=&handle[4];
        UINT32_TO_BE_STREAM(p, ul1);
    }
}

/*******************************************************************************
**
** Function         app_mce_get_msg
**
** Description      Get the message for specified handle from MAS server
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**                  phandlestr  - message handle string
**                  attachment  - if true attachments to be delivered,
**                                else no attachments
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_get_msg(BD_ADDR bd_addr, UINT8 instance_id, const char* phandlestr, BOOLEAN attachment)
{
    tAPP_MCE_CONN       *p_conn;
    tAPP_MCE_INST       *p_inst;
    tBSA_MCE_GET        mceGet;
    tBSA_MCE_CHARSET    charset = BSA_MCE_CHARSET_UTF_8;
    tBSA_MCE_FRAC_REQ   frac_req = BSA_MCE_FRAC_REQ_NO;
    tBSA_MCE_MSG_HANDLE handle = {0};
    tBSA_STATUS         status = 0;

    APP_INFO0("app_mce_get_msg");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    bsa_mce_convert_hex_str_to_64bit_handle(phandlestr, handle);

    p_conn->remove = TRUE;

    status = BSA_MceGetInit(&mceGet);
    mceGet.type = BSA_MCE_GET_MSG;
    bdcpy(mceGet.bd_addr, bd_addr);
    mceGet.param.msg.session_handle = p_inst->session_handle;
    mceGet.param.msg.msg_param.attachment = attachment;
    mceGet.param.msg.msg_param.charset = charset;
    mceGet.param.msg.msg_param.fraction_request = frac_req;
    memcpy(mceGet.param.msg.msg_param.handle, handle, sizeof(tBSA_MCE_MSG_HANDLE));

    status = BSA_MceGet(&mceGet);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_get_msg: Error:%d", status);
    }

    return status;
}

/*******************************************************************************
**
** Function         app_mce_get_msg_list
**
** Description      Get message listing from the specified MAS server
**
** Parameters       bd_addr        - BD address of the device
**                  instance_id    - MAS instance ID
**                  pfolder        - folder name string
**                  p_filter_param - filter parameters for getting the object
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_get_msg_list(BD_ADDR bd_addr, UINT8 instance_id, const char* pfolder,
        tBSA_MCE_MSG_LIST_FILTER_PARAM* p_filter_param)
{
    tAPP_MCE_CONN   *p_conn;
    tAPP_MCE_INST   *p_inst;
    tBSA_STATUS     status = 0;
    tBSA_MCE_GET    mceGet;

    APP_INFO0("app_mce_get_msg_list");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceGetInit(&mceGet);

    mceGet.type = BSA_MCE_GET_MSG_LIST;
    bdcpy(mceGet.bd_addr, bd_addr);

    strncpy(mceGet.param.msglist.folder, pfolder, sizeof(mceGet.param.msglist.folder)-1);
    mceGet.param.msglist.folder[sizeof(mceGet.param.msglist.folder)-1] = 0;
    mceGet.param.msglist.session_handle = p_inst->session_handle;

    if (p_filter_param)
    {
        memcpy(&mceGet.param.msglist.filter_param, p_filter_param,
                sizeof(tBSA_MCE_MSG_LIST_FILTER_PARAM));
    }

    status = BSA_MceGet(&mceGet);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_get_msg_list: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_get_mas_ins_info
**
** Description      Get MAS info for the specified MAS instance id
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_get_mas_ins_info(BD_ADDR bd_addr, UINT8 instance_id)
{
    tAPP_MCE_CONN   *p_conn;
    tAPP_MCE_INST   *p_inst;
    tBSA_MCE_GET    mceGet;
    tBSA_STATUS     status = 0;

    APP_INFO0("app_mce_get_mas_ins_info");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceGetInit(&mceGet);

    mceGet.type = BSA_MCE_GET_MAS_INST_INFO;
    bdcpy(mceGet.bd_addr, bd_addr);
    mceGet.param.mas_info.session_handle = p_inst->session_handle;
    mceGet.param.mas_info.instance_id = instance_id;

    status = BSA_MceGet(&mceGet);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_get_mas_ins_info: Error:%d", status);
    }

    return status;
}

/*******************************************************************************
**
** Function         app_mce_get_mas_instances
**
** Description      Get MAS instances for the specified BD address
**
** Parameters       bd_addr - BD address of the device
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_get_mas_instances(BD_ADDR bd_addr)
{
    tBSA_STATUS status = 0;
    tBSA_MCE_GET mceGet;

    APP_INFO0("app_mce_get_mas_instances");

    status = BSA_MceGetInit(&mceGet);
    mceGet.type = BSA_MCE_GET_MAS_INSTANCES;
    bdcpy(mceGet.param.mas_instances.bd_addr, bd_addr);
    status = BSA_MceGet(&mceGet);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_get_mas_instances: Error:%d", status);
    }

    return status;
}

/*******************************************************************************
**
** Function         app_mce_mn_start
**
** Description      Example of function to start notification service
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_mn_start(void)
{
    tBSA_STATUS status;
    tBSA_MCE_MN_START mn_start_param;
    char svc_name[BSA_MCE_SERVICE_NAME_LEN_MAX] = "MAP Notification Service";

    APP_INFO0("app_mce_mn_start");

    status = BSA_MceMnStartInit(&mn_start_param);

    strcpy(mn_start_param.service_name, svc_name);

    mn_start_param.sec_mask = DEFAULT_SEC_MASK;

    status = BSA_MceMnStart(&mn_start_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_mn_start: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_mn_stop
**
** Description      Example of function to stop notification service
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_mn_stop(void)
{
    tBSA_STATUS status;

    tBSA_MCE_MN_STOP mn_stop_param;

    APP_INFO0("app_mce_mn_stop");

    if (!app_mce_cb.is_mns_started)
        return BSA_ERROR_SRV_BAD_PARAM;

    status = BSA_MceMnStopInit(&mn_stop_param);

    status = BSA_MceMnStop(&mn_stop_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_mn_stop: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_mn_close
**
** Description      Example of function to disconnect current MNS connection
**
** Parameters       bd_addr - BD address of the device
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_mn_close(BD_ADDR bd_addr)
{
    tAPP_MCE_CONN       *p_conn;
    tBSA_MCE_MN_CLOSE   close_param;
    tBSA_STATUS         status;

    APP_INFO0("app_mce_mn_close");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceMnCloseInit(&close_param);

    close_param.session_handle = p_conn->mn_session_handle;
    bdcpy(close_param.bd_addr, bd_addr);

    status = BSA_MceMnClose(&close_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_mn_close: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_notif_reg
**
** Description      Set the Message Notification status to On or OFF on the MSE.
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**                  bReg        - TRUE if notification required
**                                FALSE if no notification
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_notif_reg(BD_ADDR bd_addr, UINT8 instance_id, BOOLEAN bReg)
{
    tAPP_MCE_CONN       *p_conn;
    tAPP_MCE_INST       *p_inst;
    tBSA_MCE_NOTIFYREG  notif_reg_param;
    tBSA_STATUS         status;

    APP_INFO0("app_mce_notif_reg");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceNotifRegInit(&notif_reg_param);

    notif_reg_param.session_handle = p_inst->session_handle;

    notif_reg_param.status = bReg ? BSA_MCE_NOTIF_ON : BSA_MCE_NOTIF_OFF;

    status = BSA_MceNotifReg(&notif_reg_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_notif_reg: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_set_filter
**
** Description      Example of function to perform a set filter operation
**
** Parameters       filter - message filer parameter
**
** Returns          0 if success -1 if failure
**
*******************************************************************************/
tBSA_STATUS app_mce_set_filter(UINT8 filter)
{
    tBSA_STATUS status = 0;
    /*     TODO use a global FILTER PARAM AND
    Get values and store them
    TO BE USED IN SUBSEQUENT API CALLS WHERE NECESSARY/
    */
    return status;
}

/*******************************************************************************
**
** Function         app_mce_set_msg_sts
**
** Description      Example of function to perform a set message status operation
**
** Parameters       bd_addr          - BD address of the device
**                  instance_id      - MAS instance ID
**                  phandlestr       - message handle string
**                  status_indicator - read/delete message
**                  status_value     - TRUE On, FALSE Off
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_set_msg_sts(BD_ADDR bd_addr, UINT8 instance_id, const char* phandlestr, UINT8 status_indicator,
        UINT8 status_value)
{
    tAPP_MCE_CONN       *p_conn;
    tAPP_MCE_INST       *p_inst;
    tBSA_MCE_SET        set_param;
    tBSA_MCE_MSG_HANDLE handle = {0};
    tBSA_STATUS         status = 0;

    APP_INFO0("app_mce_set_msg_sts");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceSetInit(&set_param);

    if (!phandlestr || !strlen(phandlestr))
    {
        APP_ERROR0("app_mce_set_msg_sts invalid handle");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    bsa_mce_convert_hex_str_to_64bit_handle(phandlestr,  handle);
    memcpy(set_param.param.msg_status.msg_handle, handle, sizeof(tBSA_MCE_MSG_HANDLE));

    set_param.param.msg_status.session_handle = p_inst->session_handle;
    set_param.param.msg_status.status_indicator = status_indicator;
    set_param.param.msg_status.status_value = status_value;

    set_param.type = BSA_MCE_SET_MSG_STATUS;
    bdcpy(set_param.bd_addr, bd_addr);

    status = BSA_MceSet(&set_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_set_msg_sts: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_set_folder
**
** Description      Example of function to perform a set folder operation
**
** Parameters       bd_addr          - BD address of the device
**                  instance_id      - MAS instance ID
**                  p_string_dir     - folder name string
**                  direction_flag   - 0 root or down one level
**                                     1 up one level
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_set_folder(BD_ADDR bd_addr, UINT8 instance_id, const char* p_string_dir, UINT8 direction_flag)
{
    tAPP_MCE_CONN   *p_conn;
    tAPP_MCE_INST   *p_inst;
    tBSA_MCE_SET    set_param;
    tBSA_STATUS     status = 0;

    APP_INFO0("app_mce_set_folder");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    if (!p_string_dir)
    {
        APP_ERROR0("app_mce_set_folder invalid folder");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    status = BSA_MceSetInit(&set_param);

    set_param.param.folder.direction_flag = direction_flag;
    set_param.type = BSA_MCE_SET_FOLDER;
    bdcpy(set_param.bd_addr, bd_addr);
    set_param.param.folder.session_handle = p_inst->session_handle;
    strncpy(set_param.param.folder.folder, p_string_dir, sizeof(set_param.param.folder.folder)-1);
    set_param.param.folder.folder[sizeof(set_param.param.folder.folder)-1] = 0;

    status = BSA_MceSet(&set_param);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_set_folder: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_upd_inbox
**
** Description      Example of function to send update inbox request to MAS
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_upd_inbox(BD_ADDR bd_addr, UINT8 instance_id)
{
    tAPP_MCE_CONN           *p_conn;
    tAPP_MCE_INST           *p_inst;
    tBSA_MCE_UPDATEINBOX    upd_inbox;
    tBSA_STATUS             status;

    APP_INFO0("app_mce_upd_inbox");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    status = BSA_MceUpdateInboxInit(&upd_inbox);

    upd_inbox.session_handle = p_inst->session_handle;

    status = BSA_MceUpdateInbox(&upd_inbox);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_upd_inbox: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         app_mce_push_msg
**
** Description      Example of function to push a message to MAS
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**                  folder      - folder name string
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_push_msg(BD_ADDR bd_addr, UINT8 instance_id, const char* folder)
{
    tAPP_MCE_CONN           *p_conn;
    tAPP_MCE_INST           *p_inst;
    tBSA_MCE_PUSHMSG        push_msg;
    tBSA_STATUS             status;
    tBSA_MCE_TRANSP_TYPE    trans = BSA_MCE_TRANSP_OFF;
    tBSA_MCE_RETRY_TYPE     retry = BSA_MCE_RETRY_OFF;
    tBSA_MCE_CHARSET        charset = BSA_MCE_CHARSET_UTF_8;

    APP_INFO0("app_mce_push_msg");

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return BSA_ERROR_CLI_BAD_PARAM;

    /* Open the file we just wrote out and be ready to send data to stack
       when it requests it. Data is sent over UIPC data channel */

    app_mce_open_file(p_conn);

    status = BSA_McePushMsgInit(&push_msg);

    push_msg.session_handle = p_inst->session_handle;

    push_msg.msg_param.charset = charset;
    push_msg.msg_param.retry = retry;
    push_msg.msg_param.transparent = trans;

    strncpy(push_msg.msg_param.folder, folder, sizeof(push_msg.msg_param.folder)-1);
    push_msg.msg_param.folder[sizeof(push_msg.msg_param.folder)-1] = 0;

    status = BSA_McePushMsg(&push_msg);

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_mce_push_msg: Error:%d", status);
    }
    return status;
}

/*******************************************************************************
**
** Function         mce_is_open_pending
**
** Description      Check if mce open is pending
**
** Parameters       bd_addr     - BD address of the device
**                  instance_id - MAS instance ID
**
** Returns          TRUE if open is pending, FALSE otherwise
**
*******************************************************************************/
BOOLEAN mce_is_open_pending(BD_ADDR bd_addr, UINT8 instance_id)
{
    tAPP_MCE_CONN *p_conn;
    tAPP_MCE_INST *p_inst;

    if ((p_conn = app_mce_get_conn_by_bd_addr(bd_addr)) == NULL)
        return FALSE;
    if ((p_inst = app_mce_get_inst_by_inst_id(p_conn, instance_id)) == NULL)
        return FALSE;

    return p_inst->state == APP_MCE_CONN_ST_OPEN_PENDING;
}

/*******************************************************************************
**
** Function         mce_is_mns_started
**
** Description      Is MNS service started
**
** Parameters
**
** Returns          true if mns service has been started
**
*******************************************************************************/
tBSA_STATUS mce_is_mns_started()
{
    return app_mce_cb.is_mns_started;
}

/*******************************************************************************
 **
 ** Function         app_mce_choose_connected_device
 **
 ** Description      Choose the connected device
 **
 ** Returns          tAPP_MCE_CONN *
 **
 *******************************************************************************/
tAPP_MCE_CONN *app_mce_choose_connected_device()
{
    tAPP_MCE_CONN *p_conn = NULL;
    UINT8 index, device_index, num_conn = 0;

    for (index = 0; index < APP_MCE_MAX_CONN; index++)
    {
        if (app_mce_cb.conn[index].is_open == TRUE)
        {
            num_conn++;
            APP_INFO1("Connection index %d:%02X:%02X:%02X:%02X:%02X:%02X",
                index,
                app_mce_cb.conn[index].peer_addr[0], app_mce_cb.conn[index].peer_addr[1],
                app_mce_cb.conn[index].peer_addr[2], app_mce_cb.conn[index].peer_addr[3],
                app_mce_cb.conn[index].peer_addr[4], app_mce_cb.conn[index].peer_addr[5]);
        }
    }

    if (num_conn)
    {
        APP_INFO0("Choose connection index");
        device_index = app_get_choice("\n");
        if ((device_index < APP_MCE_MAX_CONN) && (app_mce_cb.conn[device_index].is_open == TRUE))
            p_conn = &app_mce_cb.conn[device_index];
        else
            APP_INFO0("Unknown choice");
    }
    else
    {
        APP_INFO0("No connections");
    }

    return p_conn;
}

