#include "collection.h"
#include "app.h"
#include "base64.h"
#include "tt.h"
#include "ttUtil.h"
#include "global.h"

CCollection::CCollection()
{
    _overridesObjectMainProperties=false;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
}

CCollection::~CCollection()
{
}

void CCollection::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    //_initialValuesInitialized=true;
}

void CCollection::simulationAboutToStart()
{
    initializeInitialValues(false);
}

void CCollection::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    //if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    //{
    //}
    //_initialValuesInitialized=false;
}

bool CCollection::isObjectInCollection(int objectHandle) const
{
    for (size_t i=0;i<_collectionObjects.size();i++)
    {
        if (_collectionObjects[i]==objectHandle)
            return(true);
    }
    return(false);
}

void CCollection::addCollectionElement(CCollectionElement* collectionElement)
{
    bool full=(getElementCount()!=0);
    int i=0;
    while (getElementFromHandle(i)!=nullptr)
        i++;
    collectionElement->setElementHandle(i);
    _addCollectionElement(collectionElement);
    actualizeCollection(full);
    App::setFullDialogRefreshFlag();
}

bool CCollection::actualizeCollection(bool full)
{   // return value false means that this collection is empty and should be removed. Full is true by default!
    App::setLightDialogRefreshFlag();
    bool retVal=false;
    size_t i=0;
    // First we remove all collection elements which are not valid anymore:
    while (i<getElementCount())
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(getElementFromIndex(i)->getMainObject());
        if (it==nullptr)
        {
            if (getElementFromIndex(i)->getElementType()!=sim_collectionelement_all)
            {
                _CCollection_::_removeCollectionElementFromHandle(getElementFromIndex(i)->getElementHandle());
                i=0;
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
        for (size_t i=0;i<getElementCount();i++)
        {
            if (getElementFromIndex(i)->getElementType()!=sim_collectionelement_all)
                removeAll=false;
        }
        if (removeAll)
        {
            while (getElementCount()>0)
                _CCollection_::_removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
        }
    }
    // Is this collection still valid?
    if (getElementCount()!=0)
    {
        retVal=true;
        // Now we set-up the object list:
        std::vector<int> objs;
        for (size_t i=0;i<getElementCount();i++)
            getElementFromIndex(i)->addOrRemoveYourObjects(&objs);
        _updateCollectionObjects_(objs);
    }
    return(retVal);
}

void CCollection::removeCollectionElementFromHandle(int collectionElementHandle)
{
    _CCollection_::_removeCollectionElementFromHandle(collectionElementHandle);
    actualizeCollection();
}

bool CCollection::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{ // Return value true means that this collection is empty and should be removed
    bool retVal=false;
    size_t i=0;
    size_t initialSubGroupListSize=getElementCount();
    while (i<getElementCount())
    {
        if ( (getElementFromIndex(i)->getMainObject()==objectHandle) ) //  GROUP_EVERYTHING is handled a little bit further down
        {
            _CCollection_::_removeCollectionElementFromHandle(getElementFromIndex(i)->getElementHandle());
            i=0;
        }
        else
            i++;
    }

    // Now we have to take care of the GROUP_EVERYTHING type:
    bool removeAll=true;
    for (size_t i=0;i<getElementCount();i++)
    {
        if (getElementFromIndex(i)->getElementType()!=sim_collectionelement_all)
            removeAll=false;
    }
    if (removeAll)
    {
        while (getElementCount()>0)
            _CCollection_::_removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
    }

    if ( (getElementCount()!=initialSubGroupListSize)&&copyBuffer )
        retVal=true; // (during copy/paste, only intact collections should be copied!)
    else
        retVal=(getElementCount()==0);

    if (!retVal)
    {   //We also remove that object from the object list:
        std::vector<int> objs(_collectionObjects);
        for (size_t i=0;i<objs.size();i++)
        {
            if (objs[i]==objectHandle)
            {
                objs.erase(objs.begin()+i);
                break;
            }
        }
        _updateCollectionObjects_(objs);
    }

    return(retVal);
}

bool CCollection::setCollectionName(const char* newName,bool check)
{ // Overridden from _CCollection_
    bool diff=false;
    CCollection* it=nullptr;
    if (check)
        it=App::currentWorld->collections->getObjectFromHandle(_collectionHandle);
    if (it!=this)
        diff=_CCollection_::setCollectionName(newName,check); // no checking or object not yet in world
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getCollectionName()!=nm)
            {
                while (App::currentWorld->collections->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm.c_str(),!tt::isHashFree(nm.c_str()));
                diff=_CCollection_::setCollectionName(nm.c_str(),check);
            }
        }
    }
    if (diff)
        App::setFullDialogRefreshFlag();
    return(diff);
}

void CCollection::performObjectLoadingMapping(const std::vector<int>* map)
{
    for (size_t i=0;i<getElementCount();i++)
        getElementFromIndex(i)->performObjectLoadingMapping(map);
    actualizeCollection();
}

