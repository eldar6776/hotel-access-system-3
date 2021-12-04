/*
----------------------------------------------------------------------
File        : Resource.h
Content     : Main resource header file of weather forecast demo
---------------------------END-OF-HEADER------------------------------
*/

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdlib.h>
#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

extern const unsigned long disp_00_size;
extern const unsigned long disp_01_size;
extern const unsigned long disp_02_size;
extern const unsigned long disp_03_size;
extern const unsigned long disp_04_size;
extern const unsigned long disp_05_size;
extern const unsigned long disp_10_size;
extern const unsigned long disp_11_size;
extern const unsigned long disp_12_size;
extern const unsigned long disp_13_size;
extern const unsigned long disp_14_size;
extern const unsigned long disp_15_size;
//extern const unsigned long disp_20_size;
extern const unsigned long disp_21_size;
extern const unsigned long disp_22_size;
extern const unsigned long disp_23_size;
extern const unsigned long disp_24_size;
extern const unsigned long disp_25_size;
//extern const unsigned long disp_30_size;
extern const unsigned long disp_31_size;
extern const unsigned long disp_32_size;
extern const unsigned long disp_33_size;
extern const unsigned long disp_34_size;
extern const unsigned long disp_35_size;
//extern const unsigned long disp_40_size;
extern const unsigned long disp_41_size;
extern const unsigned long disp_42_size;
extern const unsigned long disp_43_size;
extern const unsigned long disp_44_size;
extern const unsigned long disp_45_size;
//extern const unsigned long disp_50_size;
//extern const unsigned long disp_51_size;
//extern const unsigned long disp_52_size;
//extern const unsigned long disp_53_size;
//extern const unsigned long disp_54_size;
//extern const unsigned long disp_55_size;
//extern const unsigned long disp_60_size;
extern const unsigned long disp_61_size;
extern const unsigned long disp_62_size;
extern const unsigned long disp_63_size;
extern const unsigned long disp_64_size;
extern const unsigned long disp_65_size;
//extern const unsigned long disp_70_size;
extern const unsigned long disp_71_size;
extern const unsigned long disp_72_size;
extern const unsigned long disp_73_size;
extern const unsigned long disp_74_size;
extern const unsigned long disp_75_size;
//extern const unsigned long disp_80_size;
extern const unsigned long disp_81_size;
extern const unsigned long disp_82_size;
extern const unsigned long disp_83_size;
extern const unsigned long disp_84_size;
extern const unsigned long disp_85_size;

extern const unsigned long trkflag_size;
extern const unsigned long ksaflag_size;
extern const unsigned long itaflag_size;
extern const unsigned long fraflag_size;
extern const unsigned long engflag_size;
extern const unsigned long gerflag_size;
extern const unsigned long chnflag_size;
extern const unsigned long bihflag_size;
extern const unsigned long cleaning_size;
extern const unsigned long bedding_size;
extern const unsigned long general_size;
extern const unsigned long logo_size;
extern const unsigned long keypad_size;
extern const unsigned long language_size;
extern const unsigned long doorbell_size;
extern const unsigned long dnd_size;
//extern const unsigned long sloflag_size;

//extern const unsigned char sloflag[];
extern const unsigned char trkflag[];
extern const unsigned char ksaflag[];
extern const unsigned char itaflag[];
extern const unsigned char fraflag[];
extern const unsigned char engflag[];
extern const unsigned char gerflag[];
extern const unsigned char chnflag[];
extern const unsigned char bihflag[];
extern const unsigned char disp_00[];
extern const unsigned char disp_01[];
extern const unsigned char disp_02[];
extern const unsigned char disp_03[];
extern const unsigned char disp_04[];
extern const unsigned char disp_05[];
extern const unsigned char disp_10[];
extern const unsigned char disp_11[];
extern const unsigned char disp_12[];
extern const unsigned char disp_13[];
extern const unsigned char disp_14[];
extern const unsigned char disp_15[];
//extern const unsigned char disp_20[];
extern const unsigned char disp_21[];
extern const unsigned char disp_22[];
extern const unsigned char disp_23[];
extern const unsigned char disp_24[];
extern const unsigned char disp_25[];
//extern const unsigned char disp_30[];
extern const unsigned char disp_31[];
extern const unsigned char disp_32[];
extern const unsigned char disp_33[];
extern const unsigned char disp_34[];
extern const unsigned char disp_35[];
//extern const unsigned char disp_40[];
extern const unsigned char disp_41[];
extern const unsigned char disp_42[];
extern const unsigned char disp_43[];
extern const unsigned char disp_44[];
extern const unsigned char disp_45[];
//extern const unsigned char disp_50[];
//extern const unsigned char disp_51[];
//extern const unsigned char disp_52[];
//extern const unsigned char disp_53[];
//extern const unsigned char disp_54[];
//extern const unsigned char disp_55[];
//extern const unsigned char disp_60[];
extern const unsigned char disp_61[];
extern const unsigned char disp_62[];
extern const unsigned char disp_63[];
extern const unsigned char disp_64[];
extern const unsigned char disp_65[];
//extern const unsigned char disp_70[];
extern const unsigned char disp_71[];
extern const unsigned char disp_72[];
extern const unsigned char disp_73[];
extern const unsigned char disp_74[];
extern const unsigned char disp_75[];
//extern const unsigned char disp_80[];
extern const unsigned char disp_81[];
extern const unsigned char disp_82[];
extern const unsigned char disp_83[];
extern const unsigned char disp_84[];
extern const unsigned char disp_85[];
//extern const unsigned char roomctrl_bckgnd[];
extern const unsigned char cleaning[];
extern const unsigned char bedding[];
extern const unsigned char general[];
extern const unsigned char logo[];
extern const unsigned char keypad[];
extern const unsigned char language[];
extern const unsigned char dnd[];
extern const unsigned char doorbell[];

extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_ok;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_dnd_0;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_dnd_1;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_maid_0;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_maid_1;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_weather;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_rst_sos_1;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_door_open;

extern GUI_CONST_STORAGE GUI_BITMAP bmmist_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmmist_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmrain_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmrain_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmsnow_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmsnow_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmclear_sky_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmclear_sky_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmfew_clouds_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmfew_clouds_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmshower_rain_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmshower_rain_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmthunderstorm_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmthunderstorm_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmscattered_clouds_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmscattered_clouds_icon;

#endif // RESOURCE_H
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
