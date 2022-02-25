#include "memorizedConf_old.h"
#include "app.h"
#include "simInternal.h"
#include "ttUtil.h"

CMemorizedConf_old::CMemorizedConf_old(CSceneObject* theObject)
{
    uniqueID=theObject->getObjectUid();
    parentUniqueID=-1;
    if (theObject->getParent()!=nullptr)
        parentUniqueID=theObject->getParent()->getObjectUid();
    configuration=theObject->getLocalTransformation();
    objectType=theObject->getObjectType();
    if (objectType==sim_object_joint_type)
    {
        CJoint* act=(CJoint*)theObject;
        position=act->getPosition();
        sphericalJointOrientation=act->getSphericalTransformation();
    }
    if (objectType==sim_object_path_type)
    {
        CPath_old* path=(CPath_old*)theObject;
        position=float(path->pathContainer->getPosition());
    }
}

CMemorizedConf_old::CMemorizedConf_old()
{ // Default constructor for serialization from memory!
}

CMemorizedConf_old::~CMemorizedConf_old()
{
}

int CMemorizedConf_old::getParentCount()
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromUid(uniqueID);
    if (it==nullptr)
        return(0);
    return(it->getParentCount());
}

void CMemorizedConf_old::restore()
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromUid(uniqueID);
    if (it==nullptr)
        return;
    it->setDynamicsResetFlag(true,false); // dynamically enabled objects have to be reset first!
    long long int puid=-1;
    if (it->getParent()!=nullptr)
        puid=it->getParent()->getObjectUid();
    if (parentUniqueID==puid)
        it->setLocalTransformation(configuration);
    if (objectType==sim_object_joint_type)
    {
        CJoint* act=(CJoint*)it;
        act->setPosition(position,false);
        act->setSphericalTransformation(sphericalJointOrientation);
    }
    if (objectType==sim_object_path_type)
    {
        CPath_old* path=(CPath_old*)it;
        path->pathContainer->setPosition(position);
    }
}

bool CMemorizedConf_old::doesStillExist()
{
    return(App::currentWorld->sceneObjects->getObjectFromUid(uniqueID)!=nullptr);
}

void CMemorizedConf_old::serializeToMemory(std::vector<char>& data)
{
    if (objectType==sim_object_path_type)
        CTTUtil::pushFloatToBuffer(position,data);
    if (objectType==sim_object_joint_type)
    {
        CTTUtil::pushFloatToBuffer(position,data);
        for (int i=0;i<4;i++)
            CTTUtil::pushFloatToBuffer(sphericalJointOrientation(i),data);
    }
    CTTUtil::pushIntToBuffer(objectType,data);
    for (int i=0;i<7;i++)
        CTTUtil::pushFloatToBuffer(configuration(i),data);
    CTTUtil::pushIntToBuffer(int(uniqueID),data);
    CTTUtil::pushIntToBuffer(int(parentUniqueID),data);
}

void CMemorizedConf_old::serializeFromMemory(std::vector<char>& data)
{
    parentUniqueID=CTTUtil::popIntFromBuffer(data);
    uniqueID=CTTUtil::popIntFromBuffer(data);
    for (int i=0;i<7;i++)
        configuration(6-i)=CTTUtil::popFloatFromBuffer(data);
    objectType=CTTUtil::popIntFromBuffer(data);
    if (objectType==sim_object_joint_type)
    {
        for (int i=0;i<4;i++)
            sphericalJointOrientation(3-i)=CTTUtil::popFloatFromBuffer(data);
        position=CTTUtil::popFloatFromBuffer(data);
    }
    if (objectType==sim_object_path_type)
        position=CTTUtil::popFloatFromBuffer(data);
}
