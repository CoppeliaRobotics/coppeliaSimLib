#pragma once

#include <simMath/4X4Matrix.h>
#include <simLib/simTypes.h>
#include <plugin.h>
#include <vMutex.h>
#include <set>
#include <string>
#include <scriptObject.h>

class CPluginContainer
{
  public:
    CPluginContainer();
    virtual ~CPluginContainer();

    CPlugin* getCurrentPlugin();
    CPlugin* loadAndInitPlugin(const char* namespaceAndVersion, long long int loadOrigin);
    bool deinitAndUnloadPlugin(int handle, long long int unloadOrigin, bool force = false);
    void unloadNewPlugins();
    void unloadLegacyPlugins();

    void announceScriptStateWillBeErased(int scriptHandle, long long int scriptUid);

    void uiCallAllPlugins(int msg, int* auxData = nullptr, void* auxPointer = nullptr);
    void sendEventCallbackMessageToAllPlugins(int msg, int* auxData = nullptr, void* auxPointer = nullptr,
                                              bool onlyToNewPlugins = false);
    void sendEventCallbackMessageToAllPlugins_old(int msg, int* auxVals = nullptr, void* data = nullptr,
                                                  int retVals[4] = nullptr);
    int getPluginCount();
    CPlugin* getPluginFromIndex(size_t index);
    CPlugin* getPluginFromName_old(const char* pluginName, bool caseSensitive);
    CPlugin* getPluginFromName(const char* pluginNamespaceAndVersion);
    CPlugin* getPluginFromHandle(int handle);
    void lockInterface();
    void unlockInterface();
    void printPluginStack();

    int addAndInitPlugin_old(const char* filename, const char* pluginName);
    bool unloadPlugin_old(int handle);
    void _removePlugin_old(int handle);

    bool selectExtRenderer(int index);
    bool extRenderer(int msg, void* data);

    bool qhull(void* data);
    bool hacd(void* data);
    bool vhacd(void* data);
    bool meshDecimator(void* data);

    std::vector<CPlugin*> currentPluginStack;

    // physics engines:
    CPlugin* currentDynEngine;
    CPlugin* bullet278Engine;
    CPlugin* bullet283Engine;
    CPlugin* odeEngine;
    CPlugin* vortexEngine;
    CPlugin* newtonEngine;
    CPlugin* mujocoEngine;
    CPlugin* drakeEngine;
    bool dyn_startSimulation(int engine, int version, const double floatParams[20], const int intParams[20]);
    bool dyn_isInitialized();
    bool dyn_isDynamicContentAvailable();
    bool dyn_removeParticleObject(int objectHandle);
    bool dyn_addParticleObjectItem(int objectHandle, const double* itemData, double simulationTime);
    bool dyn_getParticleData(const void* particle, double* pos, double* size, int* objectType, float** additionalColor);
    bool dyn_getContactForce(int dynamicPass, int objectHandle, int index, int objectHandles[2], double* contactInfo);
    void dyn_endSimulation();
    void dyn_step(double timeStep, double simulationTime);
    void dyn_serializeDynamicContent(const char* filenameAndPath, int bulletSerializationBuffer);
    int dyn_addParticleObject(int objectType, double size, double massOverVolume, const void* params, double lifeTime,
                              int maxItemCount, const float* ambient, const float* diffuse, const float* specular,
                              const float* emission);
    int dyn_getParticleObjectOtherFloatsPerItem(int objectHandle);
    double* dyn_getContactPoints(int* count);
    void** dyn_getParticles(int index, int* particlesCount, int* objectType, float** cols);
    int dyn_getDynamicStepDivider();
    double dyn_computeInertia(int shapeHandle, C7Vector& tr, C3Vector& diagI);
    double dyn_computePMI(const std::vector<double>& vertices, const std::vector<int>& indices, C7Vector& tr,
                          C3Vector& diagI);

    // geom plugin:
    CPlugin* currentGeomPlugin;
    bool isGeomPluginAvailable();
    void geomPlugin_releaseBuffer(void* buffer);

