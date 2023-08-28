# Tuya Zigbee MCU SDK Arduino Library

[English](./README.md) | [中文](./README_zh.md)

Tuya MCU SDK Arduino Library is built on top of Tuya's Zigbee generic solution that enables your product to connect to the cloud through the serial communication between your MCU and Tuya's Zigbee module. The module is flashed with the generic firmware and supports auto-baud detection for 9600 and 115200. Please read this document carefully before development.


+ The following diagram shows how the Zigbee module communicates with the cloud with a Zigbee gateway.

  <img src="https://images.tuyacn.com/smart/docs/Diagram.png" width="25%">

## [Tuya Beta Test Program](https://pages.tuya.com/develop/ArduinoBetaTest_discord?_source=f21d8ebfe2cde74110e27b26366a81f3)
Now welcome to join the [Tuya Beta Test Program](https://pages.tuya.com/develop/ArduinoBetaTest_discord?_source=f21d8ebfe2cde74110e27b26366a81f3) to get your development gifts and make your own Arduino projects with Tuya Support. Your feedback is helpful and valuable to the whole community.

![image](https://img-blog.csdnimg.cn/img_convert/e6f96d30bf98706723fe8c0de8653c8c.png)

## File introduction

```bash
├── config.h	 // Configuration file. You can add or configure required features by editing the macro.
├── examples	 // Some samples help you understand the logic and get started with your project.
├── keywords.txt
├── library.properties
├── README.md
├── README_zh.md
└── src	                   //  Tuya MCU SDK Arduino Library.
    ├── TuyaZigbee.cpp    //  APIs.
    ├── TuyaDataPoint.cpp // Classes of data point  (DP) operations.
    ├── TuyaDataPoint.h
    ├── TuyaDefs.h	       // Some constants.
    ├── TuyaZigbee.h
    ├── TuyaTools.cpp	   // Some tools.
    ├── TuyaTools.h
    ├── TuyaUart.cpp	   // Serial data handlers.
    └── TuyaUart.h
```



## Important functions

When you use this library for development with Arduino, you must add the header file `TuyaZigbee.h` in your project.

### Initialization

[Tuya IoT Development Platform](https://iot.tuya.com/) assigns each product a unique product ID (PID). The PID is associated with all information related to this product, including defined DPs, app control panel, and delivery information.

Take `unsigned char TuyaZigbee::init(unsigned char *pid, unsigned char *mcu_ver)` as an example. `pid` can be obtained from the [Tuya IoT Development Platform](https://iot.tuya.com/) by creating a product. For Zigbee products, the PID is an 8-digit alphanumeric code. `mcu_ver` represents the MCU version number that an OTA update relies on.

> **Note**: The current version of the library does not support the OTA feature.

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
void setup()
{   
  Serial.begin(9600);
  ...
  my_device.init("xxxxxxxx", "1.0.0");// "xxxxxxxx" is the PID. `1.0.0` should be replaced with the real MCU firmware version number.
      
  ...
}

void loop()
{
  ...
  my_device.zigbee_uart_service();
  ...
}

...
```


### Pass in DP data to the MCU SDK

When you create a product on the [Tuya IoT Development Platform](https://iot.tuya.com/?_source=97c44038fafc20e9c8dd5fdb508cc9c2), you can define the required DPs.

Each product feature defined on the platform is described as a DP.

+ Each DP has its data type such as Boolean, enum, and value.
+ A DP has a read/write property. For example, a 2-gang switch has two Boolean DPs, and the valid value of each DP can be either `True` or `False`.
+ The read property means to get the current value of a DP. The write property means to change the current value of a DP.

DP ID: represents the ID of a DP event under a communication protocol.


`void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)` function is used to pass in the information of your defined DPs to the MCU. Six data types are available:

```c
#define DP_TYPE_RAW     0x00    // RAW type
#define DP_TYPE_BOOL    0x01    // Boolean  type
#define DP_TYPE_VALUE   0x02    // Value type
#define DP_TYPE_STRING  0x03    // String type
#define DP_TYPE_ENUM    0x04    // Enum type
#define DP_TYPE_BITMAP  0x05    // Fault type
```

For `void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)`, `dp_cmd_array[][2]` indicates the array for information storage and `dp_cmd_num` indicates the total number of DPs.

Assume that a smart light has three DPs.

+ On/off: Its DP ID is 1 and data type is Boolean.
+ Light mode: Its DP ID is 2 and data type is enum.
+ Brightness: Its DP ID is 3 and data type is value.

```c++
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
#define DPID_SWITCH_LED 1	 // The DP of on/off control.
#define DPID_WORK_MODE 2  // The DP of light mode.
#define DPID_BRIGHT_VALUE 3  // The DP of brightness.

// `array[][0]` is DP ID, and `array[][1]` is DP type.
unsigned char dp_id_array[][2] = {
    /*  DPID     |  DP type  */
    {DPID_SWITCH_LED, DP_TYPE_BOOL},  
    {DPID_WORK_MODE, DP_TYPE_ENUM},
    {DPID_BRIGHT_VALUE, DP_TYPE_VALUE},
};
...
void setup() 
{
    ...
    my_device.set_dp_cmd_total(dp_id_array, 3);	
    ...
}
```

### Pairing mode setting

Call `void TuyaZigbee::mcu_network_start(void)` to enable the module to enter pairing mode.

```c++
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

```

### DP data processing

The function `unsigned char TuyaZigbee::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len)` is used to parse the DP data received from the cloud. It only supports three data types, `DP_TYPE_BOOL`, `DP_TYPE_VALUE`, and `DP_TYPE_ENUM`. `DP_TYPE_BITMAP` is the fault type, which is for reporting only. The handler for `DP_TYPE_RAW` and `DP_TYPE_STRING` needs to be implemented by you.


```c
/**
 * @description: mcu gets bool,value,enum type to send dp value. (raw, string type needs to be handled at the user's discretion. fault only report)
 * @param {unsigned char} dpid : data point ID 
 * @param {const unsigned char} value : dp data buffer address 
 * @param {unsigned short} len : data length
 * @return {unsigned char} Parsed data
 */
unsigned char TuyaZigbee::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len);
```


### Register DP data handler

The cloud sends the control command received from the mobile app to the device. Through data parsing, the device executes the command accordingly.

You need a callback to handle the received DP data. Call `void TuyaZigbee::dp_process_func_register(tuya_callback_dp_download _func)` to register the callback.

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...

void setup() 
{
    ...
  //register DP download processing callback function
  my_device.dp_process_func_register(dp_process);
    ...
}
```
You can use the handler in the `xxx.ino` file.
```c
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
    case DPID_SWITCH_LED:
      led_state = my_device.mcu_get_dp_download_data(dpid, value, length);
      if (led_state) {
        //Turn on

            } else {
        //Turn off

      }
      //Status changes should be reported.
      my_device.mcu_dp_update(dpid, value, length);
    break;

    default:break;
  }
  return TY_SUCCESS;
}
```

### Report device status

The device reports the current status of all DPs. You need to register functions to implement status reporting.

Six data types of DPs are defined as follows:

Report DP status:

```c
/**
 * @description: dp data upload
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} len
 * @return {*}
 */
unsigned char mcu_dp_update(unsigned char dpid, const unsigned char value[], unsigned short len);//update raw, string type
unsigned char mcu_dp_update(unsigned char dpid, unsigned long value, unsigned short len);
unsigned char mcu_dp_update(unsigned char dpid, unsigned int value, unsigned short len);
```

Example:

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;

/* Data point define */
#define DPID_SWITCH_LED 1

/* Current LED status */
unsigned char led_state = 0;
...
void setup()
{
    ...
  //register DP download processing callback function
  my_device.dp_update_all_func_register(dp_update_all);
    ...
}

/**
 * @description: Upload all DP status of the current device.
 * @param {*}
 * @return {*}
 */
void dp_update_all(void)
{
  my_device.mcu_dp_update(DPID_SWITCH_LED , led_state, 1);
}
```

## Technical support

You can get support from Tuya with the following methods:

+ [Tuya Developer Platform](https://developer.tuya.com/en/)
+ [Help Center](https://support.tuya.com/en/help)
+ [Service & Support](https://service.console.tuya.com)
