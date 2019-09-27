#ifndef VREP_CONFIG_INCLUDED
#define VREP_CONFIG_INCLUDED

#include "vrepConfigBase.h"

// For version history, check http://www.coppeliarobotics.com/helpFiles/en/versionInfo.htm

#define VREP_LICENSE_VREP_DEMO          0x00000000
#define VREP_LICENSE_VREP_PRO_EDU       0x00001000
#define VREP_LICENSE_VREP_PRO           0x00002000
#define VREP_LICENSE_VREP_PRO_P         0x00003000
#define VREP_LICENSE_VREP_SUBLICENSE    0x00004000
#define VREP_LICENSE_VREP_PLAYER        0x00005000
#define VREP_LICENSE_VREP_BASIC         0x00006000
#define VREP_LICENSE_BLUE_REALITY       0x00007000


#ifdef SIM_WITHOUT_QT_AT_ALL
    #define VREP_WEBPAGE_ADDRESS_TXT "www.coppeliarobotics.com"
    #define VREP_WEBPAGE_EMAIL_TXT "info@coppeliarobotics.com"
    #define VREP_COMPANY_NAME_TXT "Coppelia Robotics GmbH"
    #define VREP_COMPILATION_VERSION 0x00000006
    #define VREP_LICENSE_TYPE VREP_LICENSE_VREP_BASIC
    #define VREP_MODEL_EXTENSION "ttm"
    #define VREP_SCENE_EXTENSION "ttt"
    #define FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE "settings.dat"
    #define VREP_MODEL_THUMBNAILFILE_NAME "v_repModelThumbnails.ttmt"
#endif

#ifdef WIN_VREP
    #define VREP_LOWCASE_STRING_COMPARE(x,y) _stricmp(x,y) 
#else
    #define VREP_LOWCASE_STRING_COMPARE(x,y) strcasecmp(x,y)
#endif

#define VREP_SLASH "/"
#define _SECURE_SCL 0 // Disables the bound check for vectors (a bit faster)
#define SIM_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define SIM_MAX(a,b) (((a)>(b)) ? (a) : (b))
#define SIM_IS_BIT_SET(var,bit) (((var) & (1<<(bit)))!=0)
#define SIM_SET_BIT(var,bit) ((var) |= (1<<(bit)))
#define SIM_CLEAR_BIT(var,bit) ((var) &= (~(1<<(bit))))
#define SIM_TOGGLE_BIT(var,bit) ((var) ^= (1<<(bit)))
#define SIM_SET_CLEAR_BIT(var,bit,on) ((on) ? SIM_SET_BIT((var),(bit)) : SIM_CLEAR_BIT((var),(bit)) )
#define strTranslate(text) text
#define QTPROPERTYBROWSER_COLOR_GREY QColor(244,244,244)
#define QTPROPERTYBROWSER_COLOR_RED QColor(255,240,240)
#define QTPROPERTYBROWSER_COLOR_GREEN QColor(240,255,240)
#define QTPROPERTYBROWSER_COLOR_BLUE QColor(240,240,255)
#define QTPROPERTYBROWSER_COLOR_YELLOW QColor(255,255,240)
#define QTPROPERTYBROWSER_COLOR_CYAN QColor(240,255,255)
#define QTPROPERTYBROWSER_COLOR_PURPLE QColor(255,240,255)

#endif // VREP_CONFIG_INCLUDED
