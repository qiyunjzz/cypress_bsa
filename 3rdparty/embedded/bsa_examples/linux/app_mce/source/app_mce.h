/*****************************************************************************
**
**  Name:           app_mce.h
**
**  Description:    Bluetooth Message Access Profile client application
**
**  Copyright (c)   2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
**  Copyright (C) 2017 Cypress Semiconductor Corporation
**
*****************************************************************************/

#ifndef APP_MCE_H_
#define APP_MCE_H_

/* Callback to application for connection events */
typedef void (tMceCallback)(tBSA_MCE_EVT event, tBSA_MCE_MSG *p_data);

#define APP_MCE_MAX_CONN                2
#define APP_MCE_MAX_INST                4

#define APP_MCE_CONN_ST_IDLE            0
#define APP_MCE_CONN_ST_OPEN_PENDING    1
#define APP_MCE_CONN_ST_CONNECTED       2

/*
* Types
*/
typedef struct
{
    int                     state;
    tBSA_MCE_INST_ID        instance_id;
    tBSA_MCE_SESS_HANDLE    session_handle;
} tAPP_MCE_INST;

typedef struct
{
    tAPP_MCE_INST   inst[APP_MCE_MAX_INST];
    int             fd;
    BOOLEAN         is_open;
    BOOLEAN         remove;
    BD_ADDR         peer_addr;
    BOOLEAN         is_channel_open;
    tUIPC_CH_ID     uipc_mce_rx_channel;
    tUIPC_CH_ID     uipc_mce_tx_channel;
    BOOLEAN         is_mn_open;
    tBSA_MCE_SESS_HANDLE mn_session_handle;
} tAPP_MCE_CONN;

typedef struct
{
    tAPP_MCE_CONN   conn[APP_MCE_MAX_CONN];
    BOOLEAN         is_mns_started;
    BOOLEAN         discover_pending;
} tAPP_MCE_CB;

/*******************************************************************************
**
** Function         app_mce_enable
**
** Description      Example of function to enable MCE functionality
**
** Parameters       Callback for event notification (can be NULL, if NULL default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_mce_enable(tMceCallback pcb);

/*******************************************************************************
**
** Function         app_mce_disable
**
** Description      Example of function to stop MCE functionality
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_disable(void);

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
tBSA_STATUS app_mce_abort(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS app_mce_open(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS app_mce_close(BD_ADDR bd_addr, UINT8 instance_id);

/*******************************************************************************
**
** Function         app_mce_close_all
**
** Description      Function to close all MCE connections
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_close_all();

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
tBSA_STATUS app_mce_cancel(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS app_mce_get_mas_instances(BD_ADDR bd_addr);

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
tBSA_STATUS app_mce_get_mas_ins_info(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS app_mce_get_msg(BD_ADDR bd_addr, UINT8 instance_id, const char* phandlestr, BOOLEAN attachment);

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
tBSA_STATUS app_mce_push_msg(BD_ADDR bd_addr, UINT8 instance_id, const char* folder);

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
         tBSA_MCE_MSG_LIST_FILTER_PARAM* p_filter_param);

/*******************************************************************************
**
** Function         app_mce_mn_start
**
** Description      Get MAS info for the specified MAS instance id
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_mn_start(void);

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
tBSA_STATUS app_mce_mn_stop(void);

/*******************************************************************************
**
** Function         app_mce_mn_close
**
** Description      Example of function to disconnect current MN connection
**
** Parameters
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
tBSA_STATUS app_mce_mn_close(BD_ADDR bd_addr);

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
tBSA_STATUS app_mce_notif_reg(BD_ADDR bd_addr, UINT8 instance_id, BOOLEAN bReg);

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
tBSA_STATUS app_mce_upd_inbox(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS app_mce_get_folder_list(BD_ADDR bd_addr, UINT8 instance_id, UINT16 max_list_count, UINT16 start_offset);

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
tBSA_STATUS app_mce_set_folder(BD_ADDR bd_addr, UINT8 instance_id, const char* p_string_dir, UINT8 direction_flag);

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
        UINT8 status_value);

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
BOOLEAN mce_is_open_pending(BD_ADDR bd_addr, UINT8 instance_id);

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
tBSA_STATUS mce_is_mns_started();

/*******************************************************************************
 **
 ** Function         app_mce_choose_connected_device
 **
 ** Description      Choose the connected device
 **
 ** Returns          tAPP_MCE_CONN *
 **
 *******************************************************************************/
tAPP_MCE_CONN *app_mce_choose_connected_device();

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
tAPP_MCE_CONN *app_mce_get_conn_by_bd_addr(BD_ADDR bd_addr);

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
tAPP_MCE_INST *app_mce_get_inst_by_inst_id(tAPP_MCE_CONN *p_conn, tBSA_MCE_INST_ID instance_id);

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
tAPP_MCE_INST *app_mce_get_inst_by_sess_hdl(tAPP_MCE_CONN *p_conn, tBSA_MCE_SESS_HANDLE session_handle);

/*******************************************************************************
**
** Function         app_mce_get_conn_by_sess_hdl
**
** Description      Get the connection control block by session handle
**
** Parameters       session_handle - MAS session handle
**
** Returns          tAPP_MCE_INST *
**
*******************************************************************************/
tAPP_MCE_CONN *app_mce_get_conn_by_sess_hdl(tBSA_MCE_SESS_HANDLE session_handle);

#endif /* APP_MCE_H_ */
