
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "registeredCollections.h"
#include "tt.h"
#include "app.h"

CRegisteredCollections::CRegisteredCollections()
{
}

CRegisteredCollections::~CRegisteredCollections()
{
    unmarkAll();
    removeAllCollections();
}

void CRegisteredCollections::simulationAboutToStart()
{
    for (int i=0;i<int(allCollections.size());i++)
        allCollections[i]->simulationAboutToStart();
}

void CRegisteredCollections::simulationEnded()
{
    for (int i=0;i<int(allCollections.size());i++)
        allCollections[i]->simulationEnded();
}

void CRegisteredCollections::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CRegisteredCollections::newScene()
{
    unmarkAll();
    removeAllCollections();
}

void CRegisteredCollections::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(allCollections.size()))
    {
        // position could be changed after a call to announceObject...
        if (allCollections[i]->announceObjectWillBeErased(objID,false))
        {
            removeCollection(allCollections[i]->getCollectionID()); // This will call announceCollectionWillBeErased!!
            i=0; // We never know. The ordering may have changed!
        }
        else
            i++;
    }
}

void CRegisteredCollections::actualizeAllGroups()
{
    int i=0;
    while (i<int(allCollections.size()))
    {
        if (!allCollections[i]->actualizeCollection())
        {
            removeCollection(allCollections[i]->getCollectionID());
            i=0; // We start at 0 again. The ordering may have changed in removeGroup
        }
        else
            i++;
    }
}

void CRegisteredCollections::unmarkAll()
{   // Will deselect all groups. Use only from CObjCont
    for (int i=0;i<int(allCollections.size());i++)
        allCollections[i]->unmarkAll();
    selectedCollections.clear();
}

void CRegisteredCollections::mark(int objID)
{   // Will mark all objects objID in groups. If a group is completely selected,
    // it will be inserted in selectedCollections. Use only from CObjCont
    for (int i=0;i<int(allCollections.size());i++)
    {
        if (!allCollections[i]->areAllMarked())
        {
            allCollections[i]->mark(objID);
            if (allCollections[i]->areAllMarked())
            {   
                // We first remove the group from selection (just in case)
                for (int j=0;j<int(selectedCollections.size());j++)
                {
                    if (selectedCollections[j]==allCollections[i]->getCollectionID())
                    {
                        selectedCollections.erase(selectedCollections.begin()+j);
                        break;
                    }
                }
                // We add the group to the selection:
                selectedCollections.push_back(allCollections[i]->getCollectionID());
            }
        }
    }
}

bool CRegisteredCollections::getShapesAndVolumesFromCollection(int groupID,std::vector<C3DObject*>* objInGroup,int propMask,bool pathPlanningRoutineCalling)
{ // If propMask==-1, then object main properties are not checked an all objects are taken!
    pathPlanningRoutineCalling=false; // OLD_PATH_PLANNING_REMOVE
    CRegCollection* theGroup=getCollection(groupID);
    if (theGroup==nullptr)
        return(false);
    objInGroup->reserve(theGroup->collectionObjects.size());
    objInGroup->clear();
    bool overridePropertyFlag=theGroup->getOverridesObjectMainProperties();
    for (int i=0;i<int(theGroup->collectionObjects.size());i++)
    {
        C3DObject* anObject=App::ct->objCont->getObjectFromHandle(theGroup->collectionObjects[i]);
        if (anObject!=nullptr)
        {
            if ( (anObject->getCumulativeObjectSpecialProperty()&propMask)||(propMask==-1)||overridePropertyFlag )
            {
                if ( ((!pathPlanningRoutineCalling)||((anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_pathplanning_ignored)==0))||overridePropertyFlag ) // condition added on 2010/08/25
                {
                    if (anObject->getObjectType()==sim_object_shape_type)
                        objInGroup->push_back(anObject);
                }
            }
        }
    }
    return(true);
}

