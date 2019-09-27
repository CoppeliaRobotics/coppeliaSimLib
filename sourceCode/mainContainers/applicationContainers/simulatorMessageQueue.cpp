
#include "vrepMainHeader.h"
#include "simulatorMessageQueue.h"
#include "app.h"

CSimulatorMessageQueue::CSimulatorMessageQueue()
{
}

CSimulatorMessageQueue::~CSimulatorMessageQueue()
{
    _mutex.lock();
    for (int i=0;i<int(_messages.size());i++)
        delete[] _messages[i].data;
    _mutex.unlock();
}

void CSimulatorMessageQueue::addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,char* data,int dataSize)
{
    _mutex.lock();
    SMessageQueueMessage msg;
    msg.messageID=commandID;
    msg.auxValues[0]=auxVal1;
    msg.auxValues[1]=auxVal2;
    msg.auxValues[2]=auxVal3;
    msg.auxValues[3]=auxVal4;
    msg.data=data;
    msg.dataLength=dataSize;
    _messages.push_back(msg);
    _mutex.unlock();
}

char* CSimulatorMessageQueue::extractOneCommand(int& commandID,int auxVals[4],int& dataSize)
{
    _mutex.lock();
    char* retVal=nullptr;
    commandID=-1;
    if (_messages.size()!=0)
    {
        commandID=_messages[0].messageID;
        auxVals[0]=_messages[0].auxValues[0];
        auxVals[1]=_messages[0].auxValues[1];
        auxVals[2]=_messages[0].auxValues[2];
        auxVals[3]=_messages[0].auxValues[3];
        dataSize=_messages[0].dataLength;
        retVal=_messages[0].data;
        _messages.erase(_messages.begin());
    }
    _mutex.unlock();
    return(retVal);
}
