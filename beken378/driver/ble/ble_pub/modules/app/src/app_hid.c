/**
 ****************************************************************************************
 *
 * @file app_hid.c
 *
 * @brief HID Application Module entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"            // SW configuration

#include <stdio.h>
#include <string.h>

#if (BLE_APP_HID)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "application.h"                       // Application Definitions
#include "app_sec.h"                // Application Security Module API
#include "app_task.h"              // Application task definitions
#include "app_hid.h"                // HID Application Module Definitions
#include "hogpd_task.h"          // HID Over GATT Profile Device Role Functions
#include "prf_types.h"             // Profile common types Definition
#include "architect.h"                     // Platform Definitions
#include "hogpd.h"
#include "prf.h"
#include "kernel_timer.h"
#if (NVDS_SUPPORT)
#include "nvds.h"                   // NVDS Definitions
#endif //(NVDS_SUPPORT)

#if (DISPLAY_SUPPORT)
#include "app_display.h"         // Application Display Module
#endif //(DISPLAY_SUPPORT)

#include "common_utils.h"               // Common functions

#if (KERNEL_PROFILING)
#include "kernel_mem.h"
#endif //(KERNEL_PROFILING)

#include "prf_utils.h"
#include "uart.h"
/*
 * DEFINES
 ****************************************************************************************
 */


#define APP_HID_HID_REPORT_MAX_LEN       (8)
	
#define APP_HID_BK_REPORT_MAP_LEN      (sizeof(gHIDReportDescriptor)) 

/// Duration before connection update procedure if no report received (mouse is silent) - 20s
#define APP_HID_SILENCE_DURATION_1     (2000)
/// Duration before disconnection if no report is received after connection update - 60s
#define APP_HID_SILENCE_DURATION_2     (6000)

/// Number of reports that can be sent
#define APP_HID_NB_SEND_REPORT         (10)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of the Application HID Module
enum app_hid_states
{
    /// Module is disabled (Service not added in DB)
    APP_HID_DISABLED,
    /// Module is idle (Service added but profile not enabled)
    APP_HID_IDLE,
    /// Module is enabled (Device is connected and the profile is enabled)
    APP_HID_ENABLED,
    /// The application can send reports
    APP_HID_READY,
    /// Waiting for a report
    APP_HID_WAIT_REP,

    APP_HID_STATE_MAX,
};

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// HID Application Module Environment Structure
struct app_hid_env_tag app_hid_env;

/*
static const uint8_t hid_report_map_array[] =
{
    0x05, 0x01,    // Usage page(Generic desktop)
    0x09, 0x06,    // Usage(Keyboard)
    0xA1, 0x01,
    0x85, 0x01,    // Report ID(0x01)
    0x05, 0x07,    // Usage page (Keyboard)
    0x19, 0xE0,    // Modify key
    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,    // Report Size(1)
    0x95, 0x08,    // Reprt count(8)
    0x81, 0x02,    // Input Report()    
    0x95, 0x01,    // Reprt count(1)
    0x75, 0x08,    // Report Size(8)
    0x81, 0x01,    // Input Report(reserved)
    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,     // Usage page (LEDs)
    0x19, 0x01,
    0x29, 0x05,    
    0x91, 0x02,    // output report 
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,     // output report
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,    // Input Report
    
    0x09, 0x05,    // Usage(Game Pad)
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 0x02,
    0xB1, 0x02,    // Feature report
    0xC0,
    
    0x05, 0x0C,    // Usage page (Consumer page)
    0x09, 0x01,
    0xA1, 0x01,
    0x85, 0x02,    // Report ID(0x02)
    0x75, 0x10,
    0x95, 0x01,
    0x15, 0x00,
    0x26, 0xFF, 0x03,
    0x19, 0x00,
    0x2A, 0xFF, 0x03,
    0x81, 0x00,
    0xC0,
    
    0x05, 0x01, 	// Usage Page (Generic Desktop),
    0x09, 0x80,	 	// Usage (System control)
    0xa1, 0x01,	 	// Collection (Application),
    0x85, 0x03, 	// Report ID (3)
    0x05, 0x01, 	// Usage Page (Generic Desktop),
    0x19, 0x81, 	// Usage Minimum (), 
    0x29, 0x83, 	// Usage Maximum (),
    0x15, 0x00, 	// Logical Minimum (0),
    0x25, 0x01, 	// Logical Maximum (1),
    0x95, 0x03, 	// Report Count (3),
    0x75, 0x01, 	// Report Size (1),
    0x81, 0x02,     // Input (Data,Var,Abs) 0x06, Input (Data,Var,Rel)
    0x95, 0x01, 	// Report Count (1),  
    0x75, 0x05, 	// Report Size (5),	
    0x81, 0x01,		// Input (Const,Ary,Abs)
    0xc0,			// END_COLLECTION
};
*/
#if 1
#define HIDS_KB_REPORT_ID       	1
#define HIDS_MOUSE_REPORT_ID    	5
#define RMC_VENDOR_REPORT_ID_1  	0xfd
#define RMC_VENDOR_REPORT_ID_2   	0x1e
#define HIDS_MM_KB_REPORT_ID     	3
#define RMC_SENSORS_DATA_REPORT_ID  0x32 


