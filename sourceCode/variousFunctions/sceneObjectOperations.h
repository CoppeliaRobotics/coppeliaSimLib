
#pragma once

#include "3DObject.h"
#ifdef SIM_WITH_GUI
#include "vMenubar.h"
#endif

//FULLY STATIC CLASS
class CSceneObjectOperations
{
public:
    static void keyPress(int key);
    static bool processCommand(int commandID);

    static void copyObjects(std::vector<int>* selection,bool displayMessages);
    static void pasteCopyBuffer(bool displayMessages);
    static void cutObjects(std::vector<int>* selection,bool displayMessages);
    static void deleteObjects(std::vector<int>* selection,bool displayMessages);
    static void scaleObjects(const std::vector<int>& selection,float scalingFactor,bool scalePositionsToo);

    static void addRootObjectChildrenToSelection(std::vector<int>& selection);
    static int groupSelection(std::vector<int>* selection,bool showMessages);
    static void ungroupSelection(std::vector<int>* selection,bool showMessages);
    static bool mergeSelection(std::vector<int>* selection,bool showMessages);
    static void divideSelection(std::vector<int>* selection,bool showMessages);
    static void mergePathSelection(std::vector<int>* selection);
    static int generateConvexDecomposed(int shapeHandle,size_t nClusters,double maxConcavity,bool addExtraDistPoints,
                                        bool addFacesPoints,double maxConnectDist,size_t maxTrianglesInDecimatedMesh,
                                        size_t maxHullVertices,double smallClusterThreshold,
                                        bool individuallyConsiderMultishapeComponents,int maxIterations,
                                        bool useHACD,int resolution_VHACD,int depth_VHACD,float concavity_VHACD,
                                        int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                                        float alpha_VHACD,float beta_VHACD,float gamma_VHACD,bool pca_VHACD,
                                        bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,float minVolumePerCH_VHACD);
    static int generateConvexHull(int shapeHandle);
    static int convexDecompose_apiVersion(int shapeHandle,int options,const int* intParams,const float* floatParams);

#ifdef SIM_WITH_GUI
    static void addMenu(VMenu* menu);
#endif
};
