
#include "vrepMainHeader.h"
#include "statDistObj.h"
#include "distanceRoutine.h"
#include "pluginContainer.h"

CStatDistObj::CStatDistObj(CShape* theShapeA,CShape* theShapeB)
{
    shapeA=theShapeA;
    shapeB=theShapeB;
    shapeACTM=shapeA->getCumulativeTransformation().getMatrix();
    shapeBCTM=shapeB->getCumulativeTransformation().getMatrix();
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
    float distances[7];
    distances[6]=dist;

    // We first check if the transformation matrices are valid:
    if (!shapeACTM.isValid())
        return(false);
    if (!shapeBCTM.isValid())
        return(false);

    C4X4Matrix distObjMatr[2]={shapeACTM,shapeBCTM};
    const void* collInfos[2]={shapeA->geomData->collInfo,shapeB->geomData->collInfo};
    int shapeATri=CPluginContainer::mesh_getCalculatedTriangleCount(shapeA->geomData->collInfo)*3;
    int shapeBTri=CPluginContainer::mesh_getCalculatedTriangleCount(shapeB->geomData->collInfo)*3;
    if (shapeATri<shapeBTri)
        CPluginContainer::mesh_getMeshMeshDistance(shapeA->geomData->collInfo,shapeB->geomData->collInfo,distObjMatr,collInfos,false,distances,theCaching);
    else
        CPluginContainer::mesh_getMeshMeshDistance(shapeB->geomData->collInfo,shapeA->geomData->collInfo,distObjMatr,collInfos,true,distances,theCaching);


    if (distances[6]<dist)
    {
        dist=distances[6];
        ptOnShapeA.set(distances+0);
        ptOnShapeB.set(distances+3);
        return(true);
    }
    return(false);
}
