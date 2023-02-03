#pragma once

#include <shape.h>
#include <dummy.h>
#include <octree.h>
#include <pointCloud.h>
#include <vector>


//FULLY STATIC CLASS
class CCollisionRoutine  
{
public:
    CCollisionRoutine();
    virtual ~CCollisionRoutine();

    static bool doEntitiesCollide(int entity1ID,int entity2ID,std::vector<double>* intersections,bool overrideCollidableFlagIfObject1,bool overrideCollidableFlagIfObject2,int collidingObjectIDs[2]);

private:
    static bool _doesObjectCollideWithObject(CSceneObject* object1,CSceneObject* object2,bool overrideObject1CollidableFlag,bool overrideObject2CollidableFlag,std::vector<double>* intersections);
    static bool _doesShapeCollideWithShape(CShape* shape1,CShape* shape2,std::vector<double>* intersections,bool overrideShape1CollidableFlag,bool overrideShape2CollidableFlag);
    static bool _doesOctreeCollideWithShape(COctree* octree,CShape* shape,bool overrideOctreeCollidableFlag,bool overrideShapeCollidableFlag);
    static bool _doesOctreeCollideWithOctree(COctree* octree1,COctree* octree2,bool overrideOctree1CollidableFlag,bool overrideOctree2CollidableFlag);
    static bool _doesOctreeCollideWithPointCloud(COctree* octree,CPointCloud* pointCloud,bool overrideOctreeCollidableFlag,bool overridePointCloudCollidableFlag);
    static bool _doesOctreeCollideWithDummy(COctree* octree,CDummy* dummy,bool overrideOctreeCollidableFlag,bool overrideDummyCollidableFlag);

    static bool _doesGroupCollideWithShape(const std::vector<CSceneObject*>& group,CShape* shape,std::vector<double>* intersections,bool overrideShapeCollidableFlag,int& collidingGroupObject);
    static bool _doesGroupCollideWithOctree(const std::vector<CSceneObject*>& group,COctree* octree,bool overrideOctreeCollidableFlag,int& collidingGroupObject);
    static bool _doesGroupCollideWithDummy(const std::vector<CSceneObject*>& group,CDummy* dummy,bool overrideDummyCollidableFlag,int& collidingGroupObject);
    static bool _doesGroupCollideWithPointCloud(const std::vector<CSceneObject*>& group,CPointCloud* pointClout,bool overridePointCloudCollidableFlag,int& collidingGroupObject);

    static bool _doesGroupCollideWithItself(const std::vector<CSceneObject*>& group,std::vector<double>* intersections,int collidingGroupObjects[2]);
    static bool _doesGroupCollideWithGroup(const std::vector<CSceneObject*>& group1,const std::vector<CSceneObject*>& group2,std::vector<double>* intersections,int collidingGroupObjects[2]);

    static bool _areObjectBoundingBoxesOverlapping(CSceneObject* obj1,CSceneObject* obj2);
};