    // Mesh creation/destruction/manipulation/info
    void* geomPlugin_createMesh(const double* vertices, int verticesSize, const int* indices, int indicesSize,
                                const C7Vector* meshOrigin = nullptr, double triangleEdgeMaxLength = 0.3,
                                int maxTrianglesInBoundingBox = 8);
    void* geomPlugin_copyMesh(const void* meshObbStruct);
    void* geomPlugin_getMeshFromSerializationData(const unsigned char* serializationData);
    void geomPlugin_getMeshSerializationData(const void* meshObbStruct, std::vector<unsigned char>& serializationData);
    void geomPlugin_scaleMesh(void* meshObbStruct, double scalingFactor);
    void geomPlugin_destroyMesh(void* meshObbStruct);
    double geomPlugin_getMeshRootObbVolume(const void* meshObbStruct);

    // OC tree creation/destruction/manipulation/info
    void* geomPlugin_createOctreeFromPoints(const double* points, int pointCnt, const C7Vector* octreeOrigin = nullptr,
                                            double cellS = 0.05, const unsigned char rgbData[3] = nullptr,
                                            unsigned int usrData = 0);
    void* geomPlugin_createOctreeFromColorPoints(const double* points, int pointCnt,
                                                 const C7Vector* octreeOrigin = nullptr, double cellS = 0.05,
                                                 const unsigned char* rgbData = nullptr,
                                                 const unsigned int* usrData = nullptr);
    void* geomPlugin_createOctreeFromMesh(const void* meshObbStruct, const C7Vector& meshTransformation,
                                          const C7Vector* octreeOrigin = nullptr, double cellS = 0.05,
                                          const unsigned char rgbData[3] = nullptr, unsigned int usrData = 0);
    void* geomPlugin_createOctreeFromOctree(const void* otherOctreeStruct, const C7Vector& otherOctreeTransformation,
                                            const C7Vector* newOctreeOrigin = nullptr, double newOctreeCellS = 0.05,
                                            const unsigned char rgbData[3] = nullptr, unsigned int usrData = 0);
    void* geomPlugin_copyOctree(const void* ocStruct);
    void* geomPlugin_getOctreeFromSerializationData(const unsigned char* serializationData);
    void geomPlugin_getOctreeSerializationData(const void* ocStruct, std::vector<unsigned char>& serializationData);
    void* geomPlugin_getOctreeFromSerializationData_float(const unsigned char* serializationData);
    void geomPlugin_getOctreeSerializationData_float(const void* ocStruct,
                                                     std::vector<unsigned char>& serializationData);
    void geomPlugin_scaleOctree(void* ocStruct, double f);
    void geomPlugin_destroyOctree(void* ocStruct);
    void geomPlugin_getOctreeVoxelPositions(const void* ocStruct, std::vector<double>& voxelPositions);
    void geomPlugin_getOctreeVoxelColors(const void* ocStruct, std::vector<float>& voxelColors);
    void geomPlugin_getOctreeUserData(const void* ocStruct, std::vector<unsigned int>& userData);
    void geomPlugin_getOctreeCornersFromOctree(const void* ocStruct, std::vector<double>& points);

    void geomPlugin_insertPointsIntoOctree(void* ocStruct, const C7Vector& octreeTransformation, const double* points,
                                           int pointCnt, const unsigned char rgbData[3] = nullptr,
                                           unsigned int usrData = 0);
    void geomPlugin_insertColorPointsIntoOctree(void* ocStruct, const C7Vector& octreeTransformation,
                                                const double* points, int pointCnt,
                                                const unsigned char* rgbData = nullptr,
                                                const unsigned int* usrData = nullptr);
    void geomPlugin_insertMeshIntoOctree(void* ocStruct, const C7Vector& octreeTransformation, const void* obbStruct,
                                         const C7Vector& meshTransformation, const unsigned char rgbData[3] = nullptr,
                                         unsigned int usrData = 0);
    void geomPlugin_insertOctreeIntoOctree(void* oc1Struct, const C7Vector& octree1Transformation,
                                           const void* oc2Struct, const C7Vector& octree2Transformation,
                                           const unsigned char rgbData[3] = nullptr, unsigned int usrData = 0);
    bool geomPlugin_removePointsFromOctree(void* ocStruct, const C7Vector& octreeTransformation, const double* points,
                                           int pointCnt);
    bool geomPlugin_removeMeshFromOctree(void* ocStruct, const C7Vector& octreeTransformation, const void* obbStruct,
                                         const C7Vector& meshTransformation);
    bool geomPlugin_removeOctreeFromOctree(void* oc1Struct, const C7Vector& octree1Transformation,
                                           const void* oc2Struct, const C7Vector& octree2Transformation);

