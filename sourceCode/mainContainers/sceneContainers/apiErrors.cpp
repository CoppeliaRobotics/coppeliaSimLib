
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "apiErrors.h"
#include "app.h"
#include "v_rep_internal.h"
#include "luaScriptFunctions.h"
#include "threadPool.h"
#include "v_repStrings.h"
#include <iostream>

std::vector<VTHREAD_ID_TYPE> CApiErrors::_controllerLocation_threadIds;
std::vector<std::vector<int> > CApiErrors::_controllerLocation_locationStack;

int CApiErrors::_c_gui_errorReportMode=sim_api_error_report;
int CApiErrors::_c_nonGui_errorReportMode=sim_api_error_report;

std::string CApiErrors::_c_gui_lastError=SIM_API_CALL_NO_ERROR;
std::string CApiErrors::_c_nonGui_lastError=SIM_API_CALL_NO_ERROR;

std::string CApiErrors::_cSideGeneratedLuaError;
std::vector<std::string> CApiErrors::_cSideGeneratedLuaWarnings;

CApiErrors::CApiErrors()
{
}

CApiErrors::~CApiErrors()
{
}

int CApiErrors::_getIndexFromCurrentThread()
{
    VTHREAD_ID_TYPE currentThreadId=VThread::getCurrentThreadId();
    for (int i=0;i<int(_controllerLocation_threadIds.size());i++)
    {
        if (VThread::areThreadIDsSame(currentThreadId,_controllerLocation_threadIds[i]))
            return(i);
    }
    return(-1); // Error, should never happen
}

bool CApiErrors::addNewThreadForErrorReporting(int scriptId_or_0ForCGui_or_1ForCNonGui)
{
    if (_getIndexFromCurrentThread()==-1)
    {
        _controllerLocation_threadIds.push_back(VThread::getCurrentThreadId());
        std::vector<int> tmp;
        _controllerLocation_locationStack.push_back(tmp);
        _controllerLocation_locationStack[_controllerLocation_locationStack.size()-1].push_back(scriptId_or_0ForCGui_or_1ForCNonGui);
        return(true);
    }
    return(false);
}

bool CApiErrors::removeThreadFromErrorReporting()
{
    int index=_getIndexFromCurrentThread();
    if (index!=-1)
    {
        _controllerLocation_threadIds.erase(_controllerLocation_threadIds.begin()+index);
        _controllerLocation_locationStack.erase(_controllerLocation_locationStack.begin()+index);
        return(true);
    }
    return(false);
}

void CApiErrors::pushLocation(int scriptId_or_0IfNoScript)
{
    int index=_getIndexFromCurrentThread();
    if (index!=-1)
    {
        if (scriptId_or_0IfNoScript==0)
        {
            if (VThread::isCurrentThreadTheUiThread())
                scriptId_or_0IfNoScript=0;
            else
                scriptId_or_0IfNoScript=1;
        }
        _controllerLocation_locationStack[index].push_back(scriptId_or_0IfNoScript);
    }
}

void CApiErrors::popLocation()
{
    int index=_getIndexFromCurrentThread();
    if (index!=-1)
        _controllerLocation_locationStack[index].pop_back();
}

int CApiErrors::_getCurrentLocation(bool onlyLuaLocation/*=false*/)
{
    int index=_getIndexFromCurrentThread();
    if (index>=0)
    {
        if (!onlyLuaLocation)
            return(_controllerLocation_locationStack[index][_controllerLocation_locationStack[index].size()-1]);
        // We arrive here if simSetLastError was called from C/C++: we basically want to hand the error to the last Lua script that called (i.e. called-back) the plugin
        for (int i=(int)_controllerLocation_locationStack[index].size()-1;i>=0;i--)
        {
            int loc=_controllerLocation_locationStack[index][i];
            if (loc>1)
                return(loc); // ok, this is a Lua script!
        }
    }
    return(-1);
}

