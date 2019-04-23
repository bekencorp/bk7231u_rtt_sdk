/**
 ****************************************************************************************
 *
 * @file hci_msg.c
 *
 * @brief HCI module file containing the functions for command/event packing/unpacking
 * and communication with KE.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HCI_MSG
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
#include "hci_uart.h"
#include "hci_uart_msg.h"

#include "flash.h"

#if (PLF_NVDS)
#include "nvds.h"         // NVDS definitions
#endif // PLF_NVDS

/*
 * DEFINES
 ****************************************************************************************
 */
#define _8_Bit             8
#define _16_Bit            16
#define _32_Bit            32

/// PLATFORM RESET REASON: Reset and load FW from flash
#define PLATFORM_RESET_TO_FW    (0)
/// PLATFORM RESET REASON: Reset and stay in ROM code
#define PLATFORM_RESET_TO_ROM   (1)

/*
 * FUNCTION DEFINITIONS - Lower Layers: command complete events packing functions
 ****************************************************************************************
 */

void hci_ccevt_pk(uint16_t opcode, uint8_t status, uint8_t parlen)
{
    uint8_t *pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET];

    //pack event code
    *pk++ = HCI_CMD_CMPL_EVT_CODE;

    //pack command complete event parameter length  - extracted from event pack table
    // note: parlen is number of bytes of the parameters AFTER status
    *pk++ = HCI_CCEVT_HDR_PARLEN + 1 + parlen;

    //pack the number of h2c packets
    *pk++ = HCI_NB_CMD_PKTS;

    //pack opcode - always swap because it is defined MSB to LSB
    common_write16p(pk, opcode);
    pk += 2;

    // Pack status
    *pk = status;

    hci_push(HCI_CCEVT_BASIC_LEN + parlen);
}

void hci_rd_local_ver_info_ccevt_pk(void)
{
    //Must jump the transport code byte, the CCEVT header: 2 for event and 3 for normal nb packets+opcode+ 1 status
    uint8_t * pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN];

    // Put ROM HCI version
    *pk++ = 0xFF;

    // HCI revision u16
    common_write16p(pk, 0x0000);
    pk += 2;

    // LMP version
    *pk++ = 0xFF;

    // Manufacturer name
    common_write16p(pk, common_htobs(0x0060));
    pk += 2;

    // LMP subversion
    common_write16p(pk, 0x0000);
    pk += 2;

    // Send command complete event
    hci_ccevt_pk(HCI_RD_LOCAL_VER_INFO_CMD_OPCODE, COMMON_ERROR_NO_ERROR, HCI_CCEVT_RD_LOCAL_VER_INFO_RETPAR_LEN-1);
}

void hci_dbg_rd_mem_cmd_unpk( uint8_t * bufptr)
{
    uint8_t length = 0;
    uint32_t init_addr = 0;
    uint32_t value = 0;
    uint8_t type;
    uint8_t status;
    uint32_t i = 0;
    uint8_t * pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN];

    init_addr = common_read32p(bufptr);
    type = *(bufptr+4);
    length = *(bufptr+5);

    *pk++ = (uint8_t) length;

    // Check that data length is not null or too big before reading
    if (length == 0 || length > 128)
    {
        status = COMMON_ERROR_INVALID_HCI_PARAM;
    }
    else
    {
        // Check type of data to be read
        if (type == _8_Bit)
        {
            // Read bytes
            for (i = 0; i < length; i++)
            {
                // Read value at @ set in Param1+i
                pk[i] = *(volatile uint8_t *)(init_addr+i);
            }
        }
        else if (type == _16_Bit)
        {
            for (i = 0; i < length; i += 2)
            {
                // Read value at @ set in Param1+i
                value = (*(volatile uint16_t *)(init_addr+i));

                // store in the buffer
                pk[i]   = (uint8_t) value;
                value >>= 8;
                pk[i+1] = (uint8_t) value;
             }
        }
        else if (type == _32_Bit)
        {
            // Read 32 bit word
            for (i = 0; i < length; i += 4)
            {
                value = (*(volatile uint32_t *)(init_addr+i));

                // store in the buffer
                pk[i]   = (uint8_t) value;
                value >>= 8;
                pk[i+1] = (uint8_t) value;
                value >>= 8;
                pk[i+2] = (uint8_t) value;
                value >>= 8;
                pk[i+3] = (uint8_t) value;
            }
        }
        status = COMMON_ERROR_NO_ERROR;
    }

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_RD_MEM_CMD_OPCODE, status, length+1);
}

