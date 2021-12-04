/**
 ******************************************************************************
 * File Name          : main.c
 * rtcdt               : 10.3.2018.
 * Description        : hotel room thermostat main function
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef ROOM_THERMOSTAT
    #error "room thermostat not selected for application in common.h"
#endif

#ifndef APPLICATION
    #error "application not selected for application type in common.h"
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
/* Constants ----------------------------------------------------------------*/
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
RTC_t date_time; 
RTC_TimeTypeDef rtctm;
RTC_DateTypeDef rtcdt;
RTC_HandleTypeDef hrtc;
CRC_HandleTypeDef hcrc;
ADC_HandleTypeDef hadc3;
TIM_HandleTypeDef htim9;
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef hi2c3;
SPI_HandleTypeDef hspi2;
IWDG_HandleTypeDef hiwdg;
QSPI_HandleTypeDef hqspi;
LTDC_HandleTypeDef hltdc;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart4;
DMA2D_HandleTypeDef hdma2d;
SIG_BUZZER_TypeDef BUZZ_State = BUZZ_RDY;
/* Private Define ------------------------------------------------------------*/
#define TOUCH_SCREEN_UPDATE_TIME			10U     // 50ms touch screen update period
#define TOUCH_SCREEN_LAYER                  1U      // touch screen layer event
#define AMBIENT_NTC_RREF                    10000U  // 10k NTC value of at 25 degrees
#define AMBIENT_NTC_B_VALUE                 3977U   // NTC beta parameter
#define AMBIENT_NTC_PULLUP                  10000U	// 10k pullup resistor
#define FANC_NTC_RREF                       2000U  	// 2k fancoil NTC value of at 25 degrees
#define FANC_NTC_B_VALUE                    3977U   // NTC beta parameter
#define FANC_NTC_PULLUP                     2200U	// 2k2 pullup resistor
#define ADC_READOUT_PERIOD                  234U    // 89 ms ntc conversion rate
#define FAN_CONTROL_LOOP_PERIOD			    200U    // fan speed control loop 
#define TRIAC_ON_PULSE                      5U      // 500 us triac fire pulse duration
/* Private Variable ----------------------------------------------------------*/
__IO uint32_t sysfl;
static uint32_t rstsrc = 0;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
void MX_IWDG_Init(void);
static void RAM_Init(void);
static void ADC3_Read(void);
static void MPU_Config(void);
static void MX_RTC_Init(void);
static void MX_CRC_Init(void);
static void SaveResetSrc(void);
static void CACHE_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM9_Init(void);
static void MX_ADC3_Init(void);
static void MX_UART_Init(void);
static void MX_CRC_DeInit(void);
static void MX_RTC_DeInit(void);
static void MX_TIM9_DeInit(void);
static void MX_I2C3_DeInit(void);
static void MX_I2C4_DeInit(void);
static void MX_ADC3_DeInit(void);
static void MX_GPIO_DeInit(void);
static void MX_UART_DeInit(void);
static void SystemClock_Config(void);
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data);
static float ROOM_GetTemperature(uint16_t adc_value);
//static void RTC_GetDateTimeFromUnix(RTC_t* data, uint32_t unix);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
int main(void)
{
    SysInitSet();
    SaveResetSrc();
	MPU_Config();
	CACHE_Config();
	HAL_Init(); 
	SystemClock_Config();
#ifdef	USE_WATCHDOG
	MX_IWDG_Init();
#endif     
    MX_CRC_Init();
	MX_RTC_Init();	    
	MX_ADC3_Init();
    MX_UART_Init();
	MX_TIM9_Init();
	MX_GPIO_Init();
	MX_QSPI_Init();
    QSPI_MemMapMode();
    SDRAM_Init();
	EE_Init();
    TS_Init();
    LOGGER_Init();
    RAM_Init();
    DISPInit();
    OW_Init();
    RS485_Init();
    ROOM_Init();
#ifdef	USE_WATCHDOG
    HAL_IWDG_Refresh(&hiwdg);
#endif

	while(1)
	{
        ADC3_Read();
        TS_Service();
        DIO_Service();
        ROOM_Service();
		DISPService();
        RS485_Service();
        OW_Service();
#ifdef	USE_WATCHDOG
        HAL_IWDG_Refresh(&hiwdg);
#endif
	}
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void Restart(void) 
{
    MX_GPIO_DeInit();
    MX_ADC3_DeInit();
    MX_I2C3_DeInit();
    MX_I2C4_DeInit();
    MX_TIM9_DeInit();
    MX_UART_DeInit();
    HAL_QSPI_DeInit(&hqspi);
    MX_RTC_DeInit();
    MX_CRC_DeInit();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SCB_DisableICache();
    SCB_DisableDCache();
    HAL_NVIC_SystemReset();
}
/**
  * @brief
  * @param
  * @retval
  */
void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32; //(1/(32000/32))*4095 = 4,095s
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        Restart();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void SIGNAL_Buzzer(void)
{
    static uint32_t signal_pcnt = 0x0U;
    static uint32_t signal_timer = 0x0U;
    static uint32_t signal_time = 0x0U;
    /************************************************/
	/*		B U Z Z E R			S I G N A L			*/
	/************************************************/
    if((HAL_GetTick() - signal_timer) >= signal_time)
    {
        signal_timer = HAL_GetTick();
        
        switch(BUZZ_State)
        {
            case BUZZ_OFF:
            {
                BuzzerOff();
                signal_timer = 0x0U;
                signal_time = 0x0U;
                signal_pcnt = 0x0U;
                BUZZ_State = BUZZ_RDY;
                break;
            } 
            
            
            case BUZZ_CARD_INVALID: //  3 short buzzer bips
            {       
                if(!IsBuzzerActiv()) BuzzerOn();
                else BuzzerOff();
                signal_time = 50U;
                ++signal_pcnt;
                if(signal_pcnt >= 0x5U)  BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            
            case BUZZ_CARD_VALID:   //  2 short buzzer bips
            {
                BuzzerOn();
                signal_time = 500U;
                BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            
            case BUZZ_DOOR_BELL:    //  1 short buzzer bips
            {    
                BuzzerOn();
                signal_time = 50U;
                BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            
            case BUZZ_CLEANING_END:
            {    
                BuzzerOn();
                signal_time = 700U;
                BUZZ_State = BUZZ_OFF; 
                break;
            } 
            
            
            case BUZZ_SOS_ALARM:
            {  
                BUZZ_State = BUZZ_OFF; 
                break;
            } 
            
            
            case BUZZ_FIRE_ALARM:
            { 
                if ((ROOM_Status != ROOM_FIRE_ALARM) && (ROOM_Status != ROOM_FIRE_EXIT))
                {
                    BUZZ_State = BUZZ_OFF;
                }
                else
                {
                    if(!IsBuzzerActiv()) 
                    {
                        BuzzerOn();
                        signal_time = 50U;
                    }
                    else 
                    {
                        BuzzerOff();
                        signal_time = 600U;
                    }
                }
                break;
            }  
            
            case BUZZ_CLICK:
            {
                BuzzerOn();
                signal_time = 5U;
                BUZZ_State = BUZZ_OFF;
                break;
            }
            
            case BUZZ_RDY:
            default:
            {
                BUZZ_State = BUZZ_OFF; 
                break;
            }
        }
        if (IsBuzzerActiv())HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);     // BuzzerOn() 
        else                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);   // BuzzerOff()
    }
    
}
/**
  * @brief
  * @param
  * @retval
  */
void TS_Service(void)
{
    uint16_t xDiff, yDiff;
    __IO TS_StateTypeDef  ts;    
    static GUI_PID_STATE TS_State = {0};
    static uint32_t ts_update_tmr = 0U;
    
    
    if (IsTSCleanActiv() || IsSysInitActiv()) return; // don't update touch event till room cleaning activ, 
    else if ((HAL_GetTick() - ts_update_tmr) >= TOUCH_SCREEN_UPDATE_TIME)
    {
        ts_update_tmr = HAL_GetTick();
        BSP_TS_GetState((TS_StateTypeDef *)&ts);
        
        if((ts.touchX[0] >= LCD_GetXSize()) ||
           (ts.touchY[0] >= LCD_GetYSize())) 
        {
            ts.touchX[0] = 0U;
            ts.touchY[0] = 0U;
            ts.touchDetected = 0U;
        }
        xDiff = (TS_State.x > ts.touchX[0]) ? (TS_State.x - ts.touchX[0]) : (ts.touchX[0] - TS_State.x);
        yDiff = (TS_State.y > ts.touchY[0]) ? (TS_State.y - ts.touchY[0]) : (ts.touchY[0] - TS_State.y);
        
        if((TS_State.Pressed != ts.touchDetected) || (xDiff > 30U) || (yDiff > 30U))
        {
            TS_State.Pressed = ts.touchDetected;
            TS_State.Layer = TOUCH_SCREEN_LAYER;
            
            if(ts.touchDetected) 
            {
                TS_State.x = ts.touchX[0];
                TS_State.y = ts.touchY[0];
                GUI_TOUCH_StoreStateEx(&TS_State);
            }
            else
            {
                GUI_TOUCH_StoreStateEx(&TS_State);
                TS_State.x = 0;
                TS_State.y = 0;
            }
        }        
    }

}
/**
  * @brief
  * @param
  * @retval
  */
void ErrorHandler(uint8_t function, uint8_t driver)
{
    LogEvent.log_type = driver;
    LogEvent.log_group = function;
    LogEvent.log_event = FUNC_OR_DRV_FAIL;
    if (driver != I2C_DRV) LOGGER_Write();	
    Restart();
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
void RTC_GetDateTime(RTC_t* data, uint32_t format) 
{
	uint32_t unix;

	/* Get rtctm */
	HAL_RTC_GetTime(&hrtc, &rtctm, format);
	/* Format hours */
	data->hours = rtctm.Hours;
	data->minutes = rtctm.Minutes;
	data->seconds = rtctm.Seconds;	
	/* Get subseconds */
	data->subseconds = RTC->SSR;	
	/* Get rtcdt */
	HAL_RTC_GetDate(&hrtc, &rtcdt, format);
	/* Format rtcdt */
	data->year = rtcdt.Year;
	data->month = rtcdt.Month;
	data->date = rtcdt.Date;
	data->day = rtcdt.WeekDay;	
	/* Calculate unix offset */
	unix = RTC_GetUnixTimeStamp(data);
	data->unix = unix;
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_RxCpltCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_RxCpltCallback();
    }
    else if (huart->Instance == UART4)
    {
        ++crsta;
        if (rxbuf[2] == '0') 
        {
            mem_cpy(sector_0.block_0, &rxbuf[4], BLOCK_SIZE);
            mem_cpy(sector_0.block_1, &rxbuf[20],BLOCK_SIZE);
            mem_cpy(sector_0.block_2, &rxbuf[36],BLOCK_SIZE);
            mem_cpy(sector_0.block_3, &rxbuf[52],BLOCK_SIZE);
            mem_cpy(buf0, &rxbuf[20], 32);
        }
        if (rxbuf[2] == '1') 
        {
            mem_cpy(sector_1.block_0, &rxbuf[4], BLOCK_SIZE);
            mem_cpy(sector_1.block_1, &rxbuf[20],BLOCK_SIZE);
            mem_cpy(sector_1.block_2, &rxbuf[36],BLOCK_SIZE);
            mem_cpy(sector_1.block_3, &rxbuf[52],BLOCK_SIZE);
        }
        if (rxbuf[2] == '2') 
        {
            mem_cpy(sector_2.block_0, &rxbuf[4], BLOCK_SIZE);
            mem_cpy(sector_2.block_1, &rxbuf[20],BLOCK_SIZE);
            mem_cpy(sector_2.block_2, &rxbuf[36],BLOCK_SIZE);
            mem_cpy(sector_2.block_3, &rxbuf[52],BLOCK_SIZE);
            CardDataRdySet();
        }
        HAL_UART_Receive_IT(&huart4, rxbuf, sizeof(rxbuf));
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_TxCpltCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_TxCpltCallback();
    }
    else if (huart->Instance == UART4)
    {
        
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_ErrorCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_ErrorCallback();
    }
    else if (huart->Instance == UART4)
    {
        ++crsta;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
	__HAL_RCC_RTC_ENABLE(); 
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
	__HAL_RCC_RTC_DISABLE();
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if (hspi->Instance == SPI2)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_SPI2_CLK_ENABLE();
        GPIO_InitStruct.Pin       = GPIO_PIN_14; // miso
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_13;  // sck
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_15; // mosi
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
//        HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
//        HAL_NVIC_EnableIRQ(SPI2_IRQn);
    }
}
/**
  * @brief SPI MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI2)
    {
        __HAL_RCC_SPI2_FORCE_RESET();
        __HAL_RCC_SPI2_RELEASE_RESET();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
        HAL_NVIC_DisableIRQ(SPI2_IRQn);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void RAM_Init(void)
{
    uint8_t ebuf[256];
    EE_ReadBuffer(ebuf, EE_THST_FLAGS, 256);    
    
    if (ebuf[EE_DEAULT_INIT] != 0xAA)  // default value not written
    {
        ZEROFILL(ebuf, COUNTOF(ebuf));
        sysfl = 0;
        TempRegOn();        
        ThermostatOn();
        TempRegEnable();
        TempRegHeating();     
        ShowUserInfoEnable();
        
        ebuf[EE_THST_FLAGS]   = (sysfl >> 24);    
        ebuf[EE_THST_FLAGS+1] = (sysfl >> 16);
        ebuf[EE_THST_FLAGS+2] = (sysfl >>  8);
        ebuf[EE_THST_FLAGS+3] = (sysfl & 0xff);
        ebuf[EE_MIN_SETPOINT] = 16;
        ebuf[EE_MAX_SETPOINT] = 32;
        ebuf[EE_THST_SETPOINT]= 20;
        ebuf[EE_DISP_LOW_BCKLGHT]  =  5;
        ebuf[EE_DISP_HIGH_BCKLGHT] = 70;
        ebuf[EE_SCRNSVR_TOUT] = 1;
        ebuf[EE_SCRNSVR_ENABLE_HOUR] = 22;
        ebuf[EE_SCRNSVR_DISABLE_HOUR]= 6;
        ebuf[EE_SCRNSVR_CLK_COLOR] = 15;
        ebuf[EE_SCRNSVR_SEMICLK_COLOR] = 15;
        ebuf[EE_ROOM_TEMP_SP]   = 20;
        ebuf[EE_ROOM_TEMP_DIFF] = 1;
        ebuf[EE_RSIFA]          = (DEF_NEW_RSIFA >> 8);
        ebuf[EE_RSIFA+1]        = (DEF_NEW_RSIFA & 0xff);
        ebuf[EE_RSGRA]          = (DEF_RC_RSGRA >> 8);
        ebuf[EE_RSGRA+1]        = (DEF_RC_RSGRA & 0xff);
        ebuf[EE_RSBRA]          = (DEF_RSBRA >> 8);
        ebuf[EE_RSBRA+1]        = (DEF_RSBRA & 0xff);
        ebuf[EE_RSBPS]          = 6;
        ebuf[EE_SYSID]          = (DEF_SYSID >> 8);
        ebuf[EE_SYSID+1]        = (DEF_SYSID & 0xff);
        ebuf[EE_MIFARE_KEYA]    = 0xff;
        ebuf[EE_MIFARE_KEYA+1]  = 0xff;
        ebuf[EE_MIFARE_KEYA+2]  = 0xff;
        ebuf[EE_MIFARE_KEYA+3]  = 0xff;
        ebuf[EE_MIFARE_KEYA+4]  = 0xff;
        ebuf[EE_MIFARE_KEYA+5]  = 0xff;
        ebuf[EE_MIFARE_KEYB]    = 0xff;
        ebuf[EE_MIFARE_KEYB+1]  = 0xff;
        ebuf[EE_MIFARE_KEYB+2]  = 0xff;
        ebuf[EE_MIFARE_KEYB+3]  = 0xff;
        ebuf[EE_MIFARE_KEYB+4]  = 0xff;
        ebuf[EE_MIFARE_KEYB+5]  = 0xff;
        ebuf[EE_USRGR_ADD1_ID] = USERGRP_GUEST;
        ebuf[EE_USRGR_ADD2_ID] = USERGRP_MAID;
        ebuf[EE_USRGR_ADD3_ID] = USERGRP_MANAGER;
        ebuf[EE_USRGR_ADD4_ID] = USERGRP_SERVICE;      
        ebuf[EE_OWIFA] = DEF_RC_OWIFA;
        ebuf[EE_OWGRA] = DEF_RC_OWGRA;
        ebuf[EE_OWBRA] = DEF_OWBRA;
        ebuf[EE_OWBPS]  = 2;
        ebuf[EE_OWADD1] = DEF_RT_OWIFA;
        ebuf[EE_OWADD2] = DEF_CS_OWIFA;
        ebuf[EE_OWADD3] = DEF_CR_OWIFA;
        ebuf[EE_DIN_CFG_ADD_1]  = 2;
        ebuf[EE_DIN_CFG_ADD_2]  = 5;
        ebuf[EE_DIN_CFG_ADD_3]  = 5;
        ebuf[EE_DIN_CFG_ADD_4]  = 5;
        ebuf[EE_DIN_CFG_ADD_5]  = 5;
        ebuf[EE_DIN_CFG_ADD_6]  = 2;
        ebuf[EE_DIN_CFG_ADD_7]  = 5;
        ebuf[EE_DIN_CFG_ADD_8]  = 5;
        ebuf[EE_DOUT_CFG_ADD_1] = 5;
        ebuf[EE_DOUT_CFG_ADD_2] = 5;
        ebuf[EE_DOUT_CFG_ADD_3] = 5;
        ebuf[EE_DOUT_CFG_ADD_4] = 5;
        ebuf[EE_DOUT_CFG_ADD_5] = 2;
        ebuf[EE_DOUT_CFG_ADD_6] = 5;
        ebuf[EE_DOUT_CFG_ADD_7] = 2;
        ebuf[EE_DOUT_CFG_ADD_8] = 2;
        ebuf[EE_MAID_PSWRD]     = (DEF_MAID_PSWRD >>16);
        ebuf[EE_MAID_PSWRD+1]   = (DEF_MAID_PSWRD >> 8);
        ebuf[EE_MAID_PSWRD+2]   = (DEF_MAID_PSWRD & 0xff);
        ebuf[EE_MNGR_PSWRD]     = (DEF_MNGR_PSWRD >>16);
        ebuf[EE_MNGR_PSWRD+1]   = (DEF_MNGR_PSWRD >> 8);
        ebuf[EE_MNGR_PSWRD+2]   = (DEF_MNGR_PSWRD & 0xff);
        ebuf[EE_SRVC_PSWRD]     = (DEF_SRVC_PSWRD >>16);
        ebuf[EE_SRVC_PSWRD+1]   = (DEF_SRVC_PSWRD >> 8);
        ebuf[EE_SRVC_PSWRD+2]   = (DEF_SRVC_PSWRD & 0xff);
        ebuf[EE_DEAULT_INIT]    = 0xAA;
        EE_WriteBuffer(ebuf, EE_THST_FLAGS, 256);
        ZEROFILL(ebuf, COUNTOF(ebuf));
        EE_ReadBuffer(ebuf, EE_THST_FLAGS, 256);
    }    
    sysfl  = ((ebuf[0]<<24)|(ebuf[1]<<16)|(ebuf[2]<<8)|ebuf[3]);            // restore system flags
    thst_min_sp         = ebuf[EE_MIN_SETPOINT];                            // EE_MIN_SETPOINT          0x4U
    thst_max_sp         = ebuf[EE_MAX_SETPOINT];                            // EE_MAX_SETPOINT          0x5U    
    ntc_offset          = ebuf[EE_NTC_OFFSET];                              // EE_NTC_OFFSET            0x7U
    disp_low_bcklght    = ebuf[EE_DISP_LOW_BCKLGHT];                        // EE_DISPLOW_BCKLGHT      0xAU
    disp_high_bcklght   = ebuf[EE_DISP_HIGH_BCKLGHT];                       // EE_DISPHIGH_BCKLGHT     0xBU
    scrnsvr_tout        = ebuf[EE_SCRNSVR_TOUT];                            // EE_SCRNSVR_TOUT          0xCU
    scrnsvr_ena_hour    = ebuf[EE_SCRNSVR_ENABLE_HOUR];                     // EE_SCRNSVR_ENABLE_HOUR   0xDU
    scrnsvr_dis_hour    = ebuf[EE_SCRNSVR_DISABLE_HOUR];                    // EE_SCRNSVR_DISABLE_HOUR  0xEU
    scrnsvr_clk_clr     = ebuf[EE_SCRNSVR_CLK_COLOR];                       // EE_SCRNSVR_CLK_COLOR     0xFU
    scrnsvr_semiclk_clr = ebuf[EE_SCRNSVR_SEMICLK_COLOR];                   // EE_SCRNSVR_SEMICLK_COLOR 0x10U   
    //sysfl               = ebuf[EE_SYS_STATE];
    ROOM_PreStatus      = (ROOM_StatusTypeDef)ebuf[EE_ROOM_PRESTAT_ADD];                        // EE_ROOM_PRESTAT_ADD      0x12U
    ROOM_Status         = (ROOM_StatusTypeDef)ebuf[EE_ROOM_STAT_ADD];                           // EE_ROOM_STAT_ADD         0x13U	// room status address
    thst_sp             = ebuf[EE_ROOM_TEMP_SP];                            // EE_THST_SETPOINT         0x6U
    thst_dif            = ebuf[EE_ROOM_TEMP_DIFF];                          // EE_ROOM_TEMP_DIFF        0x1BU	// room tempreature on / off difference
    
    rsifa[0]            = ebuf[EE_RSIFA];                                   // EE_RSIFA                 0x1CU	// rs485 device address
    rsifa[1]            = ebuf[EE_RSIFA+1];
    rsgra[0]            = ebuf[EE_RSGRA];                                   // EE_RSGRA                 0x1EU	// rs485 group broadcast address
    rsgra[1]            = ebuf[EE_RSGRA+1];
    rsbra[0]            = ebuf[EE_RSBRA];                                   // EE_RSBRA                 0x20U	// rs485 broadcast address msb
    rsbra[1]            = ebuf[EE_RSBRA+1];
    rsbps               = ebuf[EE_RSBPS];                                   // EE_RSBPS                 0x22U	// rs485 interface baudrate
    unix_room           = ((ebuf[0x24]<<24)|(ebuf[0x25]<<16)|(ebuf[0x26]<<8)|ebuf[0x27]);   //          0x24U	// room power expiry date time
    sysid[0]            = ebuf[EE_SYSID];                                   // EE_SYSID                 0x2AU	// system id (system unique number)
    sysid[1]            = ebuf[EE_SYSID+1];
    bdng_cnt            = ebuf[EE_BEDNG_CNT_ADD];                           // EE_BEDNG_CNT_ADD         0x2CU	// bedding replacement counter
    bdng_per            = ebuf[EE_BEDNG_REPL_ADD];                          // EE_BEDNG_REPL_ADD        0x2EU	// bedding replacement period
   
    mem_cpy (cardkeya, &ebuf[EE_MIFARE_KEYA],  6);                          // EE_MIFARE_KEYA           0x30U	// mifare access authentication key A
    mem_cpy (cardkeyb, &ebuf[EE_MIFARE_KEYB],  6);                          // EE_MIFARE_KEYB           0x36U	// mifare access authentication key B 
    owifa               = ebuf[EE_OWIFA];                                    // EE_OWIFA                 0x66U	// onewire interface address
    owgra               = ebuf[EE_OWGRA];                                    // EE_OWGRA                 0x67U	// onewire group address
    owbra               = ebuf[EE_OWBRA];                                    // EE_OWBRA                 0x68U	// onewire broadcast address
    owbps               = ebuf[EE_OWBPS];                                    // EE_OWBPS                 0x69U	// onewire interface baudrate
    mem_cpy (owadd, &ebuf[EE_OWADD1], 9U);                                  // EE_OWADD1                0x6AU	// onewire slave address 1 
    buzzer_volume       = ebuf[EE_BUZZER_VOLUME_ADD];                       // EE_BUZZER_VOLUME_ADD     0x75U   // buzzer volume address
    doorlock_force      = ebuf[EE_DOORLOCK_FORCE_ADD];                      // EE_DOORLOCK_FORCE_ADD    0x76U	// doorlock force address
    disp_rot            = ebuf[EE_DISP_ROTATION_ADD];                       // EE_DISPROTATION_ADD     0x77U	// display rotation address
    mem_cpy (din_cfg,  &ebuf[EE_DIN_CFG_ADD_1],  8);                        // EE_DIN_CFG_ADD_1         0x80U   // digital input 1 config
    mem_cpy (dout_cfg, &ebuf[EE_DOUT_CFG_ADD_1], 8);                        // EE_DOUT_CFG_ADD_1        0x88U   // digital output 1 config

    LogEvent.log_event = 0U;
    if      (IsSYS_UpdOkSet())
    {
        SYS_UpdOkReset();
        DISPFwUpdSet();
        LogEvent.log_event = FW_UPDATED;
    }
    else if (IsSYS_UpdFailSet())
    {
        SYS_UpdFailReset();
        DISPFwUpdFailSet();
        LogEvent.log_event = FW_UPD_FAIL;
    }    
    if (LogEvent.log_event) EE_WriteBuffer((uint8_t*)&sysfl, EE_SYS_STATE, 1);
	if (rstsrc) LogEvent.log_event = rstsrc;  // where  PIN_RESET is ((uint8_t)0xd0)
    if (LogEvent.log_event) LOGGER_Write();
}
/**
  * @brief
  * @param
  * @retval
  */
static void SaveResetSrc(void)
{
    if      (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))  rstsrc = LOW_POWER_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))   rstsrc = POWER_ON_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))   rstsrc = SOFTWARE_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))  rstsrc = IWDG_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))   rstsrc = PIN_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))  rstsrc = WWDG_RESET;
    else                                            rstsrc = 0U;
	 __HAL_RCC_CLEAR_RESET_FLAGS();
}
/**
  * @brief
  * @param
  * @retval
  */
