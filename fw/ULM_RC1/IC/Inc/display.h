/**
 ******************************************************************************
 * File Name          : display.h
 * Date               : 10.3.2018
 * Description        : GUI Display Module Header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_H__
#define __DISP_H__                           FW_BUILD // version
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "main.h"
/* Exported Define -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum	// display button states
{
	RELEASED    = 0U,
	PRESSED     = 1U,
    BUTTON_SHIT = 2U
	
}BUTTON_StateTypeDef;

typedef struct
{
    uint8_t index;      // index position in weather forecast screen
    uint8_t week_day;   // day in week
    uint8_t cloudness;  // used to select icon
    int8_t high_temp;   // maximun daily temperature color red value
    int8_t low_temp;    // minimal daily temperature colour blue value
	
}FORECAST_DayTypeDef;

extern FORECAST_DayTypeDef FORECAST_Day[];
/* Exported variables  -------------------------------------------------------*/
extern __IO uint32_t dispfl;
extern uint8_t disp_img_id, disp_img_time, last_img_id;
extern uint8_t disp_high_bcklght, disp_low_bcklght;
extern uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour, disp_rot;
extern uint8_t scrnsvr_tout, scrnsvr_clk_clr, scrnsvr_semiclk_clr;
extern int8_t wfc_buff[];
extern char buf0[84];
/* Exported Macro   --------------------------------------------------------- */
#define DISPUpdateSet()                     (dispfl |=  (1U<<0))
#define DISPUpdateReset()                   (dispfl &=(~(1U<<0)))
#define IsDISPUpdateActiv()                 (dispfl &   (1U<<0))

#define DISPBldrUpdSet()                    (dispfl |=  (1U<<1))
#define DISPBldrUpdReset()		            (dispfl &=(~(1U<<1)))
#define IsDISPBldrUpdSetActiv()		        (dispfl &   (1U<<1))

#define DISPBldrUpdFailSet()			    (dispfl |=  (1U<<2))
#define DISPBldrUpdFailReset()	            (dispfl &=(~(1U<<2)))
#define IsDISPBldrUpdFailActiv()	        (dispfl &   (1U<<2))

#define DISPUpdProgMsgSet()                 (dispfl |=  (1U<<3))
#define DISPUpdProgMsgDel()                 (dispfl &=(~(1U<<3)))
#define IsDISPUpdProgMsgActiv()             (dispfl &   (1U<<3))

#define DISPFwrUpdd()                       (dispfl |=  (1U<<4))
#define DISPFwrUpddDelete()                 (dispfl &=(~(1U<<4)))
#define IsDISPFwrUpddActiv()                (dispfl &   (1U<<4))

#define DISPFwrUpdFail()                    (dispfl |=  (1U<<5))
#define DISPFwrUpdFailDelete()              (dispfl &=(~(1U<<5)))
#define IsDISPFwrUpdFailActiv()             (dispfl &   (1U<<5))

#define DISPFwUpdSet()                      (dispfl |=  (1U<<6))
#define DISPFwUpdReset()                    (dispfl &=(~(1U<<6)))
#define IsDISPFwUpdActiv()                  (dispfl &   (1U<<6))

#define DISPFwUpdFailSet()                  (dispfl |=  (1U<<7))
#define DISPFwUpdFailReset()                (dispfl &=(~(1U<<7)))
#define IsDISPFwUpdFailActiv()              (dispfl &   (1U<<7))

#define DISPRFSenSet()                      (dispfl |=  (1U<<8))
#define DISPRFSenReset()                    (dispfl &=(~(1U<<8)))
#define IsDISPRFSenActiv()                  (dispfl &   (1U<<8))

#define DISPKeypadSet()                     (dispfl |=  (1U<<9))
#define DISPKeypadReset()                   (dispfl &=(~(1U<<9)))
#define IsDISPKeypadActiv()                 (dispfl &   (1U<<9))

#define DISPUnlockSet()                     (dispfl |=  (1U<<10))
#define DISPUnlockReset()                   (dispfl &=(~(1U<<10)))
#define IsDISPUnlockActiv()                 (dispfl &   (1U<<10))

#define DISPDoorOpenSet()                   (dispfl |=  (1U<<11))
#define DISPDoorOpenReset()                 (dispfl &=(~(1U<<11)))
#define IsDISPDoorOpenActiv()               (dispfl &   (1U<<11))

#define DISPLanguageSet()                   (dispfl |=  (1U<<12))
#define DISPLanguageReset()                 (dispfl &=(~(1U<<12)))
#define IsDISPLanguageActiv()               (dispfl &   (1U<<12))

#define DISPSettingsInitSet()               (dispfl |=  (1U<<13))	
#define DISPSettingsInitReset()             (dispfl &=(~(1U<<13)))
#define IsDISPSetInitActiv()                (dispfl &   (1U<<13))

#define DISPRefreshSet()					(dispfl |=  (1U<<14))
#define DISPRefreshReset()                  (dispfl &=(~(1U<<14)))
#define IsDISPRefreshActiv()			    (dispfl &   (1U<<14))

#define DISPCleanUpImage()					(dispfl |=  (1U<<15))
#define DISPCleanUpImgDel()			        (dispfl &=(~(1U<<15)))
#define IsDISPCleanUpImgActiv()			    (dispfl &   (1U<<15))

#define DISPBeddRepImg()		            (dispfl |=  (1U<<16))
#define DISPBeddRepImgDelete()              (dispfl &=(~(1U<<16)))
#define IsDISPBeddRepImgActiv()             (dispfl &   (1U<<16))

#define DISPGeneralCleanUpImg()			    (dispfl |=  (1U<<17))
#define DISPGenClnImgDelete()               (dispfl &=(~(1U<<17)))
#define IsDISPGenCleanImgActiv()	        (dispfl &   (1U<<17))

#define DISPCardValidImage()				(dispfl |=  (1U<<18))
#define DISPCardValidImgDel()			    (dispfl &=(~(1U<<18)))
#define IsDISPCardValidImgActiv()           (dispfl &   (1U<<18))

#define DISPCardInvalidImage()				(dispfl |=  (1U<<19))
#define DISPCardInvalidImgDel()		        (dispfl &=(~(1U<<19)))
#define IsDISPCardInvalidImgActiv()		    (dispfl &   (1U<<19))

#define DISPWrongRoomImage()				(dispfl |=  (1U<<20))
#define DISPWrongRoomImgDel()			    (dispfl &=(~(1U<<20)))
#define IsDISPWrongRoomImgActiv()           (dispfl &   (1U<<20))

#define DISPTimeExpiredImage()				(dispfl |=  (1U<<21))
#define DISPTimeExpiredImgDel()		        (dispfl &=(~(1U<<21)))
#define IsDISPTimeExpiredImgActiv()		    (dispfl &   (1U<<21))

#define DISPOutOfSrvcImgSet()               (dispfl |=  (1U<<22))
#define DISPOutOfSrvcImgReset()	            (dispfl &=(~(1U<<22)))
#define IsDISPOutOfSrvcImgActiv()	        (dispfl &   (1U<<22))

#define DISPMaidMenuSet()                   (dispfl |=  (1U<<23))
#define DISPMaidMenuReset()                 (dispfl &=(~(1U<<23)))
#define IsDISPMaidMenuActiv()               (dispfl &   (1U<<23))
/* Exported functions  -------------------------------------------------------*/
void DISPInit(void);
void DISPService(void);
void DISPGuiSet(uint8_t id);
uint8_t DISPMessage(uint8_t *buff);
uint8_t DISPQRCode(int x0, int y0);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