void hci_dbg_wr_mem_cmd_unpk(uint8_t * bufptr)
{
    uint32_t length = 0;
    uint32_t init_addr = 0;
    uint32_t value = 0;
    uint8_t *data_buf;
    uint8_t type;
    uint8_t status;
    uint32_t i = 0;

    init_addr = common_read32p(bufptr);
    type = *(bufptr+4);
    length = *(bufptr+5);
    data_buf = bufptr+6;

    // Check that data length is not null or too big before reading
    if (length == 0 || length > 128)
    {
        status = COMMON_ERROR_INVALID_HCI_PARAM;
    }
    else
    {
        // Check type of data to be written
        if (type == _8_Bit)
        {
            // Write bytes
            for (i = 0; i < length; i++)
            {
                // Set value type at @ Param1
                *(volatile uint8_t *)(init_addr+i) = data_buf[i];
            }
        }
        else if (type == _16_Bit)
        {
            // Write 16 bits word
            for (i = 0; i < length; i += 2)
            {
                // Set value type at @ Param1
                value = ((uint32_t)data_buf[i+1]);
                value <<= 8;
                value |= ((uint32_t)data_buf[i+0]);
                *(volatile uint16_t *)(init_addr+i) = value;
            }
        }
        else if(type == _32_Bit)
        {
            // Write 32 bit word
            for (i = 0; i < length; i += 4)
            {
                // Set value at @ Param1
                value  = ((uint32_t)data_buf[i+3]);
                value <<= 8;
                value |= ((uint32_t)data_buf[i+2]);
                value <<= 8;
                value |= ((uint32_t)data_buf[i+1]);
                value <<= 8;
                value |= ((uint32_t)data_buf[i+0]);
                *(volatile uint32_t *)(init_addr+i) = value;
            }
        }
        status = COMMON_ERROR_NO_ERROR;
    }

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_WR_MEM_CMD_OPCODE, status, HCI_CCEVT_DBG_WR_MEM_RETPAR_LEN-1);
}

void hci_dbg_del_param_cmd_unpk(uint8_t *bufptr)
{
    uint8_t status = COMMON_ERROR_NO_ERROR;
    uint16_t tag;

    // Extract tag and length from parameters
    tag = common_read16p(bufptr);

    #if PLF_NVDS && NVDS_READ_WRITE
    // Perform the requested action
    if(nvds_del(tag) != NVDS_OK)
    {
        status = COMMON_ERROR_HARDWARE_FAILURE;
    }
    #else //NVDS_READ_WRITE
    status = COMMON_ERROR_UNSUPPORTED;
    #endif //NVDS_READ_WRITE

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_DEL_PAR_CMD_OPCODE, status, HCI_CCEVT_DBG_DEL_PAR_RETPAR_LEN-1);
}

void hci_dbg_flash_identify_cmd_unpk(uint8_t *bufptr)
{
    uint8_t * pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN];

    // Perform the requested action
//    *pk = flash_identify();

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_FLASH_ID_CMD_OPCODE, COMMON_ERROR_NO_ERROR, HCI_CCEVT_DBG_FLASH_ID_RETPAR_LEN-1);
}

void hci_dbg_flash_erase_cmd_unpk(uint8_t *bufptr)
{
    uint8_t flashtype;
    uint8_t status;
    uint32_t startoffset;
    uint32_t size;

    // Extract parameters from the command buffer
    flashtype = *bufptr++;
    startoffset = common_read32p(bufptr);
    bufptr += 4;
    size = common_read32p(bufptr);

    // Perform the requested action
//    status = flash_erase(flashtype, startoffset, size);

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_FLASH_ER_CMD_OPCODE, status, HCI_CCEVT_DBG_FLASH_ER_RETPAR_LEN-1);
}