void CApiErrors::setApiCallErrorMessage(const char* functionName,const char* errMsg)
{
    std::string funcName(functionName);
    if (funcName.size()>9)
    {
        if (funcName.compare(funcName.size()-9,9,"_internal")==0)
            funcName.assign(funcName.begin(),funcName.end()-9);
    }

    std::string msg(errMsg);
    msg+=" ("+funcName+")";

    int loc=_getCurrentLocation(false);

    if (loc!=-1)
    {
        if (loc==0)
        { // C API, GUI thread
            if ((_c_gui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
                _c_gui_lastError=msg;
            if ((_c_gui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_GUI+msg;
                std::cout << tmp.c_str() << std::endl;
            }
        }
        else if (loc==1)
        { // C API, Simulation thread
            if ((_c_nonGui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
                _c_nonGui_lastError=msg;
            if ((_c_nonGui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_NONGUI+msg;
                std::cout << tmp.c_str() << std::endl;
            }
        }
        else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
        { // Lua API (or C API called-back from Lua, which calls 'simSetLastError' (typically a plugin that was called-back via a custom Lua function, and that needs to output an error to the GUI console))
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
            if (it!=nullptr)
                _cSideGeneratedLuaError=errMsg;
        }
    }
}

void CApiErrors::clearCSideGeneratedLuaError()
{
    _cSideGeneratedLuaError.clear();
}

std::string CApiErrors::getCSideGeneratedLuaError()
{
    return(_cSideGeneratedLuaError);
}

std::string CApiErrors::getCSideGeneratedLuaWarning()
{
    std::string ret="";
    if (_cSideGeneratedLuaWarnings.size()>0)
    {
        ret=_cSideGeneratedLuaWarnings[0];
        _cSideGeneratedLuaWarnings.erase(_cSideGeneratedLuaWarnings.begin());
    }
    return(ret);
}

void CApiErrors::decorateLuaErrorMessage(const char* functionName,std::string& errMsg)
{
    std::string funcName(functionName);
    errMsg+=" ("+funcName+")";
    errMsg=getLocationString()+errMsg;
}

void CApiErrors::setLuaCallErrorMessage(const char* functionName,const char* errMsg)
{ // call only directly from Lua
    std::string funcName(functionName);

    std::string msg(errMsg);
    msg+=" ("+funcName+")";

    int loc=_getCurrentLocation(true);
    if (loc>1)
    {
        if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
        { // Lua API (or C API called-back from Lua, which calls 'simSetLastError' (typically a plugin that was called-back via a custom Lua function, and that needs to output an error to the GUI console))
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
            if (it!=nullptr)
            {
                // Here we can react to errors:
                App::ct->simulation->pauseOnErrorRequested();

                msg=getLocationString()+msg;

                if ((it->getErrorReportMode()&(sim_api_error_report|sim_api_error_output))!=0)
                    it->setLastErrorString(msg.c_str());
                if ((it->getErrorReportMode()&sim_api_error_output)!=0)
                {
                    std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_LUA_API+msg;
                    App::addStatusbarMessage(tmp.c_str(),true);
                    SUIThreadCommand cmdIn;
                    SUIThreadCommand cmdOut;
                    cmdIn.cmdId=FLASH_STATUSBAR_UITHREADCMD;
                    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
            }
        }
    }
}

void CApiErrors::setLuaCallWarningMessage(const char* functionName,const char* warnMsg)
{ // call only directly from Lua
    std::string funcName(functionName);

    std::string msg(warnMsg);
    msg+=" ("+funcName+")";

    int loc=_getCurrentLocation(true);
    if (loc>1)
    {
        if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
        { // Lua API (or C API called-back from Lua, which calls 'simSetLastError' (typically a plugin that was called-back via a custom Lua function, and that needs to output an error to the GUI console))
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
            if (it!=nullptr)
            {
                msg=getLocationString()+msg;
                if ((it->getErrorReportMode()&sim_api_error_output)!=0)
                {
                    std::string tmp=IDSNOTR_FUNCTION_CALL_WARNING_LUA_API+msg;
                    App::addStatusbarMessage(tmp.c_str());
                }
            }
        }
    }
}

void CApiErrors::setLuaCallErrorMessage_fromPlugin(const char* functionName,const char* errMsg)
{ // call only indirectly from Lua via a plugin callback
    int loc=_getCurrentLocation(true);
    if (loc<2)
    { // C API, GUI thread or simulation thread
        if (loc==0)
        { // C API, GUI thread
            if ((_c_gui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
                _c_gui_lastError=errMsg;
            if ((_c_gui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_GUI;
                tmp+=errMsg;
                std::cout << tmp.c_str() << std::endl;
            }
        }
        else if (loc==1)
        { // C API, Simulation thread
            if ((_c_nonGui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
                _c_nonGui_lastError=errMsg;
            if ((_c_nonGui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_NONGUI;
                tmp+=errMsg;
                std::cout << tmp.c_str() << std::endl;
            }
        }

        std::string funcName(functionName);
        std::string msg(errMsg);
        msg+=" ("+funcName+")";
        App::ct->simulation->pauseOnErrorRequested();
        msg=getLocationString()+msg;
        std::string tmp="Error: "+msg;
        App::addStatusbarMessage(tmp.c_str(),true);
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=FLASH_STATUSBAR_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    { // Lua API (or C API called-back from Lua, which calls 'simSetLastError' (typically a plugin that was called-back via a custom Lua function, and that needs to output an error to the GUI console))
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            _cSideGeneratedLuaError=errMsg;
    }
}

void CApiErrors::setLuaCallWarningMessage_fromPlugin(const char* functionName,const char* warnMsg)
{ // call only indirectly from Lua via a plugin callback
    int loc=_getCurrentLocation(true);
    if (loc<2)
    { // C API, GUI thread or simulation thread
        if (loc==0)
        { // C API, GUI thread
            //if ((_c_gui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
            //    _c_gui_lastWarning=warnMsg;
            if ((_c_gui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_WARNING_C_API_GUI;
                tmp+=warnMsg;
                std::cout << tmp.c_str() << std::endl;
            }
        }
        else if (loc==1)
        { // C API, Simulation thread
            //if ((_c_nonGui_errorReportMode&(sim_api_error_report|sim_api_error_output))!=0)
            //    _c_nonGui_lastWarning=warnMsg;
            if ((_c_nonGui_errorReportMode&sim_api_error_output)!=0)
            {
                std::string tmp=IDSNOTR_FUNCTION_CALL_WARNING_C_API_NONGUI;
                tmp+=warnMsg;
                std::cout << tmp.c_str() << std::endl;
            }
        }

        std::string funcName(functionName);
        std::string msg(warnMsg);
        msg+=" ("+funcName+")";
        // App::ct->simulation->pauseOnErrorRequested();
        msg=getLocationString()+msg;
        std::string tmp="Warning: "+msg;
        App::addStatusbarMessage(tmp.c_str());
    }
    else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    { // Lua API (or C API called-back from Lua, which calls 'simSetLastError' (typically a plugin that was called-back via a custom Lua function, and that needs to output an error to the GUI console))
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            _cSideGeneratedLuaWarnings.push_back(warnMsg);
    }
}

void CApiErrors::clearApiCallErrorMessage()
{
    int loc=_getCurrentLocation();
    if (loc==0)
        _c_gui_lastError=SIM_API_CALL_NO_ERROR;
    else if (loc==1)
        _c_nonGui_lastError=SIM_API_CALL_NO_ERROR;
    else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            it->setLastErrorString(SIM_API_CALL_NO_ERROR);
    }
}

std::string CApiErrors::getApiCallErrorMessage()
{
    int loc=_getCurrentLocation();
    if (loc==0)
        return(_c_gui_lastError);
    if (loc==1)
        return(_c_nonGui_lastError);
    if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            return(it->getLastErrorString());
    }
    return(SIM_API_CALL_NO_ERROR);
}

void CApiErrors::setApiCallErrorReportMode(int mode)
{
    int loc=_getCurrentLocation();
    if (loc==0)
        _c_gui_errorReportMode=mode;
    else if (loc==1)
        _c_nonGui_errorReportMode=mode;
    else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            it->setErrorReportMode(mode);
    }
}

int CApiErrors::getApiCallErrorReportMode()
{
    int loc=_getCurrentLocation();
    if (loc==0)
        return(_c_gui_errorReportMode);
    if (loc==1)
        return(_c_nonGui_errorReportMode);
    if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
            return(it->getErrorReportMode());
    }
    return(0);
}



void CApiErrors::setApiCallWarningMessage(const char* functionName,const char* warnMsg)
{
    std::string funcName(functionName);
    if (funcName.size()>9)
    {
        if (funcName.compare(funcName.size()-9,9,"_internal")==0)
            funcName.assign(funcName.begin(),funcName.end()-9);
    }

    std::string msg(warnMsg);
    msg+=" ("+funcName+")";

    int loc=_getCurrentLocation();
    if (loc==0)
    {
        if ((_c_gui_errorReportMode&sim_api_warning_output)!=0)
        {
// Not yet supported!
//          std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_GUI+_c_gui_lastError;
//          std::cout << tmp.c_str() << std::endl;
        }
    }
    else if (loc==1)
    {
        if ((_c_nonGui_errorReportMode&sim_api_warning_output)!=0)
        {
            // Not yet supported!
//          std::string tmp=IDSNOTR_FUNCTION_CALL_ERROR_C_API_NONGUI+_c_nonGui_lastError;
//          std::cout << tmp.c_str() << std::endl;
        }
    }
    else if ((App::ct!=nullptr)&&(App::ct->luaScriptContainer!=nullptr))
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(loc);
        if (it!=nullptr)
        {
            if ((it->getErrorReportMode()&sim_api_warning_output)!=0)
            {
                std::string msg=getLocationString()+warnMsg;
                std::string tmp=std::string(IDSNOTR_FUNCTION_CALL_WARNING_LUA_API)+msg;
                App::addStatusbarMessage(tmp.c_str());
            }
        }
    }
}

void CApiErrors::clearApiCallWarningMessage()
{
    // not yet supported
}

std::string CApiErrors::getApiCallWarningMessage()
{
    // not yet supported
    return("");
}









