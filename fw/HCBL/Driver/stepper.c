/**
 ******************************************************************************
 * File Name          : stepper.c
 * Date               : 28/02/2016 23:16:19
 * Description        : stepper motor control modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stepper.h"
#include "dio.h"

/* Variables  ----------------------------------------------------------------*/
/* Defines    ----------------------------------------------------------------*/
/* Types  --------------------------------------------------------------------*/
/* Macros     ----------------------------------------------------------------*/
/* Private prototypes    -----------------------------------------------------*/
void STEPPER_Init(void);

/* Program code   ------------------------------------------------------------*/
void STEPPER_Init(void)
{
	BarrierDriveVref1Release();
	BarrierDriveEnableRelease();
	BarrierDriveClockRelease();
	BarrierDriveResetAssert();
	BarrierDriveResetRelease();
	EntryRedSignal_On();
	ExitRedSignal_On();
	DOUT_Service();
}

/******************************   END OF FILE  **********************************/
