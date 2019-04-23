/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
#include "rwapp_config.h"
#include "app_task.h"                // Application task Definition
#include "application.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "common_bt.h"                   // Common BT Definition
#include "common_math.h"                 // Common Maths Definition
#include "kernel_timer.h"

#if (BLE_APP_FFF0)
#include "app_fff0.h"                 // Application security Definition
#endif // (BLE_APP_FFF0)

#if (BLE_APP_FFE0)
#include "app_ffe0.h"                 // Application security Definition
#endif // (BLE_APP_FFE0)


#if (BLE_APP_FEB3)
#include "ais_sdk.h"                 // Application  Definition
#endif // (BLE_APP_FEB3)


#if (BLE_APP_MS)
#include "app_ms.h"                 // Application  Definition
#endif // (BLE_APP_MS)

#if (BLE_APP_BTL)
#include "app_btl.h"                 // Application  Definition
#endif // (BLE_APP_BTL)

#if (BLE_APP_SN)
#include "app_sn.h"                 // Application  Definition
#endif // (BLE_APP_SN)

#if (BLE_APP_AYLA)
#include "app_ayla.h"                 // Application  Definition
#endif // (BLE_APP_AYLA)

#if (BLE_APP_ANCSC)
#include "app_ancsc.h"                 // Application fff0s Definition
#endif // (BLE_APP_ANCSC)

#if (BLE_APP_SEC)
#include "app_sec.h"                 // Application security Definition
#endif // (BLE_APP_SEC)

#if (BLE_APP_HT)
#include "app_ht.h"                  // Health Thermometer Application Definitions
#endif //(BLE_APP_HT)


#if (BLE_APP_DIS)
#include "app_dis.h"                 // Device Information Service Application Definitions
#endif //(BLE_APP_DIS)

#if (BLE_APP_BATT)
#include "app_batt.h"                // Battery Application Definitions
#endif //(BLE_APP_DIS)

#if (BLE_APP_HID)
#include "app_hid.h"                 // HID Application Definitions
#endif //(BLE_APP_HID)

#if (DISPLAY_SUPPORT)
#include "app_display.h"             // Application Display Definition
#endif //(DISPLAY_SUPPORT)

#if (BLE_APP_OADS)
#include "app_oads.h"                 // Application oads Definition
#endif // (BLE_APP_OADS)

#ifdef BLE_APP_AM0
#include "am0_application.h"                 // Audio Mode 0 Application
#endif //defined(BLE_APP_AM0)

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#include "ble_pub.h"
#include "gapm_task.h"
char app_dflt_dev_name[APP_DEVICE_NAME_LENGTH_MAX] = {0};


#if (BLE_APP_HID)
// HID Mouse
#define DEVICE_NAME        "Hid Mouse"
#else
#define DEVICE_NAME        "RW DEVICE"
#endif



#define DEVICE_NAME_SIZE    sizeof(DEVICE_NAME)


/**
 * UUID List part of ADV Data
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x09\x18 - Health Thermometer Service UUID
 *   or
 * x12\x18 - HID Service UUID
 * --------------------------------------------------------------------------------------
 */

#if (BLE_APP_FFF0)
#define APP_FFF0_ADV_DATA_UUID        "\x03\x03\xF0\xFF"
#define APP_FFF0_ADV_DATA_UUID_LEN    (4)
#endif //(BLE_APP_HT)

#if (BLE_APP_FFE0)
#define APP_FFE0_ADV_DATA_UUID        "\x03\x03\xE0\xFF"
#define APP_FFE0_ADV_DATA_UUID_LEN    (4)
#endif //(BLE_APP_HT)


#if (BLE_APP_HT)
#define APP_HT_ADV_DATA_UUID        "\x03\x03\x09\x18"
#define APP_HT_ADV_DATA_UUID_LEN    (4)
#endif //(BLE_APP_HT)

#if (BLE_APP_HID)
#define APP_HID_ADV_DATA_UUID       "\x03\x03\x12\x18"
#define APP_HID_ADV_DATA_UUID_LEN   (4)
#endif //(BLE_APP_HID)

/**
 * Appearance part of ADV Data
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x19 - Appearance
 * x03\x00 - Generic Thermometer
 *   or
 * xC2\x03 - HID Mouse
 * xC1\x03 - HID Keyboard
 * xC3\x03 - HID Joystick
 * --------------------------------------------------------------------------------------
 */

#if (BLE_APP_HT)
#define APP_HT_ADV_DATA_APPEARANCE    "\x03\x19\x00\x03"
#endif //(BLE_APP_HT)

