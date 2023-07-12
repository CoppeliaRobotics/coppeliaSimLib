#pragma once

class CBroadcastData  
{
public:
    CBroadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,double timeOutSimulationTime,double actionRadius,int antennaHandle,double emissionAngle1,double emissionAngle2,const char* data,int dataLength);
    virtual ~CBroadcastData();

    char* receiveData(int receiverID,double simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int& senderID,int& dataHeaderR,std::string& dataNameR,bool removeMessageForThisReceiver);
    bool doesRequireDestruction(double simulationTime);
    bool receiverPresent(int receiverID);
    int getAntennaHandle();

protected:
    int _emitterID;
    int _dataHeader;
    std::string _dataName;
    int _receiverID;
    double _timeOutSimulationTime;
    double _actionRadius;
    int _antennaHandle;
    double _emissionAngle1;
    double _emissionAngle2;
    char* _data;
    int _dataLength;
    std::vector<int> _receivedReceivers;
};
