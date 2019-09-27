#pragma once

class CIKGraphNode  
{
public:
    CIKGraphNode();
    virtual ~CIKGraphNode();
    int type;// IK_GRAPH_JOINT_TYPE,IK_GRAPH_OBJECT_TYPE
    int explorationID;
    int elementID;
    int nodeID;
    int userData0;
    int userData1;
    virtual CIKGraphNode* getUnexplored(int pos);
    virtual CIKGraphNode* getNeighbour(int pos,bool noLinkNeighbour);
    virtual CIKGraphNode* getExploredWithSmallestExplorationID();
    virtual CIKGraphNode* getNeighbourWithExplorationID(int theID);
    virtual int getNumberOfUnexplored();
    virtual int getConnectionNumber();
};