CCollection* CCollection::copyYourself() const
{
    CCollection* newCollection=new CCollection();
    newCollection->_collectionHandle=_collectionHandle; // important for copy operations connections
    newCollection->_collectionName=_collectionName;
    for (size_t i=0;i<getElementCount();i++)
        newCollection->_CCollection_::_addCollectionElement(getElementFromIndex(i)->copyYourself());
    newCollection->_overridesObjectMainProperties=_overridesObjectMainProperties;
    return(newCollection);
}

void CCollection::emptyCollection()
{
    while (getElementCount()>0)
        _CCollection_::_removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
    _collectionObjects.clear(); // added on 14/10/2016 (was forgotten)
    actualizeCollection(true); // we keep this collection alive
}

std::string CCollection::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

size_t CCollection::getSceneObjectCountInCollection() const
{
    return(_collectionObjects.size());
}

int CCollection::getSceneObjectHandleFromIndex(size_t index) const
{
    int retVal=-1;
    if (index<_collectionObjects.size())
        retVal=_collectionObjects[index];
    return(retVal);
}

void CCollection::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Grn");
            ar << _collectionName;
            ar.flush();

            ar.storeDataName("Gix");
            ar << _collectionHandle;
            ar.flush();

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_overridesObjectMainProperties);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            for (size_t i=0;i<getElementCount();i++)
            {
                ar.storeDataName("Asg");
                ar.setCountingMode();
                getElementFromIndex(i)->serialize(ar);
                if (ar.setWritingMode())
                    getElementFromIndex(i)->serialize(ar);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {   // Loading
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
                        ar >> _collectionName;
                    }
                    if (theName.compare("Gix")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _collectionHandle;
                    }
                    if (theName.compare("Asg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CCollectionElement* it=new CCollectionElement(0,0,false);
                        it->serialize(ar);
                        _CCollection_::_addCollectionElement(it);
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
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_int("handle",_collectionHandle);

            if (exhaustiveXml)
                ar.xmlAddNode_string("name",_collectionName.c_str());
            else
                ar.xmlAddNode_string("name",("@collection@"+_collectionName).c_str());

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("overrideProperties",_overridesObjectMainProperties);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                std::string str(base64_encode((unsigned char*)_uniquePersistentIdString.c_str(),_uniquePersistentIdString.size()));
                ar.xmlAddNode_string("uniquePersistentIdString_base64Coded",str.c_str());
            }

            for (size_t i=0;i<getElementCount();i++)
            {
                ar.xmlAddNode_comment(" 'item' tag: at least one of such tag is required ",exhaustiveXml);
                ar.xmlPushNewNode("item");
                getElementFromIndex(i)->serialize(ar);
                ar.xmlPopNode();
            }
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_int("handle",_collectionHandle);

            if ( ar.xmlGetNode_string("name",_collectionName,exhaustiveXml)&&(!exhaustiveXml) )
            {
                _collectionLoadName=_collectionName;
                if (_collectionName.find("@collection@")==0)
                    _collectionName.assign(_collectionLoadName.begin()+strlen("@collection@"),_collectionLoadName.end());
                tt::removeIllegalCharacters(_collectionName,true);
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("overrideProperties",_overridesObjectMainProperties,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml&&ar.xmlGetNode_string("uniquePersistentIdString_base64Coded",_uniquePersistentIdString))
                _uniquePersistentIdString=base64_decode(_uniquePersistentIdString);

            if (ar.xmlPushChildNode("item",exhaustiveXml))
            {
                while (true)
                {
                    CCollectionElement* it=new CCollectionElement(0,0,false);
                    it->serialize(ar);
                    _CCollection_::_addCollectionElement(it);
                    if (!ar.xmlPushSiblingNode("item",false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CCollection::_updateCollectionObjects_(const std::vector<int>& sceneObjectHandles)
{
    _collectionObjects.assign(sceneObjectHandles.begin(),sceneObjectHandles.end());
}

std::string CCollection::getCollectionLoadName() const
{
    return(_collectionLoadName);
}

void CCollection::_addCollectionElement(CCollectionElement* collectionElement)
{ // Overridden from _CCollection_
    _CCollection_::_addCollectionElement(collectionElement);

    if (getObjectCanSync())
        collectionElement->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
}

void CCollection::_removeCollectionElementFromHandle(int collectionElementHandle)
{ // Overridden from _CCollection_
    if (getObjectCanSync())
    {
        CCollectionElement* el=getElementFromHandle(collectionElementHandle);
        if (el!=nullptr)
            el->removeSynchronizationObject(false);
    }

    _CCollection_::_removeCollectionElementFromHandle(collectionElementHandle);
}

void CCollection::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_collectionHandle;
        r.objType=sim_syncobj_collection;
        setSyncMsgRouting(parentRouting,r);

        // Build remote collection:
        sendVoid(sim_syncobj_collection_create);

        // Update the remote object:
        _setOverridesObjectMainProperties_send(_overridesObjectMainProperties);
        _setCollectionName_send(_collectionName.c_str());

        // Populate remote collections with remote collection elements:
        for (size_t i=0;i<getElementCount();i++)
        {
            CCollectionElement* it=getElementFromIndex(i);
            it->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
        }
    }
}

void CCollection::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
    }
}

void CCollection::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote collection:
            sendVoid(sim_syncobj_collection_delete);
        }
    }
}
