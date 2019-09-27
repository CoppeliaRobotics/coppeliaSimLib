
#include "vrepMainHeader.h"
#include "memorizedConf.h"
#include "app.h"
#include "v_rep_internal.h"

CMemorizedConf::CMemorizedConf(C3DObject* theObject)
{
    uniqueID=theObject->getUniqueID();
    parentUniqueID=-1;
    if (theObject->getParentObject()!=nullptr)
        parentUniqueID=theObject->getParentObject()->getUniqueID();
    configuration=theObject->getLocalTransformationPart1();
    objectType=theObject->getObjectType();
    memorizedConfigurationValidCounter=theObject->getMemorizedConfigurationValidCounter();
    if (objectType==sim_object_joint_type)
    {
        CJoint* act=(CJoint*)theObject;
        position=act->getPosition();
        sphericalJointOrientation=act->getSphericalTransformation();
    }
    if (objectType==sim_object_path_type)
    {
        CPath* path=(CPath*)theObject;
        position=float(path->pathContainer->getPosition());
    }
}

CMemorizedConf::CMemorizedConf()
{ // Default constructor for serialization from memory!
}

CMemorizedConf::~CMemorizedConf()
{
}

int CMemorizedConf::getParentCount()
{
    C3DObject* it=App::ct->objCont->getObjectWithUniqueID(uniqueID);
    if (it==nullptr)
        return(0);
    return(it->getParentCount());
}

void CMemorizedConf::restore()
{
    C3DObject* it=App::ct->objCont->getObjectWithUniqueID(uniqueID);
    if ( (it==nullptr)||(it->getMemorizedConfigurationValidCounter()!=memorizedConfigurationValidCounter) ) // second part is in case a shape gets edited
        return;
    it->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
    int puid=-1;
    if (it->getParentObject()!=nullptr)
        puid=it->getParentObject()->getUniqueID();
    if (parentUniqueID==puid)
        it->setLocalTransformation(configuration);
    if (objectType==sim_object_joint_type)
    {
        CJoint* act=(CJoint*)it;
        act->setPosition(position);
        act->setSphericalTransformation(sphericalJointOrientation);
    }
    if (objectType==sim_object_path_type)
    {
        CPath* path=(CPath*)it;
        path->pathContainer->setPosition(position);
    }
}

bool CMemorizedConf::doesStillExist()
{
    return(App::ct->objCont->getObjectWithUniqueID(uniqueID)!=nullptr);
}

void CMemorizedConf::serializeToMemory(std::vector<char>& data)
{
    if (objectType==sim_object_path_type)
    {
        _pushFloatToData(position,data);
    }
    if (objectType==sim_object_joint_type)
    {
        _pushFloatToData(position,data);
        for (int i=0;i<4;i++)
            _pushFloatToData(sphericalJointOrientation(i),data);
    }
    _pushIntToData(objectType,data);
    for (int i=0;i<7;i++)
        _pushFloatToData(configuration(i),data);
    _pushIntToData(uniqueID,data);
    _pushIntToData(memorizedConfigurationValidCounter,data);
    _pushIntToData(parentUniqueID,data);
}

void CMemorizedConf::serializeFromMemory(std::vector<char>& data)
{
    parentUniqueID=_popIntFromData(data);
    memorizedConfigurationValidCounter=_popIntFromData(data);
    uniqueID=_popIntFromData(data);
    for (int i=0;i<7;i++)
        configuration(6-i)=_popFloatFromData(data);
    objectType=_popIntFromData(data);
    if (objectType==sim_object_joint_type)
    {
        for (int i=0;i<4;i++)
            sphericalJointOrientation(3-i)=_popFloatFromData(data);
        position=_popFloatFromData(data);
    }
    if (objectType==sim_object_path_type)
    {
        position=_popFloatFromData(data);
    }
}

void CMemorizedConf::_pushFloatToData(float d,std::vector<char>& data)
{
    for (int i=0;i<4;i++)
        data.push_back(((char*)&d)[i]);
}

float CMemorizedConf::_popFloatFromData(std::vector<char>& data)
{
    float d;
    for (int i=0;i<4;i++)
    {
        ((char*)&d)[3-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}

void CMemorizedConf::_pushIntToData(int d,std::vector<char>& data)
{
    for (int i=0;i<4;i++)
        data.push_back(((char*)&d)[i]);
}

int CMemorizedConf::_popIntFromData(std::vector<char>& data)
{
    int d;
    for (int i=0;i<4;i++)
    {
        ((char*)&d)[3-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}



