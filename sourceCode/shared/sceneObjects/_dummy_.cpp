#include "_dummy_.h"
#include "simConst.h"
#include "app.h"

_CDummy_::_CDummy_()
{
    _objectType=sim_object_dummy_type;
    _localObjectSpecialProperty=0;

    _dummySize=0.01f;
    _assignedToParentPath=false;
    _assignedToParentPathOrientation=false;
    _linkedDummyHandle=-1;
    _linkType=sim_dummy_linktype_dynamics_loop_closure;
}

_CDummy_::~_CDummy_()
{
}

void _CDummy_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color object
            routing.erase(routing.begin());
            _dummyColor.synchronizationMsg(routing,msg);
        }
    }
    else
    {
        if (msg.msg>=sim_syncobj_sceneobject_cmdstart)
        { // message is for this sceneObject
            _CSceneObject_::synchronizationMsg(routing,msg);
        }
        else
        { // message is for this dummy
            if (msg.msg==sim_syncobj_dummy_size)
            {
                setDummySize(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_linkeddummy)
            {
                setLinkedDummyHandle(((int*)msg.data)[0],true);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_linktype)
            {
                setLinkType(((int*)msg.data)[0],true);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_assignedparentpath)
            {
                setAssignedToParentPath(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_assignedparentpathorientation)
            {
                setAssignedToParentPathOrientation(((bool*)msg.data)[0]);
                return;
            }
        }
    }
}

bool _CDummy_::setDummySize(float s)
{
    bool diff=(_dummySize!=s);
    if (diff)
    {
        _dummySize=s;
        if ( _isInScene&&App::worldContainer->getEnableEvents() )
        {
            const char* cmd="size";
            auto [event,data]=App::worldContainer->createObjectEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,false);
            data->appendMapObject_stringFloat(cmd,_dummySize);
            App::worldContainer->pushEvent(event);
        }
    }
    return(diff);
}

void _CDummy_::_setDummySize_send(float s) const
{
}

bool _CDummy_::setLinkedDummyHandle(int h,bool check)
{
    bool diff=(_linkedDummyHandle!=h);
    if (diff)
    {
        _linkedDummyHandle=h;
        if (getObjectCanSync())
            _setLinkedDummyHandle_send(h);
    }
    return(diff);
}

void _CDummy_::_setLinkedDummyHandle_send(int h) const
{
}

bool _CDummy_::setLinkType(int lt,bool check)
{
    bool diff=(_linkType!=lt);
    if (diff)
    {
        _linkType=lt;
        if (getObjectCanSync())
            _setLinkType_send(lt);
    }
    return(diff);
}

void _CDummy_::_setLinkType_send(int t) const
{
}

bool _CDummy_::setAssignedToParentPath(bool a)
{
    bool diff=(_assignedToParentPath!=a);
    if (diff)
    {
        _assignedToParentPath=a;
        if (getObjectCanSync())
            _setAssignedToParentPath_send(a);
    }
    return(diff);
}

void _CDummy_::_setAssignedToParentPath_send(bool a) const
{
}

bool _CDummy_::setAssignedToParentPathOrientation(bool a)
{
    bool diff=(_assignedToParentPathOrientation!=a);
    if (diff)
    {
        _assignedToParentPathOrientation=a;
        if (getObjectCanSync())
            _setAssignedToParentPathOrientation_send(a);
    }
    return(diff);
}

void _CDummy_::_setAssignedToParentPathOrientation_send(bool a) const
{
}

bool _CDummy_::getAssignedToParentPath() const
{
    return(_assignedToParentPath);
}

bool _CDummy_::getAssignedToParentPathOrientation() const
{
    return(_assignedToParentPathOrientation);
}

float _CDummy_::getDummySize() const
{
    return(_dummySize);
}

CColorObject* _CDummy_::getDummyColor()
{
    return(&_dummyColor);
}

int _CDummy_::getLinkType() const
{
    return(_linkType);
}


int _CDummy_::getLinkedDummyHandle() const
{
    return(_linkedDummyHandle);
}

/*
void _CDummy_::_connectIkPluginCounterpart_()
{
    // Synchronize with IK plugin:
    if ( (_linkedDummyHandle!=-1)&&(_linkType==sim_dummy_linktype_ik_tip_target) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_linkedDummyHandle);
        CPluginContainer::ikPlugin_setLinkedDummy(_ikPluginCounterpartHandle_,it->getIkPluginCounterpartHandle());
    }
}

void _CDummy_::_rebuildIkPluginCounterpart_(int counterpartHandle)
{
    // Synchronize with IK plugin:
    _ikPluginCounterpartHandle_=counterpartHandle;
}
*/
