#pragma once

#include <string>
#include "MyMath.h"
#include "colorObject.h"
#include "sceneObject.h"

enum {
    sim_syncobj_dummy_create=0,
    sim_syncobj_dummy_delete,
    sim_syncobj_dummy_size,
    sim_syncobj_dummy_linkeddummy,
    sim_syncobj_dummy_linktype,
    sim_syncobj_dummy_assignedparentpath,
    sim_syncobj_dummy_assignedparentpathorientation,
};

class _CDummy_ : public CSceneObject
{
public:
    _CDummy_();
    virtual ~_CDummy_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    float getDummySize() const;
    bool getAssignedToParentPath() const;
    bool getAssignedToParentPathOrientation() const;
    int getLinkedDummyHandle() const;
    int getLinkType() const;

    CColorObject* getDummyColor();

    void setDummySize(float s);
    virtual bool setLinkedDummyHandle(int h,bool check);
    virtual bool setLinkType(int t,bool check);
    virtual bool setAssignedToParentPath(bool a);
    virtual bool setAssignedToParentPathOrientation(bool a);

protected:
    virtual void _setLinkedDummyHandle_send(int h) const;
    virtual void _setLinkType_send(int t) const;
    virtual void _setAssignedToParentPath_send(bool a) const;
    virtual void _setAssignedToParentPathOrientation_send(bool a) const;

    float _dummySize;
    int _linkedDummyHandle;
    int _linkType;
    bool _assignedToParentPath;
    bool _assignedToParentPathOrientation;

    CColorObject _dummyColor;
};
