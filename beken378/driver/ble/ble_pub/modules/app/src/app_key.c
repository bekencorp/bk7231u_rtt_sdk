/** **************************************************************************************
 *
 * @file app_key.c
 *
 * @brief ffe0 Application Module entry point
 *
 * @auth  alen
 *
 * @date  2017.03.30
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "app_key.h"               	 // health thermometer functions

#include "common_bt.h"
#include "kernel_timer.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "rf.h"
#include "prf_utils.h"
#include "uart.h"
#include "gpio.h"
#include "app_hid.h"
#include "app_sec.h"
#include "audio.h"
#include "BK3435_reg.h"
#include "lld_evt.h"


uint8_t bt_tx_buff[SEND_BUFFER_CNT][SEND_BUFFER_SIZE];
uint8_t tx_buff_len[SEND_BUFFER_CNT];
uint8_t tx_buff_head = 0; 
uint8_t tx_buff_tail = 0;
uint8_t tx_buff_count = 0;
uint8_t key_press_flag,key_rel_flag;
uint8_t stan_key_len,old_key_len;
uint8_t cow[KEYBOARD_MAX_COL_SIZE];
uint8_t row[KEYBOARD_MAX_ROW_SIZE];
uint8_t key_flag[KEYBOARD_MAX_ROW_SIZE][KEYBOARD_MAX_COL_SIZE];
uint8_t stan_key[8],old_key[8];
uint8_t real_key_value[KEYBOARD_MAX_COL_SIZE];
uint8_t key_err_time,key_status;
uint8_t m_key,s_key,c_key,p_key,key_null;
uint8_t sys_media_key[4],sys_power_key[1],sys_standard_key[8];
uint8_t data_upload_status;
uint8_t f1_f12[14] = {0,1,4,45,46,11,12,13,14,9,10,8,6,24};
uint8_t axis_x,axis_y;
const uint8_t media_key[47][2] =
{
    {0x24,0x02},    //WWW back  0
    {0x25,0x02},    //WWW forward 1
    {0x26,0x02},    //WWW Stop  2
    {0x27,0x02},    //WWW Refresh 3
    {0x21,0x02},    //WWW Search 4
    {0x2A,0x02},    //WWW Favorites  5
    {0x23,0x02},    //WWW Home 6
    {0x8A,0x01},    //Mail  7
    {0xE2,0x00},    //Mute 8
    {0xEA,0x00},    //Volume-  9
    {0xE9,0x00},    //Volume+   10
    {0xCD,0x00},    //Play/Pause  11
    {0xB7,0x00},    //Stop  12
    {0xB6,0x00},    //Prev Track 13
    {0xB5,0x00},    //Next Track  14
    {0x83,0x01},    //Media Select  15
    {0x94,0x01},    //My Computer   16
    {0x92,0x01},    //Calculator   17
    {0x09,0x02},    //More Info  18
    {0xB2,0x00},    //Record     19
    {0xB3,0x00},    //Forward     20
    {0xB4,0x00},    //Rewind      21
    {0x8D,0x00},    //Guide        22
    {0x04,0x00},    //<Reserved>   23
    {0x30,0x00},    //Eject(Mac)   24
    {0x07,0x03},    //H7    25
    {0x0A,0x03},    //H10   26lightness+
    {0x0B,0x03},    //H11   27lightness-
    {0xb1,0x01},    //photo 28
    {0xb8,0x00},    //touchkey 29
    {0x14,0x03},    //H20   30
    {0x01,0x03},    //H1   31
    {0x02,0x03},    //H2   32
    {0x03,0x03},    //H3    33
    {0x04,0x03},    //H4    34
    {0x05,0x03},    //H5    35
    {0x06,0x03},    //H6    36
    {0x08,0x03},    //H8    37
    {0x09,0x03},    //H9    38
    {0x0C,0x03},    //H12   39
    {0x0D,0x03},    //H13   40
    {0x0E,0x03},    //H14   41
    {0x0F,0x03},    //H15   42
    {0x10,0x03},    //H16   43
    {0x11,0x03},    //H17   44
    {0x12,0x03},    //H18   45
    {0x13,0x03},    //H19   46
};

uint8_t power_key[3] =
{
    0x01,   //ACPI:Power
    0x02,   //ACPI:Sleep
    0x04,   //ACPI:Wake up
};




//按键配置
uint8_t stand_key_map[5][3]=
{
    {KEY_POWER,			KEY_UP_ARROW,	KEY_NULL},		//KR0
    {KEY_RIGHT_ARROW,	KEY_ENTER,		KEY_LEFT_ARROW},//KR1
    {KEY_NULL,			KEY_DOWN_ARROW,	KEY_NULL},		//KR2
    {KEY_VOLUME_ADD,	KEY_WWW_BACK, 	KEY_VOLUME_SUB},//KR3
    {KEY_APP,			KEY_VOICE, 		KEY_HOME}, 		//KR4
};

#if BK3435_DEMO_KIT
#if DEBUG_HW
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOD_0,GPIOD_1,GPIOD_2}; //p3.0,p3.1,p3.2
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOB_1,GPIOB_4,GPIOB_5,GPIOB_6,GPIOB_7}; //P1.1,p1.4,p1.5,p1.6,p1.7
#else
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOD_0,GPIOD_1,GPIOD_2}; //p3.0,p3.1,p3.2
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOC_0,GPIOC_1,GPIOC_2,GPIOC_3,GPIOC_4}; //P2.0,p2.1,p2.2,p2.3,p2.4
#endif
#else
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOB_0,GPIOD_2,GPIOD_1}; //p1.0,p3.2,p3.1
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOA_3,GPIOA_4,GPIOA_5,GPIOA_6,GPIOA_7}; //p0.3,p0.4,p0.5,p0.6 ,p0.7
#endif


volatile uint32_t sys_flag = 0;
app_key_type app_key_state = ALL_KEY_FREE;

//语音按键状态，0x0表示释放状态
static unsigned char voice_key_press_status = 0x0;
static unsigned char old_voice_key_press_status;
static uint32_t power_key_delay = 0;
static unsigned char app_clear_bond_flag = 0;


/*******************************************************************************
    //for 48M clock
    //100=12.5us
    //16=2us
    //8=1us
    //for 32.768 clock
    //8=1.6ms
    //16M clock
    //100=120us
*******************************************************************************/
void delay_us(uint32_t num)
{

    uint32_t i,j;
    for(i=0; i<num; i++)
        for(j=0; j<3; j++)
        {
            ;
        }
}


