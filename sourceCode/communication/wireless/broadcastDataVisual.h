
#pragma once

#include "3Vector.h"
#include "7Vector.h"

class CBroadcastDataVisual  
{
public:
    CBroadcastDataVisual(float timeOutSimulationTime,float actionRadius,const C7Vector& emitterConf,float emissionAngle1,float emissionAngle2);
    CBroadcastDataVisual(const C3Vector& emitterPos,const C3Vector& receiverPos);
    virtual ~CBroadcastDataVisual();

    bool doesRequireDestruction(float simulationTime);
    void visualize();
    bool isReceiver();

    float _timeOutSimulationTime;
    float _actionRadius;
    C7Vector _emitterConf;
    C3Vector _receiverPos;
    float _emissionAngle1;
    float _emissionAngle2;
    bool _emitter;
};
