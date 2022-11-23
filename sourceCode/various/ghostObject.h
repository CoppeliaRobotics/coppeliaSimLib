#pragma once

#include <7Vector.h>

class CGhostObject
{
public:
    CGhostObject(); // only for serialization
    CGhostObject(int theGroupId,int theObjectHandle,C7Vector theTr,int theOptions,floatDouble theStartTime,floatDouble theEndTime,const floatDouble theColor[12]);
    virtual ~CGhostObject();

    void render(int displayAttributes,floatDouble simulationTime,floatDouble realTime);
    void modifyAttributes(int theOptions,int theOptionsMask);
    CGhostObject* copyYourself();

    int groupId;
    int ghostId;
    int objectHandle;
    int options;
    floatDouble startTime;
    floatDouble endTime;
    unsigned char transparencyFactor; // 0=opaque, 255=totally transparent
    floatDouble color[12];
    C7Vector tr;
};
