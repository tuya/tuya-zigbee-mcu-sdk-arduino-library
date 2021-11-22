/*
 * @FileName: Tuya.cpp
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:25:32
 * @LastEditTime: 2021-11-18 18:04:40
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: The functions that the user needs to actively call are in this file.
 */

#define TUYA_GLOBAL

#include <Arduino.h>
#include "TuyaZigbee.h"
#include "TuyaTools.h"
#include "TuyaDataPoint.h"


TuyaTools tuya_tools;
TuyaUart tuya_uart;
TuyaDataPoint tuya_dp;

/* Constants required to report product information */
unsigned char pid_key[] = {"{\"p\":\""}; 
unsigned char mcu_ver_key[] = {"\",\"v\":\""};
unsigned char product_info_end[] = {"\"}"};


/* Protocol serial port initialization */
TuyaZigbee::TuyaZigbee(void)
{
    tuya_uart.set_serial(&Serial);
}
TuyaZigbee::TuyaZigbee(HardwareSerial *serial)
{
    tuya_uart.set_serial(serial);
}

TuyaZigbee::TuyaZigbee(SoftwareSerial *serial)
{
    tuya_uart.set_serial(serial);
}

/**
 * @description: Initialize product information
 * @param {unsigned char} *pid : Product ID(Create products on the Tuya IoT platform to get)
 * @param {unsigned char} *mcu_ver : MCU Software Version Number
 * @return {*}
 */
