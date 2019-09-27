
#include "vrepMainHeader.h"
#include "geometricConstraintSolver.h"
#include "iKJoint.h"
#include "iKObjCont.h"
#include "iKGraphObject.h"
#include "iKGraphJoint.h"
#include "iKGraphObjCont.h"
#include "gCSDefs.h"

bool CGeometricConstraintSolver::solve(CIKGraphObjCont& graphContainer,SGeomConstrSolverParam& parameters)
{
    if (graphContainer.identifyElements()==0)
        return(false); // Nothing to solve (no active joint in the mechanism)
    graphContainer.putElementsInPlace();

// We create a branched tree, where each extremity has a tip dummy
// (which will later be constrained to its respective target dummy)
    
    CIKGraphObject* baseIKGraphObject=graphContainer.getBaseObject();
    CIKGraphNode* graphIterator=baseIKGraphObject;
    CIKGraphNode* previousPosition=nullptr;
    CIKGraphNode* nextPosition=nullptr;
    C7Vector localTransformation;
    localTransformation.setIdentity();
    CIKObjCont ikObjs;
    CIKJoint* lastJoint=nullptr;
    CIKJoint* treeHandle=nullptr;

// Some precalculations of some fixed rotations:
    C4X4Matrix tmpRot;
    tmpRot.setIdentity();
    tmpRot.M(0,0)=-1.0f;
    tmpRot.M(2,2)=-1.0f;
    C7Vector rotY180(tmpRot.getTransformation());
    tmpRot.M.clear();
    tmpRot.M(0,0)=1.0f;
    tmpRot.M(2,1)=1.0f;
    tmpRot.M(1,2)=-1.0f;
    C7Vector rotX90(tmpRot.getTransformation().Q,C3Vector(0.0f,0.0f,0.0f));
    tmpRot.M.clear();
    tmpRot.M(2,0)=-1.0f;
    tmpRot.M(1,1)=1.0f;
    tmpRot.M(0,2)=1.0f;
    C7Vector rotY90(tmpRot.getTransformation().Q,C3Vector(0.0f,0.0f,0.0f));
    tmpRot.M.clear();
    tmpRot.M(1,0)=1.0f;
    tmpRot.M(0,1)=-1.0f;
    tmpRot.M(2,2)=1.0f;
    C7Vector rotZ90(tmpRot.getTransformation().Q,C3Vector(0.0f,0.0f,0.0f));
    

    std::vector<CIKGraphNode*> graphObjectsToBeExplored;
    graphObjectsToBeExplored.push_back(baseIKGraphObject);
    std::vector<CIKJoint*> lastJoints;
    lastJoints.push_back(nullptr);
    std::vector<CIKGraphNode*> previousPositions;
    previousPositions.push_back(nullptr);
    std::vector<C7Vector> localTransformations;
    localTransformations.push_back(localTransformation);

    int explorationID=0;
    while (graphObjectsToBeExplored.size()!=0)
    {
        graphIterator=graphObjectsToBeExplored.back();
        graphObjectsToBeExplored.pop_back();
        lastJoint=lastJoints.back();
        lastJoints.pop_back();
        previousPosition=previousPositions.back();
        previousPositions.pop_back();
        localTransformation=localTransformations.back();
        localTransformations.pop_back();
        bool doIt=(graphIterator->explorationID==-1);
        bool goingDown=false;
        bool closeComplexLoop=false;
        while (doIt)
        {
            if (graphIterator->explorationID==-1)
                graphIterator->explorationID=explorationID;
            explorationID++;
            C7Vector previousCT;
            if (previousPosition!=nullptr)
            {
                if (previousPosition->type==IK_GRAPH_JOINT_TYPE)
                    previousCT=((CIKGraphObject*)graphIterator)->cumulativeTransformation;
                else
                    previousCT=((CIKGraphObject*)previousPosition)->cumulativeTransformation;
            }
            else
            {
                previousCT=baseIKGraphObject->cumulativeTransformation;
                localTransformation=previousCT;
            }

            if (graphIterator->type==IK_GRAPH_JOINT_TYPE)
            { // Joint: we have to introduce a joint
                CIKGraphJoint* graphJoint=(CIKGraphJoint*)graphIterator;

                if (!graphJoint->disabled)
                {
                    C7Vector sphTr;
                    sphTr.setIdentity();
                    sphTr.Q=graphJoint->sphericalTransformation;
                    CIKJoint* newIKJoint;
                    if (graphJoint->jointType==IK_GRAPH_SPHERICAL_JOINT_TYPE)
                    {
                        int dataValueBase=10*graphJoint->nodeID;
                        CIKJoint* avatarParent;
                        if (graphJoint->topObject==(CIKGraphObject*)previousPosition)
                        { // From tip to base
                            C7Vector rel(localTransformation*rotY180);
                            newIKJoint=new CIKJoint(graphJoint,rel,false,false);
                            if (lastJoint==nullptr)
                            {
                                treeHandle=newIKJoint;
                                lastJoint=treeHandle;
                                ikObjs.addRoot(lastJoint);
                            }
                            else
                            {
                                ikObjs.addChild(lastJoint,newIKJoint);
                                lastJoint=newIKJoint;
                            }
                            avatarParent=ikObjs.getJointWithData(dataValueBase+3);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+3;
                        
                            rel=rotX90;
                            newIKJoint=new CIKJoint(graphJoint,rel,false,false);
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                            avatarParent=ikObjs.getJointWithData(dataValueBase+2);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+2;    

                            rel=rotY90;
                            newIKJoint=new CIKJoint(graphJoint,rel,false,false);
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                            avatarParent=ikObjs.getJointWithData(dataValueBase+1);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+1;
                            
                                rel=rotX90*rotZ90.getInverse()*sphTr.getInverse()*rotY180;
                                newIKJoint=new CIKJoint(graphJoint,rel,true,false);
                                lastJoint->topJoint=newIKJoint; // This is mainly needed by the joint-limitation part!
                                ikObjs.addChild(lastJoint,newIKJoint);
                                lastJoint=newIKJoint;
                                lastJoint->active=false; // Inactive for now (we can activate it later)
                                avatarParent=ikObjs.getJointWithData(dataValueBase+0);
                                if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                    avatarParent->addAvatar(lastJoint);
                                lastJoint->data=dataValueBase+0;
                                localTransformation=rotY180;
                        }
                        else
                        { // From base to tip
                            C7Vector rel(localTransformation);
                            newIKJoint=new CIKJoint(graphJoint,rel,false,true);
                            if (lastJoint==nullptr)
                            {
                                treeHandle=newIKJoint;
                                lastJoint=treeHandle;
                                ikObjs.addRoot(lastJoint);
                            }
                            else
                            {
                                ikObjs.addChild(lastJoint,newIKJoint);
                                lastJoint=newIKJoint;
                            }
                            lastJoint->active=false; // Inactive for now (we can activate it later)
                            avatarParent=ikObjs.getJointWithData(dataValueBase+0);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+0;

                            rel=sphTr*rotY90;
                            newIKJoint=new CIKJoint(graphJoint,rel,false,true);
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                            avatarParent=ikObjs.getJointWithData(dataValueBase+1);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+1;
                            
                            rel=rotX90.getInverse();
                            newIKJoint=new CIKJoint(graphJoint,rel,false,true);
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                            avatarParent=ikObjs.getJointWithData(dataValueBase+2);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+2;

                            rel=rotY90.getInverse()*rotZ90.getInverse();
                            newIKJoint=new CIKJoint(graphJoint,rel,true,true);
                            newIKJoint->topJoint=newIKJoint; // Top-joint is itself!
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                            avatarParent=ikObjs.getJointWithData(dataValueBase+3);
                            if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                                avatarParent->addAvatar(lastJoint);
                            lastJoint->data=dataValueBase+3;

                            localTransformation.setIdentity();
                        }
                    }
                    else
                    {
                        if (graphJoint->topObject==(CIKGraphObject*)previousPosition)
                        { // From tip to base
                            C7Vector rel(localTransformation*rotY180);
                            newIKJoint=new CIKJoint(graphJoint,rel,false,false);
                            localTransformation=rotY180;
                        }
                        else
                        { // From base to tip
                            C7Vector rel(localTransformation);
                            newIKJoint=new CIKJoint(graphJoint,rel,false,false);
                            localTransformation.setIdentity();
                        }
                        if (lastJoint==nullptr)
                        {
                            treeHandle=newIKJoint;
                            lastJoint=treeHandle;
                            ikObjs.addRoot(lastJoint);
                        }
                        else
                        {
                            ikObjs.addChild(lastJoint,newIKJoint);
                            lastJoint=newIKJoint;
                        }
                        int dataValue=10*graphJoint->nodeID+0;
                        CIKJoint* avatarParent=ikObjs.getJointWithData(dataValue);
                        if (avatarParent!=nullptr) // This joint is used twice (going up and going down)
                            avatarParent->addAvatar(lastJoint);
                        lastJoint->data=dataValue;
                    }
                }
                else
                { // In case a graph-joint is disabled: 
                    if (graphJoint->topObject==(CIKGraphObject*)previousPosition)
                    { // From tip to base
                        localTransformation=localTransformation*graphJoint->getDownToTopTransformation().getInverse();
                    }
                    else
                    { // From base to tip
                        localTransformation=localTransformation*graphJoint->getDownToTopTransformation();
                    }
                }
            }
            else
            {
                CIKGraphObject* theObject=(CIKGraphObject*)graphIterator;
                if (theObject->objectType==IK_GRAPH_LINK_OBJECT_TYPE)
                { // Link
                    if (previousPosition!=nullptr)
                    {
                        if (theObject->linkPartner!=previousPosition)
                            localTransformation=localTransformation*previousCT.getInverse()*theObject->cumulativeTransformation;
                        // If (theObject->linkPartner==previousPosition) then we don't do anything!
                    }
                }
                else
                { // Here we have a dummy we have to assign to a configuration or a passive object
                    // We treat all cases first as passive objects:
                    if (previousPosition!=nullptr)
                    {
                        localTransformation=localTransformation*previousCT.getInverse()*theObject->cumulativeTransformation;
                        if ( (theObject->objectType==IK_GRAPH_TIP_OBJECT_TYPE)&&(lastJoint!=nullptr) )
                        { // This is a valid dummy-tip!
                            CIKDummy* newIKDummy=new CIKDummy(localTransformation,theObject->targetCumulativeTransformation);
                            ikObjs.addChild(lastJoint,newIKDummy);
                            newIKDummy->constraints=(IK_X_CONSTRAINT|IK_Y_CONSTRAINT|IK_Z_CONSTRAINT);
                            newIKDummy->dampingFactor=1.0f;
                            newIKDummy->loopClosureDummy=false;
                            if (graphIterator->getConnectionNumber()==1)
                                break;
                        }
                    }
                }
            }
            int unexploredSize=graphIterator->getNumberOfUnexplored();
            if ( (unexploredSize==0)||goingDown||closeComplexLoop )
            {
                if ( (graphIterator->getConnectionNumber()==1)&&(!closeComplexLoop) )
                    break; // This is a rare case where we have an endpoint without a tip-dummy mobile-part
                if (closeComplexLoop)
                {
                    CIKDummy* tipDummy=new CIKDummy(localTransformation,baseIKGraphObject->cumulativeTransformation);
                    ikObjs.addChild(lastJoint,tipDummy);
                    break;
                }
                nextPosition=graphIterator->getExploredWithSmallestExplorationID();
                if ( (nextPosition->explorationID==0)&&(!goingDown) )
                { // The loop can now be closed (simple loop with each joint present at most once)
                    previousCT=((CIKGraphObject*)graphIterator)->cumulativeTransformation;
                    localTransformation=localTransformation*previousCT.getInverse()*((CIKGraphObject*)nextPosition)->cumulativeTransformation;
                    CIKDummy* tipDummy=new CIKDummy(localTransformation,baseIKGraphObject->cumulativeTransformation);
                    ikObjs.addChild(lastJoint,tipDummy);
                    break;
                }
                if ( (nextPosition->explorationID==0)&&goingDown )
                    closeComplexLoop=true;
                goingDown=true;
            }
            else if ((graphIterator->getNeighbourWithExplorationID(0)!=nullptr)&&(!goingDown)&&(previousPosition->explorationID!=0))
            { // Here we have to close the loop too!
                // We first put unexplored paths onto the stack:
                for (int i=0;i<unexploredSize;i++)
                { // We throw unexplored nodes onto the exploration stack:
                    graphObjectsToBeExplored.push_back(graphIterator->getUnexplored(i));
                    lastJoints.push_back(lastJoint);
                    previousPositions.push_back(graphIterator);
                    localTransformations.push_back(localTransformation);
                }
                nextPosition=graphIterator->getExploredWithSmallestExplorationID();
                previousCT=((CIKGraphObject*)previousPosition)->cumulativeTransformation;
                localTransformation=localTransformation*previousCT.getInverse()*((CIKGraphObject*)nextPosition)->cumulativeTransformation;
                CIKDummy* tipDummy=new CIKDummy(localTransformation,baseIKGraphObject->cumulativeTransformation);
                ikObjs.addChild(lastJoint,tipDummy);
                break;
            }
            else
            {
                if (previousPosition==nullptr)
                { // This is the start. We should always explore first two links which belong together
                    // or the 3 objects making up a joint!
                    nextPosition=nullptr;
                    for (int i=0;i<unexploredSize;i++)
                    {
                        CIKGraphNode* nextPositionTmp=graphIterator->getUnexplored(i);
                        if ( (((CIKGraphObject*)graphIterator)->linkPartner==nextPositionTmp)||
                            (nextPositionTmp->type==IK_GRAPH_JOINT_TYPE) )
                            nextPosition=nextPositionTmp;
                        else
                        {
                            graphObjectsToBeExplored.push_back(graphIterator->getUnexplored(i));
                            lastJoints.push_back(lastJoint);
                            previousPositions.push_back(graphIterator);
                            localTransformations.push_back(localTransformation);
                            if (nextPosition==nullptr)
                                nextPosition=graphIterator->getUnexplored(i);
                        }
                    }
                }
                else
                {   
                    nextPosition=graphIterator->getUnexplored(0);
                    for (int i=1;i<unexploredSize;i++)
                    { // We throw unexplored nodes onto the exploration stack:
                        graphObjectsToBeExplored.push_back(graphIterator->getUnexplored(i));
                        lastJoints.push_back(lastJoint);
                        previousPositions.push_back(graphIterator);
                        localTransformations.push_back(localTransformation);
                    }
                }
            }
            previousPosition=graphIterator;
            graphIterator=nextPosition;
        }

    }

    solveHierarchy(&ikObjs,parameters);

    for (int i=0;i<int(ikObjs.allObjects.size());i++)
    {
        CIKObject* it=ikObjs.allObjects[i];
        if (it->objectType==IK_JOINT_TYPE)
        {
            CIKJoint* theJoint=(CIKJoint*)it;
            if (theJoint->avatarParent==nullptr)
            {
                if (theJoint->spherical)
                {
                    if (theJoint->topSpherical)
                    {
                        float a0=theJoint->parameter;
                        float a1=((CIKJoint*)theJoint->parent)->parameter;
                        float a2=((CIKJoint*)theJoint->parent->parent)->parameter;
                        float a3=((CIKJoint*)theJoint->parent->parent->parent)->parameter;
                        if (theJoint->sphericalUp)
                        {
                            theJoint->graphJoint->sphericalTransformation=C4Vector(a3,C3Vector(0.0f,0.0f,1.0f))*theJoint->graphJoint->sphericalTransformation*C4Vector(C3Vector(a2,a1,a0));
                        }
                        else
                        {
                            theJoint->graphJoint->sphericalTransformation=C4Vector(a0,C3Vector(0.0f,0.0f,1.0f))*theJoint->graphJoint->sphericalTransformation*C4Vector(C3Vector(a1,a2,a3));
                        }
                    }
                }
                else
                    theJoint->graphJoint->parameter=theJoint->parameter;
            }
        }
    }
    graphContainer.actualizeAllTransformations();
    graphContainer.putElementsInPlace();
    return(true);
}

