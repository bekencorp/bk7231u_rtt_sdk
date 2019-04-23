#include "include.h"
#include "arm_arch.h"

#if ((CFG_USE_AUDIO) && (CFG_SOC_NAME == SOC_BK7221U))
#include "audio_pub.h"

#include "mem_pub.h"
#include "drv_model_pub.h"
#include "sys_rtos.h"
#include "rtos_pub.h"
#include "error.h"

#if CFG_SUPPORT_ALIOS
#include "ll.h"
#endif

//#include "song.h"

#define AUD_INTF_DEBUG
#include "uart_pub.h"
#ifdef AUD_INTF_DEBUG
#define AUD_INTF_PRT                os_printf
#define AUD_INTF_WPRT               warning_prf
#define AUD_INTF_FATAL              fatal_prf
#else
#define AUD_INTF_PRT                null_prf
#define AUD_INTF_WPRT               null_prf
#define AUD_INTF_FATAL              null_prf
#endif

enum
{
    AUD_INTF_DAC_PLAY               = 0,
    AUD_INTF_DAC_PAUSE,
    AUD_INTF_DAC_SET_VOLUME,

    AUD_INTF_ADC_PLAY,
    AUD_INTF_ADC_PAUSE,
    AUD_INTF_ADC_LINEIN_TIMER,

    AUD_INTF_SET_SAMPLE_RATE,
    
    AUD_INTF_EXIT,
};

typedef struct audio_message 
{
	UINT32 audio_msg;
}AUDIO_MSG_T;
#define AUDIO_QITEM_COUNT          (5)

beken_queue_t audio_msg_que = NULL;
#if (CFG_SUPPORT_ALIOS || CFG_SUPPORT_RTT)
beken_thread_t  audio_handle;
#else
xTaskHandle  audio_handle = NULL;
#endif

beken_semaphore_t audio_intf_semaphore = NULL;

void audio_intf_send_msg(u32 new_msg);


#if CFG_USE_AUD_DAC
AUD_DAC_CFG_ST aud_dac_cfg;
DD_HANDLE aud_dac_hdl = DD_HANDLE_UNVALID;

#define AUD_DAC_BUF_LEN            (8 * 1024 + 4)
UINT8 audio_dac_buf[AUD_DAC_BUF_LEN];
UINT8 *audio_read;

static void audio_intf_dac_pause_poll(void)
{
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_PAUSE, NULL);
}

void audio_intf_dac_pause(void)
{
	audio_intf_send_msg(AUD_INTF_DAC_PAUSE);
}

static void audio_intf_dac_play_poll(void)
{
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_PLAY, NULL);
}

void audio_intf_dac_play(void)
{
	audio_intf_send_msg(AUD_INTF_DAC_PLAY);
}

static void audio_intf_dac_set_volume_poll(void)
{
    static UINT32 dac_vol = 0;

    dac_vol++;
    if(dac_vol > 100)
        dac_vol = 0;

    AUD_INTF_PRT("set dac vol :%d\r\n", dac_vol);
    
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_SET_VOLUME, &dac_vol);
}

void audio_intf_dac_set_volume(void)
{
	audio_intf_send_msg(AUD_INTF_DAC_SET_VOLUME);
}

#endif // CFG_USE_AUD_DAC

#if CFG_USE_AUD_ADC
AUD_ADC_CFG_ST aud_adc_cfg;
DD_HANDLE aud_adc_hdl = DD_HANDLE_UNVALID;

#define AUD_ADC_BUF_LEN            (3 * 1024)
UINT8 audio_adc_buf[AUD_ADC_BUF_LEN];

#define AUD_ADC_DATA_LEN        (2 * 1024)
UINT8 adc_data[AUD_ADC_DATA_LEN];

#define ADC_LINEIN_DETECT_TIMER_INTVAL           (10)
beken_timer_t audio_adc_linein_timer;

void audio_intf_adc_node_callback(UINT8 *buf, void *usr_data)
{
    //os_printf("dac callback\r\n");
    if(audio_intf_semaphore)
    {
        rtos_set_semaphore(&audio_intf_semaphore);
    }
}

static void audio_intf_adc_pause_poll(void)
{
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_PAUSE, NULL);
}

void audio_intf_adc_pause(void)
{
	audio_intf_send_msg(AUD_INTF_ADC_PAUSE);
}

static void audio_intf_adc_play_poll(void)
{
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_PLAY, NULL);
}

void audio_intf_adc_play(void)
{
	audio_intf_send_msg(AUD_INTF_ADC_PLAY);
}

static void audio_intf_adc_linein_timer_poll(void)
{   
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_DO_LINEIN_DETECT, NULL);
    
    rtos_reload_timer(&audio_adc_linein_timer);
    //AUD_INTF_PRT("restart timer\r\n");
}

