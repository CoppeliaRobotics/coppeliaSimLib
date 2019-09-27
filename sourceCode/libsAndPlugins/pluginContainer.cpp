#include "vrepMainHeader.h"
#include "app.h"
#include "funcDebug.h"
#include "pluginContainer.h"
#include "v_repConst.h"
#include "pathPlanningInterface.h"
#include "easyLock.h"
#include "v_rep_internal.h"

CPlugin::CPlugin(const char* filename,const char* pluginName)
{
    _filename=filename;
    name=pluginName;
    instance=nullptr;
    v_repMesh_createCollisionInformationStructure=nullptr;
    _codeEditor_openModal=nullptr;
    _customUi_msgBox=nullptr;
    _assimp_importShapes=nullptr;
    _loadCount=1;
    extendedVersionInt=-1;
}

CPlugin::~CPlugin()
{
    if (instance!=nullptr)
        VVarious::closeLibrary(instance);
    if (v_repMesh_createCollisionInformationStructure!=nullptr)
        CPluginContainer::currentMeshEngine=nullptr;
    if (_codeEditor_openModal!=nullptr)
        CPluginContainer::currentCodeEditor=nullptr;
    if (_customUi_msgBox!=nullptr)
        CPluginContainer::currentCustomUi=nullptr;
    if (_assimp_importShapes!=nullptr)
        CPluginContainer::currentAssimp=nullptr;
}