unsigned char TuyaZigbee::init(unsigned char *pid, unsigned char *mcu_ver)
{
    if (pid == NULL || mcu_ver == NULL)
    {
        return ERROR;
    }

    if (tuya_tools.my_strlen(pid) <= PID_LEN)
    {
        tuya_tools.my_memcpy(product_id, pid, tuya_tools.my_strlen(pid));

    }
    else
    {
        tuya_tools.my_memcpy(product_id, pid, PID_LEN);
        return ERROR;
    }

    if (tuya_tools.my_strlen(mcu_ver) <= VER_LEN)
    {
        tuya_tools.my_memcpy(mcu_ver_value, mcu_ver, tuya_tools.my_strlen(mcu_ver));
    }
    else
    {
        tuya_tools.my_memcpy(mcu_ver_value, mcu_ver, VER_LEN);
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @description: Zigbee serial port processing service
 * @param {*}
 * @return {*}
 */
void TuyaZigbee::zigbee_uart_service(void)
{

    unsigned char ret;
    static unsigned short rx_in = 0;
    unsigned short offset = 0;
    unsigned short rx_value_len = 0;
    unsigned short protocol_version = 0, protocol_head_len = PROTOCOL_HEAD;

    /* extract serial data */
    while(tuya_uart.available()) {
        ret = tuya_uart.uart_receive_input(tuya_uart.read());
        if (ret != SUCCESS) {
            break;
        }
    }

    while ((rx_in < sizeof(tuya_uart.zigbee_uart_rx_buf)) && (tuya_uart.get_queue_total_data() > 0)) {
        tuya_uart.zigbee_uart_rx_buf[rx_in++] = tuya_uart.Queue_Read_Byte();
    }
    int i = 0;
    if (rx_in < PROTOCOL_HEAD){

        return;

    }

    while ((rx_in - offset) >= PROTOCOL_HEAD)
    {
        if (tuya_uart.zigbee_uart_rx_buf[offset + HEAD_FIRST] != FIRST_FRAME_HEAD)
        {
            offset++;
            continue;
        }

        if (tuya_uart.zigbee_uart_rx_buf[offset + HEAD_SECOND] != SECOND_FRAME_HEAD)
        {
            offset++;
            continue;
        }

        protocol_version = tuya_uart.zigbee_uart_rx_buf[offset + PROTOCOL_VERSION];

       if(protocol_version != SERIAL_PROTOCOL_VER) /*version 2 is more than two byte  sequence number than version 1*/
        {
            offset += 2;
            continue;
        }

        rx_value_len = tuya_uart.zigbee_uart_rx_buf[offset + LENGTH_HIGH] * 0x100;
        rx_value_len += (tuya_uart.zigbee_uart_rx_buf[offset + LENGTH_LOW] + PROTOCOL_HEAD);


        if (rx_value_len > sizeof(tuya_uart.zigbee_uart_rx_buf))
        {
            offset += 3;
            continue;
        }

        if ((rx_in - offset) < rx_value_len)
        {
            break;
        }

        //数据接收完成
        if (tuya_tools.get_check_sum((unsigned char *)tuya_uart.zigbee_uart_rx_buf + offset, rx_value_len - 1) != tuya_uart.zigbee_uart_rx_buf[offset + rx_value_len - 1])
        {
            //校验出错
            offset += 3;
            continue;
        }
        data_handle(offset);
        offset += rx_value_len;

    } //end while

    rx_in -= offset;

    if(rx_in > 0){
		if((offset == 0)  && (rx_in >= sizeof(tuya_uart.zigbee_uart_rx_buf))){
           //如果缓存队列已满并且offset=0，此时队列已无法添加数据，但是接收数据长度不够，则丢掉第一个数据； 否则会陷入死循环
            offset++;
            rx_in -= offset;
        }
		tuya_tools.my_memcpy((char*)tuya_uart.zigbee_uart_rx_buf, (const char *)tuya_uart.zigbee_uart_rx_buf + offset, rx_in);
    }
}

/**
 * @description: Data frame processing
 * @param {unsigned short} offset : Data start position
 * @return {*}
 */
void TuyaZigbee::data_handle(unsigned short offset)
{
    unsigned char cmd_type = 0;
    unsigned short total_len = 0, seq_num = 0;
    unsigned short dp_len;  
  	unsigned char ret;
 	unsigned short i;

    cmd_type = tuya_uart.zigbee_uart_rx_buf[offset + FRAME_TYPE];

    total_len  = tuya_uart.zigbee_uart_rx_buf[offset + LENGTH_HIGH] * 0x100;
    total_len += tuya_uart.zigbee_uart_rx_buf[offset + LENGTH_LOW];

	seq_num = tuya_uart.zigbee_uart_rx_buf[offset + SEQ_HIGH] << 8;
    seq_num += tuya_uart.zigbee_uart_rx_buf[offset + SEQ_LOW];


    switch (cmd_type)
    {

    case PRODUCT_INFO_CMD: { //Product information

        product_info_update();

        }
        break;

    case ZIGBEE_STATE_CMD:{ //zigbee working status

        unsigned char current_state = tuya_uart.zigbee_uart_rx_buf[offset + DATA_START];
		zigbee_work_state_event(current_state);
        zigbee_work_state = tuya_uart.zigbee_uart_rx_buf[offset + DATA_START];
        tuya_uart.zigbee_uart_write_frame(ZIGBEE_STATE_CMD, 0);
    }
        break;

    case ZIGBEE_CFG_CMD:{

        mcu_reset_zigbee_event(tuya_uart.zigbee_uart_rx_buf[offset + DATA_START]);
    }
        break;

    case ZIGBEE_DATA_REQ_CMD:     //Order send

        for (i = 0; i < total_len;)
        {
            dp_len = tuya_uart.zigbee_uart_rx_buf[offset + DATA_START + i + 2] * 0x100;
            dp_len += tuya_uart.zigbee_uart_rx_buf[offset + DATA_START + i + 3];
    
            ret = data_point_handle((unsigned char *)tuya_uart.zigbee_uart_rx_buf + offset + DATA_START + i);

            if (SUCCESS == ret)
            {
                //Send success
            }
            else
            {
                //Send fault
            }

            i += (dp_len + 4);
        }
        break;
    case STATE_UPLOAD_SYNC_CMD:
         all_data_update();
    case STATE_UPLOAD_ASYNC_CMD:
        {
            unsigned char rsp_status = tuya_uart.zigbee_data_process_buf[offset + DATA_START];
            //zigbee_data_rsp_handle( rsp_status);
        }
        break;

    default:
        break;
    }
}


/**
 * @description: Input product All DP ID, Type, total number of DPs
 * @param {unsigned char} dp_cmd_array : DP array. array[][0] : DP ID, 
 *                                                 array[][1] : DP Type(DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP)
 * @param {unsigned char} dp_cmd_num : total number of DPs
 * @return {*}
 */
void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)
{
    download_dp_number = dp_cmd_num;
    download_cmd = dp_cmd_array;
}

/**
 * @description: Get the serial number of the DPID in the array.
 * @param {unsigned char} dpid
 * @return {unsigned char} index : The index of the input dpid in the array
 */
unsigned char TuyaZigbee::get_dowmload_dpid_index(unsigned char dpid)
{
    unsigned char index;
    unsigned char total = get_download_cmd_total();
    for (index = 0; index < download_dp_number; index++)
    {
        if (download_cmd[index][0] == dpid)
        {
            break;
        }
    }
    return index;
}

unsigned char TuyaZigbee::get_download_cmd_total(void)
{
   return(sizeof(download_cmd) / sizeof(download_cmd[0]));;
}


/**
* @description: received zigbee net_work state handle 
* @param[in] {zigbee_work_state}  zigbee current network state
* @return  void 
*/
void TuyaZigbee::zigbee_work_state_event(unsigned char zigbee_work_state)
{	
	unsigned short length= 0;
	tuya_uart.zigbee_uart_write_frame(ZIGBEE_STATE_CMD, length);	

	switch(zigbee_work_state){
		case ZIGBEE_NOT_JION:	

			break;
		
		case ZIGBEE_JOIN_GATEWAY:	

			break;
		
		case ZIGBEE_JOIN_ERROR:	

			break;
		
		case ZIGBEE_JOINING:	

			break;
		
		default:
			break;
	}
}


/**
* @description: received reset zigbee response 
* @param[in] {state} response state 
* @return  void 
*/
void TuyaZigbee::mcu_reset_zigbee_event(unsigned char state)
{	
	switch(state){
		case RESET_ZIGBEE_OK:
		
			break;
		
		case RESET_ZIGBEE_ERROR:
		
			break;
		
		default:
			break;
	}
}

/**
 * @description: Delivery data processing
 * @param {const unsigned char} value : Send data source pointer
 * @return Return data processing result
 */
unsigned char TuyaZigbee::data_point_handle(const unsigned char value[])
{
    unsigned char dp_id, index;
    unsigned char dp_type;
    unsigned char ret;
    unsigned short dp_len;

    dp_id = value[0];
    dp_type = value[1];
    dp_len = value[2] * 0x100;
    dp_len += value[3];

    index = get_dowmload_dpid_index(dp_id);

    if (dp_type != download_cmd[index][1])
    {
        //Error message
        return FALSE;
    }
    else
    {
        ret = dp_download_handle(dp_id, value + 4, dp_len);
    }

    return ret;
}

/**
 * @description: DP command processing callback function
 * @param {tuya_callback_dp_download} _func
 * @return {*}
 */
void TuyaZigbee::dp_process_func_register(tuya_callback_dp_download _func)
{
    dp_download_handle = _func;
}

/**
 * @description: Reply to the current device status callback function
 * @param {tuya_callback_dp_update_all} _func
 * @return {*}
 */
void TuyaZigbee::dp_update_all_func_register(tuya_callback_dp_update_all _func)
{
    all_data_update = _func;
}

/**
 * @description: Product information upload   产品信息获取（PID和版本号）
 * @param {*}
 * @return {*}
 */

void TuyaZigbee::product_info_update(void)
{
    unsigned char length = 0;

    length = tuya_uart.set_zigbee_uart_buffer(length, pid_key, (unsigned short)(tuya_tools.my_strlen(pid_key)));
    length = tuya_uart.set_zigbee_uart_buffer(length, product_id, PID_LEN);
    length = tuya_uart.set_zigbee_uart_buffer(length, mcu_ver_key, (unsigned short)(tuya_tools.my_strlen(mcu_ver_key)));
    length = tuya_uart.set_zigbee_uart_buffer(length, mcu_ver_value, VER_LEN);

    length = tuya_uart.set_zigbee_uart_buffer(length, product_info_end, tuya_tools.my_strlen(product_info_end));
    tuya_uart.zigbee_uart_write_frame(PRODUCT_INFO_CMD, length);

}

/**
 * @description: ZigBee module status reports a response
 * @param [in] {rsp_status}
 * @return {*}
 */
// void TuyaZigbee::zigbee_data_rsp_handle(unsigned char rsp_status)
// {
//     //用户自行处理
// }

/**
 * @description: Query the working mode of mcu and zigbee
 * @param {*}
 * @return {*}
 */
void TuyaZigbee::get_mcu_zigbee_mode(void)
{
    unsigned char length = 0;

#ifdef ZIGBEE_CONTROL_SELF_MODE //Module self-processing
    length = tuya_uart.set_zigbee_uart_byte(length, ZIGBEE_STATE_KEY);
    length = tuya_uart.set_zigbee_uart_byte(length, ZIGBEE_RESERT_KEY);
#else
    //No need to process data
#endif

    //tuya_uart.zigbee_uart_write_frame(WORK_MODE_CMD, MCU_TX_VER, length);
}

/**
 * @description: mcu gets bool,value,enum type to send dp value. (raw, string type needs to be handled at the user's discretion. fault only report)
 * @param {unsigned char} dpid : data point ID 
 * @param {const unsigned char} value : dp data buffer address 
 * @param {unsigned short} len : data length
 * @return {unsigned char} Parsed data
 */
unsigned long TuyaZigbee::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len)
{
    unsigned long ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
    case DP_TYPE_BOOL:
        ret = tuya_dp.mcu_get_dp_download_bool(value, len);
        break;

    case DP_TYPE_VALUE:
        ret = tuya_dp.mcu_get_dp_download_value(value, len);
        break;

    case DP_TYPE_ENUM:
        ret = tuya_dp.mcu_get_dp_download_enum(value, len);
        break;

    default:
        break;
    }
    return ret;
}

