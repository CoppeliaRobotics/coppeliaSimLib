#include "statDistObj.h"
#include "distanceRoutine.h"
#include "pluginContainer.h"

CStatDistObj::CStatDistObj(CShape* theShapeA,CShape* theShapeB)
{
    shapeA=theShapeA;
    shapeB=theShapeB;
    shapeACTM=shapeA->getCumulativeTransformation();
    shapeBCTM=shapeB->getCumulativeTransformation();
    shapeA->initializeCalculationStructureIfNeeded();
    shapeB->initializeCalculationStructureIfNeeded();
}

CStatDistObj::~CStatDistObj()
{

}

bool CStatDistObj::measure(float& dist,int theCaching[2])
{ // Return value true means distance (and dist also) where modified. It means that
    // the measured distance is smaller than the original 'distance' variable
    // caching can be nullptr.
    // We we start by exploring the smallest shape:

    bool retVal=false;
    if (CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeA->geomData->collInfo)<CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeB->geomData->collInfo))
        retVal=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeA->geomData->collInfo,shapeACTM,shapeB->geomData->collInfo,shapeBCTM,dist,&ptOnShapeA,&ptOnShapeB,&theCaching[0],&theCaching[1]);
    else
        retVal=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeB->geomData->collInfo,shapeBCTM,shapeA->geomData->collInfo,shapeACTM,dist,&ptOnShapeB,&ptOnShapeA,&theCaching[1],&theCaching[0]);
    return(retVal);
}
