/*
 * @FileName: TuyaUart.cpp
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:25:32
 * @LastEditTime: 2021-11-18 18:03:45
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: Tuya mcu sdk Arduino library about uart buffer, data receiving and sending.
 */
#include <Arduino.h>
#include "TuyaZigbee.h"
#include "TuyaUart.h"
#include "TuyaTools.h"

extern TuyaTools tuya_tools;
static volatile unsigned short global_seq_num;

TuyaUart::TuyaUart(void)
{
    zigbee_protocol_init();
}

TuyaUart::~TuyaUart(void)
{
}

/**
* @description: get frame seq_num
* @param[in] {void}
* @return seq_num  
*/
void TuyaUart::seq_num_set(unsigned char seq_num)
{
   global_seq_num = seq_num;
}
unsigned short TuyaUart::seq_num_get(void)
{
    return global_seq_num;
}


/**
* @description: read data from the queue
* @param[in] {void}
* @return void
*/
unsigned char  TuyaUart::get_queue_total_data(void)
{
    if(queue_total_data == 0)
    {
        tuya_uart.zigbee_protocol_init();
    }

    return (queue_total_data);
}

/**
* @description: encapsulates a generic send function, developer should use their own function to completing this fuction 
* @param[in] {value} send signle data 
* @return  void
*/
void TuyaUart::uart_transmit_output(unsigned char value)
{
    Serial.write(value);
}

/**
* @description: copy receive data from uart receive interrupt
* @param[in]  {value} Data received from interrupt
* @return void 
*/
unsigned char TuyaUart::uart_receive_input(unsigned char data)
{
    if (1 == queue_out - queue_in)
    {
        //UART receive buffer is full
        return ERROR;
    }
    else if ((queue_in > queue_out) && ((queue_in - queue_out) >= sizeof(tuya_uart.zigbee_data_process_buf)))
    {
        //UART receive buffer is full
        return ERROR;
    }
    else
    {
        //UART receive buffer is not full
        if (queue_in >= (unsigned char *)(tuya_uart.zigbee_data_process_buf + sizeof(tuya_uart.zigbee_data_process_buf)))
        {
            queue_in = (unsigned char *)(tuya_uart.zigbee_data_process_buf);
        }

        *queue_in++ = data;
         queue_total_data++;
         return SUCCESS;
    }
}


void TuyaUart::uart_receive_buff_input(unsigned char value[], unsigned short data_len)
{
    unsigned short i = 0;
    for (i = 0; i < data_len; i++)
    {
        uart_receive_input(value[i]);
    }
}

/**
* @description: init paramter and set rx_sbuf with zero
* @param[in]  {void}
* @return void 
*/
void TuyaUart::zigbee_protocol_init(void)
{
    queue_in = (unsigned char *)zigbee_data_process_buf;
    queue_out = (unsigned char *)zigbee_data_process_buf;
    queue_total_data = 0;

}

/**
* @description: read byte from zigbee_queue_buf array
* @param[in]  {void}
* @return read byte value
*/
unsigned char TuyaUart::Queue_Read_Byte(void)
{
    unsigned char date;

    if(queue_total_data > 0)
    {
        //With data
        if (queue_out >= (unsigned char *)(tuya_uart.zigbee_data_process_buf + sizeof(tuya_uart.zigbee_data_process_buf)))
        {
            //The data has reached the end
            queue_out = (unsigned char *)(tuya_uart.zigbee_data_process_buf);
        }

        date = *queue_out++;
        queue_total_data--;
    }
    return date;
}

/**
* @description: check rx buffer is empty or not
* @param[in]  {void}
* @return the queue state
*/
unsigned char TuyaUart::with_data_rxbuff(void)
{

    if(queue_out != queue_in)
        return 1;
    else
        return 0;

}

