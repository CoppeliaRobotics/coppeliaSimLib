#pragma once

#include <broadcastData.h>
#ifdef SIM_WITH_GUI
    #include <broadcastDataVisual.h>
#endif

class CBroadcastDataContainer
{
public:
    CBroadcastDataContainer();
    virtual ~CBroadcastDataContainer();

    void eraseAllObjects();
    void removeObject(int index);
    void simulationAboutToStart();
    void simulationEnded();
    void removeTimedOutObjects(double simulationTime);
    void broadcastData(int emitterID,int targetID,int dataHeader,std::string& dataName,double timeOutSimulationTime,double actionRadius,int antennaHandle,double emissionAngle1,double emissionAngle2,const char* data,int dataLength);
    char* receiveData(int receiverID,double simulationTime,int dataHeader,std::string& dataName,int antennaHandle,int& dataLength,int index,int& senderID,int& dataHeaderR,std::string& dataNameR);

    static bool getWirelessForceShow_emission();
    static void setWirelessForceShow_emission(bool f);
    static bool getWirelessForceShow_reception();
    static void setWirelessForceShow_reception(bool f);

private:
    std::vector<CBroadcastData*> _allObjects;
    static bool _wirelessForceShow_emission;
    static bool _wirelessForceShow_reception;

    #ifdef SIM_WITH_GUI
    public:
        void visualizeCommunications(int pcTimeInMs);
    private:
        std::vector<CBroadcastDataVisual*> _allVisualObjects;
    #endif
};
