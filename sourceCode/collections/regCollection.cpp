#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "regCollection.h"
#include "3DObject.h"
#include "tt.h"
#include "ttUtil.h"
#include "app.h"

CRegCollection::CRegCollection(std::string grName)
{
    groupName=grName;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    subCollectionList.reserve(4);
    subCollectionList.clear();
    markedObjects.reserve(4);
    markedObjects.clear();
    collectionObjects.reserve(4);
    collectionObjects.clear();
    _overridesObjectMainProperties=false;
    groupID=SIM_IDSTART_COLLECTION;
}

CRegCollection::~CRegCollection()
{
    for (int i=0;i<int(subCollectionList.size());i++)
        delete subCollectionList[i];
    subCollectionList.clear();
}

std::string CRegCollection::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CRegCollection::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    //_initialValuesInitialized=simulationIsRunning;
    //if (simulationIsRunning)
    //{
    //}
}

void CRegCollection::simulationAboutToStart()
{
    initializeInitialValues(true);
}
void CRegCollection::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    //if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    //{
    //}
    //_initialValuesInitialized=false;
}

bool CRegCollection::getOverridesObjectMainProperties()
{
    return(_overridesObjectMainProperties);
}

void CRegCollection::setOverridesObjectMainProperties(bool o)
{
    _overridesObjectMainProperties=o;
}


bool CRegCollection::isObjectInCollection(int objID)
{
    for (int i=0;i<int(collectionObjects.size());i++)
    {
        if (collectionObjects[i]==objID)
            return(true);
    }
    return(false);
}

CRegCollectionEl* CRegCollection::getSubCollection(int subGroupID)
{
    for (int i=0;i<int(subCollectionList.size());i++)
    {
        if (subCollectionList[i]->getSubCollectionID()==subGroupID)
            return(subCollectionList[i]);
    }
    return(nullptr);
}

void CRegCollection::addSubCollection(CRegCollectionEl* subGr)
{
    bool full=(subCollectionList.size()!=0);
    // We set a new ID:
    int i=0;
    while (getSubCollection(i)!=nullptr)
        i++;
    subGr->setSubCollectionID(i);
    subCollectionList.push_back(subGr);
    actualizeCollection(full);
    App::ct->collections->repairMarking();
    App::setFullDialogRefreshFlag();
}

bool CRegCollection::actualizeCollection(bool full)
{   // return value false means that this group is empty and should be removed. Full is true by default!
    App::setLightDialogRefreshFlag();
    // First we erase all sub-groups which are not active anymore:
    size_t i=0;
    // First we remove all sub groups which are not valid anymore:
    while (i<subCollectionList.size())
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(subCollectionList[i]->getMainObject());
        if (it==nullptr)
        {
            if (subCollectionList[i]->getCollectionType()!=GROUP_EVERYTHING)
            {
                delete subCollectionList[i];
                subCollectionList.erase(subCollectionList.begin()+i);
            }
            else
                i++;
        }
        else
            i++;
    }
    // Now we have to take care of the GROUP_EVERYTHING type:
    if (full)
    {
        bool removeAll=true;
        for (int i=0;i<int(subCollectionList.size());i++)
        {
            if (subCollectionList[i]->getCollectionType()!=GROUP_EVERYTHING)
                removeAll=false;
        }
        if (removeAll)
        {
            for (int i=0;i<int(subCollectionList.size());i++)
                delete subCollectionList[i];
            subCollectionList.clear();
        }
    }
    // Is this group still valid?
    if (subCollectionList.size()==0)
        return(false);

    // Now we set-up the object list:
    collectionObjects.clear();
    for (int i=0;i<int(subCollectionList.size());i++)
        subCollectionList[i]->addOrRemoveYourObjects(&collectionObjects);

    markedObjects.clear();
    return(true);
}

void CRegCollection::mark(int objID)
{
    bool found=false;
    for (int i=0;i<int(collectionObjects.size());i++)
    {
        if (collectionObjects[i]==objID)
        {
            found=true;
            break;
        }
    }
    if (!found)
        return;
    for (int i=0;i<int(markedObjects.size());i++)
    {
        if (markedObjects[i]==objID)
            return;
    }
    markedObjects.push_back(objID);
}

void CRegCollection::unmark(int objID)
{
    for (int i=0;i<int(markedObjects.size());i++)
    {
        if (markedObjects[i]==objID)
        {
            markedObjects.erase(markedObjects.begin()+i);
            break;
        }
    }
}

void CRegCollection::unmarkAll()
{
    markedObjects.clear();
}

