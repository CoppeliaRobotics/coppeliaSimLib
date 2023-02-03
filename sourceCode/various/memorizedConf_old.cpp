#include <memorizedConf_old.h>
#include <app.h>
#include <simInternal.h>
#include <ttUtil.h>

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
        position=double(path->pathContainer->getPosition());
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
        act->setPosition(position);
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
        pushFloatToBuffer(position,data);
    if (objectType==sim_object_joint_type)
    {
        pushFloatToBuffer(position,data);
        for (int i=0;i<4;i++)
            pushFloatToBuffer(sphericalJointOrientation(i),data);
    }
    pushIntToBuffer(objectType,data);
    for (int i=0;i<7;i++)
        pushFloatToBuffer(configuration(i),data);
    pushIntToBuffer(int(uniqueID),data);
    pushIntToBuffer(int(parentUniqueID),data);
}

void CMemorizedConf_old::serializeFromMemory(std::vector<char>& data)
{
    parentUniqueID=popIntFromBuffer(data);
    uniqueID=popIntFromBuffer(data);
    for (int i=0;i<7;i++)
        configuration(6-i)=popFloatFromBuffer(data);
    objectType=popIntFromBuffer(data);
    if (objectType==sim_object_joint_type)
    {
        for (int i=0;i<4;i++)
            sphericalJointOrientation(3-i)=popFloatFromBuffer(data);
        position=popFloatFromBuffer(data);
    }
    if (objectType==sim_object_path_type)
        position=popFloatFromBuffer(data);
}

void CMemorizedConf_old::pushFloatToBuffer(double d,std::vector<char>& data)
{
    for (size_t i=0;i<sizeof(double);i++)
        data.push_back(((char*)&d)[i]);
}

double CMemorizedConf_old::popFloatFromBuffer(std::vector<char>& data)
{
    double d;
    for (size_t i=0;i<sizeof(double);i++)
    {
        ((char*)&d)[sizeof(double)-1-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}

void CMemorizedConf_old::pushIntToBuffer(int d,std::vector<char>& data)
{
    for (size_t i=0;i<sizeof(int);i++)
        data.push_back(((char*)&d)[i]);
}

int CMemorizedConf_old::popIntFromBuffer(std::vector<char>& data)
{
    int d;
    for (size_t i=0;i<sizeof(int);i++)
    {
        ((char*)&d)[sizeof(int)-1-i]=data[data.size()-1];
        data.pop_back();
    }
    return(d);
}

