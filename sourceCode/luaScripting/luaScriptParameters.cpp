
#include "vrepMainHeader.h"
#include "luaScriptParameters.h"

#include "app.h"

CLuaScriptParameters::CLuaScriptParameters()
{
    _initialValuesInitialized=false;
}

CLuaScriptParameters::~CLuaScriptParameters()
{
}

void CLuaScriptParameters::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    clearInitialParameters();
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        for (size_t i=0;i<scriptParamEntries.size();i++)
        {
            SScriptParamEntry e;
            e.name=scriptParamEntries[i].name;
            e.unit=scriptParamEntries[i].unit;
            e.value=scriptParamEntries[i].value;
            e.properties=scriptParamEntries[i].properties;
            _initialScriptParamEntries.push_back(e);
        }
    }
}

void CLuaScriptParameters::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        std::vector<SScriptParamEntry> currentScriptParamEntries(scriptParamEntries);
        scriptParamEntries.clear();
        for (size_t i=0;i<_initialScriptParamEntries.size();i++)
        {
            if (_initialScriptParamEntries[i].properties&2)
            { // parameter is persistent
                scriptParamEntries.push_back(currentScriptParamEntries[i]);
            }
            else
            {
                scriptParamEntries.push_back(_initialScriptParamEntries[i]);
            }
        }
    }
    _initialValuesInitialized=false;
}

void CLuaScriptParameters::clearInitialParameters()
{
    _initialValuesInitialized=false;
    _initialScriptParamEntries.clear();
}

void CLuaScriptParameters::moveItem(int index,int newIndex)
{ // should only be called when simulation is stopped
    if ((index>=0)&&(index<int(scriptParamEntries.size()))&&(newIndex>=0)&&(newIndex<int(scriptParamEntries.size())))
    {
        SScriptParamEntry tmp;
        tmp.name=scriptParamEntries[newIndex].name;
        tmp.unit=scriptParamEntries[newIndex].unit;
        tmp.value=scriptParamEntries[newIndex].value;
        tmp.properties=scriptParamEntries[newIndex].properties;
        scriptParamEntries[newIndex].name=scriptParamEntries[index].name;
        scriptParamEntries[newIndex].unit=scriptParamEntries[index].unit;
        scriptParamEntries[newIndex].value=scriptParamEntries[index].value;
        scriptParamEntries[newIndex].properties=scriptParamEntries[index].properties;
        scriptParamEntries[index].name=tmp.name;
        scriptParamEntries[index].unit=tmp.unit;
        scriptParamEntries[index].value=tmp.value;
        scriptParamEntries[index].properties=tmp.properties;
    }
}

bool CLuaScriptParameters::setParameterValue(const char* paramName,const char* paramValue,int paramValueLength)
{
    int ind=getParameterIndex(paramName);
    if (ind>=0)
    {
        scriptParamEntries[ind].value.assign(paramValue,paramValueLength);
        scriptParamEntries[ind].properties=(scriptParamEntries[ind].properties|4)-4; // Indicates that string doesn't contain embedded 0's
        for (int i=0;i<paramValueLength;i++)
        {
            if (paramValue[i]==0)
            {
                scriptParamEntries[ind].properties|=4; // Indicates that string contains embedded 0's
                break;
            }
        }
        return(true);
    }
    return(false);
}

bool CLuaScriptParameters::getParameterValue(const char* paramName,std::string& paramValue)
{
    int ind=getParameterIndex(paramName);
    if (ind>=0)
    {
        paramValue=scriptParamEntries[ind].value;
        return(true);
    }
    return(false);
}

void CLuaScriptParameters::addParameterValue(const char* paramName,const char* unitText,const char* paramValue,int paramValueLength)
{
    int ind=getParameterIndex(paramName);
    if (ind<0)
    { // parameter not yet present!
        SScriptParamEntry e;
        e.name=paramName;
        e.properties=0;
        scriptParamEntries.push_back(e);
        ind=(int)scriptParamEntries.size()-1;
    }
    scriptParamEntries[ind].unit=unitText;
    scriptParamEntries[ind].value.assign(paramValue,paramValueLength);
    for (int i=0;i<paramValueLength;i++)
    {
        if (paramValue[i]==0)
        {
            scriptParamEntries[ind].properties|=4; // Indicates that string contains embedded 0's
            break;
        }
    }
}

bool CLuaScriptParameters::removeParameterValue(int index)
{
    if ( (index<0)||(index>=int(scriptParamEntries.size())) )
        return(false);
    scriptParamEntries.erase(scriptParamEntries.begin()+index);
    return(true);
}

bool CLuaScriptParameters::removeParameterValue(const char* paramName)
{
    int ind=getParameterIndex(paramName);
    if (ind<0)
        return(false);
    return(removeParameterValue(ind));
}

int CLuaScriptParameters::getParameterIndex(const char* paramName)
{
    for (size_t i=0;i<scriptParamEntries.size();i++)
    {
        if (scriptParamEntries[i].name==paramName)
            return((int)i);
    }
    return(-1);
}

CLuaScriptParameters* CLuaScriptParameters::copyYourself()
{
    // First the regular stuff:
    CLuaScriptParameters* p=new CLuaScriptParameters();
    for (size_t i=0;i<scriptParamEntries.size();i++)
        p->scriptParamEntries.push_back(scriptParamEntries[i]);

    //// Now we also have to copy the temp values (values created/modified/deleted during simulation)
    //for (int i=0;i<int(_initialParameterNames.size());i++)
    //{
    //  p->_initialParameterNames.push_back(_initialParameterNames[i]);
    //  p->_initialParameterUnits.push_back(_initialParameterUnits[i]);
    //  p->_initialParameterValues.push_back(_initialParameterValues[i]);
    //  p->_initialParameterProperties.push_back(_initialParameterProperties[i]);
    //}
    //p->_initialValuesInitialized=_initialValuesInitialized;
    return(p);  
}

void CLuaScriptParameters::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Par");
            ar << int(scriptParamEntries.size());
            for (size_t i=0;i<scriptParamEntries.size();i++)
            {
                ar << scriptParamEntries[i].name;
                ar << scriptParamEntries[i].unit;
                ar << scriptParamEntries[i].value;
                ar << scriptParamEntries[i].properties;
            }
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Par")==0)
                    {
                        noHit=false;
                        int paramCount;
                        ar >> byteQuantity;
                        ar >> paramCount;
                        for (int i=0;i<paramCount;i++)
                        {
                            SScriptParamEntry e;
                            ar >> e.name;
                            ar >> e.unit;
                            ar >> e.value;
                            ar >> e.properties;
                            scriptParamEntries.push_back(e);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}
