
#pragma once

#include <7Vector.h>

class CGhostObject
{
public:
    CGhostObject(); // only for serialization
    CGhostObject(int theGroupId,int theObjectHandle,C7Vector theTr,int theOptions,float theStartTime,float theEndTime,const float theColor[12]);
    virtual ~CGhostObject();

    void render(int displayAttributes,float simulationTime,float realTime);
    void modifyAttributes(int theOptions,int theOptionsMask);
    CGhostObject* copyYourself();

    int groupId;
    int ghostId;
    int objectHandle;
    int options;
    float startTime;
    float endTime;
    unsigned char transparencyFactor; // 0=opaque, 255=totally transparent
    float color[12];
    C7Vector tr;
};
