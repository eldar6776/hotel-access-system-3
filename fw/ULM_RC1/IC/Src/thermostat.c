/**
 ******************************************************************************
 * File Name          : thermostat.c
 * Date               : 22/02/2018 06:59:00
 * Description        : temperature control processing modul 
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Include  ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "logger.h"
#include "display.h"
#include "onewire.h"
#include "thermostat.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"


#if (__TEMP_CTRL_H__ != FW_BUILD)
    #error "thermostat header version mismatch"
#endif


/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
__IO uint32_t therm_flags;
__IO int16_t room_temp;
__IO int16_t room_ntc_temp;
__IO int16_t fancoil_temp;
__IO int16_t fancoil_ntc_temp;
__IO int8_t  room_ntc_offset;
__IO int8_t  fancoil_ntc_offset;
__IO uint8_t therm_temp_sp;
__IO uint8_t therm_error_time;
__IO uint8_t therm_limit_mode;
__IO uint8_t therm_min_heating;
__IO uint8_t therm_max_cooling;
__IO uint8_t therm_min_sp;
__IO uint8_t therm_max_sp;
__IO uint8_t therm_freezing_temp;
__IO uint8_t therm_op_mode;
__IO uint8_t fan_speed;
__IO uint8_t fan_control_mode;
__IO uint8_t fan_speed_treshold;
__IO uint8_t fan_low_speed_band;
__IO uint8_t fan_middle_speed_band;
__IO uint8_t fan_quiet_mode_start_hour;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void THSTAT_Init(void)
{
    therm_flags = therm_flags_memo;
    
	GPIO_InitTypeDef  GPIO_InitStruct;
	  
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2|GPIO_PIN_4, GPIO_PIN_RESET);
	
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}
/**
  * @brief
  * @param
  * @retval
  */