bool CRegisteredCollections::getShapesAndDummiesFromCollection(int groupID,std::vector<C3DObject*>* objInGroup,int propMask,bool pathPlanningRoutineCalling)
{ // If propMask==-1, then object main properties are not checked an all objects are taken!
    pathPlanningRoutineCalling=false; // OLD_PATH_PLANNING_REMOVE
    CRegCollection* theGroup=getCollection(groupID);
    if (theGroup==nullptr)
        return(false);
    objInGroup->reserve(theGroup->collectionObjects.size());
    objInGroup->clear();
    bool overridePropertyFlag=theGroup->getOverridesObjectMainProperties();
    for (int i=0;i<int(theGroup->collectionObjects.size());i++)
    {
        C3DObject* anObject=App::ct->objCont->getObjectFromHandle(theGroup->collectionObjects[i]);
        if (anObject!=nullptr)
        {
            if ( (anObject->getCumulativeObjectSpecialProperty()&propMask)||(propMask==-1)||overridePropertyFlag )
            {
                if ( ((!pathPlanningRoutineCalling)||((anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_pathplanning_ignored)==0))||overridePropertyFlag ) // condition added on 2010/08/25
                {
                    if (anObject->getObjectType()==sim_object_shape_type)
                        objInGroup->push_back(anObject);
                    if (anObject->getObjectType()==sim_object_dummy_type)
                        objInGroup->push_back(anObject);
                }
            }
        }
    }
    return(true);
}

