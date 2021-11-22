/*
 * @FileName: config.h
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:27:41
 * @LastEditTime: 2021-11-18 18:05:27
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: Tuya mcu sdk Arduino library config file.
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "TuyaDefs.h"

#ifndef SUPPORT_MCU_FIRM_UPDATE
#define ZIGBEE_UART_RECV_BUF_LMT 128 //using to save data received from uart
#define ZIGBEE_DATA_PROCESS_LMT 256

#else

#endif

#define ZIGBEE_UART_SEND_BUF_LMT 128

#endif /* __CONFIG_H__ */
