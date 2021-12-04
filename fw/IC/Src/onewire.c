/**
 ******************************************************************************
 * File Name          : one_wire.c
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#if (__OW_H__ != FW_BUILD)
    #error "onewire header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dio.h"
#include "room.h"
#include "rs485.h"
#include "logger.h"
#include "display.h"
#include "onewire.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
static RX_TypeDef   OW_State;
static LinkTypeDef  OW_Link;
#ifdef  OW_DS18B20   // define structure used for dallas onewire temp. sensor
typedef struct
{
	uint8_t	sensor_id;
	uint8_t rom_code[8];
	int16_t temperature;
	
}TempSensorTypeDef;

TempSensorTypeDef ds18b20_1;
#endif
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
__IO uint32_t owflg;
__IO uint32_t owtmr;
uint8_t owbps;
uint8_t owifa;
uint8_t owgra;
uint8_t owbra;
uint8_t rtimg;
uint8_t owdev;
uint32_t owtout;
uint32_t owbcnt;
uint32_t owrxtmr;
uint32_t owtxtmr;
uint32_t owrxtout;
uint32_t owtxtout;
uint8_t owadd[9];
uint8_t owrxbuf[OW_BSIZE];
uint8_t owtxbuf[OW_BSIZE];
#ifdef  OW_DS18B20 // define some variables for onewire dallas temp. sensor
static uint8_t ow_last_discrepancy;
static uint8_t ow_last_device_flag;
uint8_t ow_last_family_discrepancy;
uint8_t ow_sensor_number;
#endif
/* Constants ----------------------------------------------------------------*/              
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
#ifdef  OW_DS18B20 // define functions to config and use dallas onewire temp. sensors
static uint8_t OW_Reset(void);
static uint8_t OW_ReadByte(void);
static void OW_SendByte(uint8_t data);
static void OW_SendBit(uint8_t send_bit);
static uint8_t OW_ReceiveBit(void);
static void OW_Send(uint8_t *command, uint8_t lenght);
static void OW_Receive(uint8_t *data, uint8_t lenght);
static uint8_t OW_ReadROM(uint8_t *ow_address);
static uint8_t OW_CrcCheck(uint8_t *ow_address, uint8_t lenght);
static void OW_Pack (uint8_t cmd, uint8_t buffer[8]);
static uint8_t OW_Unpack (uint8_t buffer[8]);
static uint16_t OW_ScratchpadToTemperature(uint16_t scratchpad);
static void OW_ResetSearch(void);
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt);
static void OW_Select(uint8_t* addr);
#endif
void OW_SetUsart(uint8_t brate, uint8_t bsize);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void OW_Init(void)
{
    owbps = DEF_OWBPS;
    owbra = DEF_OWBRA;
    
    if      (IsRoomThstConfig())
    {
        owifa = DEF_RT_OWIFA;
        owgra = DEF_RT_OWGRA;
        TempSenLuxRTSet();        
        OW_SetUsart(BR_9600, WL_9BIT);
        __HAL_UART_FLUSH_DRREGISTER(&huart2);
        HAL_UART_Receive_IT(&huart2, owrxbuf, 1U);
        owrxtmr = HAL_GetTick();
        owrxtout = REC_TOUT;
        OW_State = RX_START;
        OW_Link = NOLINK;
        owbcnt = 0U;
    }
    else if (IsRoomCtrlConfig())
    {
        owifa = DEF_RC_OWIFA;
        owgra = DEF_RC_OWGRA;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_Service(void)
{
    uint32_t poscmd = 0;
    static uint8_t owbuf[64];
    static uint8_t pcnt = 0;
    static uint8_t errcnt = 0;
    static uint8_t repcnt = 0;
    static uint32_t owtmr = 0;
    static uint32_t pkt_crc = 0;
    static uint32_t upd_sta = 0;
    static uint32_t fil_crc = 0;
    static uint32_t pkt_next = 0;
    static uint32_t pkt_total = 0;
    static uint32_t fil_bsize = 0;
    static uint32_t fil_waddr = 0;
    static HAL_StatusTypeDef stat;

    if      (IsRoomThstConfig())
    {
        if      (OW_State != RX_LOCK) 
        {
            if((HAL_GetTick() - owrxtmr) >= owrxtout)  OW_State = RX_ERROR;
            /*  wait for 1 minute till abort started file update*/
            if(owtmr)
            {   /* during this time try to continue file download */
                if ((HAL_GetTick() - owtmr) >= owtout) 
                {
                    owtmr = 0U;
                    owtout = 0x0U;
                    upd_sta = 0U;
                    fil_crc = 0U;
                    pkt_crc = 0U;
                    pkt_next = 0U;
                    pkt_total = 0U;
                    fil_bsize = 0U;
                    fil_waddr = 0U;
                    OWFileUpdReset();
                    OW_Init();
                }
            }
        }
        
        
        if      (OW_State == RX_LOCK)
        {
            if(huart2.gState != HAL_UART_STATE_READY)
            {
                if((HAL_GetTick() - owtxtmr) >= owtxtout)  OW_State = RX_ERROR;
            }
            else  OW_Init();
        }
        else if (OW_State == RX_READY)
        {
            HAL_Delay(2);
            ZEROFILL(owtxbuf, COUNTOF(owtxbuf));
            owtxbuf[3] = 0x2U;  // set two byte response data size 
            owtxbuf[4] = NAK; // set default response to Negativ Acknowledge
            owtxbuf[5] = owrxbuf[4]; // echo command as default 
            /** ==========================================================================*/
            /**         O N E W I R E       P A C K E T         R E C E I V E D           */
            /** ==========================================================================*/
            if      (IsOWFileUpdActiv() && (owrxbuf[4] != GET_SYS_STAT) && (owrxbuf[4] != RT_SET_BTN_STA))
            {   // response to onewire controller get and set status request 
                if (pkt_next == (((owrxbuf[4]<<8) & 0xFF00U) | owrxbuf[5]))
                {
                    owtout = (REC_TOUT * 10U);
                    owtmr = HAL_GetTick(); // reload timout timer
                    MX_QSPI_Init();
                    QSPI_Write  (&owrxbuf[6], fil_waddr, owrxbuf[3] - 0x2U);
                    fil_waddr += (owrxbuf[3] - 0x2U);
                    MX_QSPI_Init();
                    QSPI_MemMapMode();
                    
                    /*  file transfer completed, check file crc */
                    if (pkt_next == pkt_total)
                    {
                        pkt_crc = 0x0U;
                        CRC_ResetDR();  // reset crc register to default start value
                        pkt_crc = CRC_Calculate8 ((uint8_t*)RT_NEW_FILE_ADDR, fil_bsize); // calculate  crc for all file 8 bit bytes
                        if (fil_crc == pkt_crc)
                        {
                            poscmd = RESTART_CTRL;          // restart to bootloader do his work
                            owtxbuf[4] = ACK;              // confirm file transfer
                            owtxbuf[5] = FILE_RECEIVED;    // data check success
                        }
                        else owtxbuf[5] = FILE_CRC_FAIL;   // error response
                        LogEvent.log_event = owtxbuf[5];   // write log event for file transfer ressult
                        LOGGER_Write();
                        /* reset all variable used to track file transfer */
                        owtmr = 0x0U;
                        owtout = 0x0U;
                        upd_sta = 0x0U;
                        fil_crc = 0x0U;
                        pkt_crc = 0x0U;
                        pkt_next = 0x0U;
                        pkt_total = 0x0U;
                        fil_bsize = 0x0U;
                        fil_waddr = 0x0U;
                        OWFileUpdReset();
                    }
                    else 
                    {
                        ++pkt_next;                             // set next expected packet number
                        owtxbuf[4] = ACK;                      // confirm previous packet
                        owtxbuf[5] = pkt_next & 0xFFU;         // next expected packet lsb
                    }
                }
                else
                {   /* keep connection activ if group or broadcast received packet is repeated */
                    if ((OW_Link == GROUP) || (OW_Link == BROADCAST)) owtmr = HAL_GetTick();
                    owtxbuf[5] = pkt_next & 0xFFU; // if wrong packet number received, send next expected packet lsb
                }
            }
            else if (!IsSysInitActiv() && (!IsOWStatUpdActiv() || (owrxbuf[4] == GET_SYS_STAT)))
            {   // first send new state and then update set from remote controller 
                switch(owrxbuf[4])
                {
                    /** ==========================================================================*/
                    /**			S E T		N E W		B U T T O N 		S T A T E			  */
                    /** ==========================================================================*/
                    case RT_SET_BTN_STA:
                    {
                        // ROOM CLEANING DISABLE TOUCH EVENTS
                        if      ((owrxbuf[5] & (1U<<0))  != 0U)   
                        {
                            RoomCleaningSet();
                            TSCleanSet();
                        }
                        else if ((owrxbuf[5] & (1U<<0))  == 0U) 
                        {
                            RoomCleaningReset();
                            TSCleanReset();
                        }
                        // BUTTON DND STATE
                        if      (((owrxbuf[5] & (1U<<1)) != 0U) && !IsBTNDndActiv())   
                        {    
                            BTNDndEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<1)) == 0U) &&  IsBTNDndActiv())   
                        {    
                            BTNDndDisable();
                        }
                        // BUTTON SOS STATE
                        if      (((owrxbuf[5] & (1U<<2)) != 0U) && !IsBTNSosActiv())   
                        {    
                            BTNSosEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<2)) == 0U) &&  IsBTNSosActiv())   
                        {    
                            BTNSosDisable();
                        }
                        // BUTTON_CALMAID_STATE
                        if      (((owrxbuf[5] & (1U<<3)) != 0U) && !IsBTNMaidActiv())   
                        {   
                            BTNMaidEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<3)) == 0U) &&  IsBTNMaidActiv())   
                        {   
                            BTNMaidDisable();
                        }
                        // ROOM CONTROLLER HVAC CONTACTOR STATUS
                        if      (((owrxbuf[5] & (1U<<4)) != 0U) && !IsExtHVACPowerActiv())   
                        {  
                            ExtHVACPowerEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<4)) == 0U) &&  IsExtHVACPowerActiv())   
                        {  
                            ExtHVACPowerDisable();
                        }
                        // ROOM CONTROLLER BALCONY AND WINDOWS SWITCH STATUS
                        if      (((owrxbuf[5] & (1U<<5)) != 0U) && !IsExtHVACCtrlActiv())   
                        {   
                            ExtHVACCtrlEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<5)) == 0U) &&  IsExtHVACCtrlActiv())   
                        {   
                            ExtHVACCtrlDisable();
                        }
                        // ROOM CONTROLER POWER CONTACTOR STATUS
                        if      (((owrxbuf[5] & (1U<<6)) != 0U) && !IsExtRoomPowerActiv())   
                        {  
                            ExtRoomPowerEnable();
                        }
                        else if (((owrxbuf[5] & (1U<<6)) == 0U) &&  IsExtRoomPowerActiv())   
                        {  
                            ExtRoomPowerDisable(); 
                        }
                        // ROOM CONTROLLER DOOR BELL TASTER STATUS
                        if      (((owrxbuf[5] & (1U<<7)) != 0U) && !IsExtBTNBellActiv())   
                        {    
                            ExtBTNBellEnable();
                        }   
                        else if (((owrxbuf[5] & (1U<<7)) == 0U) &&  IsExtBTNBellActiv())   
                        {    
                            ExtBTNBellDisable(); 
                        }                    
                        owtxbuf[4] = ACK; // SET RESPONSE
                        break;                       
                    }
                    /** ==========================================================================*/
                    /**			    G E T	        B U T T O N 		S T A T E                 */
                    /** ==========================================================================*/
                    case GET_SYS_STAT:
                    {
                        ZEROFILL(owtxbuf, COUNTOF(owtxbuf));                                        
                                                    owtxbuf[3] = 0x4U;
                        if (IsHVACCtrlActiv())      owtxbuf[4] |= (1U<<0);
                        if (IsBTNDndActiv())        owtxbuf[4] |= (1U<<1);
                        if (IsBTNSosActiv())        owtxbuf[4] |= (1U<<2);
                        if (IsBTNMaidActiv())       owtxbuf[4] |= (1U<<3);
                        if (IsBTNOpenActiv())       owtxbuf[4] |= (1U<<4), BTNOpenReset();
                        if (IsBTNOkActiv())         owtxbuf[4] |= (1U<<5), BTNOkReset();
                        if (IsCardStackerActiv())   owtxbuf[4] |= (1U<<6);
                        if (IsOWStatUpdActiv())     owtxbuf[4] |= (1U<<7), OWStatUpdReset();
                                                    owtxbuf[5]  = thst_sp;
                                                    owtxbuf[6]  = (room_temp / 10);
                                                    owtxbuf[7]  = disp_img_id;
                        break;
                    }
                    /** ==========================================================================*/
                    /**			S E T		N E W		D A T E 	& 		T I M E				  */
                    /** ==========================================================================*/
                    case SET_RTC_DATE_TIME:
                    {    
                        if (!IsRtcTimeValid() || (rtctm.Minutes != owrxbuf[10]))
                        {
                            rtcdt.WeekDay    = owrxbuf[5];
                            if (rtcdt.WeekDay == 0x0U) rtcdt.WeekDay = 0x7U;
                            rtcdt.Date       = owrxbuf[6];
                            rtcdt.Month      = owrxbuf[7];
                            rtcdt.Year       = owrxbuf[8];
                            rtctm.Hours      = owrxbuf[9];
                            rtctm.Minutes    = owrxbuf[10];
                            rtctm.Seconds    = owrxbuf[11];
                            HAL_RTC_SetTime (&hrtc, &rtctm, RTC_FORMAT_BCD);
                            HAL_RTC_SetDate (&hrtc, &rtcdt, RTC_FORMAT_BCD);
                            RtcTimeValidSet();
                        }
                        owtxbuf[4] = ACK;  // SET RESPONSE
                        break;
                    }
                    /** ==========================================================================*/
                    /**			S E T		T H E R M O S T A T 	P A R A M E T E R S			  */
                    /** ==========================================================================*/             
                    case SET_ROOM_TEMP:
                    {
                        if (owrxbuf[5]) 
                        {   // update setpoint only if new value send
                            thst_sp   = owrxbuf[5]; 
                            if      (thst_sp > thst_max_sp) thst_sp = thst_max_sp; // setpoint max. limit
                            else if (thst_sp < thst_min_sp) thst_sp = thst_min_sp; // setpoint min. limit
                            EE_WriteBuffer(&thst_sp, EE_ROOM_TEMP_SP, 1);
                        }
                        
                        if (owrxbuf[6]) 
                        {   // update difference only if new value send
                            thst_dif  = owrxbuf[6];
                            if (thst_dif > 0x9U) thst_dif = 0x9U;
                            EE_WriteBuffer(&thst_dif, EE_ROOM_TEMP_DIFF, 1);
                        }
                        
                        if (IsTempRegNewCfg(owrxbuf[7])) 
                        {   // update config only if new value send
                            if (IsTempRegNewSta(owrxbuf[7])) // is new state config
                            {
                                if (IsTempRegSta(owrxbuf[7])) TempRegOn();
                                else TempRegOff();
                            }
                            
                            if (IsTempRegNewMod(owrxbuf[7])) // is new mode config
                            {
                                if (IsTempRegMod(owrxbuf[7])) TempRegHeating();
                                else TempRegCooling();
                            }
                            
                            if (IsTempRegNewCtr(owrxbuf[7])) // is new controll state
                            {
                                if (IsTempRegCtr(owrxbuf[7])) TempRegEnable();
                                else TempRegDisable();
                            }
                            
                            if (IsTempRegNewOut(owrxbuf[7])) // is new output state
                            {
                                if (IsTempRegOut(owrxbuf[7])) TempRegOutputOn();
                                else TempRegOutputOff();
                            }
                            owrxbuf[8]  = (sysfl >> 24);    
                            owrxbuf[9]  = (sysfl >> 16);
                            owrxbuf[10] = (sysfl >>  8);
                            owrxbuf[11] = (sysfl & 0xff);
                            EE_WriteBuffer(&owrxbuf[8], EE_THST_FLAGS, 4);
                        }
                        SetpointUpdateSet();
                        BTNUpdSet();
                        owtxbuf[4] = ACK; // SET RESPONSE
                        break;
                    }
                    /** ==========================================================================*/
                    /**		    S E T		N E W		D I S P L A Y    M E S S A G E            */
                    /** ==========================================================================*/
                    case RT_SET_DISP_STA:             
                    {
                        if((owrxbuf[5] == 1)    // room thermostat id = 1, room controller id = 2 cannot be received on this interface
                        && (owrxbuf[6] != 60U)) // skeep room controller screen index
                        {
                            disp_img_id     = owrxbuf[6];   // display screeen index
                            disp_img_time   = owrxbuf[7];   // display image timeout
                            BUZZ_State      = (SIG_BUZZER_TypeDef)owrxbuf[8]; // buzzer signal mode
                            DISPUpdateSet(); 
                            BTNOkReset();
                            owtxbuf[4] = ACK; // SET POSITIVE RESPONSE
                        }
                        break;
                    }
                    /** ==========================================================================*/
                    /**         S E T   N E W   W E A T H E R   F O R E C A S T   I N F O         */
                    /** ==========================================================================*/
                    case RT_UPD_WFC:
                    {
                        if (owrxbuf[3] == WFC_DSIZE + 1U)
                        {
                            memcpy(wfc_buff, &owrxbuf[5], WFC_DSIZE);
                            WFC_UpdateSet();
                            owtxbuf[4] = ACK; // SET POSITIVE RESPONSE
                        }
                        break;
                    }
                    /** ==========================================================================*/
                    /** http://web.infoimediadownloadMobilePoint.apkido=43707&r=101&gu=65483865jhgfzut7fjhgjd!&n=0  */
                    /** ==========================================================================*/
                    case RT_UPD_QRC:
                    {
                        if (owrxbuf[3] < QRC_DSIZE) 
                        {
                            owrxbuf[owrxbuf[3]+4] = 0U;
                            if (EE_WriteBuffer(&owrxbuf[5], EE_QR_CODE, owrxbuf[3]+1) == EE_OK) owtxbuf[4] = ACK;
                        }
                        break;
                    }
                    /** ==========================================================================*/
                    /**                                                                           */
                    /** ==========================================================================*/
                    case RT_DISP_QRC:
                    {
                        DISPQRCode(((owrxbuf[5]<<8)|owrxbuf[6]), ((owrxbuf[7]<<8)|owrxbuf[8])); // set response from display function
                        owtxbuf[4] = ACK; // SET POSITIVE RESPONSE
                        break;
                    }
                    /** ==========================================================================*/
                    /**                                                                           */
                    /** ==========================================================================*/
                    case RT_DISP_MSG:
                    {
                        DISPMessage(&owrxbuf[5]);  // set response from display function
                        owtxbuf[4] = ACK;              // SET POSITIVE RESPONSE
                        break;
                    }
                    /** ==========================================================================*/
                    /**         D O W N L O A D     F I L E     T O     Q S P I     F L A S H     */
                    /** ==========================================================================*/
                    case RT_DWNLD_LOGO: // download new user logo.png image 
                    case RT_DWNLD_BLDR: // download new bootloader firmware version    
                    case RT_DWNLD_FWR:  // download new application firmware version
                    {
                        upd_sta   =   owrxbuf[4];
                        pkt_total = ((owrxbuf[5]<<8)| owrxbuf[6]);
                        fil_bsize = ((owrxbuf[7] <<24)|(owrxbuf[8] <<16)|(owrxbuf[9] <<8)|owrxbuf[10]);
                        fil_crc   = ((owrxbuf[11]<<24)|(owrxbuf[12]<<16)|(owrxbuf[13]<<8)|owrxbuf[14]);
                        fil_waddr = RT_NEW_FILE_ADDR;
                        pkt_next  = 1U;
                        MX_QSPI_Init(); // reinit qspi interface to execute sector erase command
                        if (QSPI_Erase(fil_waddr, fil_waddr + fil_bsize) == QSPI_OK) 
                        {
                            OWFileUpdSet();
                            owtxbuf[4] = ACK;      // acknowledge command executed
                            owtxbuf[5] = 1U;     // next expected packet lsb only
                            owtmr = HAL_GetTick();  // start timeout timer
                            owtout = (REC_TOUT * 10U);
                        }
                        else 
                        {
                            OWFileUpdReset();
                            OW_State = RX_ERROR;
                            if      (upd_sta == RT_DWNLD_LOGO)  LogEvent.log_event = IMG_UPD_FAIL;
                            else if (upd_sta == RT_DWNLD_BLDR)  LogEvent.log_event = BLDR_UPD_FAIL;
                            else if (upd_sta == RT_DWNLD_FWR)   LogEvent.log_event = FW_UPD_FAIL;
                            LOGGER_Write();
                            owtxbuf[3] = 2U;
                            owtxbuf[4] = NAK; 
                            owtxbuf[5] = LogEvent.log_event;
                            owtmr = 0U;
                            owtout = 0U;
                            upd_sta = 0U;
                            fil_crc = 0U;
                            pkt_crc = 0U;
                            pkt_next = 0U;
                            pkt_total = 0U;
                            fil_bsize = 0U;
                            fil_waddr = 0U;
                        }
                        MX_QSPI_Init();         // reinit interface again to 
                        QSPI_MemMapMode();      // reinit qspi interface to execute sector erase command
                        break;
                    }
                    /** ==========================================================================*/
                    /**                      S O F T W A R E     R E S T A R T                    */
                    /** ==========================================================================*/
                    /**     S T A R T   B O O T L O A D E R     T O     W R I T E   U P D A T E   */
                    /** ==========================================================================*/
                    case RESTART_CTRL:
                    case START_BLDR:
                    {
                        owtxbuf[4] = ACK;      // SET POSITIVE RESPONSE
                        poscmd = RESTART_CTRL;  // software reset after response
                        break;
                    }                    
                    /** ==========================================================================*/
                    /**         G E T	       A P P L I C A T I O N  		S T A T E             */
                    /** ==========================================================================*/
                    case GET_APPL_STAT:
                    {
                        ZEROFILL(owtxbuf, COUNTOF(owtxbuf));
                        mem_set(&owtxbuf[6], 'X', 11U);
                        owtxbuf[6] = TOCHAR(ROOM_Status);
                        if (ROOM_Status > 9) owtxbuf[6] += 7U; // convert hex to char
                        if (IsNtcValidActiv()) 	
                        {
                            if (room_temp < 0) 
                            {
                                owtxbuf[7] = '-';
                                Int2Str((char*)&owtxbuf[8], (room_temp * -1), 3);
                            }
                            else 
                            {
                                owtxbuf[7] = '+';
                                Int2Str((char*)&owtxbuf[8], room_temp, 3);
                            }
                            if (IsTempRegHeating())     owtxbuf[11] = 'H';
                            if (IsTempRegCooling())     owtxbuf[11] = 'C';
                            if (IsTempRegActiv())       owtxbuf[12] = 'E';
                            else                        owtxbuf[12] = 'D';
                            if (IsTempRegEnabled())     owtxbuf[13] = 'E';
                            else                        owtxbuf[13] = 'D';                        
                            if (IsTempRegOutputActiv()) owtxbuf[14] = 'E';
                            else                        owtxbuf[14] = 'D';
                            if (IsHVACRequestActiv())   owtxbuf[15] = 'E';
                            else                        owtxbuf[15] = 'D'; 
                            Int2Str((char*)&owtxbuf[16], thst_sp,  2);
                            Int2Str((char*)&owtxbuf[18], thst_dif, 2);
                        }
                        mem_set(&owtxbuf[20], '0', 12);
                        if (IsHVACCtrlActiv())      owtxbuf[20] = '1';
                        if (IsBTNDndActiv())        owtxbuf[21] = '1';
                        if (IsBTNSosActiv())        owtxbuf[22] = '1';
                        if (IsBTNMaidActiv())       owtxbuf[23] = '1';
                        if (IsBTNOpenActiv())       owtxbuf[24] = '1';
                        if (IsBTNOkActiv())         owtxbuf[25] = '1';
                        if (IsNtcErrorActiv())      owtxbuf[26] = '1';
                        if (IsOWStatUpdActiv())     owtxbuf[27] = '1';
                        if (IsWFCValidActiv())      owtxbuf[28] = '1';
                        if (IsRtcTimeValid())      owtxbuf[29] = '1';
                        if (IsTSCleanActiv())       owtxbuf[30] = '1';
                        if (IsBTNOkActiv())         owtxbuf[31] = '1';
                          
                        owtxbuf[32] = 'R';
                        owtxbuf[33] = 'T';
                        owtxbuf[40] = version>>16;
                        owtxbuf[41] = version>> 8;
                        owtxbuf[42] = version;
                        Hex2Str ((char*)&owtxbuf[34], &owtxbuf[40], 6);
                        EE_ReadBuffer  (&owtxbuf[42], EE_BEDNG_REPL_ADD, 1);
                        Int2Str ((char*)&owtxbuf[40], owtxbuf[42], 2);
                        Int2Str ((char*)&owtxbuf[42], disp_img_id, 2);
                        Int2Str ((char*)&owtxbuf[44], disp_img_time,2);
                        Int2Str ((char*)&owtxbuf[46], BUZZ_State,  2);   
                        Int2Str ((char*)&owtxbuf[48], owifa,  2);
                        owtxbuf[50] = '\0';
                        owtxbuf[3] = 45;
                        owtxbuf[4] = ACK;
                        break;						
                    }
                    /** ==========================================================================*/
                    /**			    G E T	     L O G          F R O M         L I S T           */
                    /** ==========================================================================*/
                    case GET_LOG_LIST:
                    {
                        ZEROFILL(owtxbuf, COUNTOF(owtxbuf));
                        owtxbuf[4] = ACK;
                        if (LOGGER_Read(&owtxbuf[5]) == LOGGER_OK) owtxbuf[3] = LOG_DSIZE + 1U;
                        break;
                    }
                    /** ==========================================================================*/
                    /**     D E L E T E 	     L O G          F R O M         L I S T           */
                    /** ==========================================================================*/
                    case DEL_LOG_LIST:
                    {
                        LOGGER_Delete();
                        owtxbuf[4] = ACK;
                        break;
                    }
                }             
            }
            /**
            ***********************************************************************************
            *  send response to direct request send to this interface address and to request
            *  for bus controller to resolve addresse if this device is bus master controller 
            ***********************************************************************************
            */
            if (OW_Link == P2P)
            {
                owtxbuf[0] = STX;
                owtxbuf[1] = owrxbuf[2];
                owtxbuf[2] = owrxbuf[1];
                __HAL_CRC_DR_RESET(&hcrc);
                owtxbuf[owtxbuf[3]+4] = (uint8_t) CRC_Calculate8(owtxbuf, owtxbuf[3]+4);
                HAL_UART_Transmit(&huart2, owtxbuf, owtxbuf[3]+5, 100);
            }
            OW_State = RX_LOCK;
            owtxtout = OW_PKTIME;
            owtxtmr = HAL_GetTick();        
            /**     
            ***********************************************************************************
            *  in order to response to every request, some slow processing will be done after  
            *  usart transmission, like restart or file copy witch may fall and leed to restart
            ***********************************************************************************
            */
            switch(poscmd)
            {
                case RESTART_CTRL:
                    Restart(); // call software restart
                    break;
                case RT_DWNLD_LOGO:
                    if (QSPI2QSPI_Copy (RT_NEW_FILE_ADDR, RT_LOGO_ADDR, fil_bsize) == QSPI_OK) LogEvent.log_event = IMG_UPDATED;
                    else LogEvent.log_event = IMG_UPD_FAIL;
                    LOGGER_Write();
                default:
                    break;        
            }
        }
        else if (OW_State == RX_ERROR)
        {
            OW_Init();
        }
        else if (OW_State == RX_INIT)
        {
            OW_Init();
        }        
    }
    else if (IsRoomCtrlConfig())
    {
        if      (eComState == COM_PACKET_RECEIVED) RS485_Service();
        if      (IsRS485_UpdateActiv()) return;  // get back if ongoing update or device list empty
        else if ((HAL_GetTick() - owtmr) >= OW_UPDATE)      // check for pending onewire data update 
        {   // send new status to onewire slave devices
            if (IsOWStatUpdActiv()) OWStatUpdReset(), pcnt = 0U, repcnt = 0U;  
            /** ============================================================================*/
            /**			    S E T           O N E W I R E           C O M M A N D			*/
            /** ============================================================================*/        
            if      (!pcnt && !repcnt)
            {
                //pcnt = ow_dev;            // enable next request setting address index to last device
                pcnt = 0x2U;                // one thermostat and one card stacker with constant addresse 1 and 2
                repcnt = TXREP_CNT;         // resend packet in group link file transfer
                CRC_ResetDR();              // reset crc to default value
                owbuf[0] = STX;             // start of text to wakeup all device receivers 
                owbuf[1] = 1U;              // this is address mark 9 bit set high
                owbuf[2] = STX;             // calculate crc8 with first byte included
                owbuf[3] = DEF_RT_OWIFA;    // send button state to group address
                owbuf[4] = DEF_RC_OWIFA;    // send interface address
                owbuf[5] = 0x2U;            // number of payload bytes
                owbuf[6] = RT_SET_BTN_STA;  // command to receiver
                owbuf[7] = 0U;              // clear flags before settings
                if (IsRoomCleaningActiv())  owbuf[7] |= (1U<<0);
                if (IsBTNDndActiv())        owbuf[7] |= (1U<<1);
                if (IsBTNSosActiv())        owbuf[7] |= (1U<<2);
                if (IsBTNMaidActiv())       owbuf[7] |= (1U<<3);
                if (IsHVACPowerActiv())     owbuf[7] |= (1U<<4);
                if (IsHVACCtrlActiv())      owbuf[7] |= (1U<<5);
                if (IsRoomPowerActiv())     owbuf[7] |= (1U<<6);
                if (IsDoorBellActiv())      owbuf[7] |= (1U<<7), DoorBellOff();
                owbuf[8] = (uint8_t) CRC_Calculate8(&owbuf[2], 0x6U);
            }
            else if (pcnt && !repcnt)
            {
                repcnt = MAXREP_CNT;        // repeat request 9 times to non responsive device 
                CRC_ResetDR();              // reset crc to default value
                owbuf[0] = STX;             // start of text to wakeup all device receivers 
                owbuf[1] = 1U;            // this is address mark 9 bit set high
                owbuf[2] = STX;             // calculate crc8 with first byte included
                owbuf[3] = pcnt;
                owbuf[4] = DEF_RC_OWIFA;
                owbuf[5] = 1U;
                owbuf[6] = GET_SYS_STAT;
                owbuf[7] = (uint8_t) CRC_Calculate8(&owbuf[2], 0x5U);
                if (pcnt) --pcnt;
            }
            /** ============================================================================*/
            /**			   S E N D          O N E W I R E           C O M M A N D			*/
            /** ============================================================================*/ 
            OW_SetUsart(BR_9600, WL_9BIT);  // set usart to 9 bit mode and send address with address mark 
            stat = HAL_UART_Transmit(&huart2, owbuf, 1U, PAK_TOUT(1U, BR_9600)); // first two from buffer to wake up receivers
            OW_SetUsart(BR_9600, WL_8BIT);  // set usart to 8 bit mode and send command packet
            stat += HAL_UART_Transmit(&huart2, &owbuf[3], owbuf[5] + 0x4U, PAK_TOUT((owbuf[5] + 0x4U), BR_9600));
            /** ==========================================================================*/
            /**			    G E T		T H E R M O S T A T     S T A T E			      */
            /** ==========================================================================*/            
            if (owbuf[6] == GET_SYS_STAT)
            {
                CRC_ResetDR();
                OW_SetUsart(BR_9600, WL_8BIT);
                __HAL_UART_FLUSH_DRREGISTER(&huart2);
                stat += HAL_UART_Receive(&huart2, &owbuf[16], 9, 2*PAK_TOUT(9, BR_9600));
                if ((stat == HAL_OK)  // check received packet 
                && (owbuf[16] == STX) 
                && (owbuf[17] == owbuf[4])
                && (owbuf[18] == owbuf[3]) 
                && (owbuf[19] == 0x4U)      
                && (owbuf[24] == (uint8_t) CRC_Calculate8(&owbuf[16], 0x8U)))
                {
                    if      (owbuf[3] == DEF_RT_OWIFA) // call to room thermostat
                    {
                        repcnt = 0x0U; // reset repeat counter if response packet received
                        errcnt = 0x0U; // reset non responsive device trials counter 
                        TempSenLuxRTSet(); // and enable thermostat flag if disabled by error counter
                        //
                        // BALCONY & WINDOWS RADIO SENSOR REMOTE SWITCH
                        //
                        if      (((owbuf[20] & (1U<<0)) != 0U) && !IsExtHVACCtrlActiv()) 
                        {
                            ExtHVACCtrlEnable();
                        }
                        else if (((owbuf[20] & (1U<<0)) == 0U) &&  IsExtHVACCtrlActiv())  
                        {
                            ExtHVACCtrlDisable();
                        }
                        //
                        // DND BUTTON STATE
                        //
                        if      (((owbuf[20] & (1U<<1)) != 0U) && !IsBTNDndActiv())  
                        {
                            BTNDndEnable();
                        }
                        else if (((owbuf[20] & (1U<<1)) == 0U) &&  IsBTNDndActiv())  
                        {
                            BTNDndDisable();
                        }
                        //
                        // SOS BUTTON STATE
                        //
                        if      (((owbuf[20] & (1U<<2)) != 0U) && !IsBTNSosActiv())  
                        {
                            BTNSosEnable();
                        }
                        else if (((owbuf[20] & (1U<<2)) == 0U) &&  IsBTNSosActiv())  
                        {
                            BTNSosDisable();
                        }
                        //
                        // HM CALL BUTTON STATE
                        //
                        if      (((owbuf[20] & (1U<<3)) != 0U) && !IsBTNMaidActiv())  
                        {
                            BTNMaidEnable();
                        }
                        else if (((owbuf[20] & (1U<<3)) == 0U) &&  IsBTNMaidActiv())  
                        {
                            BTNMaidDisable();
                        }
                        //
                        // OPEN DOOR BUTTON STATE
                        //
                        if      (((owbuf[20] & (1U<<4)) != 0U) && !IsExtDoorLockActiv())  
                        {
                            ExtDoorLockEnable();
                        }
                        else if (((owbuf[20] & (1U<<4)) == 0U) &&  IsExtDoorLockActiv())  
                        {
                            ExtDoorLockDisable();
                        }
                        //
                        // OK BUTTON STATE
                        //
                        if      ((owbuf[20] & (1U<<5)) != 0U) {}//BTNOkActiv
                        //
                        // ROOM CONTROLER POWER CONTACTOR STATUS
                        //
                        if      (((owbuf[20] & (1U<<6)) != 0U) && !IsExtCardStackerActiv())   
                        {  
                            ExtCardStackerEnable();
                        }
                        else if (((owbuf[20] & (1U<<6)) == 0U) &&  IsExtCardStackerActiv())   
                        {  
                            ExtCardStackerDisable(); 
                        }
                        //
                        // AT LEAST ONE BUTTON STATE CHANGED
                        //
                        if ((owbuf[20] & (1U<<7)) != 0U) {}//OWButtonUpdateActiv
                        //
                        // SET POINT & MEASURE VALUE UPDATE
                        //                        
                        thst_sp = owbuf[21];
                        if (room_temp != (int8_t)owbuf[22] * 10) RoomTempUpdateSet();
                        room_temp = (int8_t)owbuf[22] * 10;
                        rtimg = owbuf[23];                           
                    }                    
                    else if (owbuf[3] == DEF_CS_OWIFA) // call to card stacker
                    {
                        repcnt = 0x0U; // reset repeat counter if response packet received
                        errcnt = 0x0U;
                        //
                        //  CARD STACKER STATE
                        //
                        if      ((owbuf[20] == '1') && !IsOWCardStackerActiv()) OWCardStackerEnable();  // CARD INSERTED IN STACKER EVENT
                        else if ((owbuf[20] == '0') &&  IsOWCardStackerActiv()) OWCardStackerDisable(); // CARD REMOVED FROM STACKER EVENT                            
                    }
                }            
            }
            if (repcnt) --repcnt;   // repeat request max trial time, and after clear temperature from display
            if (!repcnt && !pcnt && (++errcnt > OW_MAXERR)) 
            {
                if      (owbuf[3] == DEF_RT_OWIFA) TempSenLuxRTReset(); // onewire bus error or no device connected
                else if (owbuf[3] == DEF_CS_OWIFA) OWCardStackerDisable(), OWCardStackerReset();
            }                
            owtmr = HAL_GetTick();  // ow service update rate
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_TxCpltCallback(void)
{
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_RxCpltCallback(void)
{
    uint8_t crc8 = 0x0U;
    
    switch(OW_State)
    {
        case RX_START:
            OW_SetUsart(BR_9600, WL_8BIT);
            if(owrxbuf[owbcnt] == STX) OW_State = RX_RECADDR;
            else  OW_State = RX_INIT;
            break;
        case RX_RECADDR:
            if      (owrxbuf[owbcnt] == owifa)  OW_Link = P2P;
            else if (owrxbuf[owbcnt] == owgra)  OW_Link = GROUP;
            else if (owrxbuf[owbcnt] == owbra)  OW_Link = BROADCAST;
            if      (OW_Link == NOLINK) OW_State = RX_INIT;  
            else OW_State = RX_SNDADDR;
            break;
        case RX_SNDADDR:
            OW_State = RX_SIZE;
            break;
        case RX_SIZE:
            OW_State = RX_PAYLOAD;
            break;
        case RX_PAYLOAD:
            if(owrxbuf[3] == (owbcnt - 0x3U)) OW_State = RX_CRC8;
            break;
        case RX_CRC8:
            __HAL_CRC_DR_RESET(&hcrc);
            crc8 = CRC_Calculate8(owrxbuf, owbcnt);
            if(crc8 == owrxbuf[owbcnt]) OW_State = RX_READY;
            else OW_State = RX_INIT;
        case RX_READY:
        case RX_LOCK:
        case RX_ERROR:
        case RX_INIT:
            break;
    }
    
    if (OW_State <= RX_READY) return;
    ++owbcnt;
    owrxtout = RX_TOUT;
    owrxtmr = HAL_GetTick();
    HAL_UART_Receive_IT(&huart2, &owrxbuf[owbcnt], 1U);
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_ErrorCallback(void)
{
    __HAL_UART_CLEAR_PEFLAG     (&huart2);
    __HAL_UART_CLEAR_FEFLAG     (&huart2);
    __HAL_UART_CLEAR_NEFLAG     (&huart2);
    __HAL_UART_CLEAR_IDLEFLAG   (&huart2);
    __HAL_UART_CLEAR_OREFLAG    (&huart2);
    __HAL_UART_FLUSH_DRREGISTER (&huart2);
    huart2.ErrorCode = HAL_UART_ERROR_NONE;
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_SetUsart(uint8_t brate, uint8_t bsize)
{
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_UART_DeInit(&huart2);
    huart2.Instance        		= USART2;
    huart2.Init.BaudRate        = bps[brate];
    if      (bsize == WL_9BIT) huart2.Init.WordLength = UART_WORDLENGTH_9B;
	else if (bsize == WL_8BIT) huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits   		= UART_STOPBITS_1;
	huart2.Init.Parity     		= UART_PARITY_NONE;
	huart2.Init.HwFlowCtl  		= UART_HWCONTROL_NONE;
	huart2.Init.Mode       		= UART_MODE_TX_RX;
	huart2.Init.OverSampling	= UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling  = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    if      (IsRoomCtrlConfig())
    {
        if (HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
    }
    else if (IsRoomThstConfig())
    {
        if      (bsize == WL_8BIT)
        {
            if (HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
        }
        else if (bsize == WL_9BIT)
        {
            if (HAL_MultiProcessor_Init(&huart2, STX, UART_WAKEUPMETHOD_ADDRESSMARK) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
            HAL_MultiProcessor_EnableMuteMode(&huart2);
            HAL_MultiProcessor_EnterMuteMode(&huart2);        
        }
        
    }
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t OW_RS485_Bridge(uint8_t *buff)
{
    uint8_t txhdr[2];
    uint8_t rxhdr[8];
    uint32_t repcnt = 0;
    HAL_StatusTypeDef stat;
    
    CRC_ResetDR();
    repcnt = 5U;
    txhdr[0] = STX;     // start of text to wakeup all device receivers 
    txhdr[1] = 1;    // this is address mark 9 bit set high
    buff[0] = STX;      // set for crc calculation
    buff[2] = owifa;   // insert sender interface address 
    buff[buff[3]+4] = (uint8_t) CRC_Calculate8(buff, buff[3]+4); // recalculate packet crc8 
    
    while(repcnt)
    {
        OW_SetUsart(BR_9600, WL_9BIT);
        stat  = HAL_UART_Transmit (&huart2, txhdr, 1, PAK_TOUT(1, BR_9600)); 
        OW_SetUsart(BR_9600, WL_8BIT);
        stat += HAL_UART_Transmit (&huart2, &buff[1], (buff[3]+4), PAK_TOUT((buff[3]+4), BR_9600));
        if ((buff[1] == DEF_OWBRA) 
        ||  (buff[1] == DEF_HC_OWGRA) 
        ||  (buff[1] == DEF_CR_OWGRA) 
        ||  (buff[1] == DEF_RC_OWGRA) 
        ||  (buff[1] == DEF_RT_OWGRA)) return (ESC); // brake here for request without response
        __HAL_UART_FLUSH_DRREGISTER (&huart2);
        ZEROFILL(rxhdr, COUNTOF(rxhdr));
        stat += HAL_UART_Receive (&huart2, rxhdr, 7, OW_PKTIME);
        if ((stat     == HAL_OK)
        &&  (rxhdr[0] == STX) 
        &&  (rxhdr[1] == buff[2]) 
        &&  (rxhdr[2] == buff[1])
        &&  (rxhdr[3] > 1))
        {
            CRC_ResetDR();
            if (rxhdr[3] == 2) 
            {   /* simple response to acknowledge received packet */
                if (rxhdr[6] == (uint8_t) CRC_Calculate8(rxhdr, 6))
                {   // copy cmd echo or response data byte to first buffer byte,
                    buff[5] = 1; // response packet data payload size
                    buff[0] = rxhdr[5]; // it will be included in response from bridge
                    return   (rxhdr[4]); // and send request status response from addressed device
                }   // if response packet check fail, try again till max. trials
                else stat = HAL_ERROR; 
            }
            else
            {   /* receive extende data direct to call buffer */
                buff[0] = rxhdr[5]; // copy response byte
                buff[5] = rxhdr[3]; // response packet data payload size
                buff[7] = rxhdr[6]; // copy first received payload byte
                stat = HAL_UART_Receive (&huart2, &buff[8], buff[5]-2, OW_PKTIME); // get all packet data
                CRC_Calculate8(rxhdr, 7); // calculate first part response crc and try send again if transfer not success
                if (buff[buff[5]+5] == (uint8_t) CRC_Calculate8(&buff[8], buff[5]-3)) 
                {
                    buff[buff[5]+5] = '\0';
                    return (rxhdr[4]); // return response
                }
                else stat = HAL_ERROR;
            }
        }
        --repcnt;   // valid response should allready return to caller 
        HAL_Delay(RX2TX_DEL);
    }
    return (NAK);
}
/**
  * @brief
  * @param
  * @retval
  */
#ifdef  OW_DS18B20 
static uint8_t OW_Reset(void)
{
	OW_SetUsart(BR_9600, WL_8BIT);
	ow_rxbuf[0] = 0xf0U;
	HAL_UART_Transmit(&huart2, ow_rxbuf, 1, OW_TSENS_TOUT);
	HAL_UART_Receive(&huart2, ow_rxbuf, 1, OW_TSENS_TOUT);
	OW_SetUsart(BR_115200, WL_8BIT);
	if((ow_rxbuf[0] != 0xf0U) && (ow_rxbuf[0] != 0x00U) && (ow_rxbuf[0] != 0xffU)) return (1U);
	else return(0U);	
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SendByte(uint8_t data)
{
	uint32_t i;
	
	for(i = 0U; i < 8U; i++, (data = data >> 1U))
	{
		OW_SendBit(data & (1U<<0));
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReadByte(void)
{
	uint8_t rd_byte, i;
	
	for(i = 0U; i < 8U; i++)
	{
		rd_byte = (rd_byte >> 1U) + 0x80U * OW_ReceiveBit();
	}
	
	return rd_byte;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReceiveBit(void)
{
	uint8_t txd, rxd;
	txd = 0xffU;	
	HAL_UART_Transmit(&huart2, &txd, 1U, OW_TSENS_TOUT);	
	HAL_UART_Receive(&huart2, &rxd, 1U, OW_TSENS_TOUT);	
	if(rxd == txd) return(1U);
	else return(0U);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SendBit(uint8_t send_bit)
{
	uint8_t txb, rxb;	
	if(send_bit == 0U)  txb = 0x00U;
	else txb = 0xffU;	
	HAL_UART_Transmit(&huart2, &txb, 1U, OW_TSENS_TOUT);
	HAL_UART_Receive(&huart2, &rxb, 1U, OW_TSENS_TOUT);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Send(uint8_t *command, uint8_t lenght)
{
	uint32_t i;
	
	uint32_t one_wire_lenght = lenght * 8U;
	
	for (i = 0U;  i < lenght; i++) 
	{
		OW_Pack(command[i], &(ow_rxbuf[i * 8U]));
	}
	
	HAL_UART_Transmit(&huart2, ow_rxbuf, one_wire_lenght, OW_TSENS_TOUT);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Receive(uint8_t *data, uint8_t lenght)
{
	uint32_t i;
	uint32_t ow_lenght = lenght * 8U;
	uint8_t tx_byte = 0xffU;
	
	for(i = 0U; i < ow_lenght; i++)
	{
		HAL_UART_Transmit(&huart2, &tx_byte, 1U, OW_TSENS_TOUT);
		HAL_UART_Receive(&huart2, &ow_rxbuf[i], 1U, OW_TSENS_TOUT);
	}
	
	for(i = 0U; i < lenght; i++)
	{
		data[i] = OW_Unpack(&(ow_rxbuf[i * 8U]));
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReadROM(uint8_t *ow_address)
{
	uint8_t crc;
	
	if(OW_Reset() != 0U)
	{
		OW_SendByte(OW_TSENS_RDROM);
		OW_Receive(ow_address, 8U);
		crc = OW_CrcCheck(ow_address, 7U);
		if((crc != ow_address[7U]) || (crc == 0U))return (1U);
		else return(0U);
	}
	else return (2U);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_CrcCheck(uint8_t *ow_address, uint8_t lenght)
{
	uint8_t crc = 0U;
	uint8_t i, j;

	for (i = 0U; i < lenght; i++) 
	{
		uint8_t inbyte = ow_address[i];
		
		for (j = 0U; j < 8U; j++) 
		{
			uint8_t mix = (crc ^ inbyte) & 0x01U;
			crc >>= 1U;
			if (mix) 
			crc ^= 0x8CU;
			inbyte >>= 1U;
		}
	}
	
	return crc;
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Pack(uint8_t command, uint8_t buffer[8])
{
	uint32_t i;
	
	for (i = 0U;  i < 8U; i++)
	{
		buffer[i] = (command & (1U<<i)) ? 0xffU : 0x00U;
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_Unpack (uint8_t buffer[8])
{
	uint32_t i;
	uint8_t res = 0U;

	for (i = 0U; i < 8U; i++) 
	{
		if (buffer[i] == 0xffU)
		{
			res |=  (1U<<i);
		}
	}

	return res;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint16_t OW_ScratchpadToTemperature(uint16_t scratchpad) 
{
    uint16_t result;
	
	if(scratchpad & (1U<<15))
	{
		scratchpad = ~scratchpad + 1U;
		result = scratchpad >> 4U; 							// cijelobrojni dio temperature
		result *= 10U; 										// 22 -> 220
		result += (((scratchpad & 0x000fU) *625U) / 1000U);
		result |= 0x8000U; 									// add minus sign
	}
	else
	{
		result = scratchpad >> 4U; 							// cijelobrojni dio temperature
		result *= 10U; 										// 22 -> 220
		result += (((scratchpad & 0x000fU) *625U) / 1000U);	// add decimal part
	}
    
    return result;
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_ResetSearch(void) 
{
	ow_last_discrepancy = 0U;
	ow_last_family_discrepancy = 0U;
	ow_last_device_flag = 0U;
	ow_sensor_number = 0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt) 
{
	static uint8_t init_cnt = 0U;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit, id_bit_number;
	uint8_t rom_byte_mask, search_direction;

	id_bit_number = 1U;
	last_zero = 0U;
	rom_byte_number = 0U;
	rom_byte_mask = 1U;

	if (ow_last_device_flag == 0U)
	{
		if (OW_Reset() == 0U)
		{
			ow_last_discrepancy = 0U;
			ow_last_device_flag = 0U;
			ow_last_family_discrepancy = 0U;
			return (0U);
		}

		OW_SendByte(OW_TSENS_SRCHROM); 

		do{
			id_bit = OW_ReceiveBit();
			cmp_id_bit = OW_ReceiveBit();
			
			if ((id_bit == 1U) && (cmp_id_bit == 1U)) break;
			else
			{
				if (id_bit != cmp_id_bit) search_direction = id_bit;  // bit write value for search
				else
				{
					if (id_bit_number < ow_last_discrepancy)
					{
						search_direction = ((ds18b20->rom_code[rom_byte_number] & rom_byte_mask) > 0U);
					}
					else search_direction = (id_bit_number == ow_last_discrepancy);
					
					if (search_direction == 0U)
					{
						last_zero = id_bit_number;
						if (last_zero < 9U)  ow_last_family_discrepancy = last_zero;
					}
				}
				
				if (search_direction == 1) ds18b20->rom_code[rom_byte_number] |= rom_byte_mask;
				else ds18b20->rom_code[rom_byte_number] &= ~rom_byte_mask;
				
				OW_SendBit(search_direction);
				id_bit_number++;
				rom_byte_mask <<= 1U;
				
				if (rom_byte_mask == 0U)
				{
					rom_byte_number++;
					rom_byte_mask = 1U;
				}
			}
		} while(rom_byte_number < 8U);
		
		if (!(id_bit_number < 65U))
		{
			search_result = 1U;
			ow_last_discrepancy = last_zero;
			if (ow_last_discrepancy == 0U) ow_last_device_flag = 1U;
		}
	}
	
	if ((search_result == 0U) || (ds18b20->rom_code[0] == 0U))
	{
		ow_last_discrepancy = 0U;
		ow_last_device_flag = 0U;
		ow_last_family_discrepancy = 0U;
		return (0U);
	} 
	else 
	{
		init_cnt++;
		*sensor_cnt = init_cnt;
		ds18b20->sensor_id = init_cnt;
		return (init_cnt);
	}	
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Select(uint8_t* addr) 
{
	uint8_t i;
	
	OW_SendByte(OW_TSENS_MCHROM);
	
	for (i = 0U; i < 8U; i++) 
	{
		OW_SendByte(*(addr + i));
	}
}
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
