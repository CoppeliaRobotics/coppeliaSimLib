
#include "vrepMainHeader.h"
#include "iKObjCont.h"
#include "gCSDefs.h"

CIKObjCont::CIKObjCont()
{
    nextID=0;
}

CIKObjCont::~CIKObjCont()
{
    removeAllObjects();
}

void CIKObjCont::removeAllObjects()
{
    for (int i=0;i<int(allObjects.size());i++)
        delete allObjects[i];
    nextID=0;
}

void CIKObjCont::addChild(CIKJoint* parent,CIKObject* child)
{
    child->parent=parent;
    allObjects.push_back(child);
    child->identifier=nextID;
    nextID++;
}

void CIKObjCont::addRoot(CIKJoint* root)
{
    allObjects.push_back(root);
    root->identifier=nextID;
    nextID++;
}

CIKObject* CIKObjCont::getObjectWithID(int theID)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->identifier==theID)
            return(allObjects[i]);
    }
    return(nullptr);
}

CIKJoint* CIKObjCont::getJointWithData(int theData)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->data==theData)
        {
            if (allObjects[i]->objectType==IK_JOINT_TYPE)
                return((CIKJoint*)allObjects[i]);
        }
    }
    return(nullptr);
}
