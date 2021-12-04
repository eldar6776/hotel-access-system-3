	AREA    gui_bmp, DATA, READONLY
	EXPORT  disp_00
	EXPORT  disp_01
	EXPORT  disp_02
	EXPORT  disp_03
	EXPORT  disp_04
	EXPORT  disp_05
	EXPORT  disp_10
	EXPORT  disp_11
	EXPORT  disp_12
	EXPORT  disp_13
	EXPORT  disp_14
	EXPORT  disp_15
	EXPORT  disp_21
	EXPORT  disp_22
	EXPORT  disp_23
	EXPORT  disp_24
	EXPORT  disp_25
	EXPORT  disp_31
	EXPORT  disp_32
	EXPORT  disp_33
	EXPORT  disp_34
	EXPORT  disp_35
	EXPORT  disp_41
	EXPORT  disp_42
	EXPORT  disp_43
	EXPORT  disp_44
	EXPORT  disp_45
;	EXPORT  disp_51
;	EXPORT  disp_52
;	EXPORT  disp_53
;	EXPORT  disp_54
;	EXPORT  disp_55
	EXPORT  disp_61
	EXPORT  disp_62
	EXPORT  disp_63
	EXPORT  disp_64
	EXPORT  disp_65
	EXPORT  disp_71
	EXPORT  disp_72
	EXPORT  disp_73
	EXPORT  disp_74
	EXPORT  disp_75            
	EXPORT  disp_81
	EXPORT  disp_82
	EXPORT  disp_83
	EXPORT  disp_84
	EXPORT  disp_85
    EXPORT  trkflag
    EXPORT  ksaflag
    EXPORT  itaflag
    EXPORT  fraflag
    EXPORT  engflag
    EXPORT  gerflag
    EXPORT  chnflag
    EXPORT  bihflag
    EXPORT  keypad
    EXPORT  language
    EXPORT  cleaning
    EXPORT  bedding
    EXPORT  general
    EXPORT  dnd
    EXPORT  doorbell

disp_00
	INCBIN  ../Src/Display/display_00.bmp
	ALIGN
disp_00_end
disp_00_size
	DCD     disp_00_end - disp_00
		
disp_01
	INCBIN  ../Src/Display/display_01.bmp
	ALIGN
disp_01_end
disp_01_size
	DCD     disp_01_end - disp_01

disp_02
	INCBIN  ../Src/Display/display_02.bmp
	ALIGN
disp_02_end
disp_02_size
	DCD     disp_02_end - disp_02
	
disp_03
	INCBIN  ../Src/Display/display_03.bmp
	ALIGN
disp_03_end
disp_03_size
	DCD     disp_03_end - disp_03
		
disp_04
	INCBIN  ../Src/Display/display_04.bmp
	ALIGN
disp_04_end
disp_04_size
	DCD     disp_04_end - disp_04
	
disp_05
	INCBIN  ../Src/Display/display_05.bmp
	ALIGN
disp_05_end
disp_05_size
	DCD     disp_05_end - disp_05

disp_10
	INCBIN  ../Src/Display/display_10.bmp
	ALIGN
disp_10_end
disp_10_size
	DCD     disp_10_end - disp_10
	
disp_11
	INCBIN  ../Src/Display/display_11.bmp
	ALIGN
disp_11_end
disp_11_size
	DCD     disp_11_end - disp_11

disp_12
	INCBIN  ../Src/Display/display_12.bmp
	ALIGN
disp_12_end
disp_12_size
	DCD     disp_12_end - disp_12
	
disp_13
	INCBIN  ../Src/Display/display_13.bmp
	ALIGN
disp_13_end
disp_13_size
	DCD     disp_13_end - disp_13
	
disp_14
	INCBIN  ../Src/Display/display_14.bmp
	ALIGN
disp_14_end
disp_14_size
	DCD     disp_14_end - disp_14
	
disp_15
	INCBIN  ../Src/Display/display_15.bmp
	ALIGN
disp_15_end
disp_15_size
	DCD     disp_15_end - disp_15
	
disp_21
	INCBIN  ../Src/Display/display_21.bmp
	ALIGN
disp_21_end
disp_21_size
	DCD     disp_21_end - disp_21
	
disp_22
	INCBIN  ../Src/Display/display_22.bmp
	ALIGN
disp_22_end
disp_22_size
	DCD     disp_22_end - disp_22
	
disp_23
	INCBIN  ../Src/Display/display_23.bmp
	ALIGN
disp_23_end
disp_23_size
	DCD     disp_23_end - disp_23
	
disp_24
	INCBIN  ../Src/Display/display_24.bmp
	ALIGN
disp_24_end
disp_24_size
	DCD     disp_24_end - disp_24
	
disp_25
	INCBIN  ../Src/Display/display_25.bmp
	ALIGN
disp_25_end
disp_25_size
	DCD     disp_25_end - disp_25

