/**
 ******************************************************************************
 * File Name          : rs485.c
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rs485.h"
#include "common.h"
#include "command.h"
#include "display.h"
#include "onewire.h"
#include "thermostat.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"


/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/
enum 	/* Flash operation status code */
{
    FLASHIF_OK = 0U,
    FLASHIF_ERASEKO,
    FLASHIF_WRITINGCTRL_ERROR,
    FLASHIF_WRITING_ERROR,
    FLASHIF_PROT_ERRROR
};

enum	/* Flash protection type */  
{
    FLASHIF_PROT_NONE         = 0U,
    FLASHIF_PROT_PCROPENABLED = 0x1U,
    FLASHIF_PROT_WRPENABLED   = 0x2U,
    FLASHIF_PROT_RDPENABLED   = 0x4U,
};

enum 	/* Flash protection control */
{
    FLASHIF_WRP_ENABLE,
    FLASHIF_WRP_DISABLE
};




/* Private Define  -----------------------------------------------------------*/
#define BUFF_SIZE   256U


/* Private Variables  --------------------------------------------------------*/
uint8_t sys_status;
uint8_t activ_command;
uint8_t fw_update_status;


/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
static void CopyFile(uint32_t source_address, uint32_t destination_address, uint32_t size);
static uint8_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
static void FormatFileStorage(uint32_t start_address, uint8_t number_of_blocks);
static uint8_t UpdateBootloader(void);
static void BackupOldFirmware(void);
static uint32_t FLASH_Sector(uint32_t addr);
static void FLASH_Init(void);
static uint8_t FLASH_Erase(uint32_t addr, uint32_t size);
static uint8_t FLASH_Write(uint32_t addr, uint32_t *buff, uint32_t size);

