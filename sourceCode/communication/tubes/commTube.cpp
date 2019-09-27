
#include "vrepMainHeader.h"
#include "commTube.h"
#include "tt.h"


CCommTube::CCommTube(int header,const std::string& identifier,int firstPartner,bool killAtSimulationEnd,int readBufferSize)
{
    _header=header;
    _identifier=identifier;
    _partner[0]=firstPartner;
    _partner[1]=-1; // Not yet connected
    _killPartnerAtSimulationEnd[0]=killAtSimulationEnd;
    _killPartnerAtSimulationEnd[1]=true;
    _readBufferSizes[0]=tt::getLimitedInt(1,10000,readBufferSize);
}

CCommTube::~CCommTube()
{
}

bool CCommTube::isConnected()
{
    return(_partner[1]!=-1);
}

void CCommTube::connectPartner(int secondPartner,bool killAtSimulationEnd,int readBufferSize)
{
    _partner[1]=secondPartner;
    _killPartnerAtSimulationEnd[1]=killAtSimulationEnd;
    _readBufferSizes[1]=tt::getLimitedInt(1,10000,readBufferSize);
}

bool CCommTube::disconnectPartner(int partner)
{ // return value true means this object needs destruction
    if (_partner[1]==partner)
    {
        _partner[1]=-1;
        _killPartnerAtSimulationEnd[1]=true;
        _removePacketsOfPartner(1);
        return(false);
    }
    else
    {
        if (_partner[0]==partner)
        {
            if (_partner[1]==-1)
                return(true); // remove this object!
            else
            { // the second partner becomes now the first:
                _swapPartners();
                _partner[1]=-1;
                _killPartnerAtSimulationEnd[1]=true;
                _removePacketsOfPartner(1);
                return(false);
            }
        }
    }
    return(false);
}

bool CCommTube::simulationEnded()
{ // return value true means this object needs destruction
    if ( (_partner[1]!=-1)&&_killPartnerAtSimulationEnd[1] )
    {
        if (disconnectPartner(_partner[1]))
            return(true);
    }
    if (_killPartnerAtSimulationEnd[0])
    {
        if (disconnectPartner(_partner[0]))
            return(true);
    }
    return(false);
}

void CCommTube::_removeAllPackets()
{
    _removePacketsOfPartner(0);
    _removePacketsOfPartner(1);
}

void CCommTube::_removePacketsOfPartner(int partnerIndex)
{
    for (int i=0;i<int(_packets[partnerIndex].size());i++)
        delete[] _packets[partnerIndex][i];
    _packets[partnerIndex].clear();
    _packetSizes[partnerIndex].clear();
}

void CCommTube::_swapPartners()
{
    int p0Save=_partner[0];
    _partner[0]=_partner[1];
    _partner[1]=p0Save;

    bool p99Save=_killPartnerAtSimulationEnd[0];
    _killPartnerAtSimulationEnd[0]=_killPartnerAtSimulationEnd[1];
    _killPartnerAtSimulationEnd[1]=p99Save;

    p0Save=_readBufferSizes[0];
    _readBufferSizes[0]=_readBufferSizes[1];
    _readBufferSizes[1]=p0Save;

    _packets[0].swap(_packets[1]);
    _packetSizes[0].swap(_packetSizes[1]);
}

bool CCommTube::writeData(int partner,char* data,int dataSize)
{ // data is not copied!
    for (int i=0;i<2;i++)
    {
        if (_partner[i]==partner)
        {
            int j=1-i;
            _packets[j].push_back(data);
            _packetSizes[j].push_back(dataSize);
            if (int(_packets[j].size())>_readBufferSizes[j])
            { // We remove the first element
                delete[] _packets[j][0];
                _packets[j].erase(_packets[j].begin());
                _packetSizes[j].erase(_packetSizes[j].begin());
            }
            return(true);
        }
    }
    return(false);
}

char* CCommTube::readData(int partner,int& dataSize)
{ // data is not copied!
    for (int j=0;j<2;j++)
    {
        if (_partner[j]==partner)
        {
            if (_packets[j].size()!=0)
            {
                char* retVal=_packets[j][0];
                dataSize=_packetSizes[j][0];
                _packets[j].erase(_packets[j].begin());
                _packetSizes[j].erase(_packetSizes[j].begin());
                return(retVal);
            }
            return(nullptr);
        }
    }
    return(nullptr);
}

bool CCommTube::isPartnerThere(int partner)
{
    return( (partner==_partner[0])||(partner==_partner[1]) );
}

bool CCommTube::isSameHeaderAndIdentifier(int header,const std::string& identifier)
{
    return( (header==_header)&&(identifier.compare(_identifier)==0) );
}

int CCommTube::getTubeStatus(int tubeHandle,int& readBufferFill,int& writeBufferFill)
{
    if ( (tubeHandle!=_partner[0])&&(tubeHandle!=_partner[1]) )
        return(-1);
    readBufferFill=0;
    writeBufferFill=0;
    int retVal=0;
    if (isConnected())
    {
        retVal=1;
        if (tubeHandle==_partner[0])
        {
            writeBufferFill=(int)_packets[1].size();
            readBufferFill=(int)_packets[0].size();
        }
        else
        {
            writeBufferFill=(int)_packets[0].size();
            readBufferFill=(int)_packets[1].size();
        }
    }
    else
    {
        if (tubeHandle==_partner[0])
            readBufferFill=(int)_packets[0].size();
        else
            readBufferFill=(int)_packets[1].size();
    }
    return(retVal);
}