/* 按键初始化 */
void key_init(void)
{
    uint8_t i;
    for (i = 0; i < KEYBOARD_MAX_ROW_SIZE; i++)
    {
		gpio_config(gpio_buff_r[i], OUTPUT, PULL_NONE);
		gpio_set(gpio_buff_r[i], 0);
    }
    for (i = 0; i < KEYBOARD_MAX_COL_SIZE; i++)
    {
		gpio_config(gpio_buff_c[i], INPUT, PULL_HIGH);
    }
#if (DEEP_SLEEP)
	for(i=0; i<KEYBOARD_MAX_COL_SIZE; i++)
    {
    	REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		Delay_ms(2);
    	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
    }
#endif	
	//This flag is setted to allow system into sleep mode when power on.
	app_key_state = ALL_KEY_FREE;
	
}


/* 按键扫描*/
void key_scan(void)
{
    unsigned char i,j,rw,cw;
    stan_key_len = 0;
    sys_flag &= ~FLAG_KEY_ACTIVE;
    for(i=0; i<8; i++)
    {
        stan_key[i] = 0;
    }
    for(i=0; i<KEYBOARD_MAX_COL_SIZE; i++)
    {
        cow[i] = 0;
    }
    for(i=0; i<KEYBOARD_MAX_ROW_SIZE; i++)
    {
        row[i] = 0;
    }
    for(i=0; i<KEYBOARD_MAX_ROW_SIZE; i++)
    {
        for(j=0; j<KEYBOARD_MAX_COL_SIZE; j++)
        {
            key_flag[i][j] <<= 1;
        }
    }
    for(rw = 0; rw < KEYBOARD_MAX_ROW_SIZE; rw++)
    {
        gpio_config(gpio_buff_r[rw], OUTPUT, PULL_NONE); 
		gpio_set(gpio_buff_r[rw], 0);
        delay_us(50);
        for(cw = 0; cw < KEYBOARD_MAX_COL_SIZE; cw++)
        {
			if(0 == gpio_get_input(gpio_buff_c[cw]))
            {
                sys_flag |= FLAG_KEY_ACTIVE;
                key_flag[rw][cw] |=1;
                if((key_flag[rw][cw]&SCAN_COUT)==SCAN_COUT )
                {
                    real_key_value[cw] |= (1<<rw);
					if((rw == 4) && (cw == 1))  
					{
						voice_key_press_status = 0x1;
					}else if((rw == 0)&& (cw == 0))
					{
						if(++power_key_delay > 30)
						{
							//BlueLedOn();
							power_key_delay = 0;
							app_clear_bond_flag = 0x01;
						}
					}
                }
            }
            else 
            {
            	if( (key_flag[rw][cw]&SCAN_COUT) == 0x00)
            	{
            		real_key_value[cw] &= ~(1<<rw);
	                if((rw == 4) && (cw == 1))
					{
						voice_key_press_status = 0x0;
					}else if((rw == 0)&&(cw == 0))
					{
						power_key_delay = 0;
						app_clear_bond_flag = 0;
						//BlueLedOff();
					}
				}
            }
            if( real_key_value[cw]&(1<<rw))
            {
                stan_key[stan_key_len]=stand_key_map[rw][cw];
                row[rw]++;
                cow[cw]++;
                if(stan_key_len<8)
                {
                    stan_key_len++;
                }
            }
        }
        gpio_set(gpio_buff_r[rw],  1);
		gpio_config(gpio_buff_r[rw], HIRESI, PULL_NONE); 
		
		
    }
}