bool CGeometricConstraintSolver::solveHierarchy(CIKObjCont* objects,SGeomConstrSolverParam& parameters)
{
    // We prepare all joint temporary parameters and
    // We prepare a tip-container (with all tip-dummies which will be assigned
    // target-dummies later)
    std::vector<CIKDummy*> tipContainer;
    for (int i=0;i<int(objects->allObjects.size());i++)
    {
        CIKObject* it=objects->allObjects[i];
        if (it->objectType==IK_JOINT_TYPE)
            ((CIKJoint*)it)->tempParameter=((CIKJoint*)it)->parameter;
        else
        {
            tipContainer.push_back((CIKDummy*)it);
            ((CIKDummy*)it)->computeChainSize();
        }
    }

    // We order the loops from smallest to biggest:
    // The following is a bubble sort (very slow but fast enough here: rarely more than 4 loops!)
    CIKDummy* tmp;
    if (tipContainer.size()!=0)
    {
        for (int i=0;i<int(tipContainer.size())-1;i++)
        {
            for (int j=0;j<int(tipContainer.size())-1-i;j++)
            {
                if (tipContainer[j]->chainSize>tipContainer[j+1]->chainSize)
                { // We swap the two elements:
                    tmp=tipContainer[j];
                    tipContainer[j]=tipContainer[j+1];
                    tipContainer[j+1]=tmp;
                }
            }
        }
    }

//  /* // To test solving of spaguetti (broken mechanisms)
    int maxIterations=parameters.maxIterations;
    /*bool result=*/performMainIterationLoop(tipContainer,maxIterations,parameters.interpolation,parameters);
//  if (!result)
//      result=tryIncrementalLoopSolve(tipContainer,parameters); // Solving of broken mechanisms
//  while ( (maxIterations>0)&&(!areLoopsClosedWithinTolerance(tipContainer,parameters)) )
//      result=performMainIterationLoop(tipContainer,maxIterations,parameters.interpolation,parameters);
//*/

    // We copy the temporary parameters back (according to the result)
    for (int i=0;i<int(objects->allObjects.size());i++)
    {
        CIKObject* it=objects->allObjects[i];
        if (it->objectType==IK_JOINT_TYPE)
            ((CIKJoint*)it)->parameter=((CIKJoint*)it)->tempParameter;
    }
    return(true);
}

