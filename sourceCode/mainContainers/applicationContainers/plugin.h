#pragma once

#include <vVarious.h>
#include <vector>
#include <unordered_set>
#include <simLib/simTypes.h>
#include <pluginCallbackContainer.h>
#include <pluginVariableContainer.h>

typedef int(__cdecl *ptrInit)(SSimInit *);
typedef void(__cdecl *ptrCleanup)(void);
typedef void(__cdecl *ptrMsg)(SSimMsg *);

typedef void(__cdecl *ptrInit_ui)(void);
typedef void(__cdecl *ptrCleanup_ui)(void);
typedef void(__cdecl *ptrMsg_ui)(SSimMsg_ui *);

typedef void(__cdecl *ptrExtRenderer)(int, void *);

typedef void(__cdecl *ptrQhull)(void *);

typedef void(__cdecl *ptrHACD)(void *);
typedef void(__cdecl *ptrVHACD)(void *);

typedef void(__cdecl *ptrMeshDecimator)(void *);

typedef char(__cdecl *ptr_dynPlugin_startSimulation_D)(const double *, const int *);
typedef void(__cdecl *ptr_dynPlugin_endSimulation)(void);
typedef void(__cdecl *ptr_dynPlugin_step_D)(double, double);
typedef char(__cdecl *ptr_dynPlugin_isInitialized)(void);
typedef char(__cdecl *ptr_dynPlugin_isDynamicContentAvailable)(void);
typedef void(__cdecl *ptr_dynPlugin_serializeDynamicContent)(const char *, int);
typedef int(__cdecl *ptr_dynPlugin_addParticleObject_D)(int, double, double, const void *, double, int, const float *,
                                                        const float *, const float *, const float *);
typedef char(__cdecl *ptr_dynPlugin_removeParticleObject)(int);
typedef char(__cdecl *ptr_dynPlugin_addParticleObjectItem_D)(int, const double *, double);
typedef int(__cdecl *ptr_dynPlugin_getParticleObjectOtherFloatsPerItem)(int);
typedef double *(__cdecl *ptr_dynPlugin_getContactPoints_D)(int *);
typedef void **(__cdecl *ptr_dynPlugin_getParticles)(int, int *, int *, float **);
typedef char(__cdecl *ptr_dynPlugin_getParticleData_D)(const void *, double *, double *, int *, float **);
typedef char(__cdecl *ptr_dynPlugin_getContactForce_D)(int, int, int, int *, double *);
typedef int(__cdecl *ptr_dynPlugin_getDynamicStepDivider)(void);
typedef double(__cdecl *ptr_mujocoPlugin_computeInertia)(int, double *, double *, double *);
typedef double(__cdecl *ptr_mujocoPlugin_computePMI)(const double *, int, const int *, int, double *, double *,
                                                     double *);
typedef void(__cdecl *ptr_dynPlugin_engine)(void);

typedef void(__cdecl *ptr_geomPlugin_releaseBuffer)(void *buff);
typedef void *(__cdecl *ptr_geomPlugin_createMesh)(const double *vertices, int verticesSize, const int *indices,
                                                   int indicesSize, const double meshOrigin[7],
                                                   double triangleEdgeMaxLength, int maxTrianglesInBoundingBox);
typedef void *(__cdecl *ptr_geomPlugin_copyMesh)(const void *meshObbStruct);
typedef void *(__cdecl *ptr_geomPlugin_getMeshFromSerializationData)(const unsigned char *serializationData);
typedef unsigned char *(__cdecl *ptr_geomPlugin_getMeshSerializationData)(const void *meshObbStruct,
                                                                          int *serializationDataSize);
typedef void(__cdecl *ptr_geomPlugin_scaleMesh)(void *meshObbStruct, double scalingFactor);
typedef void(__cdecl *ptr_geomPlugin_destroyMesh)(void *meshObbStruct);
typedef double(__cdecl *ptr_geomPlugin_getMeshRootObbVolume)(const void *meshObbStruct);
typedef void *(__cdecl *ptr_geomPlugin_createOctreeFromPoints)(const double *points, int pointCnt,
                                                               const double octreeOrigin[7], double cellS,
                                                               const unsigned char rgbData[3], unsigned int usrData);
typedef void *(__cdecl *ptr_geomPlugin_createOctreeFromColorPoints)(const double *points, int pointCnt,
                                                                    const double octreeOrigin[7], double cellS,
                                                                    const unsigned char *rgbData,
                                                                    const unsigned int *usrData);
typedef void *(__cdecl *ptr_geomPlugin_createOctreeFromMesh)(const void *meshObbStruct,
                                                             const double meshTransformation[7],
                                                             const double octreeOrigin[7], double cellS,
                                                             const unsigned char rgbData[3], unsigned int usrData);
typedef void *(__cdecl *ptr_geomPlugin_createOctreeFromOctree)(const void *otherOctreeStruct,
                                                               const double otherOctreeTransformation[7],
                                                               const double newOctreeOrigin[7], double newOctreeCellS,
                                                               const unsigned char rgbData[3], unsigned int usrData);
typedef void *(__cdecl *ptr_geomPlugin_copyOctree)(const void *ocStruct);
typedef void *(__cdecl *ptr_geomPlugin_getOctreeFromSerializationData)(const unsigned char *serializationData);
typedef unsigned char *(__cdecl *ptr_geomPlugin_getOctreeSerializationData)(const void *ocStruct,
                                                                            int *serializationDataSize);