    // Point cloud creation/destruction/manipulation/info
    void* geomPlugin_createPtcloudFromPoints(const double* points, int pointCnt,
                                             const C7Vector* ptcloudOrigin = nullptr, double cellS = 0.05,
                                             int maxPointCnt = 20, const unsigned char rgbData[3] = nullptr,
                                             double proximityTol = 0.005);
    void* geomPlugin_createPtcloudFromColorPoints(const double* points, int pointCnt,
                                                  const C7Vector* ptcloudOrigin = nullptr, double cellS = 0.05,
                                                  int maxPointCnt = 20, const unsigned char* rgbData = nullptr,
                                                  double proximityTol = 0.005);
    void* geomPlugin_copyPtcloud(const void* pcStruct);
    void* geomPlugin_getPtcloudFromSerializationData(const unsigned char* serializationData);
    void geomPlugin_getPtcloudSerializationData(const void* pcStruct, std::vector<unsigned char>& serializationData);
    void* geomPlugin_getPtcloudFromSerializationData_float(const unsigned char* serializationData);
    void geomPlugin_getPtcloudSerializationData_float(const void* pcStruct,
                                                      std::vector<unsigned char>& serializationData);
    void geomPlugin_scalePtcloud(void* pcStruct, double f);
    void geomPlugin_destroyPtcloud(void* pcStruct);
    void geomPlugin_getPtcloudPoints(const void* pcStruct, std::vector<double>& pointData,
                                     std::vector<double>* colors = nullptr, double prop = 1.0);
    void geomPlugin_getPtcloudOctreeCorners(const void* pcStruct, std::vector<double>& points);
    int geomPlugin_getPtcloudNonEmptyCellCount(const void* pcStruct);

    void geomPlugin_insertPointsIntoPtcloud(void* pcStruct, const C7Vector& ptcloudTransformation, const double* points,
                                            int pointCnt, const unsigned char rgbData[3] = nullptr,
                                            double proximityTol = 0.001);
    void geomPlugin_insertColorPointsIntoPtcloud(void* pcStruct, const C7Vector& ptcloudTransformation,
                                                 const double* points, int pointCnt,
                                                 const unsigned char* rgbData = nullptr, double proximityTol = 0.001);
    bool geomPlugin_removePointsFromPtcloud(void* pcStruct, const C7Vector& ptcloudTransformation, const double* points,
                                            int pointCnt, double proximityTol, int* countRemoved = nullptr);
    bool geomPlugin_removeOctreeFromPtcloud(void* pcStruct, const C7Vector& ptcloudTransformation, const void* ocStruct,
                                            const C7Vector& octreeTransformation, int* countRemoved = nullptr);
    bool geomPlugin_intersectPointsWithPtcloud(void* pcStruct, const C7Vector& ptcloudTransformation,
                                               const double* points, int pointCnt, double proximityTol = 0.001);

    // Collision detection
    bool geomPlugin_getMeshMeshCollision(const void* mesh1ObbStruct, const C7Vector& mesh1Transformation,
                                         const void* mesh2ObbStruct, const C7Vector& mesh2Transformation,
                                         std::vector<double>* intersections = nullptr, int* mesh1Caching = nullptr,
                                         int* mesh2Caching = nullptr);
    bool geomPlugin_getMeshOctreeCollision(const void* meshObbStruct, const C7Vector& meshTransformation,
                                           const void* ocStruct, const C7Vector& octreeTransformation,
                                           int* meshCaching = nullptr, unsigned long long int* ocCaching = nullptr);
    bool geomPlugin_getMeshTriangleCollision(const void* meshObbStruct, const C7Vector& meshTransformation,
                                             const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                             std::vector<double>* intersections = nullptr, int* caching = nullptr);
    bool geomPlugin_getMeshSegmentCollision(const void* meshObbStruct, const C7Vector& meshTransformation,
                                            const C3Vector& segmentExtremity, const C3Vector& segmentVector,
                                            std::vector<double>* intersections = nullptr, int* caching = nullptr);