bool CGeometricConstraintSolver::tryIncrementalLoopSolve(std::vector<CIKDummy*>& tipContainer,SGeomConstrSolverParam& parameters)
{
    for (int i=0;i<int(tipContainer.size());i++)
    {
        std::vector<CIKDummy*> auxTipContainer;
        getSmallestLoops(tipContainer,auxTipContainer,i+1);
        const int subDiv=3;
        for (int j=0;j<subDiv;j++)
        {
            int maxIter=2;
            performMainIterationLoop(tipContainer,maxIter,1.0f/((float)(subDiv-j)),parameters);
        }
    }
    int maxIter=2;
    bool result=performMainIterationLoop(tipContainer,maxIter,1.0f,parameters);
    return(result);
}

bool CGeometricConstraintSolver::performMainIterationLoop(std::vector<CIKDummy*>& tipContainer,int& maxIterations,float interpolation,SGeomConstrSolverParam& parameters)
{   // return value true: temp values can be set (joint limitations and max. joint variations respected)

    bool limitOrAvoidanceNeedMoreCalculation;
    float returnedInterpolFact;
    CIKChainCont chainCont(tipContainer,interpolation,0);
    int result=performOnePass(chainCont,limitOrAvoidanceNeedMoreCalculation,interpolation,returnedInterpolFact,parameters);
    maxIterations--;
    if (maxIterations<=0)
        return(result==0);
    if (result==0)
        return(true);
    if ((result&2)!=0)
    { // Some joint variations are not within tolerance
        if (performMainIterationLoop(tipContainer,maxIterations,interpolation/2.0f,parameters))
            return(performMainIterationLoop(tipContainer,maxIterations,interpolation,parameters));
        else
            return(false);
    }
    if ((result&1)!=0)
    { // Some joint limits were reached.
        return(performMainIterationLoop(tipContainer,maxIterations,interpolation,parameters));
    }
    return(false);
}

