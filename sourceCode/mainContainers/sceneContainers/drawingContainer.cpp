
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "drawingContainer.h"
#include "viewableBase.h"
#include "easyLock.h"

CDrawingContainer::CDrawingContainer()
{
}

CDrawingContainer::~CDrawingContainer()
{
    removeAllObjects(false,true);
}

void CDrawingContainer::simulationAboutToStart()
{

}

void CDrawingContainer::simulationEnded()
{
    removeAllObjects(true,false);
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

void CDrawingContainer::emptySceneProcedure()
{ // don't do anything here! (plugin or add-on might be using that functionality too) 
}

CDrawingObject* CDrawingContainer::getObject(int objectID)
{
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
            return(_allObjects[i]);
    }
    return(nullptr);
}


int CDrawingContainer::addObject(CDrawingObject* it)
{
    EASYLOCK(_objectMutex);
    int newID=0;
    newID++;
    while (getObject(newID)!=nullptr)
        newID++;
    it->setObjectID(newID);
    _allObjects.push_back(it);
    return(newID);
}

void CDrawingContainer::removeAllObjects(bool onlyThoseCreatedFromScripts,bool alsoPersistentObjects)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if ( (!onlyThoseCreatedFromScripts)||_allObjects[i]->getCreatedFromScript() )
        {
            if (alsoPersistentObjects||(!_allObjects[i]->getPersistent()))
            {
                delete _allObjects[i];
                _allObjects.erase(_allObjects.begin()+i);
                i--; // reprocess this position
            }
        }
    }
}

void CDrawingContainer::removeObject(int objectID)
{
    EASYLOCK(_objectMutex);
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
            break;
        }
    }
}

bool CDrawingContainer::getExportableMeshAtIndex(int parentObjectID,int index,std::vector<float>& vertices,std::vector<int>& indices)
{
    vertices.clear();
    indices.clear();
    int cnt=0;
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if ((_allObjects[i]->getSceneObjectID()==parentObjectID)&&_allObjects[i]->canMeshBeExported())
        {
            cnt++;
            if (cnt==index+1)
            {
                _allObjects[i]->getExportableMesh(vertices,indices);
                return(true);
            }
        }
    }
    return(false);
}

void CDrawingContainer::adjustForFrameChange(int objectID,const C7Vector& preCorrection)
{
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if (_allObjects[i]->getSceneObjectID()==objectID)
            _allObjects[i]->adjustForFrameChange(preCorrection);
    }
}

void CDrawingContainer::adjustForScaling(int objectID,float xScale,float yScale,float zScale)
{
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if (_allObjects[i]->getSceneObjectID()==objectID)
            _allObjects[i]->adjustForScaling(xScale,yScale,zScale);
    }
}


void CDrawingContainer::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(_allObjects.size()))
    {
        if (_allObjects[i]->announceObjectWillBeErased(objID))
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
        }
        else
            i++;
    }
}

void CDrawingContainer::renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(false,false,displayAttrib,renderingObject->getCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(false,true,displayAttrib,renderingObject->getCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(true,true,displayAttrib,renderingObject->getCumulativeTransformation().getMatrix());
}

void CDrawingContainer::drawAll(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->draw(overlay,transparentObject,displayAttrib,cameraCTM);
}

void CDrawingContainer::drawObjectsParentedWith(bool overlay,bool transparentObject,int parentObjectID,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    if ((displayAttrib&sim_displayattribute_nodrawingobjects)==0)
    {
        EASYLOCK(_objectMutex);
        for (size_t i=0;i<_allObjects.size();i++)
        {
            if ( (_allObjects[i]->getSceneObjectID()==parentObjectID)&&((_allObjects[i]->getObjectType()&sim_drawing_painttag)!=0) )
                _allObjects[i]->draw(overlay,transparentObject,displayAttrib,cameraCTM);
        }
    }
}
