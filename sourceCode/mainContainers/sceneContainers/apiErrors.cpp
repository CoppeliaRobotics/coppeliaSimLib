#include <apiErrors.h>
#include <app.h>
#include <simInternal.h>
#include <simStrings.h>
#include <iostream>

std::string CApiErrors::_lastError;

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

std::string CApiErrors::getAndClearLastError()
{
    std::string retVal = _lastError;
    _lastError.clear();
    return retVal;
}