int CPlugin::load()
{
    WLibrary lib=VVarious::openLibrary(_filename.c_str()); // here we have the extension in the filename (.dll, .so or .dylib)
    if (lib!=nullptr)
    {
        instance=lib;
        startAddress=(ptrStart)(VVarious::resolveLibraryFuncName(lib,"v_repStart"));
        endAddress=(ptrEnd)(VVarious::resolveLibraryFuncName(lib,"v_repEnd"));
        messageAddress=(ptrMessage)(VVarious::resolveLibraryFuncName(lib,"v_repMessage"));
        if ( (startAddress!=nullptr)&&(endAddress!=nullptr)&&(messageAddress!=nullptr) )
        {
            pluginVersion=startAddress(nullptr,0);

            ptrExtRenderer pov=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repPovRay"));
            ptrExtRenderer extRenderer=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repExtRenderer"));
            ptrExtRenderer extRendererWindowed=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repExtRendererWindowed"));
            ptrExtRenderer openGl3=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repOpenGL3Renderer"));
            ptrExtRenderer openGl3Windowed=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repOpenGL3RendererWindowed"));

            ptrQhull qhull=(ptrQhull)(VVarious::resolveLibraryFuncName(lib,"v_repQhull"));
            ptrHACD hacd=(ptrHACD)(VVarious::resolveLibraryFuncName(lib,"v_repHACD"));
            ptrVHACD vhacd=(ptrVHACD)(VVarious::resolveLibraryFuncName(lib,"v_repVHACD"));
            ptrMeshDecimator meshDecimator=(ptrMeshDecimator)(VVarious::resolveLibraryFuncName(lib,"v_repDecimateMesh"));

            if (pluginVersion!=0)
            {
                // For the dynamic plugins:
                v_repDyn_startSimulation=(ptrv_repDyn_startSimulation)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_startSimulation"));
                v_repDyn_endSimulation=(ptrv_repDyn_endSimulation)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_endSimulation"));
                v_repDyn_step=(ptrv_repDyn_step)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_step"));
                v_repDyn_isDynamicContentAvailable=(ptrv_repDyn_isDynamicContentAvailable)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_isDynamicContentAvailable"));
                v_repDyn_serializeDynamicContent=(ptrv_repDyn_serializeDynamicContent)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_serializeDynamicContent"));
                v_repDyn_addParticleObject=(ptrv_repDyn_addParticleObject)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_addParticleObject"));
                v_repDyn_removeParticleObject=(ptrv_repDyn_removeParticleObject)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_removeParticleObject"));
                v_repDyn_addParticleObjectItem=(ptrv_repDyn_addParticleObjectItem)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_addParticleObjectItem"));
                v_repDyn_getParticleObjectOtherFloatsPerItem=(ptrv_repDyn_getParticleObjectOtherFloatsPerItem)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getParticleObjectOtherFloatsPerItem"));
                v_repDyn_getContactPoints=(ptrv_repDyn_getContactPoints)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getContactPoints"));
                v_repDyn_getParticles=(ptrv_repDyn_getParticles)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getParticles"));
                v_repDyn_getParticleData=(ptrv_repDyn_getParticleData)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getParticleData"));
                v_repDyn_getContactForce=(ptrv_repDyn_getContactForce)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getContactForce"));
                v_repDyn_reportDynamicWorldConfiguration=(ptrv_repDyn_reportDynamicWorldConfiguration)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_reportDynamicWorldConfiguration"));
                v_repDyn_getDynamicStepDivider=(ptrv_repDyn_getDynamicStepDivider)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getDynamicStepDivider"));
                v_repDyn_getEngineInfo=(ptrv_repDyn_getEngineInfo)(VVarious::resolveLibraryFuncName(lib,"v_repDyn_getEngineInfo"));

                // For the mesh calc plugin:
                v_repMesh_createCollisionInformationStructure=(ptrv_repMesh_createCollisionInformationStructure)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createCollisionInformationStructure"));
                v_repMesh_copyCollisionInformationStructure=(ptrv_repMesh_copyCollisionInformationStructure)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_copyCollisionInformationStructure"));
                v_repMesh_destroyCollisionInformationStructure=(ptrv_repMesh_destroyCollisionInformationStructure)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_destroyCollisionInformationStructure"));
                v_repMesh_scaleCollisionInformationStructure=(ptrv_repMesh_scaleCollisionInformationStructure)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_scaleCollisionInformationStructure"));
                v_repMesh_getCollisionInformationStructureSerializationData=(ptrv_repMesh_getCollisionInformationStructureSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCollisionInformationStructureSerializationData"));
                v_repMesh_getCollisionInformationStructureFromSerializationData=(ptrv_repMesh_getCollisionInformationStructureFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCollisionInformationStructureFromSerializationData"));
                v_repMesh_releaseBuffer=(ptrv_repMesh_releaseBuffer)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_releaseBuffer"));
                v_repMesh_getCutMesh=(ptrv_repMesh_getCutMesh)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCutMesh"));
                v_repMesh_getCalculatedTriangleCount=(ptrv_repMesh_getCalculatedTriangleCount)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedTriangleCount"));
                v_repMesh_getCalculatedTrianglesPointer=(ptrv_repMesh_getCalculatedTrianglesPointer)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedTrianglesPointer"));
                v_repMesh_getCalculatedVerticeCount=(ptrv_repMesh_getCalculatedVerticeCount)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedVerticeCount"));
                v_repMesh_getCalculatedVerticesPointer=(ptrv_repMesh_getCalculatedVerticesPointer)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedVerticesPointer"));
                v_repMesh_getCalculatedSegmentCount=(ptrv_repMesh_getCalculatedSegmentCount)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedSegmentCount"));
                v_repMesh_getCalculatedSegmentsPointer=(ptrv_repMesh_getCalculatedSegmentsPointer)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedSegmentsPointer"));
                v_repMesh_getCalculatedPolygonCount=(ptrv_repMesh_getCalculatedPolygonCount)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedPolygonCount"));
                v_repMesh_getCalculatedPolygonSize=(ptrv_repMesh_getCalculatedPolygonSize)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedPolygonSize"));
                v_repMesh_getCalculatedPolygonArrayPointer=(ptrv_repMesh_getCalculatedPolygonArrayPointer)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedPolygonArrayPointer"));
                v_repMesh_getCalculatedTriangleAt=(ptrv_repMesh_getCalculatedTriangleAt)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getCalculatedTriangleAt"));
                v_repMesh_getMeshMeshCollision=(ptrv_repMesh_getMeshMeshCollision)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getMeshMeshCollision"));
                v_repMesh_getTriangleTriangleCollision=(ptrv_repMesh_getTriangleTriangleCollision)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getTriangleTriangleCollision"));
                v_repMesh_getBoxBoxCollision=(ptrv_repMesh_getBoxBoxCollision)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getBoxBoxCollision"));
                v_repMesh_getBoxPointCollision=(ptrv_repMesh_getBoxPointCollision)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getBoxPointCollision"));
                v_repMesh_getMeshMeshDistance=(ptrv_repMesh_getMeshMeshDistance)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getMeshMeshDistance"));
                v_repMesh_getDistanceAgainstDummy_ifSmaller=(ptrv_repMesh_getDistanceAgainstDummy_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getDistanceAgainstDummy_ifSmaller"));
                v_repMesh_getBoxPointDistance=(ptrv_repMesh_getBoxPointDistance)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getBoxPointDistance"));
                v_repMesh_getApproximateBoxBoxDistance=(ptrv_repMesh_getApproximateBoxBoxDistance)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getApproximateBoxBoxDistance"));
                v_repMesh_getTriangleTriangleDistance_ifSmaller=(ptrv_repMesh_getTriangleTriangleDistance_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getTriangleTriangleDistance_ifSmaller"));
                v_repMesh_getTrianglePointDistance_ifSmaller=(ptrv_repMesh_getTrianglePointDistance_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getTrianglePointDistance_ifSmaller"));
                v_repMesh_getRayProxSensorDistance_ifSmaller=(ptrv_repMesh_getRayProxSensorDistance_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getRayProxSensorDistance_ifSmaller"));
                v_repMesh_isPointInsideVolume1AndOutsideVolume2=(ptrv_repMesh_isPointInsideVolume1AndOutsideVolume2)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_isPointInsideVolume1AndOutsideVolume2"));
                v_repMesh_isPointTouchingVolume=(ptrv_repMesh_isPointTouchingVolume)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_isPointTouchingVolume"));
                v_repMesh_getProxSensorDistance_ifSmaller=(ptrv_repMesh_getProxSensorDistance_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getProxSensorDistance_ifSmaller"));
                v_repMesh_getProxSensorDistanceToSegment_ifSmaller=(ptrv_repMesh_getProxSensorDistanceToSegment_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getProxSensorDistanceToSegment_ifSmaller"));
                v_repMesh_getProxSensorDistanceToTriangle_ifSmaller=(ptrv_repMesh_getProxSensorDistanceToTriangle_ifSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getProxSensorDistanceToTriangle_ifSmaller"));
                v_repMesh_cutNodeWithVolume=(ptrv_repMesh_cutNodeWithVolume)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_cutNodeWithVolume"));
                v_repMesh_createPointCloud=(ptrv_repMesh_createPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createPointCloud"));
                v_repMesh_createColorPointCloud=(ptrv_repMesh_createColorPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createColorPointCloud"));
                v_repMesh_copyPointCloud=(ptrv_repMesh_copyPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_copyPointCloud"));
                v_repMesh_destroyPointCloud=(ptrv_repMesh_destroyPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_destroyPointCloud"));
                v_repMesh_scalePointCloud=(ptrv_repMesh_scalePointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_scalePointCloud"));
                v_repMesh_insertPointsIntoPointCloud=(ptrv_repMesh_insertPointsIntoPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertPointsIntoPointCloud"));
                v_repMesh_insertColorPointsIntoPointCloud=(ptrv_repMesh_insertColorPointsIntoPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertColorPointsIntoPointCloud"));
                v_repMesh_removePointCloudPoints=(ptrv_repMesh_removePointCloudPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_removePointCloudPoints"));
                v_repMesh_intersectPointCloudPoints=(ptrv_repMesh_intersectPointCloudPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_intersectPointCloudPoints"));
                v_repMesh_getPointCloudDebugCorners=(ptrv_repMesh_getPointCloudDebugCorners)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudDebugCorners"));
                v_repMesh_getPointCloudSerializationData=(ptrv_repMesh_getPointCloudSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudSerializationData"));
                v_repMesh_getPointCloudFromSerializationData=(ptrv_repMesh_getPointCloudFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudFromSerializationData"));
                v_repMesh_createOctreeFromPoints=(ptrv_repMesh_createOctreeFromPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createOctreeFromPoints"));
                v_repMesh_createOctreeFromColorPoints=(ptrv_repMesh_createOctreeFromColorPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createOctreeFromColorPoints"));
                v_repMesh_copyOctree=(ptrv_repMesh_copyOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_copyOctree"));
                v_repMesh_destroyOctree=(ptrv_repMesh_destroyOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_destroyOctree"));
                v_repMesh_scaleOctree=(ptrv_repMesh_scaleOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_scaleOctree"));
                v_repMesh_removeOctreeVoxelsFromPoints=(ptrv_repMesh_removeOctreeVoxelsFromPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_removeOctreeVoxelsFromPoints"));
                v_repMesh_insertPointsIntoOctree=(ptrv_repMesh_insertPointsIntoOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertPointsIntoOctree"));
                v_repMesh_insertColorPointsIntoOctree=(ptrv_repMesh_insertColorPointsIntoOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertColorPointsIntoOctree"));
                v_repMesh_getOctreeVoxels=(ptrv_repMesh_getOctreeVoxels)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeVoxels"));
                v_repMesh_getPointCloudPointData=(ptrv_repMesh_getPointCloudPointData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudPointData"));
                v_repMesh_getPartialPointCloudPointData=(ptrv_repMesh_getPartialPointCloudPointData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPartialPointCloudPointData"));
                v_repMesh_getOctreeDebugCorners=(ptrv_repMesh_getOctreeDebugCorners)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeDebugCorners"));
                v_repMesh_getOctreeSerializationData=(ptrv_repMesh_getOctreeSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeSerializationData"));
                v_repMesh_getOctreeFromSerializationData=(ptrv_repMesh_getOctreeFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeFromSerializationData"));
                v_repMesh_createOctreeFromShape=(ptrv_repMesh_createOctreeFromShape)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createOctreeFromShape"));
                v_repMesh_removeOctreeVoxelsFromShape=(ptrv_repMesh_removeOctreeVoxelsFromShape)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_removeOctreeVoxelsFromShape"));
                v_repMesh_insertShapeIntoOctree=(ptrv_repMesh_insertShapeIntoOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertShapeIntoOctree"));
                v_repMesh_createOctreeFromOctree=(ptrv_repMesh_createOctreeFromOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_createOctreeFromOctree"));
                v_repMesh_insertOctreeIntoOctree=(ptrv_repMesh_insertOctreeIntoOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_insertOctreeIntoOctree"));
                v_repMesh_removeOctreeVoxelsFromOctree=(ptrv_repMesh_removeOctreeVoxelsFromOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_removeOctreeVoxelsFromOctree"));
                v_repMesh_checkOctreeCollisionWithShape=(ptrv_repMesh_checkOctreeCollisionWithShape)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_checkOctreeCollisionWithShape"));
                v_repMesh_checkOctreeCollisionWithOctree=(ptrv_repMesh_checkOctreeCollisionWithOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_checkOctreeCollisionWithOctree"));
                v_repMesh_checkOctreeCollisionWithSinglePoint=(ptrv_repMesh_checkOctreeCollisionWithSinglePoint)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_checkOctreeCollisionWithSinglePoint"));
                v_repMesh_checkOctreeCollisionWithSeveralPoints=(ptrv_repMesh_checkOctreeCollisionWithSeveralPoints)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_checkOctreeCollisionWithSeveralPoints"));
                v_repMesh_checkOctreeCollisionWithPointCloud=(ptrv_repMesh_checkOctreeCollisionWithPointCloud)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_checkOctreeCollisionWithPointCloud"));
                v_repMesh_getPointCloudDistanceToPointIfSmaller=(ptrv_repMesh_getPointCloudDistanceToPointIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudDistanceToPointIfSmaller"));
                v_repMesh_getPointCloudDistanceToPointCloudIfSmaller=(ptrv_repMesh_getPointCloudDistanceToPointCloudIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudDistanceToPointCloudIfSmaller"));
                v_repMesh_getPointCloudPointsFromCache=(ptrv_repMesh_getPointCloudPointsFromCache)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudPointsFromCache"));
                v_repMesh_getPointCloudNonEmptyCellCount=(ptrv_repMesh_getPointCloudNonEmptyCellCount)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudNonEmptyCellCount"));
                v_repMesh_getOctreeDistanceToPointIfSmaller=(ptrv_repMesh_getOctreeDistanceToPointIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeDistanceToPointIfSmaller"));
                v_repMesh_getOctreeCellFromCache=(ptrv_repMesh_getOctreeCellFromCache)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeCellFromCache"));
                v_repMesh_getOctreeDistanceToOctreeIfSmaller=(ptrv_repMesh_getOctreeDistanceToOctreeIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeDistanceToOctreeIfSmaller"));
                v_repMesh_getOctreeDistanceToPointCloudIfSmaller=(ptrv_repMesh_getOctreeDistanceToPointCloudIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeDistanceToPointCloudIfSmaller"));
                v_repMesh_getOctreeDistanceToShapeIfSmaller=(ptrv_repMesh_getOctreeDistanceToShapeIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getOctreeDistanceToShapeIfSmaller"));
                v_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller=(ptrv_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller"));
                v_repMesh_getPointCloudDistanceToShapeIfSmaller=(ptrv_repMesh_getPointCloudDistanceToShapeIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getPointCloudDistanceToShapeIfSmaller"));
                v_repMesh_getMinDistBetweenPointAndTriangleIfSmaller=(ptrv_repMesh_getMinDistBetweenPointAndTriangleIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getMinDistBetweenPointAndTriangleIfSmaller"));
                v_repMesh_getBoxBoxDistance=(ptrv_repMesh_getBoxBoxDistance)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getBoxBoxDistance"));
                v_repMesh_getProxSensorPointCloudDistanceIfSmaller=(ptrv_repMesh_getProxSensorPointCloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getProxSensorPointCloudDistanceIfSmaller"));
                v_repMesh_getRayProxSensorOctreeDistanceIfSmaller=(ptrv_repMesh_getRayProxSensorOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getRayProxSensorOctreeDistanceIfSmaller"));
                v_repMesh_getProxSensorOctreeDistanceIfSmaller=(ptrv_repMesh_getProxSensorOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_getProxSensorOctreeDistanceIfSmaller"));
                v_repMesh_removePointCloudPointsFromOctree=(ptrv_repMesh_removePointCloudPointsFromOctree)(VVarious::resolveLibraryFuncName(lib,"v_repMesh_removePointCloudPointsFromOctree"));
                if (v_repMesh_createCollisionInformationStructure!=nullptr)
                    CPluginContainer::currentMeshEngine=this;

                _codeEditor_openModal=(ptrCodeEditor_openModal)(VVarious::resolveLibraryFuncName(lib,"codeEditor_openModal"));
                _codeEditor_open=(ptrCodeEditor_open)(VVarious::resolveLibraryFuncName(lib,"codeEditor_open"));
                _codeEditor_setText=(ptrCodeEditor_setText)(VVarious::resolveLibraryFuncName(lib,"codeEditor_setText"));
                _codeEditor_getText=(ptrCodeEditor_getText)(VVarious::resolveLibraryFuncName(lib,"codeEditor_getText"));
                _codeEditor_show=(ptrCodeEditor_show)(VVarious::resolveLibraryFuncName(lib,"codeEditor_show"));
                _codeEditor_close=(ptrCodeEditor_close)(VVarious::resolveLibraryFuncName(lib,"codeEditor_close"));
                if (_codeEditor_openModal!=nullptr)
                    CPluginContainer::currentCodeEditor=this;

                _customUi_msgBox=(ptrCustomUi_msgBox)(VVarious::resolveLibraryFuncName(lib,"customUi_msgBox"));
                _customUi_fileDialog=(ptrCustomUi_fileDialog)(VVarious::resolveLibraryFuncName(lib,"customUi_fileDialog"));
                if (_customUi_msgBox!=nullptr)
                    CPluginContainer::currentCustomUi=this;

                _assimp_importShapes=(ptrassimp_importShapes)(VVarious::resolveLibraryFuncName(lib,"assimp_importShapes"));
                _assimp_exportShapes=(ptrassimp_exportShapes)(VVarious::resolveLibraryFuncName(lib,"assimp_exportShapes"));
                _assimp_importMeshes=(ptrassimp_importMeshes)(VVarious::resolveLibraryFuncName(lib,"assimp_importMeshes"));
                _assimp_exportMeshes=(ptrassimp_exportMeshes)(VVarious::resolveLibraryFuncName(lib,"assimp_exportMeshes"));
                if (_assimp_importShapes!=nullptr)
                    CPluginContainer::currentAssimp=this;



                // For other specific plugins:
                if (pov!=nullptr)
                    CPluginContainer::_povRayAddress=pov;
                if (extRenderer!=nullptr)
                    CPluginContainer::_extRendererAddress=extRenderer;
                if (extRendererWindowed!=nullptr)
                    CPluginContainer::_extRendererWindowedAddress=extRendererWindowed;
                if (openGl3!=nullptr)
                    CPluginContainer::_openGl3Address=openGl3;
                if (openGl3Windowed!=nullptr)
                    CPluginContainer::_openGl3WindowedAddress=openGl3Windowed;

                if (qhull!=nullptr)
                    CPluginContainer::_qhullAddress=qhull;
                if (hacd!=nullptr)
                    CPluginContainer::_hacdAddress=hacd;
                if (vhacd!=nullptr)
                    CPluginContainer::_vhacdAddress=vhacd;
                if (meshDecimator!=nullptr)
                    CPluginContainer::_meshDecimatorAddress=meshDecimator;

                return(int(pluginVersion)); // success!
            }
            VVarious::closeLibrary(instance);
            instance=nullptr;
            return(0); // could not properly initialize
        }
        else
        {
            VVarious::closeLibrary(instance);
            instance=nullptr;
            return(-1); // missing entry points
        }
    }
    else
        return(-2); // could not load
}

void* CPlugin::sendEventCallbackMessage(int msg,int* auxVals,void* data,int retVals[4])
{
    return(messageAddress(msg,auxVals,data,retVals));
}


int CPluginContainer::_nextHandle=0;
std::vector<CPlugin*> CPluginContainer::_allPlugins;

std::vector<std::string> CPluginContainer::_renderingpass_eventEnabledPluginNames;
std::vector<std::string> CPluginContainer::_opengl_eventEnabledPluginNames;
std::vector<std::string> CPluginContainer::_openglframe_eventEnabledPluginNames;
std::vector<std::string> CPluginContainer::_openglcameraview_eventEnabledPluginNames;

ptrExtRenderer CPluginContainer::_povRayAddress=nullptr;
ptrExtRenderer CPluginContainer::_extRendererAddress=nullptr;
ptrExtRenderer CPluginContainer::_extRendererWindowedAddress=nullptr;
ptrExtRenderer CPluginContainer::_openGl3Address=nullptr;
ptrExtRenderer CPluginContainer::_openGl3WindowedAddress=nullptr;
ptrExtRenderer CPluginContainer::_activeExtRendererAddress=nullptr;
//int CPluginContainer::_extRendererIndex=0;


ptrQhull CPluginContainer::_qhullAddress=nullptr;
ptrHACD CPluginContainer::_hacdAddress=nullptr;
ptrVHACD CPluginContainer::_vhacdAddress=nullptr;
ptrMeshDecimator CPluginContainer::_meshDecimatorAddress=nullptr;

CPlugin* CPluginContainer::currentDynEngine=nullptr;
CPlugin* CPluginContainer::currentMeshEngine=nullptr;
CPlugin* CPluginContainer::currentCodeEditor=nullptr;
CPlugin* CPluginContainer::currentCustomUi=nullptr;
CPlugin* CPluginContainer::currentAssimp=nullptr;

VMutex _meshMutex;

CPluginContainer::CPluginContainer()
{
}

CPluginContainer::~CPluginContainer()
{
}

int CPluginContainer::addPlugin(const char* filename,const char* pluginName)
{
    FUNCTION_DEBUG;
    if (filename[0]==0)
    {       
        CPathPlanningInterface::initializeFunctionsIfNeeded();
        CPathPlanningInterface::setSystemPluginsLoadPhaseOver();
        return(-1);
    }
    FUNCTION_INSIDE_DEBUG(pluginName);

    CPlugin* plug=getPluginFromName(pluginName);
    if (plug!=nullptr)
    {
        plug->_loadCount++;
        return(plug->handle);
    }
    plug=new CPlugin(filename,pluginName);
    plug->handle=_nextHandle;
    _allPlugins.push_back(plug);
    int loadRes=plug->load();
    if (loadRes<=0)
    { // failed
        _allPlugins.pop_back();
        delete plug;
        return(loadRes-1);
    }
    _nextHandle++;
    return(plug->handle);
}

CPlugin* CPluginContainer::getPluginFromName(const char* pluginName)
{
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->name==std::string(pluginName))
            return(_allPlugins[i]);
    }
    return(nullptr);
}

CPlugin* CPluginContainer::getPluginFromIndex(int index)
{
    if ((index<0)||(index>=int(_allPlugins.size())))
        return(nullptr);
    return(_allPlugins[index]);
}

bool CPluginContainer::killPlugin(int handle)
{
    FUNCTION_DEBUG;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->handle==handle)
        {
            FUNCTION_INSIDE_DEBUG(_allPlugins[i]->name.c_str());

#ifdef SIM_WITH_GUI
            if ( (App::mainWindow!=nullptr)&&(_allPlugins[i]->name.compare("CodeEditor")==0) )
                App::mainWindow->codeEditorContainer->closeAll();
#endif

            if (_allPlugins[i]->_loadCount==1)
            { // will unload it
                _allPlugins[i]->endAddress();
                std::string nm(_allPlugins[i]->name);
                delete _allPlugins[i];
                _allPlugins.erase(_allPlugins.begin()+i);
                App::ct->luaCustomFuncAndVarContainer->announcePluginWasKilled(nm.c_str());
                return(true);
            }
            else
            { // cannot yet unload it... others might still depend on it!
                _allPlugins[i]->_loadCount--;
                return(false);
            }
        }
    }
    return(false);
}

int CPluginContainer::getPluginCount()
{
    return(int(_allPlugins.size()));
}

void* CPluginContainer::sendEventCallbackMessageToOnePlugin(const char* pluginName,int msg,int* auxVals,void* data,int retVals[4])
{
    CPlugin* plug=getPluginFromName(pluginName);
    if (plug!=nullptr)
    {
        if (retVals!=nullptr)
        {
            retVals[0]=-1;
            retVals[1]=-1;
            retVals[2]=-1;
            retVals[3]=-1;
        }
        void* returnData=plug->messageAddress(msg,auxVals,data,retVals);
        return(returnData);
    }
    return(nullptr);
}

void* CPluginContainer::sendEventCallbackMessageToAllPlugins(int msg,int* auxVals,void* data,int retVals[4])
{
    bool special=false;
    int memorized[4]={0,0,0,0};
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (retVals!=nullptr)
        {
            retVals[0]=-1;
            retVals[1]=-1;
            retVals[2]=-1;
            retVals[3]=-1;
        }
        void* returnData=_allPlugins[i]->messageAddress(msg,auxVals,data,retVals);
        if ( (returnData!=nullptr)||((retVals!=nullptr)&&((retVals[0]!=-1)||(retVals[1]!=-1)||(retVals[2]!=-1)||(retVals[3]!=-1))) )
        {
            if (msg!=sim_message_eventcallback_mainscriptabouttobecalled) // this message is handled in a special fashion, because the remoteApi and ROS might interfere otherwise!
                return(returnData); // We interrupt the callback forwarding!
            memorized[0]=retVals[0];
            memorized[1]=retVals[1];
            memorized[2]=retVals[2];
            memorized[3]=retVals[3];
            special=true;
        }
    }
    if (special)
    {
        retVals[0]=memorized[0];
        retVals[1]=memorized[1];
        retVals[2]=memorized[2];
        retVals[3]=memorized[3];
    }
    return(nullptr);
}

bool CPluginContainer::shouldSend_openglframe_msg()
{
    return(_openglframe_eventEnabledPluginNames.size()>0);
}

bool CPluginContainer::shouldSend_openglcameraview_msg()
{
    return(_openglcameraview_eventEnabledPluginNames.size()>0);
}

bool CPluginContainer::enableOrDisableSpecificEventCallback(int eventCallbackType,const char* pluginName)
{ // resources should normally be locked at this stage
    std::vector<std::string>* vect=nullptr;
    if (eventCallbackType==sim_message_eventcallback_renderingpass)
        vect=&_renderingpass_eventEnabledPluginNames;
    if (eventCallbackType==sim_message_eventcallback_opengl)
        vect=&_opengl_eventEnabledPluginNames;
    if (eventCallbackType==sim_message_eventcallback_openglframe)
        vect=&_openglframe_eventEnabledPluginNames;
    if (eventCallbackType==sim_message_eventcallback_openglcameraview)
        vect=&_openglcameraview_eventEnabledPluginNames;

    bool present=false;
    for (int i=0;i<int(vect->size());i++)
    {
        if (vect->at(i).compare(pluginName)==0)
        { // already there. We remove it
            present=true;
            vect->erase(vect->begin()+i);
            break;
        }
    }
    if (!present)
        vect->push_back(pluginName); // not yet there, we add it
    return(!present);
}

void CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(int msg,int auxVal0,int auxVal1,int auxVal2,int auxVal3)
{
    int auxVals[4];
    int retVals[4];
    auxVals[0]=auxVal0;
    auxVals[1]=auxVal1;
    auxVals[2]=auxVal2;
    auxVals[3]=auxVal3;
    sendSpecialEventCallbackMessageToSomePlugins(msg,auxVals,nullptr,retVals);
}

void CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(int msg,int* auxVals,void* data,int retVals[4])
{
    std::vector<std::string>* vect=nullptr;
    if (msg==sim_message_eventcallback_renderingpass)
        vect=&_renderingpass_eventEnabledPluginNames;
    if (msg==sim_message_eventcallback_opengl)
        vect=&_opengl_eventEnabledPluginNames;
    if (msg==sim_message_eventcallback_openglframe)
        vect=&_openglframe_eventEnabledPluginNames;
    if (msg==sim_message_eventcallback_openglcameraview)
        vect=&_openglcameraview_eventEnabledPluginNames;

    if (vect!=nullptr)
    {
        for (int i=0;i<int(vect->size());i++)
        {
            CPlugin* plug=getPluginFromName(vect->at(i).c_str());
            if (plug!=nullptr)
            {
                void* returnData=plug->messageAddress(msg,auxVals,data,retVals);
                if (returnData!=nullptr)
                    delete[] (char*)returnData;
            }
        }
    }
}

bool CPluginContainer::selectExtRenderer(int index)
{
    _activeExtRendererAddress=nullptr;
    if (index==sim_rendermode_povray-sim_rendermode_povray)
        _activeExtRendererAddress=_povRayAddress;
    if (index==sim_rendermode_extrenderer-sim_rendermode_povray)
        _activeExtRendererAddress=_extRendererAddress;
    if (index==sim_rendermode_extrendererwindowed-sim_rendermode_povray)
        _activeExtRendererAddress=_extRendererWindowedAddress;
    if (index==sim_rendermode_opengl3-sim_rendermode_povray)
        _activeExtRendererAddress=_openGl3Address;
    if (index==sim_rendermode_opengl3windowed-sim_rendermode_povray)
        _activeExtRendererAddress=_openGl3WindowedAddress;
    return(_activeExtRendererAddress!=nullptr);
}

bool CPluginContainer::extRenderer(int msg,void* data)
{
    if (_activeExtRendererAddress!=nullptr)
    {
        _activeExtRendererAddress(msg,data);
        return(true);
    }
    return(false);
/*  else
    { // old way of doing it:
        if (_extRendererIndex==0)
        {
            int auxVals[4];
            int retVals[4];
            auxVals[0]=0;
            auxVals[1]=0;
            auxVals[2]=0;
            auxVals[3]=-1;
            CPluginContainer::sendEventCallbackMessageToAllPlugins(msg,auxVals,data,retVals);
        }
    }
    return(true);
    */
}

bool CPluginContainer::qhull(void* data)
{
    if (_qhullAddress!=nullptr)
    {
        _qhullAddress(data);
        return(true);
    }
    return(false);
}

bool CPluginContainer::hacd(void* data)
{
    if (_hacdAddress!=nullptr)
    {
        _hacdAddress(data);
        return(true);
    }
    return(false);
}

bool CPluginContainer::vhacd(void* data)
{
    if (_vhacdAddress!=nullptr)
    {
        _vhacdAddress(data);
        return(true);
    }
    return(false);
}

bool CPluginContainer::meshDecimator(void* data)
{
    if (_meshDecimatorAddress!=nullptr)
    {
        _meshDecimatorAddress(data);
        return(true);
    }
    return(false);
}

bool CPluginContainer::dyn_startSimulation(int engine,int version,const float floatParams[20],const int intParams[20])
{
    bool retVal=false;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->v_repDyn_startSimulation!=nullptr)
        {
            if (_allPlugins[i]->v_repDyn_startSimulation(engine,version,floatParams,intParams)!=0)
            { // success with this plugin!
                currentDynEngine=_allPlugins[i];
                retVal=true;
                break;
            }
        }
    }
    return(retVal);
}

void CPluginContainer::dyn_endSimulation()
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->v_repDyn_endSimulation();
    currentDynEngine=nullptr;
}

void CPluginContainer::dyn_step(float timeStep,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->v_repDyn_step(timeStep,simulationTime);
}

bool CPluginContainer::dyn_isInitialized()
{
    return(currentDynEngine!=nullptr);
}

bool CPluginContainer::dyn_isDynamicContentAvailable()
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_isDynamicContentAvailable()!=0);
    return(false);
}

void CPluginContainer::dyn_serializeDynamicContent(const char* filenameAndPath,int bulletSerializationBuffer)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->v_repDyn_serializeDynamicContent(filenameAndPath,bulletSerializationBuffer);
}

int CPluginContainer::dyn_addParticleObject(int objectType,float size,float massOverVolume,const void* params,float lifeTime,int maxItemCount,const float* ambient,const float* diffuse,const float* specular,const float* emission)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_addParticleObject(objectType,size,massOverVolume,params,lifeTime,maxItemCount,ambient,diffuse,specular,emission));
    return(-1);
}

bool CPluginContainer::dyn_removeParticleObject(int objectHandle)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_removeParticleObject(objectHandle)!=0);
    return(false);
}

bool CPluginContainer::dyn_addParticleObjectItem(int objectHandle,const float* itemData,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_addParticleObjectItem(objectHandle,itemData,simulationTime)!=0);
    return(false);
}

int CPluginContainer::dyn_getParticleObjectOtherFloatsPerItem(int objectHandle)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getParticleObjectOtherFloatsPerItem(objectHandle));
    return(0);
}

float* CPluginContainer::dyn_getContactPoints(int* count)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getContactPoints(count));
    count[0]=0;
    return(nullptr);
}

void** CPluginContainer::dyn_getParticles(int index,int* particlesCount,int* objectType,float** cols)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getParticles(index,particlesCount,objectType,cols));
    return(nullptr);
}

bool CPluginContainer::dyn_getParticleData(const void* particle,float* pos,float* size,int* objectType,float** additionalColor)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getParticleData(particle,pos,size,objectType,additionalColor)!=0);
    return(false);
}

bool CPluginContainer::dyn_getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],float contactInfo[6])
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo)!=0);
    return(false);
}

void CPluginContainer::dyn_reportDynamicWorldConfiguration(int totalPassesCount,char doNotApplyJointIntrinsicPositions,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->v_repDyn_reportDynamicWorldConfiguration(totalPassesCount,doNotApplyJointIntrinsicPositions,simulationTime);
}

int CPluginContainer::dyn_getDynamicStepDivider()
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getDynamicStepDivider());
    return(0);
}

int CPluginContainer::dyn_getEngineInfo(int* engine,int* data1,char* data2,char* data3)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->v_repDyn_getEngineInfo(engine,data1,data2,data3));
    return(-1);
}

bool CPluginContainer::isMeshPluginAvailable()
{
    return(currentMeshEngine!=nullptr);
}

bool CPluginContainer::isCodeEditorPluginAvailable()
{
    return(currentCodeEditor!=nullptr);
}

bool CPluginContainer::isCustomUiPluginAvailable()
{
    return(currentCustomUi!=nullptr);
}

bool CPluginContainer::isAssimpPluginAvailable()
{
    return(currentAssimp!=nullptr);
}

void CPluginContainer::mesh_lockUnlock(bool lock)
{
    if (lock)
        _meshMutex.lock();
    else
        _meshMutex.unlock();
}

void* CPluginContainer::mesh_createCollisionInformationStructure(const float* cumulMeshVertices,int cumulMeshVerticesSize,const int* cumulMeshIndices,int cumulMeshIndicesSize,float maxTriSize,float edgeAngle,int maxTriCount)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createCollisionInformationStructure(cumulMeshVertices,cumulMeshVerticesSize,cumulMeshIndices,cumulMeshIndicesSize,maxTriSize,edgeAngle,maxTriCount));
    return(nullptr);
}

void* CPluginContainer::mesh_copyCollisionInformationStructure(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_copyCollisionInformationStructure(collInfo));
    return(nullptr);
}

void CPluginContainer::mesh_destroyCollisionInformationStructure(void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_destroyCollisionInformationStructure(collInfo);
}

void CPluginContainer::mesh_scaleCollisionInformationStructure(void* collInfo,float scaleFactor)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_scaleCollisionInformationStructure(collInfo,scaleFactor);
}

unsigned char* CPluginContainer::mesh_getCollisionInformationStructureSerializationData(const void* collInfo,int& dataSize)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCollisionInformationStructureSerializationData(collInfo,&dataSize));
    return(nullptr);
}

void* CPluginContainer::mesh_getCollisionInformationStructureFromSerializationData(const unsigned char* data,const float* cumulMeshVertices,int cumulMeshVerticesSize,const int* cumulMeshIndices,int cumulMeshIndicesSize)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCollisionInformationStructureFromSerializationData(data,cumulMeshVertices,cumulMeshVerticesSize,cumulMeshIndices,cumulMeshIndicesSize));
    return(nullptr);
}

void CPluginContainer::mesh_releaseBuffer(void* buffer)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_releaseBuffer(buffer);
}

bool CPluginContainer::mesh_getCutMesh(const void* collInfo,const C7Vector* tr,float** vertices,int* verticesSize,int** indices,int* indicesSize,int options)
{
    if (currentMeshEngine!=nullptr)
    {
        float tr_[7];
        tr->getInternalData(tr_);
        return(currentMeshEngine->v_repMesh_getCutMesh(collInfo,tr_,vertices,verticesSize,indices,indicesSize,options)!=0);
    }
    return(true);
}

int CPluginContainer::mesh_getCalculatedTriangleCount(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedTriangleCount(collInfo));
    return(0);
}

int* CPluginContainer::mesh_getCalculatedTrianglesPointer(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedTrianglesPointer(collInfo));
    return(nullptr);
}

int CPluginContainer::mesh_getCalculatedVerticeCount(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedVerticeCount(collInfo));
    return(0);
}

float* CPluginContainer::mesh_getCalculatedVerticesPointer(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedVerticesPointer(collInfo));
    return(nullptr);
}

int CPluginContainer::mesh_getCalculatedSegmentCount(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedSegmentCount(collInfo));
    return(0);
}

int* CPluginContainer::mesh_getCalculatedSegmentsPointer(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedSegmentsPointer(collInfo));
    return(nullptr);
}

int CPluginContainer::mesh_getCalculatedPolygonCount(const void* collInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedPolygonCount(collInfo));
    return(0);
}

