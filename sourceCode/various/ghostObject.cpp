
#include "vrepMainHeader.h"
#include "ghostObject.h"
#include "app.h"
#include "ghostRendering.h"

CGhostObject::CGhostObject()
{ // only for serialization or copy
    transparencyFactor=0;
}

CGhostObject::CGhostObject(int theGroupId,int theObjectHandle,C7Vector theTr,int theOptions,float theStartTime,float theEndTime,const float theColor[12])
{
    // options: bit0 set (1): model instead of object
    // options: bit1 set (2): real-time playback
    // options: bit2 set (4): original colors
    // options: bit3 set (8): force invisible objects to appear too
    // options: bit4 set (16): ghost is invisible (maybe temporarily)
    // options: bit5 set (32): backface culling (when using custom colors only)

    groupId=theGroupId;
    objectHandle=theObjectHandle;
    options=theOptions;
    startTime=theStartTime;
    endTime=theEndTime;
    tr=theTr;
    transparencyFactor=0;
    if (theColor!=nullptr)
    {
        for (int i=0;i<12;i++)
            color[i]=theColor[i];
    }
    else
    {
        for (int i=0;i<12;i++)
            color[i]=0.5f;
    }
}

CGhostObject::~CGhostObject()
{
}

void CGhostObject::modifyAttributes(int theOptions,int theOptionsMask)
{
    // options: bit0 set (1): model instead of object
    // options: bit1 set (2): real-time playback
    // options: bit2 set (4): original colors
    // options: bit3 set (8): force invisible objects to appear too
    // options: bit4 set (16): ghost is invisible (maybe temporarily)
    options=((options|theOptionsMask)-theOptionsMask)|(theOptions&theOptionsMask);
}

CGhostObject* CGhostObject::copyYourself()
{
    CGhostObject* newGhost=new CGhostObject();
    newGhost->groupId=groupId;
    newGhost->ghostId=ghostId;
    newGhost->objectHandle=objectHandle;
    newGhost->options=options;
    newGhost->startTime=startTime;
    newGhost->endTime=endTime;
    newGhost->transparencyFactor=transparencyFactor;
    for (int i=0;i<12;i++)
        newGhost->color[i]=color[i];
    newGhost->tr=tr;
    return(newGhost);
}

void CGhostObject::render(int displayAttributes,float simulationTime,float realTime)
{
    if ((options&16)!=0)
        return; // invisible
    bool doIt=false;
    if ((options&2)!=0)
        doIt=(realTime>=startTime)&&(realTime<endTime);
    else
        doIt=(simulationTime>=startTime-0.00005f)&&(simulationTime<endTime-0.00005f);
    if (doIt)
    {
        CShape* shape=App::ct->objCont->getShape(objectHandle);
        if (shape!=nullptr)
            displayGhost(shape,tr,displayAttributes,options,float(transparencyFactor)/255.0f,color);
    }
}
