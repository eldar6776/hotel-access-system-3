/**
 ******************************************************************************
 * File Name          : command.h
 * Date               : 28/02/2016 23:16:19
 * Description        : command parser and execute module header
 ******************************************************************************
 *
 *
 *
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMAND_H__
#define __COMMAND_H__					RC010319	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"


/* Exported Type  ------------------------------------------------------------*/
/**
  * @brief  
  * @param  
  * @retval 
  */
typedef enum
{
	PACKET_INIT = 0U,   // init state
    PACKET_START,       // wait address mark
    PACKET_RECEPINET,   // wait recepint address
    PACKET_SENDER,      // wait sender address received
    PACKET_SIZE,        // wait data size
    PACKET_PAYLOAD,     // wait data payload 
    PACKET_CRC8,        // 
    PACKET_END
}PACKET_TypeTypeDef;   // packet current processing state
/**
  * @brief  
  * @param  
  * @retval 
  */
typedef enum 
{
	LINK_INIT = 0U,     // init state of enum
	LINK_P2P,           // link between sender and recepient, full link controll sender and recepient in use
	LINK_GROUP,         // sender to menu listener with same group address, link controll on sender side
	LINK_BROADCAST,     // sender to all listener with same broadcast address, link controll on sender side
}PACKET_LinkTypeDef;    // packet link addressing type   
/**
  * @brief  
  * @param  
  * @retval 
  */
typedef enum                
{
	SOURCE_INIT = 0U,   // init state of enum
    SOURCE_RADIO,       // packet source radio control driver
	SOURCE_RS485,       // packet source rs485 function set
    SOURCE_WIFI,        // packet source wifi radio control
    SOURCE_LAN,         // packet source lan mac and ip stack
    SOURCE_USB,         // packet source usb control
    SOURCE_COM,         // packet source usart function set
    SOURCE_KNX,         // packet source knx driver
    SOURCE_X10,         // packet source x10 plc driver
    SOURCE_NFC,         // packet source nfc wirelles serial driver
    SOURCE_IR,          // packet source infra-red function set
    SOURCE_OW           // packet source onewire function set
}PACKET_SourceTypeDef;  // packet callback source reference
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef enum
//{
//    ERROR_INIT = 0U,    // initialized and error free state of packet
//    ERROR_TIMEOUT,      // timeout occured, reset packet and start over
//    ERROR_HDR_CTRL,     // header invalid ascii control characters
//    ERROR_HDR_FRMT,     // header invalid ascii characters
//    ERROR_HDR_CRC,      // header invalid crc8 value
//    ERROR_DAT_CTRL,     // data invalid ascii control characters
//    ERROR_DAT_FRMT,     // data invalid ascii characters
//    ERROR_DAT_CRC,      // data invalid crc8 value
//    ERROR_CMD,          // command invalid or not supported
//    ERROR_CMD_PAR,      // command parameter invalid value 
//    ERROR_LINK,         // wrong packet link type
//    ERROR_SOURCE,       // wrong packet data source
//    ERROR_STATE         // wrong packet state
//}PACKET_ErrorTypeDef;   // packet error state
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    uint16_t next;                  // next package number for link
//    uint16_t actual;                // current linked and stored packet number 
//    uint16_t total;                 // total number of packages to link
//}PACKET_NumerationTypeDef;
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    uint16_t rxsz;                  // size of data in received data buffer
//    uint16_t txsz;                  // size of data in transmission data buffer
//    uint8_t* prxb;                  // pointer to received data buffer
//    uint8_t* ptxb;                  // pointer to transmission data buffer    
//}PACKET_StorageTypeDef;
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    uint16_t rxtmr;                 // receiving timer compared to systick
//    uint16_t rxtime;                // receiving time limit for packet process
//    uint16_t txtmr;                 // transmission timer compared to systick
//    uint16_t txtime;                // transmission time limit for packet process
//}PACKET_TimersTypeDef;
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    uint16_t rcv;                   // address of last link recepient
//    uint16_t snd;                   // address of last link sender    
//}PACKET_AddressTypeDef;
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    uint8_t cmd;                    // received command
//    uint8_t par;                    // received parameter
//    uint16_t dsz;                   // received data size 
//    uint8_t res;                    // response
//    uint8_t rva;                    // response value
//}PACKET_ValueTypeDef;
///**
//  * @brief  
//  * @param  
//  * @retval 
//  */
//typedef struct
//{
//    PACKET_TypeTypeDef type;        // packet type
//    PACKET_ValueTypeDef val;        // value        
//    PACKET_AddressTypeDef add;      // addressing 
//    PACKET_TimersTypeDef tmr;       // process timers 
//    PACKET_StorageTypeDef buf;      // data storage buffer      
//    PACKET_NumerationTypeDef no;    // packet numeration
//    PACKET_LinkTypeDef lnk;         // link addressing type
//    PACKET_SourceTypeDef src;       // driver callback reference
//    PACKET_ErrorTypeDef err;        // keep error for log and diagnose
//}PACKET_TypeDef;                    // packet data structure hold received data