int CPluginContainer::mesh_getCalculatedPolygonSize(const void* collInfo,int polygonIndex)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedPolygonSize(collInfo,polygonIndex));
    return(0);
}

int* CPluginContainer::mesh_getCalculatedPolygonArrayPointer(const void* collInfo,int polygonIndex)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedPolygonArrayPointer(collInfo,polygonIndex));
    return(nullptr);
}

bool CPluginContainer::mesh_getCalculatedTriangleAt(const void* collInfo,C3Vector& a0,C3Vector& a1,C3Vector& a2,int ind)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getCalculatedTriangleAt(collInfo,a0.data,a1.data,a2.data,ind)!=0);
    return(false);
}

bool CPluginContainer::mesh_getMeshMeshCollision(const void* collInfo1,const void* collInfo2,const C4X4Matrix collObjMatr[2],const void* collInfos[2],bool inverseExploration,std::vector<float>* intersections,int caching[2])
{
    if (currentMeshEngine!=nullptr)
    {
        float collObjMatr1[12];
        float collObjMatr2[12];
        collObjMatr[0].copyToInterface(collObjMatr1);
        collObjMatr[1].copyToInterface(collObjMatr2);
        bool retVal;
        if (intersections==nullptr)
            retVal=currentMeshEngine->v_repMesh_getMeshMeshCollision(collInfo1,collInfo2,collObjMatr1,collObjMatr2,collInfos,inverseExploration,nullptr,nullptr,caching)!=0;
        else
        {
            float* _intersections;
            int _intersectionsSize;
            retVal=currentMeshEngine->v_repMesh_getMeshMeshCollision(collInfo1,collInfo2,collObjMatr1,collObjMatr2,collInfos,inverseExploration,&_intersections,&_intersectionsSize,caching)!=0;
            if (retVal)
                intersections->assign(_intersections,_intersections+_intersectionsSize);
            mesh_releaseBuffer(_intersections);
        }
        return(retVal);
    }
    return(false);
}

