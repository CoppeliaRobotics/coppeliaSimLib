#pragma once

#include <commTube.h>

class CCommTubeContainer 
{
public:
    CCommTubeContainer();
    virtual ~CCommTubeContainer();
    void simulationEnded();

    int openTube(int header,const char* identifier,bool killAtSimulationEnd,int readBufferSize); // return value is the tube handle for this partner
    bool closeTube(int tubeHandle); // returns true if tube could be closed

    bool writeToTube_copyBuffer(int tubeHandle,const char* data,int dataLength);
    char* readFromTube_bufferNotCopied(int tubeHandle,int& dataLength);
    int getTubeStatus(int tubeHandle,int& readBufferFill,int& writeBufferFill); // -1: not existant, 0: not connected, 1: connected

    void removeAllTubes();

protected:
    int _getTubeIndexForHandle(int tubeHandle);

    std::vector<CCommTube*> _allTubes;

    static int _nextPartnerID;
};
