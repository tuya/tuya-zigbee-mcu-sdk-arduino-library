# Tuya MCU SDK Arduino Library

[English](./README.md) | [中文](./README_zh.md)

Tuya MCU SDK Arduino Library is developed based on the Tuya ZigBeegeneral integration solution. The device's MCU is connected to a ZigBee module through a serial port to implement a network connection. **The development is based on general firmware, which supports the adaptative 9600 and115200 baud rate. Please read this document carefully before development.**


+ The following is the schematic diagram of ZigBee single point communication with Bluetooth gateway.

  <img src="https://images.tuyacn.com/smart/Hardware_Developer/zigbee_Arduino/1.png" alt="1" style="zoom:67%;" />




## [Tuya Beta Test Program](https://pages.tuya.com/develop/ArduinoBetaTest_discord?_source=f21d8ebfe2cde74110e27b26366a81f3)
Now welcome to join the [Tuya Beta Test Program](https://pages.tuya.com/develop/ArduinoBetaTest_discord?_source=f21d8ebfe2cde74110e27b26366a81f3) to get your development gifts and make your own arduino projects with Tuya Support. Your feedback is helpful and valuable to the whole community.
![image](https://img-blog.csdnimg.cn/img_convert/e6f96d30bf98706723fe8c0de8653c8c.png)



## Document introduction

```bash
├── config.h     // Configuration file. Add and define features in the MCU SDK with macros.
├── examples   // The folder to save routines.
├── keywords.txt
├── library.properties
├── README.md
├── README_zh.md
└── src             // The folder to save Tuya MCU SDK Arduino Library.
    ├── TuyaZigbee.cpp // The APIs for users.
    ├── TuyaDataPoint.cpp // The class of DP operations.
    ├── TuyaDataPoint.h
    ├── TuyaDefs.h // Some constants.
    ├── TuyaZigbee.h
    ├── TuyaTools.cpp // Tools used by the MCU SDK.
    ├── TuyaTools.h
    ├── TuyaUart.cpp // Functions for serial communications and data buffer.
    └── TuyaUart.h
```



## Important functions

When you use this library for development with Arduino, you must add the header file `TuyaZigbee.h` in your Arduino project.

### 1. Initialization

Every product that is created on the Tuya IoT Platform will have a unique product ID (PID). The PID is associated with all information related to this product, including specific DP, app control panel, and delivery information.

In `unsigned char TuyaZigbee::init(unsigned char *pid, unsigned char *mcu_ver)`, the PID is obtained after you create a product on the [Tuya IoT Platform](http://iot.tuya.com/?_source=bcd157afd1c16c931b7b44381c9fe884). The PID of a BLE product is typically 8 bytes. The `mcu_ver` parameter is the version number of the software. Pay attention to this parameter if you want to support OTA updates of the MCU.
> **Note**: The current version of the library does not support the OTA feature.

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
void setup() 
{   
  Serial.begin(9600);
  ...
  my_device.init("xxxxxxxx", "1.0.0");// "xxxxxxxx": the PID on the Tuya IoT Platform. "1.0.0" is the default value. You can change "1.0.0" to the actual version number of the current software. 
              
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



### 2. Pass in the DP information to the MCU SDK

Create products on the [Tuya IoT Platform](http://iot.tuya.com/?_source=bcd157afd1c16c931b7b44381c9fe884) and get information on product DP points.

A data point (DP) represents a smart device function.

+ Tuya abstracts each function into a data point. DPs are defined in different data types, such as Boolean, enumeration, and integer.
+ DPs have read and write attributes. For example, a 2-gang switch has two Boolean DPs, and each DP has either a `True` or `False` value, which is readable and writable.
+ To read means to get the current value of the switch, and to write means to change the current value of the switch.

DPID: specifies the ID of a DP event under a communication protocol.

The MCU SDK needs to know which DPs you have created and what type they are. Pass them to the MCU SDK through the `void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)` function. 
The Tuya IoT Platform has six types of DPs:

```c
#define DP_TYPE_RAW     0x00    // Raw type
#define DP_TYPE_BOOL    0x01    // Boolean type
#define DP_TYPE_VALUE   0x02    // Numeric type
#define DP_TYPE_STRING  0x03    // String type
#define DP_TYPE_ENUM    0x04    // Enum type
#define DP_TYPE_BITMAP  0x05    // Fault type
```

In the `void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)` function, `dp_cmd_array[][2]` is the array that stores DP information, and `dp_cmd_num` is the total number of DPs.



Assume that a light has three functions, corresponding to three DPs as below:
* Switch (DP ID: 1, DP type: Boolean type).
* Light mode (DP ID: 2, DP type: enum type).
* Brightness (DP ID: 3, DP type: numeric type).


```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
#define DPID_SWITCH_LED 1 // The switch DP of the light.
#define DPID_WORK_MODE 2 // The working mode DP of the light.
#define DPID_BRIGHT_VALUE 3 // The brightness DP of the light.
    
// Note: array[][0] is DP ID, and array[][1] is DP type.
unsigned char dp_id_array[][2] = {
    /*  DPID     |  DP type  */
    {DPID_SWITCH, DP_TYPE_BOOL},  
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

### 3. Pairing mode setting

Call `void TuyaZigbee::mcu_network_start(void)` to enter the pairing mode.

```cc
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

### 4. Send and process DP data

After the cloud sends data, the sent data must be parsed through the `unsigned char TuyaBLE::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len)` function. 
Currently, this function only supports three types: `DP_TYPE_BOOL`, `DP_TYPE_VALUE`, and `DP_TYPE_ENUM`. `DP_TYPE_BITMAP` refers to the data of fault type, in which the data is only reported to the cloud. You do not need to handle this type. `DP_TYPE_RAW` and `DP_TYPE_STRING` must be implemented yourself.

```c
/**
 * @description: The MCU gets Boolean, numeric, and enum types to send DP value. (The data of the raw and string types shall be handled at the user's discretion. The data of the fault type can only be reported.)
 * @param {unsigned char} dpid: Data point (DP) ID
 * @param {const unsigned char} value: DP data buffer address
 * @param {unsigned short} len: Data length
 * @return {unsigned char} Parsed data
 */
unsigned char TuyaZigbee::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len);
```



### 5. Register a function to process DP sending

The app sends DP control commands to the device through the cloud. After data parsing, the device executes the specified actions accordingly.

A callback function is required to process the sent commands, so a processing function must be registered. We can call `void TuyaZigbee::dp_process_func_register(tuya_callback_dp_download _func)` to register the callback function.

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
void setup() 
{
    ...
  // Register DP download processing callback function
  my_device.dp_process_func_register(dp_process);
    ...
}
```

Then we can use processing function in the `xxx.ion` file.
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
      if (led_state ) {
        // Turn on 

      } else {
        // Turn off

      }
      // Status changes must be reported.
      my_device.mcu_dp_update(dpid, value, length);
    break;

    default:break;
  }
  return SUCCESS;
}
```



### 6. Report device status

Reporting the device status is to report the values of all DPs. It is also implemented through function registration.

Six data types of DPs are defined as follows:

DP reporting function:

```c
/**
 * @description: DP data upload
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} length
 * @return {*}
 */
unsigned char mcu_dp_update(unsigned char dpid, const unsigned char value[], unsigned short len);//update raw, string type
unsigned char mcu_dp_update(unsigned char dpid, unsigned long value, unsigned short len);
unsigned char mcu_dp_update(unsigned char dpid, unsigned int value, unsigned short len);
```


Example of registering a device status reporting function

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
  // Register DP download processing callback function
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
  my_device.mcu_dp_update(DPID_SWITCH_LED, led_state , 1);
}
```

## Technical Support

You can get support for Tuya by using the following methods:

- Developer Centre: https://developer.tuya.com?_source=d3b1d41903c59173453028c00b26eda6
- Help Centre: https://support.tuya.com/en/help?_source=9e55ab864ce95b016070141319a5206f
- Technical Support Work Order Centre: https://service.console.tuya.com?_source=5817a709f62789fbeb91c94062bf8993 