    bool geomPlugin_getOctreeOctreeCollision(const void* oc1Struct, const C7Vector& octree1Transformation,
                                             const void* oc2Struct, const C7Vector& octree2Transformation,
                                             unsigned long long int* oc1Caching = nullptr,
                                             unsigned long long int* oc2Caching = nullptr);
    bool geomPlugin_getOctreePtcloudCollision(const void* ocStruct, const C7Vector& octreeTransformation,
                                              const void* pcStruct, const C7Vector& ptcloudTransformation,
                                              unsigned long long int* ocCaching = nullptr,
                                              unsigned long long int* pcCaching = nullptr);
    bool geomPlugin_getOctreeTriangleCollision(const void* ocStruct, const C7Vector& octreeTransformation,
                                               const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                               unsigned long long int* caching = nullptr);
    bool geomPlugin_getOctreeSegmentCollision(const void* ocStruct, const C7Vector& octreeTransformation,
                                              const C3Vector& segmentExtremity, const C3Vector& segmentVector,
                                              unsigned long long int* caching = nullptr);
    bool geomPlugin_getOctreePointsCollision(const void* ocStruct, const C7Vector& octreeTransformation,
                                             const double* points, int pointCount);
    bool geomPlugin_getOctreePointCollision(const void* ocStruct, const C7Vector& octreeTransformation,
                                            const C3Vector& point, unsigned int* usrData = nullptr,
                                            unsigned long long int* caching = nullptr);

    bool geomPlugin_getBoxBoxCollision(const C7Vector& box1Transformation, const C3Vector& box1HalfSize,
                                       const C7Vector& box2Transformation, const C3Vector& box2HalfSize,
                                       bool boxesAreSolid);
    bool geomPlugin_getBoxTriangleCollision(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                            bool boxIsSolid, const C3Vector& p, const C3Vector& v, const C3Vector& w);
    bool geomPlugin_getBoxSegmentCollision(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                           bool boxIsSolid, const C3Vector& segmentEndPoint,
                                           const C3Vector& segmentVector);
    bool geomPlugin_getBoxPointCollision(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                         const C3Vector& point);

    bool geomPlugin_getTriangleTriangleCollision(const C3Vector& p1, const C3Vector& v1, const C3Vector& w1,
                                                 const C3Vector& p2, const C3Vector& v2, const C3Vector& w2,
                                                 std::vector<double>* intersections = nullptr);
    bool geomPlugin_getTriangleSegmentCollision(const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                std::vector<double>* intersections = nullptr);

    // Distance calculation
    bool geomPlugin_getMeshMeshDistanceIfSmaller(const void* mesh1ObbStruct, const C7Vector& mesh1Transformation,
                                                 const void* mesh2ObbStruct, const C7Vector& mesh2Transformation,
                                                 double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                 C3Vector* minDistSegPt2 = nullptr, int* mesh1Caching = nullptr,
                                                 int* mesh2Caching = nullptr);
    bool geomPlugin_getMeshOctreeDistanceIfSmaller(const void* meshObbStruct, const C7Vector& meshTransformation,
                                                   const void* ocStruct, const C7Vector& octreeTransformation,
                                                   double& dist, C3Vector* meshMinDistPt = nullptr,
                                                   C3Vector* ocMinDistPt = nullptr, int* meshCaching = nullptr,
                                                   unsigned long long int* ocCaching = nullptr);
    bool geomPlugin_getMeshPtcloudDistanceIfSmaller(const void* meshObbStruct, const C7Vector& meshTransformation,
                                                    const void* pcStruct, const C7Vector& pcTransformation,
                                                    double& dist, C3Vector* meshMinDistPt = nullptr,
                                                    C3Vector* pcMinDistPt = nullptr, int* meshCaching = nullptr,
                                                    unsigned long long int* pcCaching = nullptr);
    bool geomPlugin_getMeshTriangleDistanceIfSmaller(const void* meshObbStruct, const C7Vector& meshTransformation,
                                                     const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                     double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                     C3Vector* minDistSegPt2 = nullptr, int* caching = nullptr);
    bool geomPlugin_getMeshSegmentDistanceIfSmaller(const void* meshObbStruct, const C7Vector& meshTransformation,
                                                    const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                    double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                    C3Vector* minDistSegPt2 = nullptr, int* caching = nullptr);
    bool geomPlugin_getMeshPointDistanceIfSmaller(const void* meshObbStruct, const C7Vector& meshTransformation,
                                                  const C3Vector& point, double& dist, C3Vector* minDistSegPt = nullptr,
                                                  int* caching = nullptr);