#if (BLE_APP_HID)
#define APP_HID_ADV_DATA_APPEARANCE   "\x03\x19\xC1\x03"
#endif //(BLE_APP_HID)


#define APP_ADV_DATA_APPEARANCE_LEN  (4)

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
#define APP_SCNRSP_DATA        "\x08\x08\x42\x4B\x2D\x33\x34\x33\x35" //BK-3435"
//#define APP_SCNRSP_DATA        "\x03\x19\x00\x03"
#define APP_SCNRSP_DATA_LEN     (9)

#if (BLE_APP_HID)
#define APP_HID_SCNRSP_DATA       "\x03\x03\x12\x18"
#define APP_HID_SCNRSP_DATA_LEN   (4)
#endif //(BLE_APP_HID)

/**
 * Advertising Parameters
 */

/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         (160)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         (160)
/// Fast advertising interval
#define APP_ADV_FAST_INT        (32)

#define MAX_ADV_DATA_LEN        (0x1F)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef void (*appm_add_svc_func_t)(void);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    #if (BLE_APP_FFF0)
    APPM_SVC_FFF0,
    #endif // (BLE_APP_FFF0)
    #if (BLE_APP_FFE0)
    APPM_SVC_FFE0,
    #endif // (BLE_APP_FFE0)
	
	#if (BLE_APP_FEB3)
    APPM_SVC_FEB3,
    #endif // (BLE_APP_FEB3)
	
	#if (BLE_APP_MS)
	APPM_SVC_MS,
	#endif // (BLE_APP_MS)

    #if (BLE_APP_BTL)
	APPM_SVC_BTL,
	#endif // (BLE_APP_BTL)

    #if (BLE_APP_SN)
	APPM_SVC_SN,
    #endif // (BLE_APP_SN)

    #if (BLE_APP_AYLA)
	APPM_SVC_AYLA,
    #endif // (BLE_APP_AYLA)
	
	#if (BLE_APP_ANCSC)
	APPM_SVC_ANCSC,
  	#endif //(BLE_APP_ANCSC)
    #if (BLE_APP_HT)
    APPM_SVC_HTS,
    #endif //(BLE_APP_HT)
    #if (BLE_APP_DIS)
    APPM_SVC_DIS,
    #endif //(BLE_APP_DIS)
    #if (BLE_APP_BATT)
    APPM_SVC_BATT,
    #endif //(BLE_APP_BATT)
    #if (BLE_APP_HID)
    APPM_SVC_HIDS,
    #endif //(BLE_APP_HID)
    #ifdef BLE_APP_AM0
    APPM_SVC_AM0_HAS,
    #endif //defined(BLE_APP_AM0)
	#if (BLE_APP_OADS)
	APPM_SVC_OADS,
	#endif //(BLE_APP_OADS)

    APPM_SVC_LIST_STOP ,
};

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct kernel_task_desc TASK_DESC_APP = {NULL, &appm_default_handler,
                                                  appm_state, APPM_STATE_MAX, APP_IDX_MAX};

adv_info_t adv_info = {
    .channel_map = 7,
    .interval_min = APP_ADV_INT_MIN,
    .interval_max = APP_ADV_INT_MAX
};