#define RMC_WITH_VOICE				1
#define MULTIMEDIA_KEYBOARD			1
#define RMC_WITH_SENSORS_DATA		1

const uint8_t gHIDReportDescriptor[] =
{
    0x05, 0x01,
    0x09, 0x06,
    0xa1, 0x01,
    0x85, HIDS_KB_REPORT_ID,
    0x05, 0x07,
    0x19, 0xe0,
    0x29, 0xe7,
    0x15, 0x00 ,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,

    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0xff,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0xff,
    0x81, 0x00,
    0xc0,

    0x05, 0x01,  /// USAGE PAGE (Generic Desktop) 定位到Generic Desktop页，这个相当于指针跳转一样的东西
    0x09, 0x02,  /// USAGE (Mouse) 表示这是一个鼠标
    0xa1, 0x01,  /// COLLECTION (Application) 是对Mouse的解释
    0x85, HIDS_MOUSE_REPORT_ID, /// REPORT ID (5) 
    0x09, 0x01,  /// USAGE (Pointer) 表示指针形式
    0xa1, 0x00,  /// COLLECTION (Physical)  是对Pointer的解释
    /**
     * ----------------------------------------------------------------------------
     * BUTTONS
     * ----------------------------------------------------------------------------
     */
    0x05, 0x09,  /// USAGE PAGE (Buttons)
    0x19, 0x01,  /// Usage Minimum (01) -Button 1
    0x29, 0x03,  /// Usage Maximum (03) -Button 3
    0x15, 0x00,  /// Logical Minimum (0)
    0x25, 0x01,  /// Logical Maximum (1)
    0x95, 0x03,  /// Report Count (3)
    0x75, 0x01,  /// Report Size (1)
    0x81, 0x02,  /// Input (Data, Variable,Absolute) - Button states
    0x95, 0x01,  /// Report Count (1)
    0x75, 0x05,  /// Report Size (5)
    0x81, 0x01,  /// Input (Constant) - Paddingor Reserved bits
    /**
     * ----------------------------------------------------------------------------
     * MOVEMENT DATA
     * ----------------------------------------------------------------------------
     */
    0x05, 0x01,  /// USAGE PAGE (Generic Desktop)
    0x09, 0x30,  /// USAGE (X)
    0x09, 0x31,  /// USAGE (Y)
    0x09, 0x38,  /// USAGE (Wheel)
    0x15, 0x81,  /// LOGICAL MINIMUM (-127)
    0x25, 0x7f,  /// LOGICAL MAXIMUM (127)
    0x75, 0x08,  /// REPORT SIZE (8)
    0x95, 0x03,  /// REPORT COUNT (3)
    0x81, 0x06,  /// INPUT 
    0xc0,
    0xc0,

#if RMC_WITH_VOICE

    0x06, 0x12, 0xff, 
    0x0a, 0x12, 0xff, 
    0xa1, 0x01, 		// Collection 
    0x85, RMC_VENDOR_REPORT_ID_1, // Report ID 
    0x09, 0x01, 		// Usage 
    0x75, 0x08, 		// Report Size (8),
    //0x95, 0xff, 		// Report Count (256), modify
    0x95, 0x14,			// Report Count (19),
    0x16, 0x00, 0x00, 	// Logical Minimum ,
    0x26, 0xff ,0x00, 	// Logical Maximum ,
    0x19, 0x00, 		// Usage Minimum (), 
    0x29, 0xff, 		// Usage Maximum (),
    0x81, 0x00, 		// Input  
    0xc0,				// END_COLLECTION

    0x06, 0x12, 0xff, 
    0x0a, 0x12, 0xff, 
    0xa1, 0x01, 		// Collection 
    0x85, RMC_VENDOR_REPORT_ID_2, // Report ID 
    0x09, 0x01, 		// Usage
    0x75, 0x08, 		// Report Size (8),
    //0x95, 0xff, 		// Report Count (256),
    0x95, 0x14,			// Report Count (19),
    0x16, 0x00, 0x00, 	// Logical Minimum ,
    0x26, 0xff, 0x00, 	// Logical Maximum ,
    0x19, 0x00, 		// Usage Minimum (), 
    0x29, 0xff, 		// Usage Maximum (),
    0x81, 0x00,		// Input 
    
    0x95, 0x08,		// Usage
    0x75, 0x01,		// Report Size (1),
    0x05, 0x08,		// Report Count (8),
    0x19, 0x01,		// Usage Minimum (), 
    0x29, 0x08,		// Usage Maximum (),
    0x91, 0x02,		// Output 
    0xc0,			// END_COLLECTION
#endif
 
#if MULTIMEDIA_KEYBOARD
     0x05, 0x0c,           // USAGE_PAGE (Consumer Devices) 
     0x09, 0x01,           // USAGE (Consumer Control) 
     0xa1, 0x01,           // COLLECTION (Application) 
     0x85, HIDS_MM_KB_REPORT_ID,  // REPORT_ID (3) 
     0x19, 0x00,          //USAGE_MINIMUM (0)
     0x2A, 0x9c, 0x02,    //USAGE_MAXIMUM (29c) 
     0x15, 0x00,          //LOGICAL_MINIMUM (0) 
     0x26, 0x9c, 0x02,    //LOGICAL_MAXIMUM (29c) 
     0x95, 0x01,          //REPORT_COUNT (1) 
     0x75, 0x10,          //REPORT_SIZE (16)
     0x81, 0x00,          //INPUT (Data,Ary,Abs) 
     0xc0,
#endif

#if RMC_WITH_SENSORS_DATA
	0x06, 0x00, 0xff, 
	0x09, 0x00,    // USAGE
	0xa1, 0x01,    // COLLECTION (Application) 
	0x85, RMC_SENSORS_DATA_REPORT_ID,  // REPORT_ID (0x32) 
	0x09, 0x00, 
	0x15, 0x80,   //LOGICAL_MINIMUM () 
	0x25, 0x7f,   //LOGICAL MAXIMUM ()
	0x75, 0x08,   // Report Size (8),
	0x95, 0x12,   // Report Count (18),
	0x81, 0x22,   // INPUT
	0xc0 
#endif	
};

