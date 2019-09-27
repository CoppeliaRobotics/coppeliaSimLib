
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "signalContainer.h"
#include <string.h>

CSignalContainer::CSignalContainer()
{
}

CSignalContainer::~CSignalContainer()
{
    removeAllSignals(false);
}

void CSignalContainer::removeAllSignals(bool onlyThoseFromEmbeddedScripts)
{
    clearAllIntegerSignals(onlyThoseFromEmbeddedScripts);
    clearAllFloatSignals(onlyThoseFromEmbeddedScripts);
    clearAllDoubleSignals(onlyThoseFromEmbeddedScripts);
    clearAllStringSignals(onlyThoseFromEmbeddedScripts);
}

void CSignalContainer::simulationAboutToStart()
{
}

void CSignalContainer::simulationEnded()
{
    removeAllSignals(true);
}

void CSignalContainer::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
}

void CSignalContainer::emptySceneProcedure()
{ // don't do anything here! (plugin or add-on might be using that functionality too) 
}

void CSignalContainer::setIntegerSignal(const char* signalName,int value,bool fromEmbeddedScript)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getIntegerSignalIndex(signalName);
    if (index==-1)
    {
        _intSignalNames.push_back(signalName);
        _intSignalValues.push_back(value);
        _intSignalEmbScriptCreated.push_back(fromEmbeddedScript);
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
        _intSignalEmbScriptCreated.erase(_intSignalEmbScriptCreated.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllIntegerSignals(bool onlyThoseFromEmbeddedScripts)
{
    int retVal=0;
    if (!onlyThoseFromEmbeddedScripts)
    {
        retVal=int(_intSignalNames.size());
        _intSignalNames.clear();
        _intSignalValues.clear();
        _intSignalEmbScriptCreated.clear();
    }
    else
    {
        for (int i=0;i<int(_intSignalNames.size());i++)
        {
            if (_intSignalEmbScriptCreated[i])
            {
                clearIntegerSignal(_intSignalNames[i].c_str());
                retVal++;
                i=-1; // ordering might have changed
            }
        }
    }
    return(retVal);
}

void CSignalContainer::setFloatSignal(const char* signalName,float value,bool fromEmbeddedScript)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getFloatSignalIndex(signalName);
    if (index==-1)
    {
        _floatSignalNames.push_back(signalName);
        _floatSignalValues.push_back(value);
        _floatSignalEmbScriptCreated.push_back(fromEmbeddedScript);
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
        _floatSignalEmbScriptCreated.erase(_floatSignalEmbScriptCreated.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllFloatSignals(bool onlyThoseFromEmbeddedScripts)
{
    int retVal=0;
    if (!onlyThoseFromEmbeddedScripts)
    {
        retVal=int(_floatSignalNames.size());
        _floatSignalNames.clear();
        _floatSignalValues.clear();
        _floatSignalEmbScriptCreated.clear();
    }
    else
    {
        for (int i=0;i<int(_floatSignalNames.size());i++)
        {
            if (_floatSignalEmbScriptCreated[i])
            {
                clearFloatSignal(_floatSignalNames[i].c_str());
                retVal++;
                i=-1; // ordering might have changed
            }
        }
    }
    return(retVal);
}

void CSignalContainer::setDoubleSignal(const char* signalName,double value,bool fromEmbeddedScript)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getDoubleSignalIndex(signalName);
    if (index==-1)
    {
        _doubleSignalNames.push_back(signalName);
        _doubleSignalValues.push_back(value);
        _doubleSignalEmbScriptCreated.push_back(fromEmbeddedScript);
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
        _doubleSignalEmbScriptCreated.erase(_doubleSignalEmbScriptCreated.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllDoubleSignals(bool onlyThoseFromEmbeddedScripts)
{
    int retVal=0;
    if (!onlyThoseFromEmbeddedScripts)
    {
        retVal=int(_doubleSignalNames.size());
        _doubleSignalNames.clear();
        _doubleSignalValues.clear();
        _doubleSignalEmbScriptCreated.clear();
    }
    else
    {
        for (int i=0;i<int(_doubleSignalNames.size());i++)
        {
            if (_doubleSignalEmbScriptCreated[i])
            {
                clearDoubleSignal(_doubleSignalNames[i].c_str());
                retVal++;
                i=-1; // ordering might have changed
            }
        }
    }
    return(retVal);
}

void CSignalContainer::setStringSignal(const char* signalName,const std::string& value,bool fromEmbeddedScript)
{
    if ((signalName==nullptr)||(strlen(signalName)==0))
        return;
    int index=_getStringSignalIndex(signalName);
    if (index==-1)
    {
        _stringSignalNames.push_back(signalName);
        _stringSignalValues.push_back(value);
        _stringSignalEmbScriptCreated.push_back(fromEmbeddedScript);
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
        _stringSignalEmbScriptCreated.erase(_stringSignalEmbScriptCreated.begin()+index);
        return(1);
    }
    return(0);
}

int CSignalContainer::clearAllStringSignals(bool onlyThoseFromEmbeddedScripts)
{
    int retVal=0;
    if (!onlyThoseFromEmbeddedScripts)
    {
        retVal=int(_stringSignalNames.size());
        _stringSignalNames.clear();
        _stringSignalValues.clear();
        _stringSignalEmbScriptCreated.clear();
    }
    else
    {
        for (int i=0;i<int(_stringSignalNames.size());i++)
        {
            if (_stringSignalEmbScriptCreated[i])
            {
                clearStringSignal(_stringSignalNames[i].c_str());
                retVal++;
                i=-1; // ordering might have changed
            }
        }
    }
    return(retVal);

}

int CSignalContainer::_getIntegerSignalIndex(const char* signalName)
{
    for (int i=0;i<int(_intSignalNames.size());i++)
    {
        if (_intSignalNames[i].compare(signalName)==0)
            return(i);
    }
    return(-1);
}

int CSignalContainer::_getFloatSignalIndex(const char* signalName)
{
    for (int i=0;i<int(_floatSignalNames.size());i++)
    {
        if (_floatSignalNames[i].compare(signalName)==0)
            return(i);
    }
    return(-1);
}

int CSignalContainer::_getDoubleSignalIndex(const char* signalName)
{
    for (int i=0;i<int(_doubleSignalNames.size());i++)
    {
        if (_doubleSignalNames[i].compare(signalName)==0)
            return(i);
    }
    return(-1);
}

int CSignalContainer::_getStringSignalIndex(const char* signalName)
{
    for (int i=0;i<int(_stringSignalNames.size());i++)
    {
        if (_stringSignalNames[i].compare(signalName)==0)
            return(i);
    }
    return(-1);
}

