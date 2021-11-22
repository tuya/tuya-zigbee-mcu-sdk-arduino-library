/*
 * @FileName: ZigbeeDataPointType.ino
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:26:50
 * @LastEditTime: 2021-11-22 09:51:26
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: 
 */


#include <SoftwareSerial.h>
#include "TuyaZigbee.h"
#include "TuyaUart.h"


TuyaZigbee my_device;
SoftwareSerial DebugSerial(8,9);

/* Current LED status */
unsigned char led_state = 0;

/* Connect network button pin */
int zigbee_key_pin = 7;

/* Data point define */
#define DPID_BOOL 1
#define DPID_VALUE 2
#define DPID_STRING 3
#define DPID_ENUM 4
#define DPID_RAW 0

/* Current device DP values */
unsigned char dp_bool_value = 0;
long dp_value_value = 0;
unsigned char dp_enum_value = 0;
unsigned char dp_string_value[8] = {"Hi,Tuya"};
unsigned char dp_raw_value[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

/* Stores all DPs and their types. PS: array[][0]:dpid, array[][1]:dp type. 
 *                                     dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
*/
const unsigned char dp_array[][2] = {
    {DPID_BOOL, DP_TYPE_BOOL},
    {DPID_VALUE, DP_TYPE_VALUE},
    {DPID_ENUM, DP_TYPE_ENUM},
    {DPID_STRING, DP_TYPE_STRING},
    {DPID_RAW, DP_TYPE_RAW},
};


unsigned char pid[] = {"kmsv4rij"};
unsigned char mcu_ver[] = {"1.0.0"};

/* last time */
unsigned long last_time = 0;

void setup()
{
    Serial.begin(9600);

    //Initialize led port, turn off led.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    //Initialize networking keys.
    pinMode(zigbee_key_pin, INPUT_PULLUP);
    DebugSeria.begin(9600);
    //Enter the PID and MCU software version
    my_device.init(pid, mcu_ver);
      //incoming all DPs and their types array, DP numbers
    my_device.set_dp_cmd_total(dp_array, 5);
    //register DP download processing callback function
    my_device.dp_process_func_register(dp_process);
    //register upload all DP callback function
    my_device.dp_update_all_func_register(dp_update_all);

    last_time = millis();
}



void loop()
{
  my_device.zigbee_uart_service();;

  //Enter the connection network mode when Pin6 is pressed.
  if ((digitalRead(zigbee_key_pin)  == HIGH)) {
    delay(80);
    if (digitalRead(zigbee_key_pin) == HIGH) {
      my_device.mcu_network_start();
    }
  }

  // /* LED blinks when network is being connected */
  if ((my_device.mcu_get_zigbee_work_state() != ZIGBEE_JOIN_GATEWAY) && (my_device.mcu_get_zigbee_work_state() != ZIGBEE_JOIN_ERROR)) {
    if (millis()- last_time >= 500) {
      last_time = millis();

      if (led_state == LOW) {
          led_state = HIGH;
      } else {
        led_state = LOW;
      }
      digitalWrite(LED_BUILTIN, led_state);
    }
  }

    delay(10);
}

/**
 * @description: DP download callback function.
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} length
 * @return {unsigned char}
 */
unsigned char dp_process(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  switch(dpid) {
        case DPID_BOOL:
            DebugSerial.println("Bool type:");
            dp_bool_value = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(dp_bool_value);
            /* After processing the download DP command, the current status should be reported. */
            led_state = dp_bool_value;
            if (led_state) {
                //Turn on
                digitalWrite(LED_BUILTIN, HIGH);
            } else {
                //Turn off
                digitalWrite(LED_BUILTIN, LOW);
            }
            my_device.mcu_dp_update(DPID_BOOL, dp_bool_value, 1);
        break;
        case DPID_VALUE:
            DebugSerial.println("Value type:");
            dp_value_value = my_device.mcu_get_dp_download_data(DPID_VALUE, value, length);
            DebugSerial.println(dp_value_value);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_VALUE, dp_value_value, 1);
        break;

        case DPID_ENUM:
            DebugSerial.println("Enum type:");
            dp_enum_value = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(dp_enum_value);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_ENUM, dp_enum_value, 1);
        break;

        case DPID_STRING:
            DebugSerial.println("String type:");
            /*  */
            for (unsigned int i=0; i<length; i++) {
                dp_string_value[i] = value[i];
                DebugSerial.write(dp_string_value[i]);
            }
            DebugSerial.println("");
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_STRING, dp_string_value, length);
        break;

        case DPID_RAW:
            DebugSerial.println("Raw type:");
            /*  */
            for (unsigned int i=0; i<length; i++) {
                dp_raw_value[i] = value[i];
                DebugSerial.write(dp_raw_value[i]);
            }
            DebugSerial.println("");
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_RAW, dp_raw_value, length);
        break;

    default:break;
  }
  return SUCCESS;
}

/**
 * @description: Upload all DP status of the current device.
 * @param {*}
 * @return {*}
 */
void dp_update_all(void)
{
    my_device.mcu_dp_update(DPID_BOOL, dp_bool_value, 1);
    my_device.mcu_dp_update(DPID_VALUE, dp_value_value, 1);
    my_device.mcu_dp_update(DPID_ENUM, dp_enum_value, 1);
    my_device.mcu_dp_update(DPID_STRING, dp_string_value, (sizeof(dp_string_value) / sizeof(dp_string_value[0])));
    my_device.mcu_dp_update(DPID_RAW, dp_raw_value, (sizeof(dp_raw_value) / sizeof(dp_raw_value[0])));
}
