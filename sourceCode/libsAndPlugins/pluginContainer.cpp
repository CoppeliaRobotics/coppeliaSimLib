#include "app.h"
#include "pluginContainer.h"
#include "simConst.h"
#include "pathPlanningInterface.h"
#include "easyLock.h"
#include "simInternal.h"
#include "ttUtil.h"
#include "apiErrors.h"
#include "collisionRoutines.h"
#include <algorithm>

CPlugin::CPlugin(const char* filename,const char* pluginName)
{
    _filename=filename;
    _name=pluginName;
    _loadCount=1;
    extendedVersionInt=-1;
    _consoleVerbosity=sim_verbosity_useglobal;
    _statusbarVerbosity=sim_verbosity_useglobal;

    // Following used to detect appartenance:
    instance=nullptr;
    geomPlugin_createMesh=nullptr;
    ikPlugin_createEnv=nullptr;
    _codeEditor_openModal=nullptr;
    _customUi_msgBox=nullptr;
    _assimp_importShapes=nullptr;
    pythonPlugin_initState=nullptr;
    ruckigPlugin_pos=nullptr;
}

CPlugin::~CPlugin()
{
    if (instance!=nullptr)
        VVarious::closeLibrary(instance);
    if (geomPlugin_createMesh!=nullptr) // also check constructor above
        CPluginContainer::currentGeomPlugin=nullptr;
    if (ikPlugin_createEnv!=nullptr) // also check constructor above
    {
        CPluginContainer::currentIkPlugin=nullptr;
        CPluginContainer::ikEnvironment=-1;
    }
    if (pythonPlugin_initState!=nullptr) // also check constructor above
        CPluginContainer::currentPythonPlugin=nullptr;
    if (ruckigPlugin_pos!=nullptr) // also check constructor above
        CPluginContainer::currentRuckigPlugin=nullptr;
    if (_codeEditor_openModal!=nullptr) // also check constructor above
        CPluginContainer::currentCodeEditor=nullptr;
    if (_customUi_msgBox!=nullptr) // also check constructor above
        CPluginContainer::currentCustomUi=nullptr;
    if (_assimp_importShapes!=nullptr) // also check constructor above
        CPluginContainer::currentAssimp=nullptr;
}

void CPlugin::setConsoleVerbosity(int level)
{
    _consoleVerbosity=level;
}

int CPlugin::getConsoleVerbosity() const
{
    return(_consoleVerbosity);
}

void CPlugin::setStatusbarVerbosity(int level)
{
    _statusbarVerbosity=level;
}

int CPlugin::getStatusbarVerbosity() const
{
    return(_statusbarVerbosity);
}

std::string CPlugin::getName() const
{
    return(_name);
}