void key_judge_error(void)
{
    uint8_t  i;
    key_status &= ~KEY_STATUS_CHANGE;
    if(stan_key_len != old_key_len) //按键长度变化
    {
        key_status  |= KEY_STATUS_CHANGE;
    }
    else
    {
        for(i = 0; i < stan_key_len; i++)  //按键值不一样
        {
            if(old_key[i] != stan_key[i])
            {
                key_status  |= KEY_STATUS_CHANGE;
                break;
            }
        }
    }
    if(key_status&KEY_STATUS_CHANGE)
    {
        key_status &= ~KEY_STATUS_ERROR;
        key_err_time=0;
        old_key_len = stan_key_len;
        for(i = 0; i < stan_key_len; i++)
        {
            old_key[i] = stan_key[i];
        }
    }
    if((!(key_status & KEY_STATUS_CHANGE)) && stan_key_len) //如果相同按键按下
    {
        key_err_time++;
        if(key_err_time >= KEY_ERROR_TIMEOUT)
        {
            key_status |= KEY_STATUS_ERROR;
        }
    }
}


void key_ghost_scan(void)
{
    uint8_t i,r;
    key_status &= ~KEY_STATUS_GHOST ;
    if(stan_key_len > 2)
    {
        for(r = 0; r<KEYBOARD_MAX_COL_SIZE; r++)
        {
        	//如果当前列存在两个以上按键按下
            if(cow[r] >= 2)             
            {
                for(i = 0; i<KEYBOARD_MAX_ROW_SIZE; i++)
                {
                	//判断行中当前列是否存在按键按下
                    if(real_key_value[i]&(1<<r))  
                    {
                    	//如果在行中存在两个以上案件按下
                        if(row[i] >= 2)
                        {
                            key_status |= KEY_STATUS_GHOST ;;
                            return;
                        }
                    }
                }
            }
        }
    }
}

