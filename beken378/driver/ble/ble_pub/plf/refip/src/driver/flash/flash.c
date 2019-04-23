
#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include "flash.h"         // flash definition
#include "common_error.h"      // error definition
#include "uart.h"






/// Flash environment structure
struct flash_env_tag
{
    /// base address
    uint32_t    base_addr;
    /// length
    uint32_t    length;
    /// type
    uint32_t    space_type;
};


/// Flash environment structure variable
struct flash_env_tag flash_env;


#define GD_FLASH_1	0XC84013
#define MX_FLASH_1	0XC22314
#define XTX_FLASH_1	0X1C3113

#define BY25Q80		0xe04014
#define PN25f04		0xe04013
#define MX_FLASH_4M 0XC22313

#define FLASH_LINE_1  1
#define FLASH_LINE_2  2
#define FLASH_LINE_4  4

#define DEFAULT_LINE_MODE  FLASH_LINE_4


uint32_t flash_mid = 0;
void set_flash_clk(unsigned char clk_conf) 
{
	//note :>16M don't use la for flash debug
    unsigned int temp0;
    temp0 = REG_FLASH_CONF;
    REG_FLASH_CONF = (  (clk_conf << BIT_FLASH_CLK_CONF)
                      | (temp0    &  SET_MODE_SEL)
                      | (temp0    &  SET_FWREN_FLASH_CPU)
                      | (temp0    &  SET_WRSR_DATA)
                      | (temp0    &  SET_CRC_EN));
	while(REG_FLASH_OPERATE_SW & 0x80000000){;}
}

uint32_t get_flash_ID(void)
{
    unsigned int temp0;

	while(REG_FLASH_OPERATE_SW & 0x80000000);
	temp0 = REG_FLASH_OPERATE_SW;
	REG_FLASH_OPERATE_SW = (  (temp0			 &	SET_ADDRESS_SW)
								| (FLASH_OPCODE_RDID << BIT_OP_TYPE_SW)
								| (0x1				 << BIT_OP_SW)
								| (0x01			 &	SET_WP_VALUE));
	while(REG_FLASH_OPERATE_SW & 0x80000000);
 
	return REG_FLASH_RDID_DATA_FLASH ;
}


uint32_t flash_read_sr( )
{
	uint16_t temp;
	uint32_t temp0;


	while(REG_FLASH_OPERATE_SW & 0x80000000);
	temp0 = REG_FLASH_OPERATE_SW;
	REG_FLASH_OPERATE_SW = (  (temp0             &  SET_ADDRESS_SW)
	                        | (FLASH_OPCODE_RDSR2 << BIT_OP_TYPE_SW)
	                        | (0x1               << BIT_OP_SW)
	                        | (temp0             &  SET_WP_VALUE));
	while(REG_FLASH_OPERATE_SW & 0x80000000);

	temp=(REG_FLASH_SR_DATA_CRC_CNT&0xff)<<8;

	
	 while(REG_FLASH_OPERATE_SW & 0x80000000);
	 temp0 = REG_FLASH_OPERATE_SW;
	 REG_FLASH_OPERATE_SW = (  (temp0			  &  SET_ADDRESS_SW)
							 | (FLASH_OPCODE_RDSR << BIT_OP_TYPE_SW)
							 | (0x1 			  << BIT_OP_SW)
							 | (temp0			  &  SET_WP_VALUE));

    
	while(REG_FLASH_OPERATE_SW & 0x80000000);
    temp |= (REG_FLASH_SR_DATA_CRC_CNT&0xff);
    
	return temp ;
}

void flash_write_enable( void )
{
    REG_FLASH_OPERATE_SW = ((FLASH_OPCODE_WREN << BIT_OP_TYPE_SW)
	                           | (0x1<< BIT_OP_SW)
	                           | (0x1<< BIT_WP_VALUE));
    while(REG_FLASH_OPERATE_SW & 0x80000000);
    
}
void flash_write_disable( void )
{
    REG_FLASH_OPERATE_SW = ((FLASH_OPCODE_WRDI<< BIT_OP_TYPE_SW)
	                           | (0x1<< BIT_OP_SW)
	                           | (0x1<< BIT_WP_VALUE));
    while(REG_FLASH_OPERATE_SW & 0x80000000);
    
}