bool CRegCollection::areAllMarked()
{
    if (collectionObjects.size()!=0)
        return(collectionObjects.size()==markedObjects.size());
    return(false);
}

bool CRegCollection::removeSubCollection(int subGroupID)
{
    for (int i=0;i<int(subCollectionList.size());i++)
    {
        if (subCollectionList[i]->getSubCollectionID()==subGroupID)
        {
            subCollectionList.erase(subCollectionList.begin()+i);
            actualizeCollection();
            App::ct->collections->repairMarking();
            return(true);
        }
    }
    return(false);
}

bool CRegCollection::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this group is empty and should be removed
    size_t i=0;
    size_t initialSubGroupListSize=subCollectionList.size();
    while (i<subCollectionList.size())
    {
        if ( (subCollectionList[i]->getMainObject()==objID) ) //  GROUP_EVERYTHING is handled a little bit further down
        {
            delete subCollectionList[i];
            subCollectionList.erase(subCollectionList.begin()+i);
        }
        else
            i++;
    }

    // Now we have to take care of the GROUP_EVERYTHING type:
    bool removeAll=true;
    for (int i=0;i<int(subCollectionList.size());i++)
    {
        if (subCollectionList[i]->getCollectionType()!=GROUP_EVERYTHING)
            removeAll=false;
    }
    if (removeAll)
    {
        for (int i=0;i<int(subCollectionList.size());i++)
            delete subCollectionList[i];
        subCollectionList.clear();
    }

    //We also remove that object from the object list and marked list:
    for (size_t i=0;i<collectionObjects.size();i++)
    {
        if (collectionObjects[i]==objID)
        {
            collectionObjects.erase(collectionObjects.begin()+i);
            break;
        }
    }
    for (size_t i=0;i<markedObjects.size();i++)
    {
        if (markedObjects[i]==objID)
        {
            markedObjects.erase(markedObjects.begin()+i);
            break;
        }
    }
    if ( (subCollectionList.size()!=initialSubGroupListSize)&&copyBuffer )
        return(true); // (during copy/paste, only intact collections should be copied!)
    if (subCollectionList.size()!=0)
        return(false);
    return(true);
}

int CRegCollection::getCollectionID()
{
    return(groupID);
}

void CRegCollection::setCollectionID(int newID)
{
    groupID=newID;
}

std::string CRegCollection::getCollectionName()
{
    return(groupName);
}

void CRegCollection::setCollectionName(std::string newName)
{
    groupName=newName;
    App::setFullDialogRefreshFlag();
}

void CRegCollection::performObjectLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(subCollectionList.size());i++)
        subCollectionList[i]->performObjectLoadingMapping(map);
    actualizeCollection();
}

CRegCollection* CRegCollection::copyYourself()
{
    CRegCollection* newGroup=new CRegCollection(groupName);
    newGroup->setCollectionID(groupID);
    newGroup->subCollectionList.reserve(subCollectionList.size());
    newGroup->subCollectionList.clear();
    for (int i=0;i<int(subCollectionList.size());i++)
        newGroup->subCollectionList.push_back(subCollectionList[i]->copyYourself());
    newGroup->_overridesObjectMainProperties=_overridesObjectMainProperties;
    return(newGroup);
}

void CRegCollection::emptyCollection()
{
    for (size_t i=0;i<subCollectionList.size();i++)
        delete subCollectionList[i];
    subCollectionList.clear();
    collectionObjects.clear(); // added on 14/10/2016 (was forgotten)
    actualizeCollection(true); // we keep this collection alive
}

void CRegCollection::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Grn");
            ar << groupName;
            ar.flush();

            ar.storeDataName("Gix");
            ar << groupID;
            ar.flush();

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_overridesObjectMainProperties);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            for (int i=0;i<int(subCollectionList.size());i++)
            {
                ar.storeDataName("Asg");
                ar.setCountingMode();
                subCollectionList[i]->serialize(ar);
                if (ar.setWritingMode())
                    subCollectionList[i]->serialize(ar);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {   // Loading
            subCollectionList.clear();
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Grn")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> groupName;
                    }
                    if (theName.compare("Gix")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> groupID;
                    }
                    if (theName.compare("Asg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CRegCollectionEl* it=new CRegCollectionEl(0,0,false);
                        it->serialize(ar);
                        subCollectionList.push_back(it);
                    }
                    if (theName=="Par")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _overridesObjectMainProperties=SIM_IS_BIT_SET(nothing,0);
                    }
                    if (theName.compare("Uis")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}