void key_data_analysis(void)
{
    uint8_t i = 0,j = 2;
    uint8_t numlock_sta;
    key_status &= ~KEY_STATUS_FN;
    for(i = 0; i < stan_key_len; i++)  //FN 键检测
    {
        if(KEY_FN == stan_key[i])
        {
            key_status |= KEY_STATUS_FN;
        }
    }
    //其他键检测
    for(i = 0; i < stan_key_len; i++)
    {
        m_key = 0xff;
        s_key = 0xff;
        c_key = 0xff;
        p_key = 0xff;
        key_null = 0xff;
        switch(stan_key[i])
        {
            //sepcal key
        case KEY_POWER: //POWER
            p_key = 0;
            break;     
        case KEY_SLEEP: //SLEEP
            p_key = 1;
            break;      
        case KEY_WAKERNEL_UP: //WAKE UP
            p_key = 2;
            break;      
        case KEY_VOLUME_SUB: //volume-
            m_key = 9;
            break;  
        case KEY_PLAY_PAUSE: //play/pause
            m_key = 11;
            break;  
        case KEY_VOLUME_ADD: //volume+
            m_key = 10;
            break;  
        case KEY_NEXT_TRACK: //next track
            m_key = 14;
            break;  
        case KEY_PREV_TRACK: //prev track
            m_key = 13;
            break;  
        case KEY_MUTE: //mute
            m_key = 8;
            break;      
        case KEY_FN: //FN
            break;     
	   case KEY_H7:  //H7
	   		m_key = 31; 
			break; 
	   case KEY_MAIL:  //mail
	   		m_key = 7;  
			break; 
	       case KEY_H8: //H8
		   	m_key = 32; 
			break;  
	   case KEY_H2: //H2
	   		m_key = 26; 
			break;  
	   case KEY_WWW_FAVORITE: //www favorite
	   		m_key = 5;  
			break;  
	       case KEY_H9:  //H9
		   	m_key = 33; 
			break; 
	   //case KEY_PAIR: //bind
	   //	break;     
	   case KEY_WWW_FORWARD: //www forward
	   		m_key = 1;  
		break;  
	       case KEY_H10: //H10
		   	m_key = 34; 
			break;  
	   case KEY_H1: //H1
	   		m_key = 25; 
		break;  
	       case KEY_WWW_STOP: //www stop
		   	m_key = 2;  
			break;  
	   case KEY_MY_COMPUTER: //my computer
	   		m_key = 16; 
		break;  
	   case KEY_H11: //H11
		   	m_key = 35; 
			break;  
	   case KEY_H3: //H3
	   		m_key = 27; 
			break;  
	    case KEY_WWW_BACK: //www back
		   	m_key = 0;   
			break; 
	    case KEY_STOP:  //stop
		   	m_key = 12; 
			break; 
	    case KEY_H12: //H12
		   	m_key = 36; 
			break;  
	    case KEY_WWW_REFRESH: //www refresh
		   	m_key = 3;  
			break;  
	    case KEY_CALCULATOR://calculator
		   	m_key = 17; 
			break;  
	    case KEY_H13: //H13
		   	m_key = 37; 
			break;  
	    case KEY_H4: //H4
		   	m_key = 28; 
			break;  
	    case KEY_WEB_HOME: //web home
		   	m_key = 6;  
			break;  
	    case KEY_H14: //H14
		   	m_key = 38; 
			break;  
	    case KEY_H5:  //H5
	   		m_key = 29; 
			break; 
	       case KEY_H6: //H6
		   	m_key = 30; 
			break;  
	       case KEY_MEDIA: //media
		   	m_key = 15; 
			break;  
	    case KEY_WWW_SEARCH: //www search
	   		m_key = 4;  
			break;  
	    case KEY_BACKLIGHT:  //BACKLIGHT          
		   	break;  
        // ctrl key
        case KEY_LCTRL :
        case KEY_LALT  :
        case KEY_LSHFIT:
        case KEY_LWIN  :
        case KEY_RCTRL :
        case KEY_RALT  :
        case KEY_RSHIFT:
        case KEY_RWIN  :
            c_key = (1<<(stan_key[i]-0XE0));
            break;
        case END_NULL  :
            key_null = 0x00;
            break;
        default :
            if(key_status & KEY_STATUS_FN)     //fn 键按下
            {
                switch(stan_key[i])
                {
                case KEY_INS:
                    s_key = KEY_INS;
                    break;
                case KEY_UP_ARROW:
                    s_key = KEY_UP_ARROW;
                    break;
                case KEY_DOWN_ARROW:
                    s_key = KEY_DOWN_ARROW;
                    break;
                case KEY_LEFT_ARROW:
                    s_key = KEY_LEFT_ARROW;
                    break;
                case KEY_RIGHT_ARROW:
                    s_key = KEY_RIGHT_ARROW;
                    break;
                default:
                    if(1)//system.keyboard_config.fn_num)  //带FN小键盘功能
                    {
                        numlock_sta=key_status&KEY_STATUS_NUMLOCK;
                        switch(stan_key[i])
                        {
                        case KEY_7 :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_7;
                            break;
                        case KEY_8 :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_8;
                            break;
                        case KEY_9 :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_9;
                            break;
                        case KEY_0 :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_DIV;
                            break;
                        case KEY_U :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_4;
                            break;
                        case KEY_I :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_5;
                            break;
                        case KEY_O :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_6;
                            break;
                        case KEY_P :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_MUL;
                            break;
                        case KEY_J :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_1;
                            break;
                        case KEY_K :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_2;
                            break;
                        case KEY_L :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_3;
                            break;
                        case KEY_SEMICOLON:
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_SUB;
                            break;
                        case KEY_M :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_0;
                            break;
                        case KEY_DOT:
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_DOT;
                            break;
                        case KEY_QUESTION :
                            s_key = numlock_sta ? stan_key[i] : KEY_PAD_ADD;
                            break;
                        default:
                            s_key = stan_key[i];
                            break;
                        }
                    } //不带FN小键盘功能
                    else
                    {
                        s_key = stan_key[i];
                    }
                    break;
                }
            }
            else    // fn 键没按下
            {
                if(1)//system.keyboard_config.fn_num)  //带FN小键盘功能
                {
                    numlock_sta=key_status&KEY_STATUS_NUMLOCK;
                    switch(stan_key[i])
                    {
                    //case KEY_F1 : m_key = f1_f12[0]; break;
                    //case KEY_F2 : m_key = f1_f12[1]; break;
                    case KEY_F3 :
                        m_key = f1_f12[2];
                        break;
                    case KEY_F4 :
                        m_key = f1_f12[3];
                        break;
                    case KEY_F5 :
                        m_key = f1_f12[4];
                        break;
                    //case KEY_F6 : m_key = f1_f12[6]; break;
                    case KEY_F7 :
                        m_key = f1_f12[8];
                        break;
                    case KEY_F8 :
                        m_key = f1_f12[5];
                        break;
                    case KEY_F9 :
                        m_key = f1_f12[7];
                        break;
                    case KEY_F10 :
                        m_key = f1_f12[11];
                        break;
                    case KEY_F11 :
                        m_key = f1_f12[9];
                        break;
                    case KEY_F12 :
                        m_key = f1_f12[10];
                        break;
                    case KEY_ESC :
                        m_key = f1_f12[12];
                        break;
                    case KEY_DEL :
                        m_key = f1_f12[13];
                        break;
                    case KEY_7 :
                        s_key = numlock_sta ? KEY_PAD_7: stan_key[i];
                        break;
                    case KEY_8 :
                        s_key = numlock_sta ? KEY_PAD_8: stan_key[i];
                        break;
                    case KEY_9 :
                        s_key = numlock_sta ? KEY_PAD_9: stan_key[i];
                        break;
                    case KEY_0 :
                        s_key = numlock_sta ? KEY_PAD_DIV: stan_key[i];
                        break;
                    case KEY_U :
                        s_key = numlock_sta ? KEY_PAD_4: stan_key[i];
                        break;
                    case KEY_I :
                        s_key = numlock_sta ? KEY_PAD_5: stan_key[i];
                        break;
                    case KEY_O :
                        s_key = numlock_sta ? KEY_PAD_6: stan_key[i];
                        break;
                    case KEY_P :
                        s_key = numlock_sta ? KEY_PAD_MUL: stan_key[i];
                        break;
                    case KEY_J :
                        s_key = numlock_sta ? KEY_PAD_1: stan_key[i];
                        break;
                    case KEY_K :
                        s_key = numlock_sta ? KEY_PAD_2: stan_key[i];
                        break;
                    case KEY_L :
                        s_key = numlock_sta ? KEY_PAD_3: stan_key[i];
                        break;
                    case KEY_SEMICOLON:
                        s_key = numlock_sta ? KEY_PAD_SUB: stan_key[i];
                        break;
                    case KEY_M :
                        s_key = numlock_sta ? KEY_PAD_0: stan_key[i];
                        break;
                    case KEY_DOT:
                        s_key = numlock_sta ? KEY_PAD_DOT: stan_key[i];
                        break;
                    case KEY_QUESTION:
                        s_key = numlock_sta ? KEY_PAD_ADD: stan_key[i];
                        break;
                    default:
                        s_key = stan_key[i];
                        break;
                    }
                } //不带FN小键盘功能
                else
                {
                    s_key = stan_key[i];
                }
            }
            break;
        }
        if(m_key != 0xff) //多媒体
        {
            sys_media_key[0] = media_key[m_key][0];
            sys_media_key[1] = media_key[m_key][1];
            data_upload_status |= MEDIA_KEY_UPLOAD;
            data_upload_status |= MEDIA_KEY_SEND;
        }
        if(p_key != 0xff) //电源控制
        {
            sys_power_key[0] = power_key[p_key];
            data_upload_status |= POWER_KEY_UPLOAD;
            data_upload_status |= POWER_KEY_SEND;
        }
        if(s_key != 0xff) //普通按键
        {
            if(j < 8)
            {
                sys_standard_key[j] = s_key;
                j++;
            }
            data_upload_status |= STANDARD_KEY_UPLOAD;
            data_upload_status |= STANDARD_KEY_SEND;
        }
        if(c_key != 0xff)  //控制按键
        {
            sys_standard_key[0] |= c_key;
            data_upload_status |= STANDARD_KEY_UPLOAD;
            data_upload_status |= STANDARD_KEY_SEND;
        }
    }
    if(stan_key_len == 0x00)
    {
        if(data_upload_status & STANDARD_KEY_UPLOAD)  //普通按键释放
        {
            memset(sys_standard_key,0,8);
            data_upload_status &= ~STANDARD_KEY_UPLOAD;
            data_upload_status |= STANDARD_KEY_SEND;
        }
        if(data_upload_status & MEDIA_KEY_UPLOAD)  //多媒体按键释放
        {
            data_upload_status &= ~MEDIA_KEY_UPLOAD;
            data_upload_status |= MEDIA_KEY_SEND;
            sys_media_key[0] = 0;
            sys_media_key[1] = 0;
            sys_media_key[2] = 0;
            sys_media_key[3] = 0;
        }
        if(data_upload_status & POWER_KEY_UPLOAD)  //电源控制按键释放
        {
            data_upload_status &= ~POWER_KEY_UPLOAD;
            data_upload_status |= POWER_KEY_SEND;
            sys_power_key[0] = 0x00;
        }
    }
    if((!(data_upload_status & STANDARD_KEY_SEND)) && (data_upload_status & STANDARD_KEY_UPLOAD))  //普通按键释放
    {
        memset(sys_standard_key,0,8);
        data_upload_status &= ~STANDARD_KEY_UPLOAD;
        data_upload_status |= STANDARD_KEY_SEND;
    }
    if((!(data_upload_status & MEDIA_KEY_SEND)) && (data_upload_status & MEDIA_KEY_UPLOAD))  //多媒体按键释放
    {
        data_upload_status &= ~MEDIA_KEY_UPLOAD;
        data_upload_status |= MEDIA_KEY_SEND;
        sys_media_key[0] = 0;
        sys_media_key[1] = 0;
        sys_media_key[2] = 0;
        sys_media_key[3] = 0;
    }
    if((!(data_upload_status & POWER_KEY_SEND)) && (data_upload_status & POWER_KEY_UPLOAD))  //电源控制按键释放
    {
        data_upload_status &= ~POWER_KEY_UPLOAD;
        data_upload_status |= POWER_KEY_SEND;
        sys_power_key[0] = 0x00;
    }
}