static void audio_intf_adc_linein_timer_handler(void *data)
{
	audio_intf_send_msg(AUD_INTF_ADC_LINEIN_TIMER);
}
#endif

UINT32 sample_rate_tab[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};
static void audio_intf_set_sample_rate_poll(void)
{   
    static UINT32 i = 0;
    UINT32 param = 0;

    i++;
    if(i >= sizeof(sample_rate_tab)/4)
        i = 0;
    
    param = sample_rate_tab[i];
    
    #if CFG_USE_AUD_DAC
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_SET_SAMPLE_RATE, &param);
    #endif

    #if CFG_USE_AUD_ADC
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_SET_SAMPLE_RATE, &param);
    #endif

    AUD_INTF_PRT("set sample rate:%d\r\n", param);
}

void audio_intf_set_sample_rate(void)
{
	audio_intf_send_msg(AUD_INTF_SET_SAMPLE_RATE);
}

static void audio_intf_main( beken_thread_arg_t data )
{
    OSStatus err;
    UINT32 status;
    GLOBAL_INT_DECLARATION();

    #define AUDIO_INTF_DEF_SAMPLE_RATE      44100
    #define AUDIO_INTF_DEF_VOL              80
    #define AUDIO_INTF_DEF_CHANNEL          2
    #define AUDIO_INTF_DEF_WAIT_TIME        (10)

    if(audio_intf_semaphore == NULL) 
    {
#if CFG_SUPPORT_ALIOS
        err = rtos_init_semaphore(&audio_intf_semaphore, 0);
#else
        err = rtos_init_semaphore(&audio_intf_semaphore, 1);
#endif
        ASSERT(kNoErr == err);
    }
    
#if CFG_USE_AUD_DAC
    aud_dac_cfg.buf = &audio_dac_buf[0];
    aud_dac_cfg.buf_len = AUD_DAC_BUF_LEN;
    aud_dac_cfg.freq = AUDIO_INTF_DEF_SAMPLE_RATE;
    aud_dac_cfg.channels = AUDIO_INTF_DEF_CHANNEL;
    aud_dac_cfg.dma_mode = 1;
    #if AUD_USE_EXT_PA
    aud_dac_cfg.mute_pin = AUD_DAC_MUTE_PIN;
    #else
    aud_dac_cfg.mute_pin = 0;
    #endif
    aud_dac_cfg.buf_finish_cb = audio_intf_adc_node_callback;
    aud_dac_cfg.usr_data = NULL;

    GLOBAL_INT_DISABLE();
    aud_dac_hdl = ddev_open(AUD_DAC_DEV_NAME, &status, (UINT32)&aud_dac_cfg);
    status = status;
    if(DD_HANDLE_UNVALID == aud_dac_hdl)
    {
        GLOBAL_INT_RESTORE();
        goto audio_exit;
    }
    GLOBAL_INT_RESTORE();
#endif

#if CFG_USE_AUD_ADC
    aud_adc_cfg.buf = &audio_adc_buf[0];
    aud_adc_cfg.buf_len = AUD_ADC_BUF_LEN;
    aud_adc_cfg.freq = AUDIO_INTF_DEF_SAMPLE_RATE;
    aud_adc_cfg.channels = AUDIO_INTF_DEF_CHANNEL;
    
    //aud_adc_cfg.mode = 0;
    aud_adc_cfg.mode |= AUD_ADC_MODE_DMA_BIT;
    aud_adc_cfg.linein_detect_pin = AUD_ADC_LINEIN_DETECT_PIN;

    GLOBAL_INT_DISABLE();
    aud_adc_hdl = ddev_open(AUD_ADC_DEV_NAME, &status, (UINT32)&aud_adc_cfg);
    status = status;
    if(DD_HANDLE_UNVALID == aud_adc_hdl)
    {
        GLOBAL_INT_RESTORE();
        goto audio_exit;
    }
    GLOBAL_INT_RESTORE();

    status = 80;
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_SET_VOLUME, &status);

    err = rtos_init_timer(&audio_adc_linein_timer, 
							ADC_LINEIN_DETECT_TIMER_INTVAL, 
							audio_intf_adc_linein_timer_handler, 
							(void *)0);
    ASSERT(kNoErr == err);
    
	err = rtos_start_timer(&audio_adc_linein_timer);
	ASSERT(kNoErr == err);

    ddev_control(aud_adc_hdl, AUD_ADC_CMD_PLAY, NULL);
