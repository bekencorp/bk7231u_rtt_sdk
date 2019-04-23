
#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>        // standard definition
#include "BK3435_reg.h"
#include "i2c.h"
#include "BK_HCI_Protocol.h"
#include "uart.h" 


#define NUMBER_ROUND_UP(a,b)        ((a) / (b) + (((a) % (b)) ? 1 : 0))
#define NUMBER_ROUND_DOWN(a,b)      ((a) / (b))


#define I2C_DEFAULT_CLK            16000000
#define I2C_BAUD_100KHZ            100000
#define I2C_BAUD_400KHZ            400000
#define I2C_DEFAULT_BAUD           I2C_BAUD_400KHZ
#define I2C_CLK_DIVID_SET          (NUMBER_ROUND_UP(NUMBER_ROUND_UP(I2C_DEFAULT_CLK, I2C_DEFAULT_BAUD) - 6, 3) - 1)

#define I2C_DATA_BUFFER_LEN		   8

#define I2C_MSG_WORK_MODE_RW_BIT        (1<<0)      /* 0:write,  1:read */
#define I2C_MSG_WORK_MODE_MS_BIT        (1<<1)      /* 0:master, 1:slave */
#define I2C_MSG_WORK_MODE_AL_BIT        (1<<2)      /* 0:7bit address, 1:10bit address */
#define I2C_MSG_WORK_MODE_IA_BIT        (1<<3)      /* 0:without inner address, 1: with inner address */


//----------------------------------------------
// I2C0 data typedef 
//----------------------------------------------
typedef  struct
{
    unsigned char       data[I2C_DATA_BUFFER_LEN];  // data buffer
    unsigned short      send_addr;			// send address value, only for Master
    unsigned short      slave_mode_addr;    // slave mode address, only for Slaver
    unsigned char      current_data_cnt;   // current data count
    unsigned char	all_data_cnt;       // TX or RX Data Bytes
    unsigned char	inner_addr;         // inner address, only for master write/read
    unsigned char	work_mode;			// work mode
                                            // RW(bit 0):  0:write,  1:read
                                            // MS(bit 1):  0:master, 1:slave
                                            // AL(bit 2):  0:7bit address, 1:10bit address
                                            // IA(bit 3):  0:without inner address, 1: with inner address
                                            // reserved(bit [4:7]):  reserved

/* addr_flg: Address Byte Flag for Master
 * bit[1:0]: 10bit/7bit device address flag
 *          00: first byte need to be tx
 *          01: first byte has been tx , second byte need to be tx (not supply in 7bit mode)
 *          10: second byte has been tx, after restart, first byte need to be tx again
 *          11: after restart, first byte has been tx again
 * bit[3]:   inner address flag
 *          0:  inner address need to be tx
 *          1:  inner address has been tx
 * bit[4]:   all address (device address and inner address) tx over flag
 *          0:  all address tx not over
 *          1:  all address has been tx, data tx begin
 * bit[7:5]: reserve
 */
	unsigned char	addr_flg;
	unsigned char	trans_done;			// I2C0 Transfer Finish Flag: 0:i2c Transfer havn't finished
                                            //                  1:i2c Transfer has finished
	unsigned char	ack_check;			// 0: Don't Care ACK, 1: Care ACK
    	unsigned char errno;              // error number
}  I2C_MSG;


static volatile I2C_MSG    *i2c_msg = NULL;


static STATUS i2c_msg_init(I2C_MSG *p_i2c_msg);
static void i2c_send_start(void);
static ASK is_i2c_busy(void);
static void i2c_msg_reset(void);
static I2C_MSG * i2c_get_last_msg(void);