uint8_t xyz_axis_process(uint8_t joystick_xyz)
{
    if(joystick_xyz>0x78&& joystick_xyz<0x88)
    {
        joystick_xyz=0x80;
    }
    else if(joystick_xyz<0x10)
    {
        joystick_xyz=0;
    }
    else if(joystick_xyz<=0x78)
    {
        joystick_xyz=(0x80*(joystick_xyz-0x10))/(float)0x68;
    }
    else if(joystick_xyz>=0xf0)
    {
        joystick_xyz=0xff;
    }
    else
    {
        joystick_xyz=0xff-(0x80*(0xf0-joystick_xyz))/(float)0x68;
    }
    return joystick_xyz;
}

void key_state_reset(void)
{
	tx_buff_count = 0;
	tx_buff_tail = 0;
	tx_buff_head = 0;
}


void key_process(void)
{
    key_scan();
    key_judge_error();
    key_ghost_scan();

    if( (key_status&KEY_STATUS_CHANGE)&&(!(key_status&(KEY_STATUS_ERROR|KEY_STATUS_GHOST)))
            &&(tx_buff_count<SEND_BUFFER_CNT))
    {
        key_data_analysis();
        if(data_upload_status & STANDARD_KEY_SEND)
        {
            memcpy(&(bt_tx_buff[tx_buff_head][0]),sys_standard_key,8);
            memset(sys_standard_key,0,8);
            tx_buff_len[tx_buff_head]=8;
            tx_buff_head++;
            tx_buff_count++;
            tx_buff_head=tx_buff_head%SEND_BUFFER_CNT;
            data_upload_status &= ~STANDARD_KEY_SEND;
        }
        if(data_upload_status & MEDIA_KEY_SEND)
        {
            bt_tx_buff[tx_buff_head][0]=sys_media_key[0];
            bt_tx_buff[tx_buff_head][1]=sys_media_key[1];
            tx_buff_len[tx_buff_head]=2;
            tx_buff_head++;
            tx_buff_count++;
            sys_media_key[0]=0;
            sys_media_key[1]=0;
            sys_media_key[2]=0;
            sys_media_key[3]=0;
            tx_buff_head=tx_buff_head%SEND_BUFFER_CNT;
            data_upload_status &= ~MEDIA_KEY_SEND;
        }
        if(data_upload_status & POWER_KEY_SEND)
        {
            bt_tx_buff[tx_buff_head][0]=sys_power_key[0];
            sys_power_key[0]=0;
            tx_buff_len[tx_buff_head]=1;
            tx_buff_head++;
            tx_buff_count++;
            tx_buff_head=tx_buff_head%SEND_BUFFER_CNT;
            data_upload_status &= ~POWER_KEY_SEND;
        }
    }
    
}