/* Exported variables  -------------------------------------------------------*/
extern uint8_t sys_status;
extern uint8_t activ_command;
extern uint8_t fw_update_status;


/* Exported Define  ----------------------------------------------------------*/
/** ==========================================================================*/
/**                 C O M M A N D				L I S T				          */
/** ==========================================================================*/
#define COPY_NEW_DISP_IMG                   ((uint8_t)0x63U)
#define DOWNLD_DISP_IMG_1 		            ((uint8_t)0x64U)
#define DOWNLD_DISP_IMG_2 		            ((uint8_t)0x65U)
#define DOWNLD_DISP_IMG_3		            ((uint8_t)0x66U)
#define DOWNLD_DISP_IMG_4                   ((uint8_t)0x67U)
#define DOWNLD_DISP_IMG_5		            ((uint8_t)0x68U)
#define DOWNLD_DISP_IMG_6		            ((uint8_t)0x69U)
#define DOWNLD_DISP_IMG_7		            ((uint8_t)0x6aU)
#define DOWNLD_DISP_IMG_8		            ((uint8_t)0x6bU)
#define DOWNLD_DISP_IMG_9		            ((uint8_t)0x6cU)
#define DOWNLD_DISP_IMG_10		            ((uint8_t)0x6dU)
#define DOWNLD_DISP_IMG_11		            ((uint8_t)0x6eU)
#define DOWNLD_DISP_IMG_12		            ((uint8_t)0x6fU)
#define DOWNLD_DISP_IMG_13		            ((uint8_t)0x70U)
#define DOWNLD_DISP_IMG_14		            ((uint8_t)0x71U)
#define DOWNLD_DISP_IMG_15		            ((uint8_t)0x72U)
#define DOWNLD_DISP_IMG_16		            ((uint8_t)0x73U)
#define DOWNLD_DISP_IMG_17		            ((uint8_t)0x74U)
#define DOWNLD_DISP_IMG_18		            ((uint8_t)0x75U)
#define DOWNLD_DISP_IMG_19		            ((uint8_t)0x76U)
#define DOWNLD_FW_IMG			            ((uint8_t)0x77U)
#define DOWNLD_BLDR_IMG		                ((uint8_t)0x78U)
#define DOWNLD_SMALL_FONT				    ((uint8_t)0x79U)
#define DOWNLD_MIDDLE_FONT			        ((uint8_t)0x7aU)
#define DOWNLD_BIG_FONT                     ((uint8_t)0x7bU)
#define DOWNLD_FILE 		                ((uint8_t)0x7cU)

#define DOWNLD_WTHR_FCAST                   ((uint8_t)0x81U)
#define DOWNLD_QR_CODE                      ((uint8_t)0x82U)
#define DOWNLD_LOGO_IMG                     ((uint8_t)0x83U)

#define DOWNLD_JOURNAL				        ((uint8_t)0x85U)

#define GET_SYS_STATUS					    ((uint8_t)0xa0U)
#define GET_ROOM_STATUS					    ((uint8_t)0xa1U)
#define GET_RS485_CONFIG				    ((uint8_t)0xa2U)
#define GET_LOG_LIST 					    ((uint8_t)0xa3U)
#define GET_MIFARE_USER_GROUP		        ((uint8_t)0xa4U)
#define GET_MIFAR_KEY_A                     ((uint8_t)0xa5U)
#define GET_MIFAR_KEY_B                     ((uint8_t)0xa6U)

#define GET_BUTTON_STATE                    ((uint8_t)0xa8U)
#define GET_ERROR_STATE                     ((uint8_t)0xa9U)
#define GET_THERMOSTAT_STATE                ((uint8_t)0xaaU)
#define GET_DISPLAY_STATE                   ((uint8_t)0xabU)

#define EXECUTE_APPLICATION				    ((uint8_t)0xbbU)
#define START_BLDR				            ((uint8_t)0xbcU)
#define FLASH_PROT_DISABLE		            ((uint8_t)0xbdU)
#define FLASH_PROT_ENABLE			        ((uint8_t)0xbeU)

