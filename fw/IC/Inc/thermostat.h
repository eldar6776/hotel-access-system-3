/**
 ******************************************************************************
 * File Name          : thermostat.h
 * Date               : 22/02/2018 07:03:00
 * Description        : temperature controller header
 ******************************************************************************
 */
 
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEMP_CTRL_H__
#define __TEMP_CTRL_H__                             FW_BUILD // version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"


/* Exported Type  ------------------------------------------------------------*/
/* Exported Define  ----------------------------------------------------------*/
#define FAN_OFF							            0U
#define FAN_CONST_SPEED_LOW                         72U
#define FAN_CONST_SPEED_MIDDLE                      66U
#define FAN_CONST_SPEED_HIGH                        60U   
#define FANC_RPM_MEASURE_TIME                       1234U
#define FANC_NTC_MEASURING_DELAY_TIME               600000U // 10 min. (x60 sec x 1000 ms)      
#define FANC_FAN_MIN_ON_TIME                        560U    // 0,5s between two or on/off fan speed switching
#define FANC_VALVE_MIN_ON_TIME                      5678U	// cooling valve state swith 5,5s min. time	
#define FANC_FREEZING_DIFF                          50      // 5,0*C temperature increase when freezing protection activated
#define FANC_NTC_UPDATE_TIME                        2345U   // 2.3 second fancoil ntc temperature update rate
#define THSTAT_POWER_ON_DELAY_TIME                  3456U   // 2,5 sec. power on startup time
#define FAN_BLDC_MOTOR_CONTROL_LOOP_TIME            100     // 100ms speed loop time for bldc motor fan
#define FAN_BLDC_MOTOR_MAX_SETPOINT                 100     // bldc fan motor speed set point 100% output


/* Exported Variable   -------------------------------------------------------*/
extern __IO uint32_t therm_flags;

extern __IO int16_t room_temp;
extern __IO int16_t room_ntc_temp;
extern __IO int8_t  room_ntc_offset;

extern __IO int16_t fancoil_temp;
extern __IO int16_t fancoil_ntc_temp;
extern __IO int8_t  fancoil_ntc_offset;

extern __IO uint8_t therm_min_sp;
extern __IO uint8_t therm_max_sp;
extern __IO uint8_t therm_temp_sp;
extern __IO uint8_t therm_op_mode;              //  thermostat operation off/cooling/heating/remote
extern __IO uint8_t therm_error_time;
extern __IO uint8_t therm_limit_mode;
extern __IO uint8_t therm_min_heating;
extern __IO uint8_t therm_max_cooling;
extern __IO uint8_t therm_freezing_temp;


extern __IO uint8_t fan_speed;

extern __IO uint8_t fan_speed_treshold;                     //  +/-0,3°C treshold for fan speed change 
extern __IO uint8_t fan_low_speed_band;                     //  set point +/- 0,8°C low speed fan zone
extern __IO uint8_t fan_middle_speed_band;                  //  set point +/- 1,8°C middle speed fan zone
extern __IO uint8_t fan_quiet_mode_start_hour;


/* Exported Macro ------------------------------------------------------------*/
/** ==========================================================================*/
/**       M E M O R I S E D     A N D     R E S T O R E D     F L A G S       */
/** ==========================================================================*/
#define THSTAT_ControllerOn()                       (therm_flags |= (1U << 0))
#define THSTAT_ControllerOff()                      (therm_flags &= (~ (1U << 0)))
#define IsTHSTAT_ControllerActiv()                  ((therm_flags & (1U << 0)) != 0U)

#define RoomNtcSensorConnected()                    (therm_flags |= (1U << 1))
#define RoomNtcSensorDisconnected()                 (therm_flags &= (~ (1U << 1)))
#define IsRoomNtcSensorConnected()                  ((therm_flags & (1U << 1)) != 0U)

#define FancoilNtcSensorConnected()                 (therm_flags |= (1U << 2))
#define FancoilNtcSensorDisconnected()              (therm_flags &= (~ (1U << 2)))
#define IsFancoilNtcSensorConnected()               ((therm_flags & (1U << 2)) != 0U)

#define FANC_CoolingValveSet()                      (therm_flags |= (1U << 3))
#define FANC_CoolingValveReset()                    (therm_flags &= (~ (1U << 3)))
#define IsFANC_CoolingValveActiv()                  ((therm_flags & (1U << 3)) != 0U)

#define FancoilNtcSensorErrorSet()                  (therm_flags |= (1U << 4))
#define FancoilNtcSensorErrorReset()                (therm_flags &= (~ (1U << 4)))
#define IsFancoilNtcSensorErrorActiv()              ((therm_flags & (1U << 4)) != 0U)

#define FANC_HeatingTempErrorSet()                  (therm_flags |= (1U << 5))
#define FANC_HeatingTempErrorReset()                (therm_flags &= (~ (1U << 5)))
#define IsFANC_HeatingTempErrorActiv()              ((therm_flags & (1U << 5)) != 0U)