/*
static void PrintI2CurrentMsg(I2C_MSG *p_i2c_msg)
{
    int i;

    UART_PRINTF("PrintI2C0CurrentMsg:\r\n");
    if (p_i2c_msg == NULL)
    {
        return ;
    }
    for (i=0; i<p_i2c_msg->all_data_cnt; i++)
    {
        UART_PRINTF("p_i2c_msg->data[%d] = 0x%x\r\n", i, p_i2c_msg->data[i]);
    }
    UART_PRINTF("p_i2c_msg->current_data_cnt = %d\r\n", p_i2c_msg->current_data_cnt);
    UART_PRINTF("p_i2c_msg->all_data_cnt = %d\r\n", p_i2c_msg->all_data_cnt);
    UART_PRINTF("p_i2c_msg->send_addr = 0x%x\r\n", p_i2c_msg->send_addr);
    UART_PRINTF("p_i2c_msg->inner_addr = 0x%x\r\n", p_i2c_msg->inner_addr);
    UART_PRINTF("p_i2c_msg->work_mode = 0x%x\r\n", p_i2c_msg->work_mode);
    UART_PRINTF("p_i2c_msg->addr_flg = 0x%x\r\n", p_i2c_msg->addr_flg);
    UART_PRINTF("p_i2c_msg->trans_done = %d\r\n", p_i2c_msg->trans_done);
    UART_PRINTF("p_i2c_msg->ack_check = %d\r\n", p_i2c_msg->ack_check);
    UART_PRINTF("p_i2c_msg->errno = %d\r\n", p_i2c_msg->errno);
}

*/

STATUS i2c_write(uint8_t devAddr, uint8_t addr, uint8_t*buf, uint8_t size)
{
	I2C_MSG p_i2c_msg;
	p_i2c_msg.work_mode    = 0
				& (~I2C_MSG_WORK_MODE_RW_BIT)  // write 
				& (~I2C_MSG_WORK_MODE_MS_BIT)  // master
				& (~I2C_MSG_WORK_MODE_AL_BIT)  // 7bit address
				| ( I2C_MSG_WORK_MODE_IA_BIT); // without inner address
	p_i2c_msg.send_addr    = devAddr;                      // Destiny slave address
	p_i2c_msg.inner_addr   = addr;
	p_i2c_msg.all_data_cnt = size;                         // 8 byte at most
	p_i2c_msg.addr_flg     = 0;
	p_i2c_msg.trans_done   = 0;
	p_i2c_msg.ack_check    = 1;
	p_i2c_msg.current_data_cnt = 0;
	p_i2c_msg.errno        = 0;

	memset(p_i2c_msg.data, 0, I2C_DATA_BUFFER_LEN);
	memcpy(p_i2c_msg.data, buf, size);

	if (i2c_msg_init(&p_i2c_msg) != OK)
	{
		return ERROR;
	}
	i2c_send_start();
	
	while (is_i2c_busy() == YES);       // wait until i2c0 free

	//PrintI2CurrentMsg(i2c_get_last_msg());

	i2c_msg_reset();

	return OK;
}

STATUS i2c_read(uint8_t devAddr, uint8_t addr, uint8_t*buf, uint8_t size)
{
	I2C_MSG p_i2c_msg;

	p_i2c_msg.work_mode    = 0
	                    | ( I2C_MSG_WORK_MODE_RW_BIT)  // read
	                    & (~I2C_MSG_WORK_MODE_MS_BIT)  // master
	                    & (~I2C_MSG_WORK_MODE_AL_BIT)  // 7bit address
	                    | ( I2C_MSG_WORK_MODE_IA_BIT); // without inner address
	p_i2c_msg.send_addr    = devAddr;                      // Destiny slave address
	p_i2c_msg.inner_addr   = addr;
	p_i2c_msg.all_data_cnt = size;
	p_i2c_msg.addr_flg     = 0;
	p_i2c_msg.trans_done   = 0;
	p_i2c_msg.ack_check    = 1;  
	p_i2c_msg.current_data_cnt = 0;
	p_i2c_msg.errno        = 0;

	memset(p_i2c_msg.data, 0, I2C_DATA_BUFFER_LEN);
	if (i2c_msg_init(&p_i2c_msg) != OK)
	{
		return ERROR;
	}
	i2c_send_start();

	while (is_i2c_busy() == YES);       // wait until i2c0 free
	
	if(i2c_get_last_msg() != NULL)  
	{
		//PrintI2CurrentMsg(i2c_get_last_msg());
		memcpy(buf, i2c_get_last_msg()->data, i2c_get_last_msg()->all_data_cnt);

		return OK;
	}

	return ERROR;
}