#endif

    while(1)
    {
        AUDIO_MSG_T msg;

        // for command
        err = rtos_pop_from_queue(&audio_msg_que, &msg, AUDIO_INTF_DEF_WAIT_TIME);
        if(kNoErr == err) 
        {
        	switch(msg.audio_msg) 
            {
                #if CFG_USE_AUD_DAC
                case AUD_INTF_DAC_PLAY:
                {
                    //audio_intf_dac_play_poll();
                }
                break;

                case AUD_INTF_DAC_PAUSE:
                {
                    //audio_intf_dac_pause_poll();
                }
                break;

                case AUD_INTF_DAC_SET_VOLUME:
                {
                    audio_intf_dac_set_volume_poll();
                }
                break;
                #endif // CFG_USE_AUD_DAC

                #if CFG_USE_AUD_ADC
                case AUD_INTF_ADC_PLAY:
                {
                    audio_intf_adc_play_poll();
                }
                break;

                case AUD_INTF_ADC_PAUSE:
                {
                    audio_intf_adc_pause_poll();
                }
                break;

                case AUD_INTF_ADC_LINEIN_TIMER:
                {
                    audio_intf_adc_linein_timer_poll();
                }
                break;
                #endif // CFG_USE_AUD_DAC

                case AUD_INTF_SET_SAMPLE_RATE:
                {
                    audio_intf_set_sample_rate_poll();
                }
                break;
                
                case AUD_INTF_EXIT:
                {
                    AUD_INTF_FATAL("exit aud intf\r\n");
                    goto audio_exit;
                }
                break;
                    
                default:
                    break;
            }
        }


        // for fill data
        {
            UINT32 mic_filled_len, copy_len;

            mic_filled_len = ddev_control(aud_adc_hdl, AUD_ADC_CMD_GET_FILL_BUF_SIZE, NULL);

            if(mic_filled_len < AUD_ADC_DATA_LEN) 
            {
                continue;
            }
            
            copy_len = AUD_ADC_DATA_LEN;
            
            if(copy_len) 
            {
                ddev_read(aud_adc_hdl, (char *)adc_data, copy_len, 0);
                
                while(ddev_write(aud_dac_hdl, (char *)adc_data, copy_len, 0) <= 0)
                {
                    if(rtos_get_semaphore(&audio_intf_semaphore, AUDIO_INTF_DEF_WAIT_TIME))
                    {
                        //AUD_INTF_FATAL("write timeout\r\n");
                    }
                }
            }
        }
    }

audio_exit:

    #if CFG_USE_AUD_DAC
    GLOBAL_INT_DISABLE();
    status = ddev_close(aud_dac_hdl);
    GLOBAL_INT_RESTORE();    
    aud_dac_hdl = DD_HANDLE_UNVALID;
    #endif // CFG_USE_AUD_DAC

    #if CFG_USE_AUD_ADC
    GLOBAL_INT_DISABLE();
    status = ddev_close(aud_adc_hdl);
    GLOBAL_INT_RESTORE();    
    aud_adc_hdl = DD_HANDLE_UNVALID;

    err = rtos_deinit_timer(&audio_adc_linein_timer);
    ASSERT(kNoErr == err);
    #endif // CFG_USE_AUD_ADC
 
    rtos_deinit_queue(&audio_msg_que);
    audio_msg_que = NULL;

    if(audio_intf_semaphore)
    {
        rtos_deinit_semaphore(&audio_intf_semaphore);
        audio_intf_semaphore = NULL;
    }

    audio_handle = NULL;
    rtos_delete_thread(NULL);
}

UINT32 audio_intf_init(void)
{   
    int ret;
	
    if((!audio_handle) && (!audio_msg_que))
    {

    	ret = rtos_init_queue(&audio_msg_que, 
    							"audio_queue",
    							sizeof(AUDIO_MSG_T),
    							AUDIO_QITEM_COUNT);
    	if (kNoErr != ret) 
    	{
    		AUD_INTF_FATAL("audio_intf ceate queue failed\r\n");
            return kGeneralErr;
    	}
        
        ret = rtos_create_thread(&audio_handle,
                                      BEKEN_DEFAULT_WORKER_PRIORITY,
                                      "audio",
                                      (beken_thread_function_t)audio_intf_main,
                                      1024,
                                      NULL);
        if (ret != kNoErr)
        {
            rtos_deinit_queue(&audio_msg_que);
            audio_msg_que = NULL;
            AUD_INTF_FATAL("Error: Failed to create audio_intf: %d\r\n", ret);
            return kGeneralErr;
        }
    }

    return kNoErr;
}

void audio_intf_uninit(void)
{
    int ret;
    
    if((audio_handle) && (audio_msg_que))
    {
    	audio_intf_send_msg(AUD_INTF_EXIT);
        
        // wait untill task exit
        while(audio_handle)
            rtos_delay_milliseconds(100);     
    }
}

void audio_intf_send_msg(u32 new_msg)
{
	OSStatus ret;
	AUDIO_MSG_T msg;

    if(audio_msg_que) {
    	msg.audio_msg = new_msg;
    	
    	ret = rtos_push_to_queue(&audio_msg_que, &msg, BEKEN_NO_WAIT);
    	if(kNoErr != ret)
    	{
    		os_printf("audio_intf_send_msg failed\r\n");
    	}
    }
}


#endif // CFG_USE_AUDIO