#endif




/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_hid_init(void)
{
    // Reset the environment
    memset(&app_hid_env, 0, sizeof(app_hid_env));
	app_hid_env.state = APP_HID_IDLE;
}


void app_hid_add_hids(void)
{
	struct hogpd_db_cfg *db_cfg;
	// Prepare the HOGPD_CREATE_DB_REQ message
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
	                                               TASK_GAPM, TASK_APP,
	                                               gapm_profile_task_add_cmd, sizeof(struct hogpd_db_cfg));

	// Fill message
	req->operation   = GAPM_PROFILE_TASK_ADD;
	req->sec_lvl     = 0;
	req->prf_task_id = TASK_ID_HOGPD;
	req->app_task    = TASK_APP;
	req->start_hdl   = 0;

	// Set parameters
	db_cfg = (struct hogpd_db_cfg* ) req->param;

	// Only one HIDS instance is useful
	db_cfg->hids_nb = 1;

		
	/*****************************************************/
	// The device is a keyboard
	db_cfg->cfg[0].svc_features =  HOGPD_CFG_KEYBOARD | HOGPD_CFG_PROTO_MODE ;

	// Only one Report Characteristic is requested
	db_cfg->cfg[0].report_nb    = 8;
		
	db_cfg->cfg[0].report_id[0] = HIDS_KB_REPORT_ID;    //standard key

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[0] = HOGPD_CFG_REPORT_IN; 
		
	/*****************************************************/
	db_cfg->cfg[0].report_id[1] = HIDS_KB_REPORT_ID;

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[1] = HOGPD_CFG_REPORT_OUT;
		
	/*****************************************************/
	db_cfg->cfg[0].report_id[2] = HIDS_MOUSE_REPORT_ID;   //mouse

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[2] = HOGPD_CFG_REPORT_IN;

	/*****************************************************/
	db_cfg->cfg[0].report_id[3] = RMC_VENDOR_REPORT_ID_1;   //voice 1

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[3] = HOGPD_CFG_REPORT_IN;
	/*****************************************************/

	db_cfg->cfg[0].report_id[4] = RMC_VENDOR_REPORT_ID_2;   //voice 2

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[4] = HOGPD_CFG_REPORT_IN;

	/*****************************************************/
	db_cfg->cfg[0].report_id[5] = RMC_VENDOR_REPORT_ID_2;

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[5] = HOGPD_CFG_REPORT_OUT;
	/*****************************************************/

	db_cfg->cfg[0].report_id[6] = HIDS_MM_KB_REPORT_ID;  //media  key

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[6] = HOGPD_CFG_REPORT_IN;
	/*****************************************************/

	db_cfg->cfg[0].report_id[7] = RMC_SENSORS_DATA_REPORT_ID;  //sensor 

	// The report is an input report
	db_cfg->cfg[0].report_char_cfg[7] = HOGPD_CFG_REPORT_IN;
	/*****************************************************/

	// HID Information
	db_cfg->cfg[0].hid_info.bcdHID       = 0x0111;         // HID Version 1.11
	db_cfg->cfg[0].hid_info.bCountryCode = 0x00;
	db_cfg->cfg[0].hid_info.flags        = HIDS_REMOTE_WAKERNEL_CAPABLE | HIDS_NORM_CONNECTABLE;

	// Send the message
	kernel_msg_send(req);
}