/*************************************************************
 * i2c_get_last_msg
 * Description: get i2c last message point, so that you can check errno
 *              or deal with received date. if you want to free memory,
 *              call function i2c_msg_reset()
 * Parameters:  none
 * return:      i2c_message *: the i2c last message point
 * error:       none
 */
static I2C_MSG * i2c_get_last_msg(void)
{
    if (i2c_msg != NULL)
    {
        if (i2c_msg->trans_done != 1)
        {
            return NULL;
        }
    }
    return (I2C_MSG *)i2c_msg;
}

/*************************************************************
 * i2c_msg_reset
 * Description: reset i2c message point, be caution, you must make sure
 *              the last i2c message Tx/Rx has finished
 * Parameters:  none
 * return:      none
 * error:       none
 */
static void i2c_msg_reset(void)
{
    if (i2c_msg != NULL)
    {
        i2c_msg = NULL;
    }
}


/*************************************************************
 * i2c_msg_init
 * Description: initialize i2c message, prepare for Tx/Rx.
 *              call function i2c_send_start() to start Tx/Rx
 * Parameters:  i2c_message: message description
 * return:      OK: message is good
 *              ERROR: message is wrong
 * error:       none
 */
static STATUS i2c_msg_init(I2C_MSG *p_i2c_msg)
{
    if ((i2c_msg != NULL) && (i2c_msg->trans_done != 1))
    {
        return ERROR;
    }

    if (p_i2c_msg == NULL)
    {
        return ERROR;
    }

    if (p_i2c_msg->data == NULL)
    {
        return ERROR;
    }

    if (p_i2c_msg->current_data_cnt != 0)
    {
        return ERROR;
    }

    if (p_i2c_msg->all_data_cnt == 0)
    {
        return ERROR;
    }

    if (p_i2c_msg->addr_flg != 0)
    {
        return ERROR;
    }

    if (p_i2c_msg->trans_done != 0)
    {
        return ERROR;
    }

	p_i2c_msg->work_mode &= 0x0F;
	p_i2c_msg->ack_check &= 0x01;
	p_i2c_msg->errno      = 0x00;

    i2c_msg_reset();
	
    i2c_msg = p_i2c_msg;

    return OK;
}


void i2c_init(uint32_t slaveAddr, uint32_t baudRate)
{
	uint32_t freq_div;
	
	if (baudRate == 0)
	{
		freq_div = I2C_CLK_DIVID_SET;
	}
	else
	{
		freq_div = NUMBER_ROUND_UP(NUMBER_ROUND_UP(I2C_DEFAULT_CLK, baudRate) - 6, 3) - 1;
	}

	if (REG_APB4_I2C_CN & I2C_CONFIG_I2C_ENABLE_MASK)
    	{
        	REG_APB4_I2C_STAT |= 0x200;
        	REG_APB4_I2C_CN = 0;

        	ICU_I2C_CLK_PWD_SET();
		REG_AHB0_ICU_INT_ENABLE &= ~(0x1 << 7);
    	}
	ICU_I2C_CLK_PWD_CLEAR();

	// Enable GPIO P0.2, P0.3 peripheral function for I2C
    	REG_APB5_GPIOA_CFG = (REG_APB5_GPIOA_CFG & (~((0x1<<2) | (0x1<<3) | (0x1<<26) | (0x1<<27))))
                        | ((0x1<<10) | (0x1<<11) | (0x1<<18) | (0x1<<19));
    	REG_APB5_GPIOA_DATA   = REG_APB5_GPIOA_DATA & (~((0x1<<18) | (0x1<<19)));

   	 //Close JTAG to release GPIO to normal function
    	CLOSE_JTAG_MODE();

    	REG_APB4_I2C_STAT = 0x00000040;       // 0100 0000; RXINT_MODE = 0x01, slvstop_stre_scl_en = 0x0
    	REG_APB4_I2C_CN =  (I2C_CONFIG_I2C_ENABLE_SET)
                     | (I2C_CONFIG_INH_CLEAR)
                     | (I2C_CONFIG_SMBFTE_SET)
                     | (I2C_CONFIG_SMBTOE_SET)
                     | (I2C_CONFIG_CLOCK_SEL_FREQ_DIV)
                     | ((slaveAddr & 0x03FFUL) << I2C_CONFIG_SLAVE_ADDR_POSI)
                     | ((freq_div & 0x03FFUL) << I2C_CONFIG_FREQ_DIV_POSI)  
                     | (0x04UL     << I2C_CONFIG_SCL_CR_POSI)
                     | (0x03UL     << I2C_CONFIG_IDLE_CR_POSI);

	REG_AHB0_ICU_INT_ENABLE |= (0x1 << 7);

	i2c_msg_reset();
	
}


