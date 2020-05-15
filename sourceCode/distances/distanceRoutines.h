
#pragma once

#include "shape.h"

struct SExtCache {
    int id;
    unsigned long long int cache;
};

struct SMovementCoherency {
    int object1Id;
    int object2Id;
    C7Vector object1Tr;
    C7Vector object2Tr;
};

class COctree;
class CPointCloud;

// FULLY STATIC CLASS
class CDistanceRoutine  
{
public:
    CDistanceRoutine();
    virtual ~CDistanceRoutine();

    static bool getDistanceBetweenEntitiesIfSmaller(int entity1ID,int entity2ID,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2);

    static bool getDistanceCachingEnabled();
    static void setDistanceCachingEnabled(bool e);

private:
    static bool _getObjectPairsDistanceIfSmaller(const std::vector<CSceneObject*>& unorderedPairs,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2);
    static bool _getObjectObjectDistanceIfSmaller(CSceneObject* object1,CSceneObject* object2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2);

    static bool _getDummyDummyDistanceIfSmaller(CDummy* dummy1,CDummy* dummy2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy1,bool overrideMeasurableFlagDummy2);
    static bool _getDummyShapeDistanceIfSmaller(CDummy* dummy,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagShape);
    static bool _getDummyOctreeDistanceIfSmaller(CDummy* dummy,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagOctree);
    static bool _getDummyPointCloudDistanceIfSmaller(CDummy* dummy,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagPointCloud);
    static bool _getShapeDummyDistanceIfSmaller(CShape* shape,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagDummy);
    static bool _getShapeShapeDistanceIfSmaller(CShape* shape1,CShape* shape2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape1,bool overrideMeasurableFlagShape2);
    static bool _getShapeOctreeDistanceIfSmaller(CShape* shape,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagOctree);
    static bool _getShapePointCloudDistanceIfSmaller(CShape* shape,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagPointCloud);
    static bool _getOctreeDummyDistanceIfSmaller(COctree* octree,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagDummy);
    static bool _getOctreeShapeDistanceIfSmaller(COctree* octree,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagShape);
    static bool _getOctreeOctreeDistanceIfSmaller(COctree* octree1,COctree* octree2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree1,bool overrideMeasurableFlagOctree2);
    static bool _getOctreePointCloudDistanceIfSmaller(COctree* octree,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagPointCloud);
    static bool _getPointCloudDummyDistanceIfSmaller(CPointCloud* pointCloud,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagDummy);
    static bool _getPointCloudShapeDistanceIfSmaller(CPointCloud* pointCloud,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagShape);
    static bool _getPointCloudOctreeDistanceIfSmaller(CPointCloud* pointCloud,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagOctree);
    static bool _getPointCloudPointCloudDistanceIfSmaller(CPointCloud* pointCloud1,CPointCloud* pointCloud2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud1,bool overrideMeasurableFlagPointCloud2);

    static float _getApproxBoundingBoxDistance(CSceneObject* obj1,CSceneObject* obj2);
    static void _copyInvertedRay(float originRay[7],float destinationRay[7]);
    static void _generateValidPairsFromObjectGroup(CSceneObject* obj,const std::vector<CSceneObject*>& group,std::vector<CSceneObject*>& pairs);
    static void _generateValidPairsFromGroupObject(const std::vector<CSceneObject*>& group,CSceneObject* obj,std::vector<CSceneObject*>& pairs);
    static void _generateValidPairsFromGroupGroup(const std::vector<CSceneObject*>& group1,const std::vector<CSceneObject*>& group2,std::vector<CSceneObject*>& pairs,bool collectionSelfDistanceCheck);
    static float _orderPairsAccordingToApproxBoundingBoxDistance(std::vector<CSceneObject*>& pairs);

    static bool _getCachedDistanceIfSmaller(float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed);
    static bool _getCachedDistanceIfSmaller_pairs(std::vector<CSceneObject*>& unorderedPairsCanBeModified,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed);

    static unsigned long long int getExtendedCacheValue(int id);
    static int insertExtendedCacheValue(unsigned long long int value);
    static bool getOctreesHaveCoherentMovement(COctree* octree1,COctree* octree2);
    static bool _distanceCachingOff;
    static std::vector<SExtCache> _extendedCacheBuffer;
    static int _nextExtendedCacheId;
    static std::vector<SMovementCoherency> _objectCoherency;
};