void flash_write_sr( uint8_t bytes,  uint16_t val )
{
	switch(flash_mid)
	{
		case MX_FLASH_4M:
		case MX_FLASH_1:			   //MG xx
			REG_FLASH_CONF &= 0xffdf0fff;
		break;
		case XTX_FLASH_1:			   //XTX xx
			REG_FLASH_CONF &= 0xffff0fff;
		break;
		case GD_FLASH_1:			  //QD xx ,
		case BY25Q80:
		case PN25f04:
		default:
			REG_FLASH_CONF &= 0xfefe0fff;
		break;
	}
    REG_FLASH_CONF |= (val << BIT_WRSR_DATA)|SET_FWREN_FLASH_CPU;
	while(REG_FLASH_OPERATE_SW & 0x80000000);
    if( bytes == 1 ) 
    {
        REG_FLASH_OPERATE_SW = ((FLASH_OPCODE_WRSR << BIT_OP_TYPE_SW)
	                           | (0x1<< BIT_OP_SW)
	                           | (0x1<< BIT_WP_VALUE));
        
    }
    else if(bytes == 2 )
    {
        REG_FLASH_OPERATE_SW = ((FLASH_OPCODE_WRSR2 << BIT_OP_TYPE_SW)
	                           | (0x1<< BIT_OP_SW)
	                           | (0x1<< BIT_WP_VALUE));       
    }
        
    while(REG_FLASH_OPERATE_SW & 0x80000000);
}

void flash_wp_256k( void)
{
	switch(flash_mid)
	{
		case MX_FLASH_4M:
		case MX_FLASH_1:			   //MG xx
			flash_write_sr( 2, 0x080C );
			break;
		case XTX_FLASH_1:			   //XTX xx
			flash_write_sr( 1, 0x2C );
			break;
		case GD_FLASH_1:			  //QD xx ,
		case BY25Q80:
		case PN25f04:
		default:
			flash_write_sr( 2, 0x002c );
			break;
	}
}
void fflash_wr_protect_16k( void )
{
	switch(flash_mid)
	{
		case MX_FLASH_4M:
		case MX_FLASH_1:			   //MG xx
			flash_write_sr( 2, 0x0804 );
			break;
		case XTX_FLASH_1:			   //XTX xx
			flash_write_sr( 1, 0x24 );
			break;
		case GD_FLASH_1:			  //QD xx ,
		case BY25Q80:
		case PN25f04:
		default:
			flash_write_sr( 2, 0x0024 );// 64K有问题，16K 可以
			break;
	}
}

void flash_wr_protect_none( void)
{
	switch(flash_mid)
	{
		case MX_FLASH_4M:
		case MX_FLASH_1:			   //MG xx
			flash_write_sr( 2, 0x0000 );
			break;
		case XTX_FLASH_1:			   //XTX xx
			flash_write_sr( 1, 0x00 );
			break;
		case GD_FLASH_1:			  //QD xx ,
		case BY25Q80:
		case PN25f04:
		default:
			flash_write_sr( 2, 0X0000 );
			break;
	}
}

void flash_wp_ALL( void )
{
	switch(flash_mid)
	{
		case MX_FLASH_4M:
		case MX_FLASH_1:			   //MG xx
			flash_write_sr( 2, 0x003c );
			break;
		case XTX_FLASH_1:			   //XTX xx
			flash_write_sr( 1, 0x3C );
			break;
		case GD_FLASH_1:			  //QD xx ,
		case BY25Q80:
		case PN25f04:
		default:
			flash_write_sr( 2, 0x0014 );
			break;
	}
}



void flash_init1(void)
{
	flash_set_line_mode(1);

	flash_mid = get_flash_ID();
	
	flash_set_qe();
	flash_wp_256k();
}



/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