/*
 ****************************************************************************************
 * @brief Function called when get connection complete event from the GAP
 *
 ****************************************************************************************
 */
void app_hid_enable_prf(uint8_t conidx)
{
    uint16_t ntf_cfg;

    // Store the connection handle
    app_hid_env.conidx = conidx;

    // Allocate the message
    struct hogpd_enable_req * req = KERNEL_MSG_ALLOC(HOGPD_ENABLE_REQ,
                                    prf_get_task_from_id(TASK_ID_HOGPD),
                                    TASK_APP,
                                    hogpd_enable_req);

    // Fill in the parameter structure
    req->conidx = conidx;
    // Notifications are enable.
    ntf_cfg  = 0xffff;

    // Go to Enabled state
    app_hid_env.state = APP_HID_READY;

    #if (NVDS_SUPPORT)
	// Length of the value read in NVDS
	uint8_t length   = NVDS_LEN_MOUSE_NTF_CFG;
	// Notification configuration
	if (nvds_get(NVDS_TAG_MOUSE_NTF_CFG, &length, (uint8_t *)&ntf_cfg) != NVDS_OK)
	{
		// If we are bonded this information should be present in the NVDS
		//ASSERT_ERR(0);
	}
	// CCC enable notification
   	if ((ntf_cfg & HOGPD_CFG_REPORT_NTF_EN ) != 0)
   	{
		// The device is ready to send reports to the peer device
        app_hid_env.state = APP_HID_READY;
	}
    #endif //(NVDS_SUPPORT)

    req->ntf_cfg[conidx] = ntf_cfg;

#if 0
    struct gapc_conn_param conn_param;
    
    conn_param.intv_min = BLE_UAPDATA_MIN_INTVALUE;
    conn_param.intv_max = BLE_UAPDATA_MAX_INTVALUE;
    conn_param.latency  = BLE_UAPDATA_LATENCY;
    conn_param.time_out = BLE_UAPDATA_TIMEOUT; 
    appm_update_param(&conn_param);
#endif

    // Send the message
    kernel_msg_send(req);

}


void app_hid_send_report(uint8_t *data, uint8_t len)
{
    //UART_PRINTF("%s, app_hid_state = 0x %x\r\n", __func__, app_hid_env.state);
	//app_hid_env.state = APP_HID_READY;
	switch (app_hid_env.state)
	{
	    case (APP_HID_READY):
	    {
	        // Check if the report can be sent
	 		struct hogpd_report_upd_req * req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
	                                                      prf_get_task_from_id(TASK_ID_HOGPD),
	                                                      TASK_APP,
	                                                      hogpd_report_upd_req,
	                                                      APP_HID_HID_REPORT_MAX_LEN);

			uint8_t report_buff[APP_HID_HID_REPORT_MAX_LEN];
			uint8_t status = 0;

			memset(&report_buff[0], 0, APP_HID_HID_REPORT_MAX_LEN);
		 
			if(len == APP_HID_KEYBOARD_REPORT_LEN)
			{
				status = 1;
			
			 	report_buff[0] = data[0];
			 	report_buff[1] = data[1];
			 	report_buff[2] = data[2];
			 	report_buff[3] = data[3];
			 	report_buff[4] = data[4];
			 	report_buff[5] = data[5];
			 	report_buff[6] = data[6];
				report_buff[7] = data[7];
					
	         	// Allocate the HOGPD_REPORT_UPD_REQ message
                req->conidx  = app_hid_env.conidx;
                //now fill report
                req->report.hid_idx  = app_hid_env.conidx;
                req->report.type     = HOGPD_REPORT;
                req->report.idx      = APP_HID_KEYBOARD_IN_ENDPORT; 
                req->report.length   = APP_HID_KEYBOARD_REPORT_LEN;
                memcpy(&req->report.value[0], &report_buff[0], APP_HID_KEYBOARD_REPORT_LEN);
			}
			else if(len == APP_HID_MEDIA_REPORT_LEN)
			{
				status = 1;
				
				report_buff[0] = data[0];
				report_buff[1] = data[1];

				// Allocate the HOGPD_REPORT_UPD_REQ message 
				req->conidx  = app_hid_env.conidx;
				//now fill report
				req->report.hid_idx  = app_hid_env.conidx;
				req->report.type     = HOGPD_REPORT; 
				req->report.idx      = APP_HID_MEDIA_IN_ENDPORT; 
				req->report.length   = APP_HID_MEDIA_REPORT_LEN;
				memcpy(&req->report.value[0], &report_buff[0], APP_HID_MEDIA_REPORT_LEN);
			}
			else if(len == APP_HID_POWER_REPORT_LEN)
			{
				status = 1;
				report_buff[0] = data[0];

				//Allocate the HOGPD_REPORT_UPD_REQ message
				req->conidx  = app_hid_env.conidx;
				//now fill report
				req->report.hid_idx  = app_hid_env.conidx;
				req->report.type     = HOGPD_REPORT;
				req->report.idx      = APP_HID_POWER_IN_ENDPORT;
				req->report.length   = APP_HID_POWER_REPORT_LEN;
				memcpy(&req->report.value[0], &report_buff[0], APP_HID_POWER_REPORT_LEN);
			}
	      	// Buffer used to create the Report
			if(status)
			{
				kernel_msg_send(req);
			}
			else
			{
				kernel_msg_free(kernel_param2msg(req));	// free
			}
	    } break;

	    case (APP_HID_WAIT_REP):
	    {
	        // Requested connection parameters
	        struct gapc_conn_param conn_param;

	       /*
	        * Requested connection interval: 10ms
	        * Latency: 25
	        * Supervision Timeout: 2s
	        */
	        conn_param.intv_min = 8;
	        conn_param.intv_max = 8;
	        conn_param.latency  = 25;
	        conn_param.time_out = 200;

	        appm_update_param(&conn_param);

	        // Go back to the ready state
	        app_hid_env.state = APP_HID_READY;
	    } break;

	    case (APP_HID_IDLE):
	    {
	        // Try to restart advertising if needed
#if (BLE_APP_MS)
			ms_start_advertising(&ms_adv_init_info);	
#else
            appm_start_advertising();	
#endif //(BLE_APP_MS)
        } break;

	    // DISABLE and ENABLED states
	    default:
	    {
	        // Drop the message
	    } break;
	}
}




