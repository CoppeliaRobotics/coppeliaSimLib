
#pragma once

#include "vrepMainHeader.h"
#include "vVarious.h"
#include "4X4Matrix.h"

typedef  unsigned char (__cdecl *ptrStart)(void*,int);
typedef  void (__cdecl *ptrEnd)(void);
typedef  void* (__cdecl *ptrMessage)(int,int*,void*,int*);
typedef  void (__cdecl *ptrExtRenderer)(int,void*);
typedef  void (__cdecl *ptrQhull)(void*);
typedef  void (__cdecl *ptrHACD)(void*);
typedef  void (__cdecl *ptrVHACD)(void*);
typedef  void (__cdecl *ptrMeshDecimator)(void*);

typedef char (__cdecl *ptrv_repDyn_startSimulation)(int,int,const float*,const int*);
typedef void (__cdecl *ptrv_repDyn_endSimulation)(void);
typedef void (__cdecl *ptrv_repDyn_step)(float,float);
typedef char (__cdecl *ptrv_repDyn_isInitialized)(void);
typedef char (__cdecl *ptrv_repDyn_isDynamicContentAvailable)(void);
typedef void (__cdecl *ptrv_repDyn_serializeDynamicContent)(const char*,int);
typedef int (__cdecl *ptrv_repDyn_addParticleObject)(int,float,float,const void*,float,int,const float*,const float*,const float*,const float*);
typedef char (__cdecl *ptrv_repDyn_removeParticleObject)(int);
typedef char (__cdecl *ptrv_repDyn_addParticleObjectItem)(int,const float*,float);
typedef int (__cdecl *ptrv_repDyn_getParticleObjectOtherFloatsPerItem)(int);
typedef float* (__cdecl *ptrv_repDyn_getContactPoints)(int*);
typedef void** (__cdecl *ptrv_repDyn_getParticles)(int,int*,int*,float**);
typedef char (__cdecl *ptrv_repDyn_getParticleData)(const void*,float*,float*,int*,float**);
typedef char (__cdecl *ptrv_repDyn_getContactForce)(int,int,int,int*,float*);
typedef void (__cdecl *ptrv_repDyn_reportDynamicWorldConfiguration)(int,char,float);
typedef int (__cdecl *ptrv_repDyn_getDynamicStepDivider)(void);
typedef int (__cdecl *ptrv_repDyn_getEngineInfo)(int*,int*,char*,char*);

