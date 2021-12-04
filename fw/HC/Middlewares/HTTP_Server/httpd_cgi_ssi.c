/**
 ******************************************************************************
 * @file    httpd_cg_ssi.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    31-October-2011
 * @brief   Webserver SSI and CGI handlers
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */
 
#if (__COMMON_H__ != FW_TIME)
    #error "common header version mismatch"
#endif
/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "rtc.h"
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "i2cee.h"
#include "httpd.h"
#include "fs5206.h"
#include "buzzer.h"
#include "eth_bsp.h"
#include "netconf.h"
#include "netbios.h"
#include "display.h"
#include "wiegand.h"
#include "spi_flash.h"
#include "tftpserver.h"
#include "hotel_ctrl.h"
#include "stm32f4x7_eth.h"
#include "stm32f429i_lcd.h"
#include "sdio_sd.h"
/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/
/* Private Define  -----------------------------------------------------------*/
#define TempRegOn(x)                    (x |=  (0x1U << 0))     // config On: controll loop is executed periodicaly
#define TempRegOff(x)                   (x &=(~(0x1U << 0)))    // config Off:controll loop stopped,
#define IsTempRegActiv(x)               (x &   (0x1U << 0))
#define TempRegHeating(x)               (x |=  (0x1U << 1))     // config Heating: output activ for setpoint value 
#define TempRegCooling(x)               (x &=(~(0x1U << 1)))    // config Cooling: opposite from heating
#define IsTempRegHeating(x)             (x &   (0x1U << 1))
#define TempRegEnable(x)                (x |=  (0x1U << 2))     // controll flag Enable: controll loop set output state
#define TempRegDisable(x)               (x &=(~(0x1U << 2)))    // controll flag Disable:output is forced to inactiv state, controll loop cannot change outpu
#define IsTempRegEnabled(x)             (x &   (0x1U << 2))
#define TempRegOutOn(x)                 (x |=  (0x1U << 3))     // status On: output demand for actuator to inject energy in to system
#define TempRegOutOff(x)                (x &=(~(0x1U << 3)))    // status Off:stop demanding energy for controlled system, setpoint is reached
#define IsTempRegOutActiv(x)            (x &   (0x1U << 3))
#define TempRegNewStaSet(x)             (x |=  (0x1U << 4))
#define TempRegNewModSet(x)             (x |=  (0x1U << 5))
#define TempRegNewCtrSet(x)             (x |=  (0x1U << 6))
#define TempRegNewOutSet(x)             (x |=  (0x1U << 7))
/* Private Variables  --------------------------------------------------------*/
tCGI CGI_TAB[12]        = { 0 };    // Cgi call table, only one CGI used
const char  *TAGCHAR    = {"t"};    // use character "t" as tag for CGI */
const char **TAGS       = &TAGCHAR;
const char  *weblog     = {"/log.html"};
const char  *webctrl    = {"/sysctrl.html"};
/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
const char  *HTTP_CGI_Handler(int iIndex, int iNumParams, char **pcParam, char **pcValue); // CGI handler for incoming http request */
const tCGI   HTTP_CGI   = {"/sysctrl.cgi", HTTP_CGI_Handler};   // Html request for "/sysctrl.cgi" will start HTTP_CGI_Handler
/* Program Code  -------------------------------------------------------------*/
/**
 * @brief  uljucuje LED
 * @param  1 - LED1
 * @param  2 - LED2
 * @param  3 - LED1 &L ED2
 * @retval None
 */
