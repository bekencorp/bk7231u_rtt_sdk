#ifndef _BK7011_CAL_PUB_H_
#define _BK7011_CAL_PUB_H_

#include "typedef.h"
#include "sys_config.h"
#define CFG_TEMP_DETECT_VERSION0    0U
#define CFG_TEMP_DETECT_VERSION1    1U

#define CFG_TEMP_DIFF_PWR_FREQOFFSET        1

#if (CFG_SOC_NAME == SOC_BK7231)
#define CFG_TEMP_DETECT_VERSION   CFG_TEMP_DETECT_VERSION0
#else
#define CFG_TEMP_DETECT_VERSION   CFG_TEMP_DETECT_VERSION1
#endif

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
typedef struct tmp_pwr_st {
    unsigned trx0x0c_12_15 : 4;
    signed p_index_delta : 6;
    signed p_index_delta_g : 6;
    signed p_index_delta_ble : 6;
    unsigned xtal_c_dlta : 6; 
} TMP_PWR_ST, *TMP_PWR_PTR;
#else
typedef struct tmp_pwr_st {
    UINT8 mod;
    UINT8 pa;
	UINT16 pwr_idx_shift;
} TMP_PWR_ST, *TMP_PWR_PTR;
#endif

struct temp_cal_pwr_st {
    UINT8 idx;
    UINT8 mode;
    INT16 shift;
    INT16 shift_g;
};

extern void calibration_main(void);
extern INT32 rwnx_cal_load_trx_rcbekn_reg_val(void);
extern void rwnx_cal_set_txpwr_by_rate(INT32 rate, UINT32 test_mode);
extern void rwnx_cal_set_txpwr_by_channel(UINT32 channel);
extern INT32 rwnx_cal_save_trx_rcbekn_reg_val(void);
extern void do_calibration_in_temp_dect(void);
extern void bk7011_cal_bias(void);
extern void bk7011_cal_dpll(void);
extern void rwnx_cal_set_txpwr(UINT32 pwr_gain, UINT32 grate);
extern UINT32 manual_cal_get_pwr_idx_shift(UINT32 rate, UINT32 bandwidth, UINT32 *pwr_gain);
extern int manual_cal_get_txpwr(UINT32 rate, UINT32 channel, UINT32 bandwidth, UINT32 *pwr_gain);
extern void manual_cal_save_txpwr(UINT32 rate, UINT32 channel, UINT32 pwr_gain);
extern UINT32 manual_cal_fitting_txpwr_tab(void);
extern void manual_cal_show_txpwr_tab(void);
extern UINT32 manual_cal_load_txpwr_tab_flash(void);
extern int manual_cal_save_txpwr_tab_to_flash(void);
extern int manual_cal_save_chipinfo_tab_to_flash(void);
extern UINT8 manual_cal_wirte_otp_flash(UINT32 addr, UINT32 len, UINT8 *buf);
extern UINT8 manual_cal_read_otp_flash(UINT32 addr, UINT32 len, UINT8 *buf);
extern UINT32 manual_cal_load_default_txpwr_tab(UINT32 is_ready_flash);
extern void manual_cal_set_dif_g_n40(UINT32 diff);
extern void manual_cal_set_dif_g_n20(UINT32 diff);
extern void manual_cal_get_current_temperature(void);
extern int manual_cal_write_macaddr_to_flash(UINT8 *mac_ptr);
extern int manual_cal_get_macaddr_from_flash(UINT8 *mac_ptr);
extern void manual_cal_show_otp_flash(void);
extern void manual_cal_clear_otp_flash(void);
extern void manual_cal_set_xtal(UINT32 xtal);
extern void manual_cal_set_lpf_iq(UINT32 lpf_i, UINT32 lpf_q);
extern void manual_cal_load_lpf_iq_tag_flash(void);
extern void manual_cal_load_xtal_tag_flash(void);
#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
void manual_cal_do_xtal_temp_delta_set(INT8 shift);
#endif
extern void manual_cal_do_xtal_cali(UINT16 cur_val, UINT16 *last, UINT16 thre, UINT16 init_val);
extern UINT32 manual_cal_get_xtal(void);
extern INT8 manual_cal_get_dbm_by_rate(UINT32 rate, UINT32 bandwidth);
extern INT8 manual_cal_get_cur_txpwr_dbm(void);
extern int manual_cal_load_temp_tag_from_flash(void);
extern int manual_cal_load_xtal_tag_from_flash(void);
extern void manual_cal_load_differ_tag_from_flash(void);

extern void bk7011_micopwr_config_tssi_read_prepare(void);
extern void bk7011_micopwr_tssi_read(void);
extern void bk7011_micopwr_tssi_show(void);
extern void rwnx_cal_set_reg_adda_ldo(UINT32 val);

extern void manual_cal_tmp_pwr_init(UINT16 init_temp, UINT16 init_thre, UINT16 init_dist);
extern void manual_cal_tmp_pwr_init_reg(UINT16 reg_mod, UINT16 reg_pa);
extern void manual_cal_temp_pwr_unint(void);
extern void manual_cal_set_tmp_pwr_flag(UINT8 flag);
extern TMP_PWR_PTR manual_cal_set_tmp_pwr(UINT16 cur_val, UINT16 thre, UINT16 *last);
extern UINT32 manual_cal_load_temp_tag_flash(void);
extern UINT32 manual_cal_load_adc_cali_flash(void);
extern void manual_cal_do_single_temperature(void);

extern void rwnx_cal_set_reg_mod_pa(UINT16 reg_mod, UINT16 reg_pa);
extern void rwnx_cal_do_temp_detect(UINT16 cur_val, UINT16 thre, UINT16 *last);
extern void rwnx_cal_set_lpfcap_iq(UINT32 lpfcap_i, UINT32 lpfcap_q);
extern void rwnx_cal_set_40M_extra_setting(UINT8 val);
extern void rwnx_cal_set_40M_setting(void);

extern void rwnx_cal_set_txpwr_for_ble_boardcast(void);
extern void rwnx_cal_recover_txpwr_for_wifi(void);

extern void rwnx_cal_initial_calibration(void);

extern UINT32 rwnx_tpc_pwr_idx_translate(UINT32 pwr_gain, UINT32 rate, UINT32 print_log );
extern UINT32 rwnx_tpc_get_pwridx_by_rate(UINT32 rate, UINT32 print_log);
extern void rwnx_use_tpc_set_pwr(void);
extern void rwnx_no_use_tpc_set_pwr(void);
extern UINT32 rwnx_is_tpc_bit_on(void);
extern UINT32 rwnx_sys_is_enable_hw_tpc(void);
extern void bk7011_set_rf_config_tssithred(int tssi_thred);
extern int bk7011_is_rfcali_mode_auto(void);
extern void bk7011_set_rfcali_mode(int mode);
extern void bk7011_cal_dcormod_show(void);

extern void rwnx_cal_ble_set_rfconfig(void);
extern void rwnx_cal_ble_recover_rfconfig(void);

#endif // _BK7011_CAL_PUB_H_