int CGeometricConstraintSolver::performOnePass(CIKChainCont& chainCont,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,float& nextInterpol,SGeomConstrSolverParam& parameters)
{   // Return value is bit-coded:
    // bit0 set: at least one joint limitation was not respected
    // bit1 set: max. angular or linear variations not respected.
    // bit2 set: more than one joint limitation was not respected
    // If return value is different from 0, the joints temp values are not actualized
    // Here we have the multi-ik solving algorithm:
    //********************************************************************************
    limitOrAvoidanceNeedMoreCalculation=false;
    // We prepare a vector of all used joints and a counter for the number of rows:
    std::vector<CIKJoint*> allJoints;
    int numberOfRows=0;
    for (int elementNumber=0;elementNumber<int(chainCont.allChains.size());elementNumber++)
    {
        CIKChain* element=chainCont.allChains[elementNumber];
        numberOfRows=numberOfRows+element->matrix->rows;
        for (int i=0;i<int(element->rowJoints.size());i++)
        {
            CIKJoint* current=element->rowJoints[i];
            // We check if that joint is already present:
            bool present=false;
            for (int j=0;j<int(allJoints.size());j++)
            {
                if (allJoints[j]==current)
                {
                    present=true;
                    break;
                }
            }
            if (!present)
                allJoints.push_back(current);
        }
    }

    // Now we prepare the individual joint constraints part: (part1)
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        if (allJoints[i]->graphJoint->followedJoint!=nullptr)
            numberOfRows++;
    }
    //---------------------------------------------------------------------------


    // We prepare the main matrix and the main error vector.
    CMatrix mainMatrix(numberOfRows,int(allJoints.size()));
    // We have to zero it first:
    mainMatrix.clear();
    CMatrix mainErrorVector(numberOfRows,1);
    
    // Now we fill in the main matrix and the main error vector:
    int currentRow=0;
    for (int elementNumber=0;elementNumber<int(chainCont.allChains.size());elementNumber++)
    {
        CIKChain* element=chainCont.allChains[elementNumber];
        for (int i=0;i<element->errorVector->rows;i++)
        { // We go through the rows:
            // We first set the error part:
            mainErrorVector(currentRow,0)=(*element->errorVector)(i,0);
            // Now we set the delta-parts:
            for (int j=0;j<element->matrix->cols;j++)
            { // We go through the columns:
                // We search for the right entry
                CIKJoint* thisJoint=element->rowJoints[j];
                int index=0;
                while (allJoints[index]!=thisJoint)
                    index++;
                mainMatrix(currentRow,index)=(*element->matrix)(i,j);
            }
            currentRow++;
        }
    }

    // Now we prepare the individual joint constraints part: (part2)
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        CIKGraphJoint* originalGraphJoint=allJoints[i]->graphJoint;
        CIKGraphJoint* dependenceGraphJoint=originalGraphJoint->followedJoint;
        if (dependenceGraphJoint!=nullptr)
        {
            bool found=false;
            int j;
            for (j=0;j<int(allJoints.size());j++)
            {
                if (allJoints[j]->graphJoint==dependenceGraphJoint)
                {
                    found=true;
                    break;
                }
            }
            if (found)
            { 
                float coeff=originalGraphJoint->coefficientValue;
                float fact=originalGraphJoint->constantValue;
                mainErrorVector(currentRow,0)=(allJoints[i]->tempParameter-fact-coeff*allJoints[j]->tempParameter)*interpolFact;
                mainMatrix(currentRow,i)=-1.0f;
                mainMatrix(currentRow,j)=coeff;
            }
            else
            {   // joint of dependenceID is not part of this group calculation:
                // therefore we take its current value
                    float coeff=originalGraphJoint->coefficientValue;
                    float fact=originalGraphJoint->constantValue;
                    mainErrorVector(currentRow,0)=(allJoints[i]->tempParameter-fact-coeff*dependenceGraphJoint->parameter)*interpolFact;
                    mainMatrix(currentRow,i)=-1.0f;
            }
            currentRow++;
        }
    }
    //---------------------------------------------------------------------------

    // We take the joint weights into account here (part1):
    for (int i=0;i<mainMatrix.rows;i++)
    {
        for (int j=0;j<int(allJoints.size());j++)
        {
            float coeff=allJoints[j]->weight;
            if (coeff>=0.0f)
                coeff=sqrtf(coeff);
            else
                coeff=-sqrtf(-coeff);
            mainMatrix(i,j)=mainMatrix(i,j)*coeff;
        }
    }
    // Now we just have to solve:
    int doF=mainMatrix.cols;
    int eqNumb=mainMatrix.rows;
    CMatrix solution(doF,1);