disp_31
	INCBIN  ../Src/Display/display_31.bmp
	ALIGN
disp_31_end
disp_31_size
	DCD     disp_31_end - disp_31
	
disp_32
	INCBIN  ../Src/Display/display_32.bmp
	ALIGN
disp_32_end
disp_32_size
	DCD     disp_32_end - disp_32
	
disp_33
	INCBIN  ../Src/Display/display_33.bmp
	ALIGN
disp_33_end
disp_33_size
	DCD     disp_33_end - disp_33
	
disp_34
	INCBIN  ../Src/Display/display_34.bmp
	ALIGN
disp_34_end
disp_34_size
	DCD     disp_34_end - disp_34
	
disp_35
	INCBIN  ../Src/Display/display_35.bmp
	ALIGN
disp_35_end
disp_35_size
	DCD     disp_35_end - disp_35


disp_41
	INCBIN  ../Src/Display/display_41.bmp
	ALIGN
disp_41_end
disp_41_size
	DCD     disp_41_end - disp_41
	
disp_42
	INCBIN  ../Src/Display/display_42.bmp
	ALIGN
disp_42_end
disp_42_size
	DCD     disp_42_end - disp_42
	
disp_43
	INCBIN  ../Src/Display/display_43.bmp
	ALIGN
disp_43_end
disp_43_size
	DCD     disp_43_end - disp_43
	
disp_44
	INCBIN  ../Src/Display/display_44.bmp
	ALIGN
disp_44_end
disp_44_size
	DCD     disp_44_end - disp_44
	
disp_45
	INCBIN  ../Src/Display/display_45.bmp
	ALIGN
disp_45_end
disp_45_size
	DCD     disp_45_end - disp_45

;disp_51
;	INCBIN  ../Src/Display/display_51.bmp
;	ALIGN
;disp_51_end
;disp_51_size
;	DCD     disp_51_end - disp_51
;	
;disp_52
;	INCBIN  ../Src/Display/display_52.bmp
;	ALIGN
;disp_52_end
;disp_52_size
;	DCD     disp_52_end - disp_52
;	
;disp_53
;	INCBIN  ../Src/Display/display_53.bmp
;	ALIGN
;disp_53_end
;disp_53_size
;	DCD     disp_53_end - disp_53
;	
;disp_54
;	INCBIN  ../Src/Display/display_54.bmp
;	ALIGN
;disp_54_end
;disp_54_size
;	DCD     disp_54_end - disp_54
;	
;disp_55
;	INCBIN  ../Src/Display/display_55.bmp
;	ALIGN
;disp_55_end
;disp_55_size
;	DCD     disp_55_end - disp_55
        
disp_61
	INCBIN  ../Src/Display/display_61.bmp
	ALIGN
disp_61_end
disp_61_size
	DCD     disp_61_end - disp_61

disp_62
	INCBIN  ../Src/Display/display_62.bmp
	ALIGN
disp_62_end
disp_62_size
	DCD     disp_62_end - disp_62

disp_63
	INCBIN  ../Src/Display/display_63.bmp
	ALIGN
disp_63_end
disp_63_size
	DCD     disp_63_end - disp_63

disp_64
	INCBIN  ../Src/Display/display_64.bmp
	ALIGN
disp_64_end
disp_64_size
	DCD     disp_64_end - disp_64

disp_65
	INCBIN  ../Src/Display/display_65.bmp
	ALIGN
disp_65_end
disp_65_size
	DCD     disp_65_end - disp_65

disp_71
	INCBIN  ../Src/Display/display_71.bmp
	ALIGN
disp_71_end
disp_71_size
	DCD     disp_71_end - disp_71

disp_72
	INCBIN  ../Src/Display/display_72.bmp
	ALIGN
disp_72_end
disp_72_size
	DCD     disp_72_end - disp_72

disp_73
	INCBIN  ../Src/Display/display_73.bmp
	ALIGN
disp_73_end
disp_73_size
	DCD     disp_73_end - disp_73
		
disp_74
	INCBIN  ../Src/Display/display_74.bmp
	ALIGN
disp_74_end
disp_74_size
	DCD     disp_74_end - disp_74

disp_75
	INCBIN  ../Src/Display/display_75.bmp
	ALIGN
disp_75_end
disp_75_size
	DCD     disp_75_end - disp_75

disp_81
	INCBIN  ../Src/Display/display_81.bmp
	ALIGN
disp_81_end
disp_81_size
	DCD     disp_81_end - disp_81
		
disp_82
	INCBIN  ../Src/Display/display_82.bmp
	ALIGN
disp_82_end
disp_82_size
	DCD     disp_82_end - disp_82

disp_83
	INCBIN  ../Src/Display/display_83.bmp
	ALIGN
disp_83_end
disp_83_size
	DCD     disp_83_end - disp_83

disp_84
	INCBIN  ../Src/Display/display_84.bmp
	ALIGN