/* Program Code  -------------------------------------------------------------*/
uint8_t CMD_Exe(PACKET_TypeDef* packet)
{
    RTC_TimeTypeDef time_rs;
    RTC_DateTypeDef date_rs;
    uint8_t ee_buff[24];
	static uint32_t tmp_a, tmp_b;
	static uint32_t rs485_update_timeout_timer;
    static uint32_t total_bytes_in_file;
    static uint32_t next_packet_number;
    static uint32_t total_packet_number;
    static uint32_t crc_32_calculated;
    static uint32_t crc_32_file;
    static uint32_t flash_destination;
    static uint32_t file_copy_src_addr;
    static uint32_t file_copy_dest_addr;
    static uint32_t file_copy_size;
    static uint32_t post_process;
    

    if((rs485_buff[0] >= DOWNLD_DISP_IMG_1) && (rs485_buff[0] <= DOWNLD_BIG_FONT))
    {
//               
    }
    
    switch(packet->type) // all preset byte number received in buffer, switch to packet state to process data
    {
        
        case PACKET_INIT:
        {
            
            
 
            break;
        }
        // if previous received data validation passed ok, data header is received and packet state is set to STATE_HEADER.
        // it's expected to receive data packet with data payload in ASCII or BINARY format
        case STATE_HEADER:  // valid data packet header received in previous transfer
        {
            uint8_t crc8 = HAL_CRC_Calculate (&hcrc, (uint32_t*)owrxbuf, OW_Packet.dsize); // calculate crc8 value for all data bytes
            HEX2STR ((char*)&owrxbuf[OW_Packet.dsize + 2U], &crc8);    // convert calculated crc8 value to 2 ascii char to be compared with received packet crc8
            
            if (((owrxbuf[0] != SOH) && (owrxbuf[0] != STX)) || (owrxbuf[OW_Packet.dsize - 1U] != ETX))   OW_Packet.error = ERROR_DAT_CTRL;   // data payload packet control charaters invalid if (((owrxbuf[0] != SOH) && (owrxbuf[0] != STX)) || (owrxbuf[OW_Packet.dsize - 1U] != ETX))   OW_Packet.error = ERROR_DAT_CTRL;   // data payload packet control charaters invalid 
            if (owrxbuf[0] == STX){for(uint8_t i=1U;i<(OW_Packet.dsize-1U);i++){if(owrxbuf[i] < 0x20U)  OW_Packet.error = ERROR_DAT_FRMT;}} // data payload packet text not valid ASCII
            if (strncmp((char*)&owrxbuf[OW_Packet.dsize],(char*)&owrxbuf[OW_Packet.dsize + 2U],2)!=0U)  OW_Packet.error = ERROR_DAT_CRC;    // data payload packet crc8 different from calculated crc8
            if ((OW_Packet.error != ERROR_INIT) || (OW_Packet.link == LINK_QUIT)) OW_Init(); // if any error occured drop packet and start over
            else 
            {
                OW_Packet.state = STATE_DATA; 
                OW_Packet.pbuf = owrxbuf;
                CMD_Exe(&OW_Packet); // if valid data payload received, call comman execution to apply new data
                
                if(OW_Packet.state == STATE_RSP)
                {
                    ZEROFILL(owrxbuf, COUNTOF(owrxbuf)); 
                    __HAL_UART_FLUSH_DRREGISTER(&huart2);
                    owrxbuf[0] = STX;
                    HEX2STR ((char*)&owrxbuf[1], (uint8_t*)&OW_Packet.txaddr);
                    HEX2STR ((char*)&owrxbuf[3], (uint8_t*)&OW_Packet.rxaddr);
                    owrxbuf[5] = 0x30U;
                    owrxbuf[6] = 0x36U;
                    owrxbuf[7] = 0x30U;;
                    owrxbuf[8] = 0x30U;;
                    owrxbuf[9] = ETX;
                    uint8_t crc8 = HAL_CRC_Calculate (&hcrc, (uint32_t*)owrxbuf, 10U); // calculate crc8
                    HEX2STR ((char*)&owrxbuf[10], &crc8);
                    
                    if(HAL_UART_Transmit_IT(&huart2, owrxbuf, 12U) == HAL_OK)
                    {
                        OW_Packet.time = OW_TIMEOUT;
                        OW_Packet.state = STATE_DATA;
                    }
                }
                
            }

            break;
        }
        // if received data passed start, stop, lenght, crc8 and format check with ok, packet state is set to STATE_RSP
        // enabling response to sender only if packet link is set to LINK_P2P. for all other packet link states,
        // function does not send response to sender.
        case STATE_DATA: // valid data packet payload received in previous transfer
        {
//            OnewireStopTimer();
//            
//            if(owrxbuf[OW_Packet.bcnt - 1U] == CalcCRC(owrxbuf, (OW_Packet.bcnt - 1U)))
//            {
//                OW_Packet.pbuf = owrxbuf;
//                CMD_Exe(&OW_Packet);
//            }

//            HAL_Delay(2);
//            owrxbuf[0] = OW_Packet.txaddr;  // set 9 bit lsb
//            owrxbuf[1] = 0x01U;             // set 9 bit msb to 1 for ddress
//            owrxbuf[2] = owrxbuf[2];          // set as 9 bit lsb data inside ow_cmd switch as packet lenght
//            owrxbuf[3] = 0x00U;             // set as 9 bit msb to 0 for data
//            OW_SetUsart(OW_9600, OW_9_BIT);
//            HAL_UART_Transmit(&huart2, owrxbuf, 2U, OW_TIMEOUT);
//            HAL_Delay(1);
//            OW_SetUsart(OW_9600, OW_8_BIT);
//            owrxbuf[owrxbuf[2] + 3U] = CalcCRC(&owrxbuf[4], (owrxbuf[2] - 1U));
//            HAL_UART_Transmit(&huart2, &owrxbuf[4], owrxbuf[2], OW_TIMEOUT);
            break;
        }
        // call cmd_exe function with packet reference to apply new command or data received
        // some of request cannot be executed from this interrupt handler because call of
        // lower interrupt priority driver will stuck waiting this interrupt handler to finish
        // this type of request is written in list executed with next function call from main loop
        default:
        {
            OW_Init();
            break;
        }
    }

    switch(packet->cmd)
    {
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case (DOWNLD_FILE):
        {
            if(packet->pbuf[0] == SOH)
            {
                next_packet_number = 1U;
                activ_command = rs485_buff[0];
                total_packet_number = (rs485_buff[1] << 8U) + rs485_buff[2];
                crc_32_file = (rs485_buff[7] << 24U) + (rs485_buff[8] << 16U) + (rs485_buff[9] << 8U) + rs485_buff[10];
                total_bytes_in_file = (rs485_buff[3] << 24U) + (rs485_buff[4] << 16U) + (rs485_buff[5] << 8U) + rs485_buff[6];
                rs485_update_timeout_timer = HAL_GetTick();
                SYS_FileRxFailReset();
                RS485_StartUpdate();
                
                if((activ_command != DOWNLD_FW_IMG) && (activ_command != DOWNLD_BLDR_IMG))
                {
                    SYS_ImageRqReset();
                    flash_destination = NEW_DISP_IMG_ADDR;
                    post_process = COPY_NEW_DISP_IMG;
                    file_copy_src_addr = NEW_DISP_IMG_ADDR;
                    file_copy_dest_addr = ((activ_command - DOWNLD_DISP_IMG_1) * 0x00030000U);
                    file_copy_size = total_bytes_in_file;
                    FormatFileStorage(NEW_DISP_IMG_ADDR, 3U);
                }
                else
                {
                    SYS_FwRqReset();
                    flash_destination = ((activ_command - DOWNLD_DISP_IMG_1) * 0x00030000U);
                    if(activ_command == DOWNLD_FW_IMG) FormatFileStorage(NEW_APPL_EXT_ADDR, 3U);					
                    else if(activ_command == DOWNLD_BLDR_IMG) FormatFileStorage(NEW_BLDR_EXT_ADDR, 3U);
                }
            }
            else if(packet->pbuf[0] == STX)
            {
                if((next_packet_number == ((rs485_buff[0] << 8U) + rs485_buff[1])) && (flash_destination != 0U))
                {
                    if(next_packet_number == 1U) __HAL_CRC_DR_RESET(&hcrc);
                    BSP_QSPI_Write(&rs485_buff[2], flash_destination, (packet->dsize - 2U));				
                    flash_destination += (packet->dsize - 2U);
//                    if(packet->link == LINK_P2P) 
//                    {
//                        ee_buff[0] = ACK;
//                        HAL_UART_Transmit(&huart1, ee_buff, 1U, TX_TIMEOUT);
//                    }
             
                    crc_32_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) &rs485_buff[2], (packet->dsize - 2U));
                    rs485_update_timeout_timer = HAL_GetTick();
                    
                    if(next_packet_number == total_packet_number)
                    {
                        if(crc_32_calculated == crc_32_file)
                        {	
                            if(activ_command == DOWNLD_FW_IMG)
                            {
    //							ee_buff[0] = EE_FW_UPDATE_BYTE_CNT >> 8U;
    //							ee_buff[1] = EE_FW_UPDATE_BYTE_CNT & 0xFFU;
    //							ee_buff[2] = (total_bytes_in_file >> 24U);
    //							ee_buff[3] = (total_bytes_in_file >> 16U);
    //							ee_buff[4] = (total_bytes_in_file >> 8U);
    //							ee_buff[5] = (total_bytes_in_file & 0xFFU);
    //							if (HAL_I2C_Master_Transmit(&hi2c4, EE_ADDR, ee_buff, 6U, EE_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
    //							if (HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_TRIALS, EE_WR_TIME) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
                            }
                            else if(post_process == COPY_NEW_DISP_IMG)
                            {
                                CopyFile(file_copy_src_addr, file_copy_dest_addr, file_copy_size);
                                SYS_UpdateOkSet();
                            }
                            
                            SYS_FileRxOkSet();
                        }
                        else
                        {
                            SYS_FileRxFailSet();
                        }
                        
                        post_process = 0U;
                        activ_command = 0U;
                        flash_destination = 0U;
                        RS485_StopUpdate();
                    }
                    else ++next_packet_number;
                }
//                else if(packet->link == LINK_P2P) 
//                {
//                    ee_buff[0] = NAK;
//                    HAL_UART_Transmit(&huart1, ee_buff, 1U, TX_TIMEOUT);
//                }
            }
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = activ_command;						
                RS485_ResponseSet();						
            }			
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case UPDATE_BLDR:
        {
            rs485_update_timeout_timer = HAL_GetTick();
            RS485_StartUpdate();
            
            if(UpdateBootloader() == 0U)
            {
    //							DISPLAY_BootloaderUpdateFail();
                SYS_UpdateFailSet();
            }
            else 
            {
    //							DISPLAY_BootloaderUpdated();
                SYS_UpdateOkSet();
            }
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = UPDATE_BLDR;				
                RS485_ResponseSet();
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case RESTART_CTRL:
        {
//            if (packet->link == LINK_P2P)
//            {
//                ee_buff[0] = ACK;
//                HAL_UART_Transmit(&huart1, ee_buff, 1U, TX_TIMEOUT);
//            }
            
            HAL_Delay(10U);
    //						BootloaderExe();
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case START_BLDR:
        {
//            if (packet->link == LINK_P2P)
//            {
//                ee_buff[0] = ACK;
//                HAL_UART_Transmit(&huart1, ee_buff, 1U, TX_TIMEOUT);
//            }
            ee_buff[0] = EE_BOOTLDR_CTRL >> 8U;
            ee_buff[1] = EE_BOOTLDR_CTRL & 0xFFU;
            ee_buff[2] = BOOTLDR_UPDATE_SUCCESS;
            if (HAL_I2C_Master_Transmit(&hi2c4, EE_ADDR, ee_buff, 3U, EE_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
            if (HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_TRIALS, EE_WR_TIME) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
            BackupOldFirmware();
    //						BootloaderExe();
            break;
        }	
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_RS485_OW_BRIDGE:
        {
            tmp_a = OW_RS485_Bridge(&rs485_buff[1]);
            
            if(packet->link == LINK_P2P) 
            {
                if(tmp_a == 0U)
                {
                    rs485_buff[5] = 1U;   // set not acknowledge response in case of OW transmission failure
                    rs485_buff[6] = NAK;
                }

                RS485_ResponseSet();
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case GET_SYS_STATUS:
        {
            if(packet->link == LINK_P2P)
            {
                                                        packet->pbuf[2] = 4U;
                if(IsOW_SendNewButtonStateActiv())      packet->pbuf[4] |= (1U << 0), OW_SendNewButtonStateReset();
                if(IsBUTTON_DndActiv())                 packet->pbuf[4] |= (1U << 1);
                if(IsBUTTON_SosResetActiv())            packet->pbuf[4] |= (1U << 2);
                if(IsBUTTON_CallMaidActiv())            packet->pbuf[4] |= (1U << 3);
                if(IsBUTTON_DoorOpenActiv())            packet->pbuf[4] |= (1U << 4), BUTTON_DoorOpenReset();
                if(IsBUTTON_OkActiv())                  packet->pbuf[4] |= (1U << 5), BUTTON_OkReset();
                if(IsOW_SendThermostatVariableActiv())  packet->pbuf[4] |= (1U << 6), OW_SendThermostatVariableReset();
                if(IsOW_UpdateDisplayImageActiv())      packet->pbuf[4] |= (1U << 7), OW_UpdateDisplayImageReset();
                if(IsFANC_RPM_SensorErrorActiv())       packet->pbuf[5] |= (1U << 0), FANC_RPM_SensorErrorReset();
                if(IsFancoilNtcSensorErrorActiv())      packet->pbuf[5] |= (1U << 1), FancoilNtcSensorErrorReset();
                if(IsFANC_HeatingTempErrorActiv())      packet->pbuf[5] |= (1U << 2), FANC_HeatingTempErrorReset();
                if(IsFANC_CoolingTempErrorActiv())      packet->pbuf[5] |= (1U << 3), FANC_CoolingTempErrorReset();
                if(IsFANC_FreezingAlarmActiv())         packet->pbuf[5] |= (1U << 4), FANC_FreezingAlarmReset();
                if(IsRoomNtcSensorErrorActiv())         packet->pbuf[5] |= (1U << 5), RoomNtcSensorErrorReset();
                                                        packet->pbuf[6] = (room_temp / 10);
                RS485_ResponseSet();
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_RTC_DATE_TIME:
        {
//            date.Date = packet->pbuf[1];
//            date.WeekDay = packet->pbuf[2];
//            date.Month = packet->pbuf[3];
//            date.Year = packet->pbuf[4];
//            time.Hours = packet->pbuf[5];
//            time.Minutes = packet->pbuf[6];
//            time.Seconds = packet->pbuf[7];
//            HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
//            HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
//            HAL_RTC_WaitForSynchro(&hrtc);
            Str2Hex((const char *)&rs485_buff[1], &date_rs.Date, 2U);
            Str2Hex((const char *)&rs485_buff[3], &date_rs.Month, 2U);
            Str2Hex((const char *)&rs485_buff[5], &date_rs.Year, 2U);
            Str2Hex((const char *)&rs485_buff[7], &time_rs.Hours, 2U);
            Str2Hex((const char *)&rs485_buff[9], &time_rs.Minutes, 2U);
            Str2Hex((const char *)&rs485_buff[11], &time_rs.Seconds, 2U);
            
            HAL_RTC_SetTime(&hrtc, &time_rs, RTC_FORMAT_BCD);
            HAL_RTC_SetDate(&hrtc, &date_rs, RTC_FORMAT_BCD);
            
            OW_TimeUpdateSet();
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_RTC_DATE_TIME;
                RS485_ResponseSet();
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_RS485_CONFIG:
        {
            Str2Hex((const char *)&rs485_buff[1], rs485_ifaddr, 4U);
            Str2Hex((const char *)&rs485_buff[5], rs485_graddr, 4U);
            Str2Hex((const char *)&rs485_buff[9], rs485_braddr, 4U);
            rs485_bps = rs485_buff[13];
            
            ee_buff[0] = (EE_RS485_INTERFACE_ADDRESS >> 8U);
            ee_buff[1] = EE_RS485_INTERFACE_ADDRESS & 0xFFU;
            ee_buff[2] = rs485_ifaddr[0];
            ee_buff[3] = rs485_ifaddr[1];
            ee_buff[4] = rs485_graddr[0];
            ee_buff[5] = rs485_graddr[1];
            ee_buff[6] = rs485_braddr[0];
            ee_buff[7] = rs485_braddr[1];
            ee_buff[8] = rs485_bps;
            
            if (HAL_I2C_Master_Transmit(&hi2c4, EE_ADDR, ee_buff, 9U, EE_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
            if (HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_TRIALS, EE_WR_TIME) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_RS485_CONFIG;				
                RS485_ResponseSet();
            }					
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_DOUT_STATE:
        {
            for(tmp_b = 0U; tmp_b < 9U; tmp_b++)
            {
    //							if(rs485_buff[tmp_b + 1U] == '1') dout_0_7_remote |= (uint16_t) (1U << tmp_b);
    //							else if(rs485_buff[tmp_b + 1U] == '0') dout_0_7_remote &= (uint16_t) (~(1U << tmp_b));
            }
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_DOUT_STATE;
                RS485_ResponseSet();						
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_DISPLAY_BRIGHTNESS:
        {
            
            if((rs485_buff[1] >= DISPLAY_BRIGHTNESS_MIN)  
            && (rs485_buff[1] <  displ_brightness_hi))
            {
                displ_brightness_lo = rs485_buff[1];
            }
            
            if((rs485_buff[2] >  displ_brightness_lo)  
            && (rs485_buff[2] <= DISPLAY_BRIGHTNESS_MAX))
            {
                displ_brightness_hi = rs485_buff[2];
            }
            
            if(IsScreensaverActiv()) DISPLAY_SetBrightnes(displ_brightness_lo);
            else DISPLAY_SetBrightnes(displ_brightness_hi);
            
            BSP_EEPROM_WriteBuffer(&rs485_buff[1], EE_DISPLAY_LOW_BRIGHTNESS, 2U);
        
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_DISPLAY_BRIGHTNESS;
                RS485_ResponseSet();						
            }
            break;
        }	
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case RESET_SOS_ALARM:
        {
    //						if((rs485_buff[1] == '1') && IsSosAlarmActiv())
    //						{
    //							SosAlarm_Off();
    //							LogEvent.log_event = SOS_ALARM_RESET;
    //							LOGGER_Write();
    //							DISPLAY_SosAlarmImageDelete();
    //                            OW_ButtonUpdateSet();
    //							SignalBuzzer = BUZZ_OFF;
    //							rs485_buff[7] = '1';
    //						}
    //						else
    //						{
    //							rs485_buff[7] = '0';
    //						}
        
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 2U;
                rs485_buff[6] = RESET_SOS_ALARM;
                RS485_ResponseSet();						
            }
            break;
        }	
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_ROOM_TEMPERATURE:
        {
            tmp_a = 0U;
            if(rs485_buff[1] == 'E') tmp_a |= (1U << 7U);				
            if(rs485_buff[2] == 'H') tmp_a |= (1U << 6U);
            tmp_a += (rs485_buff[3] - 48U) * 10U;
            tmp_a += (rs485_buff[4] - 48U);
                                
            tmp_b = 0U;
            if(rs485_buff[1] == 'O') tmp_b |= (1U << 7U);
            tmp_b += (rs485_buff[5] - 48U) * 100U;					
            tmp_b += (rs485_buff[6] - 48U) * 10U;
            tmp_b += (rs485_buff[7] - 48U);
            
    //                        temperature_setpoint = tmp_a;
    //                        temperature_difference = tmp_b;
    //                        ee_buff[0] = EE_ROOM_TEMPERATURE_SETPOINT >> 8U;
    //                        ee_buff[1] = EE_ROOM_TEMPERATURE_SETPOINT;
    //                        ee_buff[2] = temperature_setpoint;
    //                        if (HAL_I2C_Master_Transmit(&hi2c4, EE_ADDR, ee_buff, 3U, EE_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
    //                        if (HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_TRIALS, EE_WR_TIME) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);	

    //                        ee_buff[0] = EE_ROOM_TEMPERATURE_DIFFERENCE >> 8U;
    //                        ee_buff[1] = EE_ROOM_TEMPERATURE_DIFFERENCE;
    //                        ee_buff[2] = temperature_difference;
    //                        if (HAL_I2C_Master_Transmit(&hi2c4, EE_ADDR, ee_buff, 3U, EE_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
    //                        if (HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_TRIALS, EE_WR_TIME) != HAL_OK) Error_Handler(RS485_FUNC, I2C_DRV);
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_ROOM_TEMPERATURE;						
                RS485_ResponseSet();						
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case SET_SYSTEM_ID:
        {
    //						system_id[0] = rs485_buff[1];
    //						system_id[1] = rs485_buff[2];
    //						EEPROM_Save(EE_SYSTEM_ID_ADDRESS, system_id, 2U);
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = SET_SYSTEM_ID;						
                RS485_ResponseSet();						
            }
            break;
        }
        /**
          * @brief  
          * @param  
          * @retval 
          */
        case PREVIEW_DISP_IMG:
        {
    //						DISPLAY_PreviewImage();
            
            if(packet->link == LINK_P2P)
            {
                rs485_buff[5] = 1U;
                rs485_buff[6] = PREVIEW_DISP_IMG;						
                RS485_ResponseSet();						
            }
            break;
        }
        /** ==========================================================================*/
        /**			S E T		N E W		B U T T O N 		S T A T E			  */
        /** ==========================================================================*/
        case SET_BUTTON_STATE:
        {
            // BUTTON DND STATE
            if(((packet->pbuf[1] & (1U << 1)) != 0U) && !IsBUTTON_DndActiv())
            {
                BUTTON_DndActivSet();
                BUTTON_CallMaidActivReset();
                ButtonStateUpdateSet();
            }
            else if(((packet->pbuf[1] & (1U << 1)) == 0U) && IsBUTTON_DndActiv())
            {
                BUTTON_DndActivReset();
                ButtonStateUpdateSet();
            }
            // BUTTON SOS STATE
            if(((packet->pbuf[1] & (1U << 2)) != 0U) && !IsBUTTON_SosResetActiv())
            {
                BUTTON_SosActivSet();
                ButtonStateUpdateSet();
            }
            else if(((packet->pbuf[1] & (1U << 2)) == 0U) && IsBUTTON_SosResetActiv())
            {
                BUTTON_SosActivReset();
                ButtonStateUpdateSet();
            }
            // BUTTON_CALMAID_STATE
            if(((packet->pbuf[1] & (1U << 3)) != 0U)  && !IsBUTTON_CallMaidActiv()) 
            {
                BUTTON_DndActivReset();
                BUTTON_CallMaidActivSet();
                ButtonStateUpdateSet();
            }
            else if(((packet->pbuf[1] & (1U << 3)) == 0U) && IsBUTTON_CallMaidActiv())
            {
                BUTTON_CallMaidActivReset();
                ButtonStateUpdateSet();
            }
            // ROOM CONTROLLER HVAC CONTACTOR STATUS
            if((packet->pbuf[1] & (1U << 4)) == 0U) HvacContactorOff();
            else if((packet->pbuf[1] & (1U << 4)) != 0U) HvacContactorOn();
            // ROOM CONTROLLER BALCONY AND WINDOWS SWITCH STATUS
            if((packet->pbuf[1] & (1U << 5)) == 0U) ExternalSwitchRemoteOpen();
            else if((packet->pbuf[1] & (1U << 5)) != 0U)ExternalSwitchRemoteClosed();
            // ROOM CONTROLER POWER CONTACTOR STATUS
            if((packet->pbuf[1] & (1U << 6)) == 0U) OW_PowerContactorOff();
            else if((packet->pbuf[1] & (1U << 6)) != 0U)OW_PowerContactorOn();
            // ROOM CONTROLLER DOOR BELL TASTER STATUS
            if((packet->pbuf[1] & (1U << 7)) == 0U)OW_DoorBellOff();
            else if((packet->pbuf[1] & (1U << 7)) != 0U)OW_DoorBellOn();
            // SER RESPONSE 
            packet->pbuf[2] = 2U;
            packet->pbuf[4] = ACK;
            break;
        }
        /** ==========================================================================*/
        /**     C L E A R		T H E R M O S T A T 	E R R O R       E V E N T     */
        /** ==========================================================================*/
        case RESET_ERROR_STATE:
        {
            FANC_RPM_SensorErrorReset();
            FancoilNtcSensorErrorReset();
            FANC_HeatingTempErrorReset();
            FANC_CoolingTempErrorReset();
            FANC_FreezingAlarmReset();
            RoomNtcSensorErrorReset();
            packet->pbuf[2] = 2U;
            packet->pbuf[4] = ACK;
            break;
        }
        /** ==========================================================================*/
        /**			S E T		T H E R M O S T A T 	P A R A M E T E R S			  */
        /** ==========================================================================*/                
        case SET_THERMOSTAT_STATE:
        {
            if(!IsSYSTEM_StartupActiv() && IsTHERMOSTAT_OperationModeRemote())
            {
                therm_temp_sp = packet->pbuf[2];
    
                if  (!IsTHERMOSTAT_OperationModeRemote())
                {
                    if      ((therm_temp_sp & 0x3FU) > therm_max_sp) 
                    {
                        therm_temp_sp = (therm_temp_sp & 0xC0U) + therm_max_sp;
                    }
                    else if ((therm_temp_sp & 0x3FU) < therm_min_sp) 
                    {
                        therm_temp_sp = (therm_temp_sp & 0xC0U) + therm_min_sp;                         
                    }
                }
                
                ThermostatStateUpdateSet();
                SetpointTempUpdateSet();
                packet->pbuf[4] = ACK;                                
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**		    S E T		N E W		D I S P L A Y    M E S S A G E            */
        /** ==========================================================================*/
        case SET_DISPLAY_STATE:             
        {
            if(!IsOW_UpdateDisplayImageActiv() && !IsSYSTEM_StartupActiv())
            {
                if(disp_img_id != packet->pbuf[1])
                {
                    disp_img_id = packet->pbuf[1];
                    disp_img_time = packet->pbuf[2];
                    buzz_sig_time = packet->pbuf[3];
                    buzz_sig_id = packet->pbuf[4];
                    DISPLAY_UpdateSet(); 
                }
                
                packet->pbuf[4] = ACK;
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**         S E T   N E W   W E A T H E R   F O R E C A S T   I N F O         */
        /** ==========================================================================*/
        case DOWNLD_WTHR_FCAST:
        {
            uint32_t size = GetSize(packet->pbuf);
            
            if((size < WEATHER_FORECAST_BUFF_SIZE - 1U) && (packet->dsize < WEATHER_FORECAST_BUFF_SIZE - 1U))
            {
                for(uint8_t i = 0U; i < packet->dsize - 2U; i++)
                {
                    weather_forecast_buff[i] = (int8_t) packet->pbuf[i + 1U];
                }
                
                WeatherForecastUpdateSet();
                packet->pbuf[4] = ACK; 
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**             S E T        N E W       Q R         C O D E                 */
        /** ==========================================================================*/
        case DOWNLD_QR_CODE:
        {
            uint32_t size = GetSize(packet->pbuf);
            
            if((size < 127U) && (packet->dsize < 127U))
            {
                packet->pbuf[0] = packet->dsize;
                EEPROM_IO_WriteData(EEPROMAddress, EE_QR_CODE_ADDRESS, packet->pbuf, packet->dsize);
                BSP_EEPROM_WaitEepromStandbyState();
                packet->pbuf[4] = ACK;
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**             S E T        N E W       Q R         C O D E                 */
        /** ==========================================================================*/
        case SET_DISPLAY_QR_CODE:
        {
            uint8_t qr_size;
            
            EEPROM_IO_ReadData(EEPROMAddress, EE_QR_CODE_ADDRESS, &qr_size, 1U);

            if((qr_size > 0U) && (qr_size < 127U)) // qr code size error
            {
//                                DISPLAY_QrCodeSet();
                packet->pbuf[4] = ACK;
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**             S E T        N E W       Q R         C O D E                 */
        /** ==========================================================================*/
        case DOWNLD_LOGO_IMG:
        {
            packet->pbuf[2] = 2U;
            packet->pbuf[4] = ACK;
            break;
        }
        /** ==========================================================================*/
        /**             S E T        N E W       Q R         C O D E                 */
        /** ==========================================================================*/
        case SET_DISPLAY_MESSAGE:
        {
            uint32_t size = GetSize(packet->pbuf);
            
            if(!IsOW_UpdateDisplayImageActiv() && !IsSYSTEM_StartupActiv() && (size < 126U))
            {
//                                DIDPLAY_Message((const char *)&packet->pbuf[1]); 
                packet->pbuf[4] = ACK;
            }
            else
            {
                packet->pbuf[4] = NAK;
            }
            packet->pbuf[2] = 2U;
            break;
        }
        /** ==========================================================================*/
        /**			    G E T	        B U T T O N 		S T A T E                 */
        /** ==========================================================================*/

        /** ==========================================================================*/
        /**			    G E T	        B U T T O N 		S T A T E                 */
        /** ==========================================================================*/
        case GET_BUTTON_STATE:
        {
                                                packet->pbuf[2] = 4U;
            if (IsBUTTON_DndActiv())            packet->pbuf[4] = 1U;
            if (IsBUTTON_SosResetActiv())       packet->pbuf[5] = 1U;
            if (IsBUTTON_CallMaidActiv())       packet->pbuf[6] = 1U;
            break;
        }
        /** ==========================================================================*/
        /**			G E T		T H E R M O S T A T 	E R R O R       E V E N T     */
        /** ==========================================================================*/                       
        case GET_ERROR_STATE:
        {
                                                packet->pbuf[2] = 7U;
            if(IsFANC_RPM_SensorErrorActiv())   packet->pbuf[4] = 1U;
            if(IsRoomNtcSensorErrorActiv())     packet->pbuf[5] = 1U;
            if(IsFANC_HeatingTempErrorActiv())  packet->pbuf[6] = 1U;
            if(IsFANC_CoolingTempErrorActiv())  packet->pbuf[7] = 1U;
            if(IsFANC_FreezingAlarmActiv())     packet->pbuf[8] = 1U;
            if(IsRoomNtcSensorErrorActiv())     packet->pbuf[9] = 1U;
            break;
        }
        /** ==========================================================================*/
        /**			G E T       T H E R M O S T A T 	P A R A M E T E R S			  */
        /** ==========================================================================*/
        case GET_THERMOSTAT_STATE:
        {
            packet->pbuf[2] = 5U;
            packet->pbuf[4] = therm_temp_sp;
            packet->pbuf[5] = (room_temp / 10); 
            packet->pbuf[6] = (fancoil_temp / 10);
            packet->pbuf[7] = therm_op_mode;
            break;
        }
        /** ==========================================================================*/
        /**		    G E T		A C T I V		D I S P L A Y    S C R E E N          */
        /** ==========================================================================*/
        case GET_DISPLAY_STATE:
        {
            packet->pbuf[4] = disp_img_id;
            packet->pbuf[2] = 2U;
            break;
        }
  
           
        /**
          * @brief  
          * @param  
          * @retval 
          */
        default:
        {
            RS485_ResponseReset();
            break;
        }

              
    }
   
    
    if(IsRS485_ResponseRdy())
    {
            
//			rs485_buff[0] = ACK;
//			rs485_buff[1] = (rs485_sender_address >> 8U);
//			rs485_buff[2] = (rs485_sender_address & 0xFFU);
//			rs485_buff[3] = rs485_ifaddr[0];
//			rs485_buff[4] = rs485_ifaddr[1];			
//			rs485_packet_checksum = 0U;
//			
//			for(uint8_t i = 6U; i < (rs485_buff[5] + 6U); i++)
//			{
//				rs485_packet_checksum += rs485_buff[i];
//			}
//            
//			rs485_buff[rs485_buff[5] + 6U] = (rs485_packet_checksum >> 8U);
//			rs485_buff[rs485_buff[5] + 7U] = (rs485_packet_checksum & 0xFFU);
//			rs485_buff[rs485_buff[5] + 8U] = EOT;
//            
//			HAL_GPIO_WritePin(RS485_DIR_Port, RS485_DIR_Pin, GPIO_PIN_SET);
//			if(HAL_UART_Transmit(&huart1, rs485_buff, (rs485_buff[5] + 9U), TX_TIMEOUT) != HAL_OK) Error_Handler(RS485_FUNC, USART_DRV);
//			HAL_GPIO_WritePin(RS485_DIR_Port, RS485_DIR_Pin, GPIO_PIN_RESET);
//            RS485_ResponseReset();
		}
    
        return (0U); // cmd OK
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void CMD_PacketReset(PACKET_TypeDef* packet)
{
    packet->type = PACKET_INIT;
    packet->val.cmd = 0U;
    packet->val.dsz = 0U;
    packet->val.par = 0U;
    packet->val.res = 0U;
    packet->val.rva = 0U;
    packet->add.rcv = 0U;
    packet->add.snd = 0U;
    packet->tmr.rxtmr = 0U;
    packet->tmr.txtmr = 0U;
    packet->tmr.rxtime = 0U;
    packet->tmr.txtime = 0U;     
    packet->buf.prxb =NULL;
    packet->buf.ptxb = NULL;
    packet->buf.rxsz = 0U;
    packet->buf.txsz = 0U;
    packet->no.actual = 0U;
    packet->no.next = 0U;
    packet->no.total = 0U;
    packet->lnk = LINK_INIT;
    packet->src = SOURCE_INIT;
    packet->err = ERROR_INIT;
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void FormatFileStorage(uint32_t start_address, uint8_t number_of_blocks)
{
	while(number_of_blocks)
	{
		if(BSP_QSPI_Erase_Block(start_address) != 0U) Error_Handler(EEPROM_FUNC, QSPI_DRV);
		start_address += N25Q128A_SECTOR_SIZE;
		--number_of_blocks;
	}
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void CopyFile(uint32_t source_address, uint32_t destination_address, uint32_t size)
{
	uint8_t buff[BUFF_SIZE];
    uint32_t cnt;
	
    cnt = size / 0x00010000U;
	if(size > (cnt * 0x00010000U)) ++cnt;
	FormatFileStorage(destination_address, cnt);
	
	while(size)
	{
		if(size >= BUFF_SIZE) 
        {
            cnt = BUFF_SIZE;
            size -= BUFF_SIZE;
        }
		else 
        {
            cnt = size;
            size = 0U;
        }
        
//        SPI_FLASH_ReadPage(source_address, buff, cnt);
//        SPI_FLASH_WritePage(destination_address, buff, cnt);
//        if(SPI_FLASH_WaitReadyStatus(SPI_FLASH_TIMEOUT) == 0U) Error_Handler(EEPROM_FUNC, SPI_DRV);
        destination_address += cnt;
        source_address += cnt;
	}
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static uint8_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
    uint32_t i = 0U;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    for (i = 0U; (i < length) && (destination <= (INT_ROM_ADDR + INT_ROM_SIZE - 4U)); i++)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */ 
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(p_source + i)) == HAL_OK)      
        {
            /* Check the written value */
            if (*(uint32_t*)destination != *(uint32_t*)(p_source + i))
            {
                /* Flash content doesn't match SRAM content */
                return(0U);
            }
            /* Increment FLASH destination address */
            destination += 4U;
        }
        else
        {
            /* Error occurred while writing data in Flash memory */
            return (0U);
        }
    }
    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    return (1U);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static uint8_t UpdateBootloader(void)
{
#define BL_BUFFER_SIZE  2048U
    
    uint8_t buff[BL_BUFFER_SIZE];
    FLASH_EraseInitTypeDef FLASH_EraseInit;
    uint32_t page_erase_error;
	uint32_t fl_destination;
	uint32_t fl_address;
    uint32_t ram_source;
	uint32_t file_size;
    uint32_t bcnt;
	
	HAL_FLASH_Unlock();
	FLASH_EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
//	FLASH_EraseInit.PageAddress = BOOTLOADER_ADDR;
//	FLASH_EraseInit.Banks = FLASH_BANK_1;
//	FLASH_EraseInit.NbPages = 12U;	
	if(HAL_FLASHEx_Erase(&FLASH_EraseInit, &page_erase_error) != HAL_OK) return (0U);
	HAL_FLASH_Lock();
	
	fl_destination = BOOTLOADER_ADDR;
	fl_address = NEW_BLDR_EXT_ADDR;
	file_size = BOOTLOADER_ADDR - INT_ROM_ADDR;
    
    while(file_size)
	{
		if(file_size >= BL_BUFFER_SIZE) 
        {
            bcnt = BL_BUFFER_SIZE;
            file_size -= BL_BUFFER_SIZE;
        }
		else 
        {
            bcnt = file_size;
            file_size = 0U;
        }
        
//        SPI_FLASH_ReadPage(fl_address, buff,  bcnt);		
		ram_source = (uint32_t) buff;
        if(FLASH_If_Write(fl_destination, (uint32_t*) ram_source, (bcnt / 4)) == 0U) return (0U);
        fl_destination += bcnt;
        fl_address += bcnt;
	}
    
	return (1U);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
static void BackupOldFirmware(void)
{
    uint8_t buff[BUFF_SIZE];
    uint8_t *fl_address;
    uint32_t b_cnt;
    uint32_t b_total;
	uint32_t fl_destination;
	
	b_cnt = 0U;
	b_total = 0U;
	fl_address = (uint8_t *)APPLICATION_ADDR;
	fl_destination = BKP_APPL_EXT_ADDR;
	
	FormatFileStorage(fl_destination, 1U);
	
	while(b_total < (INT_ROM_ADDR + INT_ROM_SIZE))
	{
		while(b_cnt < BUFF_SIZE)
		{
			buff[b_cnt] = *fl_address;
            ++fl_address;
			++b_cnt;
			++b_total;
		}
		
//		SPI_FLASH_WritePage(fl_destination, buff, BUFF_SIZE);
//		if(SPI_FLASH_WaitReadyStatus(SPI_FLASH_TIMEOUT) == 0U) Error_Handler(EEPROM_FUNC, SPI_DRV);					
		fl_destination += BUFF_SIZE;
		b_cnt = 0U;
	}
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static uint32_t FLASH_Sector(uint32_t addr)
{
    if      ((addr < ADDR_FLASH_SECTOR_1) && (addr >= ADDR_FLASH_SECTOR_0)) return(FLASH_SECTOR_0);
    else if ((addr < ADDR_FLASH_SECTOR_2) && (addr >= ADDR_FLASH_SECTOR_1)) return(FLASH_SECTOR_1);
    else if ((addr < ADDR_FLASH_SECTOR_3) && (addr >= ADDR_FLASH_SECTOR_2)) return(FLASH_SECTOR_2);
    else if ((addr < ADDR_FLASH_SECTOR_4) && (addr >= ADDR_FLASH_SECTOR_3)) return(FLASH_SECTOR_3);
    else if ((addr < ADDR_FLASH_SECTOR_5) && (addr >= ADDR_FLASH_SECTOR_4)) return(FLASH_SECTOR_4);
    else if ((addr < ADDR_FLASH_SECTOR_6) && (addr >= ADDR_FLASH_SECTOR_5)) return(FLASH_SECTOR_5);
    else if ((addr < ADDR_FLASH_SECTOR_7) && (addr >= ADDR_FLASH_SECTOR_6)) return(FLASH_SECTOR_6);
    else if (addr >= ADDR_FLASH_SECTOR_7) return(FLASH_SECTOR_7);
    else return 0xFFU;
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void FLASH_Init(void)
{
    HAL_FLASH_Unlock(); 

    /* Clear pending flags (if any) */  
    __HAL_FLASH_CLEAR_FLAG (FLASH_FLAG_EOP    | FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR | 
                            FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_ERSERR);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static uint8_t FLASH_Erase(uint32_t addr, uint32_t size)
{
    uint32_t first_sector = 0U;
    uint32_t last_sector = 0U;
    uint32_t sector_err = 0U;
    FLASH_EraseInitTypeDef pEraseInit;

    /* Get the sector where start the user flash area */
    first_sector = FLASH_Sector(addr);
    last_sector  = FLASH_Sector(addr + size);
    if ((first_sector == 0xFFU) || 
        (first_sector >= FLASH_SECTOR_TOTAL))   return (1U);
    if (( last_sector == 0xFFU) || 
        ( last_sector >= FLASH_SECTOR_TOTAL))   return (2U);
    if ( first_sector > last_sector)            return (3U);
    /* Unlock the Flash to enable the flash control register access *************/ 
    FLASH_Init();
    
    pEraseInit.TypeErase = TYPEERASE_SECTORS;
    pEraseInit.Sector = first_sector;
    pEraseInit.NbSectors = ((last_sector - first_sector) + 1U);
    pEraseInit.VoltageRange = VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&pEraseInit, &sector_err) != HAL_OK)
    {
        /* Error occurred while page erase */
        return (4U);
    }

    return (0U);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static uint8_t FLASH_Write(uint32_t addr, uint32_t *buff, uint32_t size)
{
    uint32_t i = 0U;

    for (i = 0U; i < size; i++)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */ 
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, *(uint32_t*)(buff + i)) == HAL_OK)      
        {
            /* Check the written value */
            if (*(uint32_t*)addr != *(uint32_t*)(buff + i))
            {
                /* Flash content doesn't match SRAM content */
                return(FLASHIF_WRITINGCTRL_ERROR);
            }
            /* Increment FLASH destination address */
            addr += 4U;
        }
        else
        {
            /* Error occurred while writing data in Flash memory */
            return (FLASHIF_WRITING_ERROR);
        }
    }

    return (FLASHIF_OK);
}

/******************************   END OF FILE  **********************************/