#define FANC_CoolingTempErrorSet()                  (therm_flags |= (1U << 6))
#define FANC_CoolingTempErrorReset()                (therm_flags &= (~ (1U << 6)))
#define IsFANC_CoolingTempErrorActiv()              ((therm_flags & (1U << 6)) != 0U)

#define FANC_FreezingAlarmSet()                     (therm_flags |= (1U << 7))
#define FANC_FreezingAlarmReset()                   (therm_flags &= (~ (1U << 7)))
#define IsFANC_FreezingAlarmActiv()                 ((therm_flags & (1U << 7)) != 0U)

#define RoomNtcSensorErrorSet()                     (therm_flags |= (1U << 8))
#define RoomNtcSensorErrorReset()                   (therm_flags &= (~ (1U << 8)))
#define IsRoomNtcSensorErrorActiv()                 ((therm_flags & (1U << 8)) != 0U)

#define THSTAT_OnecycleHeatingSet()                 (therm_flags |= (1U << 9))
#define THSTAT_OnecycleHeatingReset()               (therm_flags &= (~ (1U << 9)))
#define IsTHSTAT_OnecycleHeatingActiv()             ((therm_flags & (1U << 9)) != 0U)

#define FanQuietModeSet()                           (therm_flags |= (1U << 10)) 
#define FanQuietModeReset()                         (therm_flags &= (~ (1U << 10)))
#define IsFanQuietModeActiv()                       ((therm_flags & (1U << 10)) != 0U)

#define FanRpmSensorErrorShutdownSet()              (therm_flags |= (1U << 11)) 
#define FanRpmSensorErrorShutdownReset()            (therm_flags &= (~ (1U << 11)))
#define IsFanRpmSensorErrorShutdownActiv()          ((therm_flags & (1U << 11)) != 0U)

#define FanRpmSensorErrorReportSet()                (therm_flags |= (1U << 12)) 
#define FanRpmSensorErrorReportReset()              (therm_flags &= (~ (1U << 12)))
#define IsFanRpmSensorErrorReportActiv()            ((therm_flags & (1U << 12)) != 0U)

#define FanRpmSensorConnected()                     (therm_flags |= (1U << 13)) 
#define FanRpmSensorDisconnected()                  (therm_flags &= (~ (1U << 13)))
#define IsFanRpmSensorConnected()                   ((therm_flags & (1U << 13)) != 0U)

#define FreezingProtectionReportSet()               (therm_flags |= (1U << 14)) 
#define FreezingProtectionReportReset()             (therm_flags &= (~ (1U << 14)))
#define IsFreezingProtectionReportActiv()           ((therm_flags & (1U << 14)) != 0U)

#define FreezingProtectionSet()                     (therm_flags |= (1U << 15)) 
#define FreezingProtectionReset()                   (therm_flags &= (~ (1U << 15)))
#define IsFreezingProtectionActiv()                 ((therm_flags & (1U << 15)) != 0U)

#define FilterDirtyShutdownSet()                    (therm_flags |= (1U << 16)) 
#define FilterDirtyShutdownReset()                  (therm_flags &= (~ (1U << 16)))
#define IsFilterDirtyShutdownActiv()                ((therm_flags & (1U << 16)) != 0U)

#define FANC_FilterDirtyReportSet()                 (therm_flags |= (1U << 17)) 
#define FANC_FilterDirtyReportReset()               (therm_flags &= (~ (1U << 17)))
#define IsFANC_FilterDirtyReportActiv()             ((therm_flags & (1U << 17)) != 0U)

#define FANC_FilterDirtyMonitorSet()                (therm_flags |= (1U << 18)) 
#define FANC_FilterDirtyMonitorReset()              (therm_flags &= (~ (1U << 18)))
#define IsFANC_FilterDirtyMonitorActiv()            ((therm_flags & (1U << 18)) != 0U)

#define FANC_ValveFollowFanSet()                    (therm_flags |= (1U << 19)) 
#define FANC_ValveFollowFanReset()                  (therm_flags &= (~ (1U << 19)))
#define IsFANC_ValveFollowFanActiv()                ((therm_flags & (1U << 19)) != 0U)

#define FANC_ErrorAutorestartSet()                  (therm_flags |= (1U << 20)) 
#define FANC_ErrorAutorestartReset()                (therm_flags &= (~ (1U << 20)))
#define IsFANC_ErrorAutorestartActiv()              ((therm_flags & (1U << 20)) != 0U)

#define CoolingErrorReportSet()                     (therm_flags |= (1U << 21)) 
#define CoolingErrorReportReset()                   (therm_flags &= (~ (1U << 21)))
#define IsCoolingErrorReportActiv()                 ((therm_flags & (1U << 21)) != 0U)

#define FANC_HeatingErrorReportSet()                (therm_flags |= (1U << 22)) 
#define FANC_HeatingErrorReportReset()              (therm_flags &= (~ (1U << 22)))
#define IsFANC_HeatingErrorReportActiv()            ((therm_flags & (1U << 22)) != 0U)

