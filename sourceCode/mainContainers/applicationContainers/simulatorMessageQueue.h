#pragma once

#include <vector>
#include <vMutex.h>

struct SMessageQueueMessage
{
    int messageID;
    int auxValues[4];
    char* data;
    int dataLength;
};


class CSimulatorMessageQueue  
{
public:
    CSimulatorMessageQueue();
    virtual ~CSimulatorMessageQueue();

    void addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,char* data,int dataSize);
    char* extractOneCommand(int& commandID,int auxVals[4],int& dataSize);

private:
    std::vector<SMessageQueueMessage> _messages;
    VMutex _mutex;
};
