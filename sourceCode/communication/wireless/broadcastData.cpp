
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "broadcastData.h"
#include "app.h"

CBroadcastData::CBroadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,float timeOutSimulationTime,float actionRadius,int antennaHandle,float emissionAngle1,float emissionAngle2,const char* data,int dataLength)
{
    _emitterID=emitterID;
    _receiverID=targetID;
    _dataHeader=dataHeader;
    _dataName=dataName;
    _timeOutSimulationTime=timeOutSimulationTime;
    _actionRadius=actionRadius;
    _antennaHandle=antennaHandle;
    _emissionAngle1=emissionAngle1;
    _emissionAngle2=emissionAngle2;
    _data=new char[dataLength];
    _dataLength=dataLength;
    for (int i=0;i<dataLength;i++)
        _data[i]=data[i];
}

CBroadcastData::~CBroadcastData()
{
    delete[] _data;
}

bool CBroadcastData::doesRequireDestruction(float simulationTime)
{
    return(simulationTime>_timeOutSimulationTime);
}

bool CBroadcastData::receiverPresent(int receiverID)
{
    for (int i=0;i<int(_receivedReceivers.size());i++)
    {
        if (_receivedReceivers[i]==receiverID)
            return(true);
    }
    return(false);
}

int CBroadcastData::getAntennaHandle()
{
    return(_antennaHandle);
}

char* CBroadcastData::receiveData(int receiverID,float simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int& senderID,int& dataHeaderR,std::string& dataNameR,bool removeMessageForThisReceiver)
{
    C7Vector antennaConf1;
    antennaConf1.setIdentity();
    if (_antennaHandle!=sim_handle_default)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(_antennaHandle);
        if (it==nullptr)
            return(nullptr); // the emission antenna was destroyed!
        antennaConf1=it->getCumulativeTransformationPart1();
    }

    C3Vector antennaPos2;
    antennaPos2.clear();
    if (antennaHandle!=sim_handle_default)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(antennaHandle);
        if (it==nullptr)
            return(nullptr); // that shouldn't happen!
        antennaPos2=it->getCumulativeTransformationPart1().X;
    }

    if ((_emitterID==receiverID)&&(_emitterID!=0)) // second part since 25/9/2012: from c/c++, emitter/receiver ID is always 0
        return(nullptr); // the emitter cannot receive its own message!
    if (simulationTime>_timeOutSimulationTime)
        return(nullptr); // data timed out!
    if ( (dataHeader!=-1)&&(dataHeader!=_dataHeader) )
        return(nullptr); // wrong data header!
    if ( (dataName.length()!=0)&&(dataName.compare(_dataName)!=0) )
        return(nullptr); // wrong data name!
    if (_receiverID!=sim_handle_all)
    { // message not for everyone
        if (_receiverID==sim_handle_tree)
        { // we have to check if receiverID has a parent _emitterID:
            CLuaScriptObject* rec=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(receiverID);
            CLuaScriptObject* em=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(_emitterID);
            if ( (rec==nullptr)||(em==nullptr) )
                return(nullptr);
            if (em->getScriptType()!=sim_scripttype_mainscript)
            {
                if (rec->getScriptType()==sim_scripttype_mainscript)
                    return(nullptr);
                C3DObject* recObj=App::ct->objCont->getObjectFromHandle(rec->getObjectIDThatScriptIsAttachedTo_child());
                C3DObject* emObj=App::ct->objCont->getObjectFromHandle(em->getObjectIDThatScriptIsAttachedTo_child());
                bool found=false;
                while (recObj!=nullptr)
                {
                    if (recObj==emObj)
                    {
                        found=true;
                        break;
                    }
                    recObj=recObj->getParentObject();
                }
                if (!found)
                    return(nullptr);
            }
        }
        if (_receiverID==sim_handle_chain)
        { // we have to check if _emitterID has a parent receiverID:
            CLuaScriptObject* rec=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(receiverID);
            CLuaScriptObject* em=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(_emitterID);
            if ( (rec==nullptr)||(em==nullptr) )
                return(nullptr);
            if (rec->getScriptType()!=sim_scripttype_mainscript)
            {
                if (em->getScriptType()==sim_scripttype_mainscript)
                    return(nullptr);
                C3DObject* recObj=App::ct->objCont->getObjectFromHandle(rec->getObjectIDThatScriptIsAttachedTo_child());
                C3DObject* emObj=App::ct->objCont->getObjectFromHandle(em->getObjectIDThatScriptIsAttachedTo_child());
                bool found=false;
                while (emObj!=nullptr)
                {
                    if (recObj==emObj)
                    {
                        found=true;
                        break;
                    }
                    emObj=emObj->getParentObject();
                }
                if (!found)
                    return(nullptr);
            }
        }
        if (_receiverID>=0)
        {
            if (_receiverID!=receiverID)
                return(nullptr);
        }
    }
        
    if ((antennaPos2-antennaConf1.X).getLength()>_actionRadius)
        return(nullptr); // outside of action radius!
    if (_emissionAngle1<piValue_f*0.99f)
    { // Check if inside the vertical "hearing" area:
        C3Vector relPos(antennaConf1.getInverse()*antennaPos2);
        float h=fabs(relPos(2));
        relPos(2)=0.0f;
        float l=relPos.getLength();
        if (l==0.0f)
            return(nullptr); // just outside of the vertical active area (border condition)
        float a=fabs(atan2(h,l));
        if (a>=_emissionAngle1*0.5f)
            return(nullptr); // just outside of the vertical active area (border condition)
    }
    if (_emissionAngle2<piValTimes2_f*0.99f)
    { // Check if inside the horizontal "hearing" area:
        C3Vector relPos(antennaConf1.getInverse()*antennaPos2);
        if (relPos(0)==0.0f)
            return(nullptr); // just outside of the horizontal active area (border condition)
        float a=fabs(atan2(relPos(1),relPos(0)));
        if (a>=_emissionAngle2*0.5f)
            return(nullptr); // just outside of the horizontal active area (border condition)
    }
    if (receiverPresent(receiverID))
        return(nullptr); // data was already read by that script!
    // We can receive the data, finally!
    if (removeMessageForThisReceiver)
        _receivedReceivers.push_back(receiverID); // remember that this receiverID already read the message!
    dataLength=_dataLength;
    senderID=_emitterID;
    dataHeaderR=_dataHeader;
    dataNameR=_dataName;
    return(_data);
}