#define FANC_NtcErrorReportSet()                    (therm_flags |= (1U << 23)) 
#define FANC_NtcErrorReportReset()                  (therm_flags &= (~ (1U << 23)))
#define IsFANC_NtcErrorReportActiv()                ((therm_flags & (1U << 23)) != 0U)

#define RoomNtcErrorReportSet()                     (therm_flags |= (1U << 24)) 
#define RoomNtcErrorReportReset()                   (therm_flags &= (~ (1U << 24)))
#define IsRoomNtcErrorReportActiv()                 ((therm_flags & (1U << 24)) != 0U)

#define CoolingErrorShutdownSet()                   (therm_flags |= (1U << 25)) 
#define CoolingErrorShutdownReset()                 (therm_flags &= (~ (1U << 25)))
#define IsCoolingErrorShutdownActiv()               ((therm_flags & (1U << 25)) != 0U)

#define HeatingErrorShutdownSet()                   (therm_flags |= (1U << 26)) 
#define HeatingErrorShutdownReset()                 (therm_flags &= (~ (1U << 26)))
#define IsHeatingErrorShutdownActiv()               ((therm_flags & (1U << 26)) != 0U)

#define FancoilNtcErrorShutdownSet()                (therm_flags |= (1U << 27)) 
#define FancoilNtcErrorShutdownReset()              (therm_flags &= (~ (1U << 27)))
#define IsFancoilNtcErrorShutdownActiv()            ((therm_flags & (1U << 27)) != 0U)

#define RoomNtcErrorShutdownSet()                   (therm_flags |= (1U << 28)) 
#define RoomNtcErrorShutdownReset()                 (therm_flags &= (~ (1U << 28)))
#define IsRoomNtcErrorShutdownActiv()               ((therm_flags & (1U << 28)) != 0U)

#define FancoilRpmSensorErrorSet()                  (therm_flags |= (1U << 29)) 
#define FancoilRpmSensorErrorReset()                (therm_flags &= (~ (1U << 29)))
#define IsFancoilRpmSensorErrorActiv()              ((therm_flags & (1U << 29)) != 0U)

#define TemperatureRegulatorOn()                    (therm_temp_sp  |= (1U << 7))
#define TemperatureRegulatorOff()                   (therm_temp_sp  &= (~ (1U << 7)))
#define IsTemperatureRegulatorOn()                  ((therm_temp_sp &  (1U << 7)) != 0U)
#define TemperatureRegulatorSetHeating()            (therm_temp_sp  |= (1U << 6))
#define IsTemperatureRegulatorHeating()             ((therm_temp_sp &  (1U << 6)) != 0U)
#define IsTemperatureRegulatorCooling()             ((therm_temp_sp &  (1U << 6)) == 0U)
#define TemperatureRegulatorSetCooling()            (therm_temp_sp  &= (~ (1U << 6)))

#define IsTHSTAT_OperationModeOff()                 (therm_op_mode == 0U)
#define IsTHSTAT_OperationModeCooling()             (therm_op_mode == 1U)
#define IsTHSTAT_OperationModeHeating()             (therm_op_mode == 2U)
#define IsTHSTAT_OperationModeRemote()              (therm_op_mode == 3U)



#define IsFANC_WaterLimitTemperatureValue()         (therm_limit_mode == 0U)
#define IsFANC_WaterLimitSetpointDifference()       (therm_limit_mode == 1U)
/** ============================================================================*/
/**   F A N C O I L   C O N T O L   W I T H   4   D I G I T A L   O U T P U T   */
/** ============================================================================*/
#define FANC_FanLowSpeedOn()        (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET))
#define FANC_FanLowSpeedOff()       (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET))
#define IsFANC_FanLowSpeedOn()      (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_SET)
#define FANC_FanMiddleSpeedOn()     (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET))
#define FANC_FanMiddleSpeedOff()    (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET))
#define IsFANC_FanMiddleSpeedOn()   (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_SET)
#define FANC_FanHighSpeedOn()       (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET))
#define FANC_FanHighSpeedOff()      (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET))	
#define IsFANC_FanHighSpeedOn()     (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_12) == GPIO_PIN_SET)
#define FANC_FanOff()               (HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET))
#define FANC_CoolingValveOn()       (HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET))
#define FANC_CoolingValveOff()      (HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET))
#define IsFANC_CoolingValveOn()     (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_SET)
/** ============================================================================*/
/**   F A N C O I L   C O N T O L   W I T H   T R I A C   A N D		D O U T     */
/** ============================================================================*/
#define FANC_SetFanOff()            (FAN_SetSpeed(FAN_OFF))
#define FANC_SetFanLowSpeed()       (FAN_SetSpeed(FAN_CONST_SPEED_LOW))
#define FANC_SetFanMiddleSpeed()    (FAN_SetSpeed(FAN_CONST_SPEED_MIDDLE))
#define FANC_SetFanHighSpeed()      (FAN_SetSpeed(FAN_CONST_SPEED_HIGH))


/* Exported Function  ------------------------------------------------------- */
void THSTAT_Init(void);
void THSTAT_Service(void);
void THSTAT_SaveSettings(void);

#endif
/******************************   END OF FILE  ********************************/
