/**
 ******************************************************************************
 * File Name          : common.h
 * Date               : 10.3.2018
 * Description        : usefull function set and program shared constants
 ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H__
#define __COMMON_H__                              RT_FW_BUILD	// header version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ISCAPLETTER(c)    	    ((*(char*)c >= 'A') && (*(char*)c <= 'F'))
#define ISLCLETTER(c)     	    ((*(char*)c >= 'a') && (*(char*)c <= 'f'))
#define ISVALIDNUM(c)           ((*(char*)c >= '0') && (*(char*)c <= '9'))
#define ISVALIDHEX(c)       	(ISCAPLETTER(c) || ISLCLETTER(c) || ISVALIDNUM(c))
#define ISVALIDDEC(c)     		ISVALIDNUM(c)
#define CHAR2NUM(c)       	    (*(char*)c - '0')
#define CHAR2ALPHA(c) 	        (ISCAPLETTER(c) ? (*(char*)c - 'A' + 10U) : (*(char*)c - 'a' + 10U))
#define CHAR2HEX(c)       	    (ISVALIDNUM(c) ? CHAR2NUM(c) : CHAR2ALPHA(c))
#define BCD2DEC(x)              ((((x) >> 4U) & 0x0FU) * 10U + ((x) & 0x0FU))
#define LEAP_YEAR(year)         ((((year) % 4U == 0U) && ((year) % 100U != 0U)) || ((year) % 400U == 0U))
#define DAYS_IN_YEAR(x)         (LEAP_YEAR(x) ? 366U : 365U)
#define UNIX_OFFSET_YEAR        1970U
#define SECONDS_PER_DAY         86400U
#define SECONDS_PER_HOUR        3600U
#define SECONDS_PER_MINUTE      60U
#define BUFF_LEN(__BUFFER__)   	(sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define COUNTOF(a)          	(sizeof(a) / sizeof(a[0]))
#define MIN(a,b)				(((a) < (b)) ? (a) : (b))
#define MAX(a,b)				(((a) > (b)) ? (a) : (b))
#define ZEROFILL(p, Size)   	(memset(p, 0, Size))
#define HEX2STR(s,h)            (Hex2Str(s, h, 2U)) // convert single hex to 2 char string null teminated


/* Exported function  ------------------------------------------------------- */
uint8_t Bcd2Dec(uint8_t val);
uint8_t Dec2Bcd(uint8_t val);
uint32_t GetSize(const uint8_t *pbuf);
void CharToBinStr(char c, char *pstr);
uint32_t BaseToPower(uint16_t base, uint8_t power);
uint8_t CalcCRC(const uint8_t *pbuf, uint16_t size);
int32_t Str2Int(const char *pstr, uint8_t str_size);
void Int2Str(char *pstr, int32_t val, uint8_t str_size);
void Str2Hex(const char *pstr, uint8_t *phex, uint16_t str_size);
void Hex2Str(char *pstr, const uint8_t *phex, uint16_t str_size);

#endif  /* __COMMON_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