void THSTAT_Service(void)
{
    static uint8_t flags = 0U;
    static int16_t temp_sp = 0U;
    static uint32_t fan_pcnt = 0U;
    static uint32_t old_fan_speed = 0U;
    static uint32_t fan_calc_speed = 0U;
    static uint32_t therm_time = 0U;
    static uint32_t therm_timer = 0U;
    static uint32_t fancoil_ntc_timer = 0U;
    static uint32_t fancoil_fan_timer = 0U;
    static uint32_t fancoil_valve_timer = 0U;
    static uint32_t fancoil_fluid_error_time = 0U;
    static uint32_t fancoil_fluid_error_timer = 0U;
    static uint32_t bldc_fan_speed_control_timer = 0U;
    /** ============================================================================*/
	/**		C H E C K       T I M E R    A N D      S Y S T E M     F L A G S	    */
	/** ============================================================================*/
    if((HAL_GetTick() - therm_timer) < therm_time) return;
    /** ============================================================================*/
	/**		    R U N       O N C E     R P M       S E N S O R     C H E C K	    */
	/** ============================================================================*/
    if(IsSYSTEM_StartupActiv())
    {
        if(IsFANC_FanMotorBldcControl())
        {
            return;
        }
        
        if(fan_pcnt == 0U)
        {
            if(IsFANC_FanMotorTriacControl()) FANC_SetFanLowSpeed();
            therm_timer = HAL_GetTick();
            therm_time = FANC_RPM_MEASURE_TIME;
            ++fan_pcnt;
        }
        else if(fan_pcnt == 1U)
        {
            therm_timer = HAL_GetTick();
            therm_time = FANC_RPM_MEASURE_TIME;
            ++fan_pcnt;
        }
        else if(fan_pcnt == 2U)
        {
            /***********************************************************************************************************
            *   run fancoil fan motor shortly with triac driver to sense hall rpm sensor if available, 
            *   this will define functions used to controll hardware interface and type of temperature controller
            *   rpm sensor will be also used for cleaning request when motor start to slow down due to dust collect
            *   if sensor not available,than tree speed motor is used for fancoil fan, and controller is simpler without
            *   precise timings for triac triggering releasing from runtime code two interrupt request services
            ************************************************************************************************************/ 
            if (fan_rpm_pulse > 10U) FANC_RPM_SensorConnected();
            else FANC_RPM_SensorNotConnected();
            therm_time = THSTAT_POWER_ON_DELAY_TIME;
            therm_timer = HAL_GetTick();
            if (IsFANC_FanMotorTriacControl()) FANC_SetFanOff();
            ++fan_pcnt;
        }
        
        return;
    }    
    /** ============================================================================*/
	/**		U P D A T E 	R O O M     A M B I E N T     T E M P E R A T U R E     */
	/** ============================================================================*/
    if(IsRoomNtcSensorConnected())
    {
        if(room_temp != room_ntc_temp + room_ntc_offset)
        {
            room_temp = room_ntc_temp + room_ntc_offset;
            RoomTempUpdateSet();
        }
    }
    else
    {
        room_temp = 0;
        return;
    }
    /** ============================================================================*/
    /**     U P D A T E     F A N C O I L     F L U I D     T E M P E R A T U R E   */
    /** ============================================================================*/
    if(IsFancoilNtcSensorConnected())
    {
        if((HAL_GetTick() - fancoil_ntc_timer) >= FANC_NTC_UPDATE_TIME) 
        {
            fancoil_ntc_timer = HAL_GetTick();
            fancoil_fluid_error_time = (uint32_t) (therm_error_time * 60000U);
            
            if(fancoil_temp != fancoil_ntc_temp + fancoil_ntc_offset)
            {
                fancoil_temp = fancoil_ntc_temp + fancoil_ntc_offset;
                FancoilTempUpdateSet();
            }
            /** ============================================================================*/
            /**     F A N C O I L       F R E E Z I N G         P R O T E C T I O N         */
            /** ============================================================================*/
            if(IsFreezingProtectionActiv() && !IsTHSTAT_OperationModeOff())
            {
                if(!IsTHSTAT_OnecycleHeatingActiv())
                {
                    if((fancoil_temp < therm_freezing_temp))
                    {
                        if(IsFreezingProtectionReportActiv()) 
                        {
                            FANC_FreezingAlarmSet();
                        }
                        
                        TemperatureRegulatorOff();
                        THSTAT_OnecycleHeatingSet();
                        
                        if      (IsFANC_FanMotor3SpeedControl()) FANC_FanHighSpeedOn();
                        else if (IsFANC_FanMotorTriacControl())  FANC_SetFanHighSpeed();
                        else if (IsFANC_FanMotorBldcControl())   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, FAN_BLDC_MOTOR_MAX_SETPOINT);
                        
                        FANC_CoolingValveOn();
                        fancoil_fan_timer = HAL_GetTick();
                        fancoil_valve_timer = HAL_GetTick();
                        return;
                    }                       
                }
                else if(IsTHSTAT_OnecycleHeatingActiv())
                {
                    if((fancoil_temp >= therm_freezing_temp + FANC_FREEZING_DIFF))
                    {
                        if(IsTHSTAT_ControllerActiv())
                        {
                            TemperatureRegulatorOn();
                        }
                        
                        THSTAT_OnecycleHeatingReset();
                        old_fan_speed = 1U;
                        fan_speed = 0U;
                    }
                    
                    fancoil_fluid_error_timer = HAL_GetTick();
                    return;
                }
            }
            /** ============================================================================*/
            /**         C H E C K    F L U I D     T E M P E R A T U R E     L I M I T S    */
            /** ============================================================================*/
            if(IsTemperatureRegulatorOn())
            {
                if(IsFANC_WaterLimitTemperatureValue())
                {
                    if(IsTemperatureRegulatorHeating())
                    {
                        if(fancoil_temp < ((int16_t) (therm_min_heating * 10)))
                        {
                            if((HAL_GetTick() - fancoil_fluid_error_timer) >= fancoil_fluid_error_time)
                            { 
                                if(IsFANC_HeatingErrorReportActiv()) 
                                {
                                    FANC_HeatingTempErrorSet();
                                }
                                
                                if(IsHeatingErrorShutdownActiv()) 
                                {
                                    TemperatureRegulatorOff();
                                } 
                                
                                fancoil_fluid_error_timer = HAL_GetTick();
                            }
                        }
                        else
                        {
                            fancoil_fluid_error_timer = HAL_GetTick();
                        }
                    }
                    else if(IsTemperatureRegulatorCooling()) 
                    {
                        if(fancoil_temp > ((int16_t) (therm_max_cooling * 10)))
                        {
                            if((HAL_GetTick() - fancoil_fluid_error_timer) >= fancoil_fluid_error_time)
                            {
                                fancoil_fluid_error_timer = HAL_GetTick();
                                
                                if(IsCoolingErrorReportActiv()) 
                                {
                                    FANC_CoolingTempErrorSet();
                                }
                                
                                if(IsCoolingErrorShutdownActiv()) 
                                {
                                    TemperatureRegulatorOff();
                                }
                            }
                        }
                        else
                        {
                            fancoil_fluid_error_timer = HAL_GetTick();
                        }
                    }                
                }
                else if(IsFANC_WaterLimitSetpointDifference())
                {
                    if(IsTemperatureRegulatorHeating())
                    {
                        if(fancoil_temp < ((int16_t) ((therm_temp_sp & 0x3fU) * 10)  - (therm_min_heating * 10)))
                        {
                            if((HAL_GetTick() - fancoil_fluid_error_timer) >= fancoil_fluid_error_time)
                            { 
                                if(IsFANC_HeatingErrorReportActiv()) 
                                {
                                    FANC_HeatingTempErrorSet();
                                }
                                
                                if(IsHeatingErrorShutdownActiv()) 
                                {
                                    TemperatureRegulatorOff();
                                } 
                                
                                fancoil_fluid_error_timer = HAL_GetTick();
                            }
                        }
                        else
                        {
                            fancoil_fluid_error_timer = HAL_GetTick();
                        }                        
                    }
                    else if(IsTemperatureRegulatorCooling()) 
                    {
                        if(fancoil_temp > ((int16_t) ((therm_temp_sp & 0x3fU) * 10) + (therm_max_cooling * 10)))
                        {
                            if((HAL_GetTick() - fancoil_fluid_error_timer) >= fancoil_fluid_error_time)
                            {
                                if(IsCoolingErrorReportActiv()) 
                                {
                                    FANC_CoolingTempErrorSet();
                                }
                                
                                if(IsCoolingErrorShutdownActiv()) 
                                {
                                    TemperatureRegulatorOff();
                                }
                                
                                fancoil_fluid_error_timer = HAL_GetTick();
                            }
                        }
                        else
                        {
                            fancoil_fluid_error_timer = HAL_GetTick();
                        }
                    }    
                }               
            }
            /** ============================================================================*/
            /**             T H E R M O S T A T         A U T O R E S T A R T               */
            /** ============================================================================*/
            else if(IsFANC_ErrorAutorestartActiv())
            {
                if((HAL_GetTick() - fancoil_fluid_error_timer) >= fancoil_fluid_error_time)
                {
                    if(IsTHSTAT_ControllerActiv())
                    {
                        if(IsHeatingErrorShutdownActiv()
                           || IsCoolingErrorShutdownActiv()
                           || IsFancoilNtcErrorShutdownActiv()
                           || IsRoomNtcErrorShutdownActiv())
                        {
                           TemperatureRegulatorOn(); 
                        }
                    }
                    
                    RoomNtcErrorShutdownReset();
                    FancoilNtcErrorShutdownReset();
                    HeatingErrorShutdownReset();
                    CoolingErrorShutdownReset();
                    fancoil_fluid_error_timer = HAL_GetTick();
                }
            }
        }
    }
    else 
    {
        fancoil_temp = 0;
    }
    /** ============================================================================*/
	/**		    C H E C K       O P E R A T I N G       C O N D I T I O N S   	    */
	/** ============================================================================*/
    if(IsTHSTAT_OperationModeOff())
    {
        if(IsTemperatureRegulatorOn())
        {
            TemperatureRegulatorOff();
            THSTAT_SaveSettings();
        }
    }
    else if(IsTHSTAT_OnecycleHeatingActiv())
    {
        return;
    }
    else if(IsTHSTAT_OperationModeCooling())
    {
        if(!IsTemperatureRegulatorOn() || !IsTemperatureRegulatorCooling())
        {
            TemperatureRegulatorOn();
            TemperatureRegulatorSetCooling();
            THSTAT_SaveSettings();
            ThermostatGuiSet();
        }
    }
    else if(IsTHSTAT_OperationModeHeating())
    {
        if(!IsTemperatureRegulatorOn() || !IsTemperatureRegulatorHeating())
        {
            TemperatureRegulatorOn();
            TemperatureRegulatorSetHeating();
            THSTAT_SaveSettings();
            ThermostatGuiSet();
        }
    }
    else if(IsTHSTAT_OperationModeRemote())
    {
        if(!IsExternalSwitchClosed() && ((flags & 0x01U) == 0U))     // allways start power cycle with thermostat disable 
        {
            flags |= 0x01U;
            TemperatureRegulatorOff();
            UserFaultSet();
        }
        else if(IsExternalSwitchClosed() && ((flags & 0x01U) != 0U))  // enable thermostat if external switch closed
        {
            flags &= 0xFEU;
            ThermostatGuiSet();
            
            if(IsTHSTAT_ControllerActiv())
            {
                TemperatureRegulatorOn();
            }
        }
        
        else if(IsHvacContactorOn() && ((flags & 0x02U) == 0U))       // enable thermostat if remote hvac contacter closed
        {
            flags |= 0x02U;
            ThermostatGuiSet();
            
            if(IsTHSTAT_ControllerActiv())
            {
                TemperatureRegulatorOn();
            }
        }
        else if(!IsHvacContactorOn() && ((flags & 0x02U) != 0U))       // disable thermostat if remote hvac contacter open
        {
            flags &= 0xFDU;
            TemperatureRegulatorOff();
            UserFaultSet();
        }
        else if(IsThermostatNewStateActiv())                             // enable thermostat on remote temperature setpoint update
        {
            ThermostatNewStateReset();
            
            if(IsTemperatureRegulatorOn()) 
            {
                THSTAT_ControllerOn();
                THSTAT_SaveSettings();
                ThermostatGuiSet();
            }
            else 
            {
                THSTAT_ControllerOff();
                THSTAT_SaveSettings();
                UserFaultSet();
            }
        }
    }
    /** ============================================================================*/
	/**				T E M P E R A T U R E		C O N T R O L L E R					*/
	/** ============================================================================*/
	if(!IsTemperatureRegulatorOn())
	{
        fan_pcnt = 0U;
		fan_speed = 0U;
        old_fan_speed = 0U;
        fan_calc_speed = 0U;
        /**
        *   set one of 3 currently supported type for fan rpm control 
        */
		if      (IsFANC_FanMotor3SpeedControl()) FANC_FanOff(); // seet to off state all 3 triac/relay outputs controlling 3 winding/speed electric fan
		else if (IsFANC_FanMotorTriacControl())  FANC_SetFanOff(); // shutdown phase control triac in P (power) regulator controlling rpm of fan
        else if (IsFANC_FanMotorBldcControl())   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fan_speed); // set to zero timer pwm output used as DAC for controle of BLDC resin pack motor 
        
        FANC_CoolingValveOff();
        FANC_CoolingValveReset();
        FANC_HeatingTempErrorReset();
        FANC_CoolingTempErrorReset();
        FANC_FreezingAlarmReset();
        FANC_RPM_SensorErrorReset();
        FancoilNtcSensorErrorReset();
        RoomNtcSensorErrorReset();
	}
	else if(IsTemperatureRegulatorOn())  
	{
        temp_sp =(int16_t) ((therm_temp_sp & 0x3FU) * 10);
        
        if(IsTemperatureRegulatorCooling())
        {
            if(IsFANC_FanMotorBldcControl())
            {
                if((HAL_GetTick() - bldc_fan_speed_control_timer) >= FAN_BLDC_MOTOR_CONTROL_LOOP_TIME)
                {
                    bldc_fan_speed_control_timer = HAL_GetTick();

                    if ((fan_speed == 0U) && (room_temp > (temp_sp + fan_speed_treshold)))   fan_speed = 1U; // only one is enough to start regulator procces
                    else if (room_temp <= temp_sp)                                           fan_speed = 0U; // stop if process variable reached set point
                    else if (fan_speed != 0U) // do math for given proportional band and differece to calculate output percentage 0 ~ 100%
                    {
                        if(room_temp > (temp_sp + fan_middle_speed_band)) fan_calc_speed = FAN_BLDC_MOTOR_MAX_SETPOINT;                        
                        else fan_calc_speed = (((room_temp - temp_sp) * 100) / fan_middle_speed_band);
                        
                        if (fan_speed < fan_calc_speed) ++fan_speed;
                        else if (fan_speed > fan_calc_speed) --fan_speed;
                        
                        if(fan_speed > FAN_BLDC_MOTOR_MAX_SETPOINT) fan_speed = FAN_BLDC_MOTOR_MAX_SETPOINT;
                        else if (fan_speed < fan_low_speed_band) fan_speed = fan_low_speed_band;
                    }
                }
            }
            else
            {
                if      ((fan_speed == 0U) && (room_temp > (temp_sp + fan_low_speed_band)))                                               fan_speed = 1U;
                else if ((fan_speed == 1U) && (room_temp > (temp_sp + fan_middle_speed_band)))                                            fan_speed = 2U;
                else if ((fan_speed == 1U) && (room_temp <= temp_sp))                                                                     fan_speed = 0U;
                else if ((fan_speed == 1U) && (room_temp <=(temp_sp + fan_speed_treshold)))                             
                {
                    if(!IsFANC_ValveFollowFanActiv())
                    {
                        FANC_CoolingValveReset();
                    }
                }
                else if ((fan_speed == 2U) && (room_temp > (temp_sp + fan_middle_speed_band + fan_low_speed_band)))                       fan_speed = 3U;
                else if ((fan_speed == 2U) && (room_temp <=(temp_sp + fan_middle_speed_band - fan_speed_treshold)))                       fan_speed = 1U; 
                else if ((fan_speed == 3U) && (room_temp <=(temp_sp + fan_middle_speed_band + fan_low_speed_band - fan_speed_treshold)))  fan_speed = 2U;                 
            }
        }
        else if(IsTemperatureRegulatorHeating())
        {
            if(IsFANC_FanMotorBldcControl())
            {
                if((HAL_GetTick() - bldc_fan_speed_control_timer) >= FAN_BLDC_MOTOR_CONTROL_LOOP_TIME)
                {
                    bldc_fan_speed_control_timer = HAL_GetTick();

                    if ((fan_speed == 0U) && (room_temp < (temp_sp - fan_speed_treshold)))   fan_speed = 1U;
                    else if (room_temp >= temp_sp)                                           fan_speed = 0U;
                    else if (fan_speed != 0U)
                    {
                        if(room_temp < (temp_sp - fan_middle_speed_band)) fan_calc_speed = FAN_BLDC_MOTOR_MAX_SETPOINT;                        
                        else fan_calc_speed = (((temp_sp - room_temp)* 100) / fan_middle_speed_band);
                        
                        if (fan_speed < fan_calc_speed) ++fan_speed;
                        else if (fan_speed > fan_calc_speed) --fan_speed;
                        
                        if(fan_speed > FAN_BLDC_MOTOR_MAX_SETPOINT) fan_speed = FAN_BLDC_MOTOR_MAX_SETPOINT;
                        else if (fan_speed < fan_low_speed_band) fan_speed = fan_low_speed_band;
                    }     
                }
            }
            else
            {
                if      ((fan_speed == 0U) && (room_temp < (temp_sp - fan_low_speed_band)))                                               fan_speed = 1U;
                else if ((fan_speed == 1U) && (room_temp < (temp_sp - fan_middle_speed_band)))                                            fan_speed = 2U;
                else if ((fan_speed == 1U) && (room_temp >= temp_sp))                                                                     fan_speed = 0U;
                else if ((fan_speed == 2U) && (room_temp < (temp_sp - fan_middle_speed_band - fan_low_speed_band)))                       fan_speed = 3U;
                else if ((fan_speed == 2U) && (room_temp >=(temp_sp - fan_middle_speed_band + fan_speed_treshold)))                       fan_speed = 1U; 
                else if ((fan_speed == 3U) && (room_temp >=(temp_sp - fan_middle_speed_band - fan_low_speed_band + fan_speed_treshold)))  fan_speed = 2U; 
            }        
        }
    }
    /** ============================================================================*/
	/**		S W I T C H		F A N		S P E E D		W I T H		D E L A Y		*/
	/** ============================================================================*/
	if (fan_speed != old_fan_speed) // don't let fan motor goes to smoke if we like to touch too much 
	{
        if((HAL_GetTick() - fancoil_fan_timer) >= FANC_FAN_MIN_ON_TIME)
        {
            if (IsFANC_FanMotorBldcControl())   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fan_speed);
            if(fan_pcnt > 1U)  fan_pcnt = 0U;
                
            if(fan_pcnt == 0U) 
            {
                if      (IsFANC_FanMotor3SpeedControl()) FANC_FanOff();
                else if (IsFANC_FanMotorTriacControl())  FANC_SetFanOff();
                
                if(old_fan_speed != 0U) fancoil_fan_timer = HAL_GetTick();
                ++fan_pcnt;
            }
            else if(fan_pcnt == 1U)
            {
                if(fan_speed == 1U) 
                {
                    if      (IsFANC_FanMotor3SpeedControl()) FANC_FanLowSpeedOn();
                    else if (IsFANC_FanMotorTriacControl())  FANC_SetFanLowSpeed();
                }
                else if(fan_speed == 2U) 
                {
                    if      (IsFANC_FanMotor3SpeedControl()) FANC_FanMiddleSpeedOn();
                    else if (IsFANC_FanMotorTriacControl())  FANC_SetFanMiddleSpeed();
                }
                else if(fan_speed == 3U) 
                {
                    if      (IsFANC_FanMotor3SpeedControl()) FANC_FanHighSpeedOn();
                    else if (IsFANC_FanMotorTriacControl())  FANC_SetFanHighSpeed();
                }
                
                if(fan_speed != 0U) 
                {
                    FANC_CoolingValveSet();
                    fancoil_fan_timer = HAL_GetTick();
                }
                else 
                {
                    if(IsFANC_ValveFollowFanActiv() || IsTemperatureRegulatorCooling())
                    {
                       FANC_CoolingValveReset(); 
                    }
                }
                
                old_fan_speed = fan_speed;
                ++fan_pcnt;
            }            
        }
	}
    /** ============================================================================*/
	/**		S W I T C H		E L E C T R O V A L V E     W I T H		D E L A Y       */
	/** ============================================================================*/
    if((HAL_GetTick() - fancoil_valve_timer) >= FANC_VALVE_MIN_ON_TIME)
    {
        fancoil_valve_timer = HAL_GetTick();
        
        if(IsFANC_CoolingValveActiv() && !IsFANC_CoolingValveOn())
        {
            FANC_CoolingValveOn();
        }       
        else if(!IsFANC_CoolingValveActiv() && IsFANC_CoolingValveOn())
        {
            FANC_CoolingValveOff();
        } 
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void THSTAT_SaveSettings(void)
{
    uint8_t ee_buff[4];
    
    ee_buff[0] = therm_temp_sp;
    if (EE_WriteData(EE_ADDR, EE_THERMO_SET_POINT, ee_buff, 1U) != HAL_OK) ErrorHandler(THSTAT_FUNC, I2C_DRV);
    if (EE_WaitStbySta() != HAL_OK)                                   ErrorHandler(THSTAT_FUNC, I2C_DRV);
    ee_buff[0] = ((therm_flags >> 24) & 0xFFU);
    ee_buff[1] = ((therm_flags >> 16) & 0xFFU);
    ee_buff[2] = ((therm_flags >>  8) & 0xFFU);
    ee_buff[3] = (therm_flags         & 0xFFU);
    if (EE_WriteData(EE_ADDR, EE_THERMO_FLAGS, ee_buff, 4U) != HAL_OK)     ErrorHandler(THSTAT_FUNC, I2C_DRV);
    if (EE_WaitStbySta() != HAL_OK)                                   ErrorHandler(THSTAT_FUNC, I2C_DRV);
}
/******************************   RAZLAZ SIJELA  ********************************/
