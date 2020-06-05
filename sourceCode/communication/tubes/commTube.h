#pragma once

#include <string>
#include <vector>

class CCommTube
{

public:
    CCommTube(int header,const char* identifier,int firstPartner,bool killAtSimulationEnd,int readBufferSize);
    ~CCommTube();

    bool isConnected();
    void connectPartner(int secondPartner,bool killAtSimulationEnd,int readBufferSize);
    bool disconnectPartner(int partner); // return value true means this object needs destruction
    bool simulationEnded(); // return value true means this object needs destruction
    bool writeData(int partner,char* data,int dataSize); // data is not copied!
    char* readData(int partner,int& dataSize); // data is not copied!
    bool isPartnerThere(int partner);
    bool isSameHeaderAndIdentifier(int header,const char* identifier);
    int getTubeStatus(int tubeHandle,int& readBufferFill,int& writeBufferFill); // -1: not existant, 0: not connected, 1: connected

protected:

    void _removeAllPackets();
    void _removePacketsOfPartner(int partnerIndex);
    void _swapPartners();

    int _header;
    std::string _identifier;
    int _partner[2];
    bool _killPartnerAtSimulationEnd[2]; // false --> don't kill
    int _readBufferSizes[2];
    std::vector<char*> _packets[2]; // _packets[0] is from partner2 to partner1, packets[1] is from partner1 to partner2
    std::vector<int> _packetSizes[2];
};
