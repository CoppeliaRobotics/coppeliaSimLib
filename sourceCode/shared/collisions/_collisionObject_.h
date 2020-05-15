#pragma once

#include "colorObject.h"
#include "syncObject.h"
#include <string>

enum {
    sim_syncobj_collisionobject_create=0,
    sim_syncobj_collisionobject_delete,
    sim_syncobj_collisionobject_setobjectname,
    sim_syncobj_collisionobject_setcolliderchangescolor,
    sim_syncobj_collisionobject_setcollideechangescolor,
    sim_syncobj_collisionobject_setexhaustivedetection,
    sim_syncobj_collisionobject_setexplicithandling,
    sim_syncobj_collisionobject_setcontourwidth,
    sim_syncobj_collisionobject_setintersections,
};

class _CCollisionObject_ : public CSyncObject
{
public:
    _CCollisionObject_();
    virtual ~_CCollisionObject_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getObjectHandle() const;
    int getEntity1Handle() const;
    int getEntity2Handle() const;
    std::string getObjectName() const;
    bool getColliderChangesColor() const;
    bool getCollideeChangesColor() const;
    bool getExhaustiveDetection() const;
    bool getExplicitHandling() const;
    int getContourWidth() const;
    const std::vector<float>* getIntersections() const;

    CColorObject* getContourColor();

    virtual bool setObjectHandle(int newHandle);

    virtual bool setObjectName(const char* newName,bool check);
    virtual bool setColliderChangesColor(bool changes);
    virtual bool setCollideeChangesColor(bool changes);
    virtual bool setExhaustiveDetection(bool exhaustive);
    virtual bool setExplicitHandling(bool explicitHandl);
    virtual bool setContourWidth(int w);
    virtual bool setIntersections(const std::vector<float>* intersections=nullptr);

protected:
    virtual void _setObjectName_send(const char* newName) const;
    virtual void _setColliderChangesColor_send(bool changes) const;
    virtual void _setCollideeChangesColor_send(bool changes) const;
    virtual void _setExhaustiveDetection_send(bool exhaustive) const;
    virtual void _setExplicitHandling_send(bool explicitHandl) const;
    virtual void _setContourWidth_send(int w) const;
    virtual void _setIntersections_send(const std::vector<float>* intersections=nullptr) const;

    int _objectHandle;
    int _entity1Handle;
    int _entity2Handle;
    std::string _objectName;
    int _countourWidth;
    bool _colliderChangesColor;
    bool _collideeChangesColor;
    bool _detectAllCollisions;
    bool _explicitHandling;

    CColorObject _contourColor;
    std::vector<float> _intersections;
};
