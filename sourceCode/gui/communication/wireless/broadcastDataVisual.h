#pragma once

#include <simMath/3Vector.h>
#include <simMath/7Vector.h>

class CBroadcastDataVisual  
{
public:
    CBroadcastDataVisual(double timeOutSimulationTime,double actionRadius,const C7Vector& emitterConf,double emissionAngle1,double emissionAngle2);
    CBroadcastDataVisual(const C3Vector& emitterPos,const C3Vector& receiverPos);
    virtual ~CBroadcastDataVisual();

    bool doesRequireDestruction(double simulationTime);
    void visualize();
    bool isReceiver();

    double _timeOutSimulationTime;
    double _actionRadius;
    C7Vector _emitterConf;
    C3Vector _receiverPos;
    double _emissionAngle1;
    double _emissionAngle2;
    bool _emitter;
};
