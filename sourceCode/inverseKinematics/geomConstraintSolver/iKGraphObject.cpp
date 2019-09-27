
#include "vrepMainHeader.h"
#include "iKGraphObject.h"
#include "iKGraphJoint.h"
#include "gCSDefs.h"

CIKGraphObject::CIKGraphObject(const C7Vector& cumulTransf,const C7Vector& targetCumulTransf)
{
    type=IK_GRAPH_OBJECT_TYPE;
    objectType=IK_GRAPH_TIP_OBJECT_TYPE;
    linkPartner=nullptr;
    jointTop=false;
    jointDown=false;
    cumulativeTransformation=cumulTransf;
    targetCumulativeTransformation=targetCumulTransf;
}

CIKGraphObject::CIKGraphObject(const C7Vector& cumulTransf)
{
    type=IK_GRAPH_OBJECT_TYPE;
    objectType=IK_GRAPH_PASSIVE_OBJECT_TYPE;
    linkPartner=nullptr;
    jointTop=false;
    jointDown=false;
    cumulativeTransformation=cumulTransf;
}

CIKGraphObject::~CIKGraphObject()
{

}

CIKGraphNode* CIKGraphObject::getUnexplored(int pos)
{
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]->explorationID==-1)
        {
            if (pos==0)
                return(neighbours[i]);
            pos--;
        }
    }
    if (linkPartner!=nullptr)
    {
        if (linkPartner->explorationID==-1)
        {
            if (pos==0)
                return(linkPartner);
            pos--;
        }
    }
    return(nullptr);
}

CIKGraphNode* CIKGraphObject::getNeighbour(int pos,bool noLinkNeighbour)
{
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (pos==0)
            return(neighbours[i]);
        pos--;
    }
    if ( (linkPartner!=nullptr)&&(!noLinkNeighbour) )
    {
        if (pos==0)
            return(linkPartner);
        pos--;
    }
    return(nullptr);
}

int CIKGraphObject::getNumberOfUnexplored()
{   
    int retVal=0;
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]->explorationID==-1)
            retVal++;
    }
    if (linkPartner!=nullptr)
    {
        if (linkPartner->explorationID==-1)
            retVal++;
    }
    return(retVal);
}

CIKGraphNode* CIKGraphObject::getNeighbourWithExplorationID(int theID)
{
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]->explorationID==theID)
            return(neighbours[i]);
    }
    if (linkPartner!=nullptr)
    {
        if (linkPartner->explorationID==theID)
            return(linkPartner);
    }
    return(nullptr);
}


CIKGraphNode* CIKGraphObject::getExploredWithSmallestExplorationID()
{
    int smallest=999999;
    CIKGraphNode* retVal=nullptr;
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]->explorationID!=-1)
        {
            if (neighbours[i]->explorationID<smallest)
            {
                smallest=neighbours[i]->explorationID;
                retVal=neighbours[i];
            }
        }
    }
    if (linkPartner!=nullptr)
    {
        if (linkPartner->explorationID!=-1)
        {
            if (linkPartner->explorationID<smallest)
            {
                smallest=linkPartner->explorationID;
                retVal=linkPartner;
            }
        }
    }
    return(retVal);
}

int CIKGraphObject::getConnectionNumber()
{
    int retVal=0;
    retVal+=int(neighbours.size());
    if (linkPartner!=nullptr)
        retVal++;
    return(retVal);
}

bool CIKGraphObject::linkWithObject(CIKGraphObject* partner)
{
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]==partner)
            return(true);
    }
    if (linkPartner==partner)
        return(false);
    neighbours.push_back(partner);
    partner->neighbours.push_back(this);
    return(true);
}

bool CIKGraphObject::elasticLinkWithObject(CIKGraphObject* partner)
{
    for (int i=0;i<int(neighbours.size());i++)
    {
        if (neighbours[i]==partner)
            return(false);
    }
    if (linkPartner==partner)
        return(true);
    if (linkPartner!=nullptr)
        return(false);
    linkPartner=partner;
    partner->linkPartner=this;
    objectType=IK_GRAPH_LINK_OBJECT_TYPE;
    partner->objectType=IK_GRAPH_LINK_OBJECT_TYPE;
    return(true);
}

bool CIKGraphObject::linkWithJoint(CIKGraphJoint* joint,bool top)
{
    if (top)
    {
        if (joint->topObject==this)
            return(true);
        if (joint->topObject!=nullptr)
            return(false);
        if (joint->downObject==this)
            return(false);
        joint->topObject=this;
        neighbours.push_back(joint);
        jointTop=true;
        jointDown=false;
        return(true);
    }
    else
    {
        if (joint->downObject==this)
            return(true);
        if (joint->downObject!=nullptr)
            return(false);
        if (joint->topObject==this)
            return(false);
        joint->downObject=this;
        neighbours.push_back(joint);
        jointTop=false;
        jointDown=true;
        return(true);
    }
}