void led_set(uint8_t led)
{
    if      (led == 0U) GPIO_ResetBits(LED_GPIO_PORT,LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_ResetBits(LED_GPIO_PORT,LED1_GPIO_PIN);
    else if (led == 2U) GPIO_ResetBits(LED_GPIO_PORT,LED2_GPIO_PIN);
}
/**
 * @brief  iskljucuje LED
 * @param  0 - LED1 &L ED2
 * @param  1 - LED1
 * @param  2 - LED2
 * @retval None
 */
void led_clr (uint8_t led)
{
    if      (led == 0U) GPIO_SetBits(LED_GPIO_PORT,LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_SetBits(LED_GPIO_PORT,LED1_GPIO_PIN);
    else if (led == 2U) GPIO_SetBits(LED_GPIO_PORT,LED2_GPIO_PIN);  
}
/**
 * @brief  change LED state to oposite 1-0-1-0...
 * @param  0 - LED1 &L ED2
 * @param  1 - LED1
 * @param  2 - LED2
 * @retval None
 */
void led_tgl (uint8_t led)
{
    if      (led == 0U) GPIO_ToggleBits(LED_GPIO_PORT, LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_ToggleBits(LED_GPIO_PORT, LED1_GPIO_PIN);
    else if (led == 2U) GPIO_ToggleBits(LED_GPIO_PORT, LED2_GPIO_PIN);
}
/**
 * @brief  insert data to HTTP response
 * @param  iIndex       = 0
 * @param  pcInsert     = pointer to response data buffer
 * @param  iInsertLen   = max. length of response data in bytes 
 * @retval length in bytes of response data set by handler 
 */
u16_t HTTP_ResponseHandler(int iIndex, char *pcInsert, int iInsertLen)
{
    if (iIndex == 0)
    {
		if      (HTTP_LogTransfer.state == HTTP_LOG_READY)
		{
            request = 0U;
            if (!strlen(hc_buff)) strcpy(pcInsert, "EMPTY");
            else strcpy(pcInsert, hc_buff);
            HTTP_LogTransfer.state = HTTP_LOG_TRANSFER_IDLE;
		}
		else if (HTTP_LogTransfer.state == HTTP_LOG_DELETED)
		{
            request = 0U;
            strcpy (pcInsert, "DELETED");
			HTTP_LogTransfer.state = HTTP_LOG_TRANSFER_IDLE;
		}
		else if (http_cmdsta == HTTP_ROOM_STAT_READY)
		{
            request = 0U;
            memcpy(pcInsert, &rx_buff[7], rx_buff[5]-2U);
		}
        else if ((http_cmdsta == HTTP_STAT_RESP_TOUT)   
        ||       (http_cmdsta == HTTP_STAT_RESP_ERROR)     
        ||       (http_cmdsta == HTTP_STAT_RESP_BUSY)      
        ||       (http_cmdsta == HTTP_STAT_RESP_OK))
        {
            if      (http_cmdsta == HTTP_STAT_RESP_TOUT)    strcpy (pcInsert, "TIMEOUT");
            else if (http_cmdsta == HTTP_STAT_RESP_BUSY)    strcpy (pcInsert, "BUSY");
            else if (http_cmdsta == HTTP_STAT_RESP_ERROR)   strcpy (pcInsert, "ERROR");
            else if (http_cmdsta == HTTP_STAT_RESP_OK)      strcpy (pcInsert, "OK");
        }
    }
    led_clr(0);
    return iInsertLen;
}
/**
 * @brief  CGI handler for HTTP request 
 */
const char *HTTP_CGI_Handler(int iIndex, int iNumParams, char **pcParam, char **pcValue)
{
    RTC_TimeTypeDef rtc_time;
    RTC_DateTypeDef rtc_date;
    uint32_t sizea = 0, sizeb = 0;  
    uint32_t unix = 0, cnt = 0;
    uint32_t addr_cnt = 0;   
	uint32_t first_file = 0, last_file  = 0;    
	uint32_t first_addr = 0, last_addr  = 0;
    uint8_t tbuf[8] = { 0 }, sta = 0, req = 0;
    char *pos = NULL;    
    if (iIndex == 0)
    {
        /* HC RC RT reset controller:           rst */
        if (!strcmp(pcParam[0], "rst"))
        { 
            if (*pcValue[0] == '0')         NVIC_SystemReset();       // if parameter value '0' restart hotel controller
            if (rsifa == atoi(pcValue[0]))  NVIC_SystemReset();   // restart if rs485 addresse is hotel controller 
            sizea = strlen(pcValue[0]);                                 // also if parameter value is rs485 hotel controller 
            memcpy(hc_buff, pcValue[0], sizea);   
            request = RESTART_CTRL;                         // or send command to bus device
            http_cmdsta = HTTP_STAT_RESP_OK;        // so far so gud, send response to http caller
            led_set(0);
            if (!strcmp(pcParam[1], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
            {
                ow_txaddr = atoi(pcValue[1]), COM_Link = P2P; // send to specified onewire address 
                COM_Bridge = BR2OW; // set bridge mode for command encapsulation
            }
            return weblog;
        }
        /* Check if file update activ and return busy response */
        if (HC_FwrUpdPck.state || HC_FilUpdPck.state)
        {
            http_cmdsta = HTTP_STAT_RESP_BUSY;
            led_set(2);
            return weblog;
        }
        fwrupd_add_cnt = 0;
        HC_FilUpdPck.cmd = 0;
        HC_FwrUpdPck.cmd = 0;
        ZEROFILL(hc_buff, HC_BSIZE);
        ZEROFILL(filupd_list, FILUPD_LIST_BSIZE);
        memset(fwrupd_add_list, 0, fwrupd_addlist_dsize*2);
        led_set(1);
        /* update weather forecast:             wfc */
        if      (!strcmp(pcParam[0], "wfc")) 
        {
            int8_t sign;
            uint32_t i = 0x4U;
            uint32_t x = 0x0U;
            uint32_t t = strlen(&pcValue[0][0]);
            
            while(x < t)
            {
                if (pcValue[0][x] == '-')
                {
                    ++x;
                    sign = -1;
                } else sign = 1;
                
                eebuff[i] = (int8_t) atoi(&pcValue[0][x]) * sign;
                pos = strchr(&pcValue[0][x], ',');
                
                if (pos != NULL)
                {
                    x = (pos - &pcValue[0][0] + 0x1U);
                    ++i;     
                }
                else x = t;
            }
            unix = rtc2unix(&rtc_time, &rtc_date);
            eebuff[0] = ((unix >> 24) & 0xFFU);
            eebuff[1] = ((unix >> 16) & 0xFFU);
            eebuff[2] = ((unix >>  8) & 0xFFU);
            eebuff[3] =  (unix        & 0xFFU);
            if (I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_FORECAST_ADD, eebuff, WFC_DSIZE) != 0x0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
            DelayMs(I2CEE_WRITE_DELAY);
            http_cmdsta = HTTP_STAT_RESP_OK;
            return weblog;
        }
        /* update hotel status:                 jrn */
        else if (!strcmp(pcParam[0], "jrn"))
        {
            /** insert broadcast address as journal recipient*/
            Int2Str(hc_buff, rsbra, 0U);
            sizea = strlen(hc_buff);
            sizeb = strlen(pcValue[0]);
            memcpy(&hc_buff[sizea + 1U], pcValue[0], sizeb);
            http_cmdsta = HTTP_STAT_RESP_OK;
            request = DWNLD_JRNL;
            return weblog;
        }
        /* update rtc date & time:              tdu */
        else if (!strcmp(pcParam[0], "tdu"))
        {
            if (strlen(pcValue[0]) == 15)
            {
                rtc_date.RTC_WeekDay = TODEC(*pcValue[0]);
                if(!rtc_date.RTC_WeekDay)rtc_date.RTC_WeekDay = 7;
                Str2Hex (&pcValue[0][1], &rtc_date.RTC_Date,    2);
                Str2Hex (&pcValue[0][3], &rtc_date.RTC_Month,   2);
                Str2Hex (&pcValue[0][7], &rtc_date.RTC_Year,    2);
                Str2Hex (&pcValue[0][9], &rtc_time.RTC_Hours,   2);
                Str2Hex (&pcValue[0][11],&rtc_time.RTC_Minutes, 2);
                Str2Hex (&pcValue[0][13],&rtc_time.RTC_Seconds, 2);
                PWR_BackupAccessCmd(ENABLE);
                RTC_SetTime(RTC_Format_BCD, &rtc_time);
                RTC_SetDate(RTC_Format_BCD, &rtc_date);
                PWR_BackupAccessCmd(DISABLE);
                RTC_State = RTC_VALID;
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* send available firmware info:        inf  */
        else if (!strcmp(pcParam[0], "inf"))
        {
            while(HC_State != PCK_ENUM) HC_Service(); // complete previous request
            http_cmdsta = HTTP_STAT_RESP_OK;
            SendFwInfo();
            return weblog;
        }
        /** ========================================================================*/
        /**                 H O T E L               C O N T R O L L E R             */
        /** ========================================================================*/
        /* HC Fat FS make new directory:        mkd */
        else if (!strcmp(pcParam[0], "mkd"))
        {
            if      (f_mount(&fatfs, "0:", 0x0U) != FR_OK)  http_cmdsta = HTTP_STAT_RESP_ERROR;
            else if (f_mkdir(pcValue[0]) != FR_OK)          http_cmdsta = HTTP_STAT_RESP_ERROR;
            else                                            http_cmdsta = HTTP_STAT_RESP_OK;
            return weblog;
        }
        /* HC Fat FS delete file/directory:     rmd */
        else if (!strcmp(pcParam[0], "rmd"))
        {
            if      (f_mount(&fatfs, "0:", 0x0U) != FR_OK)  http_cmdsta = HTTP_STAT_RESP_ERROR;
            else if (f_rmdir(pcValue[0]) != FR_OK)          http_cmdsta = HTTP_STAT_RESP_ERROR;
            else                                            http_cmdsta = HTTP_STAT_RESP_OK;
            return weblog;
        }
        /* HC onboard led control:              led */
        else if (!strcmp(pcParam[0], "led")) 
        {
            uint8_t led = TODEC(**pcValue);
            if (led < 3) // led 0,1,2
            {
                led_clr(0);
                led_set(led);
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* HC load address list:                cad */
        else if (!strcmp(pcParam[0], "cad"))
        {       
            if(HC_LoadAddrList() != FS_FILE_OK)
            {
                DISP_uSDCardSetNewState(0);
                http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else
            {
                HC_CreateAddrList();
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            return weblog;
        }
        /* HC set IP addresse:                  ipa, snm, gwa */
        else if (!strcmp(pcParam[0], "ipa")) 
        {  
            char *pchar = pcValue[0];
            ip_add[0] = Str2Int(pchar, 0x3U);			
            pchar = 0x1U + strchr(pchar, '.');
            ip_add[1] = Str2Int(pchar, 0x3U);
            pchar = 0x1U + strchr(pchar, '.');
            ip_add[2] = Str2Int(pchar, 0x3U);
            pchar = 0x1U + strchr(pchar, '.');
            ip_add[3] = Str2Int(pchar, 0x3U);
            
            if (!strcmp (pcParam[1] , "snm")) 
            {  
                char *pchar = pcValue[1];
                subnet[0] = Str2Int(pchar, 0x3U);			
                pchar = 0x1U + strchr(pchar, '.');
                subnet[1] = Str2Int(pchar, 0x3U);
                pchar = 0x1U + strchr(pchar, '.');
                subnet[2] = Str2Int(pchar, 0x3U);
                pchar = 0x1U + strchr(pchar, '.');
                subnet[3] = Str2Int(pchar, 0x3U);
                
                if (!strcmp (pcParam[2] , "gwa")) 
                {  
                    char *pchar = pcValue[2];
                    gw_add[0] = Str2Int(pchar, 0x3U);			
                    pchar = 0x1U + strchr(pchar, '.');
                    gw_add[1] = Str2Int(pchar, 0x3U);
                    pchar = 0x1U + strchr(pchar, '.');
                    gw_add[2] = Str2Int(pchar, 0x3U);
                    pchar = 0x1U + strchr(pchar, '.');
                    gw_add[3] = Str2Int(pchar, 0x3U);

                    IP4_ADDR(&new_ip, ip_add[0], ip_add[1], ip_add[2], ip_add[3]);
                    IP4_ADDR(&new_nm, subnet[0], subnet[1], subnet[2], subnet[3]);
                    IP4_ADDR(&new_gw, gw_add[0], gw_add[1], gw_add[2], gw_add[3]);

                    if(I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_ETH_IP_ADD, ip_add, 4U) != 0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
                    DelayMs(I2CEE_WRITE_DELAY);
                    if(I2CEE_WriteBytes16(I2CEE_PAGE_0,EE_ETH_SUB_ADD, subnet, 4U) != 0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
                    DelayMs(I2CEE_WRITE_DELAY);
                    if(I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_ETH_GW_ADD, gw_add, 4U) != 0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
                    DelayMs(I2CEE_WRITE_DELAY);

#ifndef USE_CONSTANT_IP
                    netif_set_addr(&netif, &new_ip , &new_nm, &new_gw);
#endif 
                    http_cmdsta = HTTP_STAT_RESP_OK;
                    return weblog;
                }
            }
            http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* HC log list request:                 log; 3, 4, 5 */   
        else if (!strcmp(pcParam[0], "log"))
        {
            if      (!strcmp(pcValue[0], "3"))
            {
                HTTP_LogTransfer.state = HTTP_GET_LOG_LIST;
                request = GET_LOG_LIST;
                HC_ReadLogListBlock();
            }
            else if (!strcmp(pcValue[0], "4"))
            {
                if(HC_LogMemory.log_cnt == 0) HTTP_LogTransfer.state = HTTP_LOG_READY;
                else
                {
                    HTTP_LogTransfer.state = HTTP_DEL_LOG_LIST;
                    request = DEL_LOG_LIST;
                    HC_DeleteLogListBlock();                    
                }
            }
            else if (!strcmp(pcValue[0], "5"))
            {
                if (HC_LogMemory.log_cnt == 0) HTTP_LogTransfer.state = HTTP_LOG_READY;
                else
                {
                    HTTP_LogTransfer.state = HTTP_FORMAT_LOG_LIST;
                    request = DEL_LOG_LIST;
                    HC_FormatLogList();                    
                }
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* HC update firmware:  fwu  */
        else if (!strcmp(pcParam[0], "fwu") || !strcmp(pcParam[0], "HCfwu"))
        {   
            if      (!strcmp(pcValue[0], "hc") || !strcmp(pcParam[0], "HCfwu"))
            {
                BLDR_Clear();
                BLDR_Enable();
                tbuf[0] = (sys_cfg >> 24) & 0xFFU;
                tbuf[1] = (sys_cfg >> 16) & 0xFFU;
                tbuf[2] = (sys_cfg >>  8) & 0xFFU;
                tbuf[3] =  sys_cfg        & 0xFFU;
                if (I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_SYS_CFG_ADD, tbuf, 0x4U) != 0x0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
                DelayMs(I2CEE_WRITE_DELAY);
                NVIC_SystemReset();
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* HC set system ID:                    sid, nid */
        else if (!strcmp(pcParam[0], "sid"))
        {     
            if (!strcmp(pcParam[1] , "nid"))
            {   
                if(rsifa == atoi(pcValue[0]))    // if hotel controller inteface address set for recepient, broadcast to all in system 
                {                   
                    system_id = atoi(pcValue[1]);   // set new system id address
                    tbuf[0] = ((system_id >> 8) & 0xFFU);
                    tbuf[1] = (system_id & 0xFFU);
                    if(I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_SYS_ID_ADD, tbuf,2U) != 0U) ErrorHandler(HTTPD_FUNC, I2C_DRV);
                    DelayMs(I2CEE_WRITE_DELAY);
                    ZEROFILL(hc_buff, HC_BSIZE);
                    Int2Str(hc_buff, rsbra, 0U);
                    sizea = strlen(hc_buff);
                    Int2Str(&hc_buff[sizea + 1U], system_id, 0U);
                    request = SET_SYSTEM_ID;	
                    http_cmdsta = HTTP_STAT_RESP_OK;
                }	
                else  http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* HC send data to wiegand interface:   wgi, wgd    */
        else if (!strcmp(pcParam[0], "wgi"))
        {
            http_cmdsta = HTTP_STAT_RESP_ERROR;
            if  (!strcmp(pcParam[1], "wgd")) 
            {
                if (WIEGAND_Send(pcValue[0][0], &pcValue[1][0]) == 0) http_cmdsta = HTTP_STAT_RESP_OK;
            }
            return weblog;
        }
        /** ========================================================================*/
        /**                 R O O M             C O N T R O L L E R                 */
        /** ========================================================================*/
        /*  set RC room status:                 stg, val */
        else if (!strcmp(pcParam[0], "stg")) 
        {
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);            
            if (!strcmp(pcParam[1], "val")) 
            {       
                sizea = strlen(hc_buff) + 1U;
                sizeb = strlen(pcValue[1]);
                memcpy(&hc_buff[sizea], pcValue[1], sizeb);
                while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer                            
                rs485_txaddr = atoi(pcValue[0]);
                HC_CreateCmdRequest(SET_APPL_STAT, hc_buff);
                sta = HTTP2RS485();
                if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
                else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
                else                 http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* RC set room status:                  set, sbr/psw/usr,    */
        else if (!strcmp(pcParam[0], "set"))
        {
            req = 0U;
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);            
            if      (!strcmp(pcParam[1], "psw")) req = SET_PASSWORD; // byte0 (U=user, M=maid, S=service, 4=reset), byte1-3,4,5(pin 3-5 number)             
            else if (!strcmp(pcParam[1], "usr")) req = SET_PERMITED_GROUP;
            else    http_cmdsta = HTTP_STAT_RESP_ERROR;            
            if(req)
            {
                sizeb = strlen(pcValue[1]);
                memcpy(&hc_buff[sizea+1U], pcValue[1], sizeb); // copy new status/bedding/password
                while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer                            
                rs485_txaddr = atoi(pcValue[0]);
                HC_CreateCmdRequest(req, hc_buff);
                sta = HTTP2RS485();
                if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
                else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
                else                 http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            return weblog;
        }
        /* RC room status request:              cst */
        else if (!strcmp(pcParam[0], "cst"))
        {
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);
            if (!strcmp(pcParam[1], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
            {
                if ((atoi(pcValue[1]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                ||  (atoi(pcValue[1]) == DEF_RT_OWGRA))
                {
                    request = 0;
                    COM_Bridge = BRNONE;
                    COM_Link = NOLINK;
                    filupd_list[0] = 0;
                    HC_FwrUpdPck.cmd = 0;
                    http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else
                {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                    ow_txaddr = atoi(pcValue[1]), COM_Link = P2P; // send to specified onewire address 
                    COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                }
            }
            while(HC_State != PCK_ENUM) HC_Service(); // complete previous request
            http_cmdsta = HTTP_GET_ROOM_STAT;
            request = GET_APPL_STAT;
            sizeb = Get_SysTick();
            while (http_cmdsta != HTTP_ROOM_STAT_READY)
            {
                if ((Get_SysTick() - sizeb) >= RESP_TOUT*60)
                {
                    request = 0;
                    COM_Link = NOLINK;
                    COM_Bridge = BRNONE;
                    http_cmdsta = HTTP_STAT_RESP_TOUT;
                    return weblog;
                }
                else
                {
                    if (!http_cmdsta)
                    {
                        HC_State = PCK_ENUM;
                        request = GET_APPL_STAT;
                        http_cmdsta = HTTP_GET_ROOM_STAT;
                    }
                    HC_Service();
                }
            }
            request = 0;
            COM_Link = NOLINK;
            COM_Bridge = BRNONE;
            return weblog;
        }
        /* RC preview display image:            ipr */
        else if (!strcmp(pcParam[0], "ipr"))
        {
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);
            request = PREVIEW_DISPL_IMG;
        }
        /* RC set digital output:               cdo */
        else if (!strcmp(pcParam[0], "cdo"))
        { 
            http_cmdsta = HTTP_STAT_RESP_ERROR;
            //
            //  check all received paremeter string
            //
            sizeb  = strcmp(pcParam[1], "do0");
            sizeb += strcmp(pcParam[2], "do1");
            sizeb += strcmp(pcParam[3], "do2");
            sizeb += strcmp(pcParam[4], "do3");
            sizeb += strcmp(pcParam[5], "do4");
            sizeb += strcmp(pcParam[6], "do5");
            sizeb += strcmp(pcParam[7], "do6");
            sizeb += strcmp(pcParam[8], "do7");
            sizeb += strcmp(pcParam[9], "ctrl");
       
            if(!sizeb) // all parameter specified
            {
                for(cnt = 1; cnt < 10; cnt++)  // loop to copy all parameter values null separated
                {
                    if (!IS09(pcValue[cnt]) || (*pcValue[cnt] > '5')) return weblog;
                    hc_buff[cnt-1] = *pcValue[cnt];                
                }
                while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer
                rs485_txaddr = atoi(pcValue[0]);
                HC_CreateCmdRequest(SET_DOUT_STATE, hc_buff);
                sta = HTTP2RS485();
                if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
                else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
            }
            return weblog;
        }
        /* RC set digital inputs:               cdi */
        else if (!strcmp(pcParam[0], "cdi"))
        { 
            http_cmdsta = HTTP_STAT_RESP_ERROR;
            //
            //  check all received paremeter string
            //
            sizeb  = strcmp(pcParam[1], "di0");
            sizeb += strcmp(pcParam[2], "di1");
            sizeb += strcmp(pcParam[3], "di2");
            sizeb += strcmp(pcParam[4], "di3");
            sizeb += strcmp(pcParam[5], "di4");
            sizeb += strcmp(pcParam[6], "di5");
            sizeb += strcmp(pcParam[7], "di6");
            sizeb += strcmp(pcParam[8], "di7");
       
            if(!sizeb) // all parameter specified
            {
                for(cnt = 1; cnt < 9; cnt++)  // loop to copy all parameter values null separated
                {
                    if (!IS09(pcValue[cnt]) || (*pcValue[cnt] > '5')) return weblog;
                    hc_buff[cnt-1] = *pcValue[cnt];                
                }
                while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer
                rs485_txaddr = atoi(pcValue[0]);
                HC_CreateCmdRequest(SET_DIN_CFG, hc_buff);
                sta = HTTP2RS485();
                if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
                else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
            }
            return weblog;
        }
		/* RC update old firmware:              cud */
        else if (!strcmp(pcParam[0], "cud"))
        {
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);
            request = UPDATE_FWR;
            http_cmdsta = HTTP_STAT_RESP_OK;
            return weblog;
        }
		/* RC update firmware:                  fuf, ful */
        else if (!strcmp(pcParam[0], "fuf"))
        {
            if  (!strcmp(pcParam[1], "ful"))
            {
                first_addr = atoi(pcValue[0]);
                last_addr  = atoi(pcValue[1]);
                //
                //  create update address list from room controller address list
                //  to update only system members
                //
                while(last_addr >= first_addr)
                {
                    if(addr_list[addr_cnt] == first_addr)
                    {
                        fwrupd_add_list[fwrupd_add_cnt] = first_addr;
                        ++fwrupd_add_cnt;
                        ++first_addr;
                        addr_cnt = 0U;
                    }
                    else if(addr_list[addr_cnt] == 0U)
                    {
                        addr_cnt = 0U;
                        ++first_addr;
                    }
                    else ++addr_cnt;
                }
                //
                //  if address list not empty start update process
                //
                if(fwrupd_add_cnt != 0U)
                {
                    fwrupd_add_cnt = 0U;
                    filupd_list_cnt = 0U;
                    filupd_list[0] = 20U;
                    request = DWNLD_DISP_IMG;
                    HC_FwrUpdPck.cmd = UPDATE_FWR;  
                    http_cmdsta = HTTP_STAT_RESP_OK;             
                }
                else http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
		/* RC update bootloader:                buf, bul */
        else if (!strcmp(pcParam[0], "buf"))
        {    
            if (!strcmp(pcParam[1] , "bul")) 
            {
                first_addr = atoi(pcValue[0]);
                last_addr  = atoi(pcValue[1]);
                //
                //  create update address list from room controller address list
                //  to update only system members
                //
                while(last_addr >= first_addr)
                {
                    if      (addr_list[addr_cnt] == first_addr)
                    {
                        fwrupd_add_list[fwrupd_add_cnt] = first_addr;
                        ++fwrupd_add_cnt;
                        ++first_addr;
                        addr_cnt = 0U;
                    }
                    else if (addr_list[addr_cnt] == 0U)
                    {
                        addr_cnt = 0U;
                        ++first_addr;
                    }
                    else ++addr_cnt;
                }
                //
                //  if address list not empty start update process
                //
                if(fwrupd_add_cnt != 0U)
                {
                    fwrupd_add_cnt = 0U;
                    filupd_list_cnt = 0U;
                    filupd_list[0] = 21U;
                    request = DWNLD_DISP_IMG;
                    HC_FwrUpdPck.cmd = UPDATE_BLDR;
                    http_cmdsta = HTTP_STAT_RESP_OK;             
                }
                else http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
		/* RC update display image:             iuf, iul, ifa, ila  */
        else if (!strcmp(pcParam[0], "iuf"))
        {  
            if (!strcmp(pcParam[1], "iul")) ++sizeb;
            if (!strcmp(pcParam[2], "ifa")) ++sizeb;
            if (!strcmp(pcParam[3], "ila")) ++sizeb;
            if (sizeb == 0x3U) // all parameter specified
            {
                addr_cnt = 0x0U;
                filupd_list_cnt = 0x0U;
                imgupd_addlist_cnt = 0x0U;
                ZEROFILL(imgupd_add_list, imgupd_addlist_dsize);
                first_file = atoi(pcValue[0]);
                last_file  = atoi(pcValue[1]);
                first_addr = atoi(pcValue[2]);
                last_addr  = atoi(pcValue[3]);
                //
                //  create file update list 
                // 
                while(first_file <= last_file)
                {
                    filupd_list[filupd_list_cnt++] = first_file++;
                }
                //
                //  create update address list from room controller address list
                //  to update only system members
                //
                while(last_addr >= first_addr)
                {
                    if      (addr_list[addr_cnt] == first_addr)
                    {
                        imgupd_add_list[imgupd_addlist_cnt] = first_addr;
                        ++imgupd_addlist_cnt;
                        ++first_addr;
                        addr_cnt = 0x0U;
                    }
                    else if (addr_list[addr_cnt] == 0U)
                    {
                        addr_cnt = 0x0U;
                        ++first_addr;
                    }
                    else ++addr_cnt;
                }
                //
                //  if address list not empty start update process
                //
                if ((imgupd_addlist_cnt != 0U) && (filupd_list_cnt != 0U))
                {
                    filupd_list_cnt = 0x0U;
                    imgupd_addlist_cnt = 0x0U;
                    request = DWNLD_DISP_IMG;
                    HC_FilUpdPck.cmd = DWNLD_DISP_IMG;
                    http_cmdsta = HTTP_STAT_RESP_OK;             
                }
                else http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
		/* RC set display brightness:           cbr */
        else if (!strcmp(pcParam[0], "cbr")) 
        {
            if (!strcmp(pcParam[1], "br")) 
            {       
                sizea = strlen(pcValue[0]);
                sizeb = strlen(pcValue[1]);
                memcpy(hc_buff, pcValue[0], sizea);
                memcpy(&hc_buff[sizea+1U], pcValue[1], sizeb);
                request = SET_DISPL_BCKLGHT;
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
		/* RC SOS alarm reset request:          rud */
        else if (!strcmp(pcParam[0], "rud") )
        {
            sizea = strlen(pcValue[0]);
            memcpy(hc_buff, pcValue[0], sizea);
            request = RESET_SOS_ALARM;
            http_cmdsta = HTTP_STAT_RESP_OK;
            return weblog;
        }
		/* RC set bedding period:               sbr, per */
        else if (!strcmp(pcParam[0], "sbr"))
        {       
            if  (!strcmp(pcParam[1] , "per")) 
            {
                sizea = strlen(pcValue[0]);
                sizeb = strlen(pcValue[1]);
                memcpy(hc_buff, pcValue[0], sizea);
                memcpy(&hc_buff[sizea + 1U], pcValue[1], sizeb);
                request = SET_BEDDING_REPL;
                http_cmdsta = HTTP_STAT_RESP_OK;
            }   
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }        
		/* RC set rs485 address:                rsc, rsa, rga, rba, rib */
        else if (!strcmp(pcParam[0], "rsc"))
        {
            sizea = strlen(pcValue[0]); // current device address
            memcpy(hc_buff, pcValue[0], sizea); // copy room controller address
            http_cmdsta = HTTP_STAT_RESP_ERROR;
            //
            //  check all received paremeter string
            //
            if (!strcmp(pcParam[1], "rsa")) ++sizeb;// rs485 device new rs485 interface address
            if (!strcmp(pcParam[2], "rga")) ++sizeb;// rs485 device new rs485 group address
            if (!strcmp(pcParam[3], "rba")) ++sizeb;// rs485 device new rs485 broadcast address
            if (!strcmp(pcParam[4], "rib")) ++sizeb;// rs485 device new rs485 interface baudrate

            if (sizeb == 0x4U) // all parameter specified
            {
                for (cnt = 0x1U; cnt < 0x5U; cnt++)  // loop to copy all parameter values null separated
                {
                    sizeb = strlen(pcValue[cnt]);
                    memcpy(&hc_buff[sizea + 0x1U], pcValue[cnt], sizeb);
                    sizea += sizeb + 0x1U;                    
                }
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else return weblog;
            /**
            *
            *	check if new rs485 interface settings are for main controller
            */
            sizeb = atoi(pcValue[0]);   // recepient device address
            
            if(rsifa == sizeb)    // if request addressed to this device interface, set new addresse
            {
                uint32_t tmp_rs485_if_addr = atoi(pcValue[1]);  // get new interface address 
                uint32_t tmp_rs485_gr_addr = atoi(pcValue[2]);  // get new group address 
                uint32_t tmp_rs485_br_addr = atoi(pcValue[3]);  // get new broadcast address 
                //
                //  check all values for error values
                //
                if      (!tmp_rs485_if_addr || (tmp_rs485_if_addr == 0xFFFFU)) return webctrl;
                else if (!tmp_rs485_gr_addr || (tmp_rs485_gr_addr == 0xFFFFU)) return webctrl;
                else if (!tmp_rs485_br_addr || (tmp_rs485_br_addr == 0xFFFFU)) return webctrl;
                else if (!IS09(pcValue[4]))                                    return webctrl;
                // 
                // if check passed, set interface address
                //
                rsifa   = tmp_rs485_if_addr;
                rsgra   = tmp_rs485_gr_addr;
                rsbra   = tmp_rs485_br_addr;
                rsbps = CONVERTDEC(pcValue[4]);
                //
                //  save new addresse to eeprom 
                //
                tbuf[0] = (tmp_rs485_if_addr >> 8) & 0xFFU;
                tbuf[1] = tmp_rs485_if_addr        & 0xFFU;
                tbuf[2] = (tmp_rs485_gr_addr >> 8) & 0xFFU;
                tbuf[3] = tmp_rs485_gr_addr        & 0xFFU;
                tbuf[4] = (tmp_rs485_br_addr >> 8) & 0xFFU;
                tbuf[5] = tmp_rs485_br_addr        & 0xFFU;
                tbuf[6] = *pcValue[4];
                if (I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_RS485_IFADD, tbuf, 7U) != 0U)  ErrorHandler(HTTPD_FUNC, I2C_DRV);
                DelayMs(I2CEE_WRITE_DELAY);
                //
                //  restart comm interface with new parameters
                //
                USART_DeInit(USARTx);
                RS485_Init();
            }
            else    // if not this device addressed, send request to rs485 bus device
            {
                request = SET_RS485_CFG;
            }
            return weblog;
        }
        /** ========================================================================*/
        /**              R O O M                 T H E R M O S T A T                */
        /** ========================================================================*/
        /* set room display thermostat:         tha, spt, dif, sta, mod, ctr, out, owa  */
        else if (!strcmp(pcParam[0], "tha")) 
        {
            while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer 
            tx_buff[7] = 0x0U;  // clear setpoint
            tx_buff[8] = 0x0U;  // clear difference
            tx_buff[9] = 0x0U;  // clear config
            http_cmdsta = HTTP_STAT_RESP_OK;
//            request = SET_ROOM_TEMP;
//            HC_State = PCK_ENUM;
            
            if (!strcmp(pcValue[0], "RCgra")) 
            {  // Room Controller rs485 Group Address Macro
                Int2Str(hc_buff, DEF_RC_RSGRA, 0x0U);
                COM_Link = GROUP;    // send to room controller group address
            }
            else if (IS09(pcValue[0]))
            {
                sizea = strlen (pcValue[0]); // get receiver rs485 address string size 
                memcpy(hc_buff, pcValue[0], sizea); // copy receiver rs485 address string                
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            
            for (cnt = 0x1U; cnt < LWIP_HTTPD_MAX_CGI_PARAMETERS; cnt++)
            {
                if      (!strcmp(pcParam[cnt], "spt"))
                {
                    if (IS09(pcValue[cnt])) tx_buff[7] = atoi(pcValue[cnt]);
                    else http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else if (!strcmp(pcParam[cnt], "dif"))
                {
                    if (ISVALIDDEC(*pcValue[cnt])) tx_buff[8] = TODEC(*pcValue[cnt]); // convert only num
                    else http_cmdsta = HTTP_STAT_RESP_ERROR; // dont send differential value more than 9
                }
                else if (!strcmp(pcParam[cnt], "sta"))
                {
                    if      (!strcmp(pcValue[cnt], "ON"))
                    {
                        TempRegOn(tx_buff[9]);
                        TempRegNewStaSet(tx_buff[9]);
                    }
                    else if (!strcmp(pcValue[cnt], "OFF"))
                    {
                        TempRegOff(tx_buff[9]);
                        TempRegNewStaSet(tx_buff[9]);
                    }
                    else http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else if (!strcmp(pcParam[cnt], "mod"))
                {
                    if      (!strcmp(pcValue[cnt], "COOL"))
                    {
                        TempRegCooling(tx_buff[9]);
                        TempRegNewModSet(tx_buff[9]);
                    }
                    else if (!strcmp(pcValue[cnt], "HEAT"))
                    {
                        TempRegHeating(tx_buff[9]);
                        TempRegNewModSet(tx_buff[9]);
                    }
                    else http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else if (!strcmp(pcParam[cnt], "ctr"))
                {
                    if      (!strcmp(pcValue[cnt], "ENA"))
                    {
                        TempRegEnable(tx_buff[9]);
                        TempRegNewCtrSet(tx_buff[9]);
                    }
                    else if (!strcmp(pcValue[cnt], "DIS"))
                    {
                        TempRegDisable(tx_buff[9]);
                        TempRegNewCtrSet(tx_buff[9]);
                    }
                    else http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else if (!strcmp(pcParam[cnt], "out"))
                {
                    if      (!strcmp(pcValue[cnt], "ON"))
                    {
                        TempRegOutOn(tx_buff[9]);
                        TempRegNewOutSet(tx_buff[9]);
                    }
                    else if (!strcmp(pcValue[cnt], "OFF"))
                    {
                        TempRegOutOff(tx_buff[9]);
                        TempRegNewOutSet(tx_buff[9]);
                    }
                    else http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else if (!strcmp(pcParam[cnt], "owa"))
                {
                    if (atoi(pcValue[cnt]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                    {
                        request = 0x0U;
                        COM_Bridge = BRNONE;
                        COM_Link = NOLINK;
                        http_cmdsta = HTTP_STAT_RESP_ERROR;
                    }
                    else
                    {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                        if      (!strcmp(pcValue[cnt], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                        else ow_txaddr = atoi(pcValue[cnt]), COM_Link = P2P; // send to specified onewire address 
                        COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                    }
                }
                if (http_cmdsta == HTTP_STAT_RESP_ERROR) return weblog;                
            }
            rs485_txaddr = atoi(pcValue[0]);
            HC_CreateCmdRequest(SET_ROOM_TEMP, hc_buff);
            sta = HTTP2RS485();
            if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
            else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
            else                 http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* update room display firmware:        tuf, owa */
        else if (!strcmp(pcParam[0], "tuf")) 
        { 
            if   (!strcmp(pcValue[0], "RCgra")) 
            {  // macro in regard to last selected bus, RS485 bus Room Controller Group Address
                fwrupd_add_list[0] = DEF_RC_RSGRA;
                COM_Link = GROUP;    // send to room thermostats group address
            }
            else if (IS09 (pcValue[0])) 
            {
                fwrupd_add_list[0] = atoi(pcValue[0]);
                COM_Link = P2P;
            }
            
            if (fwrupd_add_list[0])
            {
                fwrupd_add_cnt = 0;
                filupd_list_cnt = 0;
                filupd_list[0] = 22;
                request = DWNLD_DISP_IMG;
                HC_FwrUpdPck.cmd = UPDATE_FWR;
                http_cmdsta = HTTP_STAT_RESP_OK; 
                HC_State = PCK_ENUM;
                //
                // if onewire address specified for room thermostat receiver interface,
                // this command and parameters will be packed for onewire bus transfer and
                // encapsulated inside rs485 packet which then will be send to RS485_2_OneWire
                // bridge device, usually room controller, and finaly to addressed room thermostat 
                //
                if (!strcmp(pcParam[1], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
                {
                    if (atoi(pcValue[1]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                    {
                        request = 0;
                        COM_Link = NOLINK;
                        COM_Bridge = BRNONE;
                        filupd_list[0] = 0;
                        HC_FwrUpdPck.cmd = 0;
                        http_cmdsta = HTTP_STAT_RESP_ERROR;
                    }
                    else
                    {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                        if   (!strcmp(pcValue[1], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                        else ow_txaddr = atoi(pcValue[1]), COM_Link = P2P; // send to specified onewire address 
                        COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                    }
                }
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;            
        }
        /* upload RT display user logo image    tlg, owa   */
        else if (!strcmp(pcParam[0], "tlg")) 
        { 
            if   (!strcmp(pcValue[0], "RCgra")) 
            {  // macro in regard to last selected bus, RS485 bus Room Controller Group Address
                imgupd_add_list[0] = DEF_RC_RSGRA;
                COM_Link = GROUP;    // send to room thermostats group address
            }
            else if (IS09 (pcValue[0])) 
            {
                imgupd_add_list[0] = atoi(pcValue[0]); //  device to update address
                COM_Link = P2P;
            }
            
            if (imgupd_add_list[0])
            {
                filupd_list_cnt = 0U;
                imgupd_addlist_cnt = 0U;
                filupd_list[0] = 24U;
                request = DWNLD_DISP_IMG;
                HC_FilUpdPck.cmd = RT_DWNLD_LOGO;
                http_cmdsta = HTTP_STAT_RESP_OK;
                while (HC_State != PCK_ENUM) HC_Service(); // finish previous command
                //
                // if onewire address specified for room thermostat receiver interface,
                // this command and parameters will be packed for onewire bus transfer and
                // encapsulated inside rs485 packet which then will be send to RS485_2_OneWire
                // bridge device, usually room controller, and finaly to addressed room thermostat 
                //
                if (!strcmp(pcParam[1], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
                {
                    if (atoi(pcValue[1]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                    {
                        request = 0;
                        COM_Link = NOLINK;
                        COM_Bridge = BRNONE;
                        filupd_list[0] = 0;
                        HC_FilUpdPck.cmd = 0;
                        http_cmdsta = HTTP_STAT_RESP_ERROR;
                    }
                    else
                    {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                        if   (!strcmp(pcValue[1], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                        else ow_txaddr = atoi(pcValue[1]), COM_Link = P2P; // send to specified onewire address 
                        COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                    }
                }
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;            
        }        
        /* display text on screen:              txa, trc, tx0, ty0, tx1, ty1, txc, txf, txh, txv, txt, owa  */
        else if (!strcmp(pcParam[0], "txa")) 
        {
            sizea = strlen (pcValue[0]);        // get size of address string
            memcpy(hc_buff, pcValue[0], sizea); // copy room controller address
            //
            //  check all received paremeter string
            //
            if (!strcmp(pcParam[1], "trc")) ++sizeb; // room colour
            if (!strcmp(pcParam[2], "tx0")) ++sizeb; // room upper left corner horisontal position
            if (!strcmp(pcParam[3], "ty0")) ++sizeb; // room upper left corner vertical position
            if (!strcmp(pcParam[4], "tx1")) ++sizeb; // room lower right corner horisontal position
            if (!strcmp(pcParam[5], "ty1")) ++sizeb; // room lower right corner vertical position
            if (!strcmp(pcParam[6], "txc")) ++sizeb; // text colour: 0~25 = BLUE,....TRANSPARENT
            if (!strcmp(pcParam[7], "txf")) ++sizeb; // text font size: 0~11 = 8,10,13,13B,16,16B,20,20B,24,24B,32,32B
            if (!strcmp(pcParam[8], "txh")) ++sizeb; // text horisontal align:0=LEFT,1=HORISONTAL,2=RIGHT,3=CENTER,4=HCENTER
            if (!strcmp(pcParam[9], "txv")) ++sizeb; // text vertical align:0=TOP,1=VERTICAL,2=BOTTOM,3=BASELINE,4=VCENTER
            if (!strcmp(pcParam[10],"txt")) ++sizeb; // text to display
            
            if (sizeb == 10U) // all parameter specified
            {
                for(cnt = 1U; cnt < 11U; cnt++)  // copy to all parameter values null separated
                {
                    sizeb = strlen(pcValue[cnt]); // get parameter string size
                    memcpy (&hc_buff[sizea + 1U], pcValue[cnt], sizeb); // copy all parameter values to single buffer
                    sizea += sizeb + 1U; // set next parameter position index inside buffer
                }
                
                do
                {
                    pos = strstr(&hc_buff[sizea - sizeb], "%20");
                    
                    if (pos != NULL) 
                    {
                        *pos++ = ' ';
                        *pos++ = ' ';
                        *pos++ = ' ';
                    }
                }
                while(pos != NULL);
                
                request = RT_DISP_MSG;
                http_cmdsta = HTTP_STAT_RESP_OK;
                
                if (!strcmp(pcParam[11],"owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
                {
                    if (atoi(pcValue[11]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                    {
                        request = 0x0U;
                        COM_Bridge = BRNONE;
                        COM_Link = NOLINK;
                        http_cmdsta = HTTP_STAT_RESP_ERROR;
                    }
                    else
                    {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                        if   (!strcmp(pcValue[11], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                        else ow_txaddr = atoi(pcValue[11]), COM_Link = P2P; // send to specified onewire address 
                        COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                    }
                }
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* RT set display state:                tda, tdn, tdi, tdt, tbm, tbt, owa */
        else if (!strcmp(pcParam[0], "tda"))
        {   
            if   (!strcmp(pcValue[0], "RCgra")) // room thermostat rs485 address, interface or bridge device
            {  // macro in regard to last selected bus, RS485 bus Room Controller Group Address
                Int2Str(hc_buff, DEF_RC_RSGRA, 0);
                COM_Link = GROUP;    // send to room controller group address
            }
            else 
            {
                sizea = strlen (pcValue[0]); // get receiver rs485 address string size 
                memcpy(hc_buff, pcValue[0], sizea); // copy receiver rs485 address string                
            }
            //
            //  check all received paremeter strings
            //
            if (!strcmp(pcParam[1], "tdn")) ++sizeb; // room thermostat id number 1~9 
            if (!strcmp(pcParam[2], "tdi")) ++sizeb; // display image number
            if (!strcmp(pcParam[3], "tdt")) ++sizeb; // display image timeout
            if (!strcmp(pcParam[4], "tbm")) ++sizeb; // buzzer mode
            if (!strcmp(pcParam[5], "tbt")) ++sizeb; // buzzer repeat timer
            if (sizeb == 0x5U) // all parameter specified
            {
                for(cnt = 1U; cnt < 6U; cnt++)  // loop to copy all parameter values null separated
                {
                    sizeb = strlen (pcValue[cnt]);
                    memcpy (&hc_buff[sizea + 1U], pcValue[cnt], sizeb);
                    sizea += sizeb + 1U; // set next value position index in buffer
                }
//                request = RT_SET_DISP_STA;
//                http_cmdsta = HTTP_STAT_RESP_OK;
                while(HC_State != PCK_ENUM) HC_Service(); // finish previous transfer 
                //
                // if onewire address specified for room thermostat receiver interface,
                // this command and parameters will be packed for onewire bus transfer and
                // encapsulated inside rs485 packet which then will be send to rs485 to onewire
                // bridge device, usually room controller, and finaly to addressed room thermostat 
                //
                if (!strcmp(pcParam[6], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
                {
                    if (atoi(pcValue[6]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                    {
                        request = 0x0U;
                        COM_Bridge = BRNONE;
                        COM_Link = NOLINK;
                        http_cmdsta = HTTP_STAT_RESP_ERROR;
                        return weblog;
                    }
                    else
                    {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                        if   (!strcmp(pcValue[6], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                        else ow_txaddr = atoi(pcValue[6]), COM_Link = P2P; // send to specified onewire address 
                        COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                    }
                }                                           
                rs485_txaddr = atoi(pcValue[0]);
                HC_CreateCmdRequest(RT_SET_DISP_STA, hc_buff);
                sta = HTTP2RS485();
                if      (sta == ACK) http_cmdsta = HTTP_STAT_RESP_OK;
                else if (sta == 0)   http_cmdsta = HTTP_STAT_RESP_TOUT;
                else                 http_cmdsta = HTTP_STAT_RESP_ERROR;
            }
            else http_cmdsta = HTTP_STAT_RESP_ERROR;
            return weblog;
        }
        /* RT update/display qr code:           qra, qrc/qrd, owa  */
        else if (!strcmp(pcParam[0], "qra")) 
        {
            sizea = strlen (pcValue[0]); // get receiver rs485 address string size
            memcpy(hc_buff, pcValue[0], sizea); // copy receiver rs485 address string
            
            if      (!strcmp(pcParam[1], "qrc"))
            {
                sizeb = strlen (pcValue[1]); // get qr code text or cmd string size and
                memcpy (&hc_buff[sizea + 1U], pcValue[1], sizeb); // copy to sys buffer
                addr_cnt = sizea + 1U;
                sizea += sizeb;
                 
                while (sizeb) // replace all '+'' plus chars from qr code text to '&' ampersands 
                {
                    if (hc_buff[addr_cnt] == '+') hc_buff[addr_cnt] = '=';
                    if (hc_buff[addr_cnt] == '-') hc_buff[addr_cnt] = '&';
                    ++addr_cnt;
                    --sizeb;
                }
                request = RT_UPD_QRC;
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else if (!strcmp(pcParam[1], "qrd"))   // qr code display command parameter specified
            {
                request = RT_DISP_QRC;
                http_cmdsta = HTTP_STAT_RESP_OK;
            }
            else 
            {
                http_cmdsta = HTTP_STAT_RESP_ERROR;
                return weblog;
            }
            //
            // if onewire address specified for room thermostat receiver interface,
            // this command and parameters will be packed for onewire bus transfer and
            // encapsulated inside rs485 packet which then will be send to rs485 to onewire
            // bridge device, usually room controller, and finaly to addressed room thermostat 
            //
            if (!strcmp(pcParam[2], "owa"))   // if provided OneWire address enable bridge from RS458 to OneWire interface trough addressed RS485 device
            {
                if (atoi(pcValue[2]) == DEF_OWBRA)  // broadcast address is forbiden, abort request
                {
                    request = 0x0U;
                    COM_Bridge = BRNONE;
                    COM_Link = NOLINK;
                    http_cmdsta = HTTP_STAT_RESP_ERROR;
                }
                else
                {   // macro in regard to last selected bus, OneWire bus Room Thermostat Group Address
                    if   (!strcmp(pcValue[2], "RTgra")) ow_txaddr = DEF_RT_OWGRA, COM_Link = GROUP;
                    else ow_txaddr = atoi(pcValue[2]), COM_Link = P2P; // send to specified onewire address 
                    COM_Bridge = BR2OW; // set bridge mode for command encapsulation
                }
            }
            return weblog;
        }
	}
    /* uri to send after cgi call*/
    return webctrl;
}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{
    /* configure SSI handlers (ADC page SSI) */
    http_set_ssi_handler(HTTP_ResponseHandler, (char const **) TAGS, 1);
}

/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void)
{
    /* configure CGI handlers */
    CGI_TAB[0] = HTTP_CGI;
    http_set_cgi_handlers(CGI_TAB, 1);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
