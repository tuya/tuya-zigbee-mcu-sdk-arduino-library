/*
 * @FileName: Tuya.zigbee.h 
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:25:32
 * @LastEditTime: 2021-11-18 18:05:03
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: 
 */


#ifndef __TUYA_ZIGBEE_H__
#define __TUYA_ZIGBEE_H__

#include "../config.h"
#include "TuyaDefs.h"
#include "TuyaUart.h"

#ifdef TUYA_GLOBAL
#define TUYA_GLOBAL_EXTERN
#else
#define TUYA_GLOBAL_EXTERN extern
#endif

#ifndef TUYA_GLOBAL
extern TuyaUart tuya_uart;
#endif

#define PID_LEN 8
#define VER_LEN 5

TUYA_GLOBAL_EXTERN volatile unsigned char stop_update_flag; //ENABLE:Stop all data uploads   DISABLE:Restore all data uploads

#ifndef ZIGBEE_CONTROL_SELF_MODE
TUYA_GLOBAL_EXTERN volatile unsigned char reset_zigbee_flag;   //Reset zigbee flag (TRUE: successful / FALSE: failed)
TUYA_GLOBAL_EXTERN volatile unsigned char set_zigbeemode_flag; //Set the zigbee working mode flag (TRUE: Success / FALSE: Failed)
TUYA_GLOBAL_EXTERN volatile unsigned char zigbee_work_state;   //zigbee module current working status
#endif


extern "C"
{
    typedef void (*tuya_callback_dp_update_all)(void);
    typedef unsigned char (*tuya_callback_dp_download)(unsigned char dpid, const unsigned char value[], unsigned short length);
}

class TuyaZigbee
{
public:
    TuyaZigbee(void);
    TuyaZigbee(HardwareSerial *serial);
    TuyaZigbee(SoftwareSerial *serial);

    unsigned char init(unsigned char *pid, unsigned char *mcu_ver);
    void zigbee_uart_service(void);

    void dp_process_func_register(tuya_callback_dp_download _func);
    void dp_update_all_func_register(tuya_callback_dp_update_all _func);

    void set_dp_cmd_total(unsigned char download_cmd_array[][2], unsigned char download_cmd_num);

    unsigned long mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len);

    /* char * */
    unsigned char mcu_dp_update(unsigned char dpid, const unsigned char value[], unsigned short len); /* char raw */
    /* unsigned long / long */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned long value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, long value, unsigned short len);
    /* unsigned char / char */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned char value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, char value, unsigned short len);
    /* unsigned int / int */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned int value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, int value, unsigned short len);

    unsigned char mcu_get_zigbee_work_state(void);
    void mcu_network_start(void);
    void mcu_reset_zigbee(void);

private:

    unsigned char product_id[PID_LEN];
    unsigned char mcu_ver_value[VER_LEN];

    unsigned char download_dp_number;
    unsigned char (*download_cmd)[2];

    tuya_callback_dp_download dp_download_handle;
    tuya_callback_dp_update_all all_data_update;
    void data_handle(unsigned short offset);
    void product_info_update(void);
    void get_mcu_zigbee_mode(void);

    void seq_num_set(unsigned char seq_num);
    unsigned short seq_num_get(void);
    unsigned char data_point_handle(const unsigned char value[]);
    unsigned char get_dowmload_dpid_index(unsigned char dpid);
    unsigned char get_download_cmd_total(void);
    void zigbee_work_state_event(unsigned char zigbee_work_state);
    void mcu_reset_zigbee_event(unsigned char state);
};

#endif /* __TUYA_ZIGBEE_H__ */
