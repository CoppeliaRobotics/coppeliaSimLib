
#pragma once

#include "iKGraphNode.h"
#include "iKGraphObject.h"
#include "MyMath.h"

class CIKGraphObjCont  
{
public:
    CIKGraphObjCont();
    virtual ~CIKGraphObjCont();
    CIKGraphNode* getNodeFromUserData0(int userData);
    CIKGraphNode* getNodeFromUserData1(int userData);
    CIKGraphNode* getNodeFromNodeID(int nodeID);
    CIKGraphObject* insertPassiveObjectNode(C7Vector& transformation);
    CIKGraphObject* insertTipObjectNode(const C7Vector& transformation,const C7Vector& targetTransformation);
    CIKGraphJoint* insertRevoluteJointNode(C7Vector& transformation,float parameter,float minVal,float range,float screwPitch,bool isCyclic,float weight);
    CIKGraphJoint* insertPrismaticJointNode(C7Vector& transformation,float parameter,float minVal,float range,float weight);
    CIKGraphJoint* insertBallJointNode(C7Vector& transformation,C4Vector sphericalTransformation,float range,float weight);
    void resetExplorationIDs();
    int identifyElements();
    void putElementsInPlace();
    void replaceElementIDWithAnother(int oldID,int newID);
    void getLinkObjectsWithElementID(int elementID,std::vector<CIKGraphObject*>& links);
    void preMultiplyObjectsWithElementID(int elementID,C7Vector& transform);
    void actualizeAllTransformations();
    void setBaseObject(int nodeID);
    CIKGraphObject* getBaseObject();

    int baseObjectID;
    int nodeIDCounter;
    int numberOfElementIDs;
    void emptyContainer();

    std::vector<CIKGraphNode*> container;

private:
    void insertNode(CIKGraphNode* aNode);
    void actualizeTransformationsWithElementID(int elementID);
};
