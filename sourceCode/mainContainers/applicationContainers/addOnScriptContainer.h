#pragma once

#include "luaScriptObject.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

#define ADDON_SCRIPT_PREFIX1_AUTOSTART "simAddOnScript_"
#define ADDON_SCRIPT_PREFIX2_AUTOSTART "vrepAddOnScript_"
#define ADDON_SCRIPT_PREFIX1_NOAUTOSTART "simAddOnScript-"
#define ADDON_SCRIPT_PREFIX2_NOAUTOSTART "vrepAddOnScript-"
#define ADDON_FUNCTION_PREFIX1 "simAddOnFunc_"
#define ADDON_FUNCTION_PREFIX2 "vrepAddOnFunc_"
#define ADDON_EXTENTION "lua"

class CAddOnScriptContainer
{
public:
    CAddOnScriptContainer();
    virtual ~CAddOnScriptContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void simulationAboutToEnd();
    bool processCommand(int commandID);
    bool removeScript(int scriptID);
    int insertScript(CLuaScriptObject* script);
    void removeAllScripts();
    CLuaScriptObject* getAddOnScriptFromID(int scriptID) const;
    CLuaScriptObject* getAddOnScriptFromName(const char* name) const;

    int insertAddOnScripts();
    int prepareAddOnFunctionNames();

    bool handleAddOnScriptExecution_beforeMainScript();
    int callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack);

    std::vector<CLuaScriptObject*> allAddOnScripts;
    std::vector<std::string> allAddOnFunctionNames;

#ifdef SIM_WITH_GUI
    void addMenu(VMenu* menu);
#endif
};
