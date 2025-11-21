#include <simInternal.h>
#include <drawingContainer.h>
#include <viewableBase.h>
#include <app.h>
#include <utils.h>

CDrawingContainer::CDrawingContainer()
{
}

CDrawingContainer::~CDrawingContainer()
{
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        delete _allObjects[i];
        _allObjects.erase(_allObjects.begin() + i);
    }
}

void CDrawingContainer::simulationEnded()
{
}

CDrawingObject* CDrawingContainer::getObject(int objectId) const
{
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        if (_allObjects[i]->getObjectId() == objectId)
            return _allObjects[i];
    }
    return nullptr;
}

int CDrawingContainer::addObject(CDrawingObject* it)
{
    static int newId = SIM_IDSTART_DRAWINGOBJ;
    int id = newId++;
    it->setObjectId(id);
    _allObjects.push_back(it);
    it->pushAddEvent();
    _publishAllDrawingObjectHandlesEvent();
    return id;
}

void CDrawingContainer::_publishAllDrawingObjectHandlesEvent() const
{
    if (App::worldContainer->getEventsEnabled())
    {
        std::vector<int> handles;
        for (size_t i = 0; i < _allObjects.size(); i++)
        {
            CDrawingObject* dr = _allObjects[i];
            handles.push_back(dr->getObjectId());
        }
        const char* cmd = propDrawCont_drawingObjects.name;
        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
        ev->appendKeyIntArray(cmd, handles.data(), handles.size());
        App::worldContainer->pushEvent();
    }
}

void CDrawingContainer::removeObject(int objectId)
{
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        if (_allObjects[i]->getObjectId() == objectId)
        {
            if (App::worldContainer->getEventsEnabled())
            {
                App::worldContainer->createEvent(EVENTTYPE_DRAWINGOBJECTREMOVED,  _allObjects[i]->getObjectId(), _allObjects[i]->getObjectId(), nullptr, false);
                App::worldContainer->pushEvent();
            }

            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin() + i);

            _publishAllDrawingObjectHandlesEvent();
            break;
        }
    }
}

void CDrawingContainer::eraseAllObjects()
{
    while (_allObjects.size() > 0)
        removeObject(_allObjects[0]->getObjectId());
}

void CDrawingContainer::announceObjectWillBeErased(const CSceneObject* object)
{ // Never called from copy buffer!
    size_t i = 0;
    while (i < _allObjects.size())
    {
        if (_allObjects[i]->announceObjectWillBeErased(object))
            removeObject(_allObjects[i]->getObjectId());
        else
            i++;
    }
}

void CDrawingContainer::announceScriptStateWillBeErased(int scriptHandle, bool simulationScript,
                                                        bool sceneSwitchPersistentScript)
{
    size_t i = 0;
    while (i < _allObjects.size())
    {
        if (_allObjects[i]->announceScriptStateWillBeErased(scriptHandle, simulationScript,
                                                            sceneSwitchPersistentScript))
            removeObject(_allObjects[i]->getObjectId());
        else
            i++;
    }
}

void CDrawingContainer::pushGenesisEvents()
{
    std::vector<int> addedObjects;
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        CDrawingObject* dr = _allObjects[i];
        dr->pushAddEvent();
        // We need to "fake" adding that drawing object:
        addedObjects.push_back(dr->getObjectId());
        const char* cmd = propDrawCont_drawingObjects.name;
        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
        ev->appendKeyIntArray(cmd, addedObjects.data(), addedObjects.size());
        App::worldContainer->pushEvent();
    }
}

void CDrawingContainer::pushAppendNewPointEvents()
{
    for (size_t i = 0; i < _allObjects.size(); i++)
        _allObjects[i]->pushAppendNewPointEvent();
}

int CDrawingContainer::getStringProperty(long long int target, const char* pName, std::string& pState) const
{
    int retVal = -1;
    if (target == -1)
    {
    }
    else
    {
        CDrawingObject* it = getObject(int(target));
        if (it != nullptr)
            return it->getStringProperty(pName, pState);
        retVal = -2; // drawing object does not exist
    }
    return retVal;
}

int CDrawingContainer::getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState) const
{
    int retVal = -1;
    pState.clear();
    if (target == -1)
    {
        if (strcmp(pName, propDrawCont_drawingObjects.name) == 0)
        {
            for (size_t i = 0; i < _allObjects.size(); i++)
                pState.push_back(_allObjects[i]->getObjectId());
            retVal = 1;
        }
    }
    return retVal;
}

int CDrawingContainer::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = -1;
    if (target == -1)
    {
        for (size_t i = 0; i < allProps_drawCont.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_drawCont[i].name, pName.c_str()))
            {
                if ((allProps_drawCont[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_drawCont[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        CDrawingObject* it = getObject(int(target));
        if (it != nullptr)
        {
            appartenance = "drawingObject";
            return it->getPropertyName(index, pName, appartenance, excludeFlags);
        }
        retVal = -2; // object does not exist
    }
    return retVal;
}

int CDrawingContainer::getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = -1;
    if (target == -1)
    {
        for (size_t i = 0; i < allProps_drawCont.size(); i++)
        {
            if (strcmp(allProps_drawCont[i].name, pName) == 0)
            {
                retVal = allProps_drawCont[i].type;
                info = allProps_drawCont[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_drawCont[i].infoTxt, "") != 0))
                    infoTxt = allProps_drawCont[i].infoTxt;
                else
                    infoTxt = allProps_drawCont[i].shortInfoTxt;
                break;
            }
        }
    }
    else
    {
        CDrawingObject* it = getObject(int(target));
        if (it != nullptr)
            return it->getPropertyInfo(pName, info, infoTxt);
        retVal = -2; // object does not exist
    }
    return retVal;
}

#ifdef SIM_WITH_GUI
void CDrawingContainer::renderYour3DStuff_nonTransparent(CViewableBase* renderingObject, int displayAttrib)
{
    drawAll(false, false, displayAttrib, renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_transparent(CViewableBase* renderingObject, int displayAttrib)
{
    drawAll(false, true, displayAttrib, renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject, int displayAttrib)
{
    drawAll(true, true, displayAttrib, renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::drawAll(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix& cameraCTM)
{
    for (size_t i = 0; i < _allObjects.size(); i++)
        _allObjects[i]->draw(overlay, transparentObject, displayAttrib, cameraCTM);
}

void CDrawingContainer::drawObjectsParentedWith(bool overlay, bool transparentObject, int parentObjectId,
                                                int displayAttrib, const C4X4Matrix& cameraCTM)
{
    if ((displayAttrib & sim_displayattribute_nodrawingobjects) == 0)
    {
        for (size_t i = 0; i < _allObjects.size(); i++)
        {
            if ((_allObjects[i]->getSceneObjectId() == parentObjectId) &&
                ((_allObjects[i]->getObjectType() & sim_drawing_painttag) != 0))
                _allObjects[i]->draw(overlay, transparentObject, displayAttrib, cameraCTM);
        }
    }
}
#endif
