/**
 ****************************************************************************************
 *
 * @file hci_uart.c
 *
 * @brief HCIH EIF transport module functions.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HCI_UART
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "common_endian.h"
#include "common_utils.h"
#include "common_error.h"

#include "hci.h"
#include "hci_uart_msg.h"
#include "hci_uart.h"

#include "uart.h"

#include "rwip.h"



/*
 * MACROS
 ****************************************************************************************
 */



/// Macro to get OCF of a known command
#define OCF(cmd)        (HCI_OP2OCF(HCI_##cmd##_CMD_OPCODE))



/*LOWER LAYERS USE*/
///HCI Table with handled command opcodes for check
const uint16_t hci_cmd_opcodes[HCI_CMD_OPCODE_NB_MAX]=
{
    // Legacy commands
    HCI_RESET_CMD_OPCODE,
    HCI_RD_LOCAL_VER_INFO_CMD_OPCODE,

    // DBG commands
    HCI_DBG_RD_MEM_CMD_OPCODE,
    HCI_DBG_WR_MEM_CMD_OPCODE,
    HCI_DBG_DEL_PAR_CMD_OPCODE,
    HCI_DBG_FLASH_ID_CMD_OPCODE,
    HCI_DBG_FLASH_ER_CMD_OPCODE,
    HCI_DBG_FLASH_WR_CMD_OPCODE,
    HCI_DBG_FLASH_RD_CMD_OPCODE,
    HCI_DBG_RD_PAR_CMD_OPCODE,
    HCI_DBG_WR_PAR_CMD_OPCODE,
    HCI_DBG_PLF_RESET_CMD_OPCODE,
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
///HCI environment context
struct hci_uart_env_tag hci_uart_env;




/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ******************************************************************************************
 * @brief Local function : places HCIH EIF in RX_START state and sets the UART environment.
 ******************************************************************************************
 */
static void hci_uart_read_start(void);

/**
 ****************************************************************************************
 * @brief Local function : places HCIH EIF in RX header state and sets the UART env.
 * @param[in] len Length of header to be received in the currently set buffer.
 *****************************************************************************************
 */
static void hci_uart_read_hdr(uint8_t len);

/**
 ******************************************************************************************
 * @brief Local function : places HCIH EIF in RX payload state and sets the UART env.
 * @param[in] len Length of payload to be received in the currently set buffer.
 ******************************************************************************************
 */
static void hci_uart_read_payl(uint8_t len);

/**
 ****************************************************************************************
 * @brief Local function : extracts command header components and places them in the
 * HCI environment command header structure.
 *****************************************************************************************
 */
static void hci_uart_rx_cmd_hdr_extract(void);

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static void hci_uart_read_start(void)
{
    //Initialize UART in reception mode state
    hci_uart_env.rx_state = HCI_STATE_RX_START;

    //Set the UART environment to message type 1 byte reception
  //  uart_read(&hci_uart_env.curr_msg_type, HCI_TRANSPORT_HDR_LEN);
}

static void hci_uart_read_hdr(uint8_t len)
{
    //change Rx state - wait for header next
    hci_uart_env.rx_state = HCI_STATE_RX_HDR;

    //set UART environment to header reception of len bytes
//    uart_read(hci_uart_env.curr_hdr_buff, len);
}

static void hci_uart_read_payl(uint8_t len)
{
    //change rx state to payload reception
    hci_uart_env.rx_state = HCI_STATE_RX_PAYL;

    //set UART environment to payload reception of len bytes
  //  uart_read(hci_uart_env.cmd_buf, len);
}

static void hci_uart_rx_cmd_hdr_extract(void)
{
    //check opcode existence
    hci_uart_env.chdr.known_opcode = 0;

    //check if opcode exists
    for(int i=0; i<HCI_CMD_OPCODE_NB_MAX;i++)
    {
        if (hci_cmd_opcodes[i] == common_btohs(common_read16p(hci_uart_env.curr_hdr_buff)))
        {
            hci_uart_env.chdr.known_opcode = 1;
            break;
        }
    }

    //extract command header:ocf, ogf, parameter length
    hci_uart_env.chdr.ocf    = hci_uart_env.curr_hdr_buff[0];
    hci_uart_env.chdr.ogf    = hci_uart_env.curr_hdr_buff[1]>>2;
    hci_uart_env.chdr.parlen = hci_uart_env.curr_hdr_buff[2];
}


/**
 ****************************************************************************************
 * @brief Function called in basic kernel message sending for Controller Baseband commands
 *        with no parameters.
 *
 * @param ocf  OCF of command for identification.
 * Uses the hci_uart_env command header OGF and OCF components.
 *****************************************************************************************
 */
static void hci_cmd_dispatch_basic_cntlr_bb(uint8_t ocf)
{
    switch(ocf)
    {
        case OCF(RESET):
            // Send command complete event
            hci_ccevt_pk(HCI_RESET_CMD_OPCODE, COMMON_ERROR_NO_ERROR, HCI_CCEVT_RESET_RETPAR_LEN-1);
            break;

        // commands with params should not end up here
        default: ASSERT_ERR(0); break;
    }
}

/**
 ****************************************************************************************
 * @brief Function called in basic kernel message sending for Information Parameters
 *        commands with no parameters.
 *
 * @param ocf  OCF of command for identification.
 * Uses the hci_uart_env command header OGF and OCF components.
 *****************************************************************************************
 */
static void hci_cmd_dispatch_basic_info_par(uint8_t ocf)
{
    switch(ocf)
    {
        case OCF(RD_LOCAL_VER_INFO): hci_rd_local_ver_info_ccevt_pk(); break;

        //cmds with no params end up here
        default: ASSERT_ERR(0); break;
    }
}

/**
 ****************************************************************************************
 * @brief Function called in kernel message dispatch for Debug commands with parameters.
 *
 * @param[in]  ocf  COmmand OCF field for identification.
 * @param[in]  payl Pointer to receiver buffer payload
 *****************************************************************************************
 */
static void hci_cmd_dispatch_dbg(uint8_t ocf, uint8_t * payl)
{
    switch(ocf)
    {
        case OCF(DBG_RD_MEM)        : hci_dbg_rd_mem_cmd_unpk(payl);break;
        case OCF(DBG_WR_MEM)        : hci_dbg_wr_mem_cmd_unpk(payl);break;
        case OCF(DBG_DEL_PAR)       : hci_dbg_wr_param_cmd_unpk(payl);break;
        case OCF(DBG_FLASH_ID)      : hci_dbg_flash_identify_cmd_unpk(payl);break;
        case OCF(DBG_FLASH_ER)      : hci_dbg_flash_erase_cmd_unpk(payl);break;
        case OCF(DBG_FLASH_WR)      : hci_dbg_flash_write_cmd_unpk(payl);break;
        case OCF(DBG_FLASH_RD)      : hci_dbg_flash_read_cmd_unpk(payl);break;
        case OCF(DBG_RD_PAR)        : hci_dbg_rd_param_cmd_unpk(payl);break;
        case OCF(DBG_WR_PAR)        : hci_dbg_wr_param_cmd_unpk(payl);break;
        case OCF(DBG_PLF_RESET)     : hci_dbg_plf_reset_cmd_unpk(payl);break;
        
        //cmds with no params end up here
        default: ASSERT_ERR(0); break;
    }
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void hci_uart_init(void)
{
    //start uart reception
    hci_uart_read_start();
}


void hci_cmd_dispatch(uint8_t * payl )
{
    if (hci_uart_env.chdr.known_opcode)
    {
        switch(hci_uart_env.chdr.ogf)
        {
            case CNTLR_BB_OGF:
                hci_cmd_dispatch_basic_cntlr_bb(hci_uart_env.chdr.ocf);
                break;

            case INFO_PAR_OGF:
                hci_cmd_dispatch_basic_info_par(hci_uart_env.chdr.ocf);
                break;

            case VS_OGF:
                hci_cmd_dispatch_dbg(hci_uart_env.chdr.ocf, payl);
                break;
        }
    }
    //unknown opcode
    else
    {
        // Send command complete event
        hci_ccevt_pk(common_htobs((((uint16_t)(hci_uart_env.chdr.ogf) )<<10) | ((uint16_t)(hci_uart_env.chdr.ocf))),
                     COMMON_ERROR_UNKNOWN_HCI_COMMAND,
                     HCI_CCEVT_UNNKNOWN_RETPAR_LEN-1);
    }
}

void hci_push(uint8_t length)
{
    // Forward the message to the HCIH EIF for immediate transmission
    hci_uart_write(&hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET], length);
}



void hci_uart_write(uint8_t *buf, uint16_t len)
{
    //pack event type message (UART header)
    buf -= HCI_TRANSPORT_HDR_LEN;
    *buf = HCI_EVT_MSG_TYPE;

   // uart_write(buf, len + HCI_TRANSPORT_HDR_LEN);
}

void hci_uart_tx_done(int status)
{
    // Sanity check: Transmission should always work
    ASSERT_ERR(status == RWIP_EIF_STATUS_OK);
}


void hci_uart_rx_done(int status)
{
    //detect UART RX error and handle accordingly
    if (status == RWIP_EIF_STATUS_ERROR)
    {
        hci_uart_env.rx_state = HCI_STATE_RX_ERR;
    }

    //check HCI state to see what was received
    switch(hci_uart_env.rx_state)
    {
        /* RECEIVE MESSAGE TYPE STATE*/
        case HCI_STATE_RX_START:

            //Check message type correct
            ASSERT_ERR(hci_uart_env.curr_msg_type == HCI_CMD_MSG_TYPE);

            //change state to header reception
            hci_uart_read_hdr(HCI_CMD_HDR_LEN);

            break;
        /* RECEIVE MESSAGE TYPE STATE END*/


        /* RECEIVE HEADER STATE*/
        case HCI_STATE_RX_HDR:

            // Extract the command header components
            hci_uart_rx_cmd_hdr_extract();

            // Check if parameters have to be received
            if (hci_uart_env.chdr.parlen == 0)
            {
                // Send basic kernel message
                hci_cmd_dispatch(hci_uart_env.cmd_buf);

                //change hci rx state to message type reception
                hci_uart_read_start();
            }
            //Command has parameters so go to payload reception
            else
            {
                hci_uart_read_payl(hci_uart_env.chdr.parlen);
            }

            break;
        /* RECEIVE HEADER STATE END*/

        /* RECEIVE PAYLOAD STATE */
        case HCI_STATE_RX_PAYL:

            //call the right unpack handler
            hci_cmd_dispatch(hci_uart_env.cmd_buf);

            //change hci rx state to message type reception - common to all types
            hci_uart_read_start();

            break;
        /* RECEIVE PAYLOAD STATE END*/


        /* ERROR STATE */
        case HCI_STATE_RX_ERR:
            //start rx again
            hci_uart_init();

            break;
        /* ERROR STATE END*/

    }
    /* STATE SWITCH END */
}

/// @} HCI_UART