int CPlugin::load()
{
    WLibrary lib=VVarious::openLibrary(_filename.c_str()); // here we have the extension in the filename (.dll, .so or .dylib)
    if (lib!=nullptr)
    {
        instance=lib;
        startAddress=(ptrStart)(VVarious::resolveLibraryFuncName(lib,"simStart"));
        if (startAddress==nullptr)
            startAddress=(ptrStart)(VVarious::resolveLibraryFuncName(lib,"v_repStart")); // for backward compatibility

        endAddress=(ptrEnd)(VVarious::resolveLibraryFuncName(lib,"simEnd"));
        if (endAddress==nullptr)
            endAddress=(ptrEnd)(VVarious::resolveLibraryFuncName(lib,"v_repEnd")); // for backward compatibility
        messageAddress=(ptrMessage)(VVarious::resolveLibraryFuncName(lib,"simMessage"));
        if (messageAddress==nullptr)
            messageAddress=(ptrMessage)(VVarious::resolveLibraryFuncName(lib,"v_repMessage")); // for backward compatibility
        if ( (startAddress!=nullptr)&&(endAddress!=nullptr)&&(messageAddress!=nullptr) )
        {
            pluginVersion=startAddress(nullptr,0);

            ptrExtRenderer pov=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"simPovRay"));
            if (pov==nullptr)
                pov=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repPovRay")); // for backward compatibility
            if (pov!=nullptr)
                CPluginContainer::_povRayAddress=pov;

            ptrExtRenderer extRenderer=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"simExtRenderer"));
            if (extRenderer==nullptr)
                extRenderer=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repExtRenderer")); // for backward compatibility
            if (extRenderer!=nullptr)
                CPluginContainer::_extRendererAddress=extRenderer;

            ptrExtRenderer extRendererWindowed=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"simExtRendererWindowed"));
            if (extRenderer==nullptr)
                extRenderer=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repExtRendererWindowed")); // for backward compatibility
            if (extRendererWindowed!=nullptr)
                CPluginContainer::_extRendererWindowedAddress=extRendererWindowed;

            ptrExtRenderer openGl3=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"simOpenGL3Renderer"));
            if (openGl3==nullptr)
                openGl3=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repOpenGL3Renderer")); // for backward compatibility
            if (openGl3!=nullptr)
                CPluginContainer::_openGl3Address=openGl3;

            ptrExtRenderer openGl3Windowed=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"simOpenGL3RendererWindowed"));
            if (openGl3Windowed==nullptr)
                openGl3Windowed=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(lib,"v_repOpenGL3RendererWindowed")); // for backward compatibility
            if (openGl3Windowed!=nullptr)
                CPluginContainer::_openGl3WindowedAddress=openGl3Windowed;

            ptrQhull qhull=(ptrQhull)(VVarious::resolveLibraryFuncName(lib,"simQhull"));
            if (qhull==nullptr)
                qhull=(ptrQhull)(VVarious::resolveLibraryFuncName(lib,"v_repQhull")); // for backward compatibility
            if (qhull!=nullptr)
                CPluginContainer::_qhullAddress=qhull;

            ptrHACD hacd=(ptrHACD)(VVarious::resolveLibraryFuncName(lib,"simHACD"));
            if (hacd==nullptr)
                hacd=(ptrHACD)(VVarious::resolveLibraryFuncName(lib,"v_repHACD")); // for backward compatibility
            if (hacd!=nullptr)
                CPluginContainer::_hacdAddress=hacd;

            ptrVHACD vhacd=(ptrVHACD)(VVarious::resolveLibraryFuncName(lib,"simVHACD"));
            if (vhacd==nullptr)
                vhacd=(ptrVHACD)(VVarious::resolveLibraryFuncName(lib,"v_repVHACD")); // for backward compatibility
            if (vhacd!=nullptr)
                CPluginContainer::_vhacdAddress=vhacd;

            ptrMeshDecimator meshDecimator=(ptrMeshDecimator)(VVarious::resolveLibraryFuncName(lib,"simDecimateMesh"));
            if (meshDecimator==nullptr)
                meshDecimator=(ptrMeshDecimator)(VVarious::resolveLibraryFuncName(lib,"v_repDecimateMesh")); // for backward compatibility
            if (meshDecimator!=nullptr)
                CPluginContainer::_meshDecimatorAddress=meshDecimator;

            if (pluginVersion!=0)
            {
                syncPlugin_msg=(ptr_syncPlugin_msg)(VVarious::resolveLibraryFuncName(lib,"sync_msg"));

                // For the dynamic plugins:
                dynPlugin_startSimulation=(ptr_dynPlugin_startSimulation)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_startSimulation"));
                dynPlugin_endSimulation=(ptr_dynPlugin_endSimulation)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_endSimulation"));
                dynPlugin_step=(ptr_dynPlugin_step)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_step"));
                dynPlugin_isDynamicContentAvailable=(ptr_dynPlugin_isDynamicContentAvailable)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_isDynamicContentAvailable"));
                dynPlugin_serializeDynamicContent=(ptr_dynPlugin_serializeDynamicContent)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_serializeDynamicContent"));
                dynPlugin_addParticleObject=(ptr_dynPlugin_addParticleObject)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_addParticleObject"));
                dynPlugin_removeParticleObject=(ptr_dynPlugin_removeParticleObject)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_removeParticleObject"));
                dynPlugin_addParticleObjectItem=(ptr_dynPlugin_addParticleObjectItem)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_addParticleObjectItem"));
                dynPlugin_getParticleObjectOtherFloatsPerItem=(ptr_dynPlugin_getParticleObjectOtherFloatsPerItem)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getParticleObjectOtherFloatsPerItem"));
                dynPlugin_getContactPoints=(ptr_dynPlugin_getContactPoints)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getContactPoints"));
                dynPlugin_getParticles=(ptr_dynPlugin_getParticles)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getParticles"));
                dynPlugin_getParticleData=(ptr_dynPlugin_getParticleData)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getParticleData"));
                dynPlugin_getContactForce=(ptr_dynPlugin_getContactForce)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getContactForce"));
                dynPlugin_reportDynamicWorldConfiguration=(ptr_dynPlugin_reportDynamicWorldConfiguration)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_reportDynamicWorldConfiguration"));
                dynPlugin_getDynamicStepDivider=(ptr_dynPlugin_getDynamicStepDivider)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getDynamicStepDivider"));
                dynPlugin_getEngineInfo=(ptr_dynPlugin_getEngineInfo)(VVarious::resolveLibraryFuncName(lib,"dynPlugin_getEngineInfo"));

                // For the geom plugin:
                geomPlugin_releaseBuffer=(ptr_geomPlugin_releaseBuffer)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_releaseBuffer"));
                geomPlugin_createMesh=(ptr_geomPlugin_createMesh)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createMesh"));
                geomPlugin_copyMesh=(ptr_geomPlugin_copyMesh)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_copyMesh"));
                geomPlugin_getMeshFromSerializationData=(ptr_geomPlugin_getMeshFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshFromSerializationData"));
                geomPlugin_getMeshSerializationData=(ptr_geomPlugin_getMeshSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshSerializationData"));
                geomPlugin_scaleMesh=(ptr_geomPlugin_scaleMesh)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_scaleMesh"));
                geomPlugin_destroyMesh=(ptr_geomPlugin_destroyMesh)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_destroyMesh"));
                geomPlugin_getMeshRootObbVolume=(ptr_geomPlugin_getMeshRootObbVolume)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshRootObbVolume"));
                geomPlugin_createOctreeFromPoints=(ptr_geomPlugin_createOctreeFromPoints)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createOctreeFromPoints"));
                geomPlugin_createOctreeFromColorPoints=(ptr_geomPlugin_createOctreeFromColorPoints)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createOctreeFromColorPoints"));
                geomPlugin_createOctreeFromMesh=(ptr_geomPlugin_createOctreeFromMesh)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createOctreeFromMesh"));
                geomPlugin_createOctreeFromOctree=(ptr_geomPlugin_createOctreeFromOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createOctreeFromOctree"));
                geomPlugin_copyOctree=(ptr_geomPlugin_copyOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_copyOctree"));
                geomPlugin_getOctreeFromSerializationData=(ptr_geomPlugin_getOctreeFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeFromSerializationData"));
                geomPlugin_getOctreeSerializationData=(ptr_geomPlugin_getOctreeSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeSerializationData"));
                geomPlugin_scaleOctree=(ptr_geomPlugin_scaleOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_scaleOctree"));
                geomPlugin_destroyOctree=(ptr_geomPlugin_destroyOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_destroyOctree"));
                geomPlugin_getOctreeVoxelData=(ptr_geomPlugin_getOctreeVoxelData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeVoxelData"));
                geomPlugin_getOctreeUserData=(ptr_geomPlugin_getOctreeUserData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeUserData"));
                geomPlugin_getOctreeCornersFromOctree=(ptr_geomPlugin_getOctreeCornersFromOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeCornersFromOctree"));
                geomPlugin_insertPointsIntoOctree=(ptr_geomPlugin_insertPointsIntoOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertPointsIntoOctree"));
                geomPlugin_insertColorPointsIntoOctree=(ptr_geomPlugin_insertColorPointsIntoOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertColorPointsIntoOctree"));
                geomPlugin_insertMeshIntoOctree=(ptr_geomPlugin_insertMeshIntoOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertMeshIntoOctree"));
                geomPlugin_insertOctreeIntoOctree=(ptr_geomPlugin_insertOctreeIntoOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertOctreeIntoOctree"));
                geomPlugin_removePointsFromOctree=(ptr_geomPlugin_removePointsFromOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_removePointsFromOctree"));
                geomPlugin_removeMeshFromOctree=(ptr_geomPlugin_removeMeshFromOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_removeMeshFromOctree"));
                geomPlugin_removeOctreeFromOctree=(ptr_geomPlugin_removeOctreeFromOctree)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_removeOctreeFromOctree"));
                geomPlugin_createPtcloudFromPoints=(ptr_geomPlugin_createPtcloudFromPoints)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createPtcloudFromPoints"));
                geomPlugin_createPtcloudFromColorPoints=(ptr_geomPlugin_createPtcloudFromColorPoints)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_createPtcloudFromColorPoints"));
                geomPlugin_copyPtcloud=(ptr_geomPlugin_copyPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_copyPtcloud"));
                geomPlugin_getPtcloudFromSerializationData=(ptr_geomPlugin_getPtcloudFromSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudFromSerializationData"));
                geomPlugin_getPtcloudSerializationData=(ptr_geomPlugin_getPtcloudSerializationData)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudSerializationData"));
                geomPlugin_scalePtcloud=(ptr_geomPlugin_scalePtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_scalePtcloud"));
                geomPlugin_destroyPtcloud=(ptr_geomPlugin_destroyPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_destroyPtcloud"));
                geomPlugin_getPtcloudPoints=(ptr_geomPlugin_getPtcloudPoints)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudPoints"));
                geomPlugin_getPtcloudOctreeCorners=(ptr_geomPlugin_getPtcloudOctreeCorners)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudOctreeCorners"));
                geomPlugin_getPtcloudNonEmptyCellCount=(ptr_geomPlugin_getPtcloudNonEmptyCellCount)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudNonEmptyCellCount"));
                geomPlugin_insertPointsIntoPtcloud=(ptr_geomPlugin_insertPointsIntoPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertPointsIntoPtcloud"));
                geomPlugin_insertColorPointsIntoPtcloud=(ptr_geomPlugin_insertColorPointsIntoPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_insertColorPointsIntoPtcloud"));
                geomPlugin_removePointsFromPtcloud=(ptr_geomPlugin_removePointsFromPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_removePointsFromPtcloud"));
                geomPlugin_removeOctreeFromPtcloud=(ptr_geomPlugin_removeOctreeFromPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_removeOctreeFromPtcloud"));
                geomPlugin_intersectPointsWithPtcloud=(ptr_geomPlugin_intersectPointsWithPtcloud)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_intersectPointsWithPtcloud"));
                geomPlugin_getMeshMeshCollision=(ptr_geomPlugin_getMeshMeshCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshMeshCollision"));
                geomPlugin_getMeshOctreeCollision=(ptr_geomPlugin_getMeshOctreeCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshOctreeCollision"));
                geomPlugin_getMeshTriangleCollision=(ptr_geomPlugin_getMeshTriangleCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshTriangleCollision"));
                geomPlugin_getMeshSegmentCollision=(ptr_geomPlugin_getMeshSegmentCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshSegmentCollision"));
                geomPlugin_getOctreeOctreeCollision=(ptr_geomPlugin_getOctreeOctreeCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeOctreeCollision"));
                geomPlugin_getOctreePtcloudCollision=(ptr_geomPlugin_getOctreePtcloudCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreePtcloudCollision"));
                geomPlugin_getOctreeTriangleCollision=(ptr_geomPlugin_getOctreeTriangleCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeTriangleCollision"));
                geomPlugin_getOctreeSegmentCollision=(ptr_geomPlugin_getOctreeSegmentCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeSegmentCollision"));
                geomPlugin_getOctreePointsCollision=(ptr_geomPlugin_getOctreePointsCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreePointsCollision"));
                geomPlugin_getOctreePointCollision=(ptr_geomPlugin_getOctreePointCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreePointCollision"));
                geomPlugin_getBoxBoxCollision=(ptr_geomPlugin_getBoxBoxCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxBoxCollision"));
                geomPlugin_getBoxTriangleCollision=(ptr_geomPlugin_getBoxTriangleCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxTriangleCollision"));
                geomPlugin_getBoxSegmentCollision=(ptr_geomPlugin_getBoxSegmentCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxSegmentCollision"));
                geomPlugin_getBoxPointCollision=(ptr_geomPlugin_getBoxPointCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxPointCollision"));
                geomPlugin_getTriangleTriangleCollision=(ptr_geomPlugin_getTriangleTriangleCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getTriangleTriangleCollision"));
                geomPlugin_getTriangleSegmentCollision=(ptr_geomPlugin_getTriangleSegmentCollision)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getTriangleSegmentCollision"));
                geomPlugin_getMeshMeshDistanceIfSmaller=(ptr_geomPlugin_getMeshMeshDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshMeshDistanceIfSmaller"));
                geomPlugin_getMeshOctreeDistanceIfSmaller=(ptr_geomPlugin_getMeshOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshOctreeDistanceIfSmaller"));
                geomPlugin_getMeshPtcloudDistanceIfSmaller=(ptr_geomPlugin_getMeshPtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshPtcloudDistanceIfSmaller"));
                geomPlugin_getMeshTriangleDistanceIfSmaller=(ptr_geomPlugin_getMeshTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshTriangleDistanceIfSmaller"));
                geomPlugin_getMeshSegmentDistanceIfSmaller=(ptr_geomPlugin_getMeshSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshSegmentDistanceIfSmaller"));
                geomPlugin_getMeshPointDistanceIfSmaller=(ptr_geomPlugin_getMeshPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getMeshPointDistanceIfSmaller"));
                geomPlugin_getOctreeOctreeDistanceIfSmaller=(ptr_geomPlugin_getOctreeOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeOctreeDistanceIfSmaller"));
                geomPlugin_getOctreePtcloudDistanceIfSmaller=(ptr_geomPlugin_getOctreePtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreePtcloudDistanceIfSmaller"));
                geomPlugin_getOctreeTriangleDistanceIfSmaller=(ptr_geomPlugin_getOctreeTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeTriangleDistanceIfSmaller"));
                geomPlugin_getOctreeSegmentDistanceIfSmaller=(ptr_geomPlugin_getOctreeSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreeSegmentDistanceIfSmaller"));
                geomPlugin_getOctreePointDistanceIfSmaller=(ptr_geomPlugin_getOctreePointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getOctreePointDistanceIfSmaller"));
                geomPlugin_getPtcloudPtcloudDistanceIfSmaller=(ptr_geomPlugin_getPtcloudPtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudPtcloudDistanceIfSmaller"));
                geomPlugin_getPtcloudTriangleDistanceIfSmaller=(ptr_geomPlugin_getPtcloudTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudTriangleDistanceIfSmaller"));
                geomPlugin_getPtcloudSegmentDistanceIfSmaller=(ptr_geomPlugin_getPtcloudSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudSegmentDistanceIfSmaller"));
                geomPlugin_getPtcloudPointDistanceIfSmaller=(ptr_geomPlugin_getPtcloudPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getPtcloudPointDistanceIfSmaller"));
                geomPlugin_getApproxBoxBoxDistance=(ptr_geomPlugin_getApproxBoxBoxDistance)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getApproxBoxBoxDistance"));
                geomPlugin_getBoxBoxDistanceIfSmaller=(ptr_geomPlugin_getBoxBoxDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxBoxDistanceIfSmaller"));
                geomPlugin_getBoxTriangleDistanceIfSmaller=(ptr_geomPlugin_getBoxTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxTriangleDistanceIfSmaller"));
                geomPlugin_getBoxSegmentDistanceIfSmaller=(ptr_geomPlugin_getBoxSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxSegmentDistanceIfSmaller"));
                geomPlugin_getBoxPointDistanceIfSmaller=(ptr_geomPlugin_getBoxPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getBoxPointDistanceIfSmaller"));
                geomPlugin_getTriangleTriangleDistanceIfSmaller=(ptr_geomPlugin_getTriangleTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getTriangleTriangleDistanceIfSmaller"));
                geomPlugin_getTriangleSegmentDistanceIfSmaller=(ptr_geomPlugin_getTriangleSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getTriangleSegmentDistanceIfSmaller"));
                geomPlugin_getTrianglePointDistanceIfSmaller=(ptr_geomPlugin_getTrianglePointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getTrianglePointDistanceIfSmaller"));
                geomPlugin_getSegmentSegmentDistanceIfSmaller=(ptr_geomPlugin_getSegmentSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getSegmentSegmentDistanceIfSmaller"));
                geomPlugin_getSegmentPointDistanceIfSmaller=(ptr_geomPlugin_getSegmentPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_getSegmentPointDistanceIfSmaller"));
                geomPlugin_volumeSensorDetectMeshIfSmaller=(ptr_geomPlugin_volumeSensorDetectMeshIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_volumeSensorDetectMeshIfSmaller"));
                geomPlugin_volumeSensorDetectOctreeIfSmaller=(ptr_geomPlugin_volumeSensorDetectOctreeIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_volumeSensorDetectOctreeIfSmaller"));
                geomPlugin_volumeSensorDetectPtcloudIfSmaller=(ptr_geomPlugin_volumeSensorDetectPtcloudIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_volumeSensorDetectPtcloudIfSmaller"));
                geomPlugin_volumeSensorDetectTriangleIfSmaller=(ptr_geomPlugin_volumeSensorDetectTriangleIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_volumeSensorDetectTriangleIfSmaller"));
                geomPlugin_volumeSensorDetectSegmentIfSmaller=(ptr_geomPlugin_volumeSensorDetectSegmentIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_volumeSensorDetectSegmentIfSmaller"));
                geomPlugin_raySensorDetectMeshIfSmaller=(ptr_geomPlugin_raySensorDetectMeshIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_raySensorDetectMeshIfSmaller"));
                geomPlugin_raySensorDetectOctreeIfSmaller=(ptr_geomPlugin_raySensorDetectOctreeIfSmaller)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_raySensorDetectOctreeIfSmaller"));
                geomPlugin_isPointInVolume=(ptr_geomPlugin_isPointInVolume)(VVarious::resolveLibraryFuncName(lib,"geomPlugin_isPointInVolume"));
                if (geomPlugin_createMesh!=nullptr)
                    CPluginContainer::currentGeomPlugin=this;

                // For the IK plugin:
                ikPlugin_createEnv=(ptr_ikPlugin_createEnv)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_createEnv"));
//                ikPlugin_switchEnvironment=(ptr_ikPlugin_switchEnvironment)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_switchEnvironment"));
                ikPlugin_eraseEnvironment=(ptr_ikPlugin_eraseEnvironment)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_eraseEnvironment"));
                ikPlugin_eraseObject=(ptr_ikPlugin_eraseObject)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_eraseObject"));
                ikPlugin_setObjectParent=(ptr_ikPlugin_setObjectParent)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setObjectParent"));
                ikPlugin_createDummy=(ptr_ikPlugin_createDummy)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_createDummy"));
                ikPlugin_setLinkedDummy=(ptr_ikPlugin_setLinkedDummy)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setLinkedDummy"));
                ikPlugin_createJoint=(ptr_ikPlugin_createJoint)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_createJoint"));
                ikPlugin_setJointMode=(ptr_ikPlugin_setJointMode)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointMode"));
                ikPlugin_setJointInterval=(ptr_ikPlugin_setJointInterval)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointInterval"));
                ikPlugin_setJointScrewPitch=(ptr_ikPlugin_setJointScrewPitch)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointScrewPitch"));
                ikPlugin_setJointIkWeight=(ptr_ikPlugin_setJointIkWeight)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointIkWeight"));
                ikPlugin_setJointMaxStepSize=(ptr_ikPlugin_setJointMaxStepSize)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointMaxStepSize"));
                ikPlugin_setJointDependency=(ptr_ikPlugin_setJointDependency)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointDependency"));
                ikPlugin_getJointPosition=(ptr_ikPlugin_getJointPosition)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getJointPosition"));
                ikPlugin_setJointPosition=(ptr_ikPlugin_setJointPosition)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setJointPosition"));
                ikPlugin_getSphericalJointQuaternion=(ptr_ikPlugin_getSphericalJointQuaternion)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getSphericalJointQuaternion"));
                ikPlugin_setSphericalJointQuaternion=(ptr_ikPlugin_setSphericalJointQuaternion)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setSphericalJointQuaternion"));
                ikPlugin_createIkGroup=(ptr_ikPlugin_createIkGroup)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_createIkGroup"));
                ikPlugin_eraseIkGroup=(ptr_ikPlugin_eraseIkGroup)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_eraseIkGroup"));
                ikPlugin_setIkGroupFlags=(ptr_ikPlugin_setIkGroupFlags)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkGroupFlags"));
                ikPlugin_setIkGroupCalculation=(ptr_ikPlugin_setIkGroupCalculation)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkGroupCalculation"));
                ikPlugin_addIkElement=(ptr_ikPlugin_addIkElement)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_addIkElement"));
                ikPlugin_eraseIkElement=(ptr_ikPlugin_eraseIkElement)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_eraseIkElement"));
                ikPlugin_setIkElementFlags=(ptr_ikPlugin_setIkElementFlags)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkElementFlags"));
                ikPlugin_setIkElementBase=(ptr_ikPlugin_setIkElementBase)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkElementBase"));
                ikPlugin_setIkElementConstraints=(ptr_ikPlugin_setIkElementConstraints)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkElementConstraints"));
                ikPlugin_setIkElementPrecision=(ptr_ikPlugin_setIkElementPrecision)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkElementPrecision"));
                ikPlugin_setIkElementWeights=(ptr_ikPlugin_setIkElementWeights)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setIkElementWeights"));
                ikPlugin_handleIkGroup=(ptr_ikPlugin_handleIkGroup)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_handleIkGroup"));
                ikPlugin_computeJacobian=(ptr_ikPlugin_computeJacobian)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_computeJacobian"));
                ikPlugin_getJacobian=(ptr_ikPlugin_getJacobian)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getJacobian"));
                ikPlugin_getManipulability=(ptr_ikPlugin_getManipulability)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getManipulability"));
                ikPlugin_getConfigForTipPose=(ptr_ikPlugin_getConfigForTipPose)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getConfigForTipPose"));
                ikPlugin_getObjectLocalTransformation=(ptr_ikPlugin_getObjectLocalTransformation)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_getObjectLocalTransformation"));
                ikPlugin_setObjectLocalTransformation=(ptr_ikPlugin_setObjectLocalTransformation)(VVarious::resolveLibraryFuncName(lib,"ikPlugin_setObjectLocalTransformation"));
                if (ikPlugin_createEnv!=nullptr)
                {
                    CPluginContainer::currentIkPlugin=this;
                    CPluginContainer::ikEnvironment=CPluginContainer::currentIkPlugin->ikPlugin_createEnv();
                }

                // For the python plugin:
                pythonPlugin_initState=(ptrPythonPlugin_initState)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_initState"));
                pythonPlugin_cleanupState=(ptrPythonPlugin_cleanupState)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_cleanupState"));
                pythonPlugin_loadCode=(ptrPythonPlugin_loadCode)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_loadCode"));
                pythonPlugin_callFunc=(ptrPythonPlugin_callFunc)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_callFunc"));
                pythonPlugin_execStr=(ptrPythonPlugin_execStr)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_execStr"));
                pythonPlugin_isDeprecated=(ptrPythonPlugin_isDeprecated)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_isDeprecated"));
                pythonPlugin_getFuncs=(ptrPythonPlugin_getFuncs)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_getFuncs"));
                pythonPlugin_getConsts=(ptrPythonPlugin_getConsts)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_getConsts"));
                pythonPlugin_getCalltip=(ptrPythonPlugin_getCalltip)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_getCalltip"));
                pythonPlugin_getError=(ptrPythonPlugin_getError)(VVarious::resolveLibraryFuncName(lib,"pythonPlugin_getError"));
                if (pythonPlugin_initState!=nullptr)
                    CPluginContainer::currentPythonPlugin=this;

                // For the Ruckig plugin:
                ruckigPlugin_pos=(ptrRuckigPlugin_pos)(VVarious::resolveLibraryFuncName(lib,"ruckigPlugin_pos"));
                ruckigPlugin_vel=(ptrRuckigPlugin_vel)(VVarious::resolveLibraryFuncName(lib,"ruckigPlugin_vel"));
                ruckigPlugin_step=(ptrRuckigPlugin_step)(VVarious::resolveLibraryFuncName(lib,"ruckigPlugin_step"));
                ruckigPlugin_remove=(ptrRuckigPlugin_remove)(VVarious::resolveLibraryFuncName(lib,"ruckigPlugin_remove"));
                ruckigPlugin_dofs=(ptrRuckigPlugin_dofs)(VVarious::resolveLibraryFuncName(lib,"ruckigPlugin_dofs"));
                if (ruckigPlugin_pos!=nullptr)
                    CPluginContainer::currentRuckigPlugin=this;

                // For the code editor:
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
std::vector<CPlugin*> CPluginContainer::_syncPlugins;


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
CPlugin* CPluginContainer::currentGeomPlugin=nullptr;
CPlugin* CPluginContainer::currentIkPlugin=nullptr;
CPlugin* CPluginContainer::currentCodeEditor=nullptr;
CPlugin* CPluginContainer::currentPythonPlugin=nullptr;
CPlugin* CPluginContainer::currentRuckigPlugin=nullptr;
CPlugin* CPluginContainer::currentCustomUi=nullptr;
CPlugin* CPluginContainer::currentAssimp=nullptr;

int CPluginContainer::ikEnvironment=-1;

VMutex _geomMutex;

CPluginContainer::CPluginContainer()
{
}

CPluginContainer::~CPluginContainer()
{
}

int CPluginContainer::addPlugin(const char* filename,const char* pluginName)
{
    TRACE_INTERNAL;
    App::logMsg(sim_verbosity_debug,(std::string("addPlugin: ")+pluginName).c_str());

    CPlugin* plug=getPluginFromName(pluginName,true);
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
    if (plug->syncPlugin_msg!=nullptr)
        _syncPlugins.push_back(plug);
    _nextHandle++;
    return(plug->handle);
}

CPlugin* CPluginContainer::getPluginFromFunc(const char* func)
{
    CPlugin* retVal=nullptr;
    if (strcmp(func,"rml")==0)
    {
        retVal=getPluginFromName("RML4",true);
        if (retVal==nullptr)
            retVal=getPluginFromName("RML2",true);
    }
    else if (strcmp(func,"rml2")==0)
        retVal=getPluginFromName("RML2",true);
    else if (strcmp(func,"rml4")==0)
        retVal=getPluginFromName("RML4",true);
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromName(const char* pluginName,bool caseSensitive)
{
    CPlugin* retVal=nullptr;
    std::string thatPl(pluginName);
    if (!caseSensitive)
        std::transform(thatPl.begin(),thatPl.end(),thatPl.begin(),::tolower);
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        std::string thisPl(_allPlugins[i]->_name);
        if (!caseSensitive)
            std::transform(thisPl.begin(),thisPl.end(),thisPl.begin(),::tolower);
        if (thisPl.compare(thatPl)==0)
        {
            retVal=_allPlugins[i];
            break;
        }
    }
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromIndex(size_t index)
{
    CPlugin* retVal=nullptr;
    if (index<_allPlugins.size())
        retVal=_allPlugins[index];
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromHandle(int handle)
{
    CPlugin* retVal=nullptr;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->handle==handle)
        {
            retVal=_allPlugins[i];
            break;
        }
    }
    return(retVal);
}

void CPluginContainer::_removePlugin(int handle)
{
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->handle==handle)
        {
            delete _allPlugins[i];
            _allPlugins.erase(_allPlugins.begin()+i);
            break;
        }
    }
}

bool CPluginContainer::unloadPlugin(int handle)
{
    TRACE_INTERNAL;
    bool retVal=false;
    CPlugin* it=getPluginFromHandle(handle);
    if (it!=nullptr)
    {
        App::logMsg(sim_verbosity_debug,(std::string("unloadPlugin: ")+it->getName()).c_str());
#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(it->getName().compare("CodeEditor")==0) )
            App::mainWindow->codeEditorContainer->closeAll();
#endif

        if (it->_loadCount==1)
        { // will unload it
            for (size_t j=0;j<_syncPlugins.size();j++)
            {
                if (_syncPlugins[j]==it)
                {
                    _syncPlugins.erase(_syncPlugins.begin()+j);
                    break;
                }
            }
            it->endAddress();
            std::string nm(it->getName());
            _removePlugin(handle);
            App::worldContainer->scriptCustomFuncAndVarContainer->announcePluginWasKilled(nm.c_str());
            retVal=true;
        }
        else
        { // cannot yet unload it... others might still depend on it!
            it->_loadCount--;
            return(false);
        }
    }
    return(retVal);
}

int CPluginContainer::getPluginCount()
{
    return(int(_allPlugins.size()));
}

void* CPluginContainer::sendEventCallbackMessageToOnePlugin(const char* pluginName,int msg,int* auxVals,void* data,int retVals[4])
{
    CPlugin* plug=getPluginFromName(pluginName,true);
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
            CPlugin* plug=getPluginFromName(vect->at(i).c_str(),true);
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

void CPluginContainer::syncMsg(const SSyncMsg* msg,const SSyncRt* rt)
{
    for (size_t i=0;i<_syncPlugins.size();i++)
        _syncPlugins[i]->syncPlugin_msg(msg,rt);
}

bool CPluginContainer::dyn_startSimulation(int engine,int version,const float floatParams[20],const int intParams[20])
{
    bool retVal=false;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->dynPlugin_startSimulation!=nullptr)
        {
            if (_allPlugins[i]->dynPlugin_startSimulation(engine,version,floatParams,intParams)!=0)
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
        currentDynEngine->dynPlugin_endSimulation();
    currentDynEngine=nullptr;
}

void CPluginContainer::dyn_step(float timeStep,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->dynPlugin_step(timeStep,simulationTime);
}

bool CPluginContainer::dyn_isInitialized()
{
    return(currentDynEngine!=nullptr);
}

bool CPluginContainer::dyn_isDynamicContentAvailable()
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_isDynamicContentAvailable()!=0);
    return(false);
}

void CPluginContainer::dyn_serializeDynamicContent(const char* filenameAndPath,int bulletSerializationBuffer)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->dynPlugin_serializeDynamicContent(filenameAndPath,bulletSerializationBuffer);
}

int CPluginContainer::dyn_addParticleObject(int objectType,float size,float massOverVolume,const void* params,float lifeTime,int maxItemCount,const float* ambient,const float* diffuse,const float* specular,const float* emission)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_addParticleObject(objectType,size,massOverVolume,params,lifeTime,maxItemCount,ambient,diffuse,specular,emission));
    return(-1);
}

bool CPluginContainer::dyn_removeParticleObject(int objectHandle)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_removeParticleObject(objectHandle)!=0);
    return(false);
}

bool CPluginContainer::dyn_addParticleObjectItem(int objectHandle,const float* itemData,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_addParticleObjectItem(objectHandle,itemData,simulationTime)!=0);
    return(false);
}

int CPluginContainer::dyn_getParticleObjectOtherFloatsPerItem(int objectHandle)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getParticleObjectOtherFloatsPerItem(objectHandle));
    return(0);
}

float* CPluginContainer::dyn_getContactPoints(int* count)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getContactPoints(count));
    count[0]=0;
    return(nullptr);
}

void** CPluginContainer::dyn_getParticles(int index,int* particlesCount,int* objectType,float** cols)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getParticles(index,particlesCount,objectType,cols));
    return(nullptr);
}

bool CPluginContainer::dyn_getParticleData(const void* particle,float* pos,float* size,int* objectType,float** additionalColor)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getParticleData(particle,pos,size,objectType,additionalColor)!=0);
    return(false);
}

bool CPluginContainer::dyn_getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],float* contactInfo)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo)!=0);
    return(false);
}

void CPluginContainer::dyn_reportDynamicWorldConfiguration(int totalPassesCount,char doNotApplyJointIntrinsicPositions,float simulationTime)
{
    if (currentDynEngine!=nullptr)
        currentDynEngine->dynPlugin_reportDynamicWorldConfiguration(totalPassesCount,doNotApplyJointIntrinsicPositions,simulationTime);
}

int CPluginContainer::dyn_getDynamicStepDivider()
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getDynamicStepDivider());
    return(0);
}

int CPluginContainer::dyn_getEngineInfo(int* engine,int* data1,char* data2,char* data3)
{
    if (currentDynEngine!=nullptr)
        return(currentDynEngine->dynPlugin_getEngineInfo(engine,data1,data2,data3));
    return(-1);
}

bool CPluginContainer::isGeomPluginAvailable()
{
    return(currentGeomPlugin!=nullptr);
}

bool CPluginContainer::isIkPluginAvailable()
{
    return(currentIkPlugin!=nullptr);
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

void CPluginContainer::geomPlugin_lockUnlock(bool lock)
{
    if (lock)
        _geomMutex.lock("CPluginContainer::geomPlugin_lockUnlock()");
    else
        _geomMutex.unlock();
}

void CPluginContainer::geomPlugin_releaseBuffer(void* buffer)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_releaseBuffer(buffer);
}

void* CPluginContainer::geomPlugin_createMesh(const float* vertices,int verticesSize,const int* indices,int indicesSize,const C7Vector* meshOrigin/*=nullptr*/,float triangleEdgeMaxLength/*=0.3f*/,int maxTrianglesInBoundingBox/*=8*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float tr[7];
        float* _tr=nullptr;
        if (meshOrigin!=nullptr)
        {
            meshOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createMesh(vertices,verticesSize,indices,indicesSize,_tr,triangleEdgeMaxLength,maxTrianglesInBoundingBox);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyMesh(const void* meshObbStruct)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_copyMesh(meshObbStruct);
    return(retVal);
}
void* CPluginContainer::geomPlugin_getMeshFromSerializationData(const unsigned char* serializationData)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_getMeshFromSerializationData(serializationData);
    return(retVal);
}
void CPluginContainer::geomPlugin_getMeshSerializationData(const void* meshObbStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        unsigned char* data=currentGeomPlugin->geomPlugin_getMeshSerializationData(meshObbStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_scaleMesh(void* meshObbStruct,float scalingFactor)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_scaleMesh(meshObbStruct,scalingFactor);
}
void CPluginContainer::geomPlugin_destroyMesh(void* meshObbStruct)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_destroyMesh(meshObbStruct);
}
float CPluginContainer::geomPlugin_getMeshRootObbVolume(const void* meshObbStruct)
{
    float retVal=0.0f;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_getMeshRootObbVolume(meshObbStruct);
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromPoints(const float* points,int pointCnt,const C7Vector* octreeOrigin/*=nullptr*/,float cellS/*=0.05f*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float tr[7];
        float* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromPoints(points,pointCnt,_tr,cellS,rgbData,usrData);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromColorPoints(const float* points,int pointCnt,const C7Vector* octreeOrigin/*=nullptr*/,float cellS/*=0.05f*/,const unsigned char* rgbData/*=nullptr*/,const unsigned int* usrData/*=nullptr*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float tr[7];
        float* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromColorPoints(points,pointCnt,_tr,cellS,rgbData,usrData);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromMesh(const void* meshObbStruct,const C7Vector& meshTransformation,const C7Vector* octreeOrigin/*=nullptr*/,float cellS/*=0.05f*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float _meshTr[7];
        meshTransformation.getInternalData(_meshTr);
        float tr[7];
        float* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromMesh(meshObbStruct,_meshTr,_tr,cellS,rgbData,usrData);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromOctree(const void* otherOctreeStruct,const C7Vector& otherOctreeTransformation,const C7Vector* newOctreeOrigin/*=nullptr*/,float newOctreeCellS/*=0.05f*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float _otherOcTr[7];
        otherOctreeTransformation.getInternalData(_otherOcTr);
        float tr[7];
        float* _tr=nullptr;
        if (newOctreeOrigin!=nullptr)
        {
            newOctreeOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromOctree(otherOctreeStruct,_otherOcTr,_tr,newOctreeCellS,rgbData,usrData);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyOctree(const void* ocStruct)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_copyOctree(ocStruct);
    return(retVal);
}
void* CPluginContainer::geomPlugin_getOctreeFromSerializationData(const unsigned char* serializationData)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_getOctreeFromSerializationData(serializationData);
    return(retVal);
}
void CPluginContainer::geomPlugin_getOctreeSerializationData(const void* ocStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        unsigned char* data=currentGeomPlugin->geomPlugin_getOctreeSerializationData(ocStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_scaleOctree(void* ocStruct,float f)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_scaleOctree(ocStruct,f);
}
void CPluginContainer::geomPlugin_destroyOctree(void* ocStruct)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_destroyOctree(ocStruct);
}
void CPluginContainer::geomPlugin_getOctreeVoxelPositions(const void* ocStruct,std::vector<float>& voxelPositions)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        float* data=currentGeomPlugin->geomPlugin_getOctreeVoxelData(ocStruct,&l);
        if (data!=nullptr)
        {
            voxelPositions.resize(3*l);
            for (int i=0;i<l;i++)
            {
                voxelPositions[3*i+0]=data[6*i+0];
                voxelPositions[3*i+1]=data[6*i+1];
                voxelPositions[3*i+2]=data[6*i+2];
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_getOctreeVoxelColors(const void* ocStruct,std::vector<float>& voxelColors)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        float* data=currentGeomPlugin->geomPlugin_getOctreeVoxelData(ocStruct,&l);
        if (data!=nullptr)
        {
            voxelColors.resize(4*l);
            for (int i=0;i<l;i++)
            {
                voxelColors[4*i+0]=data[6*i+3];
                voxelColors[4*i+1]=data[6*i+4];
                voxelColors[4*i+2]=data[6*i+5];
                voxelColors[4*i+3]=0.0f;
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_getOctreeUserData(const void* ocStruct,std::vector<unsigned int>& userData)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        unsigned int* data=currentGeomPlugin->geomPlugin_getOctreeUserData(ocStruct,&l);
        if (data!=nullptr)
        {
            userData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_getOctreeCornersFromOctree(const void* ocStruct,std::vector<float>& points)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        float* data=currentGeomPlugin->geomPlugin_getOctreeCornersFromOctree(ocStruct,&l);
        if (data!=nullptr)
        {
            points.assign(data,data+3*l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_insertPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        currentGeomPlugin->geomPlugin_insertPointsIntoOctree(ocStruct,_tr,points,pointCnt,rgbData,usrData);
    }
}
void CPluginContainer::geomPlugin_insertColorPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt,const unsigned char* rgbData/*=nullptr*/,const unsigned int* usrData/*=nullptr*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        currentGeomPlugin->geomPlugin_insertColorPointsIntoOctree(ocStruct,_tr,points,pointCnt,rgbData,usrData);
    }
}
void CPluginContainer::geomPlugin_insertMeshIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        float _tr2[7];
        meshTransformation.getInternalData(_tr2);
        currentGeomPlugin->geomPlugin_insertMeshIntoOctree(ocStruct,_tr1,obbStruct,_tr2,rgbData,usrData);
    }
}
void CPluginContainer::geomPlugin_insertOctreeIntoOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octree1Transformation.getInternalData(_tr1);
        float _tr2[7];
        octree2Transformation.getInternalData(_tr2);
        currentGeomPlugin->geomPlugin_insertOctreeIntoOctree(oc1Struct,_tr1,oc2Struct,_tr2,rgbData,usrData);
    }
}
bool CPluginContainer::geomPlugin_removePointsFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCnt)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        retVal=currentGeomPlugin->geomPlugin_removePointsFromOctree(ocStruct,_tr,points,pointCnt);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeMeshFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        float _tr2[7];
        meshTransformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_removeMeshFromOctree(ocStruct,_tr1,obbStruct,_tr2);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeOctreeFromOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octree1Transformation.getInternalData(_tr1);
        float _tr2[7];
        octree2Transformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_removeOctreeFromOctree(oc1Struct,_tr1,oc2Struct,_tr2);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createPtcloudFromPoints(const float* points,int pointCnt,const C7Vector* ptcloudOrigin/*=nullptr*/,float cellS/*=0.05f*/,int maxPointCnt/*=20*/,const unsigned char rgbData[3]/*=nullptr*/,float proximityTol/*=0.005f*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float tr[7];
        float* _tr=nullptr;
        if (ptcloudOrigin!=nullptr)
        {
            ptcloudOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createPtcloudFromPoints(points,pointCnt,_tr,cellS,maxPointCnt,rgbData,proximityTol);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createPtcloudFromColorPoints(const float* points,int pointCnt,const C7Vector* ptcloudOrigin/*=nullptr*/,float cellS/*=0.05f*/,int maxPointCnt/*=20*/,const unsigned char* rgbData/*=nullptr*/,float proximityTol/*=0.005f*/)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        float tr[7];
        float* _tr=nullptr;
        if (ptcloudOrigin!=nullptr)
        {
            ptcloudOrigin->getInternalData(tr);
            _tr=tr;
        }
        retVal=currentGeomPlugin->geomPlugin_createPtcloudFromColorPoints(points,pointCnt,_tr,cellS,maxPointCnt,rgbData,proximityTol);
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyPtcloud(const void* pcStruct)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_copyPtcloud(pcStruct);
    return(retVal);
}
void* CPluginContainer::geomPlugin_getPtcloudFromSerializationData(const unsigned char* serializationData)
{
    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_getPtcloudFromSerializationData(serializationData);
    return(retVal);
}
void CPluginContainer::geomPlugin_getPtcloudSerializationData(const void* pcStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        unsigned char* data=currentGeomPlugin->geomPlugin_getPtcloudSerializationData(pcStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_scalePtcloud(void* pcStruct,float f)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_scalePtcloud(pcStruct,f);
}
void CPluginContainer::geomPlugin_destroyPtcloud(void* pcStruct)
{
    if (currentGeomPlugin!=nullptr)
        currentGeomPlugin->geomPlugin_destroyPtcloud(pcStruct);
}
void CPluginContainer::geomPlugin_getPtcloudPoints(const void* pcStruct,std::vector<float>& pointData,std::vector<float>* colorData/*=nullptr*/,float prop/*=1.0f*/)
{
    pointData.clear();
    if (colorData!=nullptr)
        colorData->clear();
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        float* data=currentGeomPlugin->geomPlugin_getPtcloudPoints(pcStruct,&l,prop);
        if (data!=nullptr)
        {
            for (int i=0;i<l;i++)
            {
                pointData.push_back(data[6*i+0]);
                pointData.push_back(data[6*i+1]);
                pointData.push_back(data[6*i+2]);
                if (colorData!=nullptr)
                {
                    colorData->push_back(data[6*i+3]);
                    colorData->push_back(data[6*i+4]);
                    colorData->push_back(data[6*i+5]);
                    colorData->push_back(1.0f);
                }
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}

void CPluginContainer::geomPlugin_getPtcloudOctreeCorners(const void* pcStruct,std::vector<float>& points)
{
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        float* data=currentGeomPlugin->geomPlugin_getPtcloudOctreeCorners(pcStruct,&l);
        if (data!=nullptr)
        {
            points.assign(data,data+3*l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
int CPluginContainer::geomPlugin_getPtcloudNonEmptyCellCount(const void* pcStruct)
{
    int retVal=0;
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_getPtcloudNonEmptyCellCount(pcStruct);
    return(retVal);
}
void CPluginContainer::geomPlugin_insertPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,const unsigned char rgbData[3]/*=nullptr*/,float proximityTol/*=0.001f*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        currentGeomPlugin->geomPlugin_insertPointsIntoPtcloud(pcStruct,_tr,points,pointCnt,rgbData,proximityTol);
    }
}
void CPluginContainer::geomPlugin_insertColorPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,const unsigned char* rgbData/*=nullptr*/,float proximityTol/*=0.001f*/)
{
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        currentGeomPlugin->geomPlugin_insertColorPointsIntoPtcloud(pcStruct,_tr,points,pointCnt,rgbData,proximityTol);
    }
}
bool CPluginContainer::geomPlugin_removePointsFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,float proximityTol,int* countRemoved/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        retVal=currentGeomPlugin->geomPlugin_removePointsFromPtcloud(pcStruct,_tr,points,pointCnt,proximityTol,countRemoved);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeOctreeFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* countRemoved/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        float _tr2[7];
        octreeTransformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_removeOctreeFromPtcloud(pcStruct,_tr,ocStruct,_tr2,countRemoved);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_intersectPointsWithPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const float* points,int pointCnt,float proximityTol/*=0.001f*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        retVal=currentGeomPlugin->geomPlugin_intersectPointsWithPtcloud(pcStruct,_tr,points,pointCnt,proximityTol);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshMeshCollision(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,std::vector<float>* intersections/*=nullptr*/,int* mesh1Caching/*=nullptr*/,int* mesh2Caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        mesh1Transformation.getInternalData(_tr1);
        float _tr2[7];
        mesh2Transformation.getInternalData(_tr2);
        float* _intersections;
        int _intersectionsSize;
        float** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        retVal=currentGeomPlugin->geomPlugin_getMeshMeshCollision(mesh1ObbStruct,_tr1,mesh2ObbStruct,_tr2,_int,&_intersectionsSize,mesh1Caching,mesh2Caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshOctreeCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* meshCaching/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        meshTransformation.getInternalData(_tr1);
        float _tr2[7];
        octreeTransformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_getMeshOctreeCollision(meshObbStruct,_tr1,ocStruct,_tr2,meshCaching,ocCaching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshTriangleCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,std::vector<float>* intersections/*=nullptr*/,int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float* _intersections;
        int _intersectionsSize;
        float** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        retVal=currentGeomPlugin->geomPlugin_getMeshTriangleCollision(meshObbStruct,_tr,p.data,v.data,w.data,_int,&_intersectionsSize,caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshSegmentCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,std::vector<float>* intersections/*=nullptr*/,int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float* _intersections;
        int _intersectionsSize;
        float** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        retVal=currentGeomPlugin->geomPlugin_getMeshSegmentCollision(meshObbStruct,_tr,segmentExtremity.data,segmentVector.data,_int,&_intersectionsSize,caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeOctreeCollision(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,unsigned long long int* oc1Caching/*=nullptr*/,unsigned long long int* oc2Caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octree1Transformation.getInternalData(_tr1);
        float _tr2[7];
        octree2Transformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_getOctreeOctreeCollision(oc1Struct,_tr1,oc2Struct,_tr2,oc1Caching,oc2Caching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePtcloudCollision(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& ptcloudTransformation,unsigned long long int* ocCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        float _tr2[7];
        ptcloudTransformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_getOctreePtcloudCollision(ocStruct,_tr1,pcStruct,_tr2,ocCaching,pcCaching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeTriangleCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,unsigned long long int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getOctreeTriangleCollision(ocStruct,_tr1,p.data,v.data,w.data,caching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeSegmentCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,unsigned long long int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getOctreeSegmentCollision(ocStruct,_tr1,segmentExtremity.data,segmentVector.data,caching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointsCollision(const void* ocStruct,const C7Vector& octreeTransformation,const float* points,int pointCount)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getOctreePointsCollision(ocStruct,_tr1,points,pointCount);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,unsigned int* usrData/*=nullptr*/,unsigned long long int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getOctreePointCollision(ocStruct,_tr1,point.data,usrData,caching);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxBoxCollision(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        box1Transformation.getInternalData(_tr1);
        float _tr2[7];
        box2Transformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_getBoxBoxCollision(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data,boxesAreSolid);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxTriangleCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        boxTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getBoxTriangleCollision(_tr1,boxHalfSize.data,boxIsSolid,p.data,v.data,w.data);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxSegmentCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        boxTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getBoxSegmentCollision(_tr1,boxHalfSize.data,boxIsSolid,segmentEndPoint.data,segmentVector.data);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxPointCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,const C3Vector& point)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        boxTransformation.getInternalData(_tr1);
        retVal=currentGeomPlugin->geomPlugin_getBoxPointCollision(_tr1,boxHalfSize.data,point.data);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleTriangleCollision(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,std::vector<float>* intersections/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float* _intersections;
        int _intersectionsSize;
        float** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        retVal=currentGeomPlugin->geomPlugin_getTriangleTriangleCollision(p1.data,v1.data,w1.data,p2.data,v2.data,w2.data,_int,&_intersectionsSize);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleSegmentCollision(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,std::vector<float>* intersections/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float* _intersections;
        int _intersectionsSize;
        float** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        retVal=currentGeomPlugin->geomPlugin_getTriangleSegmentCollision(p.data,v.data,w.data,segmentEndPoint.data,segmentVector.data,_int,&_intersectionsSize);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* mesh1Caching/*=nullptr*/,int* mesh2Caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        mesh1Transformation.getInternalData(_tr1);
        float _tr2[7];
        mesh2Transformation.getInternalData(_tr2);
        float _minDistSegPt1[3];
        float _minDistSegPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshMeshDistanceIfSmaller(mesh1ObbStruct,_tr1,mesh2ObbStruct,_tr2,&dist,_minDistSegPt1,_minDistSegPt2,mesh1Caching,mesh2Caching);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistSegPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistSegPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshOctreeDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,C3Vector* meshMinDistPt/*=nullptr*/,C3Vector* ocMinDistPt/*=nullptr*/,int* meshCaching/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        meshTransformation.getInternalData(_tr1);
        float _tr2[7];
        octreeTransformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshOctreeDistanceIfSmaller(meshObbStruct,_tr1,ocStruct,_tr2,&dist,_minDistPt1,_minDistPt2,meshCaching,ocCaching);
        if (retVal)
        {
            if (meshMinDistPt!=nullptr)
                meshMinDistPt->setInternalData(_minDistPt1);
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshPtcloudDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* pcStruct,const C7Vector& pcTransformation,float& dist,C3Vector* meshMinDistPt/*=nullptr*/,C3Vector* pcMinDistPt/*=nullptr*/,int* meshCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        meshTransformation.getInternalData(_tr1);
        float _tr2[7];
        pcTransformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshPtcloudDistanceIfSmaller(meshObbStruct,_tr1,pcStruct,_tr2,&dist,_minDistPt1,_minDistPt2,meshCaching,pcCaching);
        if (retVal)
        {
            if (meshMinDistPt!=nullptr)
                meshMinDistPt->setInternalData(_minDistPt1);
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshTriangleDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshTriangleDistanceIfSmaller(meshObbStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,caching);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshSegmentDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshSegmentDistanceIfSmaller(meshObbStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,caching);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshPointDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& point,float& dist,C3Vector* minDistSegPt/*=nullptr*/,int* caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getMeshPointDistanceIfSmaller(meshObbStruct,_tr,point.data,&dist,_minDistPt1,caching);
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeOctreeDistanceIfSmaller(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,float& dist,C3Vector* oc1MinDistPt/*=nullptr*/,C3Vector* oc2MinDistPt/*=nullptr*/,unsigned long long int* oc1Caching/*=nullptr*/,unsigned long long int* oc2Caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octree1Transformation.getInternalData(_tr1);
        float _tr2[7];
        octree2Transformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getOctreeOctreeDistanceIfSmaller(oc1Struct,_tr1,oc2Struct,_tr2,&dist,_minDistPt1,_minDistPt2,oc1Caching,oc2Caching);
        if (retVal)
        {
            if (oc1MinDistPt!=nullptr)
                oc1MinDistPt->setInternalData(_minDistPt1);
            if (oc2MinDistPt!=nullptr)
                oc2MinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePtcloudDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& pcTransformation,float& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* pcMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        octreeTransformation.getInternalData(_tr1);
        float _tr2[7];
        pcTransformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getOctreePtcloudDistanceIfSmaller(ocStruct,_tr1,pcStruct,_tr2,&dist,_minDistPt1,_minDistPt2,ocCaching,pcCaching);
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setInternalData(_minDistPt1);
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeTriangleDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* triMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getOctreeTriangleDistanceIfSmaller(ocStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,ocCaching);
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setInternalData(_minDistPt1);
            if (triMinDistPt!=nullptr)
                triMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeSegmentDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* segMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getOctreeSegmentDistanceIfSmaller(ocStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,ocCaching);
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setInternalData(_minDistPt1);
            if (segMinDistPt!=nullptr)
                segMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,float& dist,C3Vector* ocMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getOctreePointDistanceIfSmaller(ocStruct,_tr,point.data,&dist,_minDistPt1,ocCaching);
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudPtcloudDistanceIfSmaller(const void* pc1Struct,const C7Vector& pc1Transformation,const void* pc2Struct,const C7Vector& pc2Transformation,float& dist,C3Vector* pc1MinDistPt/*=nullptr*/,C3Vector* pc2MinDistPt/*=nullptr*/,unsigned long long int* pc1Caching/*=nullptr*/,unsigned long long int* pc2Caching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        pc1Transformation.getInternalData(_tr1);
        float _tr2[7];
        pc2Transformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getPtcloudPtcloudDistanceIfSmaller(pc1Struct,_tr1,pc2Struct,_tr2,&dist,_minDistPt1,_minDistPt2,pc1Caching,pc2Caching);
        if (retVal)
        {
            if (pc1MinDistPt!=nullptr)
                pc1MinDistPt->setInternalData(_minDistPt1);
            if (pc2MinDistPt!=nullptr)
                pc2MinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudTriangleDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* pcMinDistPt/*=nullptr*/,C3Vector* triMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        pcTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getPtcloudTriangleDistanceIfSmaller(pcStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,pcCaching);
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setInternalData(_minDistPt1);
            if (triMinDistPt!=nullptr)
                triMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudSegmentDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* pcMinDistPt/*=nullptr*/,C3Vector* segMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        pcTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getPtcloudSegmentDistanceIfSmaller(pcStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,pcCaching);
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setInternalData(_minDistPt1);
            if (segMinDistPt!=nullptr)
                segMinDistPt->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudPointDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& point,float& dist,C3Vector* pcMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        pcTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getPtcloudPointDistanceIfSmaller(pcStruct,_tr,point.data,&dist,_minDistPt1,pcCaching);
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
float CPluginContainer::geomPlugin_getApproxBoxBoxDistance(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize)
{
    float retVal=0.0f;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        box1Transformation.getInternalData(_tr1);
        float _tr2[7];
        box2Transformation.getInternalData(_tr2);
        retVal=currentGeomPlugin->geomPlugin_getApproxBoxBoxDistance(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data);
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxBoxDistanceIfSmaller(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid,float& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr1[7];
        box1Transformation.getInternalData(_tr1);
        float _tr2[7];
        box2Transformation.getInternalData(_tr2);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getBoxBoxDistanceIfSmaller(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data,boxesAreSolid,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setInternalData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxTriangleDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        boxTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getBoxTriangleDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setInternalData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxSegmentDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        boxTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getBoxSegmentDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setInternalData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxPointDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,float& dist,C3Vector* distSegPt1/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        boxTransformation.getInternalData(_tr);
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getBoxPointDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,point.data,&dist,_minDistPt1);
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
float CPluginContainer::geomPlugin_getBoxPointDistance(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,C3Vector* distSegPt1/*=nullptr*/)
{
    float dist=FLT_MAX;
    geomPlugin_getBoxPointDistanceIfSmaller(boxTransformation,boxHalfSize,boxIsSolid,point,dist,distSegPt1);
    return(dist);
}
bool CPluginContainer::geomPlugin_getTriangleTriangleDistanceIfSmaller(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getTriangleTriangleDistanceIfSmaller(p1.data,v1.data,w1.data,p2.data,v2.data,w2.data,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleSegmentDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getTriangleSegmentDistanceIfSmaller(p.data,v.data,w.data,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTrianglePointDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& point,float& dist,C3Vector* minDistSegPt/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getTrianglePointDistanceIfSmaller(p.data,v.data,w.data,point.data,&dist,_minDistPt1);
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getSegmentSegmentDistanceIfSmaller(const C3Vector& segment1EndPoint,const C3Vector& segment1Vector,const C3Vector& segment2EndPoint,const C3Vector& segment2Vector,float& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _minDistPt1[3];
        float _minDistPt2[3];
        retVal=currentGeomPlugin->geomPlugin_getSegmentSegmentDistanceIfSmaller(segment1EndPoint.data,segment1Vector.data,segment2EndPoint.data,segment2Vector.data,&dist,_minDistPt1,_minDistPt2);
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setInternalData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setInternalData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getSegmentPointDistanceIfSmaller(const C3Vector& segmentEndPoint,const C3Vector& segmentVector,const C3Vector& point,float& dist,C3Vector* minDistSegPt/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _minDistPt1[3];
        retVal=currentGeomPlugin->geomPlugin_getSegmentPointDistanceIfSmaller(segmentEndPoint.data,segmentVector.data,point.data,&dist,_minDistPt1);
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setInternalData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectMeshIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* obbStruct,const C7Vector& meshTransformation,float& dist,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const float* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float _detectPt[3];
        float _triN[3];
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectMeshIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),obbStruct,_tr,&dist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
            if (triN!=nullptr)
                triN->setInternalData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectOctreeIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const float* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        float _detectPt[3];
        float _triN[3];
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectOctreeIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),ocStruct,_tr,&dist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
            if (triN!=nullptr)
                triN->setInternalData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectPtcloudIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const void* pcStruct,const C7Vector& ptcloudTransformation,float& dist,bool fast/*=false*/,C3Vector* detectPt/*=nullptr*/)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const float* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        ptcloudTransformation.getInternalData(_tr);
        float _detectPt[3];
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectPtcloudIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),pcStruct,_tr,&dist,fast,_detectPt);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectTriangleIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& p,const C3Vector& v,const C3Vector& w,float& dist,bool frontDetection/*=true*/,bool backDetection/*=true*/,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const float* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        float _detectPt[3];
        float _triN[3];
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectTriangleIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),p.data,v.data,w.data,&dist,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
            if (triN!=nullptr)
                triN->setInternalData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectSegmentIfSmaller(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,float& dist,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const float* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        float _detectPt[3];
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectSegmentIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),segmentEndPoint.data,segmentVector.data,&dist,maxAngle,_detectPt);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_raySensorDetectMeshIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* obbStruct,const C7Vector& meshTransformation,float& dist,float forbiddenDist/*=0.0f*/,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/,bool* forbiddenDistTouched/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        meshTransformation.getInternalData(_tr);
        float _detectPt[3];
        float _triN[3];
        retVal=currentGeomPlugin->geomPlugin_raySensorDetectMeshIfSmaller(rayStart.data,rayVect.data,obbStruct,_tr,&dist,forbiddenDist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN,forbiddenDistTouched);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
            if (triN!=nullptr)
                triN->setInternalData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_raySensorDetectOctreeIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* ocStruct,const C7Vector& octreeTransformation,float& dist,float forbiddenDist/*=0.0f*/,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,float maxAngle/*=0.0f*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/,bool* forbiddenDistTouched/*=nullptr*/)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        float _tr[7];
        octreeTransformation.getInternalData(_tr);
        float _detectPt[3];
        float _triN[3];
        retVal=currentGeomPlugin->geomPlugin_raySensorDetectOctreeIfSmaller(rayStart.data,rayVect.data,ocStruct,_tr,&dist,forbiddenDist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN,forbiddenDistTouched);
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setInternalData(_detectPt);
            if (triN!=nullptr)
                triN->setInternalData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_isPointInVolume(const std::vector<float>& planesIn,const C3Vector& point)
{
    bool retVal=false;
    const float* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    if (currentGeomPlugin!=nullptr)
        retVal=currentGeomPlugin->geomPlugin_isPointInVolume(_planesIn,int(planesIn.size()),point.data);
    return(retVal);
}
bool CPluginContainer::geomPlugin_isPointInVolume1AndOutVolume2(const std::vector<float>& planesIn,const std::vector<float>& planesOut,const C3Vector& point)
{
    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        if (planesIn.size()==0)
            retVal=true;
        else
            retVal=currentGeomPlugin->geomPlugin_isPointInVolume(&planesIn[0],int(planesIn.size()),point.data);
        if (retVal&&(planesOut.size()>0))
            retVal=!currentGeomPlugin->geomPlugin_isPointInVolume(&planesOut[0],int(planesOut.size()),point.data);
    }
    return(retVal);
}

void CPluginContainer::ikPlugin_emptyEnvironment()
{
    if (currentIkPlugin!=nullptr)
    {
        //currentIkPlugin->ikPlugin_switchEnvironment(ikEnvironment);
        currentIkPlugin->ikPlugin_eraseEnvironment(ikEnvironment);
        ikEnvironment=currentIkPlugin->ikPlugin_createEnv();
    }
}

void CPluginContainer::ikPlugin_eraseObject(int objectHandle)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_eraseObject(ikEnvironment,objectHandle);
}
void CPluginContainer::ikPlugin_setObjectParent(int objectHandle,int parentObjectHandle)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setObjectParent(ikEnvironment,objectHandle,parentObjectHandle);
}
int CPluginContainer::ikPlugin_createDummy()
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_createDummy(ikEnvironment);
    return(retVal);
}
void CPluginContainer::ikPlugin_setLinkedDummy(int dummyHandle,int linkedDummyHandle)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setLinkedDummy(ikEnvironment,dummyHandle,linkedDummyHandle);
}
int CPluginContainer::ikPlugin_createJoint(int jointType)
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_createJoint(ikEnvironment,jointType);
    return(retVal);
}
void CPluginContainer::ikPlugin_setJointMode(int jointHandle,int jointMode)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointMode(ikEnvironment,jointHandle,jointMode);
}
void CPluginContainer::ikPlugin_setJointInterval(int jointHandle,bool cyclic,float jMin,float jRange)
{
    float mr[2]={jMin,jRange};
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointInterval(ikEnvironment,jointHandle,cyclic,mr);
}
void CPluginContainer::ikPlugin_setJointScrewPitch(int jointHandle,float pitch)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointScrewPitch(ikEnvironment,jointHandle,pitch);
}
void CPluginContainer::ikPlugin_setJointIkWeight(int jointHandle,float ikWeight)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointIkWeight(ikEnvironment,jointHandle,ikWeight);
}
void CPluginContainer::ikPlugin_setJointMaxStepSize(int jointHandle,float maxStepSize)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointMaxStepSize(ikEnvironment,jointHandle,maxStepSize);
}
void CPluginContainer::ikPlugin_setJointDependency(int jointHandle,int dependencyJointHandle,float offset,float mult)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointDependency(ikEnvironment,jointHandle,dependencyJointHandle,offset,mult);
}
float CPluginContainer::ikPlugin_getJointPosition(int jointHandle)
{
    float retVal=0.0f;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_getJointPosition(ikEnvironment,jointHandle);
    return(retVal);
}
void CPluginContainer::ikPlugin_setJointPosition(int jointHandle,float position)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setJointPosition(ikEnvironment,jointHandle,position);
}
C4Vector CPluginContainer::ikPlugin_getSphericalJointQuaternion(int jointHandle)
{
    C4Vector retVal;
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_getSphericalJointQuaternion(ikEnvironment,jointHandle,retVal.data);
    return(retVal);
}
void CPluginContainer::ikPlugin_setSphericalJointQuaternion(int jointHandle,const C4Vector& quaternion)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setSphericalJointQuaternion(ikEnvironment,jointHandle,quaternion.data);
}
int CPluginContainer::ikPlugin_createIkGroup()
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_createIkGroup(ikEnvironment);
    return(retVal);
}
void CPluginContainer::ikPlugin_eraseIkGroup(int ikGroupHandle)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_eraseIkGroup(ikEnvironment,ikGroupHandle);
}
void CPluginContainer::ikPlugin_setIkGroupFlags(int ikGroupHandle,int flags)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkGroupFlags(ikEnvironment,ikGroupHandle,flags);
}
void CPluginContainer::ikPlugin_setIkGroupCalculation(int ikGroupHandle,int method,float damping,int maxIterations)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkGroupCalculation(ikEnvironment,ikGroupHandle,method,damping,maxIterations);
}
int CPluginContainer::ikPlugin_addIkElement(int ikGroupHandle,int tipHandle)
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_addIkElement(ikEnvironment,ikGroupHandle,tipHandle);
    return(retVal);
}
void CPluginContainer::ikPlugin_eraseIkElement(int ikGroupHandle,int ikElementIndex)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_eraseIkElement(ikEnvironment,ikGroupHandle,ikElementIndex);
}
void CPluginContainer::ikPlugin_setIkElementFlags(int ikGroupHandle,int ikElementIndex,int flags)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkElementFlags(ikEnvironment,ikGroupHandle,ikElementIndex,flags);
}
void CPluginContainer::ikPlugin_setIkElementBase(int ikGroupHandle,int ikElementIndex,int baseHandle,int constraintsBaseHandle)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkElementBase(ikEnvironment,ikGroupHandle,ikElementIndex,baseHandle,constraintsBaseHandle);
}
void CPluginContainer::ikPlugin_setIkElementConstraints(int ikGroupHandle,int ikElementIndex,int constraints)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkElementConstraints(ikEnvironment,ikGroupHandle,ikElementIndex,constraints);
}
void CPluginContainer::ikPlugin_setIkElementPrecision(int ikGroupHandle,int ikElementIndex,float linearPrecision,float angularPrecision)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkElementPrecision(ikEnvironment,ikGroupHandle,ikElementIndex,linearPrecision,angularPrecision);
}
void CPluginContainer::ikPlugin_setIkElementWeights(int ikGroupHandle,int ikElementIndex,float linearWeight,float angularWeight)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setIkElementWeights(ikEnvironment,ikGroupHandle,ikElementIndex,linearWeight,angularWeight);
}
int CPluginContainer::ikPlugin_handleIkGroup(int ikGroupHandle)
{
    int retVal=sim_ikresult_not_performed;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_handleIkGroup(ikEnvironment,ikGroupHandle);
    return(retVal);
}
bool CPluginContainer::ikPlugin_computeJacobian(int ikGroupHandle,int options)
{
    bool retVal=false;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_computeJacobian(ikEnvironment,ikGroupHandle,options);
    return(retVal);
}
CMatrix* CPluginContainer::ikPlugin_getJacobian(int ikGroupHandle)
{
    CMatrix* retVal=nullptr;
    if (currentIkPlugin!=nullptr)
    {
        int matrixSize[2];
        float* jc=currentIkPlugin->ikPlugin_getJacobian(ikEnvironment,ikGroupHandle,matrixSize);
        if (jc!=nullptr)
        {
            retVal=new CMatrix(matrixSize[1],matrixSize[0]);
            for (size_t r=0;r<matrixSize[1];r++)
            {
                for (size_t c=0;c<matrixSize[0];c++)
                    (retVal[0])(r,c)=jc[r*matrixSize[0]+c];
            }
            delete[] jc;
        }
    }
    return(retVal);
}
float CPluginContainer::ikPlugin_getManipulability(int ikGroupHandle)
{
    float retVal=0.0f;
    if (currentIkPlugin!=nullptr)
        retVal=currentIkPlugin->ikPlugin_getManipulability(ikEnvironment,ikGroupHandle);
    return(retVal);
}
int CPluginContainer::ikPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,float thresholdDist,int maxIterationsOrTimeInMs,float* retConfig,const float* metric,bool(*validationCallback)(float*),const int* jointOptions,const float* lowLimits,const float* ranges,std::string& errString)
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
    {
        char* errS=currentIkPlugin->ikPlugin_getConfigForTipPose(ikEnvironment,ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxIterationsOrTimeInMs,&retVal,retConfig,metric,validationCallback,jointOptions,lowLimits,ranges);
        if ( (retVal<0)&&(errS!=nullptr) )
        {
            errString=errS;
            delete[] errS;
        }
    }
    else
        errString=SIM_ERROR_IK_PLUGIN_NOT_FOUND;
    return(retVal);
}

static std::vector<int> _ikValidationCb_collisionPairs;
static std::vector<int> _ikValidationCb_jointHandles;

bool _validationCallback(float* conf)
{
    bool collisionFree=true;
    std::vector<float> memorized;
    std::vector<CJoint*> joints;
    for (size_t i=0;i<_ikValidationCb_jointHandles.size();i++)
    {
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(_ikValidationCb_jointHandles[i]);
        joints.push_back(it);
        memorized.push_back(it->getPosition());
        it->setPosition(conf[i],false);
    }
    for (size_t i=0;i<_ikValidationCb_collisionPairs.size()/2;i++)
    {
        int robot=_ikValidationCb_collisionPairs[2*i+0];
        if (robot>=0)
        {
            int env=_ikValidationCb_collisionPairs[2*i+1];
            if (env==sim_handle_all)
                env=-1;
            if (CCollisionRoutine::doEntitiesCollide(robot,env,nullptr,false,false,nullptr))
            {
                collisionFree=false;
                break;
            }
        }
    }
    for (size_t i=0;i<_ikValidationCb_jointHandles.size();i++)
    {
        CJoint* it=joints[i];
        it->setPosition(memorized[i],false);
    }
    return(collisionFree);
}

int CPluginContainer::ikPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,float thresholdDist,int maxIterationsOrTimeInMs,float* retConfig,const float* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const float* lowLimits,const float* ranges,std::string& errString)
{
    int retVal=-1;
    if (currentIkPlugin!=nullptr)
    {
        bool(*_validationCB)(float*)=nullptr;
        bool err=false;
        if ( (collisionPairCnt>0)&&(collisionPairs!=nullptr) )
        {
            _ikValidationCb_jointHandles.assign(jointHandles,jointHandles+jointCnt);
            _ikValidationCb_collisionPairs.clear();
            for (size_t i=0;i<size_t(collisionPairCnt);i++)
            {
                CSceneObject* eo1=App::currentWorld->sceneObjects->getObjectFromHandle(collisionPairs[2*i+0]);
                CCollection* ec1=App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+0]);
                CSceneObject* eo2=App::currentWorld->sceneObjects->getObjectFromHandle(collisionPairs[2*i+1]);
                CCollection* ec2=App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+1]);
                err=err||( ((eo1==nullptr)&&(ec1==nullptr)) || ((eo2==nullptr)&&(ec2==nullptr)&&(collisionPairs[2*i+1]!=sim_handle_all)) );
                _ikValidationCb_collisionPairs.push_back(collisionPairs[2*i+0]);
                _ikValidationCb_collisionPairs.push_back(collisionPairs[2*i+1]);
            }
            _validationCB=_validationCallback;
            if (err)
                errString=SIM_ERROR_INVALID_COLLISION_PAIRS;
        }
        if (!err)
            retVal=ikPlugin_getConfigForTipPose(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxIterationsOrTimeInMs,retConfig,metric,_validationCB,jointOptions,lowLimits,ranges,errString);
    }
    else
        errString=SIM_ERROR_IK_PLUGIN_NOT_FOUND;
    return(retVal);
}
C7Vector CPluginContainer::ikPlugin_getObjectLocalTransformation(int objectHandle)
{
    C7Vector tr;
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_getObjectLocalTransformation(ikEnvironment,objectHandle,tr.X.data,tr.Q.data);
    return(tr);
}
void CPluginContainer::ikPlugin_setObjectLocalTransformation(int objectHandle,const C7Vector& tr)
{
    if (currentIkPlugin!=nullptr)
        currentIkPlugin->ikPlugin_setObjectLocalTransformation(ikEnvironment,objectHandle,tr.X.data,tr.Q.data);
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

void* CPluginContainer::pythonPlugin_initState(int scriptHandle,const char* scriptName,std::string* errorMsg)
{
    void* retVal=nullptr;
    if (currentPythonPlugin!=nullptr)
    {
        retVal=currentPythonPlugin->pythonPlugin_initState(scriptHandle,scriptName);
        if ( (retVal==nullptr)&&(errorMsg!=nullptr) )
        {
            errorMsg[0].clear();
            char* err=currentPythonPlugin->pythonPlugin_getError();
            if (err!=nullptr)
            {
                errorMsg[0]=err;
                delete[] err;
            }
        }
    }
    return(retVal);
}

void CPluginContainer::pythonPlugin_cleanupState(void* state)
{
    if (currentPythonPlugin!=nullptr)
        currentPythonPlugin->pythonPlugin_cleanupState(state);
}

int CPluginContainer::pythonPlugin_loadCode(void* state,const char* code,const char* functionsToFind,bool* functionsFound,std::string* errorMsg)
{
    int retVal=-2;
    if (currentPythonPlugin!=nullptr)
    {
        retVal=currentPythonPlugin->pythonPlugin_loadCode(state,code,functionsToFind,functionsFound);
        if ( (retVal!=1)&&(errorMsg!=nullptr) )
        {
            errorMsg[0].clear();
            char* err=currentPythonPlugin->pythonPlugin_getError();
            if (err!=nullptr)
            {
                errorMsg[0]=err;
                delete[] err;
            }
        }
    }
    return(retVal);
}

int CPluginContainer::pythonPlugin_callFunc(void* state,const char* funcName,int inStackHandle,int outStackHandle,std::string* errorMsg)
{
    int retVal=-2;
    if (currentPythonPlugin!=nullptr)
    {
        retVal=currentPythonPlugin->pythonPlugin_callFunc(state,funcName,inStackHandle,outStackHandle);
        if ( (retVal<0)&&(errorMsg!=nullptr) )
        {
            errorMsg[0].clear();
            char* err=currentPythonPlugin->pythonPlugin_getError();
            if (err!=nullptr)
            {
                errorMsg[0]=err;
                delete[] err;
            }
        }
    }
    return(retVal);
}

int CPluginContainer::pythonPlugin_execStr(void* state,const char* str,int outStackHandle)
{
    int retVal=-2;
    if (currentPythonPlugin!=nullptr)
        retVal=currentPythonPlugin->pythonPlugin_execStr(state,str,outStackHandle);
    return(retVal);
}

int CPluginContainer::pythonPlugin_isDeprecated(const char* str)
{
    int retVal=-2;
    if (currentPythonPlugin!=nullptr)
        retVal=currentPythonPlugin->pythonPlugin_isDeprecated(str);
    return(retVal);
}

void CPluginContainer::pythonPlugin_getFuncs(const char* str,std::vector<std::string>& v)
{
    if (currentPythonPlugin!=nullptr)
    {
        char* funcs=currentPythonPlugin->pythonPlugin_getFuncs(str);
        if (funcs!=nullptr)
        {
            size_t off=0;
            size_t l=strlen(funcs+off);
            while (l!=0)
            {
                v.push_back(funcs+off);
                off+=l+1;
                l=strlen(funcs+off);
            }
            delete[] funcs;
        }
    }
}

void CPluginContainer::pythonPlugin_getConsts(const char* str,std::vector<std::string>& v)
{
    if (currentPythonPlugin!=nullptr)
    {
        char* funcs=currentPythonPlugin->pythonPlugin_getConsts(str);
        if (funcs!=nullptr)
        {
            size_t off=0;
            size_t l=strlen(funcs+off);
            while (l!=0)
            {
                v.push_back(funcs+off);
                off+=l+1;
                l=strlen(funcs+off);
            }
            delete[] funcs;
        }
    }
}

std::string CPluginContainer::pythonPlugin_getCalltip(const char* func)
{
    std::string retVal;
    if (currentPythonPlugin!=nullptr)
    {
        char* tip=currentPythonPlugin->pythonPlugin_getCalltip(func);
        if (tip!=nullptr)
        {
            retVal=tip;
            delete[] tip;
        }
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_pos(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetPos,const double* targetVel)
{
    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
        retVal=currentRuckigPlugin->ruckigPlugin_pos(scriptHandle,dofs,smallestTimeStep,flags,currentPos,currentVel,currentAccel,maxVel,maxAccel,maxJerk,selection,targetPos,targetVel);
    return(retVal);
}

int CPluginContainer::ruckigPlugin_vel(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const unsigned char* selection,const double* targetVel)
{
    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
        retVal=currentRuckigPlugin->ruckigPlugin_vel(scriptHandle,dofs,smallestTimeStep,flags,currentPos,currentVel,currentAccel,maxAccel,maxJerk,selection,targetVel);
    return(retVal);
}

int CPluginContainer::ruckigPlugin_step(int objHandle,double timeStep,double* newPos,double* newVel,double* newAccel,double* syncTime)
{
    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
        retVal=currentRuckigPlugin->ruckigPlugin_step(objHandle,timeStep,newPos,newVel,newAccel,syncTime);
    return(retVal);
}

int CPluginContainer::ruckigPlugin_remove(int objHandle)
{
    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
        retVal=currentRuckigPlugin->ruckigPlugin_remove(objHandle);
    return(retVal);
}

int CPluginContainer::ruckigPlugin_dofs(int objHandle)
{
    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
        retVal=currentRuckigPlugin->ruckigPlugin_dofs(objHandle);
    return(retVal);
}

int CPluginContainer::customUi_msgBox(int type, int buttons, const char *title, const char *message,int defaultAnswer)
{
    int retVal=-1;
    if (currentCustomUi!=nullptr)
    {
        retVal=defaultAnswer;
        bool doIt=false;
        if (type==sim_msgbox_type_info)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_infos);
        if (type==sim_msgbox_type_question)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_questions);
        if (type==sim_msgbox_type_warning)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_warnings);
        if (type==sim_msgbox_type_critical)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_errors);
        if (doIt)
            retVal=currentCustomUi->_customUi_msgBox(type,buttons,title,message);
    }
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
            if (strlen(res)>0)
            {
                files.assign(res);
                retVal=true;
            }
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
    else
        App::logMsg(sim_verbosity_errors,"simExtAssimp plugin was not found.");
    return(retVal);
}

void CPluginContainer::assimp_exportShapes(const int* shapeHandles,int shapeCount,const char* filename,const char* format,float scaling,int upVector,int options)
{
    if (currentAssimp!=nullptr)
        currentAssimp->_assimp_exportShapes(shapeHandles,shapeCount,filename,format,scaling,upVector,options);
    else
        App::logMsg(sim_verbosity_errors,"simExtAssimp plugin was not found.");
}

int CPluginContainer::assimp_importMeshes(const char* fileNames,float scaling,int upVector,int options,float*** allVertices,int** verticesSizes,int*** allIndices,int** indicesSizes)
{
    int retVal=0;
    if (currentAssimp!=nullptr)
        retVal=currentAssimp->_assimp_importMeshes(fileNames,scaling,upVector,options,allVertices,verticesSizes,allIndices,indicesSizes);
    else
        App::logMsg(sim_verbosity_errors,"simExtAssimp plugin was not found.");
    return(retVal);
}

void CPluginContainer::assimp_exportMeshes(int meshCnt,const float** allVertices,const int* verticesSizes,const int** allIndices,const int* indicesSizes,const char* filename,const char* format,float scaling,int upVector,int options)
{
    if (currentAssimp!=nullptr)
        currentAssimp->_assimp_exportMeshes(meshCnt,allVertices,verticesSizes,allIndices,indicesSizes,filename,format,scaling,upVector,options);
    else
        App::logMsg(sim_verbosity_errors,"simExtAssimp plugin was not found.");
}
