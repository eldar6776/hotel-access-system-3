/**
 *******************************************************************************
 * File Name          : one_wire.h
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul header
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OW_H__
#define __OW_H__  						FW_BUILD // version

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
/* Exporeted Types   ---------------------------------------------------------*/
/* Exporeted Define   --------------------------------------------------------*/
/* Log event defines    ------------------------------------------------------*/
/* Exporeted Variable   ------------------------------------------------------*/
extern uint32_t ow_flags;
extern uint32_t ow_tmr;
extern uint8_t ow_bps;
/* Exporeted  Macro  ---------------------------------------------------------*/
#define OW_StartTmr(TIME)               (ow_tmr = TIME)
#define OW_StopTmr()                    (ow_tmr = 0)
#define IsOW_TmrExpired()               (ow_tmr == 0)
#define CardReadySet()					(ow_flags |=(1U << 0))
#define CardReadyReset()				(ow_flags &=(~(1U << 0)))
#define IsCardReady()					(ow_flags & (1U << 0))
#define CardInStackerSet()				(ow_flags |=(1U << 1))
#define CardInStackerReset()			(ow_flags &=(~(1U << 1)))
#define IsCardInStacker()				(ow_flags & (1U << 1))
#define RtcValidSet()                 	(ow_flags |=(1U << 2))
#define RtcValidReset()               	(ow_flags &=(~(1U << 2)))
#define IsRtcValidActiv()             	(ow_flags & (1U << 2))

/* Exported Function   -------------------------------------------------------*/
void OW_Init(void);
void OW_Service(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