    bool geomPlugin_getOctreeOctreeDistanceIfSmaller(const void* oc1Struct, const C7Vector& octree1Transformation,
                                                     const void* oc2Struct, const C7Vector& octree2Transformation,
                                                     double& dist, C3Vector* oc1MinDistPt = nullptr,
                                                     C3Vector* oc2MinDistPt = nullptr,
                                                     unsigned long long int* oc1Caching = nullptr,
                                                     unsigned long long int* oc2Caching = nullptr);
    bool geomPlugin_getOctreePtcloudDistanceIfSmaller(const void* ocStruct, const C7Vector& octreeTransformation,
                                                      const void* pcStruct, const C7Vector& pcTransformation,
                                                      double& dist, C3Vector* ocMinDistPt = nullptr,
                                                      C3Vector* pcMinDistPt = nullptr,
                                                      unsigned long long int* ocCaching = nullptr,
                                                      unsigned long long int* pcCaching = nullptr);
    bool geomPlugin_getOctreeTriangleDistanceIfSmaller(const void* ocStruct, const C7Vector& octreeTransformation,
                                                       const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                       double& dist, C3Vector* ocMinDistPt = nullptr,
                                                       C3Vector* triMinDistPt = nullptr,
                                                       unsigned long long int* ocCaching = nullptr);
    bool geomPlugin_getOctreeSegmentDistanceIfSmaller(const void* ocStruct, const C7Vector& octreeTransformation,
                                                      const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                      double& dist, C3Vector* ocMinDistPt = nullptr,
                                                      C3Vector* segMinDistPt = nullptr,
                                                      unsigned long long int* ocCaching = nullptr);
    bool geomPlugin_getOctreePointDistanceIfSmaller(const void* ocStruct, const C7Vector& octreeTransformation,
                                                    const C3Vector& point, double& dist,
                                                    C3Vector* ocMinDistPt = nullptr,
                                                    unsigned long long int* ocCaching = nullptr);

    bool geomPlugin_getPtcloudPtcloudDistanceIfSmaller(const void* pc1Struct, const C7Vector& pc1Transformation,
                                                       const void* pc2Struct, const C7Vector& pc2Transformation,
                                                       double& dist, C3Vector* pc1MinDistPt = nullptr,
                                                       C3Vector* pc2MinDistPt = nullptr,
                                                       unsigned long long int* pc1Caching = nullptr,
                                                       unsigned long long int* pc2Caching = nullptr);
    bool geomPlugin_getPtcloudTriangleDistanceIfSmaller(const void* pcStruct, const C7Vector& pcTransformation,
                                                        const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                        double& dist, C3Vector* pcMinDistPt = nullptr,
                                                        C3Vector* triMinDistPt = nullptr,
                                                        unsigned long long int* pcCaching = nullptr);
    bool geomPlugin_getPtcloudSegmentDistanceIfSmaller(const void* pcStruct, const C7Vector& pcTransformation,
                                                       const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                       double& dist, C3Vector* pcMinDistPt = nullptr,
                                                       C3Vector* segMinDistPt = nullptr,
                                                       unsigned long long int* pcCaching = nullptr);
    bool geomPlugin_getPtcloudPointDistanceIfSmaller(const void* pcStruct, const C7Vector& pcTransformation,
                                                     const C3Vector& point, double& dist,
                                                     C3Vector* pcMinDistPt = nullptr,
                                                     unsigned long long int* pcCaching = nullptr);