static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x20010000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for Quad-SPI area to strongly ordered
	 This setting is essentially needed to avoid MCU blockings! 
	 See also STM Application Note AN4861 */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for the QSPI 64MB to normal memory Cacheable, must reflect the real memory size */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_16MB; // Set region size according to the QSPI memory size
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for SDRAM_Banks area to strongly ordered
	 This setting is essentially needed to avoid MCU blockings!
	 See also STM Application Note AN4861 */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for SDRAM 8MB to normal memory Cacheable */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
	
	/* Disable FMC bank1 (0x6000 0000 - 0x6FFF FFFF), since it is not used.
	This setting avoids unnedded speculative access to the first FMC bank.
	See also STM Application Note AN4861 */
	FMC_Bank1->BTCR[0] = 0x000030D2U;
}
/**
  * @brief
  * @param
  * @retval
  */
static void CACHE_Config(void)
{
//	SCB_EnableICache();
//	SCB_EnableDCache();	
    (*(uint32_t *) 0xE000ED94) &= ~0x5;
	(*(uint32_t *) 0xE000ED98) = 0x0; //MPU->RNR
	(*(uint32_t *) 0xE000ED9C) = 0x20010000 | 1 << 4; //MPU->RBAR
	(*(uint32_t *) 0xE000EDA0) = 0 << 28 | 3 << 24 | 0 << 19 | 0 << 18 | 1 << 17 | 0 << 16 | 0 << 8 | 30 << 1 | 1 << 0; //MPU->RASE  WT
	(*(uint32_t *) 0xE000ED94) = 0x5;

	SCB_InvalidateICache();

	/* Enable branch prediction */
	SCB->CCR |= (1 << 18);
	__DSB();

	SCB_EnableICache();

	SCB_InvalidateDCache();
	SCB_EnableDCache();
}
/**
  * @brief
  * @param
  * @retval
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    /** Configure LSE Drive Capability 
    */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
    /** Configure the main internal regulator output voltage 
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    /** Activate the Over-Drive mode 
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_I2C4;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 57;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
    PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
    PeriphClkInitStruct.PLLSAIDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    /**Initialize RTC Only 
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, RTC_DRV);
    }
    
    if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)
    {
        

        /**Initialize RTC and set the Time and Date 
        */
        sTime.Hours = 0x0U;
        sTime.Minutes = 0x0U;
        sTime.Seconds = 0x0U;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
        {
            ErrorHandler(MAIN_FUNC, RTC_DRV);
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 0x1U;
        sDate.Year = 0x0U;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
        {
            ErrorHandler(MAIN_FUNC, RTC_DRV);
        }

        HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_DeInit(void)
{
	HAL_RTC_DeInit(&hrtc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_Init(void)
{
	TIM_OC_InitTypeDef sConfigOC;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_TIM9_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 200U;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 1000U;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
	if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = DISP_BRGHT_MAX;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	
	if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}

	/**TIM9 GPIO Configuration    
    PE5     ------> TIM9_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_DeInit(void)
{
	__HAL_RCC_TIM9_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_5);
	HAL_TIM_PWM_DeInit(&htim9);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	
	/* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_UART4_CLK_ENABLE();
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX
    */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
   
    /**USART2 GPIO Configuration    
	PD5     ------> USART2_TX
	PD6     ------> USART2_RX 
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);    
    /**UART4 GPIO Configuration
    PA12    ------> USART1_DE 
    */    
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200U;
	huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) ErrorHandler (MAIN_FUNC, USART_DRV);
    
    
    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600U;
	huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler (MAIN_FUNC, USART_DRV);
    
    
    HAL_NVIC_SetPriority(UART4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
    huart4.Instance = UART4;
	huart4.Init.BaudRate = 115200U;
	huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
	huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart4.Init.OverSampling = UART_OVERSAMPLING_16;
	huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_HalfDuplex_Init(&huart4) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART_DeInit(void)
{
	__HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();
    __HAL_RCC_UART4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_NVIC_DisableIRQ(UART4_IRQn);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart4);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void MX_CRC_Init(void)
{
    hcrc.Instance                       = CRC;
    hcrc.Init.DefaultPolynomialUse      = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse       = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode    = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode   = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat                = CRC_INPUTDATA_FORMAT_BYTES;
    __HAL_RCC_CRC_CLK_ENABLE();
    
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, CRC_DRV);
    }
//    hcrc.Instance = CRC;
//    __HAL_RCC_CRC_CLK_ENABLE();
//    
//	if (HAL_CRC_Init(&hcrc) != HAL_OK)
//	{
//		ErrorHandler(MAIN_FUNC, CRC_DRV);
//	}
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void MX_CRC_DeInit(void)
{
    __HAL_RCC_CRC_CLK_DISABLE();
	HAL_CRC_DeInit(&hcrc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void ADC3_Read(void)
{
    ADC_ChannelConfTypeDef sConfig;
    uint32_t tmp, cnt;
    static float adc_calc;
    static uint32_t adc_tmr = 0;
    static uint32_t ambient_ntc_sample_cnt = 0;
    static uint16_t ambient_ntc_sample_value[10] = {0};
    
    if (IsRoomCtrlConfig())  return;
    else if ((HAL_GetTick() - adc_tmr) >= ADC_READOUT_PERIOD)
    {
        adc_tmr = HAL_GetTick();
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
        sConfig.Channel = ADC_CHANNEL_11;
        HAL_ADC_ConfigChannel(&hadc3, &sConfig);
        HAL_ADC_Start(&hadc3);
        HAL_ADC_PollForConversion(&hadc3, 10);
        ambient_ntc_sample_value[ambient_ntc_sample_cnt] = HAL_ADC_GetValue(&hadc3);
        if (++ambient_ntc_sample_cnt >  9) ambient_ntc_sample_cnt = 0;
        if (IsSysInitActiv()) return; // just fill average temp area an leave
        tmp = 0;
        for (cnt = 0; cnt < 10; cnt++) 
        {
            tmp += ambient_ntc_sample_value[cnt];
        }
        tmp = tmp/10;
        if((tmp < 100) || (tmp > 4000)) 
        {
            if (IsNtcValidActiv()) NtcErrorSet();
            NtcValidReset();
            room_temp = 0;
        }        
        else 
        {
            NtcValidSet();
            adc_calc = ROOM_GetTemperature(tmp);
            room_ntc_temp = adc_calc*10;
            if (room_temp != room_ntc_temp + ntc_offset)
            {
                room_temp = room_ntc_temp + ntc_offset;
                RoomTempUpdateSet();
            }
        }
    }
}
/**
  * @brief
        adc_cnt = 0U;
  * @param
  * @retval
  */
static void MX_ADC3_Init(void)
{
    ADC_ChannelConfTypeDef sConfig;
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_ADC3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.ScanConvMode = DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.NbrOfDiscConversion = 0U;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1U;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

	if(HAL_ADC_Init(&hadc3) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, ADC_DRV);
	}
	
	sConfig.Channel = ADC_CHANNEL_11;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	sConfig.Offset = 0U;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_ADC3_DeInit(void)
{
	__HAL_RCC_ADC3_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_3);
	HAL_ADC_DeInit(&hadc3);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C3_DeInit(void)
{
    __HAL_RCC_I2C3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);
    HAL_I2C_DeInit(&hi2c3);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C4_DeInit(void)
{
    __HAL_RCC_I2C4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12 | GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_2);
    HAL_I2C_DeInit(&hi2c4);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // nrf24 radio modul deselect
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET); // nrf24 radio modul disable

    GPIO_InitStruct.Pin = GPIO_PIN_12;  // SPI2_NSS
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_12;   // TRIAC_CTRL,DOOR_BELL
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_7;  // heating valve output, buzzer, status led
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
	GPIO_InitStruct.Pin = GPIO_PIN_3;  // NRF24L01_CE
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
//    GPIO_InitStruct.Pin = GPIO_PIN_2;               // FANCOIL_NTC
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;  // FANCOIL_ZEROCROSS_MAINS_AC
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;         // FANCOIL_MOTOR_RPM_HAL_SENSOR
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_DeInit(void)
{
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2|GPIO_PIN_4 |GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_3|GPIO_PIN_13|GPIO_PIN_14);
}
/**
  * @brief
  * @param
  * @retval
  */
static float ROOM_GetTemperature(uint16_t adc_value)
{
	float temperature;
    float ntc_resistance;

    ntc_resistance = (float) (AMBIENT_NTC_PULLUP * ((4095.0 / (4095 - adc_value)) - 1));
    temperature = ((AMBIENT_NTC_B_VALUE * 298.1) /  (AMBIENT_NTC_B_VALUE + (298.1 * log(ntc_resistance / AMBIENT_NTC_RREF))) -273.1);
    return(temperature);
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data) 
{
	uint32_t days = 0U, seconds = 0U;
	uint16_t i;
	uint16_t year = (uint16_t) (data->year + 2000U);
	
	/* Year is below offset year */
	if (year < UNIX_OFFSET_YEAR) 
	{
		return 0U;
	}	
	/* Days in back years */
	for (i = UNIX_OFFSET_YEAR; i < year; i++) 
	{
		days += DAYS_IN_YEAR(i);
	}	
	/* Days in current year */
	for (i = 1U; i < data->month; i++) 
	{
		days += rtc_months[LEAP_YEAR(year)][i - 1U];
	}	
	/* Day starts with 1 */
	days += data->date - 1U;
	seconds = days * SECONDS_PER_DAY;
	seconds += data->hours * SECONDS_PER_HOUR;
	seconds += data->minutes * SECONDS_PER_MINUTE;
	seconds += data->seconds;	
	return seconds;
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
