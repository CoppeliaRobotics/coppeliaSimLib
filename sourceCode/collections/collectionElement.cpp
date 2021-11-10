#include "collectionElement.h"
#include "app.h"

CCollectionElement::CCollectionElement(int mainObjectHandle,int collectionType,bool add)
{
    _elementHandle=-1;
    _mainObjectHandle=mainObjectHandle;
    _elementType=collectionType;
    _additive=add;
}

CCollectionElement::~CCollectionElement()
{
}

void CCollectionElement::addOrRemoveAnObject(std::vector<int>* theObjects,int objectHandle) const
{
    if (_additive)
    {
        for (size_t i=0;i<theObjects->size();i++)
        {
            if (theObjects->at(i)==objectHandle)
                return;
        }
        theObjects->push_back(objectHandle);
    }
    else
    {
        for (size_t i=0;i<theObjects->size();i++)
        {
            if (theObjects->at(i)==objectHandle)
            {
                theObjects->erase(theObjects->begin()+i);
                return;
            }
        }
    }
}

std::string CCollectionElement::getMainObjectTempName() const
{
    return(_objectTempName);
}

void CCollectionElement::addOrRemoveYourObjects(std::vector<int>* theObjects) const
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_mainObjectHandle);
    if (it==nullptr)
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            addOrRemoveAnObject(theObjects,App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
    }
    if (_elementType==sim_collectionelement_loose)
        addOrRemoveAnObject(theObjects,it->getObjectHandle());
    if ( (_elementType==sim_collectionelement_frombaseincluded)||(_elementType==sim_collectionelement_frombaseexcluded) )
    {
        std::vector<CSceneObject*> allObj;
        allObj.reserve(8);
        allObj.clear();
        it->getAllObjectsRecursive(&allObj,_elementType==sim_collectionelement_frombaseincluded);
        for (size_t i=0;i<allObj.size();i++)
            addOrRemoveAnObject(theObjects,allObj[i]->getObjectHandle());
    }
    if ( (_elementType==sim_collectionelement_fromtipincluded)||(_elementType==sim_collectionelement_fromtipexcluded) )
    {
        if (_elementType==sim_collectionelement_fromtipincluded)
            addOrRemoveAnObject(theObjects,it->getObjectHandle());
        CSceneObject* itIt=it->getParent();
        while (itIt!=nullptr)
        {
            addOrRemoveAnObject(theObjects,itIt->getObjectHandle());
            itIt=itIt->getParent();
        }
    }
}

bool CCollectionElement::isSame(CCollectionElement* it) const
{
    if (it->getMainObject()!=_mainObjectHandle)
        return(false);
    if (it->getElementType()!=_elementType)
        return(false);
    return(it->getIsAdditive()==_additive);
}

void CCollectionElement::performObjectLoadingMapping(const std::vector<int>* map)
{
    _mainObjectHandle=CWorld::getLoadingMapping(map,_mainObjectHandle);
}

CCollectionElement* CCollectionElement::copyYourself() const
{
    CCollectionElement* newGroupEl=new CCollectionElement(_mainObjectHandle,_elementType,_additive);
    return(newGroupEl);
}

void CCollectionElement::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Obj");
            ar << _mainObjectHandle << _elementHandle;
            ar.flush();

            ar.storeDataName("Gty");
            ar << (unsigned char)_elementType;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            dummy=dummy+1*_additive;
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
                        ar >> _mainObjectHandle >> _elementHandle;
                    }
                    if (theName.compare("Gty")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _elementType=(int)dummy;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _additive=(dummy&1)==1;
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
            {
                ar.xmlAddNode_int("id",_mainObjectHandle);
                ar.xmlAddNode_int("itemId",_elementHandle);
            }
            else
            {
                std::string str;
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_mainObjectHandle);
                if (it!=nullptr)
                    str=it->getObjectName_old();
                ar.xmlAddNode_comment(" 'objectName' tag: required if 'type' is not 'everything'. Has to be an existing scene object name ",exhaustiveXml);
                ar.xmlAddNode_string("objectName",str.c_str());
            }

            ar.xmlAddNode_comment(" 'type' tag: can be 'looseObjects', 'fromBaseIncludingBase', 'fromBaseExcludingBase', 'fromTipIncludingTip', 'fromTipExcludingTip' or 'everything' ",exhaustiveXml);
            ar.xmlAddNode_enum("type",_elementType,sim_collectionelement_loose,"looseObjects",sim_collectionelement_frombaseincluded,"fromBaseIncludingBase",sim_collectionelement_frombaseexcluded,"fromBaseExcludingBase",sim_collectionelement_fromtipincluded,"fromTipIncludingTip",sim_collectionelement_fromtipexcluded,"fromTipExcludingTip",sim_collectionelement_all,"everything");

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("additive",_additive);
            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("id",_mainObjectHandle);
                ar.xmlGetNode_int("itemId",_elementHandle);
            }
            else
                ar.xmlGetNode_string("objectName",_objectTempName,exhaustiveXml);

            ar.xmlGetNode_enum("type",_elementType,exhaustiveXml,"looseObjects",sim_collectionelement_loose,"fromBaseIncludingBase",sim_collectionelement_frombaseincluded,"fromBaseExcludingBase",sim_collectionelement_frombaseexcluded,"fromTipIncludingTip",sim_collectionelement_fromtipincluded,"fromTipExcludingTip",sim_collectionelement_fromtipexcluded,"everything",sim_collectionelement_all);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("additive",_additive,exhaustiveXml);
                ar.xmlPopNode();
            }
        }
    }
}

bool CCollectionElement::getIsAdditive() const
{
    return(_additive);
}

int CCollectionElement::getMainObject() const
{
    return(_mainObjectHandle);
}

int CCollectionElement::getElementType() const
{
    return(_elementType);
}

int CCollectionElement::getElementHandle() const
{
    return(_elementHandle);
}

void CCollectionElement::setElementHandle(int newHandle)
{
    _elementHandle=newHandle;
}

void CCollectionElement::setMainObject(int mo)
{ // only used in special situations
    _mainObjectHandle=mo;
}