    double geomPlugin_getApproxBoxBoxDistance(const C7Vector& box1Transformation, const C3Vector& box1HalfSize,
                                              const C7Vector& box2Transformation, const C3Vector& box2HalfSize);
    bool geomPlugin_getBoxBoxDistanceIfSmaller(const C7Vector& box1Transformation, const C3Vector& box1HalfSize,
                                               const C7Vector& box2Transformation, const C3Vector& box2HalfSize,
                                               bool boxesAreSolid, double& dist, C3Vector* distSegPt1 = nullptr,
                                               C3Vector* distSegPt2 = nullptr, bool altRoutine = false);
    bool geomPlugin_getBoxTriangleDistanceIfSmaller(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                                    bool boxIsSolid, const C3Vector& p, const C3Vector& v,
                                                    const C3Vector& w, double& dist, C3Vector* distSegPt1 = nullptr,
                                                    C3Vector* distSegPt2 = nullptr, bool altRoutine = false);
    bool geomPlugin_getBoxSegmentDistanceIfSmaller(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                                   bool boxIsSolid, const C3Vector& segmentEndPoint,
                                                   const C3Vector& segmentVector, double& dist,
                                                   C3Vector* distSegPt1 = nullptr, C3Vector* distSegPt2 = nullptr,
                                                   bool altRoutine = false);
    bool geomPlugin_getBoxPointDistanceIfSmaller(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                                 bool boxIsSolid, const C3Vector& point, double& dist,
                                                 C3Vector* distSegPt1 = nullptr);
    double geomPlugin_getBoxPointDistance(const C7Vector& boxTransformation, const C3Vector& boxHalfSize,
                                          bool boxIsSolid, const C3Vector& point, C3Vector* distSegPt1 = nullptr);

    bool geomPlugin_getTriangleTriangleDistanceIfSmaller(const C3Vector& p1, const C3Vector& v1, const C3Vector& w1,
                                                         const C3Vector& p2, const C3Vector& v2, const C3Vector& w2,
                                                         double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                         C3Vector* minDistSegPt2 = nullptr);
    bool geomPlugin_getTriangleSegmentDistanceIfSmaller(const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                        const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                        double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                        C3Vector* minDistSegPt2 = nullptr);
    bool geomPlugin_getTrianglePointDistanceIfSmaller(const C3Vector& p, const C3Vector& v, const C3Vector& w,
                                                      const C3Vector& point, double& dist,
                                                      C3Vector* minDistSegPt = nullptr);

    bool geomPlugin_getSegmentSegmentDistanceIfSmaller(const C3Vector& segment1EndPoint, const C3Vector& segment1Vector,
                                                       const C3Vector& segment2EndPoint, const C3Vector& segment2Vector,
                                                       double& dist, C3Vector* minDistSegPt1 = nullptr,
                                                       C3Vector* minDistSegPt2 = nullptr);
    bool geomPlugin_getSegmentPointDistanceIfSmaller(const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                     const C3Vector& point, double& dist,
                                                     C3Vector* minDistSegPt = nullptr);

    // Volume sensor
    bool geomPlugin_volumeSensorDetectMeshIfSmaller(const std::vector<double>& planesIn,
                                                    const std::vector<double>& planesOut, const void* obbStruct,
                                                    const C7Vector& meshTransformation, double& dist, bool fast = false,
                                                    bool frontDetection = true, bool backDetection = true,
                                                    double maxAngle = 0.0, C3Vector* detectPt = nullptr,
                                                    C3Vector* triN = nullptr);
    bool geomPlugin_volumeSensorDetectOctreeIfSmaller(const std::vector<double>& planesIn,
                                                      const std::vector<double>& planesOut, const void* ocStruct,
                                                      const C7Vector& octreeTransformation, double& dist,
                                                      bool fast = false, bool frontDetection = true,
                                                      bool backDetection = true, double maxAngle = 0.0,
                                                      C3Vector* detectPt = nullptr, C3Vector* triN = nullptr);
    bool geomPlugin_volumeSensorDetectPtcloudIfSmaller(const std::vector<double>& planesIn,
                                                       const std::vector<double>& planesOut, const void* pcStruct,
                                                       const C7Vector& ptcloudTransformation, double& dist,
                                                       bool fast = false, C3Vector* detectPt = nullptr);
    bool geomPlugin_volumeSensorDetectTriangleIfSmaller(const std::vector<double>& planesIn,
                                                        const std::vector<double>& planesOut, const C3Vector& p,
                                                        const C3Vector& v, const C3Vector& w, double& dist,
                                                        bool frontDetection = true, bool backDetection = true,
                                                        double maxAngle = 0.0, C3Vector* detectPt = nullptr,
                                                        C3Vector* triN = nullptr);
    bool geomPlugin_volumeSensorDetectSegmentIfSmaller(const std::vector<double>& planesIn,
                                                       const std::vector<double>& planesOut,
                                                       const C3Vector& segmentEndPoint, const C3Vector& segmentVector,
                                                       double& dist, double maxAngle = 0.0,
                                                       C3Vector* detectPt = nullptr);

