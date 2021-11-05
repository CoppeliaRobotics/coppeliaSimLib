#include "simInternal.h"
#include "pointCloudContainer_old.h"
#include "viewableBase.h"
#include "easyLock.h"
#include "app.h"

CPointCloudContainer_old::CPointCloudContainer_old()
{
}

CPointCloudContainer_old::~CPointCloudContainer_old()
{ // beware, the current world could be nullptr
    removeAllObjects(false);
}

void CPointCloudContainer_old::simulationEnded()
{
    removeAllObjects(true);
}

CPtCloud_old* CPointCloudContainer_old::getObject(int objectID)
{
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
            return(_allObjects[i]);
    }
    return(nullptr);
}

int CPointCloudContainer_old::addObject(CPtCloud_old* it)
{
    EASYLOCK(_objectMutex);
    int newID=0;
    newID++;
    while (getObject(newID)!=nullptr)
        newID++;
    it->setObjectID(newID);
    it->setObjectUniqueId();
    _allObjects.push_back(it);
    it->pushAddEvent();
    return(newID);
}

void CPointCloudContainer_old::removeAllObjects(bool onlyNonPersistentOnes)
{
    EASYLOCK(_objectMutex);
    size_t i=0;
    while (i<_allObjects.size())
    {
        if ( (!onlyNonPersistentOnes)||(!_allObjects[i]->isPersistent()) )
            removeObject(_allObjects[i]->getObjectID());
        else
            i++;
    }
}

bool CPointCloudContainer_old::removeObject(int objectID)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
        {
            if (App::worldContainer->getEnableEvents())
            {
                auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTREMOVED,_allObjects[i]->getObjectUniqueId(),nullptr,false);
                App::worldContainer->pushEvent(event);
            }

            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
            return(true);
        }
    }
    return(false);
}

void CPointCloudContainer_old::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<_allObjects.size())
    {
        if (_allObjects[i]->announceObjectWillBeErased(objID))
            removeObject(_allObjects[i]->getObjectID());
        else
            i++;
    }
}

void CPointCloudContainer_old::renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib)
{
    if ((displayAttrib&sim_displayattribute_nopointclouds)==0)
        drawAll(displayAttrib);
}

void CPointCloudContainer_old::renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib)
{
}

void CPointCloudContainer_old::renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib)
{
}

void CPointCloudContainer_old::drawAll(int displayAttrib)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->draw(displayAttrib);
}

void CPointCloudContainer_old::pushAllInitialEvents()
{
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->pushAddEvent();
}
