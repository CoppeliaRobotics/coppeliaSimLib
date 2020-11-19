#include "collectionContainer.h"
#include "app.h"
#include "tt.h"

CCollectionContainer::CCollectionContainer()
{
}

CCollectionContainer::~CCollectionContainer()
{ // beware, the current world could be nullptr
}

void CCollectionContainer::simulationAboutToStart()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationAboutToStart();
}

void CCollectionContainer::simulationEnded()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationEnded();
}

void CCollectionContainer::newScene()
{
    removeAllCollections();
}

void CCollectionContainer::announceObjectWillBeErased(int objectHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        CCollection* coll=getObjectFromIndex(i);
        if (coll->announceObjectWillBeErased(objectHandle,false))
        {
            if (coll->getCreatorHandle()==-2) // Only old-type collections will be removed (those created via the GUI)
                removeCollection(coll->getCollectionHandle()); // This will call announceCollectionWillBeErased!!
            else
                i++;
        }
        else
            i++;
    }
}

void CCollectionContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    size_t i=0;
    while (i<getObjectCount())
    {
        CCollection* coll=getObjectFromIndex(i);
        if (coll->announceScriptStateWillBeErased(scriptHandle))
            removeCollection(coll->getCollectionHandle()); // This will call announceCollectionWillBeErased!!
        else
            i++;
    }
}

void CCollectionContainer::actualizeAllCollections()
{
    size_t i=0;
    while (i<getObjectCount())
    {
        CCollection* coll=getObjectFromIndex(i);
        if (!coll->actualizeCollection())
        {
            if (coll->getCreatorHandle()==-2) // Only old-type collections will be removed (those created via the GUI)
                removeCollection(coll->getCollectionHandle());
            else
                i++;
        }
        else
            i++;
    }
}

void CCollectionContainer::getCollidableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects) const
{
    objects.clear();
    CCollection* theGroup=getObjectFromHandle(collectionHandle);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties();
        for (size_t i=0;i<theGroup->getSceneObjectCountInCollection();i++)
        {
            CSceneObject* anObject=App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyCollidable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::getMeasurableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects) const
{
    objects.clear();
    CCollection* theGroup=getObjectFromHandle(collectionHandle);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties();
        for (size_t i=0;i<theGroup->getSceneObjectCountInCollection();i++)
        {
            CSceneObject* anObject=App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyMeasurable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::getDetectableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects,int detectableMask) const
{
    objects.clear();
    CCollection* theGroup=getObjectFromHandle(collectionHandle);
    if (theGroup!=nullptr)
    {
        bool overridePropertyFlags=theGroup->getOverridesObjectMainProperties()||(detectableMask==-1);
        for (size_t i=0;i<theGroup->getSceneObjectCountInCollection();i++)
        {
            CSceneObject* anObject=App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ( (anObject!=nullptr)&&(anObject->isPotentiallyDetectable()) )
            {
                if ( (anObject->getCumulativeObjectSpecialProperty()&detectableMask)||overridePropertyFlags )
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::removeAllCollections()
{
    while (getObjectCount()!=0)
        removeCollection(getObjectFromIndex(0)->getCollectionHandle());
}

void CCollectionContainer::setUpDefaultValues()
{
    removeAllCollections();
}

void CCollectionContainer::removeCollection(int collectionHandle)
{
    App::currentWorld->announceCollectionWillBeErased(collectionHandle);
    _removeCollection(collectionHandle);
    App::setFullDialogRefreshFlag();
}

void CCollectionContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
{
    minSuffix=-1;
    maxSuffix=-1;
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s=tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(),true);
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

bool CCollectionContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(),true));
            for (size_t j=0;j<getObjectCount();j++)
            {
                int s2=tt::getNameSuffixNumber(getObjectFromIndex(j)->getCollectionName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(getObjectFromIndex(j)->getCollectionName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CCollectionContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(),true));
            getObjectFromIndex(i)->setCollectionName(tt::generateNewName_hash(name1.c_str(),suffix2+1).c_str(),false);
        }
    }
}

void CCollectionContainer::addCollection(CCollection* collection,bool objectIsACopy)
{
    addCollectionWithSuffixOffset(collection,objectIsACopy,1);
}

void CCollectionContainer::addCollectionWithSuffixOffset(CCollection* collection,bool objectIsACopy,int suffixOffset)
{
    std::string theName=collection->getCollectionName();
    if (theName.length()==0)
        theName="Collection";
    if (objectIsACopy)
        theName=tt::generateNewName_hash(theName.c_str(),suffixOffset);
    while (getObjectFromName(theName.c_str())!=nullptr)
        theName=tt::generateNewName_hashOrNoHash(theName.c_str(),objectIsACopy);
    collection->setCollectionName(theName.c_str(),false);

    int handle=SIM_IDSTART_COLLECTION;
    while (getObjectFromHandle(handle)!=nullptr)
        handle++;
    collection->setCollectionHandle(handle);

    _addCollection(collection);

    App::setFullDialogRefreshFlag();
}

void CCollectionContainer::addCollectionToSelection(int collectionHandle) const
{
    CCollection* it=getObjectFromHandle(collectionHandle);
    if (it!=nullptr)
    {
        for (size_t i=0;i<it->getSceneObjectCountInCollection();i++)
            App::currentWorld->sceneObjects->addObjectToSelection(it->getSceneObjectHandleFromIndex(i));
    }
}

void CCollectionContainer::performObjectLoadingMapping(const std::vector<int>* map)
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->performObjectLoadingMapping(map);
}

void CCollectionContainer::_removeCollection(int collectionHandle)
{ // Overridden from _CCollectionContainer_
    CCollection* ig=getObjectFromHandle(collectionHandle);
    if (ig!=nullptr)
        ig->removeSynchronizationObject(false);

    _CCollectionContainer_::_removeCollection(collectionHandle);
}

void CCollectionContainer::_addCollection(CCollection* collection)
{ // Overridden from _CCollectionContainer_
    _CCollectionContainer_::_addCollection(collection);

    if (collection->setObjectCanSync(true))
        collection->buildUpdateAndPopulateSynchronizationObject(nullptr);
}

void CCollectionContainer::buildUpdateAndPopulateSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollection* it=getObjectFromIndex(i);
        it->buildUpdateAndPopulateSynchronizationObject(nullptr);
    }
}

void CCollectionContainer::connectSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollection* it=getObjectFromIndex(i);
        it->connectSynchronizationObject();
    }
}

void CCollectionContainer::removeSynchronizationObjects(bool localReferencesToItOnly)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollection* it=getObjectFromIndex(i);
        it->removeSynchronizationObject(localReferencesToItOnly);
    }
}