void key_wakeup_set(void)
{
    uint8_t i;
    for(i=0; i<KEYBOARD_MAX_ROW_SIZE; i++)
    {
        gpio_set(gpio_buff_r[i], 1);
		gpio_config(gpio_buff_r[i], HIRESI, PULL_NONE); 
    }
    REG_APB5_GPIO_WUATOD_ENABLE = 0x00000000; 
	REG_APB5_GPIO_WUATOD_STAT = 0xffffffff;
    REG_AHB0_ICU_INT_ENABLE &= (~(0x01 << 9));

}


/*配置按键中断及休眠*/
void key_wakeup_config(void)
{
    uint8_t i;
    
    for(i=0; i<KEYBOARD_MAX_ROW_SIZE; i++)
    {
        gpio_config(gpio_buff_r[i],OUTPUT,PULL_NONE);
        gpio_set(gpio_buff_r[i],0);
    }
	for(i=0; i<KEYBOARD_MAX_COL_SIZE; i++)
    {
		gpio_config(gpio_buff_c[i], INPUT, PULL_HIGH);
    }

	//Clear period timer
	kernel_timer_clear(APP_PERIOD_TIMER, TASK_APP);
	for(i=0; i<KEYBOARD_MAX_COL_SIZE; i++)
    {
    	REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		Delay_ms(2);
    	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
		REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(gpio_buff_c[i]>>4)+(gpio_buff_c[i]&0x0f));
    }
}