int CPluginContainer::mesh_getTriangleTriangleCollision(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& b0,const C3Vector& f0,const C3Vector& f1,C3Vector* intSegPart0,C3Vector* intSegPart1,bool getIntersection)
{
    if (currentMeshEngine!=nullptr)
    {
        int retVal;
        if (intSegPart0==nullptr)
            retVal=currentMeshEngine->v_repMesh_getTriangleTriangleCollision(a0.data,e0.data,e1.data,b0.data,f0.data,f1.data,nullptr,nullptr,getIntersection);
        else
        {
            float v0[3];
            float v1[3];
            retVal=currentMeshEngine->v_repMesh_getTriangleTriangleCollision(a0.data,e0.data,e1.data,b0.data,f0.data,f1.data,v0,v1,getIntersection);
            intSegPart0->setInternalData(v0);
            intSegPart1->setInternalData(v1);
        }
        return(retVal);
    }
    return(0);
}

bool CPluginContainer::mesh_getBoxBoxCollision(const C4X4Matrix& box1Tr,const C3Vector& box1Size,const C4X4Matrix& box2Tr,const C3Vector& box2Size)
{
    if (currentMeshEngine!=nullptr)
    {
        float _t1[12];
        box1Tr.copyToInterface(_t1);
        float _t2[12];
        box2Tr.copyToInterface(_t2);
        return(currentMeshEngine->v_repMesh_getBoxBoxCollision(_t1,box1Size.data,_t2,box2Size.data)!=0);
    }
    return(false);
}

