
#include "vrepMainHeader.h"
#include "regCollectionEl.h"
#include "global.h"
#include "3DObject.h"
#include "tt.h"
#include "app.h"


CRegCollectionEl::CRegCollectionEl(int objID,int grType,bool add)
{
    objectID=objID;
    groupType=grType;
    additive=add;
    subGroupID=0;
}

CRegCollectionEl::~CRegCollectionEl()
{
}

void CRegCollectionEl::addOrRemoveAnObject(std::vector<int>* theObjects,int objectID)
{
    if (additive)
    {
        for (int i=0;i<int(theObjects->size());i++)
        {
            if (theObjects->at(i)==objectID)
                return;
        }
        theObjects->push_back(objectID);
    }
    else
    {
        for (int i=0;i<int(theObjects->size());i++)
        {
            if (theObjects->at(i)==objectID)
            {
                theObjects->erase(theObjects->begin()+i);
                return;
            }
        }
    }
}

void CRegCollectionEl::addOrRemoveYourObjects(std::vector<int>* theObjects)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
    if (it==nullptr)
    {
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
            addOrRemoveAnObject(theObjects,App::ct->objCont->objectList[i]);
    }
    if (groupType==GROUP_LOOSE)
        addOrRemoveAnObject(theObjects,it->getObjectHandle());
    if ( (groupType==GROUP_FROM_BASE_INCLUDED)||(groupType==GROUP_FROM_BASE_EXCLUDED) )
    {
        std::vector<C3DObject*> allObj;
        allObj.reserve(8);
        allObj.clear();
        it->getAllObjectsRecursive(&allObj,groupType==GROUP_FROM_BASE_INCLUDED);
        for (int i=0;i<int(allObj.size());i++)
            addOrRemoveAnObject(theObjects,allObj[i]->getObjectHandle());
    }
    if ( (groupType==GROUP_FROM_TIP_INCLUDED)||(groupType==GROUP_FROM_TIP_EXCLUDED) )
    {
        if (groupType==GROUP_FROM_TIP_INCLUDED)
            addOrRemoveAnObject(theObjects,it->getObjectHandle());
        C3DObject* itIt=it->getParentObject();
        while (itIt!=nullptr)
        {
            addOrRemoveAnObject(theObjects,itIt->getObjectHandle());
            itIt=itIt->getParentObject();
        }
    }
}

bool CRegCollectionEl::isAdditive()
{
    return(additive);
}

int CRegCollectionEl::getMainObject()
{
    return(objectID);
}

int CRegCollectionEl::getCollectionType()
{
    return(groupType);
}

bool CRegCollectionEl::isSame(CRegCollectionEl* it)
{
    if (it->getMainObject()!=objectID)
        return(false);
    if (it->getCollectionType()!=groupType)
        return(false);
    return(it->isAdditive()==additive);
}

int CRegCollectionEl::getSubCollectionID()
{
    return(subGroupID);
}

void CRegCollectionEl::setSubCollectionID(int newID)
{
    subGroupID=newID;
}

void CRegCollectionEl::performObjectLoadingMapping(std::vector<int>* map)
{
    objectID=App::ct->objCont->getLoadingMapping(map,objectID);
}

CRegCollectionEl* CRegCollectionEl::copyYourself()
{
    CRegCollectionEl* newGroupEl=new CRegCollectionEl(objectID,groupType,additive);
    newGroupEl->setSubCollectionID(subGroupID);
    return(newGroupEl);
}

void CRegCollectionEl::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Obj");
            ar << objectID << subGroupID;
            ar.flush();

            ar.storeDataName("Gty");
            ar << (unsigned char)groupType;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            dummy=dummy+1*additive;
            ar << dummy;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Obj")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> objectID >> subGroupID;
                    }
                    if (theName.compare("Gty")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        groupType=(int)dummy;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        additive=(dummy&1)==1;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}