/**
 * @description: dp data upload
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} len
 * @return {*}
 */
unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, const unsigned char value[], unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_RAW:
            ret = tuya_dp.mcu_dp_raw_update(dpid, value, len);
        break;

        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, *value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, *value);
        break;

        case DP_TYPE_STRING:
            ret = tuya_dp.mcu_dp_string_update(dpid, value, len);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, *value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, *value);
        break;


        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, unsigned char value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, char value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, unsigned long value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, long value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, unsigned int value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}

unsigned char TuyaZigbee::mcu_dp_update(unsigned char dpid, int value, unsigned short len)
{
    unsigned char ret;
    switch (download_cmd[get_dowmload_dpid_index(dpid)][1])
    {
        case DP_TYPE_BOOL:
            ret = tuya_dp.mcu_dp_bool_update(dpid, value);
        break;

        case DP_TYPE_ENUM:
            ret = tuya_dp.mcu_dp_enum_update(dpid, value);
        break;

        case DP_TYPE_VALUE:
            ret = tuya_dp.mcu_dp_value_update(dpid, value);
        break;

        case DP_TYPE_FAULT:
            ret = tuya_dp.mcu_dp_fault_update(dpid, value);
        break;

        default:
            break;
    }
    return ret;
}


/**
 * @description: mcu start zigbee module    
 * @param 

 * @return {*}
 */
void TuyaZigbee::mcu_network_start(void)
{
      unsigned short length = 0;
      length =  tuya_uart.set_zigbee_uart_byte(length,1);
      tuya_uart.zigbee_uart_write_frame(ZIGBEE_CFG_CMD,length);
}


/**
 * @description: The MCU actively obtains the current zigbee working status.
 * @param {*}
 * @return {unsigned char} zigbee work state

 */
unsigned char TuyaZigbee::mcu_get_zigbee_work_state(void)
{
    return zigbee_work_state;
}


/**
 * @description: MCU actively resets zigbee working mode
 * @param {*}
 * @return {*}
 * @note   1:The MCU actively calls to obtain whether the reset zigbee is successful through the mcu_get_reset_zigbee_flag() function.
 *         2:If the module is in self-processing mode, the MCU does not need to call this function.
 */
void TuyaZigbee::mcu_reset_zigbee(void)
{
   unsigned short length = 0;

   length =  tuya_uart.set_zigbee_uart_byte(length,0);
   tuya_uart.zigbee_uart_write_frame(ZIGBEE_CFG_CMD, length);
}