bool CPluginContainer::mesh_getBoxPointCollision(const C4X4Matrix& boxTr,const C3Vector& boxSize,const C3Vector& point)
{
    if (currentMeshEngine!=nullptr)
    {
        float _t[12];
        boxTr.copyToInterface(_t);
        return(currentMeshEngine->v_repMesh_getBoxPointCollision(_t,boxSize.data,point.data)!=0);
    }
    return(false);
}

void CPluginContainer::mesh_getMeshMeshDistance(const void* collInfo1,const void* collInfo2,const C4X4Matrix distObjMatr[2],const void* collInfos[2],bool inverseExploration,float distances[7],int caching[2])
{
    if (currentMeshEngine!=nullptr)
    {
        float distObjMatr1[12];
        float distObjMatr2[12];
        distObjMatr[0].copyToInterface(distObjMatr1);
        distObjMatr[1].copyToInterface(distObjMatr2);
        currentMeshEngine->v_repMesh_getMeshMeshDistance(collInfo1,collInfo2,distObjMatr1,distObjMatr2,collInfos,inverseExploration,distances,caching);
    }
}

bool CPluginContainer::mesh_getDistanceAgainstDummy_ifSmaller(const void* collInfo,const C3Vector& dummyPos,const C4X4Matrix& itPCTM,float &dist,C3Vector& ray0,C3Vector& ray1,int& itBuff)
{
    if (currentMeshEngine!=nullptr)
    {
        float _itPCTM[12];
        itPCTM.copyToInterface(_itPCTM);
        float _ray0[3];
        float _ray1[3];
        bool retVal=currentMeshEngine->v_repMesh_getDistanceAgainstDummy_ifSmaller(collInfo,dummyPos.data,_itPCTM,&dist,_ray0,_ray1,&itBuff)!=0;
        if (retVal)
        {
            ray0.set(_ray0);
            ray1.set(_ray1);
        }
        return(retVal);
    }
    return(false);
}

float CPluginContainer::mesh_getBoxPointDistance(const C4X4Matrix& t1,const C3Vector& s1,const C3Vector& dummyPos)
{
    if (currentMeshEngine!=nullptr)
    {
        float _t1[12];
        t1.copyToInterface(_t1);
        return(currentMeshEngine->v_repMesh_getBoxPointDistance(_t1,s1.data,dummyPos.data));
    }
    return(0.0);
}

float CPluginContainer::mesh_getApproximateBoxBoxDistance(const C4X4Matrix& t1,const C3Vector& s1,const C4X4Matrix& t2,const C3Vector& s2)
{
    if (currentMeshEngine!=nullptr)
    {
        float _t1[12];
        t1.copyToInterface(_t1);
        float _t2[12];
        t2.copyToInterface(_t2);
        return(currentMeshEngine->v_repMesh_getApproximateBoxBoxDistance(_t1,s1.data,_t2,s2.data));
    }
    return(0.0);
}

bool CPluginContainer::mesh_getTriangleTriangleDistance_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& b0,const C3Vector& f0,const C3Vector& f1,float &dist,C3Vector& segA,C3Vector& segB)
{
    if (currentMeshEngine!=nullptr)
    {
        float _segA[3];
        float _segB[3];
        bool retVal=currentMeshEngine->v_repMesh_getTriangleTriangleDistance_ifSmaller(a0.data,e0.data,e1.data,b0.data,f0.data,f1.data,&dist,_segA,_segB)!=0;
        if (retVal)
        {
            segA.set(_segA);
            segB.set(_segB);
        }
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getTrianglePointDistance_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,const C3Vector& dummyPos,float &dist,C3Vector& segA)
{
    if (currentMeshEngine!=nullptr)
    {
        float _segA[3];
        bool retVal=currentMeshEngine->v_repMesh_getTrianglePointDistance_ifSmaller(a0.data,e0.data,e1.data,dummyPos.data,&dist,_segA)!=0;
        if (retVal)
            segA.set(_segA);
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getRayProxSensorDistance_ifSmaller(const void* collInfo,const C4X4Matrix& selfPCTM,float &dist,const C3Vector& lp,float closeThreshold,const C3Vector& lvFar,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,char* closeDetectionTriggered,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback)
{
    if (currentMeshEngine!=nullptr)
    {
        float _selfPCTM[12];
        selfPCTM.copyToInterface(_selfPCTM);
        float _detectPoint[3];
        float _triNormalNotNormalized[3];
        bool retVal=currentMeshEngine->v_repMesh_getRayProxSensorDistance_ifSmaller(collInfo,_selfPCTM,&dist,lp.data,closeThreshold,lvFar.data,cosAngle,_detectPoint,fast,frontFace,backFace,closeDetectionTriggered,_triNormalNotNormalized,theOcclusionCheckCallback)!=0;
        if (retVal)
        {
            detectPoint.set(_detectPoint);
            triNormalNotNormalized.set(_triNormalNotNormalized);
        }
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_isPointInsideVolume1AndOutsideVolume2(const C3Vector& p,const std::vector<float>* planes,const std::vector<float>* planesOutside)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        return(currentMeshEngine->v_repMesh_isPointInsideVolume1AndOutsideVolume2(p.data,_planes,_planesSize,_planesOutside,_planesOutsideSize)!=0);
    }
    return(false);
}

bool CPluginContainer::mesh_isPointTouchingVolume(const C3Vector& p,const std::vector<float>* planes)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        return(currentMeshEngine->v_repMesh_isPointTouchingVolume(p.data,_planes,_planesSize)!=0);
    }
    return(false);
}

bool CPluginContainer::mesh_getProxSensorDistance_ifSmaller(const void* collInfo,const C4X4Matrix& itPCTM,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,std::vector<float>* cutEdges,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        float _itPCTM[12];
        itPCTM.copyToInterface(_itPCTM);
        float _detectPoint[3];
        float _triNormalNotNormalized[3];
        bool retVal=currentMeshEngine->v_repMesh_getProxSensorDistance_ifSmaller(collInfo,_itPCTM,&dist,_planes,_planesSize,_planesOutside,_planesOutsideSize,cosAngle,_detectPoint,fast,frontFace,backFace,_triNormalNotNormalized,theOcclusionCheckCallback)!=0;
        if (retVal)
        {
            detectPoint.set(_detectPoint);
            triNormalNotNormalized.set(_triNormalNotNormalized);
        }
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getProxSensorDistanceToSegment_ifSmaller(const C3Vector& p0,const C3Vector& p1,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float maxAngle,C3Vector& detectPoint)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        float _detectPoint[3];
        bool retVal=currentMeshEngine->v_repMesh_getProxSensorDistanceToSegment_ifSmaller(p0.data,p1.data,&dist,_planes,_planesSize,_planesOutside,_planesOutsideSize,maxAngle,_detectPoint)!=0;
        if (retVal)
            detectPoint.set(_detectPoint);
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getProxSensorDistanceToTriangle_ifSmaller(const C3Vector& a0,const C3Vector& e0,const C3Vector& e1,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        float _detectPoint[3];
        float _triNormalNotNormalized[3];
        bool retVal=currentMeshEngine->v_repMesh_getProxSensorDistanceToTriangle_ifSmaller(a0.data,e0.data,e1.data,&dist,_planes,_planesSize,_planesOutside,_planesOutsideSize,cosAngle,_detectPoint,frontFace,backFace,_triNormalNotNormalized)!=0;
        if (retVal)
        {
            detectPoint.set(_detectPoint);
            triNormalNotNormalized.set(_triNormalNotNormalized);
        }
        return(retVal);
    }
    return(false);
}

float CPluginContainer::mesh_cutNodeWithVolume(void* collInfo,const C4X4Matrix& itPCTM,const std::vector<float>* planes)
{
    if (currentMeshEngine!=nullptr)
    {
        EASYLOCK(_meshMutex);
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        float _itPCTM[12];
        itPCTM.copyToInterface(_itPCTM);
        return(currentMeshEngine->v_repMesh_cutNodeWithVolume(collInfo,_itPCTM,_planes,_planesSize));
    }
    return(0.0);
}

void* CPluginContainer::mesh_createPointCloud(const float* relPoints,int ptCnt,float cellSize,int maxPointCountPerCell,const float theColor[3],float distTolerance)
{
    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createPointCloud(relPoints,ptCnt,cellSize,maxPointCountPerCell,cols,distTolerance));
    return(nullptr);
}

