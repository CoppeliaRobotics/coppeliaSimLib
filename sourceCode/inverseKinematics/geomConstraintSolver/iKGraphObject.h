
#pragma once

#include "MyMath.h"
#include "iKGraphNode.h"

class CIKGraphJoint; // Forward declaration

class CIKGraphObject : public CIKGraphNode
{
public:
    CIKGraphObject();
    CIKGraphObject(const C7Vector& cumulTransf,const C7Vector& targetCumulTransf);  // Tip object
    CIKGraphObject(const C7Vector& cumulTransf);                                // Passive object
    virtual ~CIKGraphObject();
    int objectType;// IK_GRAPH_LINK_OBJECT_TYPE,IK_GRAPH_TIP_OBJECT_TYPE,IK_GRAPH_PASSIVE_OBJECT_TYPE,IK_GRAPH_MESH_OBJECT_TYPE
    C7Vector cumulativeTransformation;
    std::vector<CIKGraphNode*> neighbours;

    bool linkWithObject(CIKGraphObject* partner);
    bool linkWithJoint(CIKGraphJoint* joint,bool top);
    bool elasticLinkWithObject(CIKGraphObject* partner);

    // Link-type:
    CIKGraphObject* linkPartner;
    
    // Tip-dummy:
    C7Vector targetCumulativeTransformation;

    // Passive object:
    bool jointTop;
    bool jointDown;

    CIKGraphNode* getUnexplored(int pos);
    CIKGraphNode* getNeighbour(int pos,bool noLinkNeighbour);
    CIKGraphNode* getExploredWithSmallestExplorationID();
    CIKGraphNode* getNeighbourWithExplorationID(int theID);
    int getNumberOfUnexplored();
    int getConnectionNumber();
};
