/*************************************************************
 * File Name:     BK_HCI_Protocol.c
 * Author:        GuWenFu
 * Created Date:  @2016-04-14
 * Description:   test code of HCI protocol
 *
 * History:       2016-04-14 gwf    create this file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  "BK3435_reg.h"
#include "uart.h"
//#include "driver_uart0.h"


#include "BK_HCI_Protocol.h"



//#define uart_rx_done        uart0_rx_done
//#define uart_rx_index       uart0_rx_index
//#define uart_rx_buf         uart0_rx_buf
//#define uart_tx_buf         uart0_tx_buf
//#define uart_send           uart0_send


HCI_COMMAND_PACKET *pHCIrxBuf = (HCI_COMMAND_PACKET *)(&uart_rx_buf[0]);
HCI_EVENT_PACKET   *pHCItxBuf = (HCI_EVENT_PACKET *)(&uart_tx_buf[0]);

//static uint32 test_tx_payload=0x0;
//static uint32 test_pkt_num=0x0;


void clear_uart_buffer(void)
{

    uart_rx_index = 0;
    uart_rx_done = 0;
    memset(uart_rx_buf, 0, sizeof(uart_rx_buf)); /**< Clear the RX buffer */
    memset(uart_tx_buf, 0, sizeof(uart_tx_buf)); /**< Clear the TX buffer */
}

/**
 * This function sends the PDU checking after each byte if it was the last
 */
void TRAhcit_UART_Tx(void) 
{
	//  UART_PRINTF("TRAhcit_UART_Tx\r\n");
    u32 tx_len       = HCI_EVENT_HEAD_LENGTH+pHCItxBuf->total;
    pHCItxBuf->code  = TRA_HCIT_EVENT;
    pHCItxBuf->event = HCI_COMMAND_COMPLETE_EVENT;
    uart_send(uart_tx_buf, tx_len);
}

/**
 * This function is called by the when a full HCI packet has been decoded
 */

#define REG_XVR_BASE_ADDR               APB_XVER_BASE

extern volatile uint32_t XVR_ANALOG_REG_BAK[16] ;
void TRAhcit_UART_Rx(void)
{
    if ((uart_rx_done != 1) || (uart_rx_index == 0))
        return;

	//loop_mode:
   	//  UART_PRINTF("code=%x, ogf=%x, ocf=%x, count=%x\r\n",pHCIrxBuf->code,pHCIrxBuf->opcode.ogf,pHCIrxBuf->opcode.ocf,uart_rx_index);
    if ((pHCIrxBuf->code != TRA_HCIT_COMMAND)
        || (pHCIrxBuf->opcode.ogf != VENDOR_SPECIFIC_DEBUG_OGF)
        || (pHCIrxBuf->opcode.ocf != BEKEN_OCF)
        || (uart_rx_index != (HCI_COMMAND_HEAD_LENGTH+pHCIrxBuf->total))
       )
        goto ret;

    /* printf("cmd=%x\r\n",pHCIrxBuf->cmd); */
    switch (pHCIrxBuf->cmd) {
    case LINK_CHECK_CMD:		// 01 e0 fc 01 00

        pHCItxBuf->total = uart_rx_index;

        memcpy(pHCItxBuf->param, uart_rx_buf, pHCItxBuf->total);
        break;

    case REGISTER_WRITE_CMD: {   // 01 e0 fc 09 01 00 28 80 00 68 00 00 00
        signed   long reg_index;
        REGISTER_PARAM *rx_param        = (REGISTER_PARAM *)pHCIrxBuf->param;
        REGISTER_PARAM *tx_param        = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];

        pHCItxBuf->total                = uart_rx_index-1;

        memcpy(pHCItxBuf->param, uart_rx_buf, 3);
        pHCItxBuf->param[3]             = pHCIrxBuf->cmd;
        tx_param->addr                  = rx_param->addr;
        tx_param->value                 = rx_param->value;
        *(volatile u32 *)rx_param->addr = rx_param->value;
        reg_index                       = (rx_param->addr-REG_XVR_BASE_ADDR)/4;
        if ((reg_index>=0) && (reg_index<=0x0f))
        {
            XVR_ANALOG_REG_BAK[reg_index] = rx_param->value;
        }
        //printf("addr=%x,val=%x\r\n",rx_param->addr, rx_param->value); 
        break;
    }

    case REGISTER_READ_CMD: {   // 01e0fc050300288000
        s32 reg_index;
        // 01 e0 fc 05 03 00 28 80 00
        REGISTER_PARAM *rx_param = (REGISTER_PARAM *)pHCIrxBuf->param;
        REGISTER_PARAM *tx_param = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];


        pHCItxBuf->total         = uart_rx_index+3;

        memcpy(pHCItxBuf->param, uart_rx_buf, 3);
        pHCItxBuf->param[3]      = pHCIrxBuf->cmd;
        tx_param->addr           = rx_param->addr;
        reg_index                = (rx_param->addr-REG_XVR_BASE_ADDR)/4;
        if ((reg_index>=0) && (reg_index<=0x0f))
        {
            tx_param->value      = XVR_ANALOG_REG_BAK[reg_index];
        }
        else
        {
            tx_param->value      = *(volatile u32 *)rx_param->addr;
        }
        //printf("addr=%x,val=%x\r\n",tx_param->addr, tx_param->value); 
        break;
    }

 
    default:
        goto ret;
    }

    TRAhcit_UART_Tx();

ret:
    clear_uart_buffer();
}

