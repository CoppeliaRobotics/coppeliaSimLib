#include <apiErrors.h>
#include <app.h>
#include <simInternal.h>
#include <simStrings.h>
#include <iostream>

std::string CApiErrors::_lastError;
std::string CApiErrors::_lastWarning;

CApiErrors::CApiErrors()
{
}

CApiErrors::~CApiErrors()
{
}

void CApiErrors::setLastError(const char* functionName, const char* errMsg)
{
    std::string funcName;
    if (functionName != nullptr)
    {
        funcName = functionName;
        if (funcName.size() > 9)
        {
            if (funcName.compare(funcName.size() - 9, 9, "_internal") == 0)
                funcName.assign(funcName.begin(), funcName.end() - 9);
        }
    }

    std::string msg(errMsg);
    if (funcName.size() > 0)
        msg += " (c: " + funcName + ")";

    _lastError = msg;
}

void CApiErrors::setLastErrorRaw(const char* errMsg)
{
    _lastError = errMsg;
}

std::string CApiErrors::getAndClearLastError()
{
    std::string retVal = _lastError;
    _lastError.clear();
    return retVal;
}

void CApiErrors::setLastWarning(const char* functionName, const char* warnMsg)
{
    std::string funcName;
    if (functionName != nullptr)
    {
        funcName = functionName;
        if (funcName.size() > 9)
        {
            if (funcName.compare(funcName.size() - 9, 9, "_internal") == 0)
                funcName.assign(funcName.begin(), funcName.end() - 9);
        }
    }

    std::string msg(warnMsg);
    if (funcName.size() > 0)
        msg += " (c: " + funcName + ")";

    _lastWarning = msg;
}

void CApiErrors::setLastWarningRaw(const char* warnMsg)
{
    _lastWarning = warnMsg;
}

std::string CApiErrors::getAndClearLastWarning()
{
    std::string retVal = _lastWarning;
    _lastWarning.clear();
    return retVal;
}