/*检测按键的状态，返回0表示语音按键按下，返回1表示普通按键或者没有按键按下*/
app_key_type key_status_check(void)
{
	if((voice_key_press_status && app_hid_env.audio_start))
	{
		if(kernel_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
		{
			kernel_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
		}
		if(kernel_timer_active(APP_DISCONNECT_TIMER, TASK_APP))
		{
			kernel_timer_clear(APP_DISCONNECT_TIMER, TASK_APP);
		}

		return (VOICE_KEY_DOWN);
	}
	else
	{
		if((stan_key_len == 0)&&(0==(sys_flag & FLAG_KEY_ACTIVE)))
		{
			if(kernel_state_get(TASK_APP) == APPM_ADVERTISING)
			{
				//Start the advertising timer
		        kernel_timer_set(APP_ADV_TIMEOUT_TIMER, TASK_APP, APP_DFLT_ADV_DURATION);
			}
			else if(kernel_state_get(TASK_APP) == APPM_CONNECTED)
			{
				//set short connect
				if(app_get_mode().target_mode == CONN_MODE_SHORT)
				{
					kernel_timer_set(APP_DISCONNECT_TIMER, TASK_APP,app_get_mode().target_timeout);
				}
			}

			key_wakeup_config();
			
			//UART_PRINTF("key sleep\r\n");
						
			return (ALL_KEY_FREE);
		}
		else //key down
		{			
			if(kernel_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
			{
				kernel_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
			}
			if(kernel_timer_active(APP_DISCONNECT_TIMER, TASK_APP))
			{
				kernel_timer_clear(APP_DISCONNECT_TIMER, TASK_APP);
			}

			return (GENERAL_KEY_DOWN);
		}
	}
}




/*发送按键键值*/
void hid_send_keycode( void )
{
    if(tx_buff_count)
    {
		if(voice_key_press_status && (bt_tx_buff[tx_buff_tail][2] == 0xDF)
									&& kernel_state_get(TASK_APP)== APPM_CONNECTED)
		{
			old_voice_key_press_status = voice_key_press_status;
			//UART_PRINTF("voice key down\r\n");
			audio_start();
		}
		else if(old_voice_key_press_status && (!voice_key_press_status))
		{
			old_voice_key_press_status = 0x0;
			voice_key_press_status = 0x0;

			//UART_PRINTF("voice key up\r\n");
			audio_stop();
		}
		else 
		{
			for(uint8_t i=0; i<tx_buff_len[tx_buff_tail]; i++)
			{
				//UART_PRINTF("0x%x ", bt_tx_buff[tx_buff_tail][i]);
			}
			//UART_PRINTF("\r\n");
		
			if((tx_buff_len[tx_buff_tail] != 0x1) && (!voice_key_press_status)) //not power key
			{
				app_hid_send_report(&bt_tx_buff[tx_buff_tail][0],tx_buff_len[tx_buff_tail]); 
			}
		}
		
        tx_buff_tail++;
        tx_buff_tail %= SEND_BUFFER_CNT;
        tx_buff_count--;
    }

	if(app_clear_bond_flag) //power key to clear bond info
	{
		//UART_PRINTF("User clear adv mode\r\n");
		app_clear_bond_flag = 0;
		appm_switch_general_adv();	
	}
/*
#if (DEEP_SLEEP)	
	app_key_state = key_status_check();
	//if(app_key_state != ALL_KEY_FREE)
	{
		kernel_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
	}
#else
	kernel_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
#endif
*/
}


