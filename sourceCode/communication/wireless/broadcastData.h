#pragma once

class CBroadcastData  
{
public:
    CBroadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,floatDouble timeOutSimulationTime,floatDouble actionRadius,int antennaHandle,floatDouble emissionAngle1,floatDouble emissionAngle2,const char* data,int dataLength);
    virtual ~CBroadcastData();

    char* receiveData(int receiverID,floatDouble simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int& senderID,int& dataHeaderR,std::string& dataNameR,bool removeMessageForThisReceiver);
    bool doesRequireDestruction(floatDouble simulationTime);
    bool receiverPresent(int receiverID);
    int getAntennaHandle();

protected:
    int _emitterID;
    int _dataHeader;
    std::string _dataName;
    int _receiverID;
    floatDouble _timeOutSimulationTime;
    floatDouble _actionRadius;
    int _antennaHandle;
    floatDouble _emissionAngle1;
    floatDouble _emissionAngle2;
    char* _data;
    int _dataLength;
    std::vector<int> _receivedReceivers;
};
