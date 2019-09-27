
#include "vrepMainHeader.h"
#include "ghostObjectContainer.h"
#include "app.h"
#include "sceneObjectOperations.h"

CGhostObjectContainer::CGhostObjectContainer()
{
}

CGhostObjectContainer::~CGhostObjectContainer()
{
    removeGhost(-1,-1);
}

void CGhostObjectContainer::simulationAboutToStart()
{

}

void CGhostObjectContainer::simulationEnded()
{

}

void CGhostObjectContainer::emptySceneProcedure()
{ // don't do anything here! (plugin or add-on might be using that functionality too)

}

int CGhostObjectContainer::addGhost(int theGroupId,int theObjectHandle,int theOptions,float theStartTime,float theEndTime,const float theColor[12])
{
    // options: bit0 set (1): model instead of object
    // options: bit1 set (2): real-time playback
    // options: bit2 set (4): original colors
    // options: bit3 set (8): force invisible objects to appear too
    // options: bit4 set (16): ghost is invisible (maybe temporarily)
    // options: bit5 set (32): backface culling (when using custom colors only)

    int retVal=-1;
    std::vector<bool> freeSpot(_allObjects.size()+1,true);
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if ((_allObjects[i]->groupId==theGroupId)&&(_allObjects[i]->ghostId<int(freeSpot.size())))
            freeSpot[_allObjects[i]->ghostId]=false;
    }
    int nextGhostId=0;
    for (size_t i=0;i<freeSpot.size();i++)
    {
        if (freeSpot[i])
        {
            nextGhostId=int(i);
            break;
        }
    }

    std::vector<int> rootSel;
    rootSel.push_back(theObjectHandle);
    if ((theOptions&1)!=0)
        CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);

    std::vector<CShape*> objsToAdd;
    for (size_t i=0;i<rootSel.size();i++)
    {
        CShape* obj=App::ct->objCont->getShape(rootSel[i]);
        if (obj!=nullptr)
            objsToAdd.push_back(obj);
    }

    for (size_t i=0;i<objsToAdd.size();i++)
    {
        CShape* obj=objsToAdd[i];
        if (obj->getShouldObjectBeDisplayed(-1,0)||(theOptions&8))
        { // only visible objects... unless we force it with bit3 (8)
            CGhostObject* ghost=new CGhostObject(theGroupId,obj->getObjectHandle(),obj->getCumulativeTransformationPart1(),theOptions,theStartTime,theEndTime,theColor);
            ghost->ghostId=nextGhostId;
            retVal=nextGhostId;
            _allObjects.push_back(ghost);
        }
    }
    return(retVal);
}

int CGhostObjectContainer::modifyGhost(int groupId,int ghostId,int operation,float floatValue,int theOptions,int theOptionsMask,const float* colorOrTransformation)
{
    // operation:
    // 0: returns the number of the specified ghost occurences (if ghost(s) exist(s)), otherwise 0
    // 1: removes the specified ghosts. RetVal: number of erased ghosts
    // 2: sets the start time of the specified ghosts
    // 3: sets the end time of the specified ghosts
    // 4: shifts the start time of the specified ghosts
    // 5: shifts the end time of the specified ghosts
    // 6: shifts the times of the specified ghosts
    // 7: scales the start time of the specified ghosts
    // 8: scales the end times of the specified ghosts
    // 9: scales the times of the specified ghosts
    // 10: modifies the attributes of the specified ghosts. Check Lua function: it uses this value
    // 11: pre-multiply transformations of the specified ghosts. Check Lua function: it uses this value
    // 12: post-multiply transformations of the specified ghosts. Check Lua function: it uses this value
    // 13: change color of the specified ghosts. Check Lua function: it uses this value
    // 14: sets the transparency factor for the specified ghosts

    if (operation==1)
    {
        return(removeGhost(groupId,ghostId));
    }
    if ((operation>=11)&&(operation<=13)&&(colorOrTransformation==nullptr))
        return(-1);
    C7Vector transf;
    if ((operation>=11)&&(operation<=12))
    {
        transf.X.set(colorOrTransformation);
        // the quaternions are differently ordered at the interfaces!
        transf.Q(0)=colorOrTransformation[6];
        transf.Q(1)=colorOrTransformation[3];
        transf.Q(2)=colorOrTransformation[4];
        transf.Q(3)=colorOrTransformation[5];
    }
    int retVal=0;
    if ((operation==0)||((operation>=2)&&(operation<=14)))
    {
        for (size_t i=0;i<_allObjects.size();i++)
        {
            if ( (_allObjects[i]->groupId==groupId)&&((_allObjects[i]->ghostId==ghostId)||(ghostId==-1)) )
            {
                retVal++;
                if (operation==2)
                    _allObjects[i]->startTime=floatValue;
                if (operation==3)
                    _allObjects[i]->endTime=floatValue;
                if (operation==4)
                    _allObjects[i]->startTime+=floatValue;
                if (operation==5)
                    _allObjects[i]->endTime+=floatValue;
                if (operation==6)
                {
                    _allObjects[i]->startTime+=floatValue;
                    _allObjects[i]->endTime+=floatValue;
                }
                if (operation==7)
                    _allObjects[i]->startTime*=floatValue;
                if (operation==8)
                    _allObjects[i]->endTime*=floatValue;
                if (operation==9)
                {
                    _allObjects[i]->startTime*=floatValue;
                    _allObjects[i]->endTime*=floatValue;
                }
                if (operation==10)
                    _allObjects[i]->modifyAttributes(theOptions,theOptionsMask);
                if (operation==11)
                    _allObjects[i]->tr=transf*_allObjects[i]->tr;
                if (operation==12)
                    _allObjects[i]->tr=_allObjects[i]->tr*transf;
                if (operation==13)
                {
                    for (int j=0;j<12;j++)
                        _allObjects[i]->color[j]=colorOrTransformation[j];
                }
                if (operation==14)
                    _allObjects[i]->transparencyFactor=int(floatValue*255.1f);
            }
        }
    }
    return(retVal);
}