void app_hid_send_mouse_report( struct mouse_msg report )
{
	if(app_hid_env.state == APP_HID_READY)
	{
		// Buffer used to create the Report
        uint8_t report_buff[APP_HID_MOUSE_REPORT_LEN];

		// Clean the report buffer
        memset(&report_buff[0], 0, APP_HID_MOUSE_REPORT_LEN);

		// Set the button states
        report_buff[0] = (report.b & 0x7);
		report_buff[1] = report.x;
		report_buff[2] = report.y;
		report_buff[3] = report.w;

		// Allocate the HOGPD_REPORT_UPD_REQ message
        struct hogpd_report_upd_req * req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                         		prf_get_task_from_id(TASK_ID_HOGPD), 
                                         		TASK_APP,
                                         		hogpd_report_upd_req,
                                         		APP_HID_MOUSE_REPORT_LEN);
		//now fill report
		req->conidx  = app_hid_env.conidx;
		req->report.hid_idx  = app_hid_env.conidx;
		req->report.type     = HOGPD_REPORT;
		req->report.idx      = APP_HID_MOUSE_IN_ENDPORT; 
        req->report.length   = APP_HID_MOUSE_REPORT_LEN;
		memcpy(&req->report.value[0], &report_buff[0], APP_HID_MOUSE_REPORT_LEN);
		
        kernel_msg_send(req);
		
	}
	else
	{
		//UART_PRINTF("app_hid_env.state = 0x%x\r\n",app_hid_env.state);
	}
}


void app_hid_send_sensor_report(uint8_t *sensor_data)
{
	if(app_hid_env.state == APP_HID_READY)
	{
		// Buffer used to create the Report
        uint8_t report_buff[APP_HID_SENSOR_REPORT_LEN];

		// Clean the report buffer
        memset(&report_buff[0], 0, APP_HID_SENSOR_REPORT_LEN);
		// Copy sensor data
		memcpy(&report_buff[0], sensor_data, APP_HID_SENSOR_REPORT_LEN);

		// Allocate the HOGPD_REPORT_UPD_REQ message
        struct hogpd_report_upd_req * req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                         		prf_get_task_from_id(TASK_ID_HOGPD), 
                                         		TASK_APP,
                                         		hogpd_report_upd_req,
                                         		APP_HID_SENSOR_REPORT_LEN);
		//now fill report
		req->conidx  = app_hid_env.conidx;
		req->report.hid_idx  = app_hid_env.conidx;
		req->report.type     = HOGPD_REPORT;
		req->report.idx      = APP_HID_SENSOR_IN_ENDPOINT; 
        req->report.length   = APP_HID_SENSOR_REPORT_LEN;
		memcpy(&req->report.value[0], &report_buff[0], APP_HID_SENSOR_REPORT_LEN);

		kernel_msg_send(req);
	}
	else
	{
		//UART_PRINTF("app_hid_env.state = 0x%x\r\n",app_hid_env.state);
	}
}