/// List of functions used to create the database
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] =
{
    #if (BLE_APP_FFF0)
    (appm_add_svc_func_t)app_fff0_add_fff0s,
    #endif // (BLE_APP_FFF0)
    #if (BLE_APP_FFE0)
    (appm_add_svc_func_t)app_ffe0_add_ffe0s,
    #endif // (BLE_APP_FFE0)

    #if (BLE_APP_FEB3)
    (appm_add_svc_func_t)app_ais_add_feb3s,
    #endif // (BLE_APP_FEB3)
	
	#if (BLE_APP_MS)
    (appm_add_svc_func_t)app_ms_add_ms,
    #endif // (BLE_APP_MS)

    #if (BLE_APP_BTL)
    (appm_add_svc_func_t)app_btl_add_btl,
    #endif // (BLE_APP_BTL)

    #if (BLE_APP_SN)
    (appm_add_svc_func_t)app_sn_add_sn,
    #endif // (BLE_APP_SN)

    #if (BLE_APP_AYLA)
    (appm_add_svc_func_t)app_ayla_add_ayla,
    #endif // (BLE_APP_AYLA)
	
	#if (BLE_APP_ANCSC)
    (appm_add_svc_func_t)app_ancs_add_ancsc,
    #endif //(BLE_APP_ANCSC)
    #if (BLE_APP_HT)
    (appm_add_svc_func_t)app_ht_add_hts,
    #endif //(BLE_APP_HT)
    #if (BLE_APP_DIS)
    (appm_add_svc_func_t)app_dis_add_dis,
    #endif //(BLE_APP_DIS)
    #if (BLE_APP_BATT)
    (appm_add_svc_func_t)app_batt_add_bas,
    #endif //(BLE_APP_BATT)
    #if (BLE_APP_HID)
    (appm_add_svc_func_t)app_hid_add_hids,
    #endif //(BLE_APP_HID)
    #ifdef BLE_APP_AM0
    (appm_add_svc_func_t)am0_app_add_has,
    #endif //defined(BLE_APP_AM0)
	#if (BLE_APP_OADS)
	(appm_add_svc_func_t)app_oad_add_oads,
	#endif //(BLE_APP_OADS)
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env;
char *dev_str_name = NULL;
int *scan_check_result = NULL;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void appm_init()
{
    #if (NVDS_SUPPORT)
	
	  uint8_t peer_irk_len = NVDS_LEN_PEER_IRK;
    uint8_t key_len = KEY_LEN;
    #endif //(NVDS_SUPPORT)

    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    kernel_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    kernel_state_set(TASK_APP, APPM_INIT);
#if 0
    // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
    #if (NVDS_SUPPORT)
    app_env.dev_name_len = APP_DEVICE_NAME_MAX_LEN;

    if (nvds_get(NVDS_TAG_DEVICE_NAME, &(app_env.dev_name_len), app_env.dev_name) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        // Get default Device Name (No name if not enough space)
        memcpy(app_env.dev_name, APP_DFLT_DEVICE_NAME, APP_DFLT_DEVICE_NAME_LEN);
        app_env.dev_name_len = APP_DFLT_DEVICE_NAME_LEN;

        // TODO update this value per profiles
    }

    #if (NVDS_SUPPORT)
    if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, app_env.loc_irk) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        uint8_t counter;

        // generate a new IRK
        for (counter = 0; counter < KEY_LEN; counter++)
        {
            app_env.loc_irk[counter]    = (uint8_t)common_rand_word();
        }

        // Store it in NVDS
        #if (NVDS_SUPPORT)
        // Store the generated value in NVDS
        if (nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&app_env.loc_irk) != NVDS_OK)
        {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
        #endif // #if (NVDS_SUPPORT)
    }
			  	#if (NVDS_SUPPORT)
           // Store peer identity in NVDS
           if (nvds_get(NVDS_TAG_PEER_IRK, &peer_irk_len, (uint8_t *)&app_env.peer_irk.irk.key) != NVDS_OK)
           {
             UART_PRINTF("not NVDS_TAG_PEER_IRK\r\n");
						 
           }else
					 {
						 for(int i = 0;i<sizeof(struct gap_sec_key);i++)
					{
							UART_PRINTF("irk.key[%d]  = %x\r\n",i,app_env.peer_irk.irk.key[i]);
					}
					
					UART_PRINTF("addr tyep = %x\r\n",app_env.peer_irk.addr.addr_type);
					for(int i = 0;i<sizeof(struct bd_addr);i++)
					{
							UART_PRINTF("addr.addr[%d]  = %x\r\n",i,app_env.peer_irk.addr.addr.addr[i]);
					}
					 
					 }
           #endif // (NVDS_SUPPORT)
#endif					 			

    /*------------------------------------------------------
     * INITIALIZE ALL MODULES
     *------------------------------------------------------*/

    // load device information:
    #if (DISPLAY_SUPPORT)
    app_display_init();
    #endif //(DISPLAY_SUPPORT)
		
    #if (BLE_APP_FFE0)
    // FFE0 Module
    app_ffe0_init();
    #endif // (BLE_APP_FFE0)

	#if (BLE_APP_ANCSC)
    // ANCS Module
    app_ancsc_init();
    #endif //(BLE_APP_ANCSC)

    #if (BLE_APP_SEC)
    // Security Module
    app_sec_init();
    #endif // (BLE_APP_SEC)

    #if (BLE_APP_HT)
    // Health Thermometer Module
    app_ht_init();
    #endif //(BLE_APP_HT)

    #if (BLE_APP_DIS)
    // Device Information Module
    app_dis_init();
    #endif //(BLE_APP_DIS)

    #if (BLE_APP_HID)
    // HID Module
    app_hid_init();
    #endif //(BLE_APP_HID)

    #if (BLE_APP_BATT)
    // Battery Module
    app_batt_init();
    #endif //(BLE_APP_BATT)

    #ifdef BLE_APP_AM0
    // Audio Mode 0 Module
    am0_app_init();
    #endif // defined(BLE_APP_AM0)
			
	#if (BLE_APP_OADS)
    app_oads_init();
    #endif //(BLE_APP_OADS)

	#if (BLE_APP_FEB3)
	app_feb3_init();
	#endif //(BLE_APP_FEB3)
		
	#if (BLE_APP_MS)
 	app_ms_init();
	#endif //(BLE_APP_MS)

    #if (BLE_APP_BTL)
 		app_btl_init();
	#endif //(BLE_APP_BTL)

    #if (BLE_APP_SN)
 		app_sn_init();
    #endif //(BLE_APP_SN)

    #if (BLE_APP_AYLA)
 		app_ayla_init();
    #endif //(BLE_APP_AYLA)
		
}

bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}


/*设备主动断开连接函数*/
void appm_disconnect(uint8_t reason)
{
    struct gapc_disconnect_cmd *cmd = KERNEL_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KERNEL_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = reason;

    // Send the message
    kernel_msg_send(cmd);
}

/**
 ****************************************************************************************
 * Advertising Functions
 ****************************************************************************************
 */


/* 设备发起定向广播函数*/
void appm_start_direct_dvertising(void)
{
#if (BLE_APP_MS)
	ms_start_advertising(&ms_adv_init_info);	
#else
    appm_start_advertising();	
#endif //(BLE_APP_MS)
	
    return;
	// Check if the advertising procedure is already is progress
    if (kernel_state_get(TASK_APP) == APPM_READY)
    {
        struct gapm_start_advertise_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;

        cmd->intv_min = APP_ADV_FAST_INT;
        cmd->intv_max = APP_ADV_FAST_INT;

        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
		 
		/*
		 * If the peripheral is already bonded with a central device, use the direct advertising
		 * procedure (BD Address of the peer device is stored in NVDS.
		 */
		if(app_sec_get_bond_status())
		{
			#if (NVDS_SUPPORT)
			uint8_t bd_len = NVDS_LEN_PEER_BD_ADDRESS;
			#endif
			cmd->op.code   = GAPM_ADV_DIRECT_LDC;
			//cmd->info.direct.addr_type = 1;
			
	
			#if (NVDS_SUPPORT)
			if (nvds_get(NVDS_TAG_PEER_BD_ADDRESS, &bd_len,
	            		(uint8_t *)cmd->info.direct.addr.addr) != NVDS_OK)
			{
			    // An error has occurred during access to the NVDS
			    ASSERT_INFO(0,NVDS_TAG_PEER_BD_ADDRESS,bd_len);
			}
			#endif

			kernel_msg_send(cmd);

			#if !(DEEP_SLEEP)
			kernel_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
			#endif

		//	UART_PRINTF("appm start direct advertising\r\n");
		}
		else
		{
			kernel_msg_free(kernel_param2msg(cmd));
#if (BLE_APP_MS)
			ms_start_advertising(&ms_adv_init_info);	
#else
            appm_start_advertising();		
#endif //(BLE_APP_MS)
 		//	UART_PRINTF("appm start general advertising\r\n");
		}

	    kernel_state_set(TASK_APP, APPM_ADVERTISING);	
	}		
}


extern /*const */struct bd_addr common_default_bdaddr;

/* 设备发起广播函数*/
ble_err_t appm_start_advertising(void)
{
    ble_err_t status = ERR_SUCCESS;
    uint8_t adv_idx, adv_name_len;
#if !(DEEP_SLEEP)
    kernel_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
#endif

    if(BLE_ROLE_MASTER == ble_get_role_mode())
    {
        bk_printf("current role should not be master\r\n");
        return ERR_ADV_FAIL;
    }

    // Check if the advertising procedure is already is progress
    if (kernel_state_get(TASK_APP) == APPM_READY)
    {				  
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = adv_info.channel_map;
        cmd->intv_min 		= adv_info.interval_min;
        cmd->intv_max 		= adv_info.interval_max;	
        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

        cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = adv_info.advDataLen;
		os_memcpy(cmd->info.host.adv_data, adv_info.advData, cmd->info.host.adv_data_len);
        cmd->info.host.scan_rsp_data_len  = adv_info.respDataLen;
		os_memcpy(cmd->info.host.scan_rsp_data, adv_info.respData, cmd->info.host.scan_rsp_data_len);
        
        
        // Send the message
        kernel_msg_send(cmd);
        bk_printf("appm start advertising\r\n");

        ble_init_over = 1;
        ble_set_role_mode(BLE_ROLE_SLAVE);

        // Set the state of the task to APPM_ADVERTISING
        kernel_state_set(TASK_APP, APPM_ADVERTISING);	
    }
    else
    {
        status = ERR_ADV_FAIL;
    }
    return status;
    // else ignore the request
}

