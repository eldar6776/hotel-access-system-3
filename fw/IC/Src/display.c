/**
 ******************************************************************************
 * File Name          : display.c
 * Date               : 17.2.2019
 * Description        : GUI Display Module
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#if (__DISPH__ != FW_BUILD)
    #error "display header version mismatch"
#endif
/* Include  ------------------------------------------------------------------*/
#include "png.h"
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
/* Private Define ------------------------------------------------------------*/
#define GUI_REFRESH_TIME                100U    // refresh gui 10 time in second
#define DATE_TIME_REFRESH_TIME          1000U   // refresh date & time info every 1 sec. 
#define SCRNSVR_TOUT                    10000U  // 10 sec timeout increment to set display in low brigntnes after last touch event
#define SETTINGS_MENU_ENABLE_TIME       3456U   // press and holde upper left corrner for period to enter setup menu
#define WFC_TOUT                        8765U   // 9 sec. weather display timeout   
#define BUTTON_RESPONSE_TIME            1234U   // button response delay till all onewire device update state
#define DISPIMG_TIME_MULTI              30000U  // 30 sec. min time increment for image display time * 255 max
#define SETTINGS_MENU_TIMEOUT           59000U  // 1 min. settings menu timeout
#define WFC_CHECK_TOUT                  (SECONDS_PER_HOUR * 1000U) // check weather forecast data validity every hour
#define KEYPAD_SIGNAL_TIME              3000
#define KEYPAD_UNLOCK_TIME              30000
#define DISPMSG_TIME                    45U     // time to display message on event * GUI_REFRESH_TIME
//eng   display image 1- 5
//ger   display image 11-15
//fra   display image 21-25
//arab  display image 31-35
//china display image 41-45
//jap   display image 51-55
//ita   display image 61-65
//tur   display image 71-75
//slov  display image 81-85

#define CLR_DARK_BLUE           GUI_MAKE_COLOR(0x613600)
#define CLR_LIGHT_BLUE          GUI_MAKE_COLOR(0xaa7d67)
#define CLR_BLUE                GUI_MAKE_COLOR(0x855a41)
#define CLR_LEMON               GUI_MAKE_COLOR(0x00d6d3)


#define BTN_SETTINGS_X0         0
#define BTN_SETTINGS_Y0         0
#define BTN_SETTINGS_X1         (BTN_SETTINGS_X0 + 120)
#define BTN_SETTINGS_Y1         (BTN_SETTINGS_Y0 + 89)

#define BTN_DEC_X0              0
#define BTN_DEC_Y0              90
#define BTN_DEC_X1              (BTN_DEC_X0 + 120)
#define BTN_DEC_Y1              (BTN_DEC_Y0 + 179)

#define BTN_DOOR_X0             0
#define BTN_DOOR_Y0             135
#define BTN_DOOR_X1             (BTN_DOOR_X0 + 200)
#define BTN_DOOR_Y1             (BTN_DOOR_Y0 + 134)

#define BTN_INC_X0              200
#define BTN_INC_Y0              90
#define BTN_INC_X1              (BTN_INC_X0 + 120)
#define BTN_INC_Y1              (BTN_INC_Y0 + 179)

#define BTN_KEYPAD_X0           320
#define BTN_KEYPAD_Y0           0
#define BTN_KEYPAD_X1           (BTN_KEYPAD_X0 + 149)
#define BTN_KEYPAD_Y1           (BTN_KEYPAD_Y0 + 135)

#define BTN_ROOM_X0             320
#define BTN_ROOM_Y0             135
#define BTN_ROOM_X1             (BTN_ROOM_X0 + 149)
#define BTN_ROOM_Y1             (BTN_ROOM_Y0 + 134)

#define BTN_DND_X0              320
#define BTN_DND_Y0              0
#define BTN_DND_X1              (BTN_DND_X0 + 149)
#define BTN_DND_Y1              (BTN_DND_Y0 + 89)

#define BTN_MAID_X0             320
#define BTN_MAID_Y0             90
#define BTN_MAID_X1             (BTN_MAID_X0 + 149)
#define BTN_MAID_Y1             (BTN_MAID_Y0 + 89)

#define BTN_SOS_X0              320
#define BTN_SOS_Y0              180
#define BTN_SOS_X1              (BTN_SOS_X0 + 149)
#define BTN_SOS_Y1              (BTN_SOS_Y0 + 89)

#define BTN_OK_X0               269
#define BTN_OK_Y0               135
#define BTN_OK_X1               (BTN_OK_X0  + 200)
#define BTN_OK_Y1               (BTN_OK_Y0  + 134)

#define SP_H_POS                200
#define SP_V_POS                150
#define CLOCK_H_POS             240
#define CLOCK_V_POS             136

#define GUI_ID_BUTTON_Dnd   			                    0x800
#define GUI_ID_BUTTON_Sos   			                    0x801
#define GUI_ID_BUTTON_Maid   			                    0x802
#define GUI_ID_BUTTON_Ok                                    0x803
#define GUI_ID_BUTTON_DoorOpen                              0x804
#define GUI_ID_BUTTON_Next                                  0x805
#define GUI_ID_BUTTON_Weather                               0x806    
#define GUI_ID_BTN_OSK_1   		                            0x80B
#define GUI_ID_BTN_OSK_2   		                            0x80C
#define GUI_ID_BTN_OSK_3   		                            0x80D
#define GUI_ID_BTN_OSK_4   		                            0x80E
#define GUI_ID_BTN_OSK_5   		                            0x80F
#define GUI_ID_BTN_OSK_6   		                            0x810
#define GUI_ID_BTN_OSK_7   		                            0x811
#define GUI_ID_BTN_OSK_8   		                            0x812
#define GUI_ID_BTN_OSK_9   		                            0x813
#define GUI_ID_BTN_OSK_0  		                            0x814
#define GUI_ID_BTN_OSK_OK   		                        0x815
#define GUI_ID_BTN_OSK_BK  		                            0x816
#define GUI_ID_BTN_LNGTRK                                   0x817
#define GUI_ID_BTN_LNGKSA                                   0x819
#define GUI_ID_BTN_LNGITA                                   0x81A
#define GUI_ID_BTN_LNGFRA                                   0x81B
#define GUI_ID_BTN_LNGENG                                   0x81C
#define GUI_ID_BTN_LNGGER                                   0x81D
#define GUI_ID_BTN_LNGCHN                                   0x81E
#define GUI_ID_BTN_LNGBIH                                   0x81F

#define GUI_ID_BTN_MAIDBELL                                 0x820
#define GUI_ID_BTN_MAIDCLR                                  0x821
#define GUI_ID_BTN_MAIDOPEN                                 0x822

#define GUI_ID_SPINBOX_AmbientNtcOffset                     0x830
#define GUI_ID_SPINBOX_MaxSetpoint                          0x832
#define GUI_ID_SPINBOX_MinSetpoint                          0x833
#define GUI_ID_SPINBOX_DisplayHighBrightness                0x834
#define GUI_ID_SPINBOX_DisplayLowBrightness                 0x835
#define GUI_ID_SPINBOX_ScrnsvrTimeout                       0x836
#define GUI_ID_SPINBOX_ScrnsvrEnableHour                    0x837
#define GUI_ID_SPINBOX_ScrnsvrDisableHour                   0x838
#define GUI_ID_SPINBOX_ScrnsvrClockColour                   0x839
#define GUI_ID_SPINBOX_ScrnsvrLogoClockColour               0x83A
#define GUI_ID_SPINBOX_Hour                                 0x83B
#define GUI_ID_SPINBOX_Minute                               0x83C
#define GUI_ID_SPINBOX_Day                                  0x83D
#define GUI_ID_SPINBOX_Month                                0x83E
#define GUI_ID_SPINBOX_Year                                 0x83F

#define GUI_ID_CHECK_Scrnsvr                                0x850
#define GUI_ID_CHECK_ScrnsvrClock                           0x851
#define GUI_ID_CHECK_ScrnsvrLogoClock                       0x852
#define GUI_ID_CHECK_RTcfg                                  0x853
#define GUI_ID_CHECK_RCcfg                                  0x854
/* Private Type --------------------------------------------------------------*/
BUTTON_Handle   hBUTTON_Dnd;
BUTTON_Handle   hBUTTON_SosReset;
BUTTON_Handle   hBUTTON_Maid;
BUTTON_Handle   hBUTTON_Increase;
BUTTON_Handle   hBUTTON_Decrease;
BUTTON_Handle   hBUTTON_Ok;
BUTTON_Handle   hBUTTON_DoorOpen;
BUTTON_Handle   hBUTTON_Next;
BUTTON_Handle   hBUTTON_Forecast;
BUTTON_Handle   hBUTTON_OSK_1;
BUTTON_Handle   hBUTTON_OSK_2;
BUTTON_Handle   hBUTTON_OSK_3;
BUTTON_Handle   hBUTTON_OSK_4;
BUTTON_Handle   hBUTTON_OSK_5;
BUTTON_Handle   hBUTTON_OSK_6;
BUTTON_Handle   hBUTTON_OSK_7;
BUTTON_Handle   hBUTTON_OSK_8;
BUTTON_Handle   hBUTTON_OSK_9;
BUTTON_Handle   hBUTTON_OSK_0;
BUTTON_Handle   hBUTTON_OSK_OK;
BUTTON_Handle   hBUTTON_OSK_BK;
BUTTON_Handle   hBUTTON_LNGTRK;
BUTTON_Handle   hBUTTON_LNGKSA;
BUTTON_Handle   hBUTTON_LNGITA;
BUTTON_Handle   hBUTTON_LNGFRA;
BUTTON_Handle   hBUTTON_LNGENG;
BUTTON_Handle   hBUTTON_LNGGER;
BUTTON_Handle   hBUTTON_LNGCHN;
BUTTON_Handle   hBUTTON_LNGBIH;

BUTTON_Handle   hBUTTON_MAIDBELL;
BUTTON_Handle   hBUTTON_MAIDCLR;
BUTTON_Handle   hBUTTON_MAIDOPEN;

SPINBOX_Handle  hSPNBX_AmbientNtcOffset;                    //  ambient measured temperature value manual offset
SPINBOX_Handle  hSPNBX_MaxSetpoint;                         //  set thermostat user maximum setpoint value
SPINBOX_Handle  hSPNBX_MinSetpoint;                         //  set thermostat user minimum setpoint value
SPINBOX_Handle  hSPNBX_DisplayHighBrightness;               //  lcd display backlight led brightness level for activ user interface (high level)
SPINBOX_Handle  hSPNBX_DisplayLowBrightness;                //  lcd display backlight led brightness level for activ screensaver (low level)
SPINBOX_Handle  hSPNBX_ScrnsvrTimeout;                      //  start screensaver (value x 10 s) after last touch event or disable screensaver for 0
SPINBOX_Handle  hSPNBX_ScrnsvrEnableHour;                   //  when to start display big digital clock for unused thermostat display 
SPINBOX_Handle  hSPNBX_ScrnsvrDisableHour;                  //  when to stop display big digital clock but to just dimm thermostat user interfaca
SPINBOX_Handle  hSPNBX_ScrnsvrClockColour;                  //  set colour for full display screensaver digital clock digits 
SPINBOX_Handle  hSPNBX_ScrnsvrSemiClkColor;                 //  set colour for user logo size and place embedded digital clock
SPINBOX_Handle  hSPNBX_Hour;
SPINBOX_Handle  hSPNBX_Minute;
SPINBOX_Handle  hSPNBX_Day;
SPINBOX_Handle  hSPNBX_Month;
SPINBOX_Handle  hSPNBX_Year;
CHECKBOX_Handle hCHKBX_RTcfg;                               //  room controller configuration selected
CHECKBOX_Handle hCHKBX_RCcfg;                               //  room thermostat configuration selected
CHECKBOX_Handle hCHKBX_ScrnsvrClock;                        //  select full display screensaver digital clock 
CHECKBOX_Handle hCHKBX_ScrnsvrLogoClock;                    //  select user logo size screensaver digital clock
GUI_HMEM hQR_Code;                                          // QR CODE

FORECAST_DayTypeDef FORECAST_Day[5] =                       //  weather forecast for 5 days
{
    { 1, 1, 0, 0, 0},
    { 2, 2, 0, 0, 0},
    { 3, 3, 0, 0, 0},
    { 4, 4, 0, 0, 0},
    { 5, 5, 0, 0, 0}
};


GUI_RECT ForecastFrame[5] =                                 //  weather forecast 5 day frames display position
{
	{  5, 120,  65, 260},
	{ 70, 120, 130, 260},
	{135, 120, 195, 260},
	{200, 120, 260, 260},
	{265, 120, 325, 260}
};


static char * _apDays[] =                                   //  weather forecast days string
{
	"0", 
    "MON", 
    "TUE", 
    "WED", 
    "THU", 
    "FRI", 
    "SAT", 
    "SUN"
};

static uint32_t clk_clrs[COLOR_BSIZE] =                     //  selectable screensaver clock colours
{
    GUI_GRAY,       
    GUI_RED,        
    GUI_BLUE,       
    GUI_GREEN,      
    GUI_CYAN,       
    GUI_MAGENTA,        
    GUI_YELLOW,     
    GUI_LIGHTGRAY,  
    GUI_LIGHTRED,   
    GUI_LIGHTBLUE,  
    GUI_LIGHTGREEN, 
    GUI_LIGHTCYAN,  
    GUI_LIGHTMAGENTA,  
    GUI_LIGHTYELLOW, 
    GUI_DARKGRAY,   
    GUI_DARKRED,    
    GUI_DARKBLUE,   
    GUI_DARKGREEN,  
    GUI_DARKCYAN,   
    GUI_DARKMAGENTA,    
    GUI_DARKYELLOW,
    GUI_WHITE,      
    GUI_BROWN,      
    GUI_ORANGE,     
    CLR_DARK_BLUE,  
    CLR_LIGHT_BLUE, 
    CLR_BLUE,           
    CLR_LEMON

};
typedef enum
{
    OSK_IDLE = 0,
    OSK_EDIT_IPADD,
    OSK_EDIT_GWADD,
    OSK_EDIT_SUBNET,
    OSK_EDIT_RS485_INTERFACE,
    OSK_EDIT_RS485_BROADCAST,
    OSK_EDIT_PASSWORD,
    OSK_ENTER_PASSWORD,
    OSK_EDIT_SYS_ID
    
}OSK_EditTypeDedf;

/* Private Variable ----------------------------------------------------------*/
__IO uint32_t dispfl, dispfl1;
uint32_t keypadtmr = 0, keypadtout = 0;
uint32_t disp_sreensvr_tmr = 0U;
uint8_t pswb = 5, old_min = 60U;
uint8_t oskb, oskip, btn_settings;
uint8_t btn_lngmenu, btn_lngmenu_state; 
uint8_t btn_keypad, btn_keypad_state;
uint8_t btn_ok_state, btn_opendoor_state;
uint8_t btn_dnd_state, btn_dnd_old_state;
uint8_t btn_sos_state, btn_sos_old_state; 
uint8_t btn_doorbell, btn_doorbell_state;
uint8_t btn_maid_state, btn_maid_old_state;
uint8_t btn_increase_state, btn_increase_old_state;
uint8_t btn_decrease_state, btn_decrease_old_state;
uint8_t disp_low_bcklght, disp_high_bcklght;
uint8_t disp_img_id, disp_img_time, last_img_id;
uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour, disp_rot;
uint8_t scrnsvr_tout, scrnsvr_clk_clr, scrnsvr_semiclk_clr;
int8_t wfc_buff[WFC_BSIZE] = {0};
uint8_t ipadd[4]  = {0}, snadd[4]    = {0}, gwadd[4]   = {0};
uint8_t iptxt[20] = {0}, sbnttxt[20] = {0}, gtwtxt[20] = {0};
char buf0[84] = {0}, buf1[84] = {0}, oskin[6] = {0};
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void DISPDateTime(void);
static void DISPKeypad(char key);
static void DISPSaveSettings(void);
static void DISPResetScrnsvr(void);
static void DISPCreateKeypad(void);
static void DISPDeleteKeypad(void);
static uint8_t DISPLoadWFC(int8_t* buff);
static uint8_t DISPSaveWFC(int8_t* buff);
static void DISPSetBrightnes(uint8_t val);
static void DISPTemperatureSetPoint(void);
static void DISPDrawScreen(uint8_t img_id);
static void DISPCreateMaidMenuScreen(void);
static void DISPDeleteMaidMenuScreen(void);
static void DISPCreateSettings1Screen(void);
static void DISPCreateSettings2Screen(void);
static void DISPDeleteSettings1Screen(void);
static void DISPDeleteSettings2Screen(void);
static void PID_Hook(GUI_PID_STATE* pState);
static uint8_t DISPMenuSettings(uint8_t btn);
static void DISPCreateLanguageSetScreen(void);
static void DISPDeleteLanguageSetScreen(void);
static void DISPCreateWFCScreen (int8_t* buff);
static void DISPInitWFCFrame(FORECAST_DayTypeDef* fday);
static void DISPCardMsg(const char *msg, GUI_COLOR colour);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void DISPInit(void)
{
    GUI_Init();
    GUI_PID_SetHook(PID_Hook);
    WM_MULTIBUF_Enable(1);
    GUI_UC_SetEncodeUTF8();
    if      (IsRoomCtrlConfig()) DISPGuiSet(60);
    else if (IsRoomThstConfig()) DISPGuiSet(0);
}
/**
  * @brief
  * @param
  * @retval
  */
