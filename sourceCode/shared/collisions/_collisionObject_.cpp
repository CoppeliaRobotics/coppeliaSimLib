#include "_collisionObject_.h"
#include "simConst.h"

_CCollisionObject_::_CCollisionObject_()
{
    _entity1Handle=-1;
    _entity2Handle=-1;
    _objectHandle=-1;
    _countourWidth=1;
    _colliderChangesColor=true;
    _collideeChangesColor=false;
    _detectAllCollisions=false;
    _explicitHandling=false;
}

_CCollisionObject_::~_CCollisionObject_()
{
}

void _CCollisionObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color object
            routing.erase(routing.begin());
            _contourColor.synchronizationMsg(routing,msg);
        }
    }
    else
    { // message is for this collision object
        if (msg.msg==sim_syncobj_collisionobject_setexplicithandling)
        {
            setExplicitHandling(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setobjectname)
        {
            setObjectName(((char*)msg.data),true);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setcolliderchangescolor)
        {
            setColliderChangesColor(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setcollideechangescolor)
        {
            setCollideeChangesColor(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setexhaustivedetection)
        {
            setExhaustiveDetection(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setcontourwidth)
        {
            setContourWidth(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collisionobject_setintersections)
        {
            std::vector<float> v(((float*)msg.data),((float*)msg.data)+msg.dataSize);
            setIntersections(&v);
            return;
        }
    }
}

int _CCollisionObject_::getObjectHandle() const
{
    return(_objectHandle);
}

int _CCollisionObject_::getEntity1Handle() const
{
    return(_entity1Handle);
}

int _CCollisionObject_::getEntity2Handle() const
{
    return(_entity2Handle);
}

std::string _CCollisionObject_::getObjectName() const
{
    return(_objectName);
}

bool _CCollisionObject_::setObjectName(const char* newName,bool check)
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

bool _CCollisionObject_::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff&&getObjectCanChange());
}

bool _CCollisionObject_::setColliderChangesColor(bool changes)
{
    bool diff=(_colliderChangesColor!=changes);
    if (diff)
    {
        if (getObjectCanChange())
            _colliderChangesColor=changes;
        if (getObjectCanSync())
            _setColliderChangesColor_send(changes);
    }
    return(diff&&getObjectCanChange());
}

bool _CCollisionObject_::getColliderChangesColor() const
{
    return(_colliderChangesColor);
}

bool _CCollisionObject_::setCollideeChangesColor(bool changes)
{
    bool diff=(_collideeChangesColor!=changes);
    if (diff)
    {
        if (getObjectCanChange())
            _collideeChangesColor=changes;
        if (getObjectCanSync())
            _setCollideeChangesColor_send(changes);
    }
    return(diff&&getObjectCanChange());
}

bool _CCollisionObject_::getCollideeChangesColor() const
{
    return(_collideeChangesColor);
}

bool _CCollisionObject_::setExhaustiveDetection(bool exhaustive)
{
    bool diff=(_detectAllCollisions!=exhaustive);
    if (diff)
    {
        if (getObjectCanChange())
            _detectAllCollisions=exhaustive;
        if (getObjectCanSync())
            _setExhaustiveDetection_send(exhaustive);
    }
    return(diff&&getObjectCanChange());
}

bool _CCollisionObject_::getExhaustiveDetection() const
{
    return(_detectAllCollisions);
}


bool _CCollisionObject_::setExplicitHandling(bool explicitHandl)
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

bool _CCollisionObject_::getExplicitHandling() const
{
    return(_explicitHandling);
}

bool _CCollisionObject_::setContourWidth(int w)
{
    bool diff=(_countourWidth!=w);
    if (diff)
    {
        if (getObjectCanChange())
            _countourWidth=w;
        if (getObjectCanSync())
            _setContourWidth_send(w);
    }
    return(diff&&getObjectCanChange());
}

bool _CCollisionObject_::setIntersections(const std::vector<float>* intersections)
{
    bool diff=false;
    if (intersections==nullptr)
    {
        diff=(_intersections.size()>0);
        if (diff)
        {
            if (getObjectCanChange())
                _intersections.clear();
            if (getObjectCanSync())
                _setIntersections_send(nullptr);
        }
    }
    else
    {
        diff=(intersections->size()!=_intersections.size());
        if (!diff)
        {
            for (size_t i=0;i<intersections->size();i++)
            {
                if (intersections->at(i)!=_intersections[i])
                {
                    diff=true;
                    break;
                }
            }
        }
        if (diff)
        {
            if (getObjectCanChange())
                _intersections.assign(intersections->begin(),intersections->end());
            if (getObjectCanSync())
                _setIntersections_send(intersections);
        }
    }
    return(diff&&getObjectCanChange());
}

int _CCollisionObject_::getContourWidth() const
{
    return(_countourWidth);
}

const std::vector<float>* _CCollisionObject_::getIntersections() const
{
    return(&_intersections);
}

void _CCollisionObject_::_setObjectName_send(const char* newName) const
{
    sendString(newName,sim_syncobj_collisionobject_setobjectname);
}

void _CCollisionObject_::_setColliderChangesColor_send(bool changes) const
{
    sendBool(changes,sim_syncobj_collisionobject_setcolliderchangescolor);
}

void _CCollisionObject_::_setCollideeChangesColor_send(bool changes) const
{
    sendBool(changes,sim_syncobj_collisionobject_setcollideechangescolor);
}

void _CCollisionObject_::_setExhaustiveDetection_send(bool exhaustive) const
{
    sendBool(exhaustive,sim_syncobj_collisionobject_setexhaustivedetection);
}

void _CCollisionObject_::_setExplicitHandling_send(bool explicitHandl) const
{
    sendBool(explicitHandl,sim_syncobj_collisionobject_setexplicithandling);
}

void _CCollisionObject_::_setContourWidth_send(int w) const
{
    sendInt32(w,sim_syncobj_collisionobject_setcontourwidth);
}

void _CCollisionObject_::_setIntersections_send(const std::vector<float>* intersections) const
{
    if ( (intersections==nullptr)||(intersections->size()==0) )
        sendFloatArray(nullptr,0,sim_syncobj_collisionobject_setintersections);
    else
        sendFloatArray(&intersections->at(0),intersections->size(),sim_syncobj_collisionobject_setintersections);
}

CColorObject* _CCollisionObject_::getContourColor()
{
    return(&_contourColor);
}
