#pragma once

#include "scriptObject.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

#define ADDON_PREFIX "simAddOn"
#define ADDON_EXTENTION "lua"

// OLD:
#define ADDON_SCRIPT_PREFIX1_AUTOSTART "simAddOnScript_"
#define ADDON_SCRIPT_PREFIX2_AUTOSTART "vrepAddOnScript_"
#define ADDON_SCRIPT_PREFIX1_NOAUTOSTART "simAddOnScript-"
#define ADDON_SCRIPT_PREFIX2_NOAUTOSTART "vrepAddOnScript-"
#define ADDON_FUNCTION_PREFIX1 "simAddOnFunc_"
#define ADDON_FUNCTION_PREFIX2 "vrepAddOnFunc_"

class CAddOnScriptContainer
{
public:
    CAddOnScriptContainer();
    virtual ~CAddOnScriptContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void simulationAboutToEnd();
    bool processCommand(int commandID);
    void removeAllAddOns();
    CScriptObject* getAddOnFromID(int scriptID) const;
    CScriptObject* getAddOnFromName(const char* name) const;

    int getSysFuncAndHookCnt(int sysCall) const;
    void setSysFuncAndHookCnt(int sysCall,int cnt);

    bool shouldTemporarilySuspendMainScript();
    int callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack);

private:
    bool _removeAddOn(int scriptID);
    int _insertAddOn(CScriptObject* script);
    int _insertAddOns();

    int _sysFuncAndHookCnt_event;
    int _sysFuncAndHookCnt_dyn;
    int _sysFuncAndHookCnt_contact;
    int _sysFuncAndHookCnt_joint;
    std::vector<CScriptObject*> _addOns;

    // OLD:
    int _prepareAddOnFunctionNames_old();
    std::vector<std::string> _allAddOnFunctionNames_old;
    std::vector<int> _allAddOnFunctionUiHandles_old;
};