void DISPService(void)
{
    static enum
    {
        DISPTHERMOSTAT  = ((uint8_t)0U),
        DISPFORECAST    = ((uint8_t)1U),
        DISPMESSAGE     = ((uint8_t)2U),
        DISPERROR       = ((uint8_t)3U),
        DISPKEYPAD      = ((uint8_t)4U),
        DISPMAIDMENU    = ((uint8_t)5U),
        DISPLANGUAGE    = ((uint8_t)6U),
        DISPROOMCTRL    = ((uint8_t)7U),       
        DISPSETTINGS_1  = ((uint8_t)8U),
        DISPSETTINGS_2  = ((uint8_t)9U)
        
    }DISPScreen = DISPTHERMOSTAT;
    
  
    static uint32_t thflag = 0U;
    static uint32_t disp_tmr = 0U;
    static uint32_t disp_rtc_tmr = 0U;
    static uint32_t disp_img_tmr = 0U;
    static uint32_t button_enable_tmr = 0U;
    static uint32_t wfc_tmr = 0U;
    static uint32_t display_time = 0U;
    static uint32_t refresh_tmr = 0;
    int cn,ct;
    /** ==========================================================================*/
	/**    D R A W     D I S P L A Y	G U I	O N	   T I M E R    E V E N T     */
	/** ==========================================================================*/
	if ((HAL_GetTick() - disp_tmr) >= GUI_REFRESH_TIME) // regular call for gui redraw, with forecast timer increment inside
	{
        GUI_Exec();
        if (display_time) 
        {
            --display_time;
            if (!display_time) DISPUpdateSet();
        }
        wfc_tmr += GUI_REFRESH_TIME;
        disp_tmr = HAL_GetTick();
	}
	else return;   
    /** ==========================================================================*/
	/**     C H E C K   N E W   W E A T H E R   F O R E C A S T   D A T A         */
	/** ==========================================================================*/
    if      (IsWFC_UpdateActiv()) // check for new received weather forecast
    {
        WFC_UpdateReset();
        BTNWfcDisable();
        
        if (DISPSaveWFC(wfc_buff) == 0U) // save new data to eeprom, function fail if any of data invalid
        {
            if (!IsWFCValidActiv()) // if data test passed, show weather button icon and enable forecast display
            {
                BTNWfcSet();
                BTNWfcEnable();
                WFC_ValidSet();
                BTNUpdSet();
            }
        }
    }
    else if (wfc_tmr > WFC_CHECK_TOUT) // check hourly is actual weather forecast data obsolate
    {
        wfc_tmr = 0U;
        
        if(DISPLoadWFC(wfc_buff) == 0U) // load forecast from eeprom, function fail if any of data invalid
        {
            if(!IsWFCValidActiv()) // if data test passed, show weather button icon and enable forecast display
            {
                BTNWfcSet();
                BTNWfcEnable();
                WFC_ValidSet();
                BTNUpdSet();
            }
        }
        else if(IsWFCValidActiv()) // if data test failed, disable forecast display and remove button icon
        {
            WFC_ValidReset();
            BTNWfcDisable();
        }
    }    
    /** ==========================================================================*/
    /**     R E D R A W    D I S P L A Y    O N    B U T T O N    U P D A T E     */
    /** ==========================================================================*/
    if (!DISPScreen && IsBTNUpdActiv()) DISPUpdateSet(); // reset screensaver and redraw buttons
    /** ==========================================================================*/
	/**             D I S P L A Y           S C R E E N S A V E R                 */
	/** ==========================================================================*/
    if ((DISPScreen == DISPTHERMOSTAT) && !IsScrnsvrActiv()) // display backlight dimmed after unused period timeout
    {   
        if((HAL_GetTick() - disp_sreensvr_tmr) >= (scrnsvr_tout * SCRNSVR_TOUT))
        {
            ScrnsvrSet();
            ScrnsvrInitReset();
            DISPSetBrightnes(disp_low_bcklght);
        }
    }
    /** ==========================================================================*/
    /**     C H E C K   S E T T I N G S   M E N U   H I D D E N   B U T T O N     */
    /** ==========================================================================*/
    if (DISPMenuSettings(btn_settings) == 1)   // function return 1 after continous call with 1 for preset time
    {
        if ((DISPScreen != DISPKEYPAD) && (DISPScreen < DISPSETTINGS_1))
        {
            oskb = 0;
            DISPKeypadSet();
            DISPRefreshSet();
            DISPCreateKeypad();
            DISPScreen = DISPKEYPAD;
            ZEROFILL(oskin, sizeof(oskin));
            keypadtout = KEYPAD_UNLOCK_TIME;
            keypadtmr = HAL_GetTick();
        }
    }
    /** ==========================================================================*/
	/**         N E W       D I S P L A Y       I M A G E       R E Q U E S T     */
	/** ==========================================================================*/
    if (IsDISPUpdateActiv() && (DISPScreen < DISPSETTINGS_1)) // display new gui screen 
    {
        DISPUpdateReset();
        DISPResetScrnsvr(); 
//        if (IsDISPKeypadActiv())   DISPDeleteKeypad();
//        if (IsDISPLanguageActiv()) DISPDeleteLanguageSetScreen();
        /************************************/
        /* SET ONEWIRE FLAGS AND SEND EVENT */
        /************************************/
        if ((disp_img_id == 0U) || (disp_img_id == 10U)) // two most used display requests: thermostat gui and external switch open user fault
        {
            if (IsBTNOkActiv() || IsBTNOpenActiv()) // to prevent update loop when display message confirmed with button
            {
                OWStatUpdSet();     // and inform one wire device for button state change
            }
        }
        /************************************/
        /*  DISPLAY     NEW     SCREEN      */
        /************************************/
        DISPDrawScreen(disp_img_id);        // redraw display layers
        /************************************/
        /*          SET     TIMERS          */
        /************************************/
        disp_img_tmr      = HAL_GetTick();    // displayed image time is in  x 30 sec. increments
        button_enable_tmr = HAL_GetTick();    // start minimal time delay to send new state to onewire device
        /************************************/
        /*      SET    NEW  SCREEN  ID      */
        /************************************/
        if      (disp_img_id == 0U)  DISPScreen = DISPTHERMOSTAT,   ScreenInitSet();
        else if (disp_img_id == 10U) DISPScreen = DISPERROR,        last_img_id = 10U;
        else if (disp_img_id == 20U) DISPScreen = DISPFORECAST,     DISPCreateWFCScreen(wfc_buff);
        else if (disp_img_id == 30U) DISPScreen = DISPSETTINGS_1,   DISPCreateSettings1Screen();
        else if (disp_img_id == 60U) DISPScreen = DISPROOMCTRL,     ScreenInitSet();
        else if (disp_img_id == 90U) DISPScreen = DISPMAIDMENU,     DISPCreateMaidMenuScreen();
        else                         DISPScreen = DISPMESSAGE,      ScreenInitSet();
    }
    /** ==========================================================================*/
	/**       S E R V I C E       U S E R       I N P U T       E V E N T S       */
	/** ==========================================================================*/
    if      (DISPScreen == DISPTHERMOSTAT) // thermostat user interface
	{
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        if  (IsScreenInitActiv())
        {
            ScreenInitReset();          // reset init enable flag
            GUI_MULTIBUF_BeginEx(1);
            GUI_PNG_Draw (&logo, logo_size, 20, 0);
            GUI_MULTIBUF_EndEx(1);
            DISPTemperatureSetPoint();  // show setpoint temperature  
            DISPDateTime();             // show clock time
            BTNUpdSet();                // enable button redraw
        }    
        /** ==========================================================================*/
        /**     C H E C K       U S E R     B U T T O N S    N E W       E V E N T    */
        /** ==========================================================================*/
        if ((HAL_GetTick() - button_enable_tmr) >= BUTTON_RESPONSE_TIME)
        {
            /************************************/
            /*      DND BUTTON STATE CHANGED    */
            /************************************/
            if      (btn_dnd_state && !btn_dnd_old_state)         
            {
                btn_dnd_old_state = 1U;
                if  (!IsBTNDndActiv()) 
                {
                    BTNDndSet();
                    BTNMaidReset();
                }
                else BTNDndReset();                
                OWStatUpdSet();
                BTNUpdSet();
            }
            else if (!btn_dnd_state && btn_dnd_old_state)  btn_dnd_old_state = 0U;
            /************************************/
            /*  CALLMAID BUTTON STATE CHANGED   */
            /************************************/        
            if      (btn_maid_state && !btn_maid_old_state)
            {
                btn_maid_old_state = 1U;
                if  (!IsBTNMaidActiv())
                {
                    BTNMaidSet();
                    BTNDndReset();
                }
                else BTNMaidReset();
                OWStatUpdSet();
                BTNUpdSet();
            }
            else if (!btn_maid_state && btn_maid_old_state) btn_maid_old_state = 0U;
            /************************************/
            /*      SOS BUTTON STATE CHANGED    */
            /************************************/ 
            if (btn_sos_state && IsBTNSosActiv())
            {
                BTNUpdSet();
                BTNSosReset();
                BUZZ_State = BUZZ_OFF;
                OWStatUpdSet();
            }
            /************************************/
            /*  WEATHER BUTTON STATE CHANGED    */
            /************************************/
            else if (IsWFCValidActiv() && btn_sos_state)
            {
                GUI_SelectLayer(0);
                GUI_SetBkColor(GUI_BLACK); 
                GUI_Clear();
                GUI_SelectLayer(1);
                GUI_SetBkColor(GUI_TRANSPARENT); 
                GUI_Clear();
                DISPGuiSet(20);
            }
        }
        /************************************/
        /*      SETPOINT  VALUE  INCREASED  */
        /************************************/ 
        if      ( btn_increase_state && !btn_increase_old_state)
		{
			btn_increase_old_state = 1U;            
            if (thst_sp < thst_max_sp) 
            {
                ++thst_sp;
                OWStatUpdSet();
                SetpointUpdateSet();
                BUZZ_State = BUZZ_CLICK;
            }
		}
		else if (!btn_increase_state &&  btn_increase_old_state) btn_increase_old_state = 0U;
        /************************************/
        /*      SETPOINT  VALUE  DECREASED  */
        /************************************/ 
        if      ( btn_decrease_state && !btn_decrease_old_state)
		{
			btn_decrease_old_state = 1U;            
            if (thst_sp > thst_min_sp) 
            {
                --thst_sp;
                OWStatUpdSet();
                SetpointUpdateSet();
                BUZZ_State = BUZZ_CLICK;
            }
		}
		else if (!btn_decrease_state &&  btn_decrease_old_state) btn_decrease_old_state = 0U;
        /** ==========================================================================*/
        /**   R E W R I T E   A N D   S A V E   N E W   S E T P O I N T   V A L U E   */
        /** ==========================================================================*/
        if (IsSetpointUpdateActiv()) // temperature setpoint changed online or gui
        {
            SetpointUpdateReset();
            if (!IsScrnsvrActiv()) DISPTemperatureSetPoint();
            EE_WriteBuffer(&thst_sp, EE_ROOM_TEMP_SP, 1);
        }
        //  ROOM TEMPERATURE
        if  (IsRoomTempUpdateActiv() && !IsScrnsvrActiv())
        {
            RoomTempUpdateReset();
            NtcUpdateReset();
            GUI_MULTIBUF_BeginEx(1);
            GUI_ClearRect(240, 220, 320, 270);
            GUI_GotoXY(240, 245);
            GUI_SetColor(GUI_YELLOW);
            GUI_SetFont(GUI_FONT_32_1);
            GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
            GUI_DispSDec(room_temp/10, 3);
            GUI_DispString("°c");
            GUI_MULTIBUF_EndEx(1);
        }   
        /** ==========================================================================*/
        /**         R E D R A W     R O M       S T A T U S      B U T T O N S        */
        /** ==========================================================================*/
        if (IsBTNUpdActiv())  
        {
            BTNUpdReset();            
            if      (IsBTNSosActiv())   BUZZ_State = BUZZ_DOOR_BELL;
            else                        BUZZ_State = BUZZ_CLICK;
            if      (IsBTNDndActiv())   GUI_DrawBitmap(&bmbtn_dnd_1,    BTN_DND_X0+10, BTN_DND_Y0);
            else                        GUI_DrawBitmap(&bmbtn_dnd_0,    BTN_DND_X0+10, BTN_DND_Y0);
            if      (IsBTNMaidActiv())  GUI_DrawBitmap(&bmbtn_maid_1,   BTN_MAID_X0+10,BTN_MAID_Y0);
            else                        GUI_DrawBitmap(&bmbtn_maid_0,   BTN_MAID_X0+10,BTN_MAID_Y0);
            if      (IsBTNSosActiv())   GUI_DrawBitmap(&bmbtn_rst_sos_1,BTN_SOS_X0+30, BTN_SOS_Y0);
            else if (IsBTNWfcActiv())   GUI_DrawBitmap(&bmbtn_weather,  BTN_SOS_X0+10, BTN_SOS_Y0);
            else                        GUI_ClearRect (BTN_SOS_X0, BTN_SOS_Y0, BTN_SOS_X1, BTN_SOS_Y1);
            button_enable_tmr = HAL_GetTick();
            DISPSaveSettings();
        }
        /** ==========================================================================*/
        /**             W R I T E    D A T E    &    T I M E    S C R E E N           */
        /** ==========================================================================*/
        if ((HAL_GetTick() - disp_rtc_tmr) >= DATE_TIME_REFRESH_TIME) // screansaver clock time update 
        {
            disp_rtc_tmr = HAL_GetTick();
            if(++refresh_tmr > 10) 
            {
                refresh_tmr = 0;
                if (!IsScrnsvrActiv()) RoomTempUpdateSet(); // refresh room temperature 
            }
            DISPDateTime();
        }
    }
    else if (DISPScreen == DISPROOMCTRL)
    {
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        if (IsScreenInitActiv() || IsDISPRefreshActiv())
        {
            DISPRefreshReset();  
            ScreenInitReset(); // reset init enable flag            
            if (IsTSCleanActiv())
            {
                DISPCardMsg("ROOM CLEANING IN PROGRESS...\n\r\n\rTOUCH SCREEN IS INACTIV\n\r\n\rUSE ONLY RFID CARD !!!", GUI_YELLOW);
                return;
            }
            GUI_MULTIBUF_BeginEx(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_Clear();
            GUI_PNG_Draw (&logo,    logo_size,     20,   0);
            GUI_PNG_Draw (&keypad,  keypad_size,  350,   5);
            if      (IsDISPCleanUpImgActiv())   GUI_PNG_Draw (&cleaning, cleaning_size,-15, 150);
            else if (IsDISPBeddRepImgActiv())   GUI_PNG_Draw (&bedding,  bedding_size,  20, 150);
            else if (IsDISPGenCleanImgActiv())  GUI_PNG_Draw (&general,  general_size, -10, 150);
            if      (IsBTNDndActiv())   GUI_PNG_Draw (&dnd,  dnd_size,BTN_ROOM_X0+30, BTN_ROOM_Y0);
            else if (IsBTNMaidActiv())  GUI_PNG_Draw (&cleaning, cleaning_size, BTN_ROOM_X0+30, BTN_ROOM_Y0+10);
            else if (IsBTNBellActiv())  GUI_PNG_Draw (&doorbell, doorbell_size, BTN_ROOM_X0+30, BTN_ROOM_Y0);
            GUI_SetColor(GUI_WHITE);
            GUI_SetFont(GUI_FONT_D80);
            GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
            GUI_GotoXY (200, 140);
            GUI_DispDecMin((rsifa[0]<<8)|rsifa[1]);
            GUI_GotoXY (200, 190);
            GUI_SetTextAlign(GUI_TA_HCENTER);
            GUI_SetFont(GUI_FONT_32_1);
            GUI_DispString("APARTMENT");
            GUI_MULTIBUF_EndEx(1);
            old_min = 60;           // force drawing clock
            disp_rtc_tmr = 0;       // force drawing clock
            RoomTempUpdateSet();    // refresh room temperature            
        }
        /*************************************************************************/
        /**             U S E R         C A R D         E V E N T               **/
        /*************************************************************************/  
        if      (IsDISPCardValidImgActiv()) 
        {
            DISPCardValidImgDel();
            if      (sCard.user_group == USERGRP_MANAGER)   DISPCardMsg("MANAGER\n\r\n\rCARD VALID", GUI_GREEN);
            else if (sCard.user_group == USERGRP_SERVICE)   DISPCardMsg("SERVICE\n\r\n\rCARD VALID", GUI_GREEN);
            else if (sCard.user_group == USERGRP_MAID)      DISPCardMsg("MAID\n\r\n\rCARD VALID", GUI_GREEN);
            else if (sCard.user_group == USERGRP_GUEST)     DISPCardMsg("GUEST\n\r\n\rCARD VALID", GUI_GREEN);
            else return;
            display_time = DISPMSG_TIME;
            
            if (IsShowUserInfoEnabled())
            {
                cn = 0;
                ct = 0;
                do
                {
                    if (ISLETTER(buf0[cn])) 
                    {
                        buf1[ct] = buf0[cn];
                        ++ct;
                    }                    
                    ++cn;
                }
                while(cn < 32);
                buf1[ct] = '\0';
                GUI_MULTIBUF_BeginEx(1);
                GUI_SetColor(GUI_YELLOW);
                GUI_SetFont(GUI_FONT_32_1);
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(235, 180);
                GUI_DispString(buf1);
                GUI_MULTIBUF_EndEx(1);
            }
        }
        else if (IsDISPCardInvalidImgActiv()) 
        {
            DISPCardInvalidImgDel();
            DISPCardMsg("CARD INVALID", GUI_RED);
            display_time = DISPMSG_TIME;
        }
        else if (IsDISPWrongRoomImgActiv()) 
        {
            DISPWrongRoomImgDel();            
            DISPCardMsg("WRONG ROOM NUMBER", GUI_RED);
            display_time = DISPMSG_TIME;
        }
        else if (IsDISPTimeExpiredImgActiv()) 
        {
            DISPTimeExpiredImgDel();
            DISPCardMsg("CARD TIME IS EXPIRED\n\rPLEASE CONTACT\n\rTHE FRONT DESK", GUI_YELLOW);
            display_time = DISPMSG_TIME;
        }
        else if (IsDISPOutOfSrvcImgActiv()) 
        {
            DISPOutOfSrvcImgReset();
            DISPCardMsg("ROOM IS OUT OF SERVICE", GUI_RED);
            display_time = DISPMSG_TIME;
        }
        else if (IsDISPDoorOpenActiv()) 
        {
            DISPDoorOpenReset();
            DISPCardMsg("THE DOOR IS OPEN\n\r\n\rPLEASE COME IN", GUI_GREEN);
            display_time = DISPMSG_TIME;
        }   
        if (IsTSCleanActiv() || display_time) return; // wait for message timeout
        /** ==========================================================================*/
        /**             W R I T E    D A T E    &    T I M E    S C R E E N           */
        /** ==========================================================================*/
        if ((HAL_GetTick() - disp_rtc_tmr) >= DATE_TIME_REFRESH_TIME) // screansaver clock time update 
        {
            disp_rtc_tmr = HAL_GetTick();
            DISPDateTime();
        }
        /** ==========================================================================*/
        /**     C H E C K       U S E R     B U T T O N S    N E W       E V E N T    */
        /** ==========================================================================*/
        if ((HAL_GetTick() - button_enable_tmr) >= BUTTON_RESPONSE_TIME)
        {
            /************************************/
            /*    BUTTON KEYPAD STATE CHANGED   */
            /************************************/
            if      (btn_keypad && !btn_keypad_state)         
            {
                btn_keypad_state = 1;
                if(DISPScreen != DISPKEYPAD)
                {
                    oskb = 0;
                    DISPKeypadSet();
                    DISPRefreshSet();
                    DISPUnlockReset();
                    DISPCreateKeypad();
                    DISPScreen = DISPKEYPAD;
                    ZEROFILL(oskin, sizeof(oskin));
                    keypadtout = KEYPAD_UNLOCK_TIME;
                    keypadtmr = HAL_GetTick();
                }
            }
            else if (!btn_keypad)  btn_keypad_state = 0;
            /************************************/
            /*   BUTTON DOORBELL STATE CHANGED  */
            /************************************/
            if      (btn_doorbell && IsBTNBellActiv() && !btn_doorbell_state)       
            {
                btn_doorbell_state = 1;
                BUZZ_State = BUZZ_DOOR_BELL;
			    LogEvent.log_event = DOOR_BELL_ACTIVE;
			    LOGGER_Write();
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
                HAL_Delay(500);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
                DoorBellOn();
                OWStatUpdSet();
                while (IsDoorBellActiv()) OW_Service();
            }
            else if (!btn_doorbell) btn_doorbell_state = 0;
        }
        /*************************************************************************/
        /**         E V E N T   =   D O   N O T  D I S T U R B   A C T I V      **/
        /*************************************************************************/
        /*  Do Not Disturb Button   */    
        if      (!IsBTNDndActiv() && IsBTNDndEnabled())
        {
            BTNDndSet();
            BTNMaidDisable();
            DISPRefreshSet();
            LogEvent.log_event = DO_NOT_DISTURB_SWITCH_ON;
            LOGGER_Write();
        }
        else if (IsBTNDndActiv() && (!IsBTNDndEnabled() || IsBTNMaidEnabled()))
        {
            BTNDndReset();
            DISPRefreshSet();
            LogEvent.log_event = DO_NOT_DISTURB_SWITCH_OFF;
            LOGGER_Write();
        }
        /*  Call Maid Button */
        if      (!IsBTNMaidActiv() && IsBTNMaidEnabled())
        {
            BTNMaidSet();
            BTNDndDisable();
            DISPRefreshSet();
            LogEvent.log_event = HANDMAID_SWITCH_ON;
            LOGGER_Write();
        }
        else if (IsBTNMaidActiv() && (!IsBTNMaidEnabled() || IsBTNDndEnabled()))
        {
            BTNMaidReset();
            DISPRefreshSet();
            LogEvent.log_event = HANDMAID_SWITCH_OFF;
            LOGGER_Write();
        }
        /*  Door Bell Button    */
        if      (!IsBTNBellActiv() && IsBTNBellEnabled() && !IsBTNDndActiv() && !IsBTNMaidActiv())
        {
            BTNBellSet();
            DISPRefreshSet();
        }
        else if (IsBTNBellActiv() && (!IsBTNBellEnabled() || IsBTNDndActiv() || IsBTNMaidActiv()))
        {
            BTNBellReset();
            DISPRefreshSet();
        }             
        /** ==========================================================================*/
        /**         D I S P L A Y       R O O M         T E M P E R A T U R E         */
        /** ==========================================================================*/
        if (IsRoomTempUpdateActiv())
        {
            RoomTempUpdateReset();
            NtcUpdateReset();
            GUI_GotoXY(210, 245);
            GUI_SetColor(GUI_YELLOW);
            GUI_SetFont(GUI_FONT_32_1);
            GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
            GUI_MULTIBUF_BeginEx(1);
            GUI_ClearRect(210, 220, 320, 270);
            GUI_DispSDec(room_temp/10, 3);
            GUI_DispString("°C");
            GUI_MULTIBUF_EndEx(1);
        }
    }
    else if (DISPScreen == DISPKEYPAD)
    {
        if ((HAL_GetTick() - keypadtmr) >= keypadtout)
        {
            if      (IsDISPSetInitActiv())  DISPGuiSet(30), DISPSettingsInitReset();
            else if (IsDISPMaidMenuActiv()) DISPGuiSet(90), DISPMaidMenuReset();
            else if (last_img_id == 10)     DISPGuiSet(10); 
            else if (IsRoomThstConfig())    DISPGuiSet(0);
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);
            DISPDeleteKeypad();
            disp_tmr = 0;
        }
        else
        {
            // BUTTON 0
            if      (BUTTON_IsPressed(hBUTTON_OSK_0)) DISPKeypad('0');
            else if (BUTTON_IsPressed(hBUTTON_OSK_1)) DISPKeypad('1');
            else if (BUTTON_IsPressed(hBUTTON_OSK_2)) DISPKeypad('2');
            else if (BUTTON_IsPressed(hBUTTON_OSK_3)) DISPKeypad('3');
            else if (BUTTON_IsPressed(hBUTTON_OSK_4)) DISPKeypad('4');
            else if (BUTTON_IsPressed(hBUTTON_OSK_5)) DISPKeypad('5');
            else if (BUTTON_IsPressed(hBUTTON_OSK_6)) DISPKeypad('6');
            else if (BUTTON_IsPressed(hBUTTON_OSK_7)) DISPKeypad('7');
            else if (BUTTON_IsPressed(hBUTTON_OSK_8)) DISPKeypad('8');
            else if (BUTTON_IsPressed(hBUTTON_OSK_9)) DISPKeypad('9');
            else if (BUTTON_IsPressed(hBUTTON_OSK_OK))DISPKeypad('O');
            else if (BUTTON_IsPressed(hBUTTON_OSK_BK))DISPKeypad('B');
            while (GUI_PID_IsPressed()) TS_Service();
        }        
    }
    else if (DISPScreen == DISPMAIDMENU)
    {
        if  (BUTTON_IsPressed(hBUTTON_MAIDBELL))
        {
            /************************************/
            /*   BUTTON DOORBELL STATE CHANGED  */
            /************************************/
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
            HAL_Delay(500);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
            DoorBellOn();
            OWStatUpdSet();            
            BUZZ_State = BUZZ_DOOR_BELL;
            while (IsDoorBellActiv()) OW_Service();
            LogEvent.log_event = DOOR_BELL_ACTIVE;
            LOGGER_Write();
            keypadtout = 0;
        }
        else if (BUTTON_IsPressed(hBUTTON_MAIDCLR) 
        &&      (IsDISPCleanUpImgActiv() || IsDISPBeddRepImgActiv() || IsDISPGenCleanImgActiv())) 
        {
            TSCleanReset(); // enable touch screen on room device
            ROOM_Status = ROOM_IDLE; // clear room conter and power timer            
            BUZZ_State = BUZZ_CLEANING_END;
            LogEvent.log_event = HANDMAID_SERVICE_END;
            LOGGER_Write();
            keypadtout = 0;
        }
        else if (BUTTON_IsPressed(hBUTTON_MAIDOPEN) && !IsBTNBellActiv() && !IsBTNDndActiv())
        {  
            CARD_Clear();
            DoorLockOn();
            BUZZ_State = BUZZ_DOOR_BELL;
            LogEvent.log_event = HANDMAID_CARD;;
            LOGGER_Write();
            DooLockTimeSet();
            DISPDoorOpenSet();
            keypadtout = 0;
        }                
       
        if ((HAL_GetTick() - keypadtmr) >= keypadtout)
        {
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
            DISPDeleteMaidMenuScreen();
        }    
    }
    else if (DISPScreen == DISPLANGUAGE)
    {     
        if (BUTTON_IsPressed(hBUTTON_LNGTRK)
        ||  BUTTON_IsPressed(hBUTTON_LNGKSA) 
        ||  BUTTON_IsPressed(hBUTTON_LNGITA)
        ||  BUTTON_IsPressed(hBUTTON_LNGFRA) 
        ||  BUTTON_IsPressed(hBUTTON_LNGENG)
        ||  BUTTON_IsPressed(hBUTTON_LNGGER) 
        ||  BUTTON_IsPressed(hBUTTON_LNGCHN)
        ||  BUTTON_IsPressed(hBUTTON_LNGBIH) 
        || ((HAL_GetTick() - keypadtmr) >= keypadtout))
        {
            if (IsDISPRefreshActiv())
            {
                DISPRefreshReset();
                DISPLanguageReset();
                DISPDeleteLanguageSetScreen();
//                if      (IsDISPKeypadActiv())   DISPKeypadReset(),  DISPDeleteKeypad();
                if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
                else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
                else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
            }
        }
    }
    else if (DISPScreen == DISPFORECAST)   // weather forecast screen
    {
        if ((HAL_GetTick() - disp_img_tmr) >= WFC_TOUT) // exit screen on timeout
        {
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
        }
    }
    else if (DISPScreen == DISPMESSAGE)    // room guest display message
	{
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        if  (IsScreenInitActiv()) // insert user logo
        {
            ScreenInitReset();          // reset init enable flag
            GUI_MULTIBUF_BeginEx(1);
            GUI_PNG_Draw (&logo, logo_size, 20, 0);
            GUI_MULTIBUF_EndEx(1);     
        }
        /** ==========================================================================*/
        /**     C H E C K       U S E R     B U T T O N S    N E W       E V E N T    */
        /** ==========================================================================*/
        if ((HAL_GetTick() - button_enable_tmr) >= BUTTON_RESPONSE_TIME) // enable user button after small delay
        {
            /************************************/
            /*      CHECK BUTTON OK STATE       */
            /************************************/
            if(btn_ok_state) // if enbled button to quit message
            {
                if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
                else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
                else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
                BUZZ_State =  BUZZ_CLICK;       // buzzer is set after display macro, or will be oweritten by macro call
                BTNOkSet();                          // rise flag for interrupt driven onewire communication to send this event
                OWStatUpdSet();
                BTNUpdSet();
            }
            /************************************/
            /*    CHECK BUTTON OPENDOOR STATE   */
            /************************************/
            if(btn_opendoor_state != 0U) // if enabled button to activate door lock and quit message
            {
                if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
                else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
                else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
                BUZZ_State =  BUZZ_CLICK;  // buzzer is set after display macro, or will be oweritten by macro call
                BTNOpenSet(); // rise flag for interrupt driven onewire communication to send this event
                OWStatUpdSet();
//                BTNUpdSet();
            }
        }
        /************************************/
        /*      CHECK FOR MESSAGE TIMEOUT   */
        /************************************/
        if (disp_img_time != 0U) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            if((HAL_GetTick() - disp_img_tmr) >= (disp_img_time * DISPIMG_TIME_MULTI))
            {
                if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
                else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
                else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
                BUZZ_State =  BUZZ_CLICK; // buzzer set after display set macro, or will be oweritten by macro call
                BTNOkReset();
            }
        }
        
        if ((HAL_GetTick() - refresh_tmr) >= 5000)
        {
            refresh_tmr = HAL_GetTick();
            BUZZ_State = BUZZ_DOOR_BELL;
        }
    }    
    else if (DISPScreen == DISPERROR)      // display error message to inform user about misuse and conditions
	{
        /************************************/
        /*      CHECK FOR MESSAGE TIMEOUT   */
        /************************************/
        if (disp_img_time) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            if((HAL_GetTick() - disp_img_tmr) >= (disp_img_time * DISPIMG_TIME_MULTI))
            {
                if      (IsRoomThstConfig())   DISPGuiSet(0);     // room trhemostat gui
                else if (IsRoomCtrlConfig())   DISPGuiSet(60);    // room trhemostat gui
            }
        }
    }
    else if (DISPScreen == DISPSETTINGS_1) // setup menu 1
    {
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        thst_max_sp     = SPINBOX_GetValue(hSPNBX_MaxSetpoint);
        thst_min_sp     = SPINBOX_GetValue(hSPNBX_MinSetpoint);
        ntc_offset      = SPINBOX_GetValue(hSPNBX_AmbientNtcOffset);
        if      ((CHECKBOX_GetState(hCHKBX_RTcfg) == 1) && IsRoomCtrlConfig()) RoomThstConfig(), CHECKBOX_SetState(hCHKBX_RCcfg, 0);
        else if  (CHECKBOX_GetState(hCHKBX_RTcfg) == 0) RoomCtrlConfig(), CHECKBOX_SetState(hCHKBX_RCcfg, 1); 
        else if ((CHECKBOX_GetState(hCHKBX_RCcfg) == 1) && IsRoomThstConfig()) RoomCtrlConfig(), CHECKBOX_SetState(hCHKBX_RTcfg, 0); 
        else if  (CHECKBOX_GetState(hCHKBX_RCcfg) == 0) RoomThstConfig(), CHECKBOX_SetState(hCHKBX_RTcfg, 1); 
        /************************************/
        /*    ROOM TEMPERATURE NTC SENSOR   */
        /************************************/
        if      (!IsNtcValidActiv())
        {
            if(!IsNtcUpdateActiv())
            {
                NtcUpdateSet();
                GUI_ClearRect(10, 220, 200, 110);
                GUI_GotoXY(20, 240);
                GUI_SetColor(GUI_YELLOW);
                GUI_SetFont(GUI_FONT_24_1);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_DispString("ERROR");                
            }
        }
        else if (IsRoomTempUpdateActiv())
        {
            RoomTempUpdateReset();
            NtcUpdateReset();
            GUI_ClearRect(10, 220, 200, 260);
            GUI_GotoXY(20, 240);
            GUI_SetColor(GUI_YELLOW);
            GUI_SetFont(GUI_FONT_24_1);
            GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
            GUI_DispSDecShift(room_temp, 5, 1);
            GUI_DispString("°C");
        }
        /************************************/
        /*      CHECK BUTTON OK STATE       */
        /************************************/
        if      (BUTTON_IsPressed(hBUTTON_Ok)) 
        {
            DISPSaveSettings();
            DISPDeleteSettings1Screen();
            DISPScreen = DISPTHERMOSTAT;
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
        }
        /************************************/
        /*      CHECK BUTTON NEXT STATE     */
        /************************************/
        else if (BUTTON_IsPressed(hBUTTON_Next)) 
        {
            disp_img_id = 30U;
            NtcUpdateReset();
            RoomTempUpdateSet();
            DISPDeleteSettings1Screen();
            DISPCreateSettings2Screen();
            DISPScreen = DISPSETTINGS_2;
        }
        /************************************/
        /*      CHECK FOR MENU TIMEOUT      */
        /************************************/
        if(disp_img_tmr != disp_sreensvr_tmr) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            disp_img_tmr = disp_sreensvr_tmr;
        }
        else if((HAL_GetTick() - disp_img_tmr) >= SETTINGS_MENU_TIMEOUT)
        {
            DISPDeleteSettings1Screen();
            DISPScreen = DISPTHERMOSTAT;
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
        }
    }
    else if (DISPScreen == DISPSETTINGS_2) // setup menu 2
    { 
        disp_high_bcklght   = SPINBOX_GetValue(hSPNBX_DisplayHighBrightness);
        disp_low_bcklght    = SPINBOX_GetValue(hSPNBX_DisplayLowBrightness);
        scrnsvr_tout        = SPINBOX_GetValue(hSPNBX_ScrnsvrTimeout);
        scrnsvr_ena_hour    = SPINBOX_GetValue(hSPNBX_ScrnsvrEnableHour);
        scrnsvr_dis_hour    = SPINBOX_GetValue(hSPNBX_ScrnsvrDisableHour);
        
        if (CHECKBOX_GetState(hCHKBX_ScrnsvrClock) == 1)        ScrnsvrClkSet();
        else ScrnsvrClkReset();
        
        if (CHECKBOX_GetState(hCHKBX_ScrnsvrLogoClock) == 1)    ScrnsvrSemiClkSet();
        else ScrnsvrSemiClkReset();
        
        if (rtctm.Hours         != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour)))
        {
            rtctm.Hours = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour));
            HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
            RtcTimeValidSet();
        }
        
        if (rtctm.Minutes       != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute)))
        {
            rtctm.Minutes = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute));
            HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
            RtcTimeValidSet();
        }
        
        if (rtcdt.Date          != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day)))
        {
            rtcdt.Date = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day));
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcTimeValidSet();
        }
        
        if (rtcdt.Month         != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month)))
        {
            rtcdt.Month = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month));
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcTimeValidSet();
        }
        
        if (rtcdt.Year          != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000))
        {
            rtcdt.Year = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000);
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcTimeValidSet();
        }
        
        if (scrnsvr_clk_clr     != SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour))
        {
            scrnsvr_clk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour);
            GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
            GUI_FillRect(340, 51, 430, 59);
        }
        
        if (scrnsvr_semiclk_clr != SPINBOX_GetValue(hSPNBX_ScrnsvrSemiClkColor))
        {
            scrnsvr_semiclk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrSemiClkColor);
            GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
            GUI_FillRect(340, 91, 430, 99);
        }
        /************************************/
        /*      CHECK BUTTON OK STATE       */
        /************************************/
        if(BUTTON_IsPressed(hBUTTON_Ok)) 
        {
            DISPSaveSettings();
            DISPDeleteSettings2Screen();
            DISPScreen = DISPTHERMOSTAT;
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
        }
        /************************************/
        /*      CHECK BUTTON NEXT STATE     */
        /************************************/        
        else if(BUTTON_IsPressed(hBUTTON_Next)) 
        {
            disp_img_id = 30U;
            DISPDeleteSettings2Screen();
            DISPCreateSettings1Screen();
            DISPScreen = DISPSETTINGS_1;
        }
        /************************************/
        /*      CHECK FOR MENU TIMEOUT      */
        /************************************/
        if(disp_img_tmr != disp_sreensvr_tmr) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            disp_img_tmr = disp_sreensvr_tmr;
        }
        else if((HAL_GetTick() - disp_img_tmr) >= SETTINGS_MENU_TIMEOUT)
        {
            DISPDeleteSettings2Screen();
            DISPScreen = DISPTHERMOSTAT;
            if      (last_img_id == 10U)    DISPGuiSet(10U);   // return user fault screen
            else if (IsRoomThstConfig())    DISPGuiSet(0);     // room trhemostat gui
            else if (IsRoomCtrlConfig())    DISPGuiSet(60);    // room trhemostat gui
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCardMsg(const char *msg, GUI_COLOR colour)
{
    GUI_SelectLayer(0);
    GUI_SetColor(GUI_BLACK);
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(0);
    GUI_MULTIBUF_EndEx(0);
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    GUI_SetColor(colour);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    GUI_GotoXY(235, 100);
    GUI_DispString(msg);
    GUI_MULTIBUF_EndEx(1);
    GUI_Exec();
}
/**
  * @brief  Display coustom color+size+type text in coustom color+size+position room   
  * @param  0roomcolour,1+2roomx0pos,3+4roomy0pos,5+6roomx1pos,7+8roomy1pos,9txtcolour,10txtfont,11txthalign,12txtvalign,,13msgtxt
  * @retval disp_img_id = 50
  */
uint8_t DISPMessage(uint8_t *buff)
{
    int hal, val;
    GUI_RECT Rect;
    //
    //  check all parameter value
    //
    Rect.x0 = ((buff[1]<<8)|buff[2]);
    Rect.y0 = ((buff[3]<<8)|buff[4]);
    Rect.x1 = ((buff[5]<<8)|buff[6]);
    Rect.y1 = ((buff[7]<<8)|buff[8]);    
    if      (buff[0] > 25U) return NAK; // room color wrong
    else if (Rect.x0 > DISP_XSIZE) return NAK; // message room  x0 position wrong
    else if (Rect.y0 > DISP_YSIZE) return NAK; // message room  y0 position wrong
    else if (Rect.x1 > DISP_XSIZE) return NAK; // message room  x1 position wrong
    else if (Rect.y1 > DISP_YSIZE) return NAK; // message room  y1 position wrong
    else if (buff[9] > 25) return NAK; // font color wrong
    else if (buff[10]> 12) return NAK; // font size wrong
    else if (buff[11]>  4) return NAK; // text horisontal align wrong
    else if (buff[12]>  4) return NAK; // text vertical align wrong
    else if (GetSize(&buff[13]) == 0U) return NAK; // message text empty
    GUI_SetTextMode(GUI_TM_TRANS);
    //
    //  draw room
    //
    if      (buff[0] == 0U)  GUI_SetColor(GUI_BLUE);
    else if (buff[0] == 1U)  GUI_SetColor(GUI_GREEN);
    else if (buff[0] == 2U)  GUI_SetColor(GUI_RED);
    else if (buff[0] == 3U)  GUI_SetColor(GUI_CYAN);
    else if (buff[0] == 4U)  GUI_SetColor(GUI_MAGENTA);
    else if (buff[0] == 5U)  GUI_SetColor(GUI_YELLOW);
    else if (buff[0] == 6U)  GUI_SetColor(GUI_LIGHTBLUE);
    else if (buff[0] == 7U)  GUI_SetColor(GUI_LIGHTGREEN);
    else if (buff[0] == 8U)  GUI_SetColor(GUI_LIGHTRED);
    else if (buff[0] == 9U)  GUI_SetColor(GUI_LIGHTCYAN);
    else if (buff[0] == 10U) GUI_SetColor(GUI_LIGHTMAGENTA);
    else if (buff[0] == 11U) GUI_SetColor(GUI_LIGHTYELLOW);
    else if (buff[0] == 12U) GUI_SetColor(GUI_DARKBLUE);
    else if (buff[0] == 13U) GUI_SetColor(GUI_DARKGREEN);
    else if (buff[0] == 14U) GUI_SetColor(GUI_DARKRED);
    else if (buff[0] == 15U) GUI_SetColor(GUI_DARKCYAN);
    else if (buff[0] == 16U) GUI_SetColor(GUI_DARKMAGENTA);
    else if (buff[0] == 17U) GUI_SetColor(GUI_DARKYELLOW);
    else if (buff[0] == 18U) GUI_SetColor(GUI_WHITE);
    else if (buff[0] == 19U) GUI_SetColor(GUI_LIGHTGRAY);
    else if (buff[0] == 20U) GUI_SetColor(GUI_GRAY);
    else if (buff[0] == 21U) GUI_SetColor(GUI_DARKGRAY);
    else if (buff[0] == 22U) GUI_SetColor(GUI_BLACK);
    else if (buff[0] == 23U) GUI_SetColor(GUI_BROWN);
    else if (buff[0] == 24U) GUI_SetColor(GUI_ORANGE);
    else if (buff[0] == 25U) GUI_SetColor(GUI_TRANSPARENT);
    GUI_FillRectEx(&Rect);
    //
    //  write text
    //
    
    if      (buff[9] == 0U)  GUI_SetColor(GUI_BLUE);
    else if (buff[9] == 1U)  GUI_SetColor(GUI_GREEN);
    else if (buff[9] == 2U)  GUI_SetColor(GUI_RED);
    else if (buff[9] == 3U)  GUI_SetColor(GUI_CYAN);
    else if (buff[9] == 4U)  GUI_SetColor(GUI_MAGENTA);
    else if (buff[9] == 5U)  GUI_SetColor(GUI_YELLOW);
    else if (buff[9] == 6U)  GUI_SetColor(GUI_LIGHTBLUE);
    else if (buff[9] == 7U)  GUI_SetColor(GUI_LIGHTGREEN);
    else if (buff[9] == 8U)  GUI_SetColor(GUI_LIGHTRED);
    else if (buff[9] == 9U)  GUI_SetColor(GUI_LIGHTCYAN);
    else if (buff[9] == 10U) GUI_SetColor(GUI_LIGHTMAGENTA);
    else if (buff[9] == 11U) GUI_SetColor(GUI_LIGHTYELLOW);
    else if (buff[9] == 12U) GUI_SetColor(GUI_DARKBLUE);
    else if (buff[9] == 13U) GUI_SetColor(GUI_DARKGREEN);
    else if (buff[9] == 14U) GUI_SetColor(GUI_DARKRED);
    else if (buff[9] == 15U) GUI_SetColor(GUI_DARKCYAN);
    else if (buff[9] == 16U) GUI_SetColor(GUI_DARKMAGENTA);
    else if (buff[9] == 17U) GUI_SetColor(GUI_DARKYELLOW);
    else if (buff[9] == 18U) GUI_SetColor(GUI_WHITE);
    else if (buff[9] == 19U) GUI_SetColor(GUI_LIGHTGRAY);
    else if (buff[9] == 20U) GUI_SetColor(GUI_GRAY);
    else if (buff[9] == 21U) GUI_SetColor(GUI_DARKGRAY);
    else if (buff[9] == 22U) GUI_SetColor(GUI_BLACK);
    else if (buff[9] == 23U) GUI_SetColor(GUI_BROWN);
    else if (buff[9] == 24U) GUI_SetColor(GUI_ORANGE);
    else if (buff[9] == 25U) GUI_SetColor(GUI_TRANSPARENT);
    if      (buff[10]== 0U)  GUI_SetFont(GUI_FONT_8_1);
    else if (buff[10]== 1U)  GUI_SetFont(GUI_FONT_10_1);
    else if (buff[10]== 2U)  GUI_SetFont(GUI_FONT_13_1);
    else if (buff[10]== 3U)  GUI_SetFont(GUI_FONT_13B_1);
    else if (buff[10]== 4U)  GUI_SetFont(GUI_FONT_16_1);
    else if (buff[10]== 5U)  GUI_SetFont(GUI_FONT_16B_1);
    else if (buff[10]== 6U)  GUI_SetFont(GUI_FONT_20_1);
    else if (buff[10]== 7U)  GUI_SetFont(GUI_FONT_20B_1);
    else if (buff[10]== 8U)  GUI_SetFont(GUI_FONT_24_1);
    else if (buff[10]== 9U)  GUI_SetFont(GUI_FONT_24B_1);
    else if (buff[10]== 10U) GUI_SetFont(GUI_FONT_32_1);
    else if (buff[10]== 11U) GUI_SetFont(GUI_FONT_32B_1);
    if      (buff[11]== 0U)  hal = GUI_TA_LEFT;
    else if (buff[11]== 1U)  hal = GUI_TA_HORIZONTAL;
    else if (buff[11]== 2U)  hal = GUI_TA_RIGHT;
    else if (buff[11]== 3U)  hal = GUI_TA_CENTER;
    else if (buff[11]== 4U)  hal = GUI_TA_HCENTER;
    if      (buff[12]== 0U)  val = GUI_TA_TOP;
    else if (buff[12]== 1U)  val = GUI_TA_VERTICAL;
    else if (buff[12]== 2U)  val = GUI_TA_BOTTOM;
    else if (buff[12]== 3U)  val = GUI_TA_BASELINE;
    else if (buff[12]== 4U)  val = GUI_TA_VCENTER;
    GUI_DispStringInRectWrap((char*)&buff[13], &Rect, hal | val, GUI_WRAPMODE_WORD);
    return ACK; // parameter error
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t DISPQRCode(int x0, int y0)
{
    uint8_t qr_code[QRC_BSIZE];
//    int rect_x0, rect_y0, rect_x1, rect_y1;
//    //
//    //  check all parameter value
//    //
//    rect_x0 = ((buff[1] << 8) & 0xFF00U) + (buff[2] & 0xFFU);
//    rect_y0 = ((buff[3] << 8) & 0xFF00U) + (buff[4] & 0xFFU);
//    rect_x1 = ((buff[5] << 8) & 0xFF00U) + (buff[6] & 0xFFU);
//    rect_y1 = ((buff[7] << 8) & 0xFF00U) + (buff[8] & 0xFFU);  
//    GUI_RECT Rect = {rect_x0, rect_y0, rect_x1, rect_y1};
//    GUI_SetColor(GUI_WHITE);
//    GUI_FillRectEx(&Rect);
    
    ZEROFILL(qr_code, COUNTOF(qr_code));
    EE_ReadBuffer(qr_code, EE_QR_CODE, QRC_DSIZE);
    GUI_MULTIBUF_BeginEx(1);
    hQR_Code = GUI_QR_Create((char*) qr_code, 4, GUI_QR_ECLEVEL_L, 0);
    GUI_QR_Draw(hQR_Code, 50, 40);
    GUI_MULTIBUF_EndEx(1);
    return 0x0U;
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval DISP sreensvr_tmr loaded with system_tmr value
  */
void DISPSetBrightnes(uint8_t val)
{
    if      (val < DISP_BRGHT_MIN) val = DISP_BRGHT_MIN;
    else if (val > DISP_BRGHT_MAX) val = DISP_BRGHT_MAX;
    
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, (uint16_t) (val * 10U));
}
/**
  * @brief  Display Screen Redraw
  * @param  new image id
  * @retval new activ screen
  */
static void DISPDrawScreen(uint8_t img_id)
{
    GUI_SelectLayer(0);
    GUI_SetColor(GUI_BLACK);
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(0);

    if      (img_id == 0U) GUI_BMP_Draw(&disp_00, 0, 0);    //  gui = thermostat 
    else if (img_id == 1U) GUI_BMP_Draw(&disp_01, 0, 0);    //  eng user message 1- 5
    else if (img_id == 2U) GUI_BMP_Draw(&disp_02, 0, 0);
    else if (img_id == 3U) GUI_BMP_Draw(&disp_03, 0, 0);
    else if (img_id == 4U) GUI_BMP_Draw(&disp_04, 0, 0);
    else if (img_id == 5U) GUI_BMP_Draw(&disp_05, 0, 0);
    
    else if (img_id == 10U) GUI_BMP_Draw(&disp_10, 0, 0);   //  gui = user fault message
    else if (img_id == 10U) GUI_BMP_Draw(&disp_11, 0, 0);   //  ger user message 11-15
    else if (img_id == 12U) GUI_BMP_Draw(&disp_12, 0, 0);   
    else if (img_id == 13U) GUI_BMP_Draw(&disp_13, 0, 0);
    else if (img_id == 14U) GUI_BMP_Draw(&disp_14, 0, 0);
    else if (img_id == 15U) GUI_BMP_Draw(&disp_15, 0, 0);

    else if (img_id == 20U);                                // gui = weather forecast
    else if (img_id == 21U) GUI_BMP_Draw(&disp_21, 0, 0);   //  fra user message 21-25
    else if (img_id == 22U) GUI_BMP_Draw(&disp_22, 0, 0);
    else if (img_id == 23U) GUI_BMP_Draw(&disp_23, 0, 0);
    else if (img_id == 24U) GUI_BMP_Draw(&disp_24, 0, 0);
    else if (img_id == 25U) GUI_BMP_Draw(&disp_25, 0, 0);

    else if (img_id == 30U);                                // gui = settings menu
    else if (img_id == 31U) GUI_BMP_Draw(&disp_31, 0, 0);   //  arab user message 31-35
    else if (img_id == 32U) GUI_BMP_Draw(&disp_32, 0, 0);
    else if (img_id == 33U) GUI_BMP_Draw(&disp_33, 0, 0);
    else if (img_id == 34U) GUI_BMP_Draw(&disp_34, 0, 0);
    else if (img_id == 35U) GUI_BMP_Draw(&disp_35, 0, 0);

    else if (img_id == 40U);                                // gui = qr code
    else if (img_id == 41U) GUI_BMP_Draw(&disp_41, 0, 0);   //  chn user message 41-45
    else if (img_id == 42U) GUI_BMP_Draw(&disp_42, 0, 0);  
    else if (img_id == 43U) GUI_BMP_Draw(&disp_43, 0, 0);
    else if (img_id == 44U) GUI_BMP_Draw(&disp_44, 0, 0);
    else if (img_id == 45U) GUI_BMP_Draw(&disp_45, 0, 0);

    else if (img_id == 50U);                                    // gui = coustom message
    else if (img_id == 51U); // GUI_BMP_Draw(&disp_51, 0, 0);   //  jap user message 51-55
    else if (img_id == 52U); // GUI_BMP_Draw(&disp_52, 0, 0);
    else if (img_id == 53U); // GUI_BMP_Draw(&disp_53, 0, 0);
    else if (img_id == 54U); // GUI_BMP_Draw(&disp_54, 0, 0);
    else if (img_id == 55U); // GUI_BMP_Draw(&disp_55, 0, 0);

    else if (img_id == 60U);                                    // gui = room controller
    else if (img_id == 61U) GUI_BMP_Draw(&disp_61, 0, 0);       //  ita user message 61-65
    else if (img_id == 62U) GUI_BMP_Draw(&disp_62, 0, 0);
    else if (img_id == 63U) GUI_BMP_Draw(&disp_63, 0, 0);
    else if (img_id == 64U) GUI_BMP_Draw(&disp_64, 0, 0);
    else if (img_id == 65U) GUI_BMP_Draw(&disp_65, 0, 0);

    else if (img_id == 70U);                                    // gui = keypad - password input
    else if (img_id == 71U) GUI_BMP_Draw(&disp_71, 0, 0);       //  tur user message 71-75
    else if (img_id == 72U) GUI_BMP_Draw(&disp_72, 0, 0);
    else if (img_id == 73U) GUI_BMP_Draw(&disp_73, 0, 0);
    else if (img_id == 74U) GUI_BMP_Draw(&disp_74, 0, 0);
    else if (img_id == 75U) GUI_BMP_Draw(&disp_75, 0, 0);

    else if (img_id == 80U);                                    // gui = language select
    else if (img_id == 81U) GUI_BMP_Draw(&disp_81, 0, 0);       //  slo user message 81-85
    else if (img_id == 82U) GUI_BMP_Draw(&disp_82, 0, 0);
    else if (img_id == 83U) GUI_BMP_Draw(&disp_83, 0, 0);
    else if (img_id == 84U) GUI_BMP_Draw(&disp_84, 0, 0);
    else if (img_id == 85U) GUI_BMP_Draw(&disp_85, 0, 0);
    
    else if (img_id == 90U) {}                                  // gui - maid menu

    GUI_MULTIBUF_EndEx(0);
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    if ((img_id  % 10U) != 0U)  GUI_DrawBitmap(&bmbtn_ok, BTN_OK_X0+30, BTN_OK_Y0+70); // draw button ok for image id with units different from 0 (1,2,3..9,11,12...)
    if ((img_id  % 10U) == 1U)  GUI_DrawBitmap(&bmbtn_door_open, BTN_DOOR_X0+30, BTN_DOOR_Y0+70); // draw button door open for image id with units of 1 (1,11,21...)    
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief  Display Date and Time in deifferent font size colour and position
  * @param  Flags: IsRtcTimeValid, IsScrnsvrActiv, BUTTON_Dnd, BUTTON_BTNMaid,
            BTNSosReset, IsScrnsvrSemiClkActiv, IsScrnsvrClkActiv
  * @retval None
  */
static void DISPDateTime(void)
{
    char dbuf[32];
    static uint8_t old_day = 0;
    unix_rtc = 0;   // clear unix time
    if(!IsRtcTimeValid()) return; // nothing to display untill system rtc validated
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    unix_rtc = rtc2unix(&rtctm, &rtcdt);
    /************************************/
    /*   CHECK IS SCREENSAVER ENABLED   */
    /************************************/ 
    if      (scrnsvr_ena_hour >= scrnsvr_dis_hour)
    {
        if      (Bcd2Dec(rtctm.Hours) >= scrnsvr_ena_hour) ScrnsvrEnable();
        else if (Bcd2Dec(rtctm.Hours)  < scrnsvr_dis_hour) ScrnsvrEnable();
        else if (IsScrnsvrEnabled()) ScrnsvrDisable(), DISPUpdateSet();
    }
    else if (scrnsvr_ena_hour < scrnsvr_dis_hour)
    {
        if      ((Bcd2Dec(rtctm.Hours) < scrnsvr_dis_hour) && (Bcd2Dec(rtctm.Hours) >= scrnsvr_ena_hour)) ScrnsvrEnable();
        else if (IsScrnsvrEnabled()) ScrnsvrDisable(), DISPUpdateSet();
    }
    /************************************/
    /*      DISPLAY  DATE  &  TIME      */
    /************************************/ 
    if      (IsScrnsvrActiv() && IsScrnsvrEnabled() && (IsBTNDndActiv() || IsBTNMaidActiv() || IsBTNSosActiv()) && IsScrnsvrSemiClkActiv())
    { 
        if(!IsScrnsvrInitActiv())
        {
            ScrnsvrInitSet();
            GUI_SelectLayer(0);
            GUI_Clear();
            GUI_SetColor(GUI_BLACK);
            GUI_FillRect(0, 0, 330, 80);
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT);
            GUI_ClearRect(0, 100, 320, 270);
            rtctm.Seconds = 0U;
        }        
        GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_RIGHT);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(150, 0, 190, 100);
        if      (rtctm.Seconds & 1U) GUI_DispCharAt(':', 150, 0);
        else if (rtctm.Seconds == 0U)
        {
            GUI_ClearRect(0, 0, 330, 80);
            GUI_DispHexAt(rtctm.Hours, 130, 0, 2);
            GUI_SetTextAlign(GUI_TA_LEFT);
            GUI_DispHexAt(rtctm.Minutes, 190, 0, 2);
        }        
        GUI_MULTIBUF_EndEx(1);
    }
    else if (IsScrnsvrActiv() && IsScrnsvrEnabled() && !IsBTNDndActiv() && !IsBTNMaidActiv() && !IsBTNSosActiv() && IsScrnsvrClkActiv())
    {
        if(!IsScrnsvrInitActiv() || (old_day != rtcdt.WeekDay))
        {
            ScrnsvrInitSet();
            GUI_SelectLayer(0);
            GUI_Clear();
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_Clear();
            old_min = 60U;
            old_day = rtcdt.WeekDay;
        }        
        HEX2STR(dbuf, &rtctm.Hours);
        if(rtctm.Seconds & 0x01U) dbuf[2] = ':';
        else dbuf[2] = ' ';
        HEX2STR(&dbuf[3], &rtctm.Minutes);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS);
        GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(0, 80, 480, 192);
        GUI_DispString(dbuf);
        if      (rtcdt.WeekDay == 0x00) rtcdt.WeekDay = 0x07U;
        if      (rtcdt.WeekDay == 0x01) memcpy(dbuf,    "  Monday  ", 10);
        else if (rtcdt.WeekDay == 0x02) memcpy(dbuf,    " Tuestday ", 10);
        else if (rtcdt.WeekDay == 0x03) memcpy(dbuf,    "Wednesday ", 10);
        else if (rtcdt.WeekDay == 0x04) memcpy(dbuf,    "Thurstday ", 10);
        else if (rtcdt.WeekDay == 0x05) memcpy(dbuf,    "  Friday  ", 10);
        else if (rtcdt.WeekDay == 0x06) memcpy(dbuf,    " Saturday ", 10);
        else if (rtcdt.WeekDay == 0x07) memcpy(dbuf,    "  Sunday  ", 10);
        HEX2STR(&dbuf[10], &rtcdt.Date);
        if      (rtcdt.Month == 0x01) memcpy(&dbuf[12], ". January ", 10);		
        else if (rtcdt.Month == 0x02) memcpy(&dbuf[12], ". February", 10);
        else if (rtcdt.Month == 0x03) memcpy(&dbuf[12], ".  March  ", 10);
        else if (rtcdt.Month == 0x04) memcpy(&dbuf[12], ".  April  ", 10);
        else if (rtcdt.Month == 0x05) memcpy(&dbuf[12], ".   May   ", 10);
        else if (rtcdt.Month == 0x06) memcpy(&dbuf[12], ".   June  ", 10);
        else if (rtcdt.Month == 0x07) memcpy(&dbuf[12], ".   July  ", 10);
        else if (rtcdt.Month == 0x08) memcpy(&dbuf[12], ". August  ", 10);
        else if (rtcdt.Month == 0x09) memcpy(&dbuf[12], ".September", 10);
        else if (rtcdt.Month == 0x10) memcpy(&dbuf[12], ". Oktober ", 10);
        else if (rtcdt.Month == 0x11) memcpy(&dbuf[12], ". November", 10);
        else if (rtcdt.Month == 0x12) memcpy(&dbuf[12], ". December", 10);
        memcpy(&dbuf[22], " 20", 3U);
        HEX2STR(&dbuf[25], &rtcdt.Year);
        dbuf[27] = '.';
        dbuf[28] = NUL;
        GUI_SetFont(GUI_FONT_24B_1);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS + 70U);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1); 
    }
    else if (old_min != rtctm.Minutes)// thermostat gui clock
    {
        old_min = rtctm.Minutes;
        HEX2STR(dbuf, &rtctm.Hours);
        dbuf[2] = ':';
        HEX2STR(&dbuf[3], &rtctm.Minutes);
        GUI_SetFont(GUI_FONT_32_1);
        GUI_SetColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        if (disp_img_id == 0)
        {
            GUI_GotoXY(5, 245);
            GUI_ClearRect(0, 220, 100, 270);
        }
        else
        {
            GUI_GotoXY(120, 245);
            GUI_ClearRect(120, 220, 200, 270);
        }
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1);
    }
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval DISP sreensvr_tmr loaded with system_tmr value
  */
