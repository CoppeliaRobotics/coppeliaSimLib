#pragma once

#include <map>
#include "sceneObject.h"
#include "syncObject.h"

enum {
    sim_syncobj_sceneobjectcont_selection=0,
};

class _CSceneObjectContainer_ : public CSyncObject
{
public:
    _CSceneObjectContainer_();
    virtual ~_CSceneObjectContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    bool doesObjectExist(const CSceneObject* obj) const;
    size_t getObjectCount() const;
    CSceneObject* getObjectFromIndex(size_t index) const;
    CSceneObject* getObjectFromHandle(int objectHandle) const;

    size_t getSelectionCount() const;
    int getObjectHandleFromSelectionIndex(size_t index) const;
    const std::vector<int>* getSelectedObjectHandlesPtr() const;

    virtual bool setSelectedObjectHandles(const std::vector<int>* v);

protected:
    virtual void _addObject(CSceneObject* object);
    virtual bool _removeObject(int objectHandle);

    virtual void _setSelectedObjectHandles_send(const std::vector<int>* v) const;

private:
    std::vector<CSceneObject*> _objects;
    std::map<int,CSceneObject*> _objectHandleMap;

    std::vector<int> _selectedObjectHandles;
};