void flash_init(void)
{
	
    flash_set_line_mode(1);
	
	flash_mid = get_flash_ID();
	
    flash_wp_256k();
	
    flash_set_qe();

	//flash_wr_protect_none();

    flash_set_line_mode(4);
    set_flash_clk(0x2);
}


void flash_erase_sector(uint32_t address)
{
	flash_set_line_mode(1);
    uint32_t reg_value;
    while(REG_FLASH_OPERATE_SW & 0x80000000);
    reg_value = REG_FLASH_OPERATE_SW;

    REG_FLASH_OPERATE_SW = (  (address << BIT_ADDRESS_SW)
                            | (FLASH_OPCODE_SE<< BIT_OP_TYPE_SW)
                            | (0x1             << BIT_OP_SW)
                            | (reg_value & SET_WP_VALUE));
	 
    while(REG_FLASH_OPERATE_SW & 0x80000000);
	flash_set_line_mode(4);
}



void flash_read_data (uint8_t *buffer, uint32_t address, uint32_t len)
{
	
    uint32_t i, j,k,reg_value;
    uint32_t addr = address;
    uint32_t buf[8];
    k=0;    
    if (len == 0)
        return;

    while(REG_FLASH_OPERATE_SW & 0x80000000);

    for(j=0;j<((len-1)/32+1);j++)
    {
        reg_value = REG_FLASH_OPERATE_SW;
        REG_FLASH_OPERATE_SW = (  (addr << BIT_ADDRESS_SW)
                                | (FLASH_OPCODE_READ << BIT_OP_TYPE_SW)
                                | (0x1 << BIT_OP_SW)
                                | (reg_value &  SET_WP_VALUE));
        while(REG_FLASH_OPERATE_SW & 0x80000000);
        addr+=32;
        for (i=0; i<8; i++)
        {
            buf[i] = REG_FLASH_DATA_FLASH_SW;
        }

        if(len>32*(j+1))
            memcpy(&buffer[k],buf,32);
        else
        {
            memcpy(&buffer[k],buf,len-k);
        }
        k += 32;        
    }
	
}




void flash_write_data (uint8_t *buffer, uint32_t address, uint32_t len)
{
    uint32_t i, j,k,reg_value;
    uint32_t addr = address;
    uint32_t buf[8];
    k=0;
    if (len == 0)
        return;
	flash_set_line_mode(1);

    while(REG_FLASH_OPERATE_SW & 0x80000000);
    for(j=0;j<((len-1)/32+1);j++)
    {
        if(len>32*(j+1))
            memcpy(buf,&buffer[k],32);
        else
        {
         	for(i=0;i<8;i++)
                buf[i]=0xffffffff;
            memcpy(buf,&buffer[k],len-k);
        }
        k += 32;
        for (i=0; i<8; i++)
            REG_FLASH_DATA_SW_FLASH = buf[i];

        while(REG_FLASH_OPERATE_SW & 0x80000000);
        reg_value = REG_FLASH_OPERATE_SW;
        REG_FLASH_OPERATE_SW = (  (addr << BIT_ADDRESS_SW)
                                | (FLASH_OPCODE_PP << BIT_OP_TYPE_SW)
                                | (0x1 << BIT_OP_SW)
                                | (0x1 << BIT_WP_VALUE) // make WP equal 1 not protect SRP
                                | (reg_value &  SET_WP_VALUE));

        while(REG_FLASH_OPERATE_SW & 0x80000000);
        addr+=32;
    }

	flash_set_line_mode(4);
	
}