/*************************************************************
 * i2c_send_addr
 * Description: send i2c slave address, only support in master mode
 * Parameters:  none
 * return:      none
 * error:       none
 */
static void i2c_send_addr(void)
{
    unsigned char   tx_data;

 	if ((i2c_msg->work_mode & I2C_MSG_WORK_MODE_AL_BIT) == 0) // 7bit address
    {
        tx_data = 0;
        tx_data |= (i2c_msg->send_addr << 1);
        tx_data &= ~0x01;
        if (i2c_msg->work_mode == 0x01)      // master read,  7bit address, without inner address
        {
            tx_data |= 0x01;
        }
        REG_APB4_I2C_DAT = tx_data;

        i2c_msg->addr_flg ++;
        if (i2c_msg->work_mode == 0x01       // master read,  7bit address, without inner address
         || i2c_msg->work_mode == 0x00)      // master write, 7bit address, without inner address
        {
            i2c_msg->addr_flg |= 0x13;       // all address tx over
        }
    }
    else //10bit address
    {
        tx_data = 0xF0;         // tx the first address byte with a WRITE in RW bit
        tx_data |= (i2c_msg->send_addr >> 7) & 0x06;
        REG_APB4_I2C_DAT = tx_data;
        i2c_msg->addr_flg ++;
    }
}


/*************************************************************
 * i2c_send_start
 * Description: start i2c Tx/Rx, only support in master mode
 * Parameters:  none
 * return:      none
 * error:       none
 */
static void i2c_send_start(void)
{
    unsigned int cfg_data;
    unsigned int int_mode;

    if (i2c_msg == NULL)
    {
        return;
    }

    if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_MS_BIT)      // slave mode
    {
        return;
    }

    if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_RW_BIT)      // read mode
    {
        if (i2c_msg->all_data_cnt > 12)
        {
            int_mode = 0x00;
        }
        else if (i2c_msg->all_data_cnt > 8)
        {
            int_mode = 0x01;
        }
        else if (i2c_msg->all_data_cnt > 4)
        {
            int_mode = 0x02;
        }
        else
        {
            int_mode = 0x03;
        }
    }
    else      // write mode
    {
        if (i2c_msg->all_data_cnt < 4 || (i2c_msg->work_mode & I2C_MSG_WORK_MODE_MS_BIT))
        {
            int_mode = 0x00;
        }
        else
        {
            int_mode = 0x01;
        }
    }

    i2c_send_addr();  //write address into REG_I2C_DATA

    cfg_data = REG_APB4_I2C_STAT & 0x0000FFFBUL;
    cfg_data |= (int_mode << 6);
    cfg_data |= I2C_STATUS_START_SET;
    REG_APB4_I2C_STAT = cfg_data;
}


/*************************************************************
 * is_i2c_busy
 * Description: ask if i2c interface is busy
 * Parameters:  none
 * return:      YES: i2c interface is busy
 *              NO:  i2c interface is free
 * error:       none
 */
static ASK is_i2c_busy(void)
{
    if (REG_APB4_I2C_STAT & (0x01<<15))
    {
        return YES;
    }

    if ((i2c_msg != NULL) && (i2c_msg->trans_done == 0))
    {
        return YES;
    }

    return NO;
}


/*************************************************************
 * i2c_isr
 * Description: i2c0 interrupt handler
 * Parameters:  none
 * return:      none
 * error:       none
 */
