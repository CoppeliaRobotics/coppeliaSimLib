#pragma once

#include <scriptObject.h>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

#define ADDON_EXTENTION_LUA "lua"
#define ADDON_EXTENTION_PY "py"

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
    void loadAllAddOns();
    void removeAllAddOns();
    CScriptObject* getAddOnFromHandle(int scriptHandle) const;
    CScriptObject* getAddOnFromUid(int uid) const;
    CScriptObject* getAddOnFromName(const char* name) const;

    int getSysFuncAndHookCnt(int sysCall) const;
    void setSysFuncAndHookCnt(int sysCall, int cnt);

    void pushGenesisEvents() const;

    bool shouldTemporarilySuspendMainScript();
    void getActiveScripts(std::vector<CScriptObject*>& scripts) const;
    int callScripts(int callType, CInterfaceStack* inStack, CInterfaceStack* outStack, int scriptToExclude = -1);

#ifdef SIM_WITH_GUI
    bool processCommand(int commandID);
#endif
  private:
    bool _removeAddOn(int scriptID);
    int _insertAddOn(CScriptObject* script);
    void _insertAddOns(const char* addOnExt);
    void _insertAdditionalAddOns(); // -a, -b options

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
