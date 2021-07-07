#include "apiErrors.h"
#include "app.h"
#include "simInternal.h"
#include "threadPool_old.h"
#include "simStrings.h"
#include <iostream>

std::string CApiErrors::_lastWarningOrError;

CApiErrors::CApiErrors()
{
}

CApiErrors::~CApiErrors()
{
}

void CApiErrors::setLastWarningOrError(const char* functionName,const char* errMsg)
{
    std::string funcName;
    if (functionName!=nullptr)
    {
        funcName=functionName;
        if (funcName.size()>9)
        {
            if (funcName.compare(funcName.size()-9,9,"_internal")==0)
                funcName.assign(funcName.begin(),funcName.end()-9);
        }
    }

    std::string msg(errMsg);
    if (funcName.size()>0)
        msg+=" ("+funcName+")";

    _lastWarningOrError=msg;

    // Old:
    setThreadBasedFirstCapiError_old(errMsg);
}

std::string CApiErrors::getAndClearLastWarningOrError()
{
    std::string retVal=_lastWarningOrError;
    _lastWarningOrError.clear();
    return(retVal);
}

// Old:
// ***********************************************************
VMutex _threadBasedFirstCapiErrorAndWarningMutex_old;
std::vector<SThreadAndMsg_old> CApiErrors::_threadBasedFirstCapiWarning_old;
std::vector<SThreadAndMsg_old> CApiErrors::_threadBasedFirstCapiError_old;
void CApiErrors::clearThreadBasedFirstCapiErrorAndWarning_old()
{
    _clearThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiWarning_old);
    _clearThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiError_old);
}
void CApiErrors::setThreadBasedFirstCapiWarning_old(const char* msg)
{
    _setThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiWarning_old,msg);
}
std::string CApiErrors::getAndClearThreadBasedFirstCapiWarning_old()
{
    return(_getAndClearThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiWarning_old));
}
void CApiErrors::setThreadBasedFirstCapiError_old(const char* msg)
{
    _setThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiError_old,msg);
}
std::string CApiErrors::getAndClearThreadBasedFirstCapiError_old()
{
    return(_getAndClearThreadBasedFirstCapiMsg_old(_threadBasedFirstCapiError_old));
}
void CApiErrors::_clearThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect)
{
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    _threadBasedFirstCapiErrorAndWarningMutex_old.lock("");
    for (size_t i=0;i<vect.size();i++)
    {
        if (vect[i].threadId==threadId)
        {
            vect.erase(vect.begin()+i);
            break;
        }
    }
    _threadBasedFirstCapiErrorAndWarningMutex_old.unlock();
}
void CApiErrors::_setThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect,const char* msg)
{
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    int index=-1;
    _threadBasedFirstCapiErrorAndWarningMutex_old.lock("");
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
        SThreadAndMsg_old v;
        v.threadId=threadId;
        v.message=msg;
        vect.push_back(v);
    }
    _threadBasedFirstCapiErrorAndWarningMutex_old.unlock();
}
std::string CApiErrors::_getAndClearThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect)
{
    std::string retVal;
    VTHREAD_ID_TYPE threadId=VThread::getCurrentThreadId();
    int index=-1;
    _threadBasedFirstCapiErrorAndWarningMutex_old.lock("");
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
    _threadBasedFirstCapiErrorAndWarningMutex_old.unlock();
    return(retVal);
}
// ***********************************************************
