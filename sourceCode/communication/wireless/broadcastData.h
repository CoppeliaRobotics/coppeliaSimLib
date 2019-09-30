
#pragma once

class CBroadcastData  
{
public:
    CBroadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,float timeOutSimulationTime,float actionRadius,int antennaHandle,float emissionAngle1,float emissionAngle2,const char* data,int dataLength);
    virtual ~CBroadcastData();

    char* receiveData(int receiverID,float simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int& senderID,int& dataHeaderR,std::string& dataNameR,bool removeMessageForThisReceiver);
    bool doesRequireDestruction(float simulationTime);
    bool receiverPresent(int receiverID);
    int getAntennaHandle();

protected:
    int _emitterID;
    int _dataHeader;
    std::string _dataName;
    int _receiverID;
    float _timeOutSimulationTime;
    float _actionRadius;
    int _antennaHandle;
    float _emissionAngle1;
    float _emissionAngle2;
    char* _data;
    int _dataLength;
    std::vector<int> _receivedReceivers;
};