void* CPluginContainer::mesh_createColorPointCloud(const float* relPoints,int ptCnt,float cellSize,int maxPointCountPerCell,const unsigned char* theColors,float distTolerance)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createColorPointCloud(relPoints,ptCnt,cellSize,maxPointCountPerCell,theColors,distTolerance));
    return(nullptr);
}

void* CPluginContainer::mesh_copyPointCloud(const void* pointCloudInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_copyPointCloud(pointCloudInfo));
    return(nullptr);
}

void CPluginContainer::mesh_destroyPointCloud(void* pointCloudInfo)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_destroyPointCloud(pointCloudInfo);
}

void CPluginContainer::mesh_scalePointCloud(void* pointCloudInfo,float scaleFactor)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_scalePointCloud(pointCloudInfo,scaleFactor);
}

void CPluginContainer::mesh_insertPointsIntoPointCloud(void* pointCloudInfo,const float* relPoints,int ptCnt,const float theColor[3],float distTolerance)
{
    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertPointsIntoPointCloud(pointCloudInfo,relPoints,ptCnt,cols,distTolerance);
}

void CPluginContainer::mesh_insertColorPointsIntoPointCloud(void* pointCloudInfo,const float* relPoints,int ptCnt,const unsigned char* theColors,float distTolerance)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertColorPointsIntoPointCloud(pointCloudInfo,relPoints,ptCnt,theColors,distTolerance);
}

bool CPluginContainer::mesh_removePointCloudPoints(void* pointCloudInfo,const float* relPoints,int ptCnt,float distTolerance,int& removedCnt)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_removePointCloudPoints(pointCloudInfo,relPoints,ptCnt,distTolerance,&removedCnt)!=0);
    return(false);
}

bool CPluginContainer::mesh_intersectPointCloudPoints(void* pointCloudInfo,const float* relPoints,int ptCnt,float distTolerance)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_intersectPointCloudPoints(pointCloudInfo,relPoints,ptCnt,distTolerance)!=0);
    return(false);
}

void CPluginContainer::mesh_getPointCloudDebugCorners(const void* pointCloudInfo,std::vector<float>& cubeCorners)
{
    cubeCorners.clear();
    if (currentMeshEngine!=nullptr)
    {
        int cubeCnt;
        float* dat=currentMeshEngine->v_repMesh_getPointCloudDebugCorners(pointCloudInfo,&cubeCnt);
        cubeCorners.assign(dat,dat+cubeCnt*8*3);
        currentMeshEngine->v_repMesh_releaseBuffer(dat);
    }
}

void CPluginContainer::mesh_getPointCloudSerializationData(const void* pointCloudInfo,std::vector<unsigned char>& data)
{
    data.clear();
    if (currentMeshEngine!=nullptr)
    {
        int dataSize;
        unsigned char* d=currentMeshEngine->v_repMesh_getPointCloudSerializationData(pointCloudInfo,&dataSize);
        for (int i=0;i<dataSize;i++)
            data.push_back(d[i]);
        currentMeshEngine->v_repMesh_releaseBuffer(d);
    }
}

void* CPluginContainer::mesh_getPointCloudFromSerializationData(const std::vector<unsigned char>& data)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getPointCloudFromSerializationData(&data[0]));
    return(nullptr);
}





void* CPluginContainer::mesh_createOctreeFromPoints(const float* relPoints,int ptCnt,float cellSize,const float theColor[3],unsigned int theTag)
{
    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createOctreeFromPoints(relPoints,ptCnt,cellSize,cols,theTag));
    return(nullptr);
}

void* CPluginContainer::mesh_createOctreeFromColorPoints(const float* relPoints,int ptCnt,float cellSize,const unsigned char* theColors,const unsigned int* theTags)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createOctreeFromColorPoints(relPoints,ptCnt,cellSize,theColors,theTags));
    return(nullptr);
}

void* CPluginContainer::mesh_copyOctree(const void* octreeInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_copyOctree(octreeInfo));
    return(nullptr);
}

void CPluginContainer::mesh_destroyOctree(void* octreeInfo)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_destroyOctree(octreeInfo);
}

void CPluginContainer::mesh_scaleOctree(void* octreeInfo,float scaleFactor)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_scaleOctree(octreeInfo,scaleFactor);
}

bool CPluginContainer::mesh_removeOctreeVoxelsFromPoints(void* octreeInfo,const float* relPoints,int ptCnt)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_removeOctreeVoxelsFromPoints(octreeInfo,relPoints,ptCnt)!=0);
    return(false);
}

void CPluginContainer::mesh_insertPointsIntoOctree(void* octreeInfo,const float* relPoints,int ptCnt,const float theColor[3],unsigned int theTag)
{
    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertPointsIntoOctree(octreeInfo,relPoints,ptCnt,cols,theTag);
}

void CPluginContainer::mesh_insertColorPointsIntoOctree(void* octreeInfo,const float* relPoints,int ptCnt,const unsigned char* theColors,const unsigned int* theTags)
{
    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertColorPointsIntoOctree(octreeInfo,relPoints,ptCnt,theColors,theTags);
}

void CPluginContainer::mesh_getOctreeVoxels(const void* octreeInfo,std::vector<float>& voxelPositions,std::vector<float>& voxelColors)
{
    voxelPositions.clear();
    voxelColors.clear();
    if (currentMeshEngine!=nullptr)
    {
        int voxelCnt;
        float* dat=currentMeshEngine->v_repMesh_getOctreeVoxels(octreeInfo,&voxelCnt);
        for (int i=0;i<voxelCnt;i++)
        {
            voxelPositions.push_back(dat[6*i+0]);
            voxelPositions.push_back(dat[6*i+1]);
            voxelPositions.push_back(dat[6*i+2]);
            voxelColors.push_back(dat[6*i+3]);
            voxelColors.push_back(dat[6*i+4]);
            voxelColors.push_back(dat[6*i+5]);
            voxelColors.push_back(0.0);
        }
        currentMeshEngine->v_repMesh_releaseBuffer(dat);
    }
}

void CPluginContainer::mesh_getPointCloudPointData(const void* pointCloudInfo,std::vector<float>& pointPositions,std::vector<float>& pointColors)
{
    pointPositions.clear();
    pointColors.clear();
    if (currentMeshEngine!=nullptr)
    {
        int pointCnt;
        float* dat=currentMeshEngine->v_repMesh_getPointCloudPointData(pointCloudInfo,&pointCnt);
        for (int i=0;i<pointCnt;i++)
        {
            pointPositions.push_back(dat[6*i+0]);
            pointPositions.push_back(dat[6*i+1]);
            pointPositions.push_back(dat[6*i+2]);
            pointColors.push_back(dat[6*i+3]);
            pointColors.push_back(dat[6*i+4]);
            pointColors.push_back(dat[6*i+5]);
            pointColors.push_back(0.0);
        }
        currentMeshEngine->v_repMesh_releaseBuffer(dat);
    }
}

void CPluginContainer::mesh_getPartialPointCloudPointData(const void* pointCloudInfo,std::vector<float>& pointPositions,std::vector<float>& pointColors,float ratio)
{
    pointPositions.clear();
    pointColors.clear();
    if (currentMeshEngine!=nullptr)
    {
        int pointCnt;
        float* dat=currentMeshEngine->v_repMesh_getPartialPointCloudPointData(pointCloudInfo,&pointCnt,ratio);
        for (int i=0;i<pointCnt;i++)
        {
            pointPositions.push_back(dat[6*i+0]);
            pointPositions.push_back(dat[6*i+1]);
            pointPositions.push_back(dat[6*i+2]);
            pointColors.push_back(dat[6*i+3]);
            pointColors.push_back(dat[6*i+4]);
            pointColors.push_back(dat[6*i+5]);
            pointColors.push_back(0.0);
        }
        currentMeshEngine->v_repMesh_releaseBuffer(dat);
    }
}

void CPluginContainer::mesh_getOctreeDebugCorners(const void* octreeInfo,std::vector<float>& cubeCorners)
{
    cubeCorners.clear();
    if (currentMeshEngine!=nullptr)
    {
        int cubeCnt;
        float* dat=currentMeshEngine->v_repMesh_getOctreeDebugCorners(octreeInfo,&cubeCnt);
        cubeCorners.assign(dat,dat+cubeCnt*8*3);
        currentMeshEngine->v_repMesh_releaseBuffer(dat);
    }
}