void i2c_isr(void)
{
    unsigned long   i2c_config, i2c_stat;
    unsigned long   work_mode, ack, sta, sto, si;
    volatile unsigned char  fifo_empty_num=0;
    volatile unsigned char  data_num=0;
    unsigned char   i;
    unsigned char   ucTemp;
    unsigned char   remain_data_cnt;

    i2c_stat = REG_APB4_I2C_STAT;
    si = i2c_stat & (0x1 << 0);

    if (!si)     // not SMBUS/I2C Interrupt
    {
        if (i2c_stat & 0x02)        // SCL low level over time
        {
            i2c_init(I2C_DEFAULT_SLAVE_ADDRESS, 0);
        }

        if (i2c_stat & 0x08)        // ARB lost
        {
            REG_APB4_I2C_STAT = i2c_stat & ~0x08;      // clear ARB
        }
        return;
    }

    if (i2c_msg == NULL)
    {
        REG_APB4_I2C_STAT = (i2c_stat | 0x0200) & ~0x01; // send stop, clear si
        return;
    }

    i2c_config = REG_APB4_I2C_CN;                      // fix bug
    REG_APB4_I2C_CN = i2c_config & (~I2C_CONFIG_CLOCK_SEL_MASK);

    ack = i2c_stat & 0x0100;
    sto = i2c_stat & 0x0200;
    sta = i2c_stat & 0x0400;
    work_mode = i2c_msg->work_mode & 0x03;
    remain_data_cnt = i2c_msg->all_data_cnt - i2c_msg->current_data_cnt;
    switch (work_mode)
    {
        case 0x00:      // master write
        {
            i2c_stat &= ~0x0400;

            if (i2c_msg->ack_check && !ack)  // nack
            {
                i2c_stat |= 0x0200;       // send stop
                i2c_msg->trans_done = 1;
                break;
            }

            ucTemp = i2c_msg->addr_flg;
            if (ucTemp & 0x10)          // all address bytes has been tx, now tx data
            {
                if (remain_data_cnt == 0)   // all data bytes has been tx, now send stop
                {
                    i2c_stat |= 0x0200;     // send stop
                    i2c_msg->trans_done = 1;
                    break;
                }
                switch (i2c_stat & 0x00C0)
                {
                    case 0x0000:   fifo_empty_num = 16;  break;
                    case 0x0040:   fifo_empty_num = 12;  break;
                    case 0x0080:   fifo_empty_num = 8;   break;
                    case 0x00C0:   fifo_empty_num = 4;   break;
                    default    :   fifo_empty_num = 0;
                                   i2c_msg->errno = 0x0002;      // it's impossible
                                   break;
                }
                if (remain_data_cnt < fifo_empty_num)
                {
                    data_num = remain_data_cnt;
                }
                else
                {
                    data_num = fifo_empty_num;
                }

                for (i = 0; i < data_num; i ++)
                {
                    REG_APB4_I2C_DAT = i2c_msg->data[i2c_msg->current_data_cnt];
                    i2c_msg->current_data_cnt ++;
                    remain_data_cnt --;
                }

                if (remain_data_cnt < fifo_empty_num)
                {
                    i2c_stat &= ~0x04C0;        // clear start, set int_mode
                }
                break;
            }

            if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_AL_BIT)      // 10bit address
            {
                if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_IA_BIT)  // with inner address
                {
                    if ((ucTemp & 0x08) == 0)       // inner address need to be tx
                    {
                        if ((ucTemp & 0x03) == 0x00)        // the first address byte should have been tx already
                        {
                            i2c_msg->errno = 0x0001;
                            break;
                        }
                        else if ((ucTemp & 0x03) == 0x01)   // tx the second address byte
                        {
                            REG_APB4_I2C_DAT = ((unsigned char)i2c_msg->send_addr & 0x00ff);
                            i2c_msg->addr_flg ++;
                        }
                        else
                        {
                            REG_APB4_I2C_DAT = i2c_msg->inner_addr;
                            i2c_msg->addr_flg |= 0x1B;
                        }
                    }
                    else   // inner address has been tx
                    {
                        i2c_msg->addr_flg |= 0x13;   // it's impossible
                    }
                }
                else        // without inner address
                {
                    if ((ucTemp & 0x03) == 0x00)        // the first address byte should have been tx already
                    {
                        i2c_msg->errno = 0x0001;
                        break;
                    }
                    else if ((ucTemp & 0x03) == 0x01)   // tx the second address byte
                    {
                        REG_APB4_I2C_DAT = ((unsigned char)i2c_msg->send_addr & 0x00ff);
                        i2c_msg->addr_flg |= 0x13;
                    }
                    else
                    {
                        i2c_msg->addr_flg |= 0x13;  // it's impossible
                    }
                }
            }
            else        // 7bit address
            {
                if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_IA_BIT)  // with inner address
                {
                    if ((ucTemp & 0x08) == 0)       // inner address need to be tx
                    {
                        REG_APB4_I2C_DAT = i2c_msg->inner_addr;
                        i2c_msg->addr_flg |= 0x13;
                    }
                    else                            // inner address has been tx
                    {
                        i2c_msg->addr_flg |= 0x13;  // it's impossible
                    }
                }
                else  // without inner address
                {
                    i2c_msg->addr_flg |= 0x13;      // it's impossible
                }
            }
            break;
        }

        case 0x01:      // master read
        {
            i2c_stat &= ~0x0400;
            if (sta && i2c_msg->ack_check && !ack) // when tx address, we need ACK
            {
                i2c_stat = i2c_stat | 0x0200;
                i2c_msg->trans_done = 1;
                break;
            }

            ucTemp = i2c_msg->addr_flg;
            if (ucTemp & 0x10)          // all address has been tx, now rx data
            {
                if (sta)
                {
                    i2c_stat = i2c_stat | 0x100;        // send ACK
                    break;
                }

                switch (i2c_stat & 0x00C0)
                {
                    case 0x0000:   data_num = 12;  break;
                    case 0x0040:   data_num = 8;   break;
                    case 0x0080:   data_num = 4;   break;
                    case 0x00C0:   data_num = 1;   break;
                    default    :   data_num = 0;
                                   i2c_msg->errno = 0x0002;      // it's impossible
                                   break;
                }

                for (i = 0; i < data_num; i ++)
                {
                    i2c_msg->data[i2c_msg->current_data_cnt] = REG_APB4_I2C_DAT;
                    i2c_msg->current_data_cnt ++;
                    remain_data_cnt --;
                }

                if (remain_data_cnt == 0)
                {
                    i2c_stat = (i2c_stat & (~0x0500)) | 0x0200;     // send NACK and STOP
                    i2c_msg->trans_done = 1;
                }
                else if (remain_data_cnt < data_num)
                {
                    i2c_stat = i2c_stat | 0x01C0;     // send ACK, set int_mode
                }
                else
                {
                    i2c_stat = i2c_stat | 0x0100;     // send ACK
                }
                break;
            }

            if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_AL_BIT)      // 10bit address
            {
                if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_IA_BIT)  // with inner address
                {
                    if ((ucTemp & 0x08) == 0)       // inner address need to be tx
                    {
                        if ((ucTemp & 0x03) == 0x00)        // the first address byte should have been tx already
                        {
                            i2c_msg->errno = 0x0001;
                            break;
                        }
                        else if ((ucTemp & 0x03) == 0x01)   // tx the second address byte
                        {
                            REG_APB4_I2C_DAT = ((unsigned char)i2c_msg->send_addr & 0x00ff);
                            i2c_msg->addr_flg ++;
                        }
                        else
                        {
                            REG_APB4_I2C_DAT = i2c_msg->inner_addr;
                            i2c_msg->addr_flg |= 0x08;
                        }
                    }
                    else                            // inner address has been tx
                    {
                        if ((ucTemp & 0x03) == 0x02)        // tx the first address byte with a READ in RW bit
                        {
                            i2c_stat |= 0x400;
                            REG_APB4_I2C_DAT = ((i2c_msg->send_addr >> 7) & 0x06) | 0xF1;
                            i2c_msg->addr_flg |= 0x13;
                        }
                        else
                        {
                            i2c_msg->errno = 0x0002;      // it's impossible to get here
                            break;
                        }
                    }
                }
                else        // without inner address
                {
                    if ((ucTemp & 0x03) == 0x00)        // the first address byte should have been tx already
                    {
                        i2c_msg->errno = 0x0001;
                        break;
                    }
                    else if ((ucTemp & 0x03) == 0x01)   // tx the second address byte
                    {
                        REG_APB4_I2C_DAT = ((unsigned char)i2c_msg->send_addr & 0x00ff);
                        i2c_msg->addr_flg ++;
                    }
                    else if ((ucTemp & 0x03) == 0x02)   // tx the first address byte with a READ in RW bit
                    {
                        i2c_stat |= 0x400;
                        REG_APB4_I2C_DAT = ((i2c_msg->send_addr >> 7) & 0x06) | 0xF1;
                        i2c_msg->addr_flg |= 0x13;
                    }
                    else
                    {
                        i2c_msg->errno = 0x0002;      // it's impossible to get here
                        break;
                    }
                }
            }
            else        // 7bit address
            {
                if (i2c_msg->work_mode & I2C_MSG_WORK_MODE_IA_BIT)  // with inner address
                {
                    if ((ucTemp & 0x08) == 0)       // inner address need to be tx
                    {
                        REG_APB4_I2C_DAT = i2c_msg->inner_addr;
                        i2c_msg->addr_flg = (i2c_msg->addr_flg & ~0x0B) | 0x0A;
                    }
                    else                            // inner address has been tx
                    {
                        i2c_stat |= 0x400;
                        REG_APB4_I2C_DAT = (i2c_msg->send_addr<<1) | 0x01;
                        i2c_msg->addr_flg |= 0x13;
                    }
                }
                else        // without inner address
                {
                    i2c_msg->addr_flg |= 0x13;      // it's impossible
                }
            }
            break;
        }

        case 0x02:      // slave write
        {
            if (sto)    // detect a STOP
            {
                i2c_msg->trans_done = 1;
                break;
            }
            if (i2c_stat & 0x0800)      // match address byte
            {
                i2c_stat |= 0x0100;     // send ACK for address byte
            }

            if ((i2c_stat & 0x2000) == 0)      // read mode
            {
                break;
            }

            if (i2c_stat & 0x0100)      // detect an ACK
            {
                REG_APB4_I2C_DAT = i2c_msg->data[i2c_msg->current_data_cnt];     // send data
                i2c_msg->current_data_cnt ++;

                remain_data_cnt --;
                if (remain_data_cnt == 0)
                {
                    // TODO
                }
            }
            break;
        }

        case 0x03:      // slave read
        {
            if (sto)    // detect a STOP
            {
                i2c_msg->trans_done = 1;
                break;
            }
            if (i2c_stat & 0x0800)      // match address byte
            {
                i2c_stat |= 0x0100;     // send ACK
                break;
            }

            switch (i2c_stat & 0x00C0)
            {
                case 0x0000:   data_num = 12;  break;
                case 0x0040:   data_num = 8;   break;
                case 0x0080:   data_num = 4;   break;
                case 0x00C0:   data_num = 1;   break;
                default    :   data_num = 0;
                               i2c_msg->errno = 0x0002;      // it's impossible
                               break;
            }

            for (i = 0; i < data_num; i ++)
            {
                i2c_msg->data[i2c_msg->current_data_cnt] = REG_APB4_I2C_DAT;
                i2c_msg->current_data_cnt ++;
                remain_data_cnt --;
            }

            if (remain_data_cnt == 0)
            {
                i2c_stat &= ~0x0100;     // send NACK
                i2c_msg->trans_done = 1;
            }
            else if (remain_data_cnt < data_num)
            {
                i2c_stat = i2c_stat | 0x01C0;     // send ACK, set int_mode
            }
            else
            {
                i2c_stat |= 0x0100;     // send ACK
            }
            break;
        }

		default:		
            break;
    }

    REG_APB4_I2C_STAT = (i2c_stat & (~0x01));  //clear si
    REG_APB4_I2C_CN = i2c_config;            // fix bug
}