void flash_set_qe(void)
{
	uint32_t temp0;
	while(REG_FLASH_OPERATE_SW & 0x80000000){;}
//XTX_FLASH_1 没有QE,不需要,靠 EQPI(38h) CMD 来处理
//这个仅对MX_FLASH_1 、MX_FLASH_4M、XTX_FLASH_1有效。

	temp0 = REG_FLASH_CONF; //配置WRSR Status data

	if(flash_mid == XTX_FLASH_1)  // wanghong
		return;
	if((flash_mid == MX_FLASH_1)||(flash_mid == MX_FLASH_4M))  // wanghong
	{
		//WRSR QE=1
		REG_FLASH_CONF = ((temp0 &  SET_FLASH_CLK_CONF)
						| (temp0 &  SET_MODE_SEL)
						| (temp0 &  SET_FWREN_FLASH_CPU)
						| (temp0 & SET_WRSR_DATA)
						| (0x1 << 16) // SET QE=1,quad enable
						| (temp0 &  SET_CRC_EN));

		//Start WRSR
		temp0 = REG_FLASH_OPERATE_SW;
		REG_FLASH_OPERATE_SW = (  (temp0 &  SET_ADDRESS_SW)
						     | (FLASH_OPCODE_WRSR2 << BIT_OP_TYPE_SW)
						     | (0x1				  << BIT_OP_SW)
						     | (0x1				  << BIT_WP_VALUE)); // makeWP equal 1 not protect SRP
	}
	else
	{
		REG_FLASH_CONF = (	(temp0 &  SET_FLASH_CLK_CONF)
						  | (temp0 &  SET_MODE_SEL)
						  | (temp0 &  SET_FWREN_FLASH_CPU)
						  |	(temp0 & SET_WRSR_DATA)
						  |	(0x01 << 19)
						  | (temp0 &  SET_CRC_EN));

		//Start WRSR
		temp0 = REG_FLASH_OPERATE_SW;
		REG_FLASH_OPERATE_SW = (  (temp0			  &  SET_ADDRESS_SW)
						        | (FLASH_OPCODE_WRSR2 << BIT_OP_TYPE_SW)
						        | (0x1				  << BIT_OP_SW)
						        | (0x1				  << BIT_WP_VALUE)); // makeWP equal 1 not protect SRP
	}
	while(REG_FLASH_OPERATE_SW & 0x80000000);
}

static void set_flash_qwfr(void)
{
	 //只操作 MCU,不操作 SR，不用写 REG_FLASH_OPERATE_SW
    uint32_t temp0,mod_sel;
    temp0 = REG_FLASH_CONF;
	mod_sel = temp0 & (0xC << BIT_MODE_SEL); 
	mod_sel |= (0x2 << BIT_MODE_SEL);
    while(REG_FLASH_OPERATE_SW & 0x80000000);
    REG_FLASH_CONF = (  (temp0 &  SET_FLASH_CLK_CONF)
                      |  mod_sel  //QWFR配置四线MODE use QWFR to fetch data. For GigaDevice (GD) Flash
                      | (temp0 &  SET_FWREN_FLASH_CPU)
                      | (temp0 & SET_WRSR_DATA)
                      | (temp0 &  SET_CRC_EN));
}

void clr_flash_qwfr(void)
{
    uint32_t temp0,mod_sel;	
	
    temp0 = REG_FLASH_CONF;
    while(REG_FLASH_OPERATE_SW & 0x80000000){;}
    mod_sel = temp0 & (0xC << BIT_MODE_SEL); //??3ymode_sel?D
    mod_sel |= (0x1 << BIT_MODE_SEL);
    REG_FLASH_CONF = (  (temp0 &  SET_FLASH_CLK_CONF)
                        | mod_sel
                        | (temp0 &  SET_FWREN_FLASH_CPU)
                        | (temp0 &  SET_WRSR_DATA)
                        | (temp0 &  SET_CRC_EN));
    //reset flash
    temp0 = REG_FLASH_OPERATE_SW;
    if(flash_mid == XTX_FLASH_1)
    {
        REG_FLASH_OPERATE_SW = (  (0<< BIT_ADDRESS_SW)
                                | (FLASH_OPCODE_CRMR << BIT_OP_TYPE_SW)
                                | (0x1               << BIT_OP_SW)
                                | (temp0             &  SET_WP_VALUE));
    }
    else
    {
        REG_FLASH_OPERATE_SW = (  (0<< BIT_ADDRESS_SW)
                                | (FLASH_OPCODE_CRMR2 << BIT_OP_TYPE_SW)
                                | (0x1               << BIT_OP_SW)
                                | (temp0             &  SET_WP_VALUE));
    }

    while(REG_FLASH_OPERATE_SW & 0x80000000);
}



