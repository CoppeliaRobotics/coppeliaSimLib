#include "userParameters.h"
#include "app.h"
#include "base64.h"

CUserParameters::CUserParameters()
{
    _initialValuesInitialized=false;
}

CUserParameters::~CUserParameters()
{
}

void CUserParameters::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    clearInitialParameters();
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        for (size_t i=0;i<userParamEntries.size();i++)
        {
            SUserParamEntry e;
            e.name=userParamEntries[i].name;
            e.unit=userParamEntries[i].unit;
            e.value=userParamEntries[i].value;
            e.properties=userParamEntries[i].properties;
            _initialUserParamEntries.push_back(e);
        }
    }
}

void CUserParameters::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        std::vector<SUserParamEntry> currentScriptParamEntries(userParamEntries);
        userParamEntries.clear();
        for (size_t i=0;i<_initialUserParamEntries.size();i++)
        {
            if (_initialUserParamEntries[i].properties&2)
            { // parameter is persistent
                userParamEntries.push_back(currentScriptParamEntries[i]);
            }
            else
            {
                userParamEntries.push_back(_initialUserParamEntries[i]);
            }
        }
    }
    _initialValuesInitialized=false;
}

void CUserParameters::clearInitialParameters()
{
    _initialValuesInitialized=false;
    _initialUserParamEntries.clear();
}

void CUserParameters::moveItem(int index,int newIndex)
{ // should only be called when simulation is stopped
    if ((index>=0)&&(index<int(userParamEntries.size()))&&(newIndex>=0)&&(newIndex<int(userParamEntries.size())))
    {
        SUserParamEntry tmp;
        tmp.name=userParamEntries[newIndex].name;
        tmp.unit=userParamEntries[newIndex].unit;
        tmp.value=userParamEntries[newIndex].value;
        tmp.properties=userParamEntries[newIndex].properties;
        userParamEntries[newIndex].name=userParamEntries[index].name;
        userParamEntries[newIndex].unit=userParamEntries[index].unit;
        userParamEntries[newIndex].value=userParamEntries[index].value;
        userParamEntries[newIndex].properties=userParamEntries[index].properties;
        userParamEntries[index].name=tmp.name;
        userParamEntries[index].unit=tmp.unit;
        userParamEntries[index].value=tmp.value;
        userParamEntries[index].properties=tmp.properties;
    }
}

bool CUserParameters::setParameterValue(const char* paramName,const char* paramValue,int paramValueLength)
{
    int ind=getParameterIndex(paramName);
    if (ind>=0)
    {
        userParamEntries[ind].value.assign(paramValue,paramValueLength);
        userParamEntries[ind].properties=(userParamEntries[ind].properties|4)-4; // Indicates that string doesn't contain embedded 0's
        for (int i=0;i<paramValueLength;i++)
        {
            if (paramValue[i]==0)
            {
                userParamEntries[ind].properties|=4; // Indicates that string contains embedded 0's
                break;
            }
        }
        return(true);
    }
    return(false);
}

bool CUserParameters::getParameterValue(const char* paramName,std::string& paramValue)
{
    int ind=getParameterIndex(paramName);
    if (ind>=0)
    {
        paramValue=userParamEntries[ind].value;
        return(true);
    }
    return(false);
}

void CUserParameters::addParameterValue(const char* paramName,const char* unitText,const char* paramValue,int paramValueLength)
{
    int ind=getParameterIndex(paramName);
    if (ind<0)
    { // parameter not yet present!
        SUserParamEntry e;
        e.name=paramName;
        e.properties=0;
        userParamEntries.push_back(e);
        ind=(int)userParamEntries.size()-1;
    }
    userParamEntries[ind].unit=unitText;
    userParamEntries[ind].value.assign(paramValue,paramValueLength);
    for (int i=0;i<paramValueLength;i++)
    {
        if (paramValue[i]==0)
        {
            userParamEntries[ind].properties|=4; // Indicates that string contains embedded 0's
            break;
        }
    }
}

bool CUserParameters::removeParameterValue(int index)
{
    if ( (index<0)||(index>=int(userParamEntries.size())) )
        return(false);
    userParamEntries.erase(userParamEntries.begin()+index);
    return(true);
}

bool CUserParameters::removeParameterValue(const char* paramName)
{
    int ind=getParameterIndex(paramName);
    if (ind<0)
        return(false);
    return(removeParameterValue(ind));
}

int CUserParameters::getParameterIndex(const char* paramName)
{
    for (size_t i=0;i<userParamEntries.size();i++)
    {
        if (userParamEntries[i].name==paramName)
            return((int)i);
    }
    return(-1);
}

CUserParameters* CUserParameters::copyYourself()
{
    // First the regular stuff:
    CUserParameters* p=new CUserParameters();
    for (size_t i=0;i<userParamEntries.size();i++)
        p->userParamEntries.push_back(userParamEntries[i]);

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

void CUserParameters::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Par");
            ar << int(userParamEntries.size());
            for (size_t i=0;i<userParamEntries.size();i++)
            {
                ar << userParamEntries[i].name;
                ar << userParamEntries[i].unit;
                ar << userParamEntries[i].value;
                ar << userParamEntries[i].properties;
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
                            SUserParamEntry e;
                            ar >> e.name;
                            ar >> e.unit;
                            ar >> e.value;
                            ar >> e.properties;
                            userParamEntries.push_back(e);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            for (size_t i=0;i<userParamEntries.size();i++)
            {
                ar.xmlPushNewNode("parameter");
                ar.xmlAddNode_string("name",userParamEntries[i].name.c_str());
                ar.xmlAddNode_string("unit",userParamEntries[i].unit.c_str());
                std::string str(base64_encode((unsigned char*)userParamEntries[i].value.c_str(),userParamEntries[i].value.size()));
                ar.xmlAddNode_string("value",str.c_str());
                ar.xmlAddNode_int("properties",userParamEntries[i].properties);
                ar.xmlPopNode();
            }

        }
        else
        {
            if (ar.xmlPushChildNode("parameter",false))
            {
                while (true)
                {
                    SUserParamEntry e;
                    ar.xmlGetNode_string("name",e.name);
                    ar.xmlGetNode_string("unit",e.unit);
                    ar.xmlGetNode_string("value",e.value);
                    e.value=base64_decode(e.value);
                    ar.xmlGetNode_int("properties",e.properties);
                    userParamEntries.push_back(e);
                    if (!ar.xmlPushSiblingNode("parameter",false))
                        break;
                }
                ar.xmlPopNode();
            }

        }
    }
}