static void app_hid_send_voice(uint8_t* buf, uint8_t len)
{
	if(app_hid_env.state == APP_HID_READY)
	{
		// Check if the report can be sent
	 	struct hogpd_report_upd_req * req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                 prf_get_task_from_id(TASK_ID_HOGPD),
                                                 TASK_APP,
                                                 hogpd_report_upd_req,
                                                 APP_HID_VOICE_REPORT_LEN);
			
       	// Allocate the HOGPD_REPORT_UPD_REQ message
        req->conidx  = app_hid_env.conidx;
		
        //now fill report
        req->report.hid_idx  = app_hid_env.conidx;
        req->report.type     = HOGPD_REPORT;
        req->report.idx      = APP_HID_RMC1_IN_ENDPORT; 
        req->report.length   = APP_HID_VOICE_REPORT_LEN;
        memcpy(&req->report.value[0], buf, APP_HID_VOICE_REPORT_LEN);

		kernel_msg_send(req);
	}
	else
	{
		//UART_PRINTF("app_hid_env.state = 0x%x\r\n",app_hid_env.state);
	}
	
}


/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */
static int hogpd_ctnl_pt_ind_handler(kernel_msg_id_t const msgid,
                                     struct hogpd_ctnl_pt_ind const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{

	//UART_PRINTF("%s\r\n",__func__);
	if (param->conidx == app_hid_env.conidx)
	{
	    //make use of param->hid_ctnl_pt
	    struct hogpd_report_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
	                                             prf_get_task_from_id(TASK_ID_HOGPD),
	                                             TASK_APP,
	                                             hogpd_report_cfm,
	                                             0);

        req->conidx = param->conidx;
        /// Operation requested (read/write @see enum hogpd_op)
        req->operation = HOGPD_OP_REPORT_WRITE;
        /// Status of the request
        req->status = GAP_ERR_NO_ERROR; 
        /// HIDS Instance
        req->report.hid_idx = app_hid_env.conidx; 
        /// type of report (@see enum hogpd_report_type)
        req->report.type = HOGPD_BOOT_REPORT_DEFAULT;	//-1;//outside 
        /// Report Length (uint8_t)
        req->report.length = 0;
        /// Report Instance - 0 for boot reports and report map
        req->report.idx = 0;
        /// Report data
        

        // Send the message
        kernel_msg_send(req);
    }
	
    return (KERNEL_MSG_CONSUMED);
}




static int hogpd_ntf_cfg_ind_handler(kernel_msg_id_t const msgid,
                                     struct hogpd_ntf_cfg_ind const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s\r\n",__func__);
	if (app_hid_env.conidx == param->conidx)
	{
		if ((param->ntf_cfg[param->conidx] & HOGPD_CFG_REPORT_NTF_EN ) != 0)
		{
			// The device is ready to send reports to the peer device
			app_hid_env.state = APP_HID_READY;
		}
		else
		{
			// Come back to the Enabled state
			if (app_hid_env.state == APP_HID_READY)
			{
				app_hid_env.state = APP_HID_ENABLED;
			}
		}
	#if (NVDS_SUPPORT)
		// Store the notification configuration in the database
		if (nvds_put(NVDS_TAG_MOUSE_NTF_CFG, NVDS_LEN_MOUSE_NTF_CFG,
						(uint8_t *)&param->ntf_cfg[param->conidx]) != NVDS_OK)
		{
			// Should not happen
			ASSERT_ERR(0);
		}
	#endif //#if (NVDS_SUPPORT)
	}

    return (KERNEL_MSG_CONSUMED);
}


