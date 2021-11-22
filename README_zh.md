
# Tuya MCU SDK Arduino Library 

[English](./README.md) | [中文](./README_zh.md) 

Tuya MCU SDK Arduino Library 基于涂鸦ZigBee通用对接方案进行开发的，设备 MCU 通过串口与 ZigBee模组连接实现设备联网。**涂鸦模组中的固件应为通用固件，通用固件支持9600，115200两种波特率自适应。开发前请仔细阅读该文档。**  


+ 以下为模组配合ZigBee网关通信示意图。

  <img src="https://images.tuyacn.com/smart/Hardware_Developer/zigbee_Arduino/1.png" alt="1" style="zoom:67%;" />




## 文件介绍 

```bash
├── config.h	//配置文件，通过一些宏对mcu sdk内的功能进行添加和裁剪
├── examples	//例程存放文件夹(DataPointType例程对于涂鸦的数据类型进行了示例，具有较强的参考价值)
├── keywords.txt
├── library.properties
├── README.md
├── README_zh.md
└── src	                  //Tuya mcu sdk Arduino Library 存放文件夹 
    ├── TuyaZigbee.cpp	  //用户会使用到的API接口
    ├── TuyaDataPoint.cpp //对DP点相关操作的类
    ├── TuyaDataPoint.h
    ├── TuyaDefs.h	      //会用到的，用户不需改动的一些常量
    ├── TuyaZigbee.h
    ├── TuyaTools.cpp	  //MCU SDK会使用到的一些工具
    ├── TuyaTools.h
    ├── TuyaUart.cpp	  //对串口和数据缓存区处理的一些函数
    └── TuyaUart.h
```



## 重要函数 

在Arduino中使用该库进行编程开发时，必须在你的Arduino工程文件中包含 `TuyaZigbee.h` 头文件。



### 1、相关信息初始化

在使用Tuya IoT 云平台的时候，Product ID（简称：PID） 关联了产品具体的功能点、App 控制面板、出货信息等所有跟这个产品相关的信息。

`unsigned char TuyaZigbee::init(unsigned char *pid, unsigned char *mcu_ver)` ，PID通过在 [Tuya IoT平台](https://iot.tuya.com/?_source=97c44038fafc20e9c8dd5fdb508cc9c2) 上创建产品获取，Zigbee类的长度一般为8，`mcu_ver` 这个参数是软件的版本号，如果你的mcu想要支持OTA的话，该信息也是非常重要的。

> **提示**：当前版本的library暂不支持OTA功能。

```c
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
void setup() 
{   
  Serial.begin(9600);
  ...
  my_device.init("xxxxxxxx", "1.0.0");//"xxxxxxxx":应为你在Tuya IoT 平台上获取的的PID，"1.0.0"应是当前软件的实际版本号
      
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



### 2、DP信息传入MCU SDK 

在[Tuya IoT平台](https://iot.tuya.com/?_source=97c44038fafc20e9c8dd5fdb508cc9c2)创建产品，获取产品DP点信息。

DP为 Data Point 的缩写，即数据点，偶尔被称为 DP 点，表示智能设备所具备的功能点。

+ 涂鸦智能将每个功能点抽象成数据点，数据点定义成不同的类型，例如布尔、枚举、数值等。
+ 数据点具备读写属性。例如，一个两路的开关，可以抽象成两个数据点，每个数据点类型为布尔型，可取值为 `True` 或 `False`。
+ 数据点可读写，读表示获取开关目前的值，写表示改变开关目前的值。

DPID:指定通信协议下 DP 事件的 ID。



MCU SDK需要知道你创建了哪些DP点，他们是什么类型的，通过 `void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)` 函数传入MCU SDK。Tuya IoT 对于DP的类型分为6类：

```c
#define DP_TYPE_RAW     0x00    //RAW type
#define DP_TYPE_BOOL    0x01    //bool type
#define DP_TYPE_VALUE   0x02    //value type
#define DP_TYPE_STRING  0x03    //string type
#define DP_TYPE_ENUM    0x04    //enum type
#define DP_TYPE_BITMAP  0x05    //fault type
```

关于`void TuyaZigbee::set_dp_cmd_total(unsigned char dp_cmd_array[][2], unsigned char dp_cmd_num)` 函数，`dp_cmd_array[][2]` 为存储的信息的数组，`dp_cmd_num` 为DP总数。



这里我们假设有一个灯，它主要有3个功能也就是3个DP点，分别是:

+ 开关（DP ID : 1, DP 类型：布尔类型）
+ 灯的模式（DP ID : 2, DP 类型：枚举类型）
+ 灯光亮度（DP ID : 3, DP 类型：数值类型）

```c++
#include <TuyaZigbee.h>

TuyaZigbee my_device;
...
#define DPID_SWITCH_LED 1	//灯的开关DP
#define DPID_WORK_MODE 2	//灯的工作模式DP
#define DPID_BRIGHT_VALUE 3	//灯的亮度DP

//注意：array[][0]为DPID, array[][1]为DP type
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

### 3、配网模式设置

调用 `void TuyaZigbee::mcu_network_start(void)` 进入配网模式。

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

### 4、下发DP数据处理 

当云端下发数据后，需要对下发的数据进行解析，通过`unsigned char TuyaZigbee::mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len)` 这个函数进行解析。该函数目前只支持`DP_TYPE_BOOL`，`DP_TYPE_VALUE`，`DP_TYPE_ENUM` 这3种类型；`DP_TYPE_BITMAP` 为故障型一般只上报处理，不会下发，对该类型不用处理；`DP_TYPE_RAW` ,`DP_TYPE_STRING` 这两种类型需要用户自己进行处理。



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



### 5、DP下发处理函数注册 

当APP控制设备的时候，会从云端下发对应的DP命令到设备，设备对数据进行解析后，对下发的命令执行相对应的动作。

对下发命令的处理，通过回调函数来调用，所以我们需要把你的处理函数注册一下。调用这个`void TuyaZigbee::dp_process_func_register(tuya_callback_dp_download _func)` 对回调函数进行注册。

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
接着，我们可以在`xxx.ino`文件中使用处理函数。
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
  return SUCCESS;
}
```



### 6、上报设备状态 

上报设备状态也就是上报所有DP点值，也是通过注册函数的方式进行的；

接收 DataPoint 的 6 种不同DP类型的数据类型定义：

DP上报函数：

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

上报设备状态函数注册示例：

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



## 技术支持

您可以通过以下方法获得涂鸦的支持:

- 开发者中心：https://developer.tuya.com
- 帮助中心: https://support.tuya.com/help
- 技术支持工单中心: https://service.console.tuya.com 