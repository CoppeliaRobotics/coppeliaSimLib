
#include "simInternal.h"
#include "signalContainer.h"
#include <string.h>

CSignalContainer::CSignalContainer()
{
}

CSignalContainer::~CSignalContainer()
{
}

void CSignalContainer::announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    if (!sceneSwitchPersistentScript)
    {
        size_t i=0;
        while (i<_intSignalCreatorHandles.size())
        {
            if (_intSignalCreatorHandles[i]==scriptHandle)
                clearIntegerSignal(_intSignalNames[i].c_str());
            else
                i++;
        }
        i=0;
        while (i<_floatSignalCreatorHandles.size())
        {
            if (_floatSignalCreatorHandles[i]==scriptHandle)
                clearFloatSignal(_floatSignalNames[i].c_str());
            else
                i++;
        }
        i=0;
        while (i<_doubleSignalCreatorHandles.size())
        {
            if (_doubleSignalCreatorHandles[i]==scriptHandle)
                clearDoubleSignal(_doubleSignalNames[i].c_str());
            else
                i++;
        }
        i=0;
        while (i<_stringSignalCreatorHandles.size())
        {
            if (_stringSignalCreatorHandles[i]==scriptHandle)
                clearStringSignal(_stringSignalNames[i].c_str());
            else
                i++;
        }
    }
}

void CSignalContainer::simulationEnded()
{
}

void CSignalContainer::setIntegerSignal(const char* signalName,int value,int creatorHandle)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getIntegerSignalIndex(signalName);
    if (index==-1)
    {
        _intSignalNames.push_back(signalName);
        _intSignalValues.push_back(value);
        _intSignalCreatorHandles.push_back(creatorHandle);
    }
    else
        _intSignalValues[index]=value;
}

bool CSignalContainer::getIntegerSignal(const char* signalName,int& value)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(false);
    int index=_getIntegerSignalIndex(signalName);
    if (index==-1)
        return(false);
    value=_intSignalValues[index];
    return(true);
}

bool CSignalContainer::getIntegerSignalNameAtIndex(int index,std::string& signalName)
{
    if ( (index<0)||(index>=int(_intSignalNames.size())) )
        return(false);
    signalName=_intSignalNames[index];
    return(true);
}

int CSignalContainer::clearIntegerSignal(const char* signalName)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(0);
    int index=_getIntegerSignalIndex(signalName);
    if (index!=-1)
    {
        _intSignalNames.erase(_intSignalNames.begin()+index);
        _intSignalValues.erase(_intSignalValues.begin()+index);
        _intSignalCreatorHandles.erase(_intSignalCreatorHandles.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllIntegerSignals()
{
    int retVal=0;
    retVal=int(_intSignalNames.size());
    _intSignalNames.clear();
    _intSignalValues.clear();
    _intSignalCreatorHandles.clear();
    return(retVal);
}

void CSignalContainer::setFloatSignal(const char* signalName,float value,int creatorHandle)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getFloatSignalIndex(signalName);
    if (index==-1)
    {
        _floatSignalNames.push_back(signalName);
        _floatSignalValues.push_back(value);
        _floatSignalCreatorHandles.push_back(creatorHandle);
    }
    else
        _floatSignalValues[index]=value;
}

bool CSignalContainer::getFloatSignal(const char* signalName,float& value)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(false);
    int index=_getFloatSignalIndex(signalName);
    if (index==-1)
        return(false);
    value=_floatSignalValues[index];
    return(true);
}

bool CSignalContainer::getFloatSignalNameAtIndex(int index,std::string& signalName)
{
    if ( (index<0)||(index>=int(_floatSignalNames.size())) )
        return(false);
    signalName=_floatSignalNames[index];
    return(true);
}

