#pragma once

#include "vVarious.h"
#include "4X4Matrix.h"
#include <vector>
#include "simTypes.h"

typedef void (__cdecl *ptr_syncPlugin_msg)(const SSyncMsg* msg,const SSyncRt* rt);

typedef  unsigned char (__cdecl *ptrStart)(void*,int);
typedef  void (__cdecl *ptrEnd)(void);
typedef  void* (__cdecl *ptrMessage)(int,int*,void*,int*);
typedef  void (__cdecl *ptrExtRenderer)(int,void*);
typedef  void (__cdecl *ptrQhull)(void*);
typedef  void (__cdecl *ptrHACD)(void*);
typedef  void (__cdecl *ptrVHACD)(void*);
typedef  void (__cdecl *ptrMeshDecimator)(void*);

typedef char (__cdecl *ptr_dynPlugin_startSimulation)(int,int,const float*,const int*);
typedef void (__cdecl *ptr_dynPlugin_endSimulation)(void);
typedef void (__cdecl *ptr_dynPlugin_step)(float,float);
typedef char (__cdecl *ptr_dynPlugin_isInitialized)(void);
typedef char (__cdecl *ptr_dynPlugin_isDynamicContentAvailable)(void);
typedef void (__cdecl *ptr_dynPlugin_serializeDynamicContent)(const char*,int);
typedef int (__cdecl *ptr_dynPlugin_addParticleObject)(int,float,float,const void*,float,int,const float*,const float*,const float*,const float*);
typedef char (__cdecl *ptr_dynPlugin_removeParticleObject)(int);
typedef char (__cdecl *ptr_dynPlugin_addParticleObjectItem)(int,const float*,float);
typedef int (__cdecl *ptr_dynPlugin_getParticleObjectOtherFloatsPerItem)(int);
typedef float* (__cdecl *ptr_dynPlugin_getContactPoints)(int*);
typedef void** (__cdecl *ptr_dynPlugin_getParticles)(int,int*,int*,float**);
typedef char (__cdecl *ptr_dynPlugin_getParticleData)(const void*,float*,float*,int*,float**);
typedef char (__cdecl *ptr_dynPlugin_getContactForce)(int,int,int,int*,float*);
typedef void (__cdecl *ptr_dynPlugin_reportDynamicWorldConfiguration)(int,char,float);
typedef int (__cdecl *ptr_dynPlugin_getDynamicStepDivider)(void);
typedef int (__cdecl *ptr_dynPlugin_getEngineInfo)(int*,int*,char*,char*);


