/*
 * @FileName: TuyaDefs.h 
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-11-16 11:25:32
 * @LastEditTime: 2021-11-18 18:02:30
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: Some necessary constant definitions. 
 */
#ifndef __TUYA_DEFS_H__
#define __TUYA_DEFS_H__

/* Define constant */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL   ((void *)0)
#endif 
#endif

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef ERROR
#define ERROR 0
#endif

#ifndef INVALID
#define INVALID 0xFF
#endif

#ifndef ENABLE
#define ENABLE 1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

//=============================================================================
// frme data define 
//=============================================================================
#define SERIAL_PROTOCOL_VER                 0x02                               // the version of frame 
#define PROTOCOL_HEAD                       0x09                               // the lcation of frame length except for tail
#define FIRST_FRAME_HEAD                    0x55                               // first byte of frame 
#define SECOND_FRAME_HEAD                   0xaa                               // second byte of frame 

//=============================================================================
//Byte order of the frame   帧的字节顺序
//=============================================================================
#define         HEAD_FIRST                      0
#define         HEAD_SECOND                     1
#define         PROTOCOL_VERSION                2
#define         SEQ_HIGH                        3
#define         SEQ_LOW                         4
#define         FRAME_TYPE                      5
#define         LENGTH_HIGH                     6
#define         LENGTH_LOW                      7
#define         DATA_START                      8

//=============================================================================
//Data frame type
//=============================================================================
#define         ZIGBEE_FACTORY_NEW_CMD          0                               //Heartbeat package
#define         PRODUCT_INFO_CMD                1                               //Product information
#define         ZIGBEE_STATE_CMD                2                                 //zigbee工作状态	
#define         ZIGBEE_CFG_CMD                  3                               //配置zigbee模块
#define         ZIGBEE_DATA_REQ_CMD             4                              //zigbee数据下发命令
#define         STATE_UPLOAD_SYNC_CMD           5                                 //MCU状态主动上报    同步
#define         STATE_UPLOAD_ASYNC_CMD          6                                //MCU状态被动上报    异步
#define         FUNC_TEST_CMD                   8                              //zigbee模块功能测试

//=============================================================================
//define communication state
//=============================================================================
#define ZIGBEE_NOT_JION							        0x00	    //zigbee module not jion network
#define ZIGBEE_JOIN_GATEWAY								  0x01	    //zigbee module had jioned network 
#define ZIGBEE_JOIN_ERROR								    0x02	    //zigbee module network state error
#define ZIGBEE_JOINING							    	  0x03	    //zigbee module jioning 

//=============================================================================
//ZIGBEE reset status
//=============================================================================
#define RESET_ZIGBEE_OK			               0x00	       //rest zigbee success
#define RESET_ZIGBEE_ERROR  	             0x01	       //reset zigbee error

//=============================================================================
// dp data point type
//=============================================================================
#define         DP_TYPE_RAW                     0x00				 //RAW
#define         DP_TYPE_BOOL                    0x01	       //bool
#define         DP_TYPE_VALUE                   0x02	       //value
#define         DP_TYPE_STRING                  0x03				 //string
#define         DP_TYPE_ENUM                    0x04			 	 //enum 
#define         DP_TYPE_BITMAP                  0x05			 	 //fault
#define         DP_TYPE_FAULT                   DP_TYPE_BITMAP    



#endif /* __TUYA_DEFS_H__ */