typedef void *(__cdecl *ptr_geomPlugin_getOctreeFromSerializationData_float)(const unsigned char *serializationData);
typedef unsigned char *(__cdecl *ptr_geomPlugin_getOctreeSerializationData_float)(const void *ocStruct,
                                                                                  int *serializationDataSize);
typedef void(__cdecl *ptr_geomPlugin_scaleOctree)(void *ocStruct, double f);
typedef void(__cdecl *ptr_geomPlugin_destroyOctree)(void *ocStruct);
typedef double *(__cdecl *ptr_geomPlugin_getOctreeVoxelData)(const void *ocStruct, int *voxelCount);
typedef unsigned int *(__cdecl *ptr_geomPlugin_getOctreeUserData)(const void *ocStruct, int *voxelCount);
typedef double *(__cdecl *ptr_geomPlugin_getOctreeCornersFromOctree)(const void *ocStruct, int *pointCount);
typedef void(__cdecl *ptr_geomPlugin_insertPointsIntoOctree)(void *ocStruct, const double octreeTransformation[7],
                                                             const double *points, int pointCnt,
                                                             const unsigned char rgbData[3], unsigned int usrData);
typedef void(__cdecl *ptr_geomPlugin_insertColorPointsIntoOctree)(void *ocStruct, const double octreeTransformation[7],
                                                                  const double *points, int pointCnt,
                                                                  const unsigned char *rgbData,
                                                                  const unsigned int *usrData);
typedef void(__cdecl *ptr_geomPlugin_insertMeshIntoOctree)(void *ocStruct, const double octreeTransformation[7],
                                                           const void *obbStruct, const double meshTransformation[7],
                                                           const unsigned char rgbData[3], unsigned int usrData);
typedef void(__cdecl *ptr_geomPlugin_insertOctreeIntoOctree)(void *oc1Struct, const double octree1Transformation[7],
                                                             const void *oc2Struct,
                                                             const double octree2Transformation[7],
                                                             const unsigned char rgbData[3], unsigned int usrData);
typedef bool(__cdecl *ptr_geomPlugin_removePointsFromOctree)(void *ocStruct, const double octreeTransformation[7],
                                                             const double *points, int pointCnt);
typedef bool(__cdecl *ptr_geomPlugin_removeMeshFromOctree)(void *ocStruct, const double octreeTransformation[7],
                                                           const void *obbStruct, const double meshTransformation[7]);
typedef bool(__cdecl *ptr_geomPlugin_removeOctreeFromOctree)(void *oc1Struct, const double octree1Transformation[7],
                                                             const void *oc2Struct,
                                                             const double octree2Transformation[7]);
typedef void *(__cdecl *ptr_geomPlugin_createPtcloudFromPoints)(const double *points, int pointCnt,
                                                                const double ptcloudOrigin[7], double cellS,
                                                                int maxPointCnt, const unsigned char rgbData[3],
                                                                double proximityTol);
typedef void *(__cdecl *ptr_geomPlugin_createPtcloudFromColorPoints)(const double *points, int pointCnt,
                                                                     const double ptcloudOrigin[7], double cellS,
                                                                     int maxPointCnt, const unsigned char *rgbData,
                                                                     double proximityTol);
typedef void *(__cdecl *ptr_geomPlugin_copyPtcloud)(const void *pcStruct);
typedef void *(__cdecl *ptr_geomPlugin_getPtcloudFromSerializationData)(const unsigned char *serializationData);
typedef unsigned char *(__cdecl *ptr_geomPlugin_getPtcloudSerializationData)(const void *pcStruct,
                                                                             int *serializationDataSize);
typedef void *(__cdecl *ptr_geomPlugin_getPtcloudFromSerializationData_float)(const unsigned char *serializationData);
typedef unsigned char *(__cdecl *ptr_geomPlugin_getPtcloudSerializationData_float)(const void *pcStruct,
                                                                                   int *serializationDataSize);
typedef void(__cdecl *ptr_geomPlugin_scalePtcloud)(void *pcStruct, double f);
typedef void(__cdecl *ptr_geomPlugin_destroyPtcloud)(void *pcStruct);
typedef double *(__cdecl *ptr_geomPlugin_getPtcloudPoints)(const void *pcStruct, int *pointCount, double prop);
typedef double *(__cdecl *ptr_geomPlugin_getPtcloudOctreeCorners)(const void *pcStruct, int *pointCount);
typedef int(__cdecl *ptr_geomPlugin_getPtcloudNonEmptyCellCount)(const void *pcStruct);
typedef void(__cdecl *ptr_geomPlugin_insertPointsIntoPtcloud)(void *pcStruct, const double ptcloudTransformation[7],
                                                              const double *points, int pointCnt,
                                                              const unsigned char rgbData[3], double proximityTol);
typedef void(__cdecl *ptr_geomPlugin_insertColorPointsIntoPtcloud)(void *pcStruct,
                                                                   const double ptcloudTransformation[7],
                                                                   const double *points, int pointCnt,
                                                                   const unsigned char *rgbData, double proximityTol);
typedef bool(__cdecl *ptr_geomPlugin_removePointsFromPtcloud)(void *pcStruct, const double ptcloudTransformation[7],
                                                              const double *points, int pointCnt, double proximityTol,
                                                              int *countRemoved);