typedef void* (__cdecl *ptrv_repMesh_createCollisionInformationStructure)(const float*,int,const int*,int,float,float,int);
typedef void* (__cdecl *ptrv_repMesh_copyCollisionInformationStructure)(const void*);
typedef void (__cdecl *ptrv_repMesh_destroyCollisionInformationStructure)(void*);
typedef void (__cdecl *ptrv_repMesh_scaleCollisionInformationStructure)(void*,float);
typedef unsigned char* (__cdecl *ptrv_repMesh_getCollisionInformationStructureSerializationData)(const void*,int*);
typedef void* (__cdecl *ptrv_repMesh_getCollisionInformationStructureFromSerializationData)(const unsigned char*,const float*,int,const int*,int);
typedef void (__cdecl *ptrv_repMesh_releaseBuffer)(void*);
typedef char (__cdecl *ptrv_repMesh_getCutMesh)(const void*,const float*,float**,int*,int**,int*,int);
typedef int (__cdecl *ptrv_repMesh_getCalculatedTriangleCount)(const void*);
typedef int* (__cdecl *ptrv_repMesh_getCalculatedTrianglesPointer)(const void*);
typedef int (__cdecl *ptrv_repMesh_getCalculatedVerticeCount)(const void*);
typedef float* (__cdecl *ptrv_repMesh_getCalculatedVerticesPointer)(const void*);
typedef int (__cdecl *ptrv_repMesh_getCalculatedSegmentCount)(const void*);
typedef int* (__cdecl *ptrv_repMesh_getCalculatedSegmentsPointer)(const void*);
typedef int (__cdecl *ptrv_repMesh_getCalculatedPolygonCount)(const void*);
typedef int (__cdecl *ptrv_repMesh_getCalculatedPolygonSize)(const void*,int);
typedef int* (__cdecl *ptrv_repMesh_getCalculatedPolygonArrayPointer)(const void*,int);
typedef char (__cdecl *ptrv_repMesh_getCalculatedTriangleAt)(const void*,float*,float*,float*,int);
typedef char (__cdecl *ptrv_repMesh_getMeshMeshCollision)(const void*,const void*,const float*,const float*,const void**,char,float**,int*,int*);
typedef int (__cdecl *ptrv_repMesh_getTriangleTriangleCollision)(const float*,const float*,const float*,const float*,const float*,const float*,float*,float*,char);
typedef char (__cdecl *ptrv_repMesh_getBoxBoxCollision)(const float*,const float*,const float*,const float*);
typedef char (__cdecl *ptrv_repMesh_getBoxPointCollision)(const float*,const float*,const float*);
typedef void (__cdecl *ptrv_repMesh_getMeshMeshDistance)(const void*,const void*,const float*,const float*,const void**,char,float*,int*);
typedef char (__cdecl *ptrv_repMesh_getDistanceAgainstDummy_ifSmaller)(const void*,const float*,const float*,float*,float*,float*,int*);
typedef float (__cdecl *ptrv_repMesh_getBoxPointDistance)(const float*,const float*,const float*);
typedef float (__cdecl *ptrv_repMesh_getApproximateBoxBoxDistance)(const float*,const float*,const float*,const float*);
typedef char (__cdecl *ptrv_repMesh_getTriangleTriangleDistance_ifSmaller)(const float*,const float*,const float*,const float*,const float*,const float*,float*,float*,float*);
typedef char (__cdecl *ptrv_repMesh_getTrianglePointDistance_ifSmaller)(const float*,const float*,const float*,const float*,float*,float*);
typedef char (__cdecl *ptrv_repMesh_getRayProxSensorDistance_ifSmaller)(const void*,const float*,float*,const float*,float,const float*,float,float*,bool,bool,bool,char*,float*,void*);
typedef char (__cdecl *ptrv_repMesh_isPointInsideVolume1AndOutsideVolume2)(const float*,const float*,int,const float*,int);
typedef char (__cdecl *ptrv_repMesh_isPointTouchingVolume)(const float*,const float*,int);
typedef char (__cdecl *ptrv_repMesh_getProxSensorDistance_ifSmaller)(const void*,const float*,float*,const float*,int,const float*,int,float,float*,bool,bool,bool,float*,void*);
typedef char (__cdecl *ptrv_repMesh_getProxSensorDistanceToSegment_ifSmaller)(const float*,const float*,float*,const float*,int,const float*,int,float,float*);
typedef char (__cdecl *ptrv_repMesh_getProxSensorDistanceToTriangle_ifSmaller)(const float*,const float*,const float*,float*,const float*,int,const float*,int,float,float*,bool,bool,float*);
typedef float (__cdecl *ptrv_repMesh_cutNodeWithVolume)(void*,const float*,const float*,int);
typedef void* (__cdecl *ptrv_repMesh_createPointCloud)(const float*,int,float,int,const unsigned char*,float);
typedef void* (__cdecl *ptrv_repMesh_createColorPointCloud)(const float*,int,float,int,const unsigned char*,float);
typedef void* (__cdecl *ptrv_repMesh_copyPointCloud)(const void*);
typedef void (__cdecl *ptrv_repMesh_destroyPointCloud)(void*);
typedef void (__cdecl *ptrv_repMesh_scalePointCloud)(void*,float);
typedef void (__cdecl *ptrv_repMesh_insertPointsIntoPointCloud)(void*,const float*,int,const unsigned char*,float);
typedef void (__cdecl *ptrv_repMesh_insertColorPointsIntoPointCloud)(void*,const float*,int,const unsigned char*,float);
typedef char (__cdecl *ptrv_repMesh_removePointCloudPoints)(void*,const float*,int,float,int*);
typedef char (__cdecl *ptrv_repMesh_intersectPointCloudPoints)(void*,const float*,int,float);
typedef float* (__cdecl *ptrv_repMesh_getPointCloudDebugCorners)(const void*,int*);
typedef unsigned char* (__cdecl *ptrv_repMesh_getPointCloudSerializationData)(const void*,int*);
typedef void* (__cdecl *ptrv_repMesh_getPointCloudFromSerializationData)(const unsigned char*);
typedef void* (__cdecl *ptrv_repMesh_createOctreeFromPoints)(const float*,int,float,const unsigned char*,unsigned int);
typedef void* (__cdecl *ptrv_repMesh_createOctreeFromColorPoints)(const float*,int,float,const unsigned char*,const unsigned int*);
typedef void* (__cdecl *ptrv_repMesh_copyOctree)(const void*);
typedef void (__cdecl *ptrv_repMesh_destroyOctree)(void*);
typedef void (__cdecl *ptrv_repMesh_scaleOctree)(void*,float);
typedef char (__cdecl *ptrv_repMesh_removeOctreeVoxelsFromPoints)(void*,const float*,int);
typedef void (__cdecl *ptrv_repMesh_insertPointsIntoOctree)(void*,const float*,int,const unsigned char*,unsigned int);
typedef void (__cdecl *ptrv_repMesh_insertColorPointsIntoOctree)(void*,const float*,int,const unsigned char*,const unsigned int*);
typedef float* (__cdecl *ptrv_repMesh_getOctreeVoxels)(const void*,int*);
typedef float* (__cdecl *ptrv_repMesh_getPointCloudPointData)(const void*,int*);
typedef float* (__cdecl *ptrv_repMesh_getPartialPointCloudPointData)(const void*,int*,float);
typedef float* (__cdecl *ptrv_repMesh_getOctreeDebugCorners)(const void*,int*);
typedef unsigned char* (__cdecl *ptrv_repMesh_getOctreeSerializationData)(const void*,int*);
typedef void* (__cdecl *ptrv_repMesh_getOctreeFromSerializationData)(const unsigned char*);
typedef void* (__cdecl *ptrv_repMesh_createOctreeFromShape)(const float*,const void*,const float*,float,const unsigned char*,unsigned int);
typedef char (__cdecl *ptrv_repMesh_removeOctreeVoxelsFromShape)(void*,const float*,const void*,const float*);
typedef void (__cdecl *ptrv_repMesh_insertShapeIntoOctree)(void*,const float*,const void*,const float*,const unsigned char*,unsigned int);
typedef void* (__cdecl *ptrv_repMesh_createOctreeFromOctree)(const float*,const void*,const float*,float,const unsigned char*,unsigned int);
typedef char (__cdecl *ptrv_repMesh_removeOctreeVoxelsFromOctree)(void*,const float*,const void*,const float*);
typedef void (__cdecl *ptrv_repMesh_insertOctreeIntoOctree)(void*,const float*,const void*,const float*,const unsigned char*,unsigned int);
typedef char (__cdecl *ptrv_repMesh_checkOctreeCollisionWithShape)(const void*,const float*,const void*,const float*);
typedef char (__cdecl *ptrv_repMesh_checkOctreeCollisionWithOctree)(const void*,const float*,const void*,const float*);
typedef char (__cdecl *ptrv_repMesh_checkOctreeCollisionWithSeveralPoints)(const void*,const float*,const float*,int);
typedef char (__cdecl *ptrv_repMesh_checkOctreeCollisionWithSinglePoint)(const void*,const float*,const float*,unsigned int*,unsigned long long int*);
typedef char (__cdecl *ptrv_repMesh_checkOctreeCollisionWithPointCloud)(const void*,const float*,const void*,const float*);
typedef char (__cdecl *ptrv_repMesh_getPointCloudDistanceToPointIfSmaller)(const void*,const float*,const float*,float*,long long int*);
typedef char (__cdecl *ptrv_repMesh_getPointCloudDistanceToPointCloudIfSmaller)(const void*,const void*,const float*,const float*,float*,long long int*,long long int*);
typedef float* (__cdecl *ptrv_repMesh_getPointCloudPointsFromCache)(const void*,const float*,const long long int,int*,float*);
typedef int (__cdecl *ptrv_repMesh_getPointCloudNonEmptyCellCount)(const void*);
typedef char (__cdecl *ptrv_repMesh_getOctreeDistanceToPointIfSmaller)(const void*,const float*,const float*,float*,long long int*);
typedef char (__cdecl *ptrv_repMesh_getOctreeCellFromCache)(const void*,const float*,const long long int,float*,float*);
typedef char (__cdecl *ptrv_repMesh_getOctreeDistanceToOctreeIfSmaller)(const void*,const void*,const float*,const float*,float*,long long int*,long long int*,char);
typedef char (__cdecl *ptrv_repMesh_getOctreeDistanceToPointCloudIfSmaller)(const void*,const void*,const float*,const float*,float*,long long int*,long long int*);
typedef char (__cdecl *ptrv_repMesh_getOctreeDistanceToShapeIfSmaller)(const void*,const void*,const float*,const float*,float*,long long int*,int*);
typedef char (__cdecl *ptrv_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller)(float,const float*,const float*,const float*,float*,float*,float*);
typedef char (__cdecl *ptrv_repMesh_getPointCloudDistanceToShapeIfSmaller)(const void*,const void*,const float*,const float*,float*,long long int*,int*);
typedef char (__cdecl *ptrv_repMesh_getMinDistBetweenPointAndTriangleIfSmaller)(const float*,const float*,const float*,const float*,float*,float*);
typedef float (__cdecl *ptrv_repMesh_getBoxBoxDistance)(const float*,const float*,const float*,const float*);
typedef char (__cdecl *ptrv_repMesh_getProxSensorPointCloudDistanceIfSmaller)(const void*,const float*,float*,const float*,int,const float*,int,float*,char,void*);
typedef char (__cdecl *ptrv_repMesh_getRayProxSensorOctreeDistanceIfSmaller)(const void*,const float*,float*,const float*,const float*,float,float*,char,char,char,float*,void*);
typedef char (__cdecl *ptrv_repMesh_getProxSensorOctreeDistanceIfSmaller)(const void*,const float*,float*,const float*,int,const float*,int,float,float*,char,char,char,float*,void*);
typedef char (__cdecl *ptrv_repMesh_removePointCloudPointsFromOctree)(void*,const float*,const void*,const float*,int*);