static void DISPResetScrnsvr(void)
{
    /**
    *   if screensaver activ mode digital clock request display redraw 
    *   or just set backlight to high 
    **/
    if (IsScrnsvrActiv() && IsScrnsvrEnabled() && (IsScrnsvrSemiClkActiv() || IsScrnsvrClkActiv())) 
    {
        DISPUpdateSet();
    }    
    ScrnsvrReset();
    ScrnsvrInitReset();
    disp_sreensvr_tmr = HAL_GetTick();
    DISPSetBrightnes(disp_high_bcklght);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPTemperatureSetPoint(void)
{
    GUI_MULTIBUF_BeginEx(1);
    GUI_ClearRect(SP_H_POS - 5, SP_V_POS - 5, SP_H_POS + 120, SP_V_POS + 85);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_D48);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_GotoXY(SP_H_POS, SP_V_POS);
    GUI_DispDec(thst_sp , 2);
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISPLoadWFC(int8_t *buff)
{
    RTC_t dt;
    uint32_t wfc_time_stamp;
    int8_t wfc_buf[WFC_DSIZE];
    
    if (!IsRtcTimeValid()) return 0x1U;
    RTC_GetDateTime(&dt, RTC_FORMAT_BIN);
    if (EE_ReadBuffer ((uint8_t*)wfc_buf, EE_WFORECAST, WFC_DSIZE) != 0x0U) return 0x1U;
    wfc_time_stamp = (((wfc_buf[0] << 24) & 0xFF000000U)|((wfc_buf[1] << 16) & 0x00FF0000U)|
                      ((wfc_buf[2] <<  8) & 0x0000FF00U)| (wfc_buf[3]        & 0x000000FFU));
    if ((wfc_time_stamp == 0x0U) || (wfc_time_stamp == 0xFFFFFFFFU))  return 0x1U;
    if (wfc_time_stamp > (dt.unix + SECONDS_PER_DAY))  return 0x1U;
    if (dt.unix > (wfc_time_stamp + SECONDS_PER_DAY))  return 0x1U;
    if ((wfc_buf[4]  < -60) || (wfc_buf[4]  > 60)) return 0x1U;  // BYTE4: 	ACTUAL TEMPERATURE		int8  -60 ~ +60
    if ((wfc_buf[5]  <   1) || (wfc_buf[5]  >  7)) return 0x1U;  // BYTE5:  ACTUAL CLOUDNESS		uint8 	 1 ~ 7
    if ((wfc_buf[6]  < -60) || (wfc_buf[6]  > 60)) return 0x1U;  // BYTE6:  DAY + 1 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[7]  < -60) || (wfc_buf[7]  > 60)) return 0x1U;  // BYTE7:  DAY + 1 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[8]  <   1) || (wfc_buf[8]  >  7)) return 0x1U;  // BYTE8:  DAY + 1 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[9]  < -60) || (wfc_buf[9]  > 60)) return 0x1U;  // BYTE9:  DAY + 2 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[10] < -60) || (wfc_buf[10] > 60)) return 0x1U;  // BYTE10: DAY + 2 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[11] <   1) || (wfc_buf[11] >  7)) return 0x1U;  // BYTE11:	DAY + 2 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[12] < -60) || (wfc_buf[12] > 60)) return 0x1U;  // BYTE12:	DAY + 3 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[13] < -60) || (wfc_buf[13] > 60)) return 0x1U;  // BYTE13:	DAY + 3 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[14] <   1) || (wfc_buf[14] >  7)) return 0x1U;  // BYTE14:	DAY + 3 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[15] < -60) || (wfc_buf[15] > 60)) return 0x1U;  // BYTE15:	DAY + 4 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[16] < -60) || (wfc_buf[16] > 60)) return 0x1U;  // BYTE16:	DAY + 4 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[17] <   1) || (wfc_buf[17] >  7)) return 0x1U;  // BYTE17:	DAY + 4 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[18] < -60) || (wfc_buf[18] > 60)) return 0x1U;  // BYTE18:	DAY + 5 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[19] < -60) || (wfc_buf[19] > 60)) return 0x1U;  // BYTE19:	DAY + 5 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[20] <   1) || (wfc_buf[20] >  7)) return 0x1U;  // BYTE20:	DAY + 5 CLOUDS			uint8	 1 ~ 7
    memcpy(buff, wfc_buf, WFC_DSIZE);
    return 0x0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISPSaveWFC(int8_t *buff)
{
    RTC_t dt;
    uint32_t wfc_time_stamp;
    /**
    *   FULL CHECK OF DATA BEFORE WRITING TO EEPROM
    */
    if (!IsRtcTimeValid()) return 0x1U;
    RTC_GetDateTime(&dt, RTC_FORMAT_BIN);
    wfc_time_stamp = (((buff[0] << 24) & 0xFF000000U)|((buff[1] << 16) & 0x00FF0000U)|
                      ((buff[2] <<  8) & 0x0000FF00U)| (buff[3]        & 0x000000FFU));
    if (!wfc_time_stamp || (wfc_time_stamp == 0xFFFFFFFFU))  return 0x1U;
    if (wfc_time_stamp > (dt.unix + SECONDS_PER_DAY))    return 0x1U;
    if (dt.unix > (wfc_time_stamp + SECONDS_PER_DAY))    return 0x1U;
    if ((buff[4]  < -60) || (buff[4]  > 60)) return 0x1U;  // BYTE4:	ACTUAL TEMPERATURE		int8  -60 ~ +60
    if ((buff[5]  <   1) || (buff[5]  >  7)) return 0x1U;  // BYTE5:	ACTUAL CLOUDNESS		uint8 	 1 ~ 7
    if ((buff[6]  < -60) || (buff[6]  > 60)) return 0x1U;  // BYTE6:	DAY + 1 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[7]  < -60) || (buff[7]  > 60)) return 0x1U;  // BYTE7:	DAY + 1 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[8]  <   1) || (buff[8]  >  7)) return 0x1U;  // BYTE8:	DAY + 1 CLOUDS			uint8	 1 ~ 7
    if ((buff[9]  < -60) || (buff[9]  > 60)) return 0x1U;  // BYTE9:	DAY + 2 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[10] < -60) || (buff[10] > 60)) return 0x1U;  // BYTE10:	DAY + 2 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[11] <   1) || (buff[11] >  7)) return 0x1U;  // BYTE11:	DAY + 2 CLOUDS			uint8	 1 ~ 7
    if ((buff[12] < -60) || (buff[12] > 60)) return 0x1U;  // BYTE12:	DAY + 3 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[13] < -60) || (buff[13] > 60)) return 0x1U;  // BYTE13:	DAY + 3 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[14] <   1) || (buff[14] >  7)) return 0x1U;  // BYTE14:	DAY + 3 CLOUDS			uint8	 1 ~ 7
    if ((buff[15] < -60) || (buff[15] > 60)) return 0x1U;  // BYTE15:	DAY + 4 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[16] < -60) || (buff[16] > 60)) return 0x1U;  // BYTE16:	DAY + 4 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[17] <   1) || (buff[17] >  7)) return 0x1U;  // BYTE17:	DAY + 4 CLOUDS			uint8	 1 ~ 7
    if ((buff[18] < -60) || (buff[18] > 60)) return 0x1U;  // BYTE18:	DAY + 5 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[19] < -60) || (buff[19] > 60)) return 0x1U;  // BYTE19:	DAY + 5 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[20] <   1) || (buff[20] >  7)) return 0x1U;  // BYTE20:	DAY + 5 CLOUDS			uint8	 1 ~ 7
    if (EE_WriteBuffer((uint8_t *)buff, EE_WFORECAST, WFC_DSIZE) != 0x0U) return 0x1U;
    return 0x0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPInitWFCFrame(FORECAST_DayTypeDef *fday)
{
        char dsp[8];
        uint32_t tp;
        //
        //  drav rounded frame for one forecast day
        //
		GUI_SetColor((0x62ul << 24) | GUI_DARKGRAY);
		GUI_FillRoundedRect(    ForecastFrame[fday->index].x0 + 1U, 
                                ForecastFrame[fday->index].y0 + 1U,
								ForecastFrame[fday->index].x1 - 1U, 
								ForecastFrame[fday->index].y1 - 1U, 10U);
                    
        GUI_SetColor(GUI_WHITE);            
		GUI_SetFont(GUI_FONT_24B_1);
		GUI_SetTextMode(GUI_TM_TRANS);
		ForecastFrame[fday->index].y0 += 5U;
		GUI_DispStringInRect(_apDays[fday->week_day], &ForecastFrame[fday->index], GUI_TA_TOP|GUI_TA_HCENTER);
		ForecastFrame[fday->index].y0 -= 5U;
        
        switch(fday->cloudness)
        {
            case 1:
                GUI_DrawBitmap(&bmclear_sky_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 2:
                GUI_DrawBitmap(&bmfew_clouds_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 3:
                GUI_DrawBitmap(&bmscattered_clouds_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 4:
                GUI_DrawBitmap(&bmrain_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 5:
                GUI_DrawBitmap(&bmshower_rain_icon,
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 6:
                GUI_DrawBitmap(&bmthunderstorm_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 7:
                GUI_DrawBitmap(&bmsnow_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
        }
        
		GUI_SetColor(GUI_RED);
        GUI_SetFont(GUI_FONT_20_1);
		GUI_SetTextMode(GUI_TM_TRANS);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_GotoXY((ForecastFrame[fday->index].x0 + 
					((ForecastFrame[fday->index].x1 - 
					ForecastFrame[fday->index].x0) / 2U)), 
					(ForecastFrame[fday->index].y0 + 90U));

        ZEROFILL(dsp, COUNTOF(dsp));
        if(fday->high_temp < 0) 
        {
            Int2Str(dsp, fday->high_temp, 0U);
        }
        else 
        {
            dsp[0] = '+';
            Int2Str(&dsp[1], fday->high_temp, 0U);
        }
        
        tp = strlen(dsp);
        memcpy(&dsp[tp], "°C", 3U);
        GUI_DispString(dsp);
        
        GUI_SetColor(GUI_LIGHTBLUE);
        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextMode(GUI_TM_TRANS);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_GotoXY((ForecastFrame[fday->index].x0 + 
					((ForecastFrame[fday->index].x1 - 
					ForecastFrame[fday->index].x0) / 2U)), 
					(ForecastFrame[fday->index].y0 + 120U));
        
		ZEROFILL(dsp, COUNTOF(dsp));
        if(fday->low_temp < 0) 
        {
            Int2Str(dsp, fday->low_temp, 0U);
        }
        else 
        {
            dsp[0] = '+';
            Int2Str(&dsp[1], fday->low_temp, 0U);
        }
        
        tp = strlen(dsp);
        memcpy(&dsp[tp], "°C", 3U);
        GUI_DispString(dsp);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateWFCScreen(int8_t *buff)
{
    char disp_buff[16];
    uint32_t i;
    /* 
    *   convert and display clock time
    */
    GUI_GotoXY(400, 150);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    HEX2STR(disp_buff, &rtctm.Hours);
    disp_buff[2] = ':';
    HEX2STR(&disp_buff[3], &rtctm.Minutes);
    GUI_DispString(disp_buff);
    /* 
    *   convert and display date day
    */
    GUI_GotoXY(400, 185);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_24_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    if      (rtcdt.WeekDay == 0x00)   rtcdt.WeekDay = 0x07;
    if      (rtcdt.WeekDay == 0x01)   GUI_DispString("Monday");
    else if (rtcdt.WeekDay == 0x02)   GUI_DispString("Tuestday");
    else if (rtcdt.WeekDay == 0x03)   GUI_DispString("Wednesday");
    else if (rtcdt.WeekDay == 0x04)   GUI_DispString("Thurstday");
    else if (rtcdt.WeekDay == 0x05)   GUI_DispString("Friday");
    else if (rtcdt.WeekDay == 0x06)   GUI_DispString("Saturday");
    else if (rtcdt.WeekDay == 0x07)   GUI_DispString("Sunday");
    /* 
    *   convert and display month and date
    */
    GUI_GotoXY(400, 215);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    HEX2STR(disp_buff, &rtcdt.Date);
    disp_buff[2] = '.';
    if      (rtcdt.Month == 0x01) memcpy(&disp_buff[3], "January",    7);		
    else if (rtcdt.Month == 0x02) memcpy(&disp_buff[3], "February",   8);
    else if (rtcdt.Month == 0x03) memcpy(&disp_buff[3], "March",      5);
    else if (rtcdt.Month == 0x04) memcpy(&disp_buff[3], "April",      5);
    else if (rtcdt.Month == 0x05) memcpy(&disp_buff[3], "May",        3);
    else if (rtcdt.Month == 0x06) memcpy(&disp_buff[3], "June",       4);
    else if (rtcdt.Month == 0x07) memcpy(&disp_buff[3], "July",       4);
    else if (rtcdt.Month == 0x08) memcpy(&disp_buff[3], "August",     6);
    else if (rtcdt.Month == 0x09) memcpy(&disp_buff[3], "September",  9);
    else if (rtcdt.Month == 0x10) memcpy(&disp_buff[3], "October",    7);
    else if (rtcdt.Month == 0x11) memcpy(&disp_buff[3], "November",   8);
    else if (rtcdt.Month == 0x12) memcpy(&disp_buff[3], "December",   8);
    GUI_DispString(disp_buff);
    /* 
    *   convert and display year
    */
    GUI_GotoXY(400, 245);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    disp_buff[0] = '2';
    disp_buff[1] = '0';
    HEX2STR(&disp_buff[2], &rtcdt.Year);
    GUI_DispString(disp_buff);
    /* 
    *   display city
    */
    GUI_GotoXY(20, 10);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString("Sarajevo Today");
    /* 
    *   display city actual temperature
    */
    GUI_GotoXY(20, 50);
    GUI_SetFont(GUI_FONT_32_1);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    buff += 4U; // buffer data offset
    
    if(*buff < 0) 
    {
        Int2Str(disp_buff, *buff++, 0U);
    }
    else 
    {
        disp_buff[0] = '+';
        Int2Str(&disp_buff[1], *buff++, 0U);
    }
    
    i = strlen(disp_buff);
    memcpy(&disp_buff[i], "°C", 3U);
    GUI_DispString(disp_buff);
    /* 
    *   show cloudnes icon
    */
    GUI_GotoXY(120, 50);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_32_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    
    switch(*buff++)
    {
        case 1:
            GUI_DispString("Clear Sky");
            GUI_DrawBitmap(&bmclear_sky_img, 320, 0);
            break;
        case 2:
            GUI_DispString("Few Clouds");
            GUI_DrawBitmap(&bmfew_clouds_img, 320, 0);
            break;
        case 3:
            GUI_DispString("Scattered Clouds");
            GUI_DrawBitmap(&bmscattered_clouds_img, 320, 0);
            break;
        case 4:
            GUI_DispString("Rain");
            GUI_DrawBitmap(&bmrain_img, 320, 0);
            break;
        case 5:
            GUI_DispString("Shower Rain");
            GUI_DrawBitmap(&bmshower_rain_img  , 320, 0);
            break;
        case 6:
            GUI_DispString("Thunderstorm");
            GUI_DrawBitmap(&bmthunderstorm_img, 320, 0);
            break;
        case 7:
            GUI_DispString("Snow");
            GUI_DrawBitmap(&bmsnow_img, 320, 0);
            break;
        default:
            GUI_DispString("Data Unavailable");
            break;
    }
    /* 
    *   draw weather forecast for nex 5 days
    */
    for(i = 0U; i < 5U; i++)   // init forecast day structure
    {
        FORECAST_Day[i].index = i;
        FORECAST_Day[i].week_day = rtcdt.WeekDay + i + 1U;
        if(FORECAST_Day[i].week_day > 0x07U) FORECAST_Day[i].week_day -= 0x07U;
        FORECAST_Day[i].high_temp   = *buff++;
        FORECAST_Day[i].low_temp    = *buff++;
        FORECAST_Day[i].cloudness   = *buff++;
        DISPInitWFCFrame(&FORECAST_Day[i]);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateSettings1Screen(void)
{
    uint8_t ebuf[8];
    uint32_t count;
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);    
    GUI_GotoXY(10, 4);
    GUI_DispString("SETPOINT LIMITS");
    GUI_DrawHLine(12, 5, 190);
    hSPNBX_MaxSetpoint = SPINBOX_CreateEx(10, 20, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_MaxSetpoint, 15, 40);
    SPINBOX_SetEdge(hSPNBX_MaxSetpoint, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_MaxSetpoint, thst_max_sp);
    GUI_GotoXY(130, 30);
    GUI_DispString("MAX. USER");
    GUI_GotoXY(130, 42);
    GUI_DispString("SETPOINT");
    GUI_GotoXY(130, 54);
    GUI_DispString("TEMP. x1*C");
    hSPNBX_MinSetpoint = SPINBOX_CreateEx(10, 80, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_MinSetpoint, 15, 40);
    SPINBOX_SetEdge(hSPNBX_MinSetpoint, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_MinSetpoint, thst_min_sp);
    GUI_GotoXY(130, 90);
    GUI_DispString("MIN. USER");
    GUI_GotoXY(130, 102);
    GUI_DispString("SETPOINT");
    GUI_GotoXY(130, 114);
    GUI_DispString("TEMP. x1*C");    
    GUI_GotoXY(10, 140);
    GUI_DispString("ROOM NTC OFFSET");
    GUI_DrawHLine(150, 5, 190);
    hSPNBX_AmbientNtcOffset = SPINBOX_CreateEx(10, 160, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_AmbientNtcOffset, -100, 100);
    SPINBOX_SetEdge(hSPNBX_AmbientNtcOffset, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_AmbientNtcOffset, ntc_offset);
    GUI_GotoXY(130, 170);
    GUI_DispString("SENSOR");
    GUI_GotoXY(130, 182);
    GUI_DispString("OFFSET");
    GUI_GotoXY(130, 194);
    GUI_DispString("TEMP. x0.1*C");    
    GUI_GotoXY(210, 4);
    GUI_DispString("SYSTEM CONFIGURATION");
    GUI_DrawHLine(12, 205, 470);    
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);    
    GUI_GotoXY(210, 30);
    GUI_DispString("SYSTEM ID: ");
    GUI_DispDecMin(((sysid[0]<<8)|sysid[1]));
    GUI_GotoXY(210, 45);
    GUI_DispString("OW ADDRESS: ");
    GUI_DispDecMin(owifa);
    GUI_GotoXY(210, 60);
    GUI_DispString("OW GROUP: ");
    GUI_DispDecMin(owgra);
    GUI_GotoXY(210, 75);
    GUI_DispString("OW BROADCAST: ");
    GUI_DispDecMin(owbra);
    GUI_GotoXY(210, 90);
    GUI_DispString("OW BAUDRATE: ");
    GUI_DispDecMin(bps[owbps]);  
    GUI_GotoXY(210, 105);
    GUI_DispString("HANDMAID PASSW: "); 
    EE_ReadBuffer(ebuf,EE_MAID_PSWRD,3);
    GUI_DispDecMin(((ebuf[0]<<16)|(ebuf[1]<<8)|ebuf[2]));
    GUI_GotoXY(210, 120);
    GUI_DispString("MANAGER PASSW: "); 
    EE_ReadBuffer(ebuf,EE_MNGR_PSWRD,3);
    GUI_DispDecMin(((ebuf[0]<<16)|(ebuf[1]<<8)|ebuf[2]));   
    for (count = 0; count < 8; count++)
    {
        EE_ReadBuffer (ebuf, EE_USER_PSWRD+(count*8U), 4);
        GUI_SetTextAlign(GUI_TA_LEFT);
        GUI_SetTextAlign(GUI_TA_VCENTER);  
        GUI_GotoXY(210, 135+(count*15));
        if(ebuf[0] != 0)
        {
            GUI_DispString("USER PASSW "); 
            GUI_DispDecMin(count+1);
            GUI_DispString(" : ");
            GUI_DispDecMin(((ebuf[1]<<16)|(ebuf[2]<<8)|ebuf[3]));            
        } 
    }   
    hCHKBX_RTcfg = CHECKBOX_Create(370, 20, 110, 30, 0, GUI_ID_CHECK_RTcfg, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_RTcfg, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_RTcfg, "THERMOSTAT");
    hCHKBX_RCcfg = CHECKBOX_Create(370, 60, 110, 30, 0, GUI_ID_CHECK_RCcfg, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_RCcfg, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_RCcfg, "ROOM  CTRL");
    if      (IsRoomThstConfig()) CHECKBOX_SetState(hCHKBX_RTcfg, 1), CHECKBOX_SetState(hCHKBX_RCcfg, 0);
    else if (IsRoomCtrlConfig()) CHECKBOX_SetState(hCHKBX_RCcfg, 1), CHECKBOX_SetState(hCHKBX_RTcfg, 0);    
    hBUTTON_Next = BUTTON_Create(370, 160, 100, 40, GUI_ID_BUTTON_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");    
    hBUTTON_Ok = BUTTON_Create(370, 220, 100, 40, GUI_ID_BUTTON_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "OK");
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateSettings2Screen(void)
{
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    hSPNBX_DisplayHighBrightness = SPINBOX_CreateEx(10, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_DisplayHighBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayHighBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayHighBrightness, disp_high_bcklght);
    hSPNBX_DisplayLowBrightness = SPINBOX_CreateEx(10, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_DisplayLowBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayLowBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayLowBrightness, disp_low_bcklght);
    hSPNBX_ScrnsvrTimeout = SPINBOX_CreateEx(10, 130, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrTimeout, 1, 240);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrTimeout, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrTimeout, scrnsvr_tout);
    hSPNBX_ScrnsvrEnableHour = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrEnableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrEnableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrEnableHour, scrnsvr_ena_hour);
    hSPNBX_ScrnsvrDisableHour = SPINBOX_CreateEx(10, 210, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrDisableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrDisableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrDisableHour, scrnsvr_dis_hour);
    hSPNBX_Hour = SPINBOX_CreateEx(190, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Hour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_Hour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Hour, Bcd2Dec(rtctm.Hours));    
    hSPNBX_Minute = SPINBOX_CreateEx(190, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Minute, 0, 59);
    SPINBOX_SetEdge(hSPNBX_Minute, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Minute, Bcd2Dec(rtctm.Minutes));    
    hSPNBX_Day = SPINBOX_CreateEx(190, 130, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Day, 1, 31);
    SPINBOX_SetEdge(hSPNBX_Day, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Day, Bcd2Dec(rtcdt.Date));    
    hSPNBX_Month = SPINBOX_CreateEx(190, 170, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Month, 1, 12);
    SPINBOX_SetEdge(hSPNBX_Month, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Month, Bcd2Dec(rtcdt.Month));    
    hSPNBX_Year = SPINBOX_CreateEx(190, 210, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Year, 2000, 2099);
    SPINBOX_SetEdge(hSPNBX_Year, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Year, (Bcd2Dec(rtcdt.Year) + 2000));
    hSPNBX_ScrnsvrClockColour = SPINBOX_CreateEx(340, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrClockColour, 1, COLOR_BSIZE);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrClockColour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrClockColour, scrnsvr_clk_clr);
    hSPNBX_ScrnsvrSemiClkColor = SPINBOX_CreateEx(340, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrLogoClockColour, 1, COLOR_BSIZE);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrSemiClkColor, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrSemiClkColor, scrnsvr_semiclk_clr);
    hCHKBX_ScrnsvrClock = CHECKBOX_Create(340, 110, 90, 20, 0, GUI_ID_CHECK_ScrnsvrClock, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_ScrnsvrClock, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_ScrnsvrClock, "FULL CLOCK");
    if (IsScrnsvrClkActiv()) CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 1);
    else CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 0);
    hCHKBX_ScrnsvrLogoClock = CHECKBOX_Create(340, 140, 90, 20, 0, GUI_ID_CHECK_ScrnsvrLogoClock, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_ScrnsvrLogoClock, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_ScrnsvrLogoClock, "LOGO CLOCK");
    if (IsScrnsvrSemiClkActiv()) CHECKBOX_SetState(hCHKBX_ScrnsvrLogoClock, 1);
    else CHECKBOX_SetState(hCHKBX_ScrnsvrLogoClock, 0);
    hBUTTON_Next = BUTTON_Create(390, 180, 80, 30, GUI_ID_BUTTON_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(390, 230, 80, 30, GUI_ID_BUTTON_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "OK");
    GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
    GUI_FillRect(340, 51, 430, 59);
    GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
    GUI_FillRect(340, 91, 430, 99);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    /************************************/
    /* DISPLAY BACKLIGHT LED BRIGHTNESS */
    /************************************/
    GUI_DrawHLine   ( 15,   5, 160);
    GUI_GotoXY      ( 10,   5);
    GUI_DispString  ("DISPLAY BACKLIGHT");
    GUI_GotoXY      (110,  35);
    GUI_DispString  ("HIGH");
    GUI_GotoXY      (110,  75);
    GUI_DispString  ("LOW");
    GUI_DrawHLine   ( 15, 185, 320);
    GUI_GotoXY      (190,   5);
    /************************************/
    /*          SET        TIME         */
    /************************************/
    GUI_DispString  ("SET TIME");
    GUI_GotoXY      (290,  35);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (290,  75);
    GUI_DispString  ("MINUTE");
    GUI_DrawHLine   ( 15, 335, 475);
    /************************************/
    /*    SET SCREENSAVER CLOCK COOLOR  */
    /************************************/
    GUI_GotoXY      (340,   5);
    GUI_DispString  ("SET COLOR");
    GUI_GotoXY      (440, 26);
    GUI_DispString  ("FULL");
    GUI_GotoXY      (440, 38);
    GUI_DispString  ("CLOCK");
    GUI_GotoXY      (440, 66);
    GUI_DispString  ("LOGO");
    GUI_GotoXY      (440, 78);
    GUI_DispString  ("CLOCK");
    /************************************/
    /*      SCREENSAVER     OPTION      */
    /************************************/ 
    GUI_DrawHLine   (125,   5, 160);
    GUI_GotoXY      ( 10, 115);
    GUI_DispString  ("SCREENSAVER OPTION");
    GUI_GotoXY      (110, 145);
    GUI_DispString  ("TIMEOUT");
    GUI_GotoXY      (110, 176);
    GUI_DispString  ("ENABLE");
    GUI_GotoXY      (110, 188);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (110, 216);
    GUI_DispString  ("DISABLE");
    GUI_GotoXY      (110, 228);
    GUI_DispString  ("HOUR");
    /************************************/
    /*          SET        DATE         */
    /************************************/
    GUI_DrawHLine   (125, 185, 320);
    GUI_GotoXY      (190, 115);
    GUI_DispString  ("SET DATE");
    GUI_GotoXY      (290, 145);
    GUI_DispString  ("DAY");
    GUI_GotoXY      (290, 185);
    GUI_DispString  ("MONTH");
    GUI_GotoXY      (290, 225);
    GUI_DispString  ("YEAR");
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateLanguageSetScreen(void)
{
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    hBUTTON_LNGTRK = BUTTON_Create( 30,  30, 80, 80, GUI_ID_BTN_LNGTRK, WM_CF_SHOW);
    hBUTTON_LNGKSA = BUTTON_Create(140,  30, 80, 80, GUI_ID_BTN_LNGKSA, WM_CF_SHOW);
    hBUTTON_LNGITA = BUTTON_Create(250,  30, 80, 80, GUI_ID_BTN_LNGITA, WM_CF_SHOW);
    hBUTTON_LNGFRA = BUTTON_Create(360,  30, 80, 80, GUI_ID_BTN_LNGFRA, WM_CF_SHOW);
    hBUTTON_LNGENG = BUTTON_Create( 30, 140, 80, 80, GUI_ID_BTN_LNGENG, WM_CF_SHOW);
    hBUTTON_LNGGER = BUTTON_Create(140, 140, 80, 80, GUI_ID_BTN_LNGGER, WM_CF_SHOW);
    hBUTTON_LNGCHN = BUTTON_Create(250, 140, 80, 80, GUI_ID_BTN_LNGCHN, WM_CF_SHOW);
    hBUTTON_LNGBIH = BUTTON_Create(360, 140, 80, 80, GUI_ID_BTN_LNGBIH, WM_CF_SHOW);
    BUTTON_SetBMP(hBUTTON_LNGTRK, BUTTON_BI_UNPRESSED, trkflag);
    BUTTON_SetBMP(hBUTTON_LNGKSA, BUTTON_BI_UNPRESSED, ksaflag);
    BUTTON_SetBMP(hBUTTON_LNGITA, BUTTON_BI_UNPRESSED, itaflag);
    BUTTON_SetBMP(hBUTTON_LNGFRA, BUTTON_BI_UNPRESSED, fraflag);
    BUTTON_SetBMP(hBUTTON_LNGENG, BUTTON_BI_UNPRESSED, engflag);
    BUTTON_SetBMP(hBUTTON_LNGGER, BUTTON_BI_UNPRESSED, gerflag);
    BUTTON_SetBMP(hBUTTON_LNGCHN, BUTTON_BI_UNPRESSED, chnflag);
    BUTTON_SetBMP(hBUTTON_LNGBIH, BUTTON_BI_UNPRESSED, bihflag);
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPDeleteLanguageSetScreen(void)
{
    WM_DeleteWindow(hBUTTON_LNGTRK);
    WM_DeleteWindow(hBUTTON_LNGKSA);
    WM_DeleteWindow(hBUTTON_LNGITA);
    WM_DeleteWindow(hBUTTON_LNGFRA);
    WM_DeleteWindow(hBUTTON_LNGENG);
    WM_DeleteWindow(hBUTTON_LNGGER);
    WM_DeleteWindow(hBUTTON_LNGCHN);
    WM_DeleteWindow(hBUTTON_LNGBIH);
    DISPLanguageReset();
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateMaidMenuScreen(void)
{
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_BLUE); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    
    if (IsDISPCleanUpImgActiv() || IsDISPBeddRepImgActiv() || IsDISPGenCleanImgActiv())
    { 
        hBUTTON_MAIDCLR = BUTTON_Create( 30, 30, 100, 100, GUI_ID_BTN_MAIDCLR,  WM_CF_SHOW); 
    }
    else hBUTTON_MAIDCLR = BUTTON_Create(350, 30, 100, 100, GUI_ID_BTN_MAIDCLR, WM_CF_HIDE);
    
    hBUTTON_MAIDBELL    = BUTTON_Create(190, 30, 100, 100, GUI_ID_BTN_MAIDBELL, WM_CF_SHOW);
    
    if (!IsBTNBellActiv() && !IsBTNDndActiv())  
    {
        hBUTTON_MAIDOPEN = BUTTON_Create(350, 30, 100, 100, GUI_ID_BTN_MAIDOPEN, WM_CF_SHOW);
    }
    else hBUTTON_MAIDOPEN = BUTTON_Create(350, 30, 100, 100, GUI_ID_BTN_MAIDOPEN, WM_CF_HIDE);
    BUTTON_SetFont(hBUTTON_MAIDBELL, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_MAIDCLR,  GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_MAIDOPEN, GUI_FONT_24B_1);
    BUTTON_SetText(hBUTTON_MAIDBELL, "DOOR\n\rBELL");
    BUTTON_SetText(hBUTTON_MAIDCLR,  "CLEAR\n\rSERVICE");
    BUTTON_SetText(hBUTTON_MAIDOPEN, "DOOR\n\rOPEN");
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    GUI_GotoXY(235, 180);
    GUI_DispString("AVAILABLE MAID OPTIONS");
    GUI_MULTIBUF_EndEx(1);
    keypadtout = KEYPAD_UNLOCK_TIME;
    keypadtmr = HAL_GetTick();
    DISPMaidMenuSet();
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPDeleteMaidMenuScreen(void)
{
    WM_DeleteWindow(hBUTTON_MAIDBELL);
    WM_DeleteWindow(hBUTTON_MAIDCLR);
    WM_DeleteWindow(hBUTTON_MAIDOPEN);
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    DISPMaidMenuReset();
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPDeleteSettings1Screen(void)
{
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hCHKBX_RTcfg);
    WM_DeleteWindow(hCHKBX_RCcfg);
    WM_DeleteWindow(hSPNBX_MaxSetpoint);
    WM_DeleteWindow(hSPNBX_MinSetpoint);
    WM_DeleteWindow(hSPNBX_AmbientNtcOffset);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPDeleteSettings2Screen(void)
{
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hSPNBX_DisplayHighBrightness);
    WM_DeleteWindow(hSPNBX_DisplayLowBrightness);
    WM_DeleteWindow(hCHKBX_ScrnsvrClock);
    WM_DeleteWindow(hCHKBX_ScrnsvrLogoClock);
    WM_DeleteWindow(hSPNBX_ScrnsvrTimeout);
    WM_DeleteWindow(hSPNBX_ScrnsvrEnableHour);
    WM_DeleteWindow(hSPNBX_ScrnsvrDisableHour);
    WM_DeleteWindow(hSPNBX_Hour);
    WM_DeleteWindow(hSPNBX_Minute);
    WM_DeleteWindow(hSPNBX_Day);
    WM_DeleteWindow(hSPNBX_Month);
    WM_DeleteWindow(hSPNBX_Year);
    WM_DeleteWindow(hSPNBX_ScrnsvrClockColour);
    WM_DeleteWindow(hSPNBX_ScrnsvrSemiClkColor);
}
/**
  * @brief
  * @param
  * @retval
  */
static void PID_Hook(GUI_PID_STATE * pState) 
{
    if (pState->Pressed  == 1U)
    {
        pState->Layer = 1U;
        DISPResetScrnsvr();
        
        if ((pState->x >= BTN_SETTINGS_X0) 
        &&  (pState->y >= BTN_SETTINGS_Y0) 
        &&  (pState->x <  BTN_SETTINGS_X1) 
        &&  (pState->y <  BTN_SETTINGS_Y1)) 
        {
            btn_settings = 1U;
        }
        
        if ((pState->x >= BTN_INC_X0) 
        && (pState->y >= BTN_INC_Y0) 
        &&  (pState->x <  BTN_INC_X1)
        &&  (pState->y <  BTN_INC_Y1)) 
        {	
            btn_increase_state = 1U;   
        }
        
        if ((pState->x >= BTN_DEC_X0)
        &&  (pState->y >= BTN_DEC_Y0)
        &&  (pState->x <  BTN_DEC_X1) 
        &&  (pState->y <  BTN_DEC_Y1)) 
        {	
            btn_decrease_state = 1U;
        }  
        if ((pState->x >= BTN_DOOR_X0)
        &&  (pState->y >= BTN_DOOR_Y0) 
        &&  (pState->x <  BTN_DOOR_X1)
        &&  (pState->y <  BTN_DOOR_Y1)) 
        {
            btn_opendoor_state = 1U;
        }
           
        if ((pState->x >= BTN_OK_X0)
        &&  (pState->y >= BTN_OK_Y0)
        &&  (pState->x <  BTN_OK_X1)
        &&  (pState->y <  BTN_OK_Y1)) 
        {
            btn_ok_state = 1U;
        } 
        
        if ((pState->x >= BTN_KEYPAD_X0)
        &&  (pState->y >= BTN_KEYPAD_Y0)
        &&  (pState->x <  BTN_KEYPAD_X1)
        &&  (pState->y <  BTN_KEYPAD_Y1)) 
        {
            btn_keypad = 1U;
        }
        
        if ((pState->x >= BTN_DND_X0)
        &&  (pState->y >= BTN_DND_Y0)
        &&  (pState->x <  BTN_DND_X1)
        &&  (pState->y <  BTN_DND_Y1)) 
        {
            btn_dnd_state = 1U;
        }
        
        if ((pState->x >= BTN_MAID_X0) 
        &&  (pState->y >= BTN_MAID_Y0) 
        &&  (pState->x <  BTN_MAID_X1) 
        &&  (pState->y <  BTN_MAID_Y1)) 
        {
            btn_maid_state = 1U;
        }
        
        if ((pState->x >= BTN_SOS_X0) 
        &&  (pState->y >= BTN_SOS_Y0) 
        &&  (pState->x <  BTN_SOS_X1) 
        &&  (pState->y <  BTN_SOS_Y1)) 
        {
            btn_sos_state = 1U;
        }
        
        if ((pState->x >= BTN_ROOM_X0)
        &&  (pState->y >= BTN_ROOM_Y0)
        &&  (pState->x <  BTN_ROOM_X1)
        &&  (pState->y <  BTN_ROOM_Y1)) 
        {
            btn_doorbell = 1U;
        }
    }
    else
    {
        btn_keypad = 0U;
        btn_doorbell = 0U;
        btn_ok_state = 0U; 
        btn_settings = 0U;
        btn_sos_state = 0U; 
        btn_dnd_state = 0U;
        btn_maid_state = 0U;
        btn_decrease_state = 0U;   
        btn_increase_state = 0U;
        btn_opendoor_state = 0U;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISPMenuSettings(uint8_t btn)
{
    static uint8_t last_state = 0U;
    static uint32_t menu_tmr = 0U;
    
    if      ((btn == 1U) && (last_state == 0U))
    {
        last_state = 1U;
        menu_tmr = HAL_GetTick(); 
    }
    else if ((btn == 1U) && (last_state == 1U))
    {
        if((HAL_GetTick() - menu_tmr) >= SETTINGS_MENU_ENABLE_TIME)
        {
            last_state = 0U;
            return (1U);
        }
    }
    else if ((btn == 0U) && (last_state == 1U)) last_state = 0U;
    
    return (0U);
}

/**
  * @brief
  * @param
  * @retval
  */
static void DISPSaveSettings(void)
{
    uint8_t sbuf[18];
    sbuf[0]  = (sysfl >> 24);    
    sbuf[1]  = (sysfl >> 16);
    sbuf[2]  = (sysfl >>  8);
    sbuf[3]  = (sysfl & 0xff);
    sbuf[4] = thst_min_sp;
    sbuf[5] = thst_max_sp;
    sbuf[6] = thst_sp;
    sbuf[7] = ntc_offset;
    sbuf[10]= disp_low_bcklght;
    sbuf[11]= disp_high_bcklght;
    sbuf[12]= scrnsvr_tout;
    sbuf[13]= scrnsvr_ena_hour;
    sbuf[14]= scrnsvr_dis_hour;
    sbuf[15]= scrnsvr_clk_clr;
    sbuf[16]= scrnsvr_semiclk_clr;
    sbuf[17]= sysfl & 0xFF;
    EE_WriteBuffer(sbuf, EE_THST_FLAGS, 18);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPCreateKeypad(void)
{
	GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_BLUE); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    
	hBUTTON_OSK_1   = BUTTON_Create(100,  50, 60, 60, GUI_ID_BTN_OSK_1, WM_CF_SHOW);	
	hBUTTON_OSK_2   = BUTTON_Create(170,  50, 60, 60, GUI_ID_BTN_OSK_2, WM_CF_SHOW);	
	hBUTTON_OSK_3   = BUTTON_Create(240,  50, 60, 60, GUI_ID_BTN_OSK_3, WM_CF_SHOW);	
	hBUTTON_OSK_4   = BUTTON_Create(310,  50, 60, 60, GUI_ID_BTN_OSK_4, WM_CF_SHOW);	
	hBUTTON_OSK_5   = BUTTON_Create(100, 120, 60, 60, GUI_ID_BTN_OSK_5, WM_CF_SHOW);	
	hBUTTON_OSK_6   = BUTTON_Create(170, 120, 60, 60, GUI_ID_BTN_OSK_6, WM_CF_SHOW);	
	hBUTTON_OSK_7   = BUTTON_Create(240, 120, 60, 60, GUI_ID_BTN_OSK_7, WM_CF_SHOW);	
	hBUTTON_OSK_8   = BUTTON_Create(310, 120, 60, 60, GUI_ID_BTN_OSK_8, WM_CF_SHOW);
    hBUTTON_OSK_BK  = BUTTON_Create(100, 190, 60, 60, GUI_ID_BTN_OSK_BK,WM_CF_SHOW);
	hBUTTON_OSK_9   = BUTTON_Create(170, 190, 60, 60, GUI_ID_BTN_OSK_9, WM_CF_SHOW);
    hBUTTON_OSK_0   = BUTTON_Create(240, 190, 60, 60, GUI_ID_BTN_OSK_0, WM_CF_SHOW);
	hBUTTON_OSK_OK  = BUTTON_Create(310, 190, 60, 60, GUI_ID_BTN_OSK_OK,WM_CF_SHOW);
    
    BUTTON_SetFont(hBUTTON_OSK_0, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_1, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_2, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_3, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_4, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_5, GUI_FONT_24B_1);    
    BUTTON_SetFont(hBUTTON_OSK_6, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_7, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_8, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_9, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_OK,GUI_FONT_20B_1);
    BUTTON_SetFont(hBUTTON_OSK_BK,GUI_FONT_20B_1);
    
    BUTTON_SetText(hBUTTON_OSK_0, "0");
    BUTTON_SetText(hBUTTON_OSK_1, "1");
    BUTTON_SetText(hBUTTON_OSK_2, "2");	
    BUTTON_SetText(hBUTTON_OSK_3, "3");
    BUTTON_SetText(hBUTTON_OSK_4, "4");
    BUTTON_SetText(hBUTTON_OSK_5, "5");
    BUTTON_SetText(hBUTTON_OSK_6, "6");
    BUTTON_SetText(hBUTTON_OSK_7, "7");
    BUTTON_SetText(hBUTTON_OSK_8, "8");
    BUTTON_SetText(hBUTTON_OSK_9, "9");
    BUTTON_SetText(hBUTTON_OSK_OK,"OK");
	BUTTON_SetText(hBUTTON_OSK_BK,"BCK");
    
    GUI_GotoXY(235, 20);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32_1);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    GUI_DispString("ENTER PASSWORD");
    GUI_MULTIBUF_EndEx(1);
    DISPKeypadSet();
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPDeleteKeypad(void)
{
	WM_DeleteWindow(hBUTTON_OSK_0);
	WM_DeleteWindow(hBUTTON_OSK_1);
	WM_DeleteWindow(hBUTTON_OSK_2);
	WM_DeleteWindow(hBUTTON_OSK_3);
	WM_DeleteWindow(hBUTTON_OSK_4);
	WM_DeleteWindow(hBUTTON_OSK_5);
	WM_DeleteWindow(hBUTTON_OSK_6);
	WM_DeleteWindow(hBUTTON_OSK_7);
	WM_DeleteWindow(hBUTTON_OSK_8);
	WM_DeleteWindow(hBUTTON_OSK_9);
	WM_DeleteWindow(hBUTTON_OSK_OK);
	WM_DeleteWindow(hBUTTON_OSK_BK);
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    DISPKeypadReset();
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISPKeypad(char key)
{
	uint8_t  ebuf[16];
    uint32_t loop, unixrtc;
    int hmdpsw, mgrpsw, srvpwd, kpdpwd;
    BUZZ_State = BUZZ_CLICK;
    
    if((key == 'B') || (key == 'O'))		
    {
        GUI_SetColor(GUI_BLUE);
        GUI_FillRect(100, 0, 370, 50);       
        if (key == 'B')
        {
            oskin[oskb] = NUL;
            if(oskb > 0)
            {
                --oskb;
                loop = 0;
                while(loop < oskb)
                {
                    GUI_GotoXY      (235, 20);
                    GUI_SetColor    (GUI_WHITE);
                    GUI_SetFont     (&GUI_Font32_1);
                    GUI_SetTextMode (GUI_TEXTMODE_TRANS);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_DispCharAt  ('*',loop*50+130,15);
                    ++loop;
                }
            }
            oskin[oskb] = NUL;
            keypadtout = KEYPAD_UNLOCK_TIME;
        }
        else if (key == 'O')
        {
            oskin[5] = 0;
            kpdpwd = Str2Int(oskin, pswb);
            unixrtc = rtc2unix(&rtctm, &rtcdt);
            EE_ReadBuffer(ebuf, EE_MAID_PSWRD, 3);
            hmdpsw = ((ebuf[0]<<16)|(ebuf[1]<<8)|ebuf[2]);
            EE_ReadBuffer(ebuf, EE_MNGR_PSWRD, 3);
            mgrpsw = ((ebuf[0]<<16)|(ebuf[1]<<8)|ebuf[2]);
            EE_ReadBuffer(ebuf, EE_SRVC_PSWRD, 3);
            srvpwd = ((ebuf[0]<<16)|(ebuf[1]<<8)|ebuf[2]);                        
            GUI_GotoXY      (235, 20);
            GUI_SetColor    (GUI_WHITE);
            GUI_SetFont     (&GUI_Font32B_1);
            GUI_SetTextMode (GUI_TEXTMODE_TRANS);
            GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
            if      ((kpdpwd == hmdpsw)  &&  (hmdpsw > 0) && (hmdpsw < 100000))  
            {
                GUI_DispString("MAID PASSWORD OK");
                LogEvent.log_group = USERGRP_MAID;
                LogEvent.log_event = PASSWORD_VALID;
                LOGGER_Write();
                BUZZ_State = BUZZ_CARD_VALID;
                DISPMaidMenuSet();
            }
            else if ((kpdpwd == mgrpsw)  &&  (mgrpsw > 0) && (mgrpsw < 100000))  
            {
                GUI_DispString("MANAGER PASSWORD OK");
                LogEvent.log_group = USERGRP_MANAGER;
                LogEvent.log_event = PASSWORD_VALID;
                LOGGER_Write();
                DooLockTimeSet();
                DoorLockOn();
                CARD_Clear();
                BUZZ_State = BUZZ_CARD_VALID;
            }
            else if ((kpdpwd == srvpwd)  &&  (srvpwd > 0) && (srvpwd < 100000))  
            {
                GUI_DispString("SERVICE PASSWORD OK");
                LogEvent.log_group = USERGRP_SERVICE;
                LogEvent.log_event = PASSWORD_VALID;
                LOGGER_Write();
                DooLockTimeSet();
                DoorLockOn();
                CARD_Clear();
                BUZZ_State = BUZZ_CARD_VALID;
                DISPSettingsInitSet();
            }
            else 
            {
                // loop to check users 1 - 8 passwords
                for (loop = 0; loop < 8; loop++)
                {
                    EE_ReadBuffer(ebuf, (EE_USER_PSWRD1+(loop*8)), 8);
                    if (ebuf[0] == (loop+1U))
                    {
                        uint32_t usrpsw = ((ebuf[1]<<16)|(ebuf[2]<< 8)| ebuf[3]);
                        uint32_t usrtime =((ebuf[4]<<24)|(ebuf[5]<<16)|(ebuf[6]<<8)|ebuf[7]);
                        if ((kpdpwd == usrpsw) && (usrpsw > 0)&&(usrpsw < 100000)) 
                        {
                            if (usrtime > unixrtc)
                            {
                                loop = 8; // exit password search loop
                                GUI_DispString("USER PASSWORD OK");
                                mem_cpy(LogEvent.log_card_id, ebuf, 4);
                                LogEvent.log_group = USERGRP_GUEST;
                                LogEvent.log_event = PASSWORD_VALID;
                                LOGGER_Write();
                                DooLockTimeSet();
                                DoorLockOn();
                                CARD_Clear();
                                BUZZ_State = BUZZ_CARD_VALID;
                            }
                        }
                    } 
                }    
            }
            // if keyin password valid open door lock, write log and make some noise
            
            if (BUZZ_State != BUZZ_CARD_VALID) // or wish more lacky next time
            {
                BUZZ_State = BUZZ_CARD_INVALID;
                GUI_GotoXY      (235, 20);
                GUI_SetColor    (GUI_RED);
                GUI_SetFont     (&GUI_Font32B_1);
                GUI_SetTextMode (GUI_TEXTMODE_TRANS);
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_DispString  ("WRONG PASSWORD");
                LogEvent.log_event = PASSWORD_INVALID;
                LOGGER_Write();
            }            
            
            keypadtout = KEYPAD_SIGNAL_TIME;
        }
    }
    else if (ISVALIDDEC(key))
    {
        if(oskb == 0U)
        {
            GUI_SetColor(GUI_BLUE);
            GUI_FillRect(100, 0, 370, 50);
        }
        GUI_SetColor    (GUI_WHITE);
        GUI_SetFont     (&GUI_Font32_1);
        GUI_SetTextMode (GUI_TEXTMODE_TRANS);
        GUI_SetTextAlign(GUI_TA_VCENTER);
        if(oskb < 5) GUI_DispCharAt('*', ((oskb*50U)+ 130U), 15U);
        oskin[oskb++] = key;
        if(oskb > 5) --oskb;
        keypadtout = KEYPAD_UNLOCK_TIME;
    }
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
    keypadtmr = HAL_GetTick();
}
/**
  * @brief
  * @param
  * @retval
  */
void DISPGuiSet(uint8_t id)
{
    last_img_id     = id;
    disp_img_id     = id;
    disp_img_time   = 0U;
    DISPUpdateSet();
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

