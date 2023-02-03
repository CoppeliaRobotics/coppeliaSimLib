#pragma once

#include <simMath/7Vector.h>

class CGhostObject
{
public:
    CGhostObject(); // only for serialization
    CGhostObject(int theGroupId,int theObjectHandle,C7Vector theTr,int theOptions,double theStartTime,double theEndTime,const float theColor[12]);
    virtual ~CGhostObject();

    void render(int displayAttributes,double simulationTime,double realTime);
    void modifyAttributes(int theOptions,int theOptionsMask);
    CGhostObject* copyYourself();

    int groupId;
    int ghostId;
    int objectHandle;
    int options;
    double startTime;
    double endTime;
    unsigned char transparencyFactor; // 0=opaque, 255=totally transparent
    float color[12];
    C7Vector tr;
};