#define RESTART_CTRL 				        ((uint8_t)0xc0U)
#define UPDATE_FIRMWARE           		    ((uint8_t)0xc1U)
#define UPDATE_BLDR				            ((uint8_t)0xc2U)

#define SET_ROOM_STATUS					    ((uint8_t)0xd0U)
#define SET_RS485_CONFIG				    ((uint8_t)0xd1U)
#define SET_DOUT_STATE					    ((uint8_t)0xd2U)
#define DELETE_LOG_LIST 				    ((uint8_t)0xd3U)
#define RESET_SOS_ALARM					    ((uint8_t)0xd4U)
#define SET_RTC_DATE_TIME				    ((uint8_t)0xd5U)
#define SET_ROOM_TEMPERATURE			    ((uint8_t)0xd6U)
#define SET_DISPLAY_BRIGHTNESS			    ((uint8_t)0xd7U)
#define SET_SYSTEM_ID					    ((uint8_t)0xd8U)
#define PREVIEW_DISP_IMG			        ((uint8_t)0xd9U)
#define SET_BEDDING_REPLACE	                ((uint8_t)0xdaU)
#define SET_MIFARE_USER_GROUP 		        ((uint8_t)0xdbU)
#define SET_MIFARE_KEY_A	                ((uint8_t)0xdcU)
#define SET_MIFARE_KEY_B	                ((uint8_t)0xddU)
#define SET_RS485_OW_BRIDGE                 ((uint8_t)0xdeU)
#define SET_BUTTON_STATE                    ((uint8_t)0xdfU)
#define RESET_ERROR_STATE                   ((uint8_t)0xe0U)
#define SET_THERMOSTAT_STATE                ((uint8_t)0xe1U)
#define SET_DISPLAY_STATE                   ((uint8_t)0xe2U)
#define SET_DISPLAY_QR_CODE                 ((uint8_t)0xe3U)
#define SET_DISPLAY_MESSAGE                 ((uint8_t)0xe4U)
/** ========================================================================*/
/**    	                 L O G 		         L I S T                        */
/** ========================================================================*/
#define FANC_RPM_SENSOR_ERROR                   ((uint8_t)0xc0U)
#define FANC_NTC_SENSOR_ERROR                   ((uint8_t)0xc1U)
#define FANC_LO_TEMP_ERROR                      ((uint8_t)0xc2U)
#define FANC_HI_TEMP_ERROR                      ((uint8_t)0xc3U)
#define FANC_FREEZING_PROTECTION                ((uint8_t)0xc4U)
#define THERMOSTAT_NTC_SENSOR_ERROR             ((uint8_t)0xc5U)
#define THERMOSTAT_ERROR                        ((uint8_t)0xc6U) // + 0-e = event (0xd0-0xde)

#define PIN_RESET                               ((uint8_t)0xd0U)
#define POWER_ON_RESET				            ((uint8_t)0xd1U)
#define SOFTWARE_RESET				            ((uint8_t)0xd2U)
#define IWDG_RESET					            ((uint8_t)0xd3U)
#define WWDG_RESET					            ((uint8_t)0xd4U)
#define LOW_POWER_RESET				            ((uint8_t)0xd5U)
#define FIRMWARE_UPDATED                        ((uint8_t)0xd6U)
#define FIRMWARE_UPDATE_FAIL		            ((uint8_t)0xd7U)
#define BOOTLOADER_UPDATED			            ((uint8_t)0xd8U)
#define BOOTLOADER_UPDATE_FAIL		            ((uint8_t)0xd9U)
#define IMAGE_UPDATED				            ((uint8_t)0xdaU)
#define IMAGE_UPDATE_FAIL			            ((uint8_t)0xdbU)
#define DISPLAY_FAIL				            ((uint8_t)0xdcU)
#define DRIVER_OR_FUNCTION_FAIL                 ((uint8_t)0xddU)
#define OW_BUS_EXCESSIVE_ERROR                  ((uint8_t)0xdeU)