disp_84_end
disp_84_size
	DCD     disp_84_end - disp_84

disp_85
	INCBIN  ../Src/Display/display_85.bmp
	ALIGN
disp_85_end
disp_85_size
	DCD     disp_85_end - disp_85

trkflag
    INCBIN  ../Src/Display/trkflag.bmp
	ALIGN
trkflag_end
trkflag_size
	DCD     trkflag_end - trkflag
        
ksaflag
    INCBIN  ../Src/Display/ksaflag.bmp
	ALIGN
ksaflag_end
ksaflag_size
	DCD     ksaflag_end - ksaflag

engflag
    INCBIN  ../Src/Display/engflag.bmp
	ALIGN
engflag_end
engflag_size
	DCD     engflag_end - engflag

gerflag
    INCBIN  ../Src/Display/gerflag.bmp
	ALIGN
gerflag_end
gerflag_size
	DCD     gerflag_end - gerflag
        
chnflag
    INCBIN  ../Src/Display/chnflag.bmp
	ALIGN
chnflag_end
chnflag_size
	DCD     chnflag_end - chnflag

bihflag
    INCBIN  ../Src/Display/bihflag.bmp
	ALIGN
bihflag_end
bihflag_size
	DCD     bihflag_end - bihflag

itaflag
    INCBIN  ../Src/Display/itaflag.bmp
	ALIGN
itaflag_end
itaflag_size
	DCD     itaflag_end - itaflag
        
fraflag
    INCBIN  ../Src/Display/fraflag.bmp
	ALIGN
fraflag_end
fraflag_size
	DCD     fraflag_end - fraflag 
        
keypad
	INCBIN  ../Src/Display/keypad.png
	ALIGN
keypad_end
keypad_size
	DCD     keypad_end - keypad

language
	INCBIN  ../Src/Display/language.png
	ALIGN
language_end
language_size
	DCD     language_end - language
        
cleaning
	INCBIN  ../Src/Display/cleaning.png
	ALIGN
cleaning_end
cleaning_size
	DCD     cleaning_end - cleaning

bedding
	INCBIN  ../Src/Display/bedding.png
	ALIGN
bedding_end
bedding_size
	DCD     bedding_end - bedding
        
general
	INCBIN  ../Src/Display/general.png
	ALIGN
general_end
general_size
	DCD     general_end - general

doorbell
    INCBIN  ../Src/Display/doorbell.png
	ALIGN
doorbell_end
doorbell_size
	DCD     doorbell_end - doorbell
        
dnd
    INCBIN  ../Src/Display/dnd.png
	ALIGN
dnd_end
dnd_size
	DCD     dnd_end - dnd
        
	EXPORT  disp_00_size            
	EXPORT  disp_01_size
	EXPORT  disp_02_size            
	EXPORT  disp_03_size
	EXPORT  disp_04_size            
	EXPORT  disp_05_size
	EXPORT  disp_10_size            
	EXPORT  disp_11_size
	EXPORT  disp_12_size            
	EXPORT  disp_13_size
	EXPORT  disp_14_size            
	EXPORT  disp_15_size            
	EXPORT  disp_21_size
	EXPORT  disp_22_size            
	EXPORT  disp_23_size
	EXPORT  disp_24_size            
	EXPORT  disp_25_size
	EXPORT  disp_31_size
	EXPORT  disp_32_size            
	EXPORT  disp_33_size
	EXPORT  disp_34_size            
	EXPORT  disp_35_size        
    EXPORT  disp_41_size
	EXPORT  disp_42_size            
	EXPORT  disp_43_size
	EXPORT  disp_44_size            
	EXPORT  disp_45_size       
;    EXPORT  disp_51_size
;	EXPORT  disp_52_size            
;	EXPORT  disp_53_size
;	EXPORT  disp_54_size            
;	EXPORT  disp_55_size        
	EXPORT  disp_61_size
	EXPORT  disp_62_size            
	EXPORT  disp_63_size
	EXPORT  disp_64_size            
	EXPORT  disp_65_size           
	EXPORT  disp_71_size
	EXPORT  disp_72_size            
	EXPORT  disp_73_size
	EXPORT  disp_74_size            
	EXPORT  disp_75_size           
	EXPORT  disp_81_size
	EXPORT  disp_82_size            
	EXPORT  disp_83_size
	EXPORT  disp_84_size            
	EXPORT  disp_85_size
    EXPORT  trkflag_size
    EXPORT  ksaflag_size
    EXPORT  itaflag_size
    EXPORT  fraflag_size
    EXPORT  engflag_size
    EXPORT  gerflag_size
    EXPORT  chnflag_size
    EXPORT  bihflag_size
    EXPORT  keypad_size
    EXPORT  language_size
	EXPORT  cleaning_size
    EXPORT  bedding_size
    EXPORT  general_size
    EXPORT  doorbell_size
    EXPORT  dnd_size
        
	END  