typedef void (__cdecl *ptr_geomPlugin_releaseBuffer)(void* buff);
typedef void* (__cdecl *ptr_geomPlugin_createMesh)(const float* vertices,int verticesSize,const int* indices,int indicesSize,const float meshOrigin[7],float triangleEdgeMaxLength,int maxTrianglesInBoundingBox);
typedef void* (__cdecl *ptr_geomPlugin_copyMesh)(const void* meshObbStruct);
typedef void* (__cdecl *ptr_geomPlugin_getMeshFromSerializationData)(const unsigned char* serializationData);
typedef unsigned char* (__cdecl *ptr_geomPlugin_getMeshSerializationData)(const void* meshObbStruct,int* serializationDataSize);
typedef void (__cdecl *ptr_geomPlugin_scaleMesh)(void* meshObbStruct,float scalingFactor);
typedef void (__cdecl *ptr_geomPlugin_destroyMesh)(void* meshObbStruct);
typedef float (__cdecl *ptr_geomPlugin_getMeshRootObbVolume)(const void* meshObbStruct);
typedef void* (__cdecl *ptr_geomPlugin_createOctreeFromPoints)(const float* points,int pointCnt,const float octreeOrigin[7],float cellS,const unsigned char rgbData[3],unsigned int usrData);
typedef void* (__cdecl *ptr_geomPlugin_createOctreeFromColorPoints)(const float* points,int pointCnt,const float octreeOrigin[7],float cellS,const unsigned char* rgbData,const unsigned int* usrData);
typedef void* (__cdecl *ptr_geomPlugin_createOctreeFromMesh)(const void* meshObbStruct,const float meshTransformation[7],const float octreeOrigin[7],float cellS,const unsigned char rgbData[3],unsigned int usrData);
typedef void* (__cdecl *ptr_geomPlugin_createOctreeFromOctree)(const void* otherOctreeStruct,const float otherOctreeTransformation[7],const float newOctreeOrigin[7],float newOctreeCellS,const unsigned char rgbData[3],unsigned int usrData);
typedef void* (__cdecl *ptr_geomPlugin_copyOctree)(const void* ocStruct);
typedef void* (__cdecl *ptr_geomPlugin_getOctreeFromSerializationData)(const unsigned char* serializationData);
typedef unsigned char* (__cdecl *ptr_geomPlugin_getOctreeSerializationData)(const void* ocStruct,int* serializationDataSize);
typedef void (__cdecl *ptr_geomPlugin_scaleOctree)(void* ocStruct,float f);
typedef void (__cdecl *ptr_geomPlugin_destroyOctree)(void* ocStruct);
typedef float* (__cdecl *ptr_geomPlugin_getOctreeVoxelData)(const void* ocStruct,int* voxelCount);
typedef unsigned int* (__cdecl *ptr_geomPlugin_getOctreeUserData)(const void* ocStruct,int* voxelCount);
typedef float* (__cdecl *ptr_geomPlugin_getOctreeCornersFromOctree)(const void* ocStruct,int* pointCount);
typedef void (__cdecl *ptr_geomPlugin_insertPointsIntoOctree)(void* ocStruct,const float octreeTransformation[7],const float* points,int pointCnt,const unsigned char rgbData[3],unsigned int usrData);
typedef void (__cdecl *ptr_geomPlugin_insertColorPointsIntoOctree)(void* ocStruct,const float octreeTransformation[7],const float* points,int pointCnt,const unsigned char* rgbData,const unsigned int* usrData);
typedef void (__cdecl *ptr_geomPlugin_insertMeshIntoOctree)(void* ocStruct,const float octreeTransformation[7],const void* obbStruct,const float meshTransformation[7],const unsigned char rgbData[3],unsigned int usrData);
typedef void (__cdecl *ptr_geomPlugin_insertOctreeIntoOctree)(void* oc1Struct,const float octree1Transformation[7],const void* oc2Struct,const float octree2Transformation[7],const unsigned char rgbData[3],unsigned int usrData);
typedef bool (__cdecl *ptr_geomPlugin_removePointsFromOctree)(void* ocStruct,const float octreeTransformation[7],const float* points,int pointCnt);
typedef bool (__cdecl *ptr_geomPlugin_removeMeshFromOctree)(void* ocStruct,const float octreeTransformation[7],const void* obbStruct,const float meshTransformation[7]);
typedef bool (__cdecl *ptr_geomPlugin_removeOctreeFromOctree)(void* oc1Struct,const float octree1Transformation[7],const void* oc2Struct,const float octree2Transformation[7]);
typedef void* (__cdecl *ptr_geomPlugin_createPtcloudFromPoints)(const float* points,int pointCnt,const float ptcloudOrigin[7],float cellS,int maxPointCnt,const unsigned char rgbData[3],float proximityTol);
typedef void* (__cdecl *ptr_geomPlugin_createPtcloudFromColorPoints)(const float* points,int pointCnt,const float ptcloudOrigin[7],float cellS,int maxPointCnt,const unsigned char* rgbData,float proximityTol);
typedef void* (__cdecl *ptr_geomPlugin_copyPtcloud)(const void* pcStruct);
typedef void* (__cdecl *ptr_geomPlugin_getPtcloudFromSerializationData)(const unsigned char* serializationData);
typedef unsigned char* (__cdecl *ptr_geomPlugin_getPtcloudSerializationData)(const void* pcStruct,int* serializationDataSize);
typedef void (__cdecl *ptr_geomPlugin_scalePtcloud)(void* pcStruct,float f);
typedef void (__cdecl *ptr_geomPlugin_destroyPtcloud)(void* pcStruct);
typedef float* (__cdecl *ptr_geomPlugin_getPtcloudPoints)(const void* pcStruct,int* pointCount,float prop);
typedef float* (__cdecl *ptr_geomPlugin_getPtcloudOctreeCorners)(const void* pcStruct,int* pointCount);
typedef int (__cdecl *ptr_geomPlugin_getPtcloudNonEmptyCellCount)(const void* pcStruct);
typedef void (__cdecl *ptr_geomPlugin_insertPointsIntoPtcloud)(void* pcStruct,const float ptcloudTransformation[7],const float* points,int pointCnt,const unsigned char rgbData[3],float proximityTol);
typedef void (__cdecl *ptr_geomPlugin_insertColorPointsIntoPtcloud)(void* pcStruct,const float ptcloudTransformation[7],const float* points,int pointCnt,const unsigned char* rgbData,float proximityTol);
typedef bool (__cdecl *ptr_geomPlugin_removePointsFromPtcloud)(void* pcStruct,const float ptcloudTransformation[7],const float* points,int pointCnt,float proximityTol,int* countRemoved);
typedef bool (__cdecl *ptr_geomPlugin_removeOctreeFromPtcloud)(void* pcStruct,const float ptcloudTransformation[7],const void* ocStruct,const float octreeTransformation[7],int* countRemoved);
typedef bool (__cdecl *ptr_geomPlugin_intersectPointsWithPtcloud)(void* pcStruct,const float ptcloudTransformation[7],const float* points,int pointCnt,float proximityTol);
typedef bool (__cdecl *ptr_geomPlugin_getMeshMeshCollision)(const void* mesh1ObbStruct,const float mesh1Transformation[7],const void* mesh2ObbStruct,const float mesh2Transformation[7],float** intersections,int* intersectionsSize,int* mesh1Caching,int* mesh2Caching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshOctreeCollision)(const void* meshObbStruct,const float meshTransformation[7],const void* ocStruct,const float octreeTransformation[7],int* meshCaching,unsigned long long int* ocCaching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshTriangleCollision)(const void* meshObbStruct,const float meshTransformation[7],const float p[3],const float v[3],const float w[3],float** intersections,int* intersectionsSize,int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshSegmentCollision)(const void* meshObbStruct,const float meshTransformation[7],const float segmentExtremity[3],const float segmentVector[3],float** intersections,int* intersectionsSize,int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeOctreeCollision)(const void* oc1Struct,const float octree1Transformation[7],const void* oc2Struct,const float octree2Transformation[7],unsigned long long int* oc1Caching,unsigned long long int* oc2Caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreePtcloudCollision)(const void* ocStruct,const float octreeTransformation[7],const void* pcStruct,const float ptcloudTransformation[7],unsigned long long int* ocCaching,unsigned long long int* pcCaching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeTriangleCollision)(const void* ocStruct,const float octreeTransformation[7],const float p[3],const float v[3],const float w[3],unsigned long long int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeSegmentCollision)(const void* ocStruct,const float octreeTransformation[7],const float segmentExtremity[3],const float segmentVector[3],unsigned long long int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreePointsCollision)(const void* ocStruct,const float octreeTransformation[7],const float* points,int pointCount);
typedef bool (__cdecl *ptr_geomPlugin_getOctreePointCollision)(const void* ocStruct,const float octreeTransformation[7],const float point[3],unsigned* usrData,unsigned long long int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getBoxBoxCollision)(const float box1Transformation[7],const float box1HalfSize[3],const float box2Transformation[7],const float box2HalfSize[3],bool boxesAreSolid);
typedef bool (__cdecl *ptr_geomPlugin_getBoxTriangleCollision)(const float boxTransformation[7],const float boxHalfSize[3],bool boxIsSolid,const float p[3],const float v[3],const float w[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxSegmentCollision)(const float boxTransformation[7],const float boxHalfSize[3],bool boxIsSolid,const float segmentEndPoint[3],const float segmentVector[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxPointCollision)(const float boxTransformation[7],const float boxHalfSize[3],const float point[3]);
typedef bool (__cdecl *ptr_geomPlugin_getTriangleTriangleCollision)(const float p1[3],const float v1[3],const float w1[3],const float p2[3],const float v2[3],const float w2[3],float** intersections,int* intersectionsSize);
typedef bool (__cdecl *ptr_geomPlugin_getTriangleSegmentCollision)(const float p[3],const float v[3],const float w[3],const float segmentEndPoint[3],const float segmentVector[3],float** intersections,int* intersectionsSize);
typedef bool (__cdecl *ptr_geomPlugin_getMeshMeshDistanceIfSmaller)(const void* mesh1ObbStruct,const float mesh1Transformation[7],const void* mesh2ObbStruct,const float mesh2Transformation[7],float* dist,float minDistSegPt1[3],float minDistSegPt2[3],int* mesh1Caching,int* mesh2Caching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshOctreeDistanceIfSmaller)(const void* meshObbStruct,const float meshTransformation[7],const void* ocStruct,const float octreeTransformation[7],float* dist,float meshMinDistPt[3],float ocMinDistPt[3],int* meshCaching,unsigned long long int* ocCaching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshPtcloudDistanceIfSmaller)(const void* meshObbStruct,const float meshTransformation[7],const void* pcStruct,const float pcTransformation[7],float* dist,float meshMinDistPt[3],float pcMinDistPt[3],int* meshCaching,unsigned long long int* pcCaching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshTriangleDistanceIfSmaller)(const void* meshObbStruct,const float meshTransformation[7],const float p[3],const float v[3],const float w[3],float* dist,float minDistSegPt1[3],float minDistSegPt2[3],int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshSegmentDistanceIfSmaller)(const void* meshObbStruct,const float meshTransformation[7],const float segmentEndPoint[3],const float segmentVector[3],float* dist,float minDistSegPt1[3],float minDistSegPt2[3],int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getMeshPointDistanceIfSmaller)(const void* meshObbStruct,const float meshTransformation[7],const float point[3],float* dist,float minDistSegPt[3],int* caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeOctreeDistanceIfSmaller)(const void* oc1Struct,const float octree1Transformation[7],const void* oc2Struct,const float octree2Transformation[7],float* dist,float oc1MinDistPt[3],float oc2MinDistPt[3],unsigned long long int* oc1Caching,unsigned long long int* oc2Caching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreePtcloudDistanceIfSmaller)(const void* ocStruct,const float octreeTransformation[7],const void* pcStruct,const float pcTransformation[7],float* dist,float ocMinDistPt[3],float pcMinDistPt[3],unsigned long long int* ocCaching,unsigned long long int* pcCaching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeTriangleDistanceIfSmaller)(const void* ocStruct,const float octreeTransformation[7],const float p[3],const float v[3],const float w[3],float* dist,float ocMinDistPt[3],float triMinDistPt[3],unsigned long long int* ocCaching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreeSegmentDistanceIfSmaller)(const void* ocStruct,const float octreeTransformation[7],const float segmentEndPoint[3],const float segmentVector[3],float* dist,float ocMinDistPt[3],float segMinDistPt[3],unsigned long long int* ocCaching);
typedef bool (__cdecl *ptr_geomPlugin_getOctreePointDistanceIfSmaller)(const void* ocStruct,const float octreeTransformation[7],const float point[3],float* dist,float ocMinDistPt[3],unsigned long long int* ocCaching);
typedef bool (__cdecl *ptr_geomPlugin_getPtcloudPtcloudDistanceIfSmaller)(const void* pc1Struct,const float pc1Transformation[7],const void* pc2Struct,const float pc2Transformation[7],float* dist,float* pc1MinDistPt,float* pc2MinDistPt,unsigned long long int* pc1Caching,unsigned long long int* pc2Caching);
typedef bool (__cdecl *ptr_geomPlugin_getPtcloudTriangleDistanceIfSmaller)(const void* pcStruct,const float pcTransformation[7],const float p[3],const float v[3],const float w[3],float* dist,float* pcMinDistPt,float* triMinDistPt,unsigned long long int* pcCaching);
typedef bool (__cdecl *ptr_geomPlugin_getPtcloudSegmentDistanceIfSmaller)(const void* pcStruct,const float pcTransformation[7],const float segmentEndPoint[3],const float segmentVector[3],float* dist,float* pcMinDistPt,float* segMinDistPt,unsigned long long int* pcCaching);
typedef bool (__cdecl *ptr_geomPlugin_getPtcloudPointDistanceIfSmaller)(const void* pcStruct,const float pcTransformation[7],const float point[3],float* dist,float* pcMinDistPt,unsigned long long int* pcCaching);
typedef float (__cdecl *ptr_geomPlugin_getApproxBoxBoxDistance)(const float box1Transformation[7],const float box1HalfSize[3],const float box2Transformation[7],const float box2HalfSize[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxBoxDistanceIfSmaller)(const float box1Transformation[7],const float box1HalfSize[3],const float box2Transformation[7],const float box2HalfSize[3],bool boxesAreSolid,float* dist,float distSegPt1[3],float distSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxTriangleDistanceIfSmaller)(const float boxTransformation[7],const float boxHalfSize[3],bool boxIsSolid,const float p[3],const float v[3],const float w[3],float* dist,float distSegPt1[3],float distSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxSegmentDistanceIfSmaller)(const float boxTransformation[7],const float boxHalfSize[3],bool boxIsSolid,const float segmentEndPoint[3],const float segmentVector[3],float* dist,float distSegPt1[3],float distSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getBoxPointDistanceIfSmaller)(const float boxTransformation[7],const float boxHalfSize[3],bool boxIsSolid,const float point[3],float* dist,float distSegPt1[3]);
typedef bool (__cdecl *ptr_geomPlugin_getTriangleTriangleDistanceIfSmaller)(const float p1[3],const float v1[3],const float w1[3],const float p2[3],const float v2[3],const float w2[3],float* dist,float minDistSegPt1[3],float minDistSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getTriangleSegmentDistanceIfSmaller)(const float p[3],const float v[3],const float w[3],const float segmentEndPoint[3],const float segmentVector[3],float* dist,float minDistSegPt1[3],float minDistSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getTrianglePointDistanceIfSmaller)(const float p[3],const float v[3],const float w[3],const float point[3],float* dist,float minDistSegPt[3]);
typedef bool (__cdecl *ptr_geomPlugin_getSegmentSegmentDistanceIfSmaller)(const float segment1EndPoint[3],const float segment1Vector[3],const float segment2EndPoint[3],const float segment2Vector[3],float* dist,float minDistSegPt1[3],float minDistSegPt2[3]);
typedef bool (__cdecl *ptr_geomPlugin_getSegmentPointDistanceIfSmaller)(const float segmentEndPoint[3],const float segmentVector[3],const float point[3],float* dist,float minDistSegPt[3]);
typedef bool (__cdecl *ptr_geomPlugin_volumeSensorDetectMeshIfSmaller)(const float* planesIn,int planesInSize,const float* planesOut,int planesOutSize,const void* obbStruct,const float meshTransformationRelative[7],float* dist,bool fast,bool frontDetection,bool backDetection,float maxAngle,float detectPt[3],float triN[3]);
typedef bool (__cdecl *ptr_geomPlugin_volumeSensorDetectOctreeIfSmaller)(const float* planesIn,int planesInSize,const float* planesOut,int planesOutSize,const void* ocStruct,const float octreeTransformationRelative[7],float* dist,bool fast,bool frontDetection,bool backDetection,float maxAngle,float detectPt[3],float triN[3]);
typedef bool (__cdecl *ptr_geomPlugin_volumeSensorDetectPtcloudIfSmaller)(const float* planesIn,int planesInSize,const float* planesOut,int planesOutSize,const void* pcStruct,const float ptcloudTransformationRelative[7],float* dist,bool fast,float detectPt[3]);
typedef bool (__cdecl *ptr_geomPlugin_volumeSensorDetectTriangleIfSmaller)(const float* planesIn,int planesInSize,const float* planesOut,int planesOutSize,const float p[3],const float v[3],const float w[3],float* dist,bool frontDetection,bool backDetection,float maxAngle,float detectPt[3],float triN[3]);
typedef bool (__cdecl *ptr_geomPlugin_volumeSensorDetectSegmentIfSmaller)(const float* planesIn,int planesInSize,const float* planesOut,int planesOutSize,const float segmentEndPoint[3],const float segmentVector[3],float* dist,float maxAngle,float detectPt[3]);
typedef bool (__cdecl *ptr_geomPlugin_raySensorDetectMeshIfSmaller)(const float rayStart[3],const float rayVect[3],const void* obbStruct,const float meshTransformationRelative[7],float* dist,float forbiddenDist,bool fast,bool frontDetection,bool backDetection,float maxAngle,float detectPt[3],float triN[3],bool* forbiddenDistTouched);
typedef bool (__cdecl *ptr_geomPlugin_raySensorDetectOctreeIfSmaller)(const float rayStart[3],const float rayVect[3],const void* ocStruct,const float octreeTransformationRealtive[7],float* dist,float forbiddenDist,bool fast,bool frontDetection,bool backDetection,float maxAngle,float detectPt[3],float triN[3],bool* forbiddenDistTouched);
typedef bool (__cdecl *ptr_geomPlugin_isPointInVolume)(const float* planesIn,int planesInSize,const float point[3]);

typedef int (__cdecl *ptr_ikPlugin_createEnv)();
//typedef bool (__cdecl *ptr_ikPlugin_switchEnvironment)(int handle);
typedef void (__cdecl *ptr_ikPlugin_eraseEnvironment)(int ikEnv);
typedef void (__cdecl *ptr_ikPlugin_eraseObject)(int ikEnv,int objectHandle);
typedef void (__cdecl *ptr_ikPlugin_setObjectParent)(int ikEnv,int objectHandle,int parentObjectHandle);
typedef int (__cdecl *ptr_ikPlugin_createDummy)(int ikEnv);
typedef void (__cdecl *ptr_ikPlugin_setLinkedDummy)(int ikEnv,int dummyHandle,int linkedDummyHandle);
typedef int (__cdecl *ptr_ikPlugin_createJoint)(int ikEnv,int jointType);
typedef void (__cdecl *ptr_ikPlugin_setJointMode)(int ikEnv,int jointHandle,int jointMode);
typedef void (__cdecl *ptr_ikPlugin_setJointInterval)(int ikEnv,int jointHandle,bool cyclic,const float* intervalMinAndRange);
typedef void (__cdecl *ptr_ikPlugin_setJointScrewPitch)(int ikEnv,int jointHandle,float pitch);
typedef void (__cdecl *ptr_ikPlugin_setJointIkWeight)(int ikEnv,int jointHandle,float ikWeight);
typedef void (__cdecl *ptr_ikPlugin_setJointMaxStepSize)(int ikEnv,int jointHandle,float maxStepSize);
typedef void (__cdecl *ptr_ikPlugin_setJointDependency)(int ikEnv,int jointHandle,int dependencyJointHandle,float offset,float mult);
typedef float (__cdecl *ptr_ikPlugin_getJointPosition)(int ikEnv,int jointHandle);
typedef void (__cdecl *ptr_ikPlugin_setJointPosition)(int ikEnv,int jointHandle,float position);
typedef void (__cdecl *ptr_ikPlugin_getSphericalJointQuaternion)(int ikEnv,int jointHandle,float* quaternion);
typedef void (__cdecl *ptr_ikPlugin_setSphericalJointQuaternion)(int ikEnv,int jointHandle,const float* quaternion);
typedef int (__cdecl *ptr_ikPlugin_createIkGroup)(int ikEnv);
typedef void (__cdecl *ptr_ikPlugin_eraseIkGroup)(int ikEnv,int ikGroupHandle);
typedef void (__cdecl *ptr_ikPlugin_setIkGroupFlags)(int ikEnv,int ikGroupHandle,int flags);
typedef void (__cdecl *ptr_ikPlugin_setIkGroupCalculation)(int ikEnv,int ikGroupHandle,int method,float damping,int maxIterations);
typedef int (__cdecl *ptr_ikPlugin_addIkElement)(int ikEnv,int ikGroupHandle,int tipHandle);
typedef void (__cdecl *ptr_ikPlugin_eraseIkElement)(int ikEnv,int ikGroupHandle,int ikElementIndex);
typedef void (__cdecl *ptr_ikPlugin_setIkElementFlags)(int ikEnv,int ikGroupHandle,int ikElementIndex,int flags);
typedef void (__cdecl *ptr_ikPlugin_setIkElementBase)(int ikEnv,int ikGroupHandle,int ikElementIndex,int baseHandle,int constraintsBaseHandle);
typedef void (__cdecl *ptr_ikPlugin_setIkElementConstraints)(int ikEnv,int ikGroupHandle,int ikElementIndex,int constraints);
typedef void (__cdecl *ptr_ikPlugin_setIkElementPrecision)(int ikEnv,int ikGroupHandle,int ikElementIndex,float linearPrecision,float angularPrecision);
typedef void (__cdecl *ptr_ikPlugin_setIkElementWeights)(int ikEnv,int ikGroupHandle,int ikElementIndex,float linearWeight,float angularWeight);
typedef int (__cdecl *ptr_ikPlugin_handleIkGroup)(int ikEnv,int ikGroupHandle);
typedef bool (__cdecl *ptr_ikPlugin_computeJacobian)(int ikEnv,int ikGroupHandle,int options);
typedef float* (__cdecl *ptr_ikPlugin_getJacobian)(int ikEnv,int ikGroupHandle,int* matrixSize);
typedef float (__cdecl *ptr_ikPlugin_getManipulability)(int ikEnv,int ikGroupHandle);
typedef char* (__cdecl *ptr_ikPlugin_getConfigForTipPose)(int ikEnv,int ikGroupHandle,int jointCnt,const int* jointHandles,float thresholdDist,int maxIterations,int* result,float* retConfig,const float* metric,bool(*validationCallback)(float*),const int* jointOptions,const float* lowLimits,const float* ranges);
typedef void (__cdecl *ptr_ikPlugin_getObjectLocalTransformation)(int ikEnv,int objectHandle,float* pos,float* quat);
typedef void (__cdecl *ptr_ikPlugin_setObjectLocalTransformation)(int ikEnv,int objectHandle,const float* pos,const float* quat);

typedef char* (__cdecl *ptrCodeEditor_openModal)(const char* initText,const char* properties,int* positionAndSize);
typedef int (__cdecl *ptrCodeEditor_open)(const char* initText,const char* properties);
typedef int (__cdecl *ptrCodeEditor_setText)(int handle,const char* text,int insertMode);
typedef char* (__cdecl *ptrCodeEditor_getText)(int handle,int* positionAndSize);
typedef int (__cdecl *ptrCodeEditor_show)(int handle,int showState);
typedef int (__cdecl *ptrCodeEditor_close)(int handle,int* positionAndSize);

typedef void* (__cdecl *ptrPythonPlugin_initState)(int scriptHandle,const char* scriptName);
typedef void (__cdecl *ptrPythonPlugin_cleanupState)(void* state);
typedef char* (__cdecl *ptrPythonPlugin_loadCode)(void* state,const char* code,const char* functionsToFind,bool* functionsFound,int* result);
typedef char* (__cdecl *ptrPythonPlugin_callFunc)(void* state,const char* funcName,int inStackHandle,int outStackHandle,int* result);
typedef int (__cdecl *ptrPythonPlugin_execStr)(void* state,const char* str,int outStackHandle);
typedef int (__cdecl *ptrPythonPlugin_isDeprecated)(const char* str);
typedef char* (__cdecl *ptrPythonPlugin_getFuncs)(const char* str);
typedef char* (__cdecl *ptrPythonPlugin_getConsts)(const char* str);
typedef char* (__cdecl *ptrPythonPlugin_getCalltip)(const char* func);

typedef int (__cdecl *ptrRuckigPlugin_pos)(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetPos,const double* targetVel);
typedef int (__cdecl *ptrRuckigPlugin_vel)(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetVel);
typedef int (__cdecl *ptrRuckigPlugin_step)(int objHandle,double timeStep,double* newPos,double* newVel,double* newAccel,double* syncTime);
typedef int (__cdecl *ptrRuckigPlugin_remove)(int objHandle);
typedef int (__cdecl *ptrRuckigPlugin_dofs)(int objHandle);

typedef int (__cdecl *ptrCustomUi_msgBox)(int type, int buttons, const char *title, const char *message);
typedef char* (__cdecl *ptrCustomUi_fileDialog)(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext, int native);

typedef int* (__cdecl *ptrassimp_importShapes)(const char* fileNames,int maxTextures,float scaling,int upVector,int options,int* shapeCount);
typedef void (__cdecl *ptrassimp_exportShapes)(const int* shapeHandles,int shapeCount,const char* filename,const char* format,float scaling,int upVector,int options);
typedef int (__cdecl *ptrassimp_importMeshes)(const char* fileNames,float scaling,int upVector,int options,float*** allVertices,int** verticesSizes,int*** allIndices,int** indicesSizes);
typedef void (__cdecl *ptrassimp_exportMeshes)(int meshCnt,const float** allVertices,const int* verticesSizes,const int** allIndices,const int* indicesSizes,const char* filename,const char* format,float scaling,int upVector,int options);



class CPlugin
{
public:
    CPlugin(const char* filename,const char* pluginName);
    virtual ~CPlugin();
    int load();
    void setConsoleVerbosity(int level);
    int getConsoleVerbosity() const;
    void setStatusbarVerbosity(int level);
    int getStatusbarVerbosity() const;
    std::string getName() const;
    void* sendEventCallbackMessage(int msg,int* auxVals,void* data,int retVals[4]);

    ptrStart startAddress;
    ptrEnd endAddress;
    ptrMessage messageAddress;

    ptr_syncPlugin_msg syncPlugin_msg;

    ptr_dynPlugin_startSimulation dynPlugin_startSimulation;
    ptr_dynPlugin_endSimulation dynPlugin_endSimulation;
    ptr_dynPlugin_step dynPlugin_step;
    ptr_dynPlugin_isDynamicContentAvailable dynPlugin_isDynamicContentAvailable;
    ptr_dynPlugin_serializeDynamicContent dynPlugin_serializeDynamicContent;
    ptr_dynPlugin_addParticleObject dynPlugin_addParticleObject;
    ptr_dynPlugin_removeParticleObject dynPlugin_removeParticleObject;
    ptr_dynPlugin_addParticleObjectItem dynPlugin_addParticleObjectItem;
    ptr_dynPlugin_getParticleObjectOtherFloatsPerItem dynPlugin_getParticleObjectOtherFloatsPerItem;
    ptr_dynPlugin_getContactPoints dynPlugin_getContactPoints;
    ptr_dynPlugin_getParticles dynPlugin_getParticles;
    ptr_dynPlugin_getParticleData dynPlugin_getParticleData;
    ptr_dynPlugin_getContactForce dynPlugin_getContactForce;
    ptr_dynPlugin_reportDynamicWorldConfiguration dynPlugin_reportDynamicWorldConfiguration;
    ptr_dynPlugin_getDynamicStepDivider dynPlugin_getDynamicStepDivider;
    ptr_dynPlugin_getEngineInfo dynPlugin_getEngineInfo;

    ptr_geomPlugin_releaseBuffer geomPlugin_releaseBuffer;
    ptr_geomPlugin_createMesh geomPlugin_createMesh;
    ptr_geomPlugin_copyMesh geomPlugin_copyMesh;
    ptr_geomPlugin_getMeshFromSerializationData geomPlugin_getMeshFromSerializationData;
    ptr_geomPlugin_getMeshSerializationData geomPlugin_getMeshSerializationData;
    ptr_geomPlugin_scaleMesh geomPlugin_scaleMesh;
    ptr_geomPlugin_destroyMesh geomPlugin_destroyMesh;
    ptr_geomPlugin_getMeshRootObbVolume geomPlugin_getMeshRootObbVolume;
    ptr_geomPlugin_createOctreeFromPoints geomPlugin_createOctreeFromPoints;
    ptr_geomPlugin_createOctreeFromColorPoints geomPlugin_createOctreeFromColorPoints;
    ptr_geomPlugin_createOctreeFromMesh geomPlugin_createOctreeFromMesh;
    ptr_geomPlugin_createOctreeFromOctree geomPlugin_createOctreeFromOctree;
    ptr_geomPlugin_copyOctree geomPlugin_copyOctree;
    ptr_geomPlugin_getOctreeFromSerializationData geomPlugin_getOctreeFromSerializationData;
    ptr_geomPlugin_getOctreeSerializationData geomPlugin_getOctreeSerializationData;
    ptr_geomPlugin_scaleOctree geomPlugin_scaleOctree;
    ptr_geomPlugin_destroyOctree geomPlugin_destroyOctree;
    ptr_geomPlugin_getOctreeVoxelData geomPlugin_getOctreeVoxelData;
    ptr_geomPlugin_getOctreeUserData geomPlugin_getOctreeUserData;
    ptr_geomPlugin_getOctreeCornersFromOctree geomPlugin_getOctreeCornersFromOctree;
    ptr_geomPlugin_insertPointsIntoOctree geomPlugin_insertPointsIntoOctree;
    ptr_geomPlugin_insertColorPointsIntoOctree geomPlugin_insertColorPointsIntoOctree;
    ptr_geomPlugin_insertMeshIntoOctree geomPlugin_insertMeshIntoOctree;
    ptr_geomPlugin_insertOctreeIntoOctree geomPlugin_insertOctreeIntoOctree;
    ptr_geomPlugin_removePointsFromOctree geomPlugin_removePointsFromOctree;
    ptr_geomPlugin_removeMeshFromOctree geomPlugin_removeMeshFromOctree;
    ptr_geomPlugin_removeOctreeFromOctree geomPlugin_removeOctreeFromOctree;
    ptr_geomPlugin_createPtcloudFromPoints geomPlugin_createPtcloudFromPoints;
    ptr_geomPlugin_createPtcloudFromColorPoints geomPlugin_createPtcloudFromColorPoints;
    ptr_geomPlugin_copyPtcloud geomPlugin_copyPtcloud;
    ptr_geomPlugin_getPtcloudFromSerializationData geomPlugin_getPtcloudFromSerializationData;
    ptr_geomPlugin_getPtcloudSerializationData geomPlugin_getPtcloudSerializationData;
    ptr_geomPlugin_scalePtcloud geomPlugin_scalePtcloud;
    ptr_geomPlugin_destroyPtcloud geomPlugin_destroyPtcloud;
    ptr_geomPlugin_getPtcloudPoints geomPlugin_getPtcloudPoints;
    ptr_geomPlugin_getPtcloudOctreeCorners geomPlugin_getPtcloudOctreeCorners;
    ptr_geomPlugin_getPtcloudNonEmptyCellCount geomPlugin_getPtcloudNonEmptyCellCount;
    ptr_geomPlugin_insertPointsIntoPtcloud geomPlugin_insertPointsIntoPtcloud;
    ptr_geomPlugin_insertColorPointsIntoPtcloud geomPlugin_insertColorPointsIntoPtcloud;
    ptr_geomPlugin_removePointsFromPtcloud geomPlugin_removePointsFromPtcloud;
    ptr_geomPlugin_removeOctreeFromPtcloud geomPlugin_removeOctreeFromPtcloud;
    ptr_geomPlugin_intersectPointsWithPtcloud geomPlugin_intersectPointsWithPtcloud;
    ptr_geomPlugin_getMeshMeshCollision geomPlugin_getMeshMeshCollision;
    ptr_geomPlugin_getMeshOctreeCollision geomPlugin_getMeshOctreeCollision;
    ptr_geomPlugin_getMeshTriangleCollision geomPlugin_getMeshTriangleCollision;
    ptr_geomPlugin_getMeshSegmentCollision geomPlugin_getMeshSegmentCollision;
    ptr_geomPlugin_getOctreeOctreeCollision geomPlugin_getOctreeOctreeCollision;
    ptr_geomPlugin_getOctreePtcloudCollision geomPlugin_getOctreePtcloudCollision;
    ptr_geomPlugin_getOctreeTriangleCollision geomPlugin_getOctreeTriangleCollision;
    ptr_geomPlugin_getOctreeSegmentCollision geomPlugin_getOctreeSegmentCollision;
    ptr_geomPlugin_getOctreePointsCollision geomPlugin_getOctreePointsCollision;
    ptr_geomPlugin_getOctreePointCollision geomPlugin_getOctreePointCollision;
    ptr_geomPlugin_getBoxBoxCollision geomPlugin_getBoxBoxCollision;
    ptr_geomPlugin_getBoxTriangleCollision geomPlugin_getBoxTriangleCollision;
    ptr_geomPlugin_getBoxSegmentCollision geomPlugin_getBoxSegmentCollision;
    ptr_geomPlugin_getBoxPointCollision geomPlugin_getBoxPointCollision;
    ptr_geomPlugin_getTriangleTriangleCollision geomPlugin_getTriangleTriangleCollision;
    ptr_geomPlugin_getTriangleSegmentCollision geomPlugin_getTriangleSegmentCollision;
    ptr_geomPlugin_getMeshMeshDistanceIfSmaller geomPlugin_getMeshMeshDistanceIfSmaller;
    ptr_geomPlugin_getMeshOctreeDistanceIfSmaller geomPlugin_getMeshOctreeDistanceIfSmaller;
    ptr_geomPlugin_getMeshPtcloudDistanceIfSmaller geomPlugin_getMeshPtcloudDistanceIfSmaller;
    ptr_geomPlugin_getMeshTriangleDistanceIfSmaller geomPlugin_getMeshTriangleDistanceIfSmaller;
    ptr_geomPlugin_getMeshSegmentDistanceIfSmaller geomPlugin_getMeshSegmentDistanceIfSmaller;
    ptr_geomPlugin_getMeshPointDistanceIfSmaller geomPlugin_getMeshPointDistanceIfSmaller;
    ptr_geomPlugin_getOctreeOctreeDistanceIfSmaller geomPlugin_getOctreeOctreeDistanceIfSmaller;
    ptr_geomPlugin_getOctreePtcloudDistanceIfSmaller geomPlugin_getOctreePtcloudDistanceIfSmaller;
    ptr_geomPlugin_getOctreeTriangleDistanceIfSmaller geomPlugin_getOctreeTriangleDistanceIfSmaller;
    ptr_geomPlugin_getOctreeSegmentDistanceIfSmaller geomPlugin_getOctreeSegmentDistanceIfSmaller;
    ptr_geomPlugin_getOctreePointDistanceIfSmaller geomPlugin_getOctreePointDistanceIfSmaller;
    ptr_geomPlugin_getPtcloudPtcloudDistanceIfSmaller geomPlugin_getPtcloudPtcloudDistanceIfSmaller;
    ptr_geomPlugin_getPtcloudTriangleDistanceIfSmaller geomPlugin_getPtcloudTriangleDistanceIfSmaller;
    ptr_geomPlugin_getPtcloudSegmentDistanceIfSmaller geomPlugin_getPtcloudSegmentDistanceIfSmaller;
    ptr_geomPlugin_getPtcloudPointDistanceIfSmaller geomPlugin_getPtcloudPointDistanceIfSmaller;
    ptr_geomPlugin_getApproxBoxBoxDistance geomPlugin_getApproxBoxBoxDistance;
    ptr_geomPlugin_getBoxBoxDistanceIfSmaller geomPlugin_getBoxBoxDistanceIfSmaller;
    ptr_geomPlugin_getBoxTriangleDistanceIfSmaller geomPlugin_getBoxTriangleDistanceIfSmaller;
    ptr_geomPlugin_getBoxSegmentDistanceIfSmaller geomPlugin_getBoxSegmentDistanceIfSmaller;
    ptr_geomPlugin_getBoxPointDistanceIfSmaller geomPlugin_getBoxPointDistanceIfSmaller;
    ptr_geomPlugin_getTriangleTriangleDistanceIfSmaller geomPlugin_getTriangleTriangleDistanceIfSmaller;
    ptr_geomPlugin_getTriangleSegmentDistanceIfSmaller geomPlugin_getTriangleSegmentDistanceIfSmaller;
    ptr_geomPlugin_getTrianglePointDistanceIfSmaller geomPlugin_getTrianglePointDistanceIfSmaller;
    ptr_geomPlugin_getSegmentSegmentDistanceIfSmaller geomPlugin_getSegmentSegmentDistanceIfSmaller;
    ptr_geomPlugin_getSegmentPointDistanceIfSmaller geomPlugin_getSegmentPointDistanceIfSmaller;
    ptr_geomPlugin_volumeSensorDetectMeshIfSmaller geomPlugin_volumeSensorDetectMeshIfSmaller;
    ptr_geomPlugin_volumeSensorDetectOctreeIfSmaller geomPlugin_volumeSensorDetectOctreeIfSmaller;
    ptr_geomPlugin_volumeSensorDetectPtcloudIfSmaller geomPlugin_volumeSensorDetectPtcloudIfSmaller;
    ptr_geomPlugin_volumeSensorDetectTriangleIfSmaller geomPlugin_volumeSensorDetectTriangleIfSmaller;
    ptr_geomPlugin_volumeSensorDetectSegmentIfSmaller geomPlugin_volumeSensorDetectSegmentIfSmaller;
    ptr_geomPlugin_raySensorDetectMeshIfSmaller geomPlugin_raySensorDetectMeshIfSmaller;
    ptr_geomPlugin_raySensorDetectOctreeIfSmaller geomPlugin_raySensorDetectOctreeIfSmaller;
    ptr_geomPlugin_isPointInVolume geomPlugin_isPointInVolume;

    ptr_ikPlugin_createEnv ikPlugin_createEnv;
//    ptr_ikPlugin_switchEnvironment ikPlugin_switchEnvironment;
    ptr_ikPlugin_eraseEnvironment ikPlugin_eraseEnvironment;
    ptr_ikPlugin_eraseObject ikPlugin_eraseObject;
    ptr_ikPlugin_setObjectParent ikPlugin_setObjectParent;
    ptr_ikPlugin_createDummy ikPlugin_createDummy;
    ptr_ikPlugin_setLinkedDummy ikPlugin_setLinkedDummy;
    ptr_ikPlugin_createJoint ikPlugin_createJoint;
    ptr_ikPlugin_setJointMode ikPlugin_setJointMode;
    ptr_ikPlugin_setJointInterval ikPlugin_setJointInterval;
    ptr_ikPlugin_setJointScrewPitch ikPlugin_setJointScrewPitch;
    ptr_ikPlugin_setJointIkWeight ikPlugin_setJointIkWeight;
    ptr_ikPlugin_setJointMaxStepSize ikPlugin_setJointMaxStepSize;
    ptr_ikPlugin_setJointDependency ikPlugin_setJointDependency;
    ptr_ikPlugin_getJointPosition ikPlugin_getJointPosition;
    ptr_ikPlugin_setJointPosition ikPlugin_setJointPosition;
    ptr_ikPlugin_getSphericalJointQuaternion ikPlugin_getSphericalJointQuaternion;
    ptr_ikPlugin_setSphericalJointQuaternion ikPlugin_setSphericalJointQuaternion;
    ptr_ikPlugin_createIkGroup ikPlugin_createIkGroup;
    ptr_ikPlugin_eraseIkGroup ikPlugin_eraseIkGroup;
    ptr_ikPlugin_setIkGroupFlags ikPlugin_setIkGroupFlags;
    ptr_ikPlugin_setIkGroupCalculation ikPlugin_setIkGroupCalculation;
    ptr_ikPlugin_addIkElement ikPlugin_addIkElement;
    ptr_ikPlugin_eraseIkElement ikPlugin_eraseIkElement;
    ptr_ikPlugin_setIkElementFlags ikPlugin_setIkElementFlags;
    ptr_ikPlugin_setIkElementBase ikPlugin_setIkElementBase;
    ptr_ikPlugin_setIkElementConstraints ikPlugin_setIkElementConstraints;
    ptr_ikPlugin_setIkElementPrecision ikPlugin_setIkElementPrecision;
    ptr_ikPlugin_setIkElementWeights ikPlugin_setIkElementWeights;
    ptr_ikPlugin_handleIkGroup ikPlugin_handleIkGroup;
    ptr_ikPlugin_computeJacobian ikPlugin_computeJacobian;
    ptr_ikPlugin_getJacobian ikPlugin_getJacobian;
    ptr_ikPlugin_getManipulability ikPlugin_getManipulability;
    ptr_ikPlugin_getConfigForTipPose ikPlugin_getConfigForTipPose;
    ptr_ikPlugin_getObjectLocalTransformation ikPlugin_getObjectLocalTransformation;
    ptr_ikPlugin_setObjectLocalTransformation ikPlugin_setObjectLocalTransformation;

    ptrPythonPlugin_initState pythonPlugin_initState;
    ptrPythonPlugin_cleanupState pythonPlugin_cleanupState;
    ptrPythonPlugin_loadCode pythonPlugin_loadCode;
    ptrPythonPlugin_callFunc pythonPlugin_callFunc;
    ptrPythonPlugin_execStr pythonPlugin_execStr;
    ptrPythonPlugin_isDeprecated pythonPlugin_isDeprecated;
    ptrPythonPlugin_getFuncs pythonPlugin_getFuncs;
    ptrPythonPlugin_getConsts pythonPlugin_getConsts;
    ptrPythonPlugin_getCalltip pythonPlugin_getCalltip;


    ptrRuckigPlugin_pos ruckigPlugin_pos;
    ptrRuckigPlugin_vel ruckigPlugin_vel;
    ptrRuckigPlugin_step ruckigPlugin_step;
    ptrRuckigPlugin_remove ruckigPlugin_remove;
    ptrRuckigPlugin_dofs ruckigPlugin_dofs;

    ptrCodeEditor_openModal _codeEditor_openModal;
    ptrCodeEditor_open _codeEditor_open;
    ptrCodeEditor_setText _codeEditor_setText;
    ptrCodeEditor_getText _codeEditor_getText;
    ptrCodeEditor_show _codeEditor_show;
    ptrCodeEditor_close _codeEditor_close;

    ptrCustomUi_msgBox _customUi_msgBox;
    ptrCustomUi_fileDialog _customUi_fileDialog;

    ptrassimp_importShapes _assimp_importShapes;
    ptrassimp_exportShapes _assimp_exportShapes;
    ptrassimp_importMeshes _assimp_importMeshes;
    ptrassimp_exportMeshes _assimp_exportMeshes;

    std::string _name;
    std::string _filename;
    unsigned char pluginVersion;
    int extendedVersionInt;
    std::string extendedVersionString;
    std::string buildDateString;
    int handle;
    int _consoleVerbosity;
    int _statusbarVerbosity;
    int _loadCount;
    WLibrary instance;
};


class CPluginContainer  
{
public:
    CPluginContainer();
    virtual ~CPluginContainer();

    static int addPlugin(const char* filename,const char* pluginName);
    static void* sendEventCallbackMessageToAllPlugins(int msg,int* auxVals,void* data,int retVals[4]);
    static void* sendEventCallbackMessageToOnePlugin(const char* pluginName,int msg,int* auxVals,void* data,int retVals[4]);
    static int getPluginCount();
    static CPlugin* getPluginFromIndex(size_t index);
    static CPlugin* getPluginFromName(const char* pluginName,bool caseSensitive);
    static CPlugin* getPluginFromFunc(const char* func);
    static CPlugin* getPluginFromHandle(int handle);
    static bool unloadPlugin(int handle);
    static void _removePlugin(int handle);

    static bool selectExtRenderer(int index);
    static bool extRenderer(int msg,void* data);

    static bool qhull(void* data);
    static bool hacd(void* data);
    static bool vhacd(void* data);
    static bool meshDecimator(void* data);

    static std::vector<CPlugin*> _syncPlugins;
    static void syncMsg(const SSyncMsg* msg,const SSyncRt* rt);

    // physics engines:
    static CPlugin* currentDynEngine;
    static bool dyn_startSimulation(int engine,int version,const float floatParams[20],const int intParams[20]);
    static bool dyn_isInitialized();
    static bool dyn_isDynamicContentAvailable();
    static bool dyn_removeParticleObject(int objectHandle);
    static bool dyn_addParticleObjectItem(int objectHandle,const float* itemData,float simulationTime);
    static bool dyn_getParticleData(const void* particle,float* pos,float* size,int* objectType,float** additionalColor);
    static bool dyn_getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],float* contactInfo);
    static void dyn_endSimulation();
    static void dyn_step(float timeStep,float simulationTime);
    static void dyn_serializeDynamicContent(const char* filenameAndPath,int bulletSerializationBuffer);
    static int dyn_addParticleObject(int objectType,float size,float massOverVolume,const void* params,float lifeTime,int maxItemCount,const float* ambient,const float* diffuse,const float* specular,const float* emission);
    static int dyn_getParticleObjectOtherFloatsPerItem(int objectHandle);
    static float* dyn_getContactPoints(int* count);
    static void** dyn_getParticles(int index,int* particlesCount,int* objectType,float** cols);
    static void dyn_reportDynamicWorldConfiguration(int totalPassesCount,char doNotApplyJointIntrinsicPositions,float simulationTime);
    static int dyn_getDynamicStepDivider();
    static int dyn_getEngineInfo(int* engine,int* data1,char* data2,char* data3);

    // geom plugin:
    static CPlugin* currentGeomPlugin;
    static bool isGeomPluginAvailable();
    static void geomPlugin_lockUnlock(bool lock);
    static void geomPlugin_releaseBuffer(void* buffer);

    // Mesh creation/destruction/manipulation/info
    static void* geomPlugin_createMesh(const float* vertices,int verticesSize,const int* indices,int indicesSize,const C7Vector* meshOrigin=nullptr,float triangleEdgeMaxLength=0.3f,int maxTrianglesInBoundingBox=8);
    static void* geomPlugin_copyMesh(const void* meshObbStruct);
    static void* geomPlugin_getMeshFromSerializationData(const unsigned char* serializationData);
    static void geomPlugin_getMeshSerializationData(const void* meshObbStruct,std::vector<unsigned char>& serializationData);
    static void geomPlugin_scaleMesh(void* meshObbStruct,float scalingFactor);
    static void geomPlugin_destroyMesh(void* meshObbStruct);
    static float geomPlugin_getMeshRootObbVolume(const void* meshObbStruct);

    // OC tree creation/destruction/manipulation/info
    static void* geomPlugin_createOctreeFromPoints(const float* points,int pointCnt,const C7Vector* octreeOrigin=nullptr,float cellS=0.05f,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static void* geomPlugin_createOctreeFromColorPoints(const float* points,int pointCnt,const C7Vector* octreeOrigin=nullptr,float cellS=0.05f,const unsigned char* rgbData=nullptr,const unsigned int* usrData=nullptr);
    static void* geomPlugin_createOctreeFromMesh(const void* meshObbStruct,const C7Vector& meshTransformation,const C7Vector* octreeOrigin=nullptr,float cellS=0.05f,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static void* geomPlugin_createOctreeFromOctree(const void* otherOctreeStruct,const C7Vector& otherOctreeTransformation,const C7Vector* newOctreeOrigin=nullptr,float newOctreeCellS=0.05f,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static void* geomPlugin_copyOctree(const void* ocStruct);
    static void* geomPlugin_getOctreeFromSerializationData(const unsigned char* serializationData);
    static void geomPlugin_getOctreeSerializationData(const void* ocStruct,std::vector<unsigned char>& serializationData);
    static void geomPlugin_scaleOctree(void* ocStruct,float f);
    static void geomPlugin_destroyOctree(void* ocStruct);
    static void geomPlugin_getOctreeVoxelPositions(const void* ocStruct,std::vector<float>& voxelPositions);
    static void geomPlugin_getOctreeVoxelColors(const void* ocStruct,std::vector<float>& voxelColors);
    static void geomPlugin_getOctreeUserData(const void* ocStruct,std::vector<unsigned int>& userData);
    static void geomPlugin_getOctreeCornersFromOctree(const void* ocStruct,std::vector<float>& points);

    static void geomPlugin_insertPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static void geomPlugin_insertColorPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt,const unsigned char* rgbData=nullptr,const unsigned int* usrData=nullptr);
    static void geomPlugin_insertMeshIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static void geomPlugin_insertOctreeIntoOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,const unsigned char rgbData[3]=nullptr,unsigned int usrData=0);
    static bool geomPlugin_removePointsFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt);
    static bool geomPlugin_removeMeshFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation);
    static bool geomPlugin_removeOctreeFromOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation);

    // Point cloud creation/destruction/manipulation/info
    static void* geomPlugin_createPtcloudFromPoints(const float* points,int pointCnt,const C7Vector* ptcloudOrigin=nullptr,float cellS=0.05f,int maxPointCnt=20,const unsigned char rgbData[3]=nullptr,float proximityTol=0.005f);
    static void* geomPlugin_createPtcloudFromColorPoints(const float* points,int pointCnt,const C7Vector* ptcloudOrigin=nullptr,float cellS=0.05f,int maxPointCnt=20,const unsigned char* rgbData=nullptr,float proximityTol=0.005f);
    static void* geomPlugin_copyPtcloud(const void* pcStruct);
    static void* geomPlugin_getPtcloudFromSerializationData(const unsigned char* serializationData);
    static void geomPlugin_getPtcloudSerializationData(const void* pcStruct,std::vector<unsigned char>& serializationData);
    static void geomPlugin_scalePtcloud(void* pcStruct,float f);
    static void geomPlugin_destroyPtcloud(void* pcStruct);
    static void geomPlugin_getPtcloudPoints(const void* pcStruct,std::vector<float>& pointData,std::vector<float>* colors=nullptr,float prop=1.0f);
    static void geomPlugin_getPtcloudOctreeCorners(const void* pcStruct,std::vector<float>& points);
    static int geomPlugin_getPtcloudNonEmptyCellCount(const void* pcStruct);

    static void geomPlugin_insertPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,const unsigned char rgbData[3]=nullptr,float proximityTol=0.001f);
    static void geomPlugin_insertColorPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,const unsigned char* rgbData=nullptr,float proximityTol=0.001f);
    static bool geomPlugin_removePointsFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,float proximityTol,int* countRemoved=nullptr);
    static bool geomPlugin_removeOctreeFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* countRemoved=nullptr);
    static bool geomPlugin_intersectPointsWithPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,float proximityTol=0.001f);

    // Collision detection
    static bool geomPlugin_getMeshMeshCollision(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,std::vector<float>* intersections=nullptr,int* mesh1Caching=nullptr,int* mesh2Caching=nullptr);
    static bool geomPlugin_getMeshOctreeCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* meshCaching=nullptr,unsigned long long int* ocCaching=nullptr);
    static bool geomPlugin_getMeshTriangleCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,std::vector<float>* intersections=nullptr,int* caching=nullptr);
    static bool geomPlugin_getMeshSegmentCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,std::vector<float>* intersections=nullptr,int* caching=nullptr);

    static bool geomPlugin_getOctreeOctreeCollision(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,unsigned long long int* oc1Caching=nullptr,unsigned long long int* oc2Caching=nullptr);
    static bool geomPlugin_getOctreePtcloudCollision(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& ptcloudTransformation,unsigned long long int* ocCaching=nullptr,unsigned long long int* pcCaching=nullptr);
    static bool geomPlugin_getOctreeTriangleCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,unsigned long long int* caching=nullptr);
    static bool geomPlugin_getOctreeSegmentCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,unsigned long long int* caching=nullptr);
    static bool geomPlugin_getOctreePointsCollision(const void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCount);
    static bool geomPlugin_getOctreePointCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,unsigned int* usrData=nullptr,unsigned long long int* caching=nullptr);

    static bool geomPlugin_getBoxBoxCollision(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid);
    static bool geomPlugin_getBoxTriangleCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w);
    static bool geomPlugin_getBoxSegmentCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector);
    static bool geomPlugin_getBoxPointCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,const C3Vector& point);

    static bool geomPlugin_getTriangleTriangleCollision(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,std::vector<float>* intersections=nullptr);
    static bool geomPlugin_getTriangleSegmentCollision(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,std::vector<float>* intersections=nullptr);

    // Distance calculation
    static bool geomPlugin_getMeshMeshDistanceIfSmaller(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr,int* mesh1Caching=nullptr,int* mesh2Caching=nullptr);
    static bool geomPlugin_getMeshOctreeDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,C3Vector* meshMinDistPt=nullptr,C3Vector* ocMinDistPt=nullptr,int* meshCaching=nullptr,unsigned long long int* ocCaching=nullptr);
    static bool geomPlugin_getMeshPtcloudDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* pcStruct,const C7Vector& pcTransformation,float& dist,C3Vector* meshMinDistPt=nullptr,C3Vector* pcMinDistPt=nullptr,int* meshCaching=nullptr,unsigned long long int* pcCaching=nullptr);
    static bool geomPlugin_getMeshTriangleDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr,int* caching=nullptr);
    static bool geomPlugin_getMeshSegmentDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr,int* caching=nullptr);
    static bool geomPlugin_getMeshPointDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& point,float& dist,C3Vector* minDistSegPt=nullptr,int* caching=nullptr);

    static bool geomPlugin_getOctreeOctreeDistanceIfSmaller(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,float& dist,C3Vector* oc1MinDistPt=nullptr,C3Vector* oc2MinDistPt=nullptr,unsigned long long int* oc1Caching=nullptr,unsigned long long int* oc2Caching=nullptr);
    static bool geomPlugin_getOctreePtcloudDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& pcTransformation,float& dist,C3Vector* ocMinDistPt=nullptr,C3Vector* pcMinDistPt=nullptr,unsigned long long int* ocCaching=nullptr,unsigned long long int* pcCaching=nullptr);
    static bool geomPlugin_getOctreeTriangleDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* ocMinDistPt=nullptr,C3Vector* triMinDistPt=nullptr,unsigned long long int* ocCaching=nullptr);
    static bool geomPlugin_getOctreeSegmentDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* ocMinDistPt=nullptr,C3Vector* segMinDistPt=nullptr,unsigned long long int* ocCaching=nullptr);
    static bool geomPlugin_getOctreePointDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,float& dist,C3Vector* ocMinDistPt=nullptr,unsigned long long int* ocCaching=nullptr);

    static bool geomPlugin_getPtcloudPtcloudDistanceIfSmaller(const void* pc1Struct,const C7Vector& pc1Transformation,const void* pc2Struct,const C7Vector& pc2Transformation,float& dist,C3Vector* pc1MinDistPt=nullptr,C3Vector* pc2MinDistPt=nullptr,unsigned long long int* pc1Caching=nullptr,unsigned long long int* pc2Caching=nullptr);
    static bool geomPlugin_getPtcloudTriangleDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* pcMinDistPt=nullptr,C3Vector* triMinDistPt=nullptr,unsigned long long int* pcCaching=nullptr);
    static bool geomPlugin_getPtcloudSegmentDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* pcMinDistPt=nullptr,C3Vector* segMinDistPt=nullptr,unsigned long long int* pcCaching=nullptr);
    static bool geomPlugin_getPtcloudPointDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& point,float& dist,C3Vector* pcMinDistPt=nullptr,unsigned long long int* pcCaching=nullptr);

    static float geomPlugin_getApproxBoxBoxDistance(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize);
    static bool geomPlugin_getBoxBoxDistanceIfSmaller(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid,float& dist,C3Vector* distSegPt1=nullptr,C3Vector* distSegPt2=nullptr,bool altRoutine=false);
    static bool geomPlugin_getBoxTriangleDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* distSegPt1=nullptr,C3Vector* distSegPt2=nullptr,bool altRoutine=false);
    static bool geomPlugin_getBoxSegmentDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* distSegPt1=nullptr,C3Vector* distSegPt2=nullptr,bool altRoutine=false);
    static bool geomPlugin_getBoxPointDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,float& dist,C3Vector* distSegPt1=nullptr);
    static float geomPlugin_getBoxPointDistance(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,C3Vector* distSegPt1=nullptr);

    static bool geomPlugin_getTriangleTriangleDistanceIfSmaller(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr);
    static bool geomPlugin_getTriangleSegmentDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr);
    static bool geomPlugin_getTrianglePointDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& point,float& dist,C3Vector* minDistSegPt=nullptr);

    static bool geomPlugin_getSegmentSegmentDistanceIfSmaller(const C3Vector& segment1EndPoint,const C3Vector& segment1Vector,const C3Vector& segment2EndPoint,const C3Vector& segment2Vector,float& dist,C3Vector* minDistSegPt1=nullptr,C3Vector* minDistSegPt2=nullptr);
    static bool geomPlugin_getSegmentPointDistanceIfSmaller(const C3Vector& segmentEndPoint,const C3Vector& segmentVector,const C3Vector& point,float& dist,C3Vector* minDistSegPt=nullptr);

    // Volume sensor
    static bool geomPlugin_volumeSensorDetectMeshIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* obbStruct,const C7Vector& meshTransformation,float& dist,bool fast=false,bool frontDetection=true,bool backDetection=true,float maxAngle=0.0f,C3Vector* detectPt=nullptr,C3Vector* triN=nullptr);
    static bool geomPlugin_volumeSensorDetectOctreeIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,bool fast=false,bool frontDetection=true,bool backDetection=true,float maxAngle=0.0f,C3Vector* detectPt=nullptr,C3Vector* triN=nullptr);
    static bool geomPlugin_volumeSensorDetectPtcloudIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* pcStruct,const C7Vector& ptcloudTransformation,float& dist,bool fast=false,C3Vector* detectPt=nullptr);
    static bool geomPlugin_volumeSensorDetectTriangleIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,bool frontDetection=true,bool backDetection=true,float maxAngle=0.0f,C3Vector* detectPt=nullptr,C3Vector* triN=nullptr);
    static bool geomPlugin_volumeSensorDetectSegmentIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,float maxAngle=0.0f,C3Vector* detectPt=nullptr);

    // Ray sensor
    static bool geomPlugin_raySensorDetectMeshIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* obbStruct,const C7Vector& meshTransformation,float& dist,float forbiddenDist=0.0f,bool fast=false,bool frontDetection=true,bool backDetection=true,float maxAngle=0.0f,C3Vector* detectPt=nullptr,C3Vector* triN=nullptr,bool* forbiddenDistTouched=nullptr);
    static bool geomPlugin_raySensorDetectOctreeIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,float forbiddenDist=0.0f,bool fast=false,bool frontDetection=true,bool backDetection=true,float maxAngle=0.0f,C3Vector* detectPt=nullptr,C3Vector* triN=nullptr,bool* forbiddenDistTouched=nullptr);

    // Volume-pt test
    static bool geomPlugin_isPointInVolume(const std::vector<float>& planesIn,const C3Vector& point);
    static bool geomPlugin_isPointInVolume1AndOutVolume2(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& point);

    // IK plugin:
    static CPlugin* currentIkPlugin;
    static int ikEnvironment;
    static bool isIkPluginAvailable();
    static void ikPlugin_emptyEnvironment();
    static void ikPlugin_eraseObject(int objectHandle);
    static void ikPlugin_setObjectParent(int objectHandle,int parentObjectHandle);
    static int ikPlugin_createDummy();
    static void ikPlugin_setLinkedDummy(int dummyHandle,int linkedDummyHandle);
    static int ikPlugin_createJoint(int jointType);
    static void ikPlugin_setJointMode(int jointHandle,int jointMode);
    static void ikPlugin_setJointInterval(int jointHandle,bool cyclic,float jMin,float jRange);
    static void ikPlugin_setJointScrewPitch(int jointHandle,float pitch);
    static void ikPlugin_setJointIkWeight(int jointHandle,float ikWeight);
    static void ikPlugin_setJointMaxStepSize(int jointHandle,float maxStepSize);
    static void ikPlugin_setJointDependency(int jointHandle,int dependencyJointHandle,float offset,float mult);
    static float ikPlugin_getJointPosition(int jointHandle);
    static void ikPlugin_setJointPosition(int jointHandle,float position);
    static C4Vector ikPlugin_getSphericalJointQuaternion(int jointHandle);
    static void ikPlugin_setSphericalJointQuaternion(int jointHandle,const C4Vector& quaternion);
    static int ikPlugin_createIkGroup();
    static void ikPlugin_eraseIkGroup(int ikGroupHandle);
    static void ikPlugin_setIkGroupFlags(int ikGroupHandle,int flags);
    static void ikPlugin_setIkGroupCalculation(int ikGroupHandle,int method,float damping,int maxIterations);
    static int ikPlugin_addIkElement(int ikGroupHandle,int tipHandle);
    static void ikPlugin_eraseIkElement(int ikGroupHandle,int ikElementIndex);
    static void ikPlugin_setIkElementFlags(int ikGroupHandle,int ikElementIndex,int flags);
    static void ikPlugin_setIkElementBase(int ikGroupHandle,int ikElementIndex,int baseHandle,int constraintsBaseHandle);
    static void ikPlugin_setIkElementConstraints(int ikGroupHandle,int ikElementIndex,int constraints);
    static void ikPlugin_setIkElementPrecision(int ikGroupHandle,int ikElementIndex,float linearPrecision,float angularPrecision);
    static void ikPlugin_setIkElementWeights(int ikGroupHandle,int ikElementIndex,float linearWeight,float angularWeight);
    static int ikPlugin_handleIkGroup(int ikGroupHandle);
    static bool ikPlugin_computeJacobian(int ikGroupHandle,int options);
    static CMatrix* ikPlugin_getJacobian(int ikGroupHandle);
    static float ikPlugin_getManipulability(int ikGroupHandle);
    static int ikPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,float thresholdDist,int maxIterationsOrTimeInMs,float* retConfig,const float* metric,bool(*validationCallback)(float*),const int* jointOptions,const float* lowLimits,const float* ranges,std::string& errSting);
    static int ikPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,float thresholdDist,int maxIterationsOrTimeInMs,float* retConfig,const float* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const float* lowLimits,const float* ranges,std::string& errSting);
    static C7Vector ikPlugin_getObjectLocalTransformation(int objectHandle);
    static void ikPlugin_setObjectLocalTransformation(int objectHandle,const C7Vector& tr);

    // code editor plugin:
    static CPlugin* currentCodeEditor;
    static bool isCodeEditorPluginAvailable();
    static bool codeEditor_openModal(const char* initText,const char* properties,std::string& modifiedText,int* positionAndSize);
    static int codeEditor_open(const char* initText,const char* properties);
    static int codeEditor_setText(int handle,const char* text,int insertMode);
    static bool codeEditor_getText(int handle,std::string& text,int* positionAndSize);
    static int codeEditor_show(int handle,int showState);
    static int codeEditor_close(int handle,int* positionAndSize);

    // Python plugin:
    static CPlugin* currentPythonPlugin;
    static void* pythonPlugin_initState(int scriptHandle,const char* scriptName);
    static void pythonPlugin_cleanupState(void* state);
    static int pythonPlugin_loadCode(void* state,const char* code,const char* functionsToFind,bool* functionsFound,std::string* errorMsg);
    static int pythonPlugin_callFunc(void* state,const char* funcName,int inStackHandle,int outStackHandle,std::string* errorMsg);
    static int pythonPlugin_execStr(void* state,const char* str,int outStackHandle);
    static int pythonPlugin_isDeprecated(const char* str);
    static void pythonPlugin_getFuncs(const char* str,std::vector<std::string>& v);
    static void pythonPlugin_getConsts(const char* str,std::vector<std::string>& v);
    static std::string pythonPlugin_getCalltip(const char* func);

    // Ruckig plugin:
    static CPlugin* currentRuckigPlugin;
    static int ruckigPlugin_pos(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetPos,const double* targetVel);
    static int ruckigPlugin_vel(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetVel);
    static int ruckigPlugin_step(int objHandle,double timeStep,double* newPos,double* newVel,double* newAccel,double* syncTime);
    static int ruckigPlugin_remove(int objHandle);
    static int ruckigPlugin_dofs(int objHandle);

    // Custom UI plugin:
    static CPlugin* currentCustomUi;
    static bool isCustomUiPluginAvailable();
    static int customUi_msgBox(int type, int buttons, const char *title, const char *message,int defaultAnswer);
    static bool customUi_fileDialog(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext, int native,std::string& files);

    // Assimp plugin:
    static CPlugin* currentAssimp;
    static bool isAssimpPluginAvailable();
    static int* assimp_importShapes(const char* fileNames,int maxTextures,float scaling,int upVector,int options,int* shapeCount);
    static void assimp_exportShapes(const int* shapeHandles,int shapeCount,const char* filename,const char* format,float scaling,int upVector,int options);
    static int assimp_importMeshes(const char* fileNames,float scaling,int upVector,int options,float*** allVertices,int** verticesSizes,int*** allIndices,int** indicesSizes);
    static void assimp_exportMeshes(int meshCnt,const float** allVertices,const int* verticesSizes,const int** allIndices,const int* indicesSizes,const char* filename,const char* format,float scaling,int upVector,int options);


    static bool enableOrDisableSpecificEventCallback(int eventCallbackType,const char* pluginName);
    static void sendSpecialEventCallbackMessageToSomePlugins(int msg,int* auxVals,void* data,int retVals[4]);
    static void sendSpecialEventCallbackMessageToSomePlugins(int msg,int auxVal0,int auxVal1,int auxVal2,int auxVal3);
    static bool shouldSend_openglframe_msg();
    static bool shouldSend_openglcameraview_msg();

    static ptrExtRenderer _povRayAddress;
    static ptrExtRenderer _extRendererAddress;
    static ptrExtRenderer _extRendererWindowedAddress;
    static ptrExtRenderer _openGl3Address;
    static ptrExtRenderer _openGl3WindowedAddress;
    static ptrExtRenderer _activeExtRendererAddress;

    static ptrQhull _qhullAddress;
    static ptrHACD _hacdAddress;
    static ptrVHACD _vhacdAddress;
    static ptrMeshDecimator _meshDecimatorAddress;

private:
    static int _nextHandle;
    static std::vector<CPlugin*> _allPlugins;

    static std::vector<std::string> _renderingpass_eventEnabledPluginNames;
    static std::vector<std::string> _opengl_eventEnabledPluginNames;
    static std::vector<std::string> _openglframe_eventEnabledPluginNames;
    static std::vector<std::string> _openglcameraview_eventEnabledPluginNames;

};