int CSignalContainer::clearFloatSignal(const char* signalName)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(0);
    int index=_getFloatSignalIndex(signalName);
    if (index!=-1)
    {
        _floatSignalNames.erase(_floatSignalNames.begin()+index);
        _floatSignalValues.erase(_floatSignalValues.begin()+index);
        _floatSignalCreatorHandles.erase(_floatSignalCreatorHandles.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllFloatSignals()
{
    int retVal=int(_floatSignalNames.size());
    _floatSignalNames.clear();
    _floatSignalValues.clear();
    _floatSignalCreatorHandles.clear();
    return(retVal);
}

void CSignalContainer::setDoubleSignal(const char* signalName,double value,int creatorHandle)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getDoubleSignalIndex(signalName);
    if (index==-1)
    {
        _doubleSignalNames.push_back(signalName);
        _doubleSignalValues.push_back(value);
        _doubleSignalCreatorHandles.push_back(creatorHandle);
    }
    else
        _doubleSignalValues[index]=value;
}

bool CSignalContainer::getDoubleSignal(const char* signalName,double& value)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(false);
    int index=_getDoubleSignalIndex(signalName);
    if (index==-1)
        return(false);
    value=_doubleSignalValues[index];
    return(true);
}

bool CSignalContainer::getDoubleSignalNameAtIndex(int index,std::string& signalName)
{
    if ( (index<0)||(index>=int(_doubleSignalNames.size())) )
        return(false);
    signalName=_doubleSignalNames[index];
    return(true);
}

int CSignalContainer::clearDoubleSignal(const char* signalName)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(0);
    int index=_getDoubleSignalIndex(signalName);
    if (index!=-1)
    {
        _doubleSignalNames.erase(_doubleSignalNames.begin()+index);
        _doubleSignalValues.erase(_doubleSignalValues.begin()+index);
        _doubleSignalCreatorHandles.erase(_doubleSignalCreatorHandles.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllDoubleSignals()
{
    int retVal=int(_doubleSignalNames.size());
    _doubleSignalNames.clear();
    _doubleSignalValues.clear();
    _doubleSignalCreatorHandles.clear();
    return(retVal);
}

void CSignalContainer::setStringSignal(const char* signalName,const std::string& value,int creatorHandle)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getStringSignalIndex(signalName);
    if (index==-1)
    {
        _stringSignalNames.push_back(signalName);
        _stringSignalValues.push_back(value);
        _stringSignalCreatorHandles.push_back(creatorHandle);
    }
    else
        _stringSignalValues[index]=value;
}

bool CSignalContainer::getStringSignal(const char* signalName,std::string& value)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(false);
    int index=_getStringSignalIndex(signalName);
    if (index==-1)
        return(false);
    value=_stringSignalValues[index];
    return(true);
}

bool CSignalContainer::getStringSignalNameAtIndex(int index,std::string& signalName)
{
    if ( (index<0)||(index>=int(_stringSignalNames.size())) )
        return(false);
    signalName=_stringSignalNames[index];
    return(true);
}

int CSignalContainer::clearStringSignal(const char* signalName)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return(0);
    int index=_getStringSignalIndex(signalName);
    if (index!=-1)
    {
        _stringSignalNames.erase(_stringSignalNames.begin()+index);
        _stringSignalValues.erase(_stringSignalValues.begin()+index);
        _stringSignalCreatorHandles.erase(_stringSignalCreatorHandles.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllStringSignals()
{
    int retVal=int(_stringSignalNames.size());
    _stringSignalNames.clear();
    _stringSignalValues.clear();
    _stringSignalCreatorHandles.clear();
    return(retVal);

}

int CSignalContainer::_getIntegerSignalIndex(const char* signalName)
{
    for (size_t i=0;i<_intSignalNames.size();i++)
    {
        if (_intSignalNames[i].compare(signalName)==0)
            return(int(i));
    }
    return(-1);
}

int CSignalContainer::_getFloatSignalIndex(const char* signalName)
{
    for (size_t i=0;i<_floatSignalNames.size();i++)
    {
        if (_floatSignalNames[i].compare(signalName)==0)
            return(int(i));
    }
    return(-1);
}

int CSignalContainer::_getDoubleSignalIndex(const char* signalName)
{
    for (size_t i=0;i<_doubleSignalNames.size();i++)
    {
        if (_doubleSignalNames[i].compare(signalName)==0)
            return(int(i));
    }
    return(-1);
}

int CSignalContainer::_getStringSignalIndex(const char* signalName)
{
    for (size_t i=0;i<_stringSignalNames.size();i++)
    {
        if (_stringSignalNames[i].compare(signalName)==0)
            return(int(i));
    }
    return(-1);
}