void flash_set_line_mode(uint8_t mode)
{
//XTX_FLASH_1 没有 QE，不需要，靠 EQPI(38h) CMD 来处理
//这个仅对MX_FLASH_1 、MX_FLASH_4M、XTX_FLASH_1有效。
//但还是要写下 XTX_FLASH_1 的位置，因为程序会读qe状态。
    switch(mode) 
    {
    case FLASH_LINE_1:
        clr_flash_qwfr();
        break;
    case FLASH_LINE_2:
        clr_flash_qwfr();
        REG_FLASH_CONF &= (~(7<<BIT_MODE_SEL));
        REG_FLASH_CONF |= (1<<BIT_MODE_SEL);
        break;
    case FLASH_LINE_4:

#if(DEFAULT_LINE_MODE == FLASH_LINE_4)
		if((flash_mid == MX_FLASH_1) || (flash_mid == XTX_FLASH_1)||(flash_mid == MX_FLASH_4M) )
		{
			REG_FLASH_SR_DATA_CRC_CNT = (0xa5 << 22);
		}
		set_flash_qwfr();                  /**< 4??? */

#else
        clr_flash_qwfr();
        REG_FLASH_CONF &= (~(7<<BIT_MODE_SEL));
        REG_FLASH_CONF |= (1<<BIT_MODE_SEL);
#endif
        break;
    default:
        break;
    }
    /* Flash read data operation after setting 4 line mode */
	while(REG_FLASH_OPERATE_SW & 0x80000000);

	
}


uint8_t flash_read(uint8_t flash_space, uint32_t address, uint32_t len, uint8_t *buffer, void (*callback)(void))
{
    uint32_t pre_address;
    uint32_t post_address;
    uint32_t pre_len;
    uint32_t post_len;
    uint32_t page0;
    uint32_t page1;
    page0 = address &(~FLASH_PAGE_MASK);
    page1 = (address + len) &(~FLASH_PAGE_MASK);
    if(page0 != page1)
    {
        pre_address = address;
        pre_len = page1 - address;
        flash_read_data(buffer, pre_address, pre_len);
        post_address = page1;
        post_len = address + len - page1;
        flash_read_data((buffer + pre_len), post_address, post_len);
    }
    else
    {
        flash_read_data(buffer, address, len);
    }
		
    return COMMON_ERROR_NO_ERROR;
}



uint8_t flash_write(uint8_t flash_space, uint32_t address, uint32_t len, uint8_t *buffer, void (*callback)(void))
{
    uint32_t pre_address;
    uint32_t post_address;
    uint32_t pre_len;
    uint32_t post_len;
    uint32_t page0;
    uint32_t page1;
    page0 = address &(~FLASH_PAGE_MASK);
    page1 = (address + len) &(~FLASH_PAGE_MASK);
    if(page0 != page1)
    {
        pre_address = address;
        pre_len = page1 - address;
        flash_write_data(buffer, pre_address, pre_len);
        
        post_address = page1;
        post_len = address + len - page1;
        flash_write_data((buffer + pre_len), post_address, post_len);

    }
    else
    {
        flash_write_data(buffer, address, len);

    }
		
     return COMMON_ERROR_NO_ERROR;
}



