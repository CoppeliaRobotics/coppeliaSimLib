
#pragma once

#include "luaScriptObject.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

#define ADDON_SCRIPT_PREFIX_AUTOSTART "vrepAddOnScript_"
#define ADDON_SCRIPT_PREFIX_NOAUTOSTART "vrepAddOnScript-"
#define ADDON_FUNCTION_PREFIX "vrepAddOnFunc_"
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

    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_new(SScriptCallBack* object);
    void addCallbackStructureObjectToDestroyAtTheEndOfSimulation_old(SLuaCallBack* object);

    bool handleAddOnScriptExecution_beforeMainScript();
    int handleAddOnScriptExecution(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack);
    void setAdditionalAddOnScript(std::string scriptNameWithExtension);

    std::vector<CLuaScriptObject*> allAddOnScripts;
    std::vector<std::string> allAddOnFunctionNames;

#ifdef SIM_WITH_GUI
    void addMenu(VMenu* menu);
#endif

protected:
    std::string _additionalAddOnScriptAllScenes;
};