void CRegisteredCollections::getCollidableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects)
{
    objects.clear();
    CRegCollection* theGroup=getCollection(collectionID);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties();
        for (int i=0;i<int(theGroup->collectionObjects.size());i++)
        {
            C3DObject* anObject=App::ct->objCont->getObjectFromHandle(theGroup->collectionObjects[i]);
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyCollidable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CRegisteredCollections::getMeasurableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects)
{
    objects.clear();
    CRegCollection* theGroup=getCollection(collectionID);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties();
        for (int i=0;i<int(theGroup->collectionObjects.size());i++)
        {
            C3DObject* anObject=App::ct->objCont->getObjectFromHandle(theGroup->collectionObjects[i]);
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyMeasurable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CRegisteredCollections::getDetectableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects,int detectableMask)
{
    objects.clear();
    CRegCollection* theGroup=getCollection(collectionID);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties()||(detectableMask==-1);
        for (int i=0;i<int(theGroup->collectionObjects.size());i++)
        {
            C3DObject* anObject=App::ct->objCont->getObjectFromHandle(theGroup->collectionObjects[i]);
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyDetectable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&detectableMask)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CRegisteredCollections::unmark(int objID)
{   // Will unmark all objects objID in groups. If a group is deselected,
    // it will be removed from selectedCollections. Use only from CObjCont
    for (int i=0;i<int(allCollections.size());i++)
    {
        allCollections[i]->unmark(objID);
        if (!allCollections[i]->areAllMarked())
        {
            if (isCollectionInSelection(allCollections[i]->getCollectionID()))
            {
                for (int j=0;j<int(selectedCollections.size());j++)
                {
                    if (selectedCollections[j]==allCollections[i]->getCollectionID())
                    {
                        selectedCollections.erase(selectedCollections.begin()+j);
                        break;
                    }
                }
            }
        }
    }
}

void CRegisteredCollections::repairMarking()
{
    unmarkAll();
    if (App::ct->objCont==nullptr)
        return;
    for (int i=0;i<App::ct->objCont->getSelSize();i++)
        mark(App::ct->objCont->getSelID(i));
}

void CRegisteredCollections::removeAllCollections()
{
    deselectAllCollections();
    while (allCollections.size()!=0)
        removeCollection(allCollections[0]->getCollectionID());
    App::setFullDialogRefreshFlag();
    repairMarking();
}

void CRegisteredCollections::setUpDefaultValues()
{
    removeAllCollections();
}

CRegCollection* CRegisteredCollections::getCollection(int groupID)
{
    for (int i=0;i<int(allCollections.size());i++)
    {
        if (allCollections[i]->getCollectionID()==groupID)
            return(allCollections[i]);
    }
    return(nullptr);
}

CRegCollection* CRegisteredCollections::getCollection(std::string groupName)
{
    for (int i=0;i<int(allCollections.size());i++)
    {
        if (allCollections[i]->getCollectionName().compare(groupName)==0)
            return(allCollections[i]);
    }
    return(nullptr);
}

bool CRegisteredCollections::removeCollection(int groupID)
{   // Return value true: the group was removed
    removeCollectionFromSelection(groupID);
    App::ct->objCont->announceCollectionWillBeErased(groupID);
    for (int i=0;i<int(allCollections.size());i++)
    {
        if (allCollections[i]->getCollectionID()==groupID)
        {
            delete allCollections[i];
            allCollections.erase(allCollections.begin()+i);
            App::setFullDialogRefreshFlag();
            repairMarking();
            return(false);
        }
    }
    return(false);
}

void CRegisteredCollections::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(allCollections.size());i++)
    {
        int s=tt::getNameSuffixNumber(allCollections[i]->getCollectionName().c_str(),true);
        if (i==0)
        {
            minSuffix=s;
            maxSuffix=s;
        }
        else
        {
            if (s<minSuffix)
                minSuffix=s;
            if (s>maxSuffix)
                maxSuffix=s;
        }
    }
}

bool CRegisteredCollections::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allCollections.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allCollections[i]->getCollectionName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allCollections[i]->getCollectionName().c_str(),true));
            for (int j=0;j<int(allCollections.size());j++)
            {
                int s2=tt::getNameSuffixNumber(allCollections[j]->getCollectionName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(allCollections[j]->getCollectionName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CRegisteredCollections::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allCollections.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allCollections[i]->getCollectionName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allCollections[i]->getCollectionName().c_str(),true));
            allCollections[i]->setCollectionName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

void CRegisteredCollections::addCollection(CRegCollection* theGroup,bool objectIsACopy)
{
    addCollectionWithSuffixOffset(theGroup,objectIsACopy,1);
}

void CRegisteredCollections::addCollectionWithSuffixOffset(CRegCollection* theGroup,bool objectIsACopy,int suffixOffset)
{
    // Does that name already exist?
    std::string theName=theGroup->getCollectionName();
    if (objectIsACopy)
        theName=tt::generateNewName_dash(theName,suffixOffset);
    else
    {
        while (getCollection(theName)!=nullptr)
            theName=tt::generateNewName_noDash(theName);
    }
    theGroup->setCollectionName(theName);
    // Find a free ID:
    int id=SIM_IDSTART_COLLECTION;
    while (getCollection(id)!=nullptr)
        id++;
    theGroup->setCollectionID(id);
    // Add the group:
    allCollections.push_back(theGroup);
    App::setFullDialogRefreshFlag();
    repairMarking();
}

bool CRegisteredCollections::isObjectInMarkedCollection(int objID)
{
    for (int i=0;i<int(allCollections.size());i++)
    {
        if (allCollections[i]->areAllMarked())
        {
            if (allCollections[i]->isObjectInCollection(objID))
                return(true);
        }
    }
    return(false);
}

void CRegisteredCollections::addCollectionToSelection(int groupID)
{
    // Don't call from App::ct->objCont->addObject(), App::ct->objCont->... !!
    CRegCollection* it=getCollection(groupID);
    if (it==nullptr)
        return;
    for (size_t i=0;i<selectedCollections.size();i++)
    {
        if (selectedCollections[i]==groupID)
            return; // already selected
    }
    for (size_t i=0;i<it->collectionObjects.size();i++)
        App::ct->objCont->addObjectToSelection(it->collectionObjects[i]);
}
bool CRegisteredCollections::isCollectionInSelection(int groupID)
{
    for (int i=0;i<int(selectedCollections.size());i++)
    {
        if (selectedCollections[i]==groupID)
            return(true);
    }
    return(false);
}
void CRegisteredCollections::removeCollectionFromSelection(int groupID)
{
    // Don't call from App::ct->objCont->addObject(), App::ct->objCont->... !!
    CRegCollection* it=getCollection(groupID);
    if (it==nullptr)
        return;
    for (int i=0;i<int(it->collectionObjects.size());i++)
        App::ct->objCont->removeObjectFromSelection(it->collectionObjects[i]);
}
void CRegisteredCollections::deselectAllCollections()
{
    // Don't call from App::ct->objCont->addObject(), App::ct->objCont->... !!
    for (int i=0;i<int(allCollections.size());i++)
        removeCollectionFromSelection(allCollections[i]->getCollectionID());
}

void CRegisteredCollections::performObjectLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(allCollections.size());i++)
        allCollections[i]->performObjectLoadingMapping(map);
}