void udi_exchange_fdata_to_adjoining_next_sector(uint32_t data_addr, uint32_t len, uint32_t wr_point_inpage)
{
    /* assume: the space, from address(current sector) to next sector, can be operated  */
    uint8_t tmp[UPDATE_CHUNK_SIZE];
    int next_sector_addr;
    int rd_addr;
    int wr_addr;
    int total_cnt;
    int sub_cnt;
    int wr_point = wr_point_inpage;
    if((len > FLASH_ERASE_SECTOR_SIZE)
            || (0 == len))
    {
        return;
    }
    next_sector_addr = (data_addr & (~FLASH_ERASE_SECTOR_SIZE_MASK)) + FLASH_ERASE_SECTOR_SIZE;
    flash_erase_sector(next_sector_addr);
    total_cnt = len;
    rd_addr = data_addr;
    wr_point = wr_point & FLASH_ERASE_SECTOR_SIZE_MASK;
    wr_addr = next_sector_addr + wr_point;
    while(total_cnt > 0)
    {
        sub_cnt = MIN(UPDATE_CHUNK_SIZE, total_cnt);
        flash_read(flash_env.space_type, rd_addr, sub_cnt,tmp,NULL);
        flash_write(flash_env.space_type, wr_addr, sub_cnt,tmp,NULL);
        total_cnt -= sub_cnt;
        rd_addr += sub_cnt;
        wr_addr += sub_cnt;
    }
}


void udi_exchange_fdata_to_adjoining_previous_sector(uint32_t data_addr, uint32_t len, uint32_t wr_point_inpage)
{
    /* assume: the space, from previous sector to address+len, can be operated  */
    uint8_t tmp[UPDATE_CHUNK_SIZE];
    uint32_t pre_sector_addr;
    uint32_t rd_addr;
    uint32_t wr_addr;
    uint32_t total_cnt;
    uint32_t sub_cnt;
    uint32_t wr_point = wr_point_inpage;
    if((len > FLASH_ERASE_SECTOR_SIZE)
            || (0 == len))
    {
        return;
    }
    pre_sector_addr = (data_addr & (~FLASH_ERASE_SECTOR_SIZE_MASK)) - (uint32_t)FLASH_ERASE_SECTOR_SIZE;
    flash_erase_sector(pre_sector_addr);
    total_cnt = len;
    rd_addr = data_addr;
    wr_point = wr_point & FLASH_ERASE_SECTOR_SIZE_MASK;
    wr_addr = pre_sector_addr + wr_point;
    while(total_cnt > 0)
    {
        sub_cnt = MIN(UPDATE_CHUNK_SIZE, total_cnt);
        flash_read(flash_env.space_type, rd_addr, sub_cnt,tmp,NULL);
	 	flash_write(flash_env.space_type, wr_addr, sub_cnt,tmp,NULL);
        total_cnt -= sub_cnt;
        rd_addr += sub_cnt;
        wr_addr += sub_cnt;
    }
}



uint8_t flash_erase(uint8_t flash_type, uint32_t address, uint32_t len, void (*callback)(void))
{
    /* assume: the worst span is four sectors*/
    int first_len;
    int first_addr;
    int last_len;
    int last_addr;
    int mid_addr;
    int erase_addr;
    int erase_len;
    int wr_point;
    first_len  = address & FLASH_ERASE_SECTOR_SIZE_MASK;
    first_addr = address & (~FLASH_ERASE_SECTOR_SIZE_MASK);
    mid_addr   = first_addr + FLASH_ERASE_SECTOR_SIZE;
    wr_point   = 0;
    if(first_len)
    {
        udi_exchange_fdata_to_adjoining_next_sector(first_addr, first_len, wr_point);
        udi_exchange_fdata_to_adjoining_previous_sector(mid_addr, first_len, wr_point);
        erase_addr = mid_addr;
        erase_len = len - (FLASH_ERASE_SECTOR_SIZE - first_len);
    }
    else
    {
        erase_addr = address;
        erase_len = len;
    }
    do
    {
        int i;
        int erase_whole_sector_cnt;
        erase_whole_sector_cnt = erase_len >> FLASH_ERASE_SECTOR_SIZE_RSL_BIT_CNT;
        for(i = 0; i < erase_whole_sector_cnt; i ++)
        {
            flash_erase_sector(erase_addr);
            erase_addr += FLASH_ERASE_SECTOR_SIZE;
            erase_len -= FLASH_ERASE_SECTOR_SIZE;
        }
    }
    while(0);
    do
    {
        if(0 == erase_len)
        {
            break;
        }
		
        last_len = FLASH_ERASE_SECTOR_SIZE - erase_len;
        last_addr = erase_addr + erase_len;
        wr_point = 0;
        udi_exchange_fdata_to_adjoining_previous_sector(last_addr, last_len, wr_point);
        last_addr = erase_addr - FLASH_ERASE_SECTOR_SIZE;
        wr_point = erase_len;
        udi_exchange_fdata_to_adjoining_next_sector(last_addr, last_len, wr_point);
        flash_erase_sector(last_addr);
    }
    while(0);
	
    return COMMON_ERROR_NO_ERROR;
}