int CGhostObjectContainer::removeGhost(int groupId,int ghostId)
{
    int retVal=0;
    if (groupId==-1)
    { // remove all ghosts:
        retVal=(int)_allObjects.size();
        for (int i=0;i<int(_allObjects.size());i++)
            delete _allObjects[i];
        _allObjects.clear();
    }
    else
    { // remove ghosts that match the groupId and the ghostId
        size_t i=0;
        while (i<_allObjects.size())
        {
            if ( (_allObjects[i]->groupId==groupId)&&((_allObjects[i]->ghostId==ghostId)||(ghostId==-1)) )
            {
                delete _allObjects[i];
                _allObjects.erase(_allObjects.begin()+i);
                retVal++;
            }
            else
                i++;
        }
    }
    return(retVal);
}

void CGhostObjectContainer::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<_allObjects.size())
    {
        if (_allObjects[i]->objectHandle==objID)
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
        }
        else
            i++;
    }
}

void CGhostObjectContainer::performObjectLoadingMapping(std::vector<int>* map)
{
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->objectHandle=App::ct->objCont->getLoadingMapping(map,_allObjects[i]->objectHandle);
}

void CGhostObjectContainer::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {
            ar.storeDataName("V02");
            ar << int(_allObjects.size());
            for (size_t i=0;i<_allObjects.size();i++)
            {
                ar << _allObjects[i]->groupId;
                ar << _allObjects[i]->ghostId;
                ar << _allObjects[i]->objectHandle;
                ar << _allObjects[i]->options;
                ar << _allObjects[i]->startTime;
                ar << _allObjects[i]->endTime;
                ar << _allObjects[i]->transparencyFactor;
                for (int j=0;j<12;j++)
                    ar << _allObjects[i]->color[j];
                ar << _allObjects[i]->tr.X(0) << _allObjects[i]->tr.X(1) << _allObjects[i]->tr.X(2);
                ar << _allObjects[i]->tr.Q(0) << _allObjects[i]->tr.Q(1) << _allObjects[i]->tr.Q(2) << _allObjects[i]->tr.Q(3);
            }
            ar.flush();

            ar.storeDataName(SER_NEXT_STEP);
        }
        else
        {       // Loading
            removeGhost(-1,-1);
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_NEXT_STEP)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_NEXT_STEP)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("V01")==0)
                    { // for backward compatibility
                        noHit=false;
                        ar >> byteQuantity;
                        int ghostCnt;
                        ar >> ghostCnt;
                        for (int i=0;i<ghostCnt;i++)
                        {
                            CGhostObject* go=new CGhostObject();
                            ar >> go->groupId;
                            ar >> go->ghostId;
                            ar >> go->objectHandle;
                            ar >> go->options;
                            ar >> go->startTime;
                            ar >> go->endTime;
                            for (int j=0;j<12;j++)
                                ar >> go->color[j];
                            ar >> go->tr.X(0) >> go->tr.X(1) >> go->tr.X(2);
                            ar >> go->tr.Q(0) >> go->tr.Q(1) >> go->tr.Q(2) >> go->tr.Q(3);
                            _allObjects.push_back(go);
                        }
                    }
                    if (theName.compare("V02")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int ghostCnt;
                        ar >> ghostCnt;
                        for (int i=0;i<ghostCnt;i++)
                        {
                            CGhostObject* go=new CGhostObject();
                            ar >> go->groupId;
                            ar >> go->ghostId;
                            ar >> go->objectHandle;
                            ar >> go->options;
                            ar >> go->startTime;
                            ar >> go->endTime;
                            ar >> go->transparencyFactor;
                            for (int j=0;j<12;j++)
                                ar >> go->color[j];
                            ar >> go->tr.X(0) >> go->tr.X(1) >> go->tr.X(2);
                            ar >> go->tr.Q(0) >> go->tr.Q(1) >> go->tr.Q(2) >> go->tr.Q(3);
                            _allObjects.push_back(go);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CGhostObjectContainer::renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib)
{
    if ((displayAttrib&sim_displayattribute_noghosts)==0)
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            float simulationTime=float(App::ct->simulation->getSimulationTime_ns())/1000000.0f;
            float realTime=float(App::ct->simulation->getSimulationTime_real_ns())/1000000.0f;
            for (size_t i=0;i<_allObjects.size();i++)
            {
                if (_allObjects[i]->transparencyFactor==0)
                    _allObjects[i]->render(displayAttrib,simulationTime,realTime);
            }
        }
    }
}

void CGhostObjectContainer::renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib)
{
    if ((displayAttrib&sim_displayattribute_noghosts)==0)
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            float simulationTime=float(App::ct->simulation->getSimulationTime_ns())/1000000.0f;
            float realTime=float(App::ct->simulation->getSimulationTime_real_ns())/1000000.0f;
            for (size_t i=0;i<_allObjects.size();i++)
            {
                if (_allObjects[i]->transparencyFactor!=0)
                    _allObjects[i]->render(displayAttrib,simulationTime,realTime);
            }
        }
    }
}

void CGhostObjectContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib)
{
//  if ((displayAttrib&sim_displayattribute_noghosts)==0)
//  {
//  }
}
