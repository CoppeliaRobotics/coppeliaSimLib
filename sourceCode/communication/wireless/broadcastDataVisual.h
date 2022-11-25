#pragma once

#include "3Vector.h"
#include "7Vector.h"

class CBroadcastDataVisual  
{
public:
    CBroadcastDataVisual(floatDouble timeOutSimulationTime,floatDouble actionRadius,const C7Vector& emitterConf,floatDouble emissionAngle1,floatDouble emissionAngle2);
    CBroadcastDataVisual(const C3Vector& emitterPos,const C3Vector& receiverPos);
    virtual ~CBroadcastDataVisual();

    bool doesRequireDestruction(floatDouble simulationTime);
    void visualize();
    bool isReceiver();

    floatDouble _timeOutSimulationTime;
    floatDouble _actionRadius;
    C7Vector _emitterConf;
    C3Vector _receiverPos;
    floatDouble _emissionAngle1;
    floatDouble _emissionAngle2;
    bool _emitter;
};