void CPluginContainer::mesh_getOctreeSerializationData(const void* octreeInfo,std::vector<unsigned char>& data)
{
    data.clear();
    if (currentMeshEngine!=nullptr)
    {
        int dataSize;
        unsigned char* d=currentMeshEngine->v_repMesh_getOctreeSerializationData(octreeInfo,&dataSize);
        for (int i=0;i<dataSize;i++)
            data.push_back(d[i]);
        currentMeshEngine->v_repMesh_releaseBuffer(d);
    }
}

void* CPluginContainer::mesh_getOctreeFromSerializationData(const std::vector<unsigned char>& data)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getOctreeFromSerializationData(&data[0]));
    return(nullptr);
}



void* CPluginContainer::mesh_createOctreeFromShape(const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM,float cellSize,const float theColor[3],unsigned int theTag)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _collnodePCTM[12];
    collnodePCTM.copyToInterface(_collnodePCTM);

    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createOctreeFromShape(_octreePCTM,collInfo,_collnodePCTM,cellSize,cols,theTag));
    return(nullptr);
}

bool CPluginContainer::mesh_removeOctreeVoxelsFromShape(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _collnodePCTM[12];
    collnodePCTM.copyToInterface(_collnodePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_removeOctreeVoxelsFromShape(octreeInfo,_octreePCTM,collInfo,_collnodePCTM)!=0);
    return(false);
}

void CPluginContainer::mesh_insertShapeIntoOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collnodePCTM,const float theColor[3],unsigned int theTag)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _collnodePCTM[12];
    collnodePCTM.copyToInterface(_collnodePCTM);

    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};

    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertShapeIntoOctree(octreeInfo,_octreePCTM,collInfo,_collnodePCTM,cols,theTag);
}

void* CPluginContainer::mesh_createOctreeFromOctree(const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM,float cellSize,const float theColor[3],unsigned int theTag)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _octree2PCTM[12];
    octree2PCTM.copyToInterface(_octree2PCTM);

    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_createOctreeFromOctree(_octreePCTM,octree2Info,_octree2PCTM,cellSize,cols,theTag));
    return(nullptr);
}

bool CPluginContainer::mesh_removeOctreeVoxelsFromOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _octree2PCTM[12];
    octree2PCTM.copyToInterface(_octree2PCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_removeOctreeVoxelsFromOctree(octreeInfo,_octreePCTM,octree2Info,_octree2PCTM)!=0);
    return(false);
}

void CPluginContainer::mesh_insertOctreeIntoOctree(void* octreeInfo,const C4X4Matrix& octreePCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM,const float theColor[3],unsigned int theTag)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _octree2PCTM[12];
    octree2PCTM.copyToInterface(_octree2PCTM);

    unsigned char cols[3]={(unsigned char)(theColor[0]*255.0f),(unsigned char)(theColor[1]*255.0f),(unsigned char)(theColor[2]*255.0f)};

    if (currentMeshEngine!=nullptr)
        currentMeshEngine->v_repMesh_insertOctreeIntoOctree(octreeInfo,_octreePCTM,octree2Info,_octree2PCTM,cols,theTag);
}

bool CPluginContainer::mesh_checkOctreeCollisionWithShape(const void* octreeInfo,const C4X4Matrix& octreePCTM,const void* collInfo,const C4X4Matrix& collNodePCTM)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _collNodePCTM[12];
    collNodePCTM.copyToInterface(_collNodePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_checkOctreeCollisionWithShape(octreeInfo,_octreePCTM,collInfo,_collNodePCTM)!=0);
    return(false);
}

bool CPluginContainer::mesh_checkOctreeCollisionWithOctree(const void* octree1Info,const C4X4Matrix& octree1PCTM,const void* octree2Info,const C4X4Matrix& octree2PCTM)
{
    float _octree1PCTM[12];
    octree1PCTM.copyToInterface(_octree1PCTM);

    float _octree2PCTM[12];
    octree2PCTM.copyToInterface(_octree2PCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_checkOctreeCollisionWithOctree(octree1Info,_octree1PCTM,octree2Info,_octree2PCTM)!=0);
    return(false);
}

bool CPluginContainer::mesh_checkOctreeCollisionWithSeveralPoints(const void* octreeInfo,const C4X4Matrix& octreePCTM,const float* absPoints,int ptCnt)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_checkOctreeCollisionWithSeveralPoints(octreeInfo,_octreePCTM,absPoints,ptCnt)!=0);
    return(false);
}

bool CPluginContainer::mesh_checkOctreeCollisionWithSinglePoint(const void* octreeInfo,const C4X4Matrix& octreePCTM,const C3Vector& absPoint,unsigned int* tag,unsigned long long int* location)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_checkOctreeCollisionWithSinglePoint(octreeInfo,_octreePCTM,absPoint.data,tag,location)!=0);
    return(false);
}

bool CPluginContainer::mesh_checkOctreeCollisionWithPointCloud(const void* octreeInfo,const C4X4Matrix& octreePCTM,const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_checkOctreeCollisionWithPointCloud(octreeInfo,_octreePCTM,pointCloudInfo,_pointCloudPCTM)!=0);
    return(false);
}

bool CPluginContainer::mesh_getPointCloudDistanceToPointIfSmaller(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const C3Vector& absPoint,float ray[7],long long int& cacheValue)
{
    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getPointCloudDistanceToPointIfSmaller(pointCloudInfo,_pointCloudPCTM,absPoint.data,ray,&cacheValue)!=0);
    return(false);
}

bool CPluginContainer::mesh_getPointCloudDistanceToPointCloudIfSmaller(const void* pointCloudInfo1,const void* pointCloudInfo2,const C4X4Matrix& thisPcPCTM,const C4X4Matrix& otherPcPCTM,float ray[7],long long int& thisCacheValue,long long int& otherCacheValue)
{
    float _pointCloud1PCTM[12];
    thisPcPCTM.copyToInterface(_pointCloud1PCTM);

    float _pointCloud2PCTM[12];
    otherPcPCTM.copyToInterface(_pointCloud2PCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getPointCloudDistanceToPointCloudIfSmaller(pointCloudInfo1,pointCloudInfo2,_pointCloud1PCTM,_pointCloud2PCTM,ray,&thisCacheValue,&otherCacheValue)!=0);
    return(false);
}

float* CPluginContainer::mesh_getPointCloudPointsFromCache(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const long long int cacheValue,int& ptCnt,C4X4Matrix& ptsRetToThisM)
{
    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    float _retM[12];

    if (currentMeshEngine!=nullptr)
    {
        float* retVal=currentMeshEngine->v_repMesh_getPointCloudPointsFromCache(pointCloudInfo,_pointCloudPCTM,cacheValue,&ptCnt,_retM);
        if (retVal!=nullptr)
            ptsRetToThisM.copyFromInterface(_retM);
        return(retVal);
    }
    return(nullptr);
}

int CPluginContainer::mesh_getPointCloudNonEmptyCellCount(const void* pointCloudInfo)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getPointCloudNonEmptyCellCount(pointCloudInfo));
    return(0);
}

bool CPluginContainer::mesh_getOctreeDistanceToPointIfSmaller(const void* octreeInfo,const C4X4Matrix& octreePCTM,const C3Vector& absPoint,float ray[7],long long int& cacheValue)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getOctreeDistanceToPointIfSmaller(octreeInfo,_octreePCTM,absPoint.data,ray,&cacheValue)!=0);
    return(false);
}

bool CPluginContainer::mesh_getOctreeCellFromCache(const void* octreeInfo,const C4X4Matrix& octreePCTM,const long long int cacheValue,float& cellSize,C4X4Matrix& cellRetToThisM)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _retM[12];

    if (currentMeshEngine!=nullptr)
    {
        bool retVal=currentMeshEngine->v_repMesh_getOctreeCellFromCache(octreeInfo,_octreePCTM,cacheValue,&cellSize,_retM);
        if (retVal)
            cellRetToThisM.copyFromInterface(_retM);
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getOctreeDistanceToOctreeIfSmaller(const void* octreeInfo1,const void* octreeInfo2,const C4X4Matrix& octree1PCTM,const C4X4Matrix& octree2PCTM,float ray[7],long long int& octree1CacheValue,long long int& octree2CacheValue,bool weHaveSomeCoherency)
{
    float _octree1PCTM[12];
    octree1PCTM.copyToInterface(_octree1PCTM);

    float _octree2PCTM[12];
    octree2PCTM.copyToInterface(_octree2PCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getOctreeDistanceToOctreeIfSmaller(octreeInfo1,octreeInfo2,_octree1PCTM,_octree2PCTM,ray,&octree1CacheValue,&octree2CacheValue,weHaveSomeCoherency)!=0);
    return(false);
}

bool CPluginContainer::mesh_getOctreeDistanceToPointCloudIfSmaller(const void* octreeInfo,const void* pointCloudInfo,const C4X4Matrix& octreePCTM,const C4X4Matrix& pointCloudPCTM,float ray[7],long long int& octreeCacheValue,long long int& pointCloudCacheValue)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getOctreeDistanceToPointCloudIfSmaller(octreeInfo,pointCloudInfo,_octreePCTM,_pointCloudPCTM,ray,&octreeCacheValue,&pointCloudCacheValue)!=0);
    return(false);
}

bool CPluginContainer::mesh_getOctreeDistanceToShapeIfSmaller(const void* octreeInfo,const void* collInfo,const C4X4Matrix& octreePCTM,const C4X4Matrix& collNodePCTM,float ray[7],long long int& octreeCacheValue,int& collNodeCacheValue)
{
    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    float _collNodePCTM[12];
    collNodePCTM.copyToInterface(_collNodePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getOctreeDistanceToShapeIfSmaller(octreeInfo,collInfo,_octreePCTM,_collNodePCTM,ray,&octreeCacheValue,&collNodeCacheValue)!=0);
    return(false);
}