/**
* @description: padding a byte in send buf base on dest
* @param[in] {dest} the location of padding byte 
* @param[in] {byte} padding byte 
* @return  sum of frame after this operation 
*/
unsigned short TuyaUart::set_zigbee_uart_byte(unsigned short dest, unsigned char byte)
{
    unsigned char *obj = (unsigned char *)tuya_uart.zigbee_uart_tx_buf + DATA_START + dest;

    *obj = byte;
    dest += 1;

    return dest;
}

/**
* @description: padding buf in send buf base on dest
* @param[in] {dest} the location of padding 
* @param[in] {src}  the head address of padding buf
* @param[in] {len}  the length of padding buf
* @return  sum of frame after this operation 
*/
unsigned short TuyaUart::set_zigbee_uart_buffer(unsigned short dest, const unsigned char *src, unsigned short len)
{
    unsigned char *obj = (unsigned char *)tuya_uart.zigbee_uart_tx_buf + DATA_START + dest;

    tuya_tools.my_memcpy(obj, src, len);

    dest += len;
    return dest;
}

/**
* @description: send len bytes data
* @param[in] {in} the head address of send data
* @param[in] {len}  the length of send data
* @return  void
*/
void TuyaUart::zigbee_uart_write_data(unsigned char *in, unsigned short len)
{
    if ((NULL == in) || (0 == len))
    {
        return;
    }

    while (len--)
    {
        uart_transmit_output(*in);
        in++;
    }
}

/**
* @description: send a frame data
* @param[in] {fr_cmd} frame cmd id
* @param[in] {len}    len of frame data 
* @return  void
*/
void TuyaUart::zigbee_uart_write_frame(unsigned char fr_type, unsigned short len)
{
    unsigned char check_sum = 0;
    unsigned short seq_num = seq_num_get();

    zigbee_uart_tx_buf[HEAD_FIRST] = FIRST_FRAME_HEAD;
    zigbee_uart_tx_buf[HEAD_SECOND] = SECOND_FRAME_HEAD;
    zigbee_uart_tx_buf[PROTOCOL_VERSION] = SERIAL_PROTOCOL_VER;

    zigbee_uart_tx_buf[SEQ_HIGH] = seq_num >> 8 ;
    zigbee_uart_tx_buf[SEQ_LOW] = seq_num & 0xff;

    zigbee_uart_tx_buf[FRAME_TYPE] = fr_type;
    zigbee_uart_tx_buf[LENGTH_HIGH] = len >> 8;
    zigbee_uart_tx_buf[LENGTH_LOW] = len & 0xff;

    seq_num++;
	seq_num_set(seq_num);
    len += PROTOCOL_HEAD;
    check_sum = tuya_tools.get_check_sum((unsigned char *)zigbee_uart_tx_buf, len - 1);
    zigbee_uart_tx_buf[len - 1] = check_sum;

    zigbee_uart_write_data((unsigned char *)zigbee_uart_tx_buf, len);
}


void TuyaUart::set_serial(HardwareSerial *serial)
{
    _isHWSerial = TRUE;
    _port = serial;
}

void TuyaUart::set_serial(SoftwareSerial *serial)
{
    _isHWSerial = FALSE;
    _port = serial;
}

void TuyaUart::begin(long baud_rate)
{
    if (_isHWSerial) {
        static_cast<HardwareSerial*>(_port)->begin(baud_rate);
    } else {
        static_cast<SoftwareSerial*>(_port)->begin(baud_rate);
    }
}

char TuyaUart::read(void)
{
    if (_isHWSerial) {
        return static_cast<HardwareSerial*>(_port)->read();
    } else {
        return static_cast<SoftwareSerial*>(_port)->read();
    }
}

size_t TuyaUart::write(char value)
{
    if (_isHWSerial) {
        return static_cast<HardwareSerial*>(_port)->write(value);
    } else {
        return static_cast<SoftwareSerial*>(_port)->write(value);
    }
}

int TuyaUart::available(void)
{
    if (_isHWSerial) {
        return static_cast<HardwareSerial*>(_port)->available();
    } else {
        return static_cast<SoftwareSerial*>(_port)->available();
    }
}