void hci_dbg_flash_write_cmd_unpk(uint8_t *bufptr)
{
    uint8_t status;
    uint8_t  flashtype;
    uint32_t offset;
    uint8_t length;

    // Extract parameters from the command buffer
    flashtype = *bufptr++;
    offset = common_read32p(bufptr);
    bufptr += 4;
    length = *bufptr++;

    // Perform the requested action
//    status = flash_write(flashtype, offset, length, bufptr);

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_FLASH_WR_CMD_OPCODE, status, HCI_CCEVT_DBG_FLASH_WR_RETPAR_LEN-1);
}

void hci_dbg_flash_read_cmd_unpk(uint8_t *bufptr)
{
    uint8_t status;
    uint8_t  flashtype;
    uint32_t offset;
    uint8_t length;
    uint8_t *pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN];

    // Extract parameters from the command buffer
    flashtype = *bufptr++;
    offset = common_read32p(bufptr);
    bufptr += 4;
    length = *bufptr;

    // Perform the requested action
//    status = flash_read(flashtype, offset, length, pk+1);

    *pk = length;

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_FLASH_RD_CMD_OPCODE, status, length+1);
}

void hci_dbg_rd_param_cmd_unpk(uint8_t *bufptr)
{
    uint8_t status = COMMON_ERROR_NO_ERROR;
    #if (PLF_NVDS)
    uint16_t tag;
    uint8_t length;
    uint8_t *pk = &hci_uart_env.evt_buf[HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN];

    // Extract tag from parameters
    tag = common_read16p(bufptr);

    // Initialize size with maximum space in buffer
    length = sizeof(hci_uart_env.evt_buf) - (HCI_EVT_HDR_OFFSET + HCI_CCEVT_BASIC_LEN);
    // Perform the requested action
    if(nvds_get(tag, &length, pk+1) != NVDS_OK)
    {
        *pk = 0;
    }
    else
    {
        *pk = length;
    }

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_RD_PAR_CMD_OPCODE, status, length+1);
    #endif // PLF_NVDS
}

void hci_dbg_wr_param_cmd_unpk(uint8_t *bufptr)
{
    uint8_t status = COMMON_ERROR_NO_ERROR;
    #if (PLF_NVDS)
    uint16_t tag;
    uint8_t length;

    // Extract tag and length from parameters
    tag = common_read16p(bufptr);
    length = *(bufptr+2);

    #if NVDS_READ_WRITE
    // Perform the requested action
    if(nvds_put(tag, length, bufptr+3) != NVDS_OK)
    {
        status = COMMON_ERROR_HARDWARE_FAILURE;
    }
    #else //NVDS_READ_WRITE
    status = COMMON_ERROR_UNSUPPORTED;
    #endif //NVDS_READ_WRITE

    // Send command complete event
    hci_ccevt_pk(HCI_DBG_WR_PAR_CMD_OPCODE, status, HCI_CCEVT_DBG_WR_PAR_RETPAR_LEN-1);
    #endif // PLF_NVDS
}

///HCI Debug Platform Reset Command unpacking function for DBG.
void hci_dbg_plf_reset_cmd_unpk(uint8_t * bufptr)
{
    uint8_t  reset_type = *(bufptr);
    uint32_t error = RESET_NO_ERROR;

    switch(reset_type)
    {
        case PLATFORM_RESET_TO_FW : error = RESET_AND_LOAD_FW; break;
        case PLATFORM_RESET_TO_ROM : error = RESET_TO_ROM; break;
        default: break;
    }

    if(error != RESET_NO_ERROR)
    {
        // perform platform reset
        platform_reset(error);
    }

    // If platform reset not performed, it means that platform reset type was not supported ==> invalid param
    // Succeed Command Complete event is sent in rom code startup.
    hci_ccevt_pk(HCI_DBG_PLF_RESET_CMD_OPCODE, COMMON_ERROR_INVALID_HCI_PARAM, HCI_CCEVT_DBG_PLF_RESET_RETPAR_LEN -1);
}

/// @} HCI_MSG