#define NO_EVENT                	            ((uint8_t)0xe0U)
#define GUEST_CARD_VALID        	            ((uint8_t)0xe1U)
#define GUEST_CARD_INVALID      	            ((uint8_t)0xe2U)
#define HANDMAID_CARD_VALID                     ((uint8_t)0xe3U)
#define ENTRY_DOOR_CLOSED                       ((uint8_t)0xe4U)
#define PRESET_CARD					            ((uint8_t)0xe5U)
#define HANDMAID_SERVICE_END    	            ((uint8_t)0xe6U)
#define MANAGER_CARD            	            ((uint8_t)0xe7U)
#define SERVICE_CARD            	            ((uint8_t)0xe8U)
#define ENTRY_DOOR_OPENED          	            ((uint8_t)0xe9U)
#define MINIBAR_USED            	            ((uint8_t)0xeaU)
#define BALCON_DOOR_OPENED                      ((uint8_t)0xebU)
#define BALCON_DOOR_CLOSED			            ((uint8_t)0xecU)
#define CARD_STACKER_ON				            ((uint8_t)0xedU)		
#define CARD_STACKER_OFF			            ((uint8_t)0xeeU)
#define DO_NOT_DISTURB_SWITCH_ON 	            ((uint8_t)0xefU)
#define DO_NOT_DISTURB_SWITCH_OFF	            ((uint8_t)0xf0U)
#define HANDMAID_SWITCH_ON			            ((uint8_t)0xf1U)
#define HANDMAID_SWITCH_OFF			            ((uint8_t)0xf2U)
#define SOS_ALARM_TRIGGER			            ((uint8_t)0xf3U)
#define SOS_ALARM_RESET				            ((uint8_t)0xf4U)
#define FIRE_ALARM_TRIGGER			            ((uint8_t)0xf5U)
#define FIRE_ALARM_RESET          	            ((uint8_t)0xf6U)
#define UNKNOWN_CARD				            ((uint8_t)0xf7U)
#define CARD_EXPIRED				            ((uint8_t)0xf8U)
#define WRONG_ROOM					            ((uint8_t)0xf9U)
#define WRONG_SYSTEM_ID				            ((uint8_t)0xfaU)
#define CONTROLLER_RESET			            ((uint8_t)0xfbU)
#define ENTRY_DOOR_NOT_CLOSED		            ((uint8_t)0xfcU)
#define	DOOR_BELL_ACTIVE			            ((uint8_t)0xfdU)
#define	DOOR_LOCK_USER_OPEN			            ((uint8_t)0xfeU)


/* Exported macros     -------------------------------------------------------*/
#define SYS_NewLogSet()						(sys_status |= (1U << 0U))
#define SYS_NewLogReset()					(sys_status &= (~ (1U << 0U)))
#define IsSYS_NewLogSet()					((sys_status & (1U << 0U)) != 0U)

#define SYS_LogListFullSet()				(sys_status |= (1U << 1U))
#define SYS_LogListFullReset()				(sys_status &= (~ (1U << 1U)))
#define IsSYS_LogListFullSet()				((sys_status & (1U << 1U)) != 0U)

#define SYS_FileRxOkSet()		            (sys_status |= (1U << 2U))
#define SYS_FileRxOkReset()                 (sys_status &= (~ (1U << 2U)))
#define IsSYS_FileRxOkSet()		            ((sys_status & (1U << 2U)) != 0U)

#define SYS_FileRxFailSet()			        (sys_status |= (1U << 3U))
#define SYS_FileRxFailReset()			    (sys_status &= (~ (1U << 3U)))
#define IsSYS_FileRxFailSet()			    ((sys_status & (1U << 3U)) != 0U)

#define SYS_UpdateOkSet()				    (sys_status |= (1U << 4U))
#define SYS_UpdateOkReset()			        (sys_status &= (~ (1U << 4U)))
#define IsSYS_UpdateOkSet()			        ((sys_status & (1U << 4U)) != 0U)

#define SYS_UpdateFailSet()					(sys_status |= (1U << 5U))
#define SYS_UpdateFailReset()				(sys_status &= (~ (1U << 5U)))
#define IsSYS_UpdateFailSet()				((sys_status & (1U << 5U)) != 0U)

#define SYS_ImageRqSet()			        (sys_status |= (1U << 6U))
#define SYS_ImageRqReset()		            (sys_status &= (~ (1U << 6U)))
#define IsSYS_ImageRqSet()		            ((sys_status & (1U << 6U)) != 0U)

#define SYS_FwRqSet()		                (sys_status |= (1U << 7U))
#define SYS_FwRqReset()	                    (sys_status &= (~ (1U << 7U)))
#define IsSYS_FwRqSet()	                    ((sys_status & (1U << 7U)) != 0U)


/* Exported functions ------------------------------------------------------- */
//uint8_t CMD_Exe(PACKET_TypeDef* packet);
//void CMD_PacketReset(PACKET_TypeDef* packet);

#endif
/******************************   END OF FILE  **********************************/