#define  c_Temp_SIZE    0X20
#define  TEST_FLASH_ADDRESS  0x72000
void flash_test(void)
{
	unsigned char cTemp[c_Temp_SIZE];
	unsigned char cTemp1[c_Temp_SIZE];
	int i;

	flash_read(0,TEST_FLASH_ADDRESS, c_Temp_SIZE,cTemp, NULL);
	//UART_PRINTF("cTemp:\r\n");
	for (i=0; i<c_Temp_SIZE; i++)
	{
	    //UART_PRINTF("%x ", cTemp[i]);
	}
	//UART_PRINTF("\r\n");
	flash_write_sr(2,0);
  
  	flash_erase(0, TEST_FLASH_ADDRESS, 0x1000, NULL);
 	// flash_erase_sector(0x72000);

	flash_read(0,TEST_FLASH_ADDRESS, c_Temp_SIZE,cTemp, NULL);
	//UART_PRINTF("cTemp:\r\n");
	for (i=0; i<0x10; i++)
	{
	    //UART_PRINTF("%x ", cTemp[i]);
	}
	//UART_PRINTF("\r\n");

	for (i=0; i<c_Temp_SIZE; i++)
	{
	    cTemp[i] = i;
	}
	//UART_PRINTF("cTemp:\r\n");
	for (i=0; i<c_Temp_SIZE; i++)
	{
	    //UART_PRINTF("%x ", cTemp[i]);
	}
	//UART_PRINTF("\r\n");
	flash_write(0,TEST_FLASH_ADDRESS ,c_Temp_SIZE,cTemp,NULL);

	memset(cTemp1, 0, c_Temp_SIZE);
	//UART_PRINTF("cTemp1:\r\n");
	for (i=0; i<c_Temp_SIZE; i++)
	{
	    //UART_PRINTF("%x ", cTemp1[i]);
	}
	//UART_PRINTF("\r\n");

	flash_read(0,TEST_FLASH_ADDRESS, c_Temp_SIZE,cTemp1, NULL);
	//UART_PRINTF("cTemp1:\r\n");
	for (i=0; i<c_Temp_SIZE; i++)
	{
	    //UART_PRINTF("%x ", cTemp1[i]);
	}

	//UART_PRINTF("\r\n");
	if (memcmp(cTemp, cTemp1, sizeof(cTemp)) == 0)
	{
		//UART_PRINTF("memcmp OK\r\n");
	}
	else
	{
		//UART_PRINTF("memcmp ERROR\r\n");
	}
}


#define MAX_SIZE     100



void flash_rw_test()
{
	uint16_t i;
	uint8_t data[MAX_SIZE];
	uint8_t data2[MAX_SIZE];

	for(i=0; i<MAX_SIZE; i++)
	{
		data[i] = i;
	}
	//UART_PRINTF("Gengrate data\r\n");
	flash_write_sr(2,0);
	flash_erase(0, TEST_FLASH_ADDRESS, 0x1000, NULL);
	//UART_PRINTF("earse flash\r\n");
	flash_write(0,TEST_FLASH_ADDRESS ,MAX_SIZE,data,NULL);
	flash_read(0,TEST_FLASH_ADDRESS, MAX_SIZE, data2, NULL);
	//UART_PRINTF("data:\r\n");
	for (i=0; i<MAX_SIZE; i++)
	{
	    //UART_PRINTF("%x", data2[i]);
	}

	//UART_PRINTF("\r\n");
}


