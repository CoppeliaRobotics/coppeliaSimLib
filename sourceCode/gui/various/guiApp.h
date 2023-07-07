#include <uiThread.h>
#include <gm.h>
#include <simQApp.h>
#include <simAndUiThreadSync.h>
#include <mainWindow.h>

#pragma once

class GuiApp
{
public:
    GuiApp();
    virtual ~GuiApp();

    void initGui(int options);
    void runGui();
    void cleanupGui();

    static int getEditModeType(); // helper
    static void setRebuildHierarchyFlag(); // helper
    static void setResetHierarchyViewFlag(); // helper
    static void setRefreshHierarchyViewFlag(); // helper
    static void setLightDialogRefreshFlag(); // helper
    static void setFullDialogRefreshFlag(); // helper
    static void setDialogRefreshDontPublishFlag(); // helper
    static void setToolbarRefreshFlag(); // helper
    static int getMouseMode(); // helper
    static void setMouseMode(int mm); // helper
    static void setDefaultMouseMode(); // helper
    static bool isFullScreen(); // helper
    static void setFullScreen(bool f); // helper
    static bool getShowInertias();
    static void setShowInertias(bool show);

    static void setBrowserEnabled(bool e);
    static bool getBrowserEnabled();
    static bool isOnline();
    static bool executeUiThreadCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    static void logMsgToStatusbar(const char* msg,bool html);

#ifdef SIM_WITH_GUI
    static void showSplashScreen();
    static void setIcon();
    static CMainWindow* mainWindow;
    static void createMainWindow();
    static void deleteMainWindow();
    static void setShowConsole(bool s);
#endif


    static CSimQApp* qtApp;
    static CUiThread* uiThread;
    static CGm* gm;

    static int operationalUIParts;
    static int sc;

private:
    static void _loadLegacyPlugins();

    static int _qApp_argc;
    static char _qApp_arg0[];
    static char* _qApp_argv[1];

    static bool _browserEnabled;
    static bool _online;
    static bool _showInertias;
};