static int hogpd_report_req_ind_handler(kernel_msg_id_t const msgid,
                                    struct hogpd_report_req_ind const *param,
                                    kernel_task_id_t const dest_id,
                                    kernel_task_id_t const src_id)
{
	
	//UART_PRINTF("%s operation = %x,type = %x\r\n",__func__,param->operation,param->report.type);

	if ((param->operation == HOGPD_OP_REPORT_READ) && (param->report.type == HOGPD_REPORT_MAP))
	{
		struct hogpd_report_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
	                                        prf_get_task_from_id(TASK_ID_HOGPD),
	                                        TASK_APP, 
	                                        hogpd_report_cfm,
	                                        APP_HID_BK_REPORT_MAP_LEN );

		req->conidx = app_hid_env.conidx; 
		/// Operation requested (read/write @see enum hogpd_op)
		req->operation = HOGPD_OP_REPORT_READ;
		/// Status of the request
		req->status = GAP_ERR_NO_ERROR; 
		/// HIDS Instance
		req->report.hid_idx = param->report.hid_idx;
		/// type of report (@see enum hogpd_report_type)
		req->report.type = HOGPD_REPORT_MAP;
		/// Report Length (uint8_t)
		req->report.length =  APP_HID_BK_REPORT_MAP_LEN; 
		/// Report Instance - 0 for boot reports and report map
		req->report.idx = 0;
		/// Report data

		//memcpy(&req->report.value[0], &hid_report_map_array[0], APP_HID_BK_REPORT_MAP_LEN);
		memcpy(&req->report.value[0], &gHIDReportDescriptor[0], APP_HID_BK_REPORT_MAP_LEN);
				
		// Send the message
		kernel_msg_send(req);

	}
	else
	{
		if (param->report.type == HOGPD_BOOT_MOUSE_INPUT_REPORT)
		{ 	
			//request of boot mouse report
			struct hogpd_report_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
			                                        prf_get_task_from_id(TASK_ID_HOGPD),
			                                        TASK_APP,
			                                        hogpd_report_cfm,
			                                        0/*param->report.length*/);

			req->conidx = param->conidx; ///app_hid_env.conidx; 
			/// Operation requested (read/write @see enum hogpd_op)
			req->operation = HOGPD_OP_REPORT_READ;
			/// Status of the request
			req->status = GAP_ERR_NO_ERROR;  
			/// HIDS Instance
			req->report.hid_idx = app_hid_env.conidx; 
			/// type of report (@see enum hogpd_report_type)
			req->report.type = param->report.type;//-1;//outside 
			/// Report Length (uint8_t)
			req->report.length = 0; //param->report.length;
			/// Report Instance - 0 for boot reports and report map
			req->report.idx = param->report.idx; //0;
			/// Report data

			// Send the message
			kernel_msg_send(req);
		}
		else if (param->report.type == HOGPD_REPORT)
		{
			//request of mouse report
			struct hogpd_report_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
			                                                prf_get_task_from_id(TASK_ID_HOGPD),
			                                                TASK_APP,
			                                                hogpd_report_cfm,
			                                                8/*param->report.length*/);

			req->conidx = param->conidx; ///app_hid_env.conidx; 
			/// Operation requested (read/write @see enum hogpd_op)
			req->operation = HOGPD_OP_REPORT_READ;
			/// Status of the request
			req->status = GAP_ERR_NO_ERROR;  
			/// HIDS Instance
			req->report.hid_idx = app_hid_env.conidx; 
			/// type of report (@see enum hogpd_report_type)
			req->report.type = param->report.type;
			/// Report Length (uint8_t)
			req->report.length = 8; //param->report.length;
			/// Report Instance - 0 for boot reports and report map
			req->report.idx = param->report.idx; //0;
			/// Report data
			memset(&req->report.value[0], 0, 8); 
			req->report.value[0] = param->report.hid_idx;  /// HIDS Instance
			req->report.value[1] = param->report.type;    /// type of report (@see enum hogpd_report_type)
			req->report.value[2] = param->report.length; /// Report Length (uint8_t)
			req->report.value[3] = param->report.idx;    /// Report Instance - 0 for boot reports and report map

			// Send the message
			kernel_msg_send(req);
		}
		else
		{
			struct hogpd_report_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
			                                            prf_get_task_from_id(TASK_ID_HOGPD),
			                                            TASK_APP,
			                                            hogpd_report_cfm,
			                                            8/*param->report.length*/);

			req->conidx = param->conidx; ///app_hid_env.conidx;
			/// Operation requested (read/write @see enum hogpd_op)
			req->operation = HOGPD_OP_REPORT_READ;
			/// Status of the request
			req->status = GAP_ERR_NO_ERROR;  
			/// Report Info
			//req->report;
			/// HIDS Instance
			req->report.hid_idx = app_hid_env.conidx;
			/// type of report (@see enum hogpd_report_type)
			req->report.type = param->report.type;
			/// Report Length (uint8_t)
			req->report.length = 8; //param->report.length;
			/// Report Instance - 0 for boot reports and report map
			req->report.idx = param->report.idx; //0;
			/// Report data
			memset(&req->report.value[0], 0, 8); 
			req->report.value[0] = param->report.hid_idx; /// HIDS Instance
			req->report.value[1] = param->report.type;    /// type of report (@see enum hogpd_report_type)
			req->report.value[2] = param->report.length;  /// Report Length (uint8_t)
			req->report.value[3] = param->report.idx;     /// Report Instance - 0 for boot reports and report map

			// Send the message
			kernel_msg_send(req);
		}
	}

    return (KERNEL_MSG_CONSUMED);
}