//************************************** RESOLUTION ***************************************
    CMatrix JT(mainMatrix.rows,mainMatrix.cols);
    JT=mainMatrix;
    JT.transpose();
    CMatrix DLSJ(doF,eqNumb);
    CMatrix JJTInv(eqNumb,eqNumb);
    JJTInv=mainMatrix*JT;
    CMatrix ID(mainMatrix.rows,mainMatrix.rows);
    ID.setIdentity();
    for (int i=0;i<ID.rows;i++)
        ID(i,i)=0.0f;
    int rowPos=0;
    for (int elementNumber=0;elementNumber<int(chainCont.allChains.size());elementNumber++)
    {
        CIKChain* element=chainCont.allChains[elementNumber];
        for (int i=0;i<element->errorVector->rows;i++)
        {
            float damping=element->tooltip->dampingFactor+parameters.generalDamping;
            ID(rowPos,rowPos)=damping*damping;
            rowPos++;
        }
    }
    JJTInv+=ID;
    if (!JJTInv.inverse())
        return(false); // error occured (matrix not invertible, nan numbers or such!)
    DLSJ=JT*JJTInv;
    solution=DLSJ*mainErrorVector;
//*****************************************************************************************



    // We take the joint weights into account here (part2) and prepare the probable delta-values:
    for (int i=0;i<doF;i++)
    {
        float coeff=sqrtf(fabs(allJoints[i]->weight));
        solution(i,0)=solution(i,0)*coeff;
        allJoints[i]->probableDeltaValue=solution(i,0);
    }

    // We check if some variations are too big:
    int returnValue=0;
    int lockJointNb=-1;
    for (int i=0;i<doF;i++)
    {
        CIKJoint* it=allJoints[i];
        if (it->revolute)
        {
            if (fabs(it->probableDeltaValue)>parameters.maxAngularVariation)
                returnValue=returnValue|2;
        }
        else
        {
            if (fabs(it->probableDeltaValue)>parameters.maxLinearVariation)
                returnValue=returnValue|2;
        }
        // ******************** This is for joint limitation *********************
        bool doIt=true;
        if (it->spherical)
        {
            if (it->topJoint!=nullptr)
                it=it->topJoint;
            else
                doIt=false;
        }
        if (doIt)
        {
            float overV=it->getValueOverLimitation(false);
            if (overV>-0.0001f)
            {
                if (overV>0.99f)
                    overV=fabs(1000000.0f*it->probableDeltaValue)+1.0f;
                if ((returnValue&1)!=0)
                {
                    if (overV>nextInterpol)
                    {
                        nextInterpol=overV;
                        lockJointNb=i;
                    }
                    returnValue|=4;
                }
                else
                {
                    nextInterpol=overV;
                    returnValue|=1;
                    lockJointNb=i;
                }
            }
        }
        // *************************************************************************
    }


    if (lockJointNb!=-1)
    {
        CIKJoint* it=allJoints[lockJointNb];
        if (it->spherical)
        {
            if (it->topJoint!=nullptr)
                it=it->topJoint;
        }

        it->getValueOverLimitation(true);
        // Now we have to lock all joints which are linked to that one through a
        // linear equation:
        std::vector<CIKJoint*> lockDependent;
        lockDependent.push_back(it);
        while (lockDependent.size()!=0)
        {
            it=lockDependent.back();
            lockDependent.pop_back();
            for (int i=0;i<int(allJoints.size());i++)
            {
                if ( (allJoints[i]->active)&&(allJoints[i]->graphJoint->followedJoint==it->graphJoint) )
                {
                    CIKJoint* it2=allJoints[i];
                    it2->tempParameter=it2->graphJoint->constantValue+it2->graphJoint->coefficientValue*it->tempParameter;
                    if (!it2->cyclic)
                    {
                        if (it2->tempParameter<it2->minValue)
                            it2->tempParameter=it2->minValue;
                        if (it2->tempParameter>(it2->minValue+it2->range))
                            it2->tempParameter=it2->minValue+it2->range;
                    }
                    it2->active=false;
                    it2->copyStateToAvatarKids();
                    lockDependent.push_back(it2);
                }
            }
        }
    }

    if (returnValue==0)
    { // Now we set the computed values
        for (int i=0;i<doF;i++)
        {
            CIKJoint* it=allJoints[i];
            if (it->active)
            {
                it->tempParameter+=it->probableDeltaValue;
                it->copyStateToAvatarKids();
            }
        }
    }
    return(returnValue);
}

void CGeometricConstraintSolver::getSmallestLoops(std::vector<CIKDummy*>& tipContainer,std::vector<CIKDummy*>& returnContainer,int numberOfLoops)
{
    returnContainer.clear();
    for (int i=0;i<int(tipContainer.size());i++)
    {
        if (numberOfLoops<=0)
            break;
        numberOfLoops--;
        returnContainer.push_back(tipContainer[i]);
    }
}

bool CGeometricConstraintSolver::areLoopsClosedWithinTolerance(std::vector<CIKDummy*>& tipContainer,SGeomConstrSolverParam& parameters)
{
    for (int i=0;i<int(tipContainer.size());i++)
    {
        if (tipContainer[i]->loopClosureDummy)
        {
            if (!tipContainer[i]->withinTolerance(parameters.loopClosurePositionTolerance,parameters.loopClosureOrientationTolerance))
                return(false);
        }
    }
    return(true);
}




