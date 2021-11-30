/**
  ******************************************************************************
  * @file    stm32746g_discovery_eeprom.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for 
  *          the stm32746g_discovery_eeprom.c firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H__                        
#define __EEPROM_H__                        FW_BUILD // version

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32746g.h"


/* EEPROM hardware address and page size */ 
#define EE_PGSIZE                           64U
#define EE_PGNUM                            0x100U  // number of pages
#define EE_MAXSIZE                          0x4000U /* 64Kbit */
#define EE_ENDADDR                          0x3FFFU    
#define EE_ADDR                             0xA0U
#define EETOUT                              1000U
#define EE_WR_TIME                          15U
#define EE_TRIALS                           200U
#define EE_MAX_TRIALS                       3000U
#define EE_OK                               0x0U
#define EE_FAIL                             0x1U
#define EE_TOUT                             0x2U
#define EE_THST_FLAGS                       0x0U    // first group of system flags
#define EE_MIN_SETPOINT                     0x4U
#define EE_MAX_SETPOINT                     0x5U
#define EE_THST_SETPOINT                    0x6U
#define EE_NTC_OFFSET                       0x7U
#define EE_OW_BPS                           0x8U
#define EE_OWIF_ADDR                        0x9U
#define EE_DISP_LOW_BCKLGHT                 0xAU
#define EE_DISP_HIGH_BCKLGHT                0xBU
#define EE_SCRNSVR_TOUT                     0xCU
#define EE_SCRNSVR_ENABLE_HOUR              0xDU
#define EE_SCRNSVR_DISABLE_HOUR             0xEU
#define EE_SCRNSVR_CLK_COLOR                0xFU
#define EE_SCRNSVR_SEMICLK_COLOR            0x10U
#define EE_SYS_STATE                        0x11U
#define EE_ROOM_PRESTAT_ADD				    0x12U	// room pre status address
#define EE_ROOM_STAT_ADD				    0x13U	// room status address
#define EE_FW_UPDATE_BYTE_CNT               0x14U	// firmware update byte count
#define EE_FW_UPDATE_STATUS                 0x18U	// firmware update status
#define EE_ROOM_TEMP_SP                     0x19U	// room setpoint temp  in degree of Celsious
#define EE_ROOM_TEMP_DIFF		            0x1BU	// room tempreature on / off difference
#define EE_RSIFA			                0x1CU	// rs485 device address
#define EE_RSGRA				            0x1EU	// rs485 group broadcast address
#define EE_RSBRA			                0x20U	// rs485 broadcast address msb
#define EE_RSBPS					        0x22U	// rs485 interface baudrate
#define EE_ROOM_PWRTOUT 				    0x24U	// room power expiry date time
#define EE_SYSID				            0x2AU	// system id (system unique number)
#define EE_BEDNG_CNT_ADD		            0x2CU	// rfid signal user selected options
#define	EE_BEDNG_REPL_ADD		            0x2EU	// do-not-disturb signal user selected options
#define EE_ROOM_TEMP_CFG					0x2FU	// Room Controlle Thermostat Config:OFF/HEATING/COOLING
#define EE_MIFARE_KEYA                      0x30U	// mifare access authentication key A
#define EE_MIFARE_KEYB                      0x36U	// mifare access authentication key B 
#define EE_SYS_FLAGS1                       0x3CU   // second group of system flags
#define EE_USRGR_ADD                        0x40U   // permited group first address
#define EE_USRGR_ADD1_ID                    0x40U	// mifare access permited group 1 id
#define EE_USRGR_ADD2_ID                    0x41U	// mifare access permited group 2 id
#define EE_USRGR_ADD3_ID                    0x42U	// mifare access permited group 3 id
#define EE_USRGR_ADD4_ID                    0x43U	// mifare access permited group 4 id
#define EE_USRGR_ADD5_ID                    0x44U   // mifare access permited group 5 id
#define EE_USRGR_ADD6_ID                    0x45U   // mifare access permited group 6 id
#define EE_USRGR_ADD7_ID                    0x46U   // mifare access permited group 7 id
#define EE_USRGR_ADD8_ID                    0x47U   // mifare access permited group 8 id
#define EE_USRGR_ADD9_ID                    0x48U   // mifare access permited group 9 id
#define EE_USRGR_ADD10_ID                   0x49U   // mifare access permited group 10 id
#define EE_USRGR_ADD11_ID                   0x4AU   // mifare access permited group 11 id
#define EE_USRGR_ADD12_ID                   0x4BU   // mifare access permited group 12 id
#define EE_USRGR_ADD13_ID                   0x4CU   // mifare access permited group 13 id
#define EE_USRGR_ADD14_ID                   0x4DU   // mifare access permited group 14 id
#define EE_USRGR_ADD15_ID                   0x4EU   // mifare access permited group 15 id
#define EE_USRGR_ADD16_ID                   0x4FU   // mifare access permited group 16 id
#define EE_PERM_EXTADD1                     0x50U	// additional permited address 1
#define EE_PERM_EXTADD2                     0x52U 	// additional permited address 2
#define EE_PERM_EXTADD3                     0x54U	// additional permited address 3
#define EE_PERM_EXTADD4                     0x56U	// additional permited address 4
#define EE_PERM_EXTADD5                     0x58U	// additional permited address 5
#define EE_PERM_EXTADD6                     0x5AU   // additional permited address 6
#define EE_PERM_EXTADD7                     0x5CU   // additional permited address 7
#define EE_PERM_EXTADD8                     0x5EU	// additional permited address 8
#define EE_SYS_FLAGS2                       0x60U   // third group of system flags
#define EE_OWIFA                            0x66U	// onewire interface address
#define EE_OWGRA                            0x67U	// onewire group address
#define EE_OWBRA			                0x68U	// onewire broadcast address
#define EE_OWBPS                            0x69U	// onewire interface baudrate
#define EE_OWADD1                           0x6AU	// onewire slave address 1 
#define EE_OWADD2                           0x6BU	// onewire slave address 2
#define EE_OWADD3                           0x6CU	// onewire slave address 3 
#define EE_OWADD4                           0x6DU	// onewire slave address 4 
#define EE_OWADD5                           0x6EU	// onewire slave address 5 
#define EE_OWADD6                           0x6FU	// onewire slave address 6 
#define EE_OWADD7                           0x70U	// onewire slave address 7 
#define EE_OWADD8                           0x71U	// onewire slave address 8 
#define EE_OWADD9                           0x72U	// onewire slave address 9
#define EE_DISP_STATUS_ADD                  0x74U	// display status flags
#define EE_BUZZER_VOLUME_ADD                0x75U   // buzzer volume address
#define EE_DOORLOCK_FORCE_ADD               0x76U	// doorlock force address
#define EE_DISP_ROTATION_ADD                0x77U	// display rotation address
#define EE_LCD_BRIGHTNESS					0x78U   // Display backlight 
#define EE_SYS_FLAGS3                       0x7CU   // fourth group of system flags
#define EE_DIN_CFG_ADD_1                    0x80U   // digital input 1 config
#define EE_DIN_CFG_ADD_2                    0x81U   // digital input 2 config
#define EE_DIN_CFG_ADD_3                    0x82U   // digital input 3 config
#define EE_DIN_CFG_ADD_4                    0x83U   // digital input 4 config
#define EE_DIN_CFG_ADD_5                    0x84U   // digital input 5 config
#define EE_DIN_CFG_ADD_6                    0x85U   // digital input 6 config
#define EE_DIN_CFG_ADD_7                    0x86U   // digital input 7 config
#define EE_DIN_CFG_ADD_8                    0x87U   // digital input 8 config
#define EE_DOUT_CFG_ADD_1                   0x88U   // digital output 1 config
#define EE_DOUT_CFG_ADD_2                   0x89U   // digital output 2 config
#define EE_DOUT_CFG_ADD_3                   0x8AU   // digital output 3 config
#define EE_DOUT_CFG_ADD_4                   0x8BU   // digital output 4 config
#define EE_DOUT_CFG_ADD_5                   0x8CU   // digital output 5 config
#define EE_DOUT_CFG_ADD_6                   0x8DU   // digital output 6 config
#define EE_DOUT_CFG_ADD_7                   0x8EU   // digital output 7 config
#define EE_DOUT_CFG_ADD_8                   0x8FU   // digital output 8 config
#define EE_USER_PSWRD                       0x90U   // user password first address
#define EE_USER_PSWRD1                      0x90U   // user password
#define EE_USER_TIME1                       0x94U   // password 1 expiry time
#define EE_USER_PSWRD2                      0x98U   // user 2 password
#define EE_USER_TIME2                       0x9CU   // password 2 expiry time
#define EE_USER_PSWRD3                      0xA0U   // user 3 password
#define EE_USER_TIME3                       0xA4U   // password 3 expiry time
#define EE_USER_PSWRD4                      0xA8U   // user 4 password
#define EE_USER_TIME4                       0xACU   // password 4 expiry time
#define EE_USER_PSWRD5                      0xB0U   // user password
#define EE_USER_TIME5                       0xB4U   // password 1 expiry time
#define EE_USER_PSWRD6                      0xB8U   // user 2 password
#define EE_USER_TIME6                       0xBCU   // password 2 expiry time
#define EE_USER_PSWRD7                      0xC0U   // user 3 password
#define EE_USER_TIME7                       0xC4U   // password 3 expiry time
#define EE_USER_PSWRD8                      0xC8U   // user 4 password
#define EE_USER_TIME8                       0xCCU   // password 4 expiry time
#define EE_MAID_PSWRD                       0xD0U   // handmaid password
#define EE_MNGR_PSWRD                       0xD4U   // menager password
#define EE_SRVC_PSWRD                       0xD8U   // service password
#define EE_RST_PSWRD                        0xDCU   // reset password
#define EE_PLYRM_PSWRD                      0xE0U   // playroom password
#define EE_POOL_PSWRD                       0xE4U   // pool password
#define EE_PARKING_PSWRD                    0xE8U   // parking password
#define EE_GARAGE_PSWRD                     0xECU   // garage password
#define EE_ENTRY_PSWRD                      0xF0U   // entry password
#define EE_LIFT_PSWRD                       0xF4U   // lift password
#define EE_DEAULT_INIT                      0xFFU   // value 0xA5U written if default value written 
#define EE_WFORECAST                        0x100U  // one page =  64 byte for weather forecast
#define EE_QR_CODE                          0x140U  // two page = 128 byte for qr code 
#define EE_LOG_LIST_START_ADDR              0x200U
#define EE_LOG_LIST_END_ADDR                EE_ENDADDR

/* Link function for I2C EEPROM peripheral */
void     EE_Init         (void);
uint32_t EE_ReadBuffer   (uint8_t *pBuffer, uint16_t ReadAddr,  uint16_t NumByteToRead);
uint32_t EE_WriteBuffer  (uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);


#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