static int hogpd_proto_mode_req_ind_handler(kernel_msg_id_t const msgid,
                                        struct hogpd_proto_mode_req_ind const *param,
                                        kernel_task_id_t const dest_id,
                                        kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s\r\n",__func__);
	if ((param->conidx == app_hid_env.conidx) && (param->operation == HOGPD_OP_PROT_UPDATE))
	{

		//make use of param->proto_mode
		struct hogpd_proto_mode_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_PROTO_MODE_CFM,
		                                            prf_get_task_from_id(TASK_ID_HOGPD),
		                                            TASK_APP,
		                                            hogpd_proto_mode_cfm,
		                                            0);
		/// Connection Index
		req->conidx = app_hid_env.conidx; 
		/// Status of the request
		req->status = GAP_ERR_NO_ERROR;
		/// HIDS Instance
		req->hid_idx = app_hid_env.conidx;
		/// New Protocol Mode Characteristic Value
		req->proto_mode = param->proto_mode;


		// Send the message
		kernel_msg_send(req);
	}
	else
	{
		struct hogpd_proto_mode_cfm *req = KERNEL_MSG_ALLOC_DYN(HOGPD_PROTO_MODE_CFM,
		                                                prf_get_task_from_id(TASK_ID_HOGPD),
		                                                TASK_APP,
		                                                hogpd_proto_mode_cfm,
		                                                0);
		/// Status of the request
		req->status = ATT_ERR_APP_ERROR;

		/// Connection Index
		req->conidx = app_hid_env.conidx;
		/// HIDS Instance
		req->hid_idx = app_hid_env.conidx;
		/// New Protocol Mode Characteristic Value
		req->proto_mode = param->proto_mode;

		// Send the message
		kernel_msg_send(req);
	}
    return (KERNEL_MSG_CONSUMED);
}



/*发送音频数据*/
void app_hid_send_audio(void)
{
	uint8_t empty_buf_count;
	uint8_t i;

	if(app_hid_env.audio_start )
	{
		empty_buf_count = 4 - app_hid_env.encode_tx_cnt;
		if( empty_buf_count > 0)
		{
			for(i=0; i<empty_buf_count; i++)
			{
				if(read_encode_data(send_buf)) 
				{
					app_hid_env.encode_tx_cnt++;
					send_buf[0] = app_hid_env.encode_send_cnt++; //debug
					//app_hid_send_voice(&send_buf[1], APP_HID_VOICE_REPORT_LEN);
					app_hid_send_voice(&send_buf[0], APP_HID_VOICE_REPORT_LEN);
				}
			}
		}
	}
}



static int hogpd_report_upd_handler(kernel_msg_id_t const msgid,
                                   struct hogpd_report_upd_rsp const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
	if(app_hid_env.encode_tx_cnt > 0)
	{
		app_hid_env.encode_tx_cnt--;
	}
	
    if (app_hid_env.conidx == param->conidx)
    {
        if (GAP_ERR_NO_ERROR == param->status)
        {	
			//UART_PRINTF("key report succ\r\n");
        }
        else
        {
            // Go back to the ready state
            app_hid_env.state = APP_HID_READY;
        }
    }
	
    return (KERNEL_MSG_CONSUMED);
}


static int hogpd_enable_rsp_handler(kernel_msg_id_t const msgid,
                                     struct hogpd_enable_rsp const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s\r\n",__func__);
	
    return (KERNEL_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_hid_msg_dflt_handler(kernel_msg_id_t const msgid,
                                    void const *param,
                                    kernel_task_id_t const dest_id,
                                    kernel_task_id_t const src_id)
{
    // Drop the message
	//UART_PRINTF("%s\r\n",__func__);
	
    return (KERNEL_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Set the value of the Report Map Characteristic in the database
 ****************************************************************************************
 */
void app_hid_set_report_map(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_hid_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,        (kernel_msg_func_t)app_hid_msg_dflt_handler},
    {HOGPD_ENABLE_RSP,              (kernel_msg_func_t)hogpd_enable_rsp_handler},
    {HOGPD_NTF_CFG_IND,             (kernel_msg_func_t)hogpd_ntf_cfg_ind_handler},
    {HOGPD_REPORT_REQ_IND,          (kernel_msg_func_t)hogpd_report_req_ind_handler},
    {HOGPD_PROTO_MODE_REQ_IND,  	(kernel_msg_func_t)hogpd_proto_mode_req_ind_handler},
    {HOGPD_CTNL_PT_IND,             (kernel_msg_func_t)hogpd_ctnl_pt_ind_handler},
    {HOGPD_REPORT_UPD_RSP,          (kernel_msg_func_t)hogpd_report_upd_handler},
};

const struct kernel_state_handler app_hid_table_handler =
    {&app_hid_msg_handler_list[0], (sizeof(app_hid_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //(BLE_APP_HID)

/// @} APP