typedef char* (__cdecl *ptrCodeEditor_openModal)(const char* initText,const char* properties,int* positionAndSize);
typedef int (__cdecl *ptrCodeEditor_open)(const char* initText,const char* properties);
typedef int (__cdecl *ptrCodeEditor_setText)(int handle,const char* text,int insertMode);
typedef char* (__cdecl *ptrCodeEditor_getText)(int handle,int* positionAndSize);
typedef int (__cdecl *ptrCodeEditor_show)(int handle,int showState);
typedef int (__cdecl *ptrCodeEditor_close)(int handle,int* positionAndSize);

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
    void* sendEventCallbackMessage(int msg,int* auxVals,void* data,int retVals[4]);

    ptrStart startAddress;
    ptrEnd endAddress;
    ptrMessage messageAddress;

    ptrv_repDyn_startSimulation v_repDyn_startSimulation;
    ptrv_repDyn_endSimulation v_repDyn_endSimulation;
    ptrv_repDyn_step v_repDyn_step;
    ptrv_repDyn_isDynamicContentAvailable v_repDyn_isDynamicContentAvailable;
    ptrv_repDyn_serializeDynamicContent v_repDyn_serializeDynamicContent;
    ptrv_repDyn_addParticleObject v_repDyn_addParticleObject;
    ptrv_repDyn_removeParticleObject v_repDyn_removeParticleObject;
    ptrv_repDyn_addParticleObjectItem v_repDyn_addParticleObjectItem;
    ptrv_repDyn_getParticleObjectOtherFloatsPerItem v_repDyn_getParticleObjectOtherFloatsPerItem;
    ptrv_repDyn_getContactPoints v_repDyn_getContactPoints;
    ptrv_repDyn_getParticles v_repDyn_getParticles;
    ptrv_repDyn_getParticleData v_repDyn_getParticleData;
    ptrv_repDyn_getContactForce v_repDyn_getContactForce;
    ptrv_repDyn_reportDynamicWorldConfiguration v_repDyn_reportDynamicWorldConfiguration;
    ptrv_repDyn_getDynamicStepDivider v_repDyn_getDynamicStepDivider;
    ptrv_repDyn_getEngineInfo v_repDyn_getEngineInfo;

    ptrv_repMesh_createCollisionInformationStructure v_repMesh_createCollisionInformationStructure;
    ptrv_repMesh_copyCollisionInformationStructure v_repMesh_copyCollisionInformationStructure;
    ptrv_repMesh_destroyCollisionInformationStructure v_repMesh_destroyCollisionInformationStructure;
    ptrv_repMesh_scaleCollisionInformationStructure v_repMesh_scaleCollisionInformationStructure;
    ptrv_repMesh_getCollisionInformationStructureSerializationData v_repMesh_getCollisionInformationStructureSerializationData;
    ptrv_repMesh_getCollisionInformationStructureFromSerializationData v_repMesh_getCollisionInformationStructureFromSerializationData;
    ptrv_repMesh_releaseBuffer v_repMesh_releaseBuffer;
    ptrv_repMesh_getCutMesh v_repMesh_getCutMesh;
    ptrv_repMesh_getCalculatedTriangleCount v_repMesh_getCalculatedTriangleCount;
    ptrv_repMesh_getCalculatedTrianglesPointer v_repMesh_getCalculatedTrianglesPointer;
    ptrv_repMesh_getCalculatedVerticeCount v_repMesh_getCalculatedVerticeCount;
    ptrv_repMesh_getCalculatedVerticesPointer v_repMesh_getCalculatedVerticesPointer;
    ptrv_repMesh_getCalculatedSegmentCount v_repMesh_getCalculatedSegmentCount;
    ptrv_repMesh_getCalculatedSegmentsPointer v_repMesh_getCalculatedSegmentsPointer;
    ptrv_repMesh_getCalculatedPolygonCount v_repMesh_getCalculatedPolygonCount;
    ptrv_repMesh_getCalculatedPolygonSize v_repMesh_getCalculatedPolygonSize;
    ptrv_repMesh_getCalculatedPolygonArrayPointer v_repMesh_getCalculatedPolygonArrayPointer;
    ptrv_repMesh_getCalculatedTriangleAt v_repMesh_getCalculatedTriangleAt;
    ptrv_repMesh_getMeshMeshCollision v_repMesh_getMeshMeshCollision;
    ptrv_repMesh_getTriangleTriangleCollision v_repMesh_getTriangleTriangleCollision;
    ptrv_repMesh_getBoxBoxCollision v_repMesh_getBoxBoxCollision;
    ptrv_repMesh_getBoxPointCollision v_repMesh_getBoxPointCollision;
    ptrv_repMesh_getMeshMeshDistance v_repMesh_getMeshMeshDistance;
    ptrv_repMesh_getDistanceAgainstDummy_ifSmaller v_repMesh_getDistanceAgainstDummy_ifSmaller;
    ptrv_repMesh_getBoxPointDistance v_repMesh_getBoxPointDistance;
    ptrv_repMesh_getApproximateBoxBoxDistance v_repMesh_getApproximateBoxBoxDistance;
    ptrv_repMesh_getTriangleTriangleDistance_ifSmaller v_repMesh_getTriangleTriangleDistance_ifSmaller;
    ptrv_repMesh_getTrianglePointDistance_ifSmaller v_repMesh_getTrianglePointDistance_ifSmaller;
    ptrv_repMesh_getRayProxSensorDistance_ifSmaller v_repMesh_getRayProxSensorDistance_ifSmaller;
    ptrv_repMesh_isPointInsideVolume1AndOutsideVolume2 v_repMesh_isPointInsideVolume1AndOutsideVolume2;
    ptrv_repMesh_isPointTouchingVolume v_repMesh_isPointTouchingVolume;
    ptrv_repMesh_getProxSensorDistance_ifSmaller v_repMesh_getProxSensorDistance_ifSmaller;
    ptrv_repMesh_getProxSensorDistanceToSegment_ifSmaller v_repMesh_getProxSensorDistanceToSegment_ifSmaller;
    ptrv_repMesh_getProxSensorDistanceToTriangle_ifSmaller v_repMesh_getProxSensorDistanceToTriangle_ifSmaller;
    ptrv_repMesh_cutNodeWithVolume v_repMesh_cutNodeWithVolume;
    ptrv_repMesh_createPointCloud v_repMesh_createPointCloud;
    ptrv_repMesh_createColorPointCloud v_repMesh_createColorPointCloud;
    ptrv_repMesh_copyPointCloud v_repMesh_copyPointCloud;
    ptrv_repMesh_destroyPointCloud v_repMesh_destroyPointCloud;
    ptrv_repMesh_scalePointCloud v_repMesh_scalePointCloud;
    ptrv_repMesh_insertPointsIntoPointCloud v_repMesh_insertPointsIntoPointCloud;
    ptrv_repMesh_insertColorPointsIntoPointCloud v_repMesh_insertColorPointsIntoPointCloud;
    ptrv_repMesh_removePointCloudPoints v_repMesh_removePointCloudPoints;
    ptrv_repMesh_intersectPointCloudPoints v_repMesh_intersectPointCloudPoints;
    ptrv_repMesh_getPointCloudDebugCorners v_repMesh_getPointCloudDebugCorners;
    ptrv_repMesh_getPointCloudSerializationData v_repMesh_getPointCloudSerializationData;
    ptrv_repMesh_getPointCloudFromSerializationData v_repMesh_getPointCloudFromSerializationData;
    ptrv_repMesh_createOctreeFromPoints v_repMesh_createOctreeFromPoints;
    ptrv_repMesh_createOctreeFromColorPoints v_repMesh_createOctreeFromColorPoints;
    ptrv_repMesh_copyOctree v_repMesh_copyOctree;
    ptrv_repMesh_destroyOctree v_repMesh_destroyOctree;
    ptrv_repMesh_scaleOctree v_repMesh_scaleOctree;
    ptrv_repMesh_removeOctreeVoxelsFromPoints v_repMesh_removeOctreeVoxelsFromPoints;
    ptrv_repMesh_insertPointsIntoOctree v_repMesh_insertPointsIntoOctree;
    ptrv_repMesh_insertColorPointsIntoOctree v_repMesh_insertColorPointsIntoOctree;
    ptrv_repMesh_getOctreeVoxels v_repMesh_getOctreeVoxels;
    ptrv_repMesh_getPointCloudPointData v_repMesh_getPointCloudPointData;
    ptrv_repMesh_getPartialPointCloudPointData v_repMesh_getPartialPointCloudPointData;
    ptrv_repMesh_getOctreeDebugCorners v_repMesh_getOctreeDebugCorners;
    ptrv_repMesh_getOctreeSerializationData v_repMesh_getOctreeSerializationData;
    ptrv_repMesh_getOctreeFromSerializationData v_repMesh_getOctreeFromSerializationData;
    ptrv_repMesh_createOctreeFromShape v_repMesh_createOctreeFromShape;
    ptrv_repMesh_removeOctreeVoxelsFromShape v_repMesh_removeOctreeVoxelsFromShape;
    ptrv_repMesh_insertShapeIntoOctree v_repMesh_insertShapeIntoOctree;
    ptrv_repMesh_createOctreeFromOctree v_repMesh_createOctreeFromOctree;
    ptrv_repMesh_removeOctreeVoxelsFromOctree v_repMesh_removeOctreeVoxelsFromOctree;
    ptrv_repMesh_insertOctreeIntoOctree v_repMesh_insertOctreeIntoOctree;
    ptrv_repMesh_checkOctreeCollisionWithShape v_repMesh_checkOctreeCollisionWithShape;
    ptrv_repMesh_checkOctreeCollisionWithOctree v_repMesh_checkOctreeCollisionWithOctree;
    ptrv_repMesh_checkOctreeCollisionWithSinglePoint v_repMesh_checkOctreeCollisionWithSinglePoint;
    ptrv_repMesh_checkOctreeCollisionWithSeveralPoints v_repMesh_checkOctreeCollisionWithSeveralPoints;
    ptrv_repMesh_checkOctreeCollisionWithPointCloud v_repMesh_checkOctreeCollisionWithPointCloud;
    ptrv_repMesh_getPointCloudDistanceToPointIfSmaller v_repMesh_getPointCloudDistanceToPointIfSmaller;
    ptrv_repMesh_getPointCloudDistanceToPointCloudIfSmaller v_repMesh_getPointCloudDistanceToPointCloudIfSmaller;
    ptrv_repMesh_getPointCloudPointsFromCache v_repMesh_getPointCloudPointsFromCache;
    ptrv_repMesh_getPointCloudNonEmptyCellCount v_repMesh_getPointCloudNonEmptyCellCount;
    ptrv_repMesh_getOctreeDistanceToPointIfSmaller v_repMesh_getOctreeDistanceToPointIfSmaller;
    ptrv_repMesh_getOctreeCellFromCache v_repMesh_getOctreeCellFromCache;
    ptrv_repMesh_getOctreeDistanceToOctreeIfSmaller v_repMesh_getOctreeDistanceToOctreeIfSmaller;
    ptrv_repMesh_getOctreeDistanceToPointCloudIfSmaller v_repMesh_getOctreeDistanceToPointCloudIfSmaller;
    ptrv_repMesh_getOctreeDistanceToShapeIfSmaller v_repMesh_getOctreeDistanceToShapeIfSmaller;
    ptrv_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller v_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller;
    ptrv_repMesh_getPointCloudDistanceToShapeIfSmaller v_repMesh_getPointCloudDistanceToShapeIfSmaller;
    ptrv_repMesh_getMinDistBetweenPointAndTriangleIfSmaller v_repMesh_getMinDistBetweenPointAndTriangleIfSmaller;
    ptrv_repMesh_getBoxBoxDistance v_repMesh_getBoxBoxDistance;
    ptrv_repMesh_getProxSensorPointCloudDistanceIfSmaller v_repMesh_getProxSensorPointCloudDistanceIfSmaller;
    ptrv_repMesh_getRayProxSensorOctreeDistanceIfSmaller v_repMesh_getRayProxSensorOctreeDistanceIfSmaller;
    ptrv_repMesh_getProxSensorOctreeDistanceIfSmaller v_repMesh_getProxSensorOctreeDistanceIfSmaller;
    ptrv_repMesh_removePointCloudPointsFromOctree v_repMesh_removePointCloudPointsFromOctree;

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

    std::string name;
    std::string _filename;
    unsigned char pluginVersion;
    int extendedVersionInt;
    std::string extendedVersionString;
    std::string buildDateString;
    int handle;
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
    static CPlugin* getPluginFromName(const char* pluginName);
    static CPlugin* getPluginFromIndex(int index);
    static bool killPlugin(int handle);
    static int getPluginCount();

    static bool selectExtRenderer(int index);
    static bool extRenderer(int msg,void* data);

    static bool qhull(void* data);
    static bool hacd(void* data);
    static bool vhacd(void* data);
    static bool meshDecimator(void* data);

    // physics engines:
    static CPlugin* currentDynEngine;
    static bool dyn_startSimulation(int engine,int version,const float floatParams[20],const int intParams[20]);
    static bool dyn_isInitialized();
    static bool dyn_isDynamicContentAvailable();
    static bool dyn_removeParticleObject(int objectHandle);
    static bool dyn_addParticleObjectItem(int objectHandle,const float* itemData,float simulationTime);
    static bool dyn_getParticleData(const void* particle,float* pos,float* size,int* objectType,float** additionalColor);
    static bool dyn_getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],float contactInfo[6]);

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

    // mesh calc engine:
    static CPlugin* currentMeshEngine;
    static bool isMeshPluginAvailable();
    static void mesh_lockUnlock(bool lock);
    static void* mesh_createCollisionInformationStructure(const float* cumulMeshVertices,int cumulMeshVerticesSize,const int* cumulMeshIndices,int cumulMeshIndicesSize,float maxTriSize,float edgeAngle,int maxTriCount);
    static void* mesh_copyCollisionInformationStructure(const void* collInfo);
    static void mesh_destroyCollisionInformationStructure(void* collInfo);
    static void mesh_scaleCollisionInformationStructure(void* collInfo,float scaleFactor);
    static unsigned char* mesh_getCollisionInformationStructureSerializationData(const void* collInfo,int& dataSize);
    static void* mesh_getCollisionInformationStructureFromSerializationData(const unsigned char* data,const float* cumulMeshVertices,int cumulMeshVerticesSize,const int* cumulMeshIndices,int cumulMeshIndicesSize);
    static void mesh_releaseBuffer(void* buffer);
    static bool mesh_getCutMesh(const void* collInfo,const C7Vector* tr,float** vertices,int* verticesSize,int** indices,int* indicesSize,int options);
    static int mesh_getCalculatedTriangleCount(const void* collInfo);
    static int* mesh_getCalculatedTrianglesPointer(const void* collInfo);
    static int mesh_getCalculatedVerticeCount(const void* collInfo);
    static float* mesh_getCalculatedVerticesPointer(const void* collInfo);
    static int mesh_getCalculatedSegmentCount(const void* collInfo);
    static int* mesh_getCalculatedSegmentsPointer(const void* collInfo);
    static int mesh_getCalculatedPolygonCount(const void* collInfo);
    static int mesh_getCalculatedPolygonSize(const void* collInfo,int polygonIndex);
    static int* mesh_getCalculatedPolygonArrayPointer(const void* collInfo,int polygonIndex);
    static bool mesh_getCalculatedTriangleAt(const void* collInfo,C3Vector& a0,C3Vector& a1,C3Vector& a2,int ind);
    static bool mesh_getMeshMeshCollision(const void* collInfo1,const void* collInfo2,const C4X4Matrix collObjMatr[2],const void* collInfos[2],bool inverseExploration,std::vector<float>* intersections,int caching[2]);
    static int mesh_getTriangleTriangleCollision(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& b0,const C3Vector& f0,const C3Vector& f1,C3Vector* intSegPart0,C3Vector* intSegPart1,bool getIntersection);
    static bool mesh_getBoxBoxCollision(const C4X4Matrix& box1Tr,const C3Vector& box1Size,const C4X4Matrix& box2Tr,const C3Vector& box2Size);
    static bool mesh_getBoxPointCollision(const C4X4Matrix& boxTr,const C3Vector& boxSize,const C3Vector& point);
    static void mesh_getMeshMeshDistance(const void* collInfo1,const void* collInfo2,const C4X4Matrix distObjMatr[2],const void* collInfos[2],bool inverseExploration,float distances[7],int caching[2]);
    static bool mesh_getDistanceAgainstDummy_ifSmaller(const void* collInfo,const C3Vector& dummyPos,const C4X4Matrix& itPCTM,float &dist,C3Vector& ray0,C3Vector& ray1,int& itBuff);
    static float mesh_getBoxPointDistance(const C4X4Matrix& t1,const C3Vector& s1,const C3Vector& dummyPos);
    static float mesh_getApproximateBoxBoxDistance(const C4X4Matrix& t1,const C3Vector& s1,const C4X4Matrix& t2,const C3Vector& s2);
    static bool mesh_getTriangleTriangleDistance_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& b0,const C3Vector& f0,const C3Vector& f1,float &dist,C3Vector& segA,C3Vector& segB);
    static bool mesh_getTrianglePointDistance_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& dummyPos,float &dist,C3Vector& segA);
    static bool mesh_getRayProxSensorDistance_ifSmaller(const void* collInfo,const C4X4Matrix& selfPCTM,float &dist,const C3Vector& lp,float closeThreshold,const C3Vector& lvFar,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,char* closeDetectionTriggered,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback);
    static bool mesh_isPointInsideVolume1AndOutsideVolume2(const C3Vector& p,const std::vector<float>* planes,const std::vector<float>* planesOutside);
    static bool mesh_isPointTouchingVolume(const C3Vector& p,const std::vector<float>* planes);
    static bool mesh_getProxSensorDistance_ifSmaller(const void* collInfo,const C4X4Matrix& itPCTM,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,std::vector<float>* cutEdges,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback);
    static bool mesh_getProxSensorDistanceToSegment_ifSmaller(const C3Vector& p0,const C3Vector& p1,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float maxAngle,C3Vector& detectPoint);
    static bool mesh_getProxSensorDistanceToTriangle_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized);
    static float mesh_cutNodeWithVolume(void* collInfo,const C4X4Matrix& itPCTM,const std::vector<float>* planes);
    static void* mesh_createPointCloud(const float* relPoints,int ptCnt,float cellSize,int maxPointCountPerCell,const float theColor[3],float distTolerance);
    static void* mesh_createColorPointCloud(const float* relPoints,int ptCnt,float cellSize,int maxPointCountPerCell,const unsigned char* theColors,float distTolerance);
    static void* mesh_copyPointCloud(const void* pointCloudInfo);
    static void mesh_destroyPointCloud(void* pointCloudInfo);
    static void mesh_scalePointCloud(void* pointCloudInfo,float scaleFactor);
    static void mesh_insertPointsIntoPointCloud(void* pointCloudInfo,const float* relPoints,int ptCnt,const float theColor[3],float distTolerance);
    static void mesh_insertColorPointsIntoPointCloud(void* pointCloudInfo,const float* relPoints,int ptCnt,const unsigned char* theColors,float distTolerance);
    static bool mesh_removePointCloudPoints(void* pointCloudInfo,const float* relPoints,int ptCnt,float distTolerance,int& removedCnt);
    static bool mesh_intersectPointCloudPoints(void* pointCloudInfo,const float* relPoints,int ptCnt,float distTolerance);
    static void mesh_getPointCloudDebugCorners(const void* pointCloudInfo,std::vector<float>& cubeCorners);
    static void mesh_getPointCloudSerializationData(const void* pointCloudInfo,std::vector<unsigned char>& data);
    static void* mesh_getPointCloudFromSerializationData(const std::vector<unsigned char>& data);
    static void* mesh_createOctreeFromPoints(const float* relPoints,int ptCnt,float cellSize,const float theColor[3],unsigned int theTag);
    static void* mesh_createOctreeFromColorPoints(const float* relPoints,int ptCnt,float cellSize,const unsigned char* theColors,const unsigned int* theTags);
    static void* mesh_copyOctree(const void* octreeInfo);
    static void mesh_destroyOctree(void* octreeInfo);
    static void mesh_scaleOctree(void* octreeInfo,float scaleFactor);
    static bool mesh_removeOctreeVoxelsFromPoints(void* octreeInfo,const float* relPoints,int ptCnt);
    static void mesh_insertPointsIntoOctree(void* octreeInfo,const float* relPoints,int ptCnt,const float theColor[3],unsigned int theTag);
    static void mesh_insertColorPointsIntoOctree(void* octreeInfo,const float* relPoints,int ptCnt,const unsigned char* theColors,const unsigned int* theTags);
    static void mesh_getOctreeVoxels(const void* octreeInfo,std::vector<float>& voxelPositions,std::vector<float>& voxelColors);
    static void mesh_getPointCloudPointData(const void* pointCloudInfo,std::vector<float>& pointPositions,std::vector<float>& pointColors);
    static void mesh_getPartialPointCloudPointData(const void* pointCloudInfo,std::vector<float>& pointPositions,std::vector<float>& pointColors,float ratio);
    static void mesh_getOctreeDebugCorners(const void* octreeInfo,std::vector<float>& cubeCorners);
    static void mesh_getOctreeSerializationData(const void* octreeInfo,std::vector<unsigned char>& data);
    static void* mesh_getOctreeFromSerializationData(const std::vector<unsigned char>& data);
    static void* mesh_createOctreeFromShape(const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM,float cellSize,const float theColor[3],unsigned int theTag);
    static bool mesh_removeOctreeVoxelsFromShape(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM);
    static void mesh_insertShapeIntoOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM,const float theColor[3],unsigned int theTag);
    static void* mesh_createOctreeFromOctree(const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM,float cellSize,const float theColor[3],unsigned int theTag);
    static bool mesh_removeOctreeVoxelsFromOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM);
    static void mesh_insertOctreeIntoOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM,const float theColor[3],unsigned int theTag);
    static bool mesh_checkOctreeCollisionWithShape(const void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collNodePCTM);
    static bool mesh_checkOctreeCollisionWithOctree(const void* octree1Info,const C4X4Matrix& octree1PCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM);
    static bool mesh_checkOctreeCollisionWithSeveralPoints(const void* octreeInfo,const C4X4Matrix& octreePCTM,const float* absPoints,int ptCnt);
    static bool mesh_checkOctreeCollisionWithSinglePoint(const void* octreeInfo,const C4X4Matrix& octreePCTM,const C3Vector& absPoint,unsigned int* tag,unsigned long long int* location);
    static bool mesh_checkOctreeCollisionWithPointCloud(const void* octreeInfo,const C4X4Matrix& octreePCTM,const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM);
    static bool mesh_getPointCloudDistanceToPointIfSmaller(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const C3Vector& absPoint,float ray[7],long long int& cacheValue);
    static bool mesh_getPointCloudDistanceToPointCloudIfSmaller(const void* pointCloudInfo1,const void* pointCloudInfo2,const C4X4Matrix& thisPcPCTM,const C4X4Matrix& otherPcPCTM,float ray[7],long long int& thisCacheValue,long long int& otherCacheValue);
    static float* mesh_getPointCloudPointsFromCache(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const long long int cacheValue,int& ptCnt,C4X4Matrix& ptsRetToThisM);
    static int mesh_getPointCloudNonEmptyCellCount(const void* pointCloudInfo);
    static bool mesh_getOctreeDistanceToPointIfSmaller(const void* octreeInfo,const C4X4Matrix& octreePCTM,const C3Vector& absPoint,float ray[7],long long int& cacheValue);
    static bool mesh_getOctreeCellFromCache(const void* octreeInfo,const C4X4Matrix& octreePCTM,const long long int cacheValue,float& cellSize,C4X4Matrix& cellRetToThisM);
    static bool mesh_getOctreeDistanceToOctreeIfSmaller(const void* octreeInfo1,const void* octreeInfo2,const C4X4Matrix& octree1PCTM,const C4X4Matrix& octree2PCTM,float ray[7],long long int& octree1CacheValue,long long int& octree2CacheValue,bool weHaveSomeCoherency);
    static bool mesh_getOctreeDistanceToPointCloudIfSmaller(const void* octreeInfo,const void* pointCloudInfo,const C4X4Matrix& octreePCTM,const C4X4Matrix& pointCloudPCTM,float ray[7],long long int& octreeCacheValue,long long int& pointCloudCacheValue);
    static bool mesh_getOctreeDistanceToShapeIfSmaller(const void* octreeInfo,const void* collInfo,const C4X4Matrix& octreePCTM,const C4X4Matrix& collNodePCTM,float ray[7],long long int& octreeCacheValue,int& collNodeCacheValue);
    static bool mesh_getMinDistBetweenCubeAndTriangleIfSmaller(float cubeSize,const C3Vector& b1,const C3Vector& b1e,const C3Vector& b1f,float& dist,C3Vector& distPt1,C3Vector& distPt2);
    static bool mesh_getPointCloudDistanceToShapeIfSmaller(const void* pointCloudInfo,const void* collInfo,const C4X4Matrix& pointCloudPCTM,const C4X4Matrix& collNodePCTM,float ray[7],long long int& pointCloudCacheValue,int& collNodeCacheValue);
    static bool mesh_getMinDistBetweenPointAndTriangleIfSmaller(const C3Vector& point,const C3Vector& b1,const C3Vector& b1e,const C3Vector& b1f,float& dist,C3Vector& distPt1);
    static float mesh_getBoxBoxDistance(const C4X4Matrix& m1,const C3Vector& halfSize1,const C4X4Matrix& m2,const C3Vector& halfSize2);
    static bool mesh_getProxSensorPointCloudDistanceIfSmaller(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,C3Vector& detectPoint,bool fast,void* theOcclusionCheckCallback);
    static bool mesh_getRayProxSensorOctreeDistanceIfSmaller(const void* octreeInfo,const C4X4Matrix& octreePCTM,float &dist,const C3Vector& lp,const C3Vector& lvFar,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback);
    static bool mesh_getProxSensorOctreeDistanceIfSmaller(const void* octreeInfo,const C4X4Matrix& octreeRTM,float& dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback);
    static bool mesh_removePointCloudPointsFromOctree(void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const void* octreeInfo,const C4X4Matrix& octreePCTM,int& removedCnt);

    // code editor plugin:
    static CPlugin* currentCodeEditor;
    static bool isCodeEditorPluginAvailable();
    static bool codeEditor_openModal(const char* initText,const char* properties,std::string& modifiedText,int* positionAndSize);
    static int codeEditor_open(const char* initText,const char* properties);
    static int codeEditor_setText(int handle,const char* text,int insertMode);
    static bool codeEditor_getText(int handle,std::string& text,int* positionAndSize);
    static int codeEditor_show(int handle,int showState);
    static int codeEditor_close(int handle,int* positionAndSize);

    // Custom UI plugin:
    static CPlugin* currentCustomUi;
    static bool isCustomUiPluginAvailable();
    static int customUi_msgBox(int type, int buttons, const char *title, const char *message);
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