bool CPluginContainer::mesh_getMinDistBetweenCubeAndTriangleIfSmaller(float cubeSize,const C3Vector& b1,const C3Vector& b1e,const C3Vector& b1f,float& dist,C3Vector& distPt1,C3Vector& distPt2)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getMinDistBetweenCubeAndTriangleIfSmaller(cubeSize,b1.data,b1e.data,b1f.data,&dist,distPt1.data,distPt2.data)!=0);
    return(false);
}

bool CPluginContainer::mesh_getPointCloudDistanceToShapeIfSmaller(const void* pointCloudInfo,const void* collInfo,const C4X4Matrix& pointCloudPCTM,const C4X4Matrix& collNodePCTM,float ray[7],long long int& pointCloudCacheValue,int& collNodeCacheValue)
{
    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    float _collNodePCTM[12];
    collNodePCTM.copyToInterface(_collNodePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getPointCloudDistanceToShapeIfSmaller(pointCloudInfo,collInfo,_pointCloudPCTM,_collNodePCTM,ray,&pointCloudCacheValue,&collNodeCacheValue)!=0);
    return(false);
}

bool CPluginContainer::mesh_getMinDistBetweenPointAndTriangleIfSmaller(const C3Vector& point,const C3Vector& b1,const C3Vector& b1e,const C3Vector& b1f,float& dist,C3Vector& distPt1)
{
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getMinDistBetweenPointAndTriangleIfSmaller(point.data,b1.data,b1e.data,b1f.data,&dist,distPt1.data)!=0);
    return(false);
}

float CPluginContainer::mesh_getBoxBoxDistance(const C4X4Matrix& m1,const C3Vector& halfSize1,const C4X4Matrix& m2,const C3Vector& halfSize2)
{
    float _m1[12];
    m1.copyToInterface(_m1);
    float _m2[12];
    m2.copyToInterface(_m2);
    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_getBoxBoxDistance(_m1,halfSize1.data,_m2,halfSize2.data));
    return(0.0);
}

bool CPluginContainer::mesh_getProxSensorPointCloudDistanceIfSmaller(const void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,float &dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,C3Vector& detectPoint,bool fast,void* theOcclusionCheckCallback)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        float _pointCloudPCTM[12];
        pointCloudPCTM.copyToInterface(_pointCloudPCTM);
        return(currentMeshEngine->v_repMesh_getProxSensorPointCloudDistanceIfSmaller(pointCloudInfo,_pointCloudPCTM,&dist,_planes,_planesSize,_planesOutside,_planesOutsideSize,detectPoint.data,fast,theOcclusionCheckCallback)!=0);
    }
    return(false);
}

bool CPluginContainer::mesh_getRayProxSensorOctreeDistanceIfSmaller(const void* octreeInfo,const C4X4Matrix& octreePCTM,float &dist,const C3Vector& lp,const C3Vector& lvFar,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback)
{
    if (currentMeshEngine!=nullptr)
    {
        float _octreePCTM[12];
        octreePCTM.copyToInterface(_octreePCTM);
        float _detectPoint[3];
        float _triNormalNotNormalized[3];
        bool retVal=currentMeshEngine->v_repMesh_getRayProxSensorOctreeDistanceIfSmaller(octreeInfo,_octreePCTM,&dist,lp.data,lvFar.data,cosAngle,_detectPoint,fast,frontFace,backFace,_triNormalNotNormalized,theOcclusionCheckCallback)!=0;
        if (retVal)
        {
            detectPoint.set(_detectPoint);
            triNormalNotNormalized.set(_triNormalNotNormalized);
        }
        return(retVal);
    }
    return(false);
}

bool CPluginContainer::mesh_getProxSensorOctreeDistanceIfSmaller(const void* octreeInfo,const C4X4Matrix& octreeRTM,float& dist,const std::vector<float>* planes,const std::vector<float>* planesOutside,float cosAngle,C3Vector& detectPoint,bool fast,bool frontFace,bool backFace,C3Vector& triNormalNotNormalized,void* theOcclusionCheckCallback)
{
    if (currentMeshEngine!=nullptr)
    {
        const float* _planes=nullptr;
        int _planesSize=0;
        if ((planes!=nullptr)&&(planes->size()!=0))
        {
            _planes=&(*planes)[0];
            _planesSize=(int)planes->size();
        }
        const float* _planesOutside=nullptr;
        int _planesOutsideSize=0;
        if ((planesOutside!=nullptr)&&(planesOutside->size()!=0))
        {
            _planesOutside=&(*planesOutside)[0];
            _planesOutsideSize=(int)planesOutside->size();
        }
        float _octreeRTM[12];
        octreeRTM.copyToInterface(_octreeRTM);
        return(currentMeshEngine->v_repMesh_getProxSensorOctreeDistanceIfSmaller(octreeInfo,_octreeRTM,&dist,_planes,_planesSize,_planesOutside,_planesOutsideSize,cosAngle,detectPoint.data,fast,frontFace,backFace,triNormalNotNormalized.data,theOcclusionCheckCallback)!=0);
    }
    return(false);
}

bool CPluginContainer::mesh_removePointCloudPointsFromOctree(void* pointCloudInfo,const C4X4Matrix& pointCloudPCTM,const void* octreeInfo,const C4X4Matrix& octreePCTM,int& removedCnt)
{
    float _pointCloudPCTM[12];
    pointCloudPCTM.copyToInterface(_pointCloudPCTM);

    float _octreePCTM[12];
    octreePCTM.copyToInterface(_octreePCTM);

    if (currentMeshEngine!=nullptr)
        return(currentMeshEngine->v_repMesh_removePointCloudPointsFromOctree(pointCloudInfo,_pointCloudPCTM,octreeInfo,_octreePCTM,&removedCnt)!=0);
    return(false);
}

bool CPluginContainer::codeEditor_openModal(const char* initText,const char* properties,std::string& modifiedText,int* positionAndSize)
{
    bool retVal=false;
    if (currentCodeEditor!=nullptr)
    {
        char* buffer=currentCodeEditor->_codeEditor_openModal(initText,properties,positionAndSize);
        if (buffer!=nullptr)
        {
            modifiedText=buffer;
            delete[] buffer;
            retVal=true;
        }
    }
    return(retVal);
}

int CPluginContainer::codeEditor_open(const char* initText,const char* properties)
{
    int retVal=-1;
    if (currentCodeEditor!=nullptr)
        retVal=currentCodeEditor->_codeEditor_open(initText,properties);
    return(retVal);
}

int CPluginContainer::codeEditor_setText(int handle,const char* text,int insertMode)
{
    int retVal=-1;
    if (currentCodeEditor!=nullptr)
        retVal=currentCodeEditor->_codeEditor_setText(handle,text,insertMode);
    return(retVal);
}

bool CPluginContainer::codeEditor_getText(int handle,std::string& text,int* positionAndSize)
{
    bool retVal=false;
    if (currentCodeEditor!=nullptr)
    {
        char* buffer=currentCodeEditor->_codeEditor_getText(handle,positionAndSize);
        if (buffer!=nullptr)
        {
            text=buffer;
            delete[] buffer;
            retVal=true;
        }
    }
    return(retVal);
}

int CPluginContainer::codeEditor_show(int handle,int showState)
{
    int retVal=-1;
    if (currentCodeEditor!=nullptr)
        retVal=currentCodeEditor->_codeEditor_show(handle,showState);
    return(retVal);
}

int CPluginContainer::codeEditor_close(int handle,int* positionAndSize)
{
    int retVal=-1;
    if (currentCodeEditor!=nullptr)
        retVal=currentCodeEditor->_codeEditor_close(handle,positionAndSize);
    return(retVal);
}

int CPluginContainer::customUi_msgBox(int type, int buttons, const char *title, const char *message)
{
    int retVal=-1;
    if (currentCustomUi!=nullptr)
        retVal=currentCustomUi->_customUi_msgBox(type,buttons,title,message);
    return(retVal);
}

bool CPluginContainer::customUi_fileDialog(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext, int native,std::string& files)
{
    bool retVal=false;
    if (currentCustomUi!=nullptr)
    {
        char* res=currentCustomUi->_customUi_fileDialog(type,title,startPath,initName,extName,ext,native);
        if (res!=nullptr)
        {
            files.assign(res);
            retVal=true;
            simReleaseBuffer_internal(res);
        }
    }
    return(retVal);
}

int* CPluginContainer::assimp_importShapes(const char* fileNames,int maxTextures,float scaling,int upVector,int options,int* shapeCount)
{
    int* retVal=nullptr;
    if (currentAssimp!=nullptr)
        retVal=currentAssimp->_assimp_importShapes(fileNames,maxTextures,scaling,upVector,options,shapeCount);
    return(retVal);
}

void CPluginContainer::assimp_exportShapes(const int* shapeHandles,int shapeCount,const char* filename,const char* format,float scaling,int upVector,int options)
{
    if (currentAssimp!=nullptr)
        currentAssimp->_assimp_exportShapes(shapeHandles,shapeCount,filename,format,scaling,upVector,options);
}

int CPluginContainer::assimp_importMeshes(const char* fileNames,float scaling,int upVector,int options,float*** allVertices,int** verticesSizes,int*** allIndices,int** indicesSizes)
{
    int retVal=0;
    if (currentAssimp!=nullptr)
        retVal=currentAssimp->_assimp_importMeshes(fileNames,scaling,upVector,options,allVertices,verticesSizes,allIndices,indicesSizes);
    return(retVal);
}

void CPluginContainer::assimp_exportMeshes(int meshCnt,const float** allVertices,const int* verticesSizes,const int** allIndices,const int* indicesSizes,const char* filename,const char* format,float scaling,int upVector,int options)
{
    if (currentAssimp!=nullptr)
        currentAssimp->_assimp_exportMeshes(meshCnt,allVertices,verticesSizes,allIndices,indicesSizes,filename,format,scaling,upVector,options);
}
