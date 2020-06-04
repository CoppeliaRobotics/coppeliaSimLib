#include "apiErrors.h"
#include "app.h"
#include "simInternal.h"
#include "luaScriptFunctions.h"
#include "threadPool.h"
#include "simStrings.h"
#include <iostream>

std::string CApiErrors::_c_lastError;
VMutex _threadBasedFirstCapiErrorAndWarningMutex;
std::vector<SThreadAndMsg> CApiErrors::_threadBasedFirstCapiWarning;
std::vector<SThreadAndMsg> CApiErrors::_threadBasedFirstCapiError;

CApiErrors::CApiErrors()
{
}

CApiErrors::~CApiErrors()
{
}

void CApiErrors::setCapiCallErrorMessage(const char* functionName,const char* errMsg)
{
    setThreadBasedFirstCapiError(errMsg);
    std::string funcName(functionName);
    if (funcName.size()>9)
    {
        if (funcName.compare(funcName.size()-9,9,"_internal")==0)
            funcName.assign(funcName.begin(),funcName.end()-9);
    }

    std::string msg(errMsg);
    msg+=" ("+funcName+")";

    _c_lastError=msg;
}

void CApiErrors::clearCapiCallErrorMessage()
{
    _c_lastError.clear();
}

std::string CApiErrors::getCapiCallErrorMessage()
{
    return(_c_lastError);
}

void CApiErrors::clearThreadBasedFirstCapiErrorAndWarning()
{
    _clearThreadBasedFirstCapiMsg(_threadBasedFirstCapiWarning);
    _clearThreadBasedFirstCapiMsg(_threadBasedFirstCapiError);
}

void CApiErrors::setThreadBasedFirstCapiWarning(const char* msg)
{
    _setThreadBasedFirstCapiMsg(_threadBasedFirstCapiWarning,msg);
}

std::string CApiErrors::getAndClearThreadBasedFirstCapiWarning()
{
    return(_getAndClearThreadBasedFirstCapiMsg(_threadBasedFirstCapiWarning));
}

void CApiErrors::setThreadBasedFirstCapiError(const char* msg)
{
    _setThreadBasedFirstCapiMsg(_threadBasedFirstCapiError,msg);
}

std::string CApiErrors::getAndClearThreadBasedFirstCapiError()
{
    return(_getAndClearThreadBasedFirstCapiMsg(_threadBasedFirstCapiError));
}

void CApiErrors::_clearThreadBasedFirstCapiMsg(std::vector<SThreadAndMsg>& vect)
{
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    _threadBasedFirstCapiErrorAndWarningMutex.lock("");
    for (size_t i=0;i<vect.size();i++)
    {
        if (vect[i].threadId==threadId)
        {
            vect.erase(vect.begin()+i);
            break;
        }
    }
    _threadBasedFirstCapiErrorAndWarningMutex.unlock();
}

void CApiErrors::_setThreadBasedFirstCapiMsg(std::vector<SThreadAndMsg>& vect,const char* msg)
{
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    int index=-1;
    _threadBasedFirstCapiErrorAndWarningMutex.lock("");
    for (size_t i=0;i<vect.size();i++)
    {
        if (vect[i].threadId==threadId)
        {
            index=int(i);
            break;
        }
    }
    if (index==-1)
    {
        SThreadAndMsg v;
        v.threadId=threadId;
        v.message=msg;
        vect.push_back(v);
    }
    _threadBasedFirstCapiErrorAndWarningMutex.unlock();
}

std::string CApiErrors::_getAndClearThreadBasedFirstCapiMsg(std::vector<SThreadAndMsg>& vect)
{
    std::string retVal;
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    int index=-1;
    _threadBasedFirstCapiErrorAndWarningMutex.lock("");
    for (size_t i=0;i<vect.size();i++)
    {
        if (vect[i].threadId==threadId)
        {
            index=int(i);
            break;
        }
    }
    if (index!=-1)
    {
        retVal=vect[size_t(index)].message;
        vect.erase(vect.begin()+size_t(index));
    }
    _threadBasedFirstCapiErrorAndWarningMutex.unlock();
    return(retVal);
}
