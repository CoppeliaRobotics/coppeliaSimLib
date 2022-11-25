#pragma once

#include "broadcastData.h"
#include "vMutex.h"
#include "broadcastDataVisual.h"

class CBroadcastDataContainer
{
public:
    CBroadcastDataContainer();
    virtual ~CBroadcastDataContainer();

    void visualizeCommunications(int pcTimeInMs);
    void eraseAllObjects();
    void removeObject(int index);
    void simulationAboutToStart();
    void simulationEnded();
    void removeTimedOutObjects(floatDouble simulationTime);
    void broadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,floatDouble timeOutSimulationTime,floatDouble actionRadius,int antennaHandle,floatDouble emissionAngle1,floatDouble emissionAngle2,const char* data,int dataLength);
    char* receiveData(int receiverID,floatDouble simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int index,int& senderID,int& dataHeaderR,std::string& dataNameR);

    static bool getWirelessForceShow_emission();
    static void setWirelessForceShow_emission(bool f);
    static bool getWirelessForceShow_reception();
    static void setWirelessForceShow_reception(bool f);

private:
    VMutex _objectMutex;

    std::vector<CBroadcastData*> _allObjects;
    std::vector<CBroadcastDataVisual*> _allVisualObjects;
    static bool _wirelessForceShow_emission;
    static bool _wirelessForceShow_reception;
};