typedef bool(__cdecl *ptr_geomPlugin_removeOctreeFromPtcloud)(void *pcStruct, const double ptcloudTransformation[7],
                                                              const void *ocStruct,
                                                              const double octreeTransformation[7], int *countRemoved);
typedef bool(__cdecl *ptr_geomPlugin_intersectPointsWithPtcloud)(void *pcStruct, const double ptcloudTransformation[7],
                                                                 const double *points, int pointCnt,
                                                                 double proximityTol);
typedef bool(__cdecl *ptr_geomPlugin_getMeshMeshCollision)(const void *mesh1ObbStruct,
                                                           const double mesh1Transformation[7],
                                                           const void *mesh2ObbStruct,
                                                           const double mesh2Transformation[7], double **intersections,
                                                           int *intersectionsSize, int *mesh1Caching,
                                                           int *mesh2Caching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshOctreeCollision)(const void *meshObbStruct,
                                                             const double meshTransformation[7], const void *ocStruct,
                                                             const double octreeTransformation[7], int *meshCaching,
                                                             unsigned long long int *ocCaching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshTriangleCollision)(const void *meshObbStruct,
                                                               const double meshTransformation[7], const double p[3],
                                                               const double v[3], const double w[3],
                                                               double **intersections, int *intersectionsSize,
                                                               int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshSegmentCollision)(const void *meshObbStruct,
                                                              const double meshTransformation[7],
                                                              const double segmentExtremity[3],
                                                              const double segmentVector[3], double **intersections,
                                                              int *intersectionsSize, int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeOctreeCollision)(
    const void *oc1Struct, const double octree1Transformation[7], const void *oc2Struct,
    const double octree2Transformation[7], unsigned long long int *oc1Caching, unsigned long long int *oc2Caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreePtcloudCollision)(
    const void *ocStruct, const double octreeTransformation[7], const void *pcStruct,
    const double ptcloudTransformation[7], unsigned long long int *ocCaching, unsigned long long int *pcCaching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeTriangleCollision)(const void *ocStruct,
                                                                 const double octreeTransformation[7],
                                                                 const double p[3], const double v[3],
                                                                 const double w[3], unsigned long long int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeSegmentCollision)(const void *ocStruct,
                                                                const double octreeTransformation[7],
                                                                const double segmentExtremity[3],
                                                                const double segmentVector[3],
                                                                unsigned long long int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreePointsCollision)(const void *ocStruct,
                                                               const double octreeTransformation[7],
                                                               const double *points, int pointCount);
typedef bool(__cdecl *ptr_geomPlugin_getOctreePointCollision)(const void *ocStruct,
                                                              const double octreeTransformation[7],
                                                              const double point[3], unsigned *usrData,
                                                              unsigned long long int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getBoxBoxCollision)(const double box1Transformation[7],
                                                         const double box1HalfSize[3],
                                                         const double box2Transformation[7],
                                                         const double box2HalfSize[3], bool boxesAreSolid);