#if (BLE_APP_MS)
msm_result_t ms_start_advertising(msm_ble_adv_content_t *adv_info)
{
    msm_result_t status = MSM_RESULT_SUCCESS;
	
	bk_printf("ms_start_advertising START\r\n");
#if !(DEEP_SLEEP)
    kernel_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
#endif
	
    // Check if the advertising procedure is already is progress
    if (kernel_state_get(TASK_APP) == APPM_READY)
    {				  
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;
        cmd->intv_min 		= APP_ADV_INT_MIN;
        cmd->intv_max 		= APP_ADV_INT_MAX;	
        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

        cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = adv_info->advDataLen;
		os_memcpy(cmd->info.host.adv_data, adv_info->advData, cmd->info.host.adv_data_len);
        cmd->info.host.scan_rsp_data_len  = adv_info->respDataLen;
		os_memcpy(cmd->info.host.scan_rsp_data, adv_info->respData, cmd->info.host.scan_rsp_data_len);
        
        // Send the message
        kernel_msg_send(cmd);

        // Set the state of the task to APPM_ADVERTISING
        kernel_state_set(TASK_APP, APPM_ADVERTISING);	

		msm_ble_stack_callback(MS_BLE_STACK_EVENT_START_ADV_OK);
			
        ble_init_over = 1;
    }
    else
    {
        status = MSM_RESULT_ERROR;
    }
	
    return status;
}

void ms_start_scan(void *name, int *check_result)
{
    bk_printf("ms_start_scan START %s.\r\n", name);
    if (kernel_state_get(TASK_APP) == APPM_READY)
    {
        struct gapm_start_scan_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_START_SCAN_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_scan_cmd);

        cmd->op.addr_src = GAPM_STATIC_ADDR;
        cmd->op.code = GAPM_SCAN_ACTIVE;
        cmd->interval = 50;
        cmd->window = 40;
        cmd->mode = GAP_OBSERVER_MODE;
        cmd->filt_policy = SCAN_ALLOW_ADV_ALL;
        cmd->filter_duplic = SCAN_FILT_DUPLIC_DIS;

        dev_str_name = name;
        scan_check_result = check_result;

        kernel_msg_send(cmd);

        kernel_state_set(TASK_APP, APPM_SCANNING);

    }
    else
    {
        bk_printf("not ready\r\n");
    }

}
#endif //(BLE_APP_MS)

/* 设备主动停止广播函数*/
ble_err_t appm_stop_advertising(void)
{
	
	ble_err_t status = ERR_SUCCESS;
    if (kernel_state_get(TASK_APP) == APPM_ADVERTISING)
    {

        // Go in ready state
        kernel_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        kernel_msg_send(cmd);
	
#if (BLE_APP_MS)
        msm_ble_stack_callback(MS_BLE_STACK_EVENT_START_ADV_OK);
#endif
	
    }
    else
    {
        status = ERR_STOP_ADV_FAIL;	
    }
	
    return  status;
    // else ignore the request
}

ble_err_t appm_stop_scan(void)
{
    ble_err_t status = ERR_SUCCESS;
    if (kernel_state_get(TASK_APP) == APPM_SCANNING)
    {
        // Go in ready state
        kernel_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        kernel_msg_send(cmd);

        kernel_state_set(TASK_APP, APPM_READY);
	
    }
    else
    {
        status = -1;	
    }
	
    return  status;
}

void appm_update_param(struct gapc_conn_param *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KERNEL_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KERNEL_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

    cmd->operation  = GAPC_UPDATE_PARAMS;
    cmd->intv_min   = conn_param->intv_min;
    cmd->intv_max   = conn_param->intv_max;
    cmd->latency    = conn_param->latency;
    cmd->time_out   = conn_param->time_out;

    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
		
  //  printf("intv_min = %d,intv_max = %d,latency = %d,time_out = %d\r\n",
//		cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
	
    // Send the message
    kernel_msg_send(cmd);
}


uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
    // return name length
    return app_env.dev_name_len;
}


#if (BLE_APP_SEC)
void appm_send_seurity_req(void)
{
	//printf("%s \r\n",__func__);
    return;
    app_sec_send_security_req(app_env.conidx);
}
#endif


#endif //(BLE_APP_PRESENT)



/// @} APP


