#include "_distanceObject_.h"
#include "simConst.h"

_CDistanceObject_::_CDistanceObject_()
{
    _entity1Handle=-1;
    _entity2Handle=-1;
    _objectHandle=-1;
    _displaySegment=true;
    _explicitHandling=false;
    _threshold=0.5f;
    _thresholdEnabled=false;
    _segmentWidth=2;
}

_CDistanceObject_::~_CDistanceObject_()
{
}

void _CDistanceObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color object
            routing.erase(routing.begin());
            _segmentColor.synchronizationMsg(routing,msg);
        }
    }
    else
    { // message is for this distance object
        /*
        if (msg.msg==sim_syncobj_distanceobject_create)
        {
            setObjectHandle(((int*)msg.data)[0]);
            return;
        }
        */
        if (msg.msg==sim_syncobj_distanceobject_setexplicithandling)
        {
            setExplicitHandling(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setobjectname)
        {
            setObjectName(((char*)msg.data),true);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setthreshold)
        {
            setThreshold(((float*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setthresholdenabled)
        {
            setThresholdEnabled(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setsegmentwidth)
        {
            setSegmentWidth(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setdisplaysegment)
        {
            setDisplaySegment(((bool*)msg.data)[0]);
            return;
        }
    }
}

int _CDistanceObject_::getObjectHandle() const
{
    return(_objectHandle);
}

int _CDistanceObject_::getEntity1Handle() const
{
    return(_entity1Handle);
}

int _CDistanceObject_::getEntity2Handle() const
{
    return(_entity2Handle);
}

std::string _CDistanceObject_::getObjectName() const
{
    return(_objectName);
}

bool _CDistanceObject_::setObjectName(const char* newName,bool check)
{
    bool diff=(_objectName!=newName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectName=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff&&getObjectCanChange());
}

bool _CDistanceObject_::setThreshold(float tr)
{
    bool diff=(_threshold!=tr);
    if (diff)
    {
        if (getObjectCanChange())
            _threshold=tr;
        if (getObjectCanSync())
            _setThreshold_send(tr);
    }
    return(diff&&getObjectCanChange());
}

float _CDistanceObject_::getTreshhold() const
{
    return(_threshold);
}

bool _CDistanceObject_::setThresholdEnabled(bool enabled)
{
    bool diff=(_thresholdEnabled!=enabled);
    if (diff)
    {
        if (getObjectCanChange())
            _thresholdEnabled=enabled;
        if (getObjectCanSync())
            _setThresholdEnabled_send(enabled);
    }
    return(diff&&getObjectCanChange());
}

bool _CDistanceObject_::getTreshholdEnabled() const
{
    return(_thresholdEnabled);
}

bool _CDistanceObject_::setDisplaySegment(bool display)
{
    bool diff=(_displaySegment!=display);
    if (diff)
    {
        if (getObjectCanChange())
            _displaySegment=display;
        if (getObjectCanSync())
            _setDisplaySegment_send(display);
    }
    return(diff&&getObjectCanChange());
}

bool _CDistanceObject_::setExplicitHandling(bool explicitHandl)
{
    bool diff=(_explicitHandling!=explicitHandl);
    if (diff)
    {
        if (getObjectCanChange())
            _explicitHandling=explicitHandl;
        if (getObjectCanSync())
            _setExplicitHandling_send(explicitHandl);
    }
    return(diff&&getObjectCanChange());
}

bool _CDistanceObject_::getExplicitHandling() const
{
    return(_explicitHandling);
}

bool _CDistanceObject_::getDisplaySegment() const
{
    return(_displaySegment);
}

bool _CDistanceObject_::setSegmentWidth(int w)
{
    bool diff=(_segmentWidth!=w);
    if (diff)
    {
        if (getObjectCanChange())
            _segmentWidth=w;
        if (getObjectCanSync())
            _setSegmentWidth_send(w);
    }
    return(diff&&getObjectCanChange());
}

int _CDistanceObject_::getSegmentWidth() const
{
    return(_segmentWidth);
}

CColorObject* _CDistanceObject_::getSegmentColor()
{
    return(&_segmentColor);
}

bool _CDistanceObject_::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff);
}

void _CDistanceObject_::_setObjectName_send(const char* newName) const
{
    sendString(newName,sim_syncobj_distanceobject_setobjectname);
}

void _CDistanceObject_::_setThreshold_send(float tr) const
{
    sendFloat(tr,sim_syncobj_distanceobject_setthreshold);
}

void _CDistanceObject_::_setThresholdEnabled_send(bool enabled) const
{
    sendBool(enabled,sim_syncobj_distanceobject_setthresholdenabled);
}

void _CDistanceObject_::_setDisplaySegment_send(bool display) const
{
    sendBool(display,sim_syncobj_distanceobject_setdisplaysegment);
}

void _CDistanceObject_::_setExplicitHandling_send(bool explicitHandl) const
{
    sendBool(explicitHandl,sim_syncobj_distanceobject_setexplicithandling);
}

void _CDistanceObject_::_setSegmentWidth_send(int w) const
{
    sendInt32(w,sim_syncobj_distanceobject_setsegmentwidth);
}