typedef bool(__cdecl *ptr_geomPlugin_getBoxTriangleCollision)(const double boxTransformation[7],
                                                              const double boxHalfSize[3], bool boxIsSolid,
                                                              const double p[3], const double v[3], const double w[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxSegmentCollision)(const double boxTransformation[7],
                                                             const double boxHalfSize[3], bool boxIsSolid,
                                                             const double segmentEndPoint[3],
                                                             const double segmentVector[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxPointCollision)(const double boxTransformation[7],
                                                           const double boxHalfSize[3], const double point[3]);
typedef bool(__cdecl *ptr_geomPlugin_getTriangleTriangleCollision)(const double p1[3], const double v1[3],
                                                                   const double w1[3], const double p2[3],
                                                                   const double v2[3], const double w2[3],
                                                                   double **intersections, int *intersectionsSize);
typedef bool(__cdecl *ptr_geomPlugin_getTriangleSegmentCollision)(const double p[3], const double v[3],
                                                                  const double w[3], const double segmentEndPoint[3],
                                                                  const double segmentVector[3], double **intersections,
                                                                  int *intersectionsSize);
typedef bool(__cdecl *ptr_geomPlugin_getMeshMeshDistanceIfSmaller)(const void *mesh1ObbStruct,
                                                                   const double mesh1Transformation[7],
                                                                   const void *mesh2ObbStruct,
                                                                   const double mesh2Transformation[7], double *dist,
                                                                   double minDistSegPt1[3], double minDistSegPt2[3],
                                                                   int *mesh1Caching, int *mesh2Caching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshOctreeDistanceIfSmaller)(
    const void *meshObbStruct, const double meshTransformation[7], const void *ocStruct,
    const double octreeTransformation[7], double *dist, double meshMinDistPt[3], double ocMinDistPt[3],
    int *meshCaching, unsigned long long int *ocCaching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshPtcloudDistanceIfSmaller)(
    const void *meshObbStruct, const double meshTransformation[7], const void *pcStruct,
    const double pcTransformation[7], double *dist, double meshMinDistPt[3], double pcMinDistPt[3], int *meshCaching,
    unsigned long long int *pcCaching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshTriangleDistanceIfSmaller)(
    const void *meshObbStruct, const double meshTransformation[7], const double p[3], const double v[3],
    const double w[3], double *dist, double minDistSegPt1[3], double minDistSegPt2[3], int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshSegmentDistanceIfSmaller)(
    const void *meshObbStruct, const double meshTransformation[7], const double segmentEndPoint[3],
    const double segmentVector[3], double *dist, double minDistSegPt1[3], double minDistSegPt2[3], int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getMeshPointDistanceIfSmaller)(const void *meshObbStruct,
                                                                    const double meshTransformation[7],
                                                                    const double point[3], double *dist,
                                                                    double minDistSegPt[3], int *caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeOctreeDistanceIfSmaller)(
    const void *oc1Struct, const double octree1Transformation[7], const void *oc2Struct,
    const double octree2Transformation[7], double *dist, double oc1MinDistPt[3], double oc2MinDistPt[3],
    unsigned long long int *oc1Caching, unsigned long long int *oc2Caching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreePtcloudDistanceIfSmaller)(
    const void *ocStruct, const double octreeTransformation[7], const void *pcStruct, const double pcTransformation[7],
    double *dist, double ocMinDistPt[3], double pcMinDistPt[3], unsigned long long int *ocCaching,
    unsigned long long int *pcCaching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeTriangleDistanceIfSmaller)(
    const void *ocStruct, const double octreeTransformation[7], const double p[3], const double v[3], const double w[3],
    double *dist, double ocMinDistPt[3], double triMinDistPt[3], unsigned long long int *ocCaching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreeSegmentDistanceIfSmaller)(const void *ocStruct,
                                                                        const double octreeTransformation[7],
                                                                        const double segmentEndPoint[3],
                                                                        const double segmentVector[3], double *dist,
                                                                        double ocMinDistPt[3], double segMinDistPt[3],
                                                                        unsigned long long int *ocCaching);
typedef bool(__cdecl *ptr_geomPlugin_getOctreePointDistanceIfSmaller)(const void *ocStruct,
                                                                      const double octreeTransformation[7],
                                                                      const double point[3], double *dist,
                                                                      double ocMinDistPt[3],
                                                                      unsigned long long int *ocCaching);
typedef bool(__cdecl *ptr_geomPlugin_getPtcloudPtcloudDistanceIfSmaller)(
    const void *pc1Struct, const double pc1Transformation[7], const void *pc2Struct, const double pc2Transformation[7],
    double *dist, double *pc1MinDistPt, double *pc2MinDistPt, unsigned long long int *pc1Caching,
    unsigned long long int *pc2Caching);
typedef bool(__cdecl *ptr_geomPlugin_getPtcloudTriangleDistanceIfSmaller)(
    const void *pcStruct, const double pcTransformation[7], const double p[3], const double v[3], const double w[3],
    double *dist, double *pcMinDistPt, double *triMinDistPt, unsigned long long int *pcCaching);
typedef bool(__cdecl *ptr_geomPlugin_getPtcloudSegmentDistanceIfSmaller)(const void *pcStruct,
                                                                         const double pcTransformation[7],
                                                                         const double segmentEndPoint[3],
                                                                         const double segmentVector[3], double *dist,
                                                                         double *pcMinDistPt, double *segMinDistPt,
                                                                         unsigned long long int *pcCaching);
typedef bool(__cdecl *ptr_geomPlugin_getPtcloudPointDistanceIfSmaller)(const void *pcStruct,
                                                                       const double pcTransformation[7],
                                                                       const double point[3], double *dist,
                                                                       double *pcMinDistPt,
                                                                       unsigned long long int *pcCaching);
typedef double(__cdecl *ptr_geomPlugin_getApproxBoxBoxDistance)(const double box1Transformation[7],
                                                                const double box1HalfSize[3],
                                                                const double box2Transformation[7],
                                                                const double box2HalfSize[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxBoxDistanceIfSmaller)(
    const double box1Transformation[7], const double box1HalfSize[3], const double box2Transformation[7],
    const double box2HalfSize[3], bool boxesAreSolid, double *dist, double distSegPt1[3], double distSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxTriangleDistanceIfSmaller)(const double boxTransformation[7],
                                                                      const double boxHalfSize[3], bool boxIsSolid,
                                                                      const double p[3], const double v[3],
                                                                      const double w[3], double *dist,
                                                                      double distSegPt1[3], double distSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxSegmentDistanceIfSmaller)(const double boxTransformation[7],
                                                                     const double boxHalfSize[3], bool boxIsSolid,
                                                                     const double segmentEndPoint[3],
                                                                     const double segmentVector[3], double *dist,
                                                                     double distSegPt1[3], double distSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getBoxPointDistanceIfSmaller)(const double boxTransformation[7],
                                                                   const double boxHalfSize[3], bool boxIsSolid,
                                                                   const double point[3], double *dist,
                                                                   double distSegPt1[3]);
typedef bool(__cdecl *ptr_geomPlugin_getTriangleTriangleDistanceIfSmaller)(const double p1[3], const double v1[3],
                                                                           const double w1[3], const double p2[3],
                                                                           const double v2[3], const double w2[3],
                                                                           double *dist, double minDistSegPt1[3],
                                                                           double minDistSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getTriangleSegmentDistanceIfSmaller)(
    const double p[3], const double v[3], const double w[3], const double segmentEndPoint[3],
    const double segmentVector[3], double *dist, double minDistSegPt1[3], double minDistSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getTrianglePointDistanceIfSmaller)(const double p[3], const double v[3],
                                                                        const double w[3], const double point[3],
                                                                        double *dist, double minDistSegPt[3]);
typedef bool(__cdecl *ptr_geomPlugin_getSegmentSegmentDistanceIfSmaller)(
    const double segment1EndPoint[3], const double segment1Vector[3], const double segment2EndPoint[3],
    const double segment2Vector[3], double *dist, double minDistSegPt1[3], double minDistSegPt2[3]);
typedef bool(__cdecl *ptr_geomPlugin_getSegmentPointDistanceIfSmaller)(const double segmentEndPoint[3],
                                                                       const double segmentVector[3],
                                                                       const double point[3], double *dist,
                                                                       double minDistSegPt[3]);
typedef bool(__cdecl *ptr_geomPlugin_volumeSensorDetectMeshIfSmaller)(
    const double *planesIn, int planesInSize, const double *planesOut, int planesOutSize, const void *obbStruct,
    const double meshTransformationRelative[7], double *dist, bool fast, bool frontDetection, bool backDetection,
    double maxAngle, double detectPt[3], double triN[3]);
typedef bool(__cdecl *ptr_geomPlugin_volumeSensorDetectOctreeIfSmaller)(
    const double *planesIn, int planesInSize, const double *planesOut, int planesOutSize, const void *ocStruct,
    const double octreeTransformationRelative[7], double *dist, bool fast, bool frontDetection, bool backDetection,
    double maxAngle, double detectPt[3], double triN[3]);
typedef bool(__cdecl *ptr_geomPlugin_volumeSensorDetectPtcloudIfSmaller)(const double *planesIn, int planesInSize,
                                                                         const double *planesOut, int planesOutSize,
                                                                         const void *pcStruct,
                                                                         const double ptcloudTransformationRelative[7],
                                                                         double *dist, bool fast, double detectPt[3]);
typedef bool(__cdecl *ptr_geomPlugin_volumeSensorDetectTriangleIfSmaller)(
    const double *planesIn, int planesInSize, const double *planesOut, int planesOutSize, const double p[3],
    const double v[3], const double w[3], double *dist, bool frontDetection, bool backDetection, double maxAngle,
    double detectPt[3], double triN[3]);
typedef bool(__cdecl *ptr_geomPlugin_volumeSensorDetectSegmentIfSmaller)(const double *planesIn, int planesInSize,
                                                                         const double *planesOut, int planesOutSize,
                                                                         const double segmentEndPoint[3],
                                                                         const double segmentVector[3], double *dist,
                                                                         double maxAngle, double detectPt[3]);
typedef bool(__cdecl *ptr_geomPlugin_raySensorDetectMeshIfSmaller)(
    const double rayStart[3], const double rayVect[3], const void *obbStruct,
    const double meshTransformationRelative[7], double *dist, double forbiddenDist, bool fast, bool frontDetection,
    bool backDetection, double maxAngle, double detectPt[3], double triN[3], bool *forbiddenDistTouched);
typedef bool(__cdecl *ptr_geomPlugin_raySensorDetectOctreeIfSmaller)(
    const double rayStart[3], const double rayVect[3], const void *ocStruct,
    const double octreeTransformationRealtive[7], double *dist, double forbiddenDist, bool fast, bool frontDetection,
    bool backDetection, double maxAngle, double detectPt[3], double triN[3], bool *forbiddenDistTouched);
typedef bool(__cdecl *ptr_geomPlugin_isPointInVolume)(const double *planesIn, int planesInSize, const double point[3]);

typedef int(__cdecl *ptr_oldIkPlugin_createEnv)();
// typedef bool (__cdecl *ptr_oldIkPlugin_switchEnvironment)(int handle);
typedef void(__cdecl *ptr_oldIkPlugin_eraseEnvironment)(int ikEnv);
typedef void(__cdecl *ptr_oldIkPlugin_eraseObject)(int ikEnv, int objectHandle);
typedef void(__cdecl *ptr_oldIkPlugin_setObjectParent)(int ikEnv, int objectHandle, int parentObjectHandle);
typedef int(__cdecl *ptr_oldIkPlugin_createDummy)(int ikEnv);
typedef void(__cdecl *ptr_oldIkPlugin_setLinkedDummy)(int ikEnv, int dummyHandle, int linkedDummyHandle);
typedef int(__cdecl *ptr_oldIkPlugin_createJoint)(int ikEnv, int jointType);
typedef void(__cdecl *ptr_oldIkPlugin_setJointMode)(int ikEnv, int jointHandle, int jointMode);
typedef void(__cdecl *ptr_oldIkPlugin_setJointInterval)(int ikEnv, int jointHandle, bool cyclic,
                                                        const double *intervalMinAndRange);
typedef void(__cdecl *ptr_oldIkPlugin_setJointScrewPitch)(int ikEnv, int jointHandle, double pitch);
typedef void(__cdecl *ptr_oldIkPlugin_setJointIkWeight)(int ikEnv, int jointHandle, double ikWeight);
typedef void(__cdecl *ptr_oldIkPlugin_setJointMaxStepSize)(int ikEnv, int jointHandle, double maxStepSize);
typedef void(__cdecl *ptr_oldIkPlugin_setJointDependency)(int ikEnv, int jointHandle, int dependencyJointHandle,
                                                          double offset, double mult);
typedef double(__cdecl *ptr_oldIkPlugin_getJointPosition)(int ikEnv, int jointHandle);
typedef void(__cdecl *ptr_oldIkPlugin_setJointPosition)(int ikEnv, int jointHandle, double position);
typedef void(__cdecl *ptr_oldIkPlugin_getSphericalJointQuaternion)(int ikEnv, int jointHandle, double *quaternion);
typedef void(__cdecl *ptr_oldIkPlugin_setSphericalJointQuaternion)(int ikEnv, int jointHandle,
                                                                   const double *quaternion);
typedef int(__cdecl *ptr_oldIkPlugin_createIkGroup)(int ikEnv);
typedef void(__cdecl *ptr_oldIkPlugin_eraseIkGroup)(int ikEnv, int ikGroupHandle);
typedef void(__cdecl *ptr_oldIkPlugin_setIkGroupFlags)(int ikEnv, int ikGroupHandle, int flags);
typedef void(__cdecl *ptr_oldIkPlugin_setIkGroupCalculation)(int ikEnv, int ikGroupHandle, int method, double damping,
                                                             int maxIterations);
typedef int(__cdecl *ptr_oldIkPlugin_addIkElement)(int ikEnv, int ikGroupHandle, int tipHandle);
typedef void(__cdecl *ptr_oldIkPlugin_eraseIkElement)(int ikEnv, int ikGroupHandle, int ikElementIndex);
typedef void(__cdecl *ptr_oldIkPlugin_setIkElementFlags)(int ikEnv, int ikGroupHandle, int ikElementIndex, int flags);
typedef void(__cdecl *ptr_oldIkPlugin_setIkElementBase)(int ikEnv, int ikGroupHandle, int ikElementIndex,
                                                        int baseHandle, int constraintsBaseHandle);
typedef void(__cdecl *ptr_oldIkPlugin_setIkElementConstraints)(int ikEnv, int ikGroupHandle, int ikElementIndex,
                                                               int constraints);
typedef void(__cdecl *ptr_oldIkPlugin_setIkElementPrecision)(int ikEnv, int ikGroupHandle, int ikElementIndex,
                                                             double linearPrecision, double angularPrecision);
typedef void(__cdecl *ptr_oldIkPlugin_setIkElementWeights)(int ikEnv, int ikGroupHandle, int ikElementIndex,
                                                           double linearWeight, double angularWeight);
typedef int(__cdecl *ptr_oldIkPlugin_handleIkGroup)(int ikEnv, int ikGroupHandle);
typedef bool(__cdecl *ptr_oldIkPlugin_computeJacobian)(int ikEnv, int ikGroupHandle, int options);
typedef double *(__cdecl *ptr_oldIkPlugin_getJacobian)(int ikEnv, int ikGroupHandle, int *matrixSize);
typedef double(__cdecl *ptr_oldIkPlugin_getManipulability)(int ikEnv, int ikGroupHandle);
typedef char *(__cdecl *ptr_oldIkPlugin_getConfigForTipPose)(int ikEnv, int ikGroupHandle, int jointCnt,
                                                             const int *jointHandles, double thresholdDist,
                                                             int maxIterations, int *result, double *retConfig,
                                                             const double *metric, bool (*validationCallback)(double *),
                                                             const int *jointOptions, const double *lowLimits,
                                                             const double *ranges);
typedef void(__cdecl *ptr_oldIkPlugin_getObjectLocalTransformation)(int ikEnv, int objectHandle, double *pos,
                                                                    double *quat);
typedef void(__cdecl *ptr_oldIkPlugin_setObjectLocalTransformation)(int ikEnv, int objectHandle, const double *pos,
                                                                    const double *quat);

typedef char *(__cdecl *ptrCodeEditor_openModal)(const char *initText, const char *properties, int *positionAndSize);
typedef int(__cdecl *ptrCodeEditor_open)(const char *initText, const char *properties);
typedef int(__cdecl *ptrCodeEditor_setText)(int handle, const char *text, int insertMode);
typedef char *(__cdecl *ptrCodeEditor_getText)(int handle, int *positionAndSize);
typedef int(__cdecl *ptrCodeEditor_show)(int handle, int showState);
typedef int(__cdecl *ptrCodeEditor_close)(int handle, int *positionAndSize);

typedef int(__cdecl *ptrRuckigPlugin_pos)(int scriptHandle, int dofs, double smallestTimeStep, int flags,
                                          const double *currentPos, const double *currentVel,
                                          const double *currentAccel, const double *maxVel, const double *maxAccel,
                                          const double *maxJerk, const bool *selection, const double *targetPos,
                                          const double *targetVel);
typedef int(__cdecl *ptrRuckigPlugin_vel)(int scriptHandle, int dofs, double smallestTimeStep, int flags,
                                          const double *currentPos, const double *currentVel,
                                          const double *currentAccel, const double *maxAccel, const double *maxJerk,
                                          const bool *selection, const double *targetVel);
typedef int(__cdecl *ptrRuckigPlugin_step)(int objHandle, double timeStep, double *newPos, double *newVel,
                                           double *newAccel, double *syncTime);
typedef int(__cdecl *ptrRuckigPlugin_remove)(int objHandle);
typedef int(__cdecl *ptrRuckigPlugin_dofs)(int objHandle);

// Following 2 only used with deprecated API functions:
typedef int(__cdecl *ptrCustomUi_msgBox)(int type, int buttons, const char *title, const char *message);
typedef char *(__cdecl *ptrCustomUi_fileDialog)(int type, const char *title, const char *startPath,
                                                const char *initName, const char *extName, const char *ext, int native);

typedef int *(__cdecl *ptrassimp_importShapes)(const char *fileNames, int maxTextures, double scaling, int upVector,
                                               int options, int *shapeCount);
typedef void(__cdecl *ptrassimp_exportShapes)(const int *shapeHandles, int shapeCount, const char *filename,
                                              const char *format, double scaling, int upVector, int options);
typedef int(__cdecl *ptrassimp_importMeshes)(const char *fileNames, double scaling, int upVector, int options,
                                             double ***allVertices, int **verticesSizes, int ***allIndices,
                                             int **indicesSizes);
typedef void(__cdecl *ptrassimp_exportMeshes)(int meshCnt, const double **allVertices, const int *verticesSizes,
                                              const int **allIndices, const int *indicesSizes, const char *filename,
                                              const char *format, double scaling, int upVector, int options);

// old:
typedef unsigned char(__cdecl *ptrStart)(void *, int);
typedef void(__cdecl *ptrEnd)(void);
typedef void *(__cdecl *ptrMessage)(int, int *, void *, int *);

class CPlugin
{
  public:
    enum
    {
        stage_none = -1,
        stage_siminitdone = 0,
        stage_uiinitdone = 1, // set by UI thread
        stage_docleanup = 2,
        stage_uicleanupdone = 3, // set by UI thread
    };

    CPlugin(const char *filename, const char *pluginnamespaceAndVersion, int loadOrigin);
    virtual ~CPlugin();
    int load(std::string *errStr);

    bool init(std::string *errStr);
    bool msg(int msgId, int *auxData = nullptr, void *auxPointer = nullptr, int *reserved_legacy = nullptr);
    void cleanup();

    void init_ui();
    void msg_ui(int msgId, int *auxData = nullptr, void *auxPointer = nullptr);
    void cleanup_ui();

    int loadAndInit_old(std::string *errStr);
    void setConsoleVerbosity(int level);
    int getConsoleVerbosity() const;
    void setStatusbarVerbosity(int level);
    int getStatusbarVerbosity() const;
    std::string getName() const;
    std::string getNamespace() const;
    int getPluginVersion() const;
    std::string getExtendedVersionString() const;
    void setExtendedVersionString(const char *str);
    std::string getBuildDateString() const;
    void setBuildDateString(const char *str);
    int getExtendedVersionInt() const;
    void setExtendedVersionInt(int v);
    int getHandle() const;
    bool isLegacyPlugin() const;
    bool isUiPlugin() const;
    void setHandle(int h);
    void pushCurrentPlugin();
    void popCurrentPlugin();
    CPluginCallbackContainer *getPluginCallbackContainer();
    CPluginVariableContainer *getPluginVariableContainer();
    void addDependency(int loadOrigin);
    void removeDependency(int loadOrigin);
    bool hasDependency(int loadOrigin) const;
    std::string getDependencies() const;
    bool hasAnyDependency() const;

    ptr_dynPlugin_startSimulation_D dynPlugin_startSimulation;
    ptr_dynPlugin_endSimulation dynPlugin_endSimulation;
    ptr_dynPlugin_step_D dynPlugin_step;
    ptr_dynPlugin_isDynamicContentAvailable dynPlugin_isDynamicContentAvailable;
    ptr_dynPlugin_serializeDynamicContent dynPlugin_serializeDynamicContent;
    ptr_dynPlugin_addParticleObject_D dynPlugin_addParticleObject;
    ptr_dynPlugin_removeParticleObject dynPlugin_removeParticleObject;
    ptr_dynPlugin_addParticleObjectItem_D dynPlugin_addParticleObjectItem;
    ptr_dynPlugin_getParticleObjectOtherFloatsPerItem dynPlugin_getParticleObjectOtherFloatsPerItem;
    ptr_dynPlugin_getContactPoints_D dynPlugin_getContactPoints;
    ptr_dynPlugin_getParticles dynPlugin_getParticles;
    ptr_dynPlugin_getParticleData_D dynPlugin_getParticleData;
    ptr_dynPlugin_getContactForce_D dynPlugin_getContactForce;
    ptr_dynPlugin_getDynamicStepDivider dynPlugin_getDynamicStepDivider;
    ptr_mujocoPlugin_computePMI mujocoPlugin_computePMI;
    ptr_mujocoPlugin_computeInertia mujocoPlugin_computeInertia;
    ptr_dynPlugin_engine bullet278_engine;
    ptr_dynPlugin_engine bullet283_engine;
    ptr_dynPlugin_engine ode_engine;
    ptr_dynPlugin_engine vortex_engine;
    ptr_dynPlugin_engine newton_engine;
    ptr_dynPlugin_engine mujoco_engine;

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
    ptr_geomPlugin_getOctreeFromSerializationData_float geomPlugin_getOctreeFromSerializationData_float;
    ptr_geomPlugin_getOctreeSerializationData_float geomPlugin_getOctreeSerializationData_float;
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
    ptr_geomPlugin_getPtcloudFromSerializationData_float geomPlugin_getPtcloudFromSerializationData_float;
    ptr_geomPlugin_getPtcloudSerializationData_float geomPlugin_getPtcloudSerializationData_float;
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

    ptr_oldIkPlugin_createEnv oldIkPlugin_createEnv;
    //    ptr_oldIkPlugin_switchEnvironment oldIkPlugin_switchEnvironment;
    ptr_oldIkPlugin_eraseEnvironment oldIkPlugin_eraseEnvironment;
    ptr_oldIkPlugin_eraseObject oldIkPlugin_eraseObject;
    ptr_oldIkPlugin_setObjectParent oldIkPlugin_setObjectParent;
    ptr_oldIkPlugin_createDummy oldIkPlugin_createDummy;
    ptr_oldIkPlugin_setLinkedDummy oldIkPlugin_setLinkedDummy;
    ptr_oldIkPlugin_createJoint oldIkPlugin_createJoint;
    ptr_oldIkPlugin_setJointMode oldIkPlugin_setJointMode;
    ptr_oldIkPlugin_setJointInterval oldIkPlugin_setJointInterval;
    ptr_oldIkPlugin_setJointScrewPitch oldIkPlugin_setJointScrewPitch;
    ptr_oldIkPlugin_setJointIkWeight oldIkPlugin_setJointIkWeight;
    ptr_oldIkPlugin_setJointMaxStepSize oldIkPlugin_setJointMaxStepSize;
    ptr_oldIkPlugin_setJointDependency oldIkPlugin_setJointDependency;
    ptr_oldIkPlugin_getJointPosition oldIkPlugin_getJointPosition;
    ptr_oldIkPlugin_setJointPosition oldIkPlugin_setJointPosition;
    ptr_oldIkPlugin_getSphericalJointQuaternion oldIkPlugin_getSphericalJointQuaternion;
    ptr_oldIkPlugin_setSphericalJointQuaternion oldIkPlugin_setSphericalJointQuaternion;
    ptr_oldIkPlugin_createIkGroup oldIkPlugin_createIkGroup;
    ptr_oldIkPlugin_eraseIkGroup oldIkPlugin_eraseIkGroup;
    ptr_oldIkPlugin_setIkGroupFlags oldIkPlugin_setIkGroupFlags;
    ptr_oldIkPlugin_setIkGroupCalculation oldIkPlugin_setIkGroupCalculation;
    ptr_oldIkPlugin_addIkElement oldIkPlugin_addIkElement;
    ptr_oldIkPlugin_eraseIkElement oldIkPlugin_eraseIkElement;
    ptr_oldIkPlugin_setIkElementFlags oldIkPlugin_setIkElementFlags;
    ptr_oldIkPlugin_setIkElementBase oldIkPlugin_setIkElementBase;
    ptr_oldIkPlugin_setIkElementConstraints oldIkPlugin_setIkElementConstraints;
    ptr_oldIkPlugin_setIkElementPrecision oldIkPlugin_setIkElementPrecision;
    ptr_oldIkPlugin_setIkElementWeights oldIkPlugin_setIkElementWeights;
    ptr_oldIkPlugin_handleIkGroup oldIkPlugin_handleIkGroup;
    ptr_oldIkPlugin_computeJacobian oldIkPlugin_computeJacobian;
    ptr_oldIkPlugin_getJacobian oldIkPlugin_getJacobian;
    ptr_oldIkPlugin_getManipulability oldIkPlugin_getManipulability;
    ptr_oldIkPlugin_getConfigForTipPose oldIkPlugin_getConfigForTipPose;
    ptr_oldIkPlugin_getObjectLocalTransformation oldIkPlugin_getObjectLocalTransformation;
    ptr_oldIkPlugin_setObjectLocalTransformation oldIkPlugin_setObjectLocalTransformation;

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

    // Following 2 only used with deprecated API functions:
    ptrCustomUi_msgBox _customUi_msgBox;
    ptrCustomUi_fileDialog _customUi_fileDialog;

    ptrassimp_importShapes _assimp_importShapes;
    ptrassimp_exportShapes _assimp_exportShapes;
    ptrassimp_importMeshes _assimp_importMeshes;
    ptrassimp_exportMeshes _assimp_exportMeshes;

    ptrExtRenderer povRayAddr;

    ptrExtRenderer openGl3Addr;

    ptrHACD hacdAddr;
    ptrVHACD vhacdAddr;
    ptrQhull qhullAddr;

    ptrMeshDecimator decimatorAddr;

  private:
    void _loadAuxEntryPoints();

    std::string _name;      // e.g. simAssimp, simAssimp-2-78, etc. (old plugins: Assimp, etc.)
    std::string _namespace; // e.g. simBullet, simAssimp, etc. (old plugins: empty)

    CPluginCallbackContainer _pluginCallbackContainer;
    CPluginVariableContainer _pluginVariableContainer;

    std::unordered_set<int> _dependencies; // -1=c++, otherwise script handles

    std::string _filename;
    int pluginVersion;
    int extendedVersionInt;
    std::string extendedVersionString;
    std::string buildDateString;
    int handle;
    int _consoleVerbosity;
    int _statusbarVerbosity;
    volatile int _stage; // not for legacy plugins
    WLibrary instance;

    ptrInit _initAddress;
    ptrCleanup _cleanupAddress;
    ptrMsg _msgAddress;

    ptrInit_ui _initAddress_ui;
    ptrCleanup_ui _cleanupAddress_ui;
    ptrMsg_ui _msgAddress_ui;

    ptrStart _startAddress_legacy;
    ptrEnd _endAddress_legacy;
    ptrMessage _messageAddress_legacy;
};