    // Ray sensor
    bool geomPlugin_raySensorDetectMeshIfSmaller(const C3Vector& rayStart, const C3Vector& rayVect,
                                                 const void* obbStruct, const C7Vector& meshTransformation,
                                                 double& dist, double forbiddenDist = 0.0, bool fast = false,
                                                 bool frontDetection = true, bool backDetection = true,
                                                 double maxAngle = 0.0, C3Vector* detectPt = nullptr,
                                                 C3Vector* triN = nullptr, bool* forbiddenDistTouched = nullptr);
    bool geomPlugin_raySensorDetectOctreeIfSmaller(const C3Vector& rayStart, const C3Vector& rayVect,
                                                   const void* ocStruct, const C7Vector& octreeTransformation,
                                                   double& dist, double forbiddenDist = 0.0, bool fast = false,
                                                   bool frontDetection = true, bool backDetection = true,
                                                   double maxAngle = 0.0, C3Vector* detectPt = nullptr,
                                                   C3Vector* triN = nullptr, bool* forbiddenDistTouched = nullptr);

    // Volume-pt test
    bool geomPlugin_isPointInVolume(const std::vector<double>& planesIn, const C3Vector& point);
    bool geomPlugin_isPointInVolume1AndOutVolume2(const std::vector<double>& planesIn,
                                                  const std::vector<double>& planesOut, const C3Vector& point);

    // IK plugin:
    CPlugin* currentIKPlugin;
    int ikEnvironment;
    void oldIkPlugin_emptyEnvironment();
    void oldIkPlugin_eraseObject(int objectHandle);
    void oldIkPlugin_setObjectParent(int objectHandle, int parentObjectHandle);
    int oldIkPlugin_createDummy();
    void oldIkPlugin_setLinkedDummy(int dummyHandle, int linkedDummyHandle);
    int oldIkPlugin_createJoint(int jointType);
    void oldIkPlugin_setJointMode(int jointHandle, int jointMode);
    void oldIkPlugin_setJointInterval(int jointHandle, bool cyclic, double jMin, double jRange);
    void oldIkPlugin_setJointScrewPitch(int jointHandle, double pitch);
    void oldIkPlugin_setJointIkWeight(int jointHandle, double ikWeight);
    void oldIkPlugin_setJointMaxStepSize(int jointHandle, double maxStepSize);
    void oldIkPlugin_setJointDependency(int jointHandle, int dependencyJointHandle, double offset, double mult);
    double oldIkPlugin_getJointPosition(int jointHandle);
    void oldIkPlugin_setJointPosition(int jointHandle, double position);
    C4Vector oldIkPlugin_getSphericalJointQuaternion(int jointHandle);
    void oldIkPlugin_setSphericalJointQuaternion(int jointHandle, const C4Vector& quaternion);
    int oldIkPlugin_createIkGroup();
    void oldIkPlugin_eraseIkGroup(int ikGroupHandle);
    void oldIkPlugin_setIkGroupFlags(int ikGroupHandle, int flags);
    void oldIkPlugin_setIkGroupCalculation(int ikGroupHandle, int method, double damping, int maxIterations);
    int oldIkPlugin_addIkElement(int ikGroupHandle, int tipHandle);
    void oldIkPlugin_eraseIkElement(int ikGroupHandle, int ikElementIndex);
    void oldIkPlugin_setIkElementFlags(int ikGroupHandle, int ikElementIndex, int flags);
    void oldIkPlugin_setIkElementBase(int ikGroupHandle, int ikElementIndex, int baseHandle, int constraintsBaseHandle);
    void oldIkPlugin_setIkElementConstraints(int ikGroupHandle, int ikElementIndex, int constraints);
    void oldIkPlugin_setIkElementPrecision(int ikGroupHandle, int ikElementIndex, double linearPrecision,
                                           double angularPrecision);
    void oldIkPlugin_setIkElementWeights(int ikGroupHandle, int ikElementIndex, double linearWeight,
                                         double angularWeight);
    int oldIkPlugin_handleIkGroup(int ikGroupHandle);
    bool oldIkPlugin_computeJacobian(int ikGroupHandle, int options);
    CMatrix* oldIkPlugin_getJacobian(int ikGroupHandle);
    double oldIkPlugin_getManipulability(int ikGroupHandle);
    int oldIkPlugin_getConfigForTipPose(int ikGroupHandle, int jointCnt, const int* jointHandles, double thresholdDist,
                                        int maxIterationsOrTimeInMs, double* retConfig, const double* metric,
                                        bool (*validationCallback)(double*), const int* jointOptions,
                                        const double* lowLimits, const double* ranges, std::string& errSting);
    int oldIkPlugin_getConfigForTipPose(int ikGroupHandle, int jointCnt, const int* jointHandles, double thresholdDist,
                                        int maxIterationsOrTimeInMs, double* retConfig, const double* metric,
                                        int collisionPairCnt, const int* collisionPairs, const int* jointOptions,
                                        const double* lowLimits, const double* ranges, std::string& errSting);
    C7Vector oldIkPlugin_getObjectLocalTransformation(int objectHandle);
    void oldIkPlugin_setObjectLocalTransformation(int objectHandle, const C7Vector& tr);

    // code editor plugin:
    CPlugin* currentCodeEditorPlugin;
    bool isCodeEditorPluginAvailable();
    bool codeEditor_openModal(const char* initText, const char* properties, std::string& modifiedText,
                              int* positionAndSize);
    int codeEditor_open(const char* initText, const char* properties);
    int codeEditor_setText(int handle, const char* text, int insertMode);
    bool codeEditor_getText(int handle, std::string& text, int* positionAndSize);
    int codeEditor_show(int handle, int showState);
    int codeEditor_close(int handle, int* positionAndSize);

    // Ruckig plugin:
    CPlugin* currentRuckigPlugin;
    int ruckigPlugin_pos(int scriptHandle, int dofs, double smallestTimeStep, int flags, const double* currentPos,
                         const double* currentVel, const double* currentAccel, const double* maxVel,
                         const double* maxAccel, const double* maxJerk, const bool* selection, const double* targetPos,
                         const double* targetVel);
    int ruckigPlugin_vel(int scriptHandle, int dofs, double smallestTimeStep, int flags, const double* currentPos,
                         const double* currentVel, const double* currentAccel, const double* maxAccel,
                         const double* maxJerk, const bool* selection, const double* targetVel);
    int ruckigPlugin_step(int objHandle, double timeStep, double* newPos, double* newVel, double* newAccel,
                          double* syncTime);
    int ruckigPlugin_remove(int objHandle);
    int ruckigPlugin_dofs(int objHandle);

    // Custom UI plugin:
    CPlugin* currentUIPlugin;
    int customUi_msgBox(int type, int buttons, const char* title, const char* message, int defaultAnswer);
    bool customUi_fileDialog(int type, const char* title, const char* startPath, const char* initName,
                             const char* extName, const char* ext, int native, std::string& files);

    // Assimp plugin:
    CPlugin* currentAssimpPlugin;
    bool isAssimpPluginAvailable();
    int* assimp_importShapes(const char* fileNames, int maxTextures, double scaling, int upVector, int options,
                             int* shapeCount);
    void assimp_exportShapes(const int* shapeHandles, int shapeCount, const char* filename, const char* format,
                             double scaling, int upVector, int options);
    int assimp_importMeshes(const char* fileNames, double scaling, int upVector, int options, double*** allVertices,
                            int** verticesSizes, int*** allIndices, int** indicesSizes);
    void assimp_exportMeshes(int meshCnt, const double** allVertices, const int* verticesSizes, const int** allIndices,
                             const int* indicesSizes, const char* filename, const char* format, double scaling,
                             int upVector, int options);

    CPlugin* currentExternalRendererPlugin;

    CPlugin* currentPovRayPlugin;

    CPlugin* currentOpenGl3Plugin;

    CPlugin* currentConvexPlugin;

    CPlugin* currentMeshDecimationPlugin;

  private:
    CPlugin* _tryToLoadPluginOnce(const char* namespaceAndVersion);

    int _nextHandle;
    VMutex _pluginInterfaceMutex;
    std::vector<CPlugin*> _allPlugins;
    std::set<std::string> _autoLoadPluginsTrials;
};
