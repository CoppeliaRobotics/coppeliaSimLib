#pragma once

#include <sceneObject.h>
#include <mesh.h>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

// FULLY STATIC CLASS
class CSceneObjectOperations
{
  public:
    static void scaleObjects(const std::vector<int> &selection, double scalingFactor, bool scalePositionsToo);
    static int groupSelection(std::vector<int> *selection);
    static void ungroupSelection(std::vector<int> *selection, bool fullUngroup = false);
    static int mergeSelection(std::vector<int> *selection);
    static void divideSelection(std::vector<int> *selection);
    static int convexDecompose(int shapeHandle, int options, const int *intParams, const double *floatParams);

  private:
    static void _deleteObjects(std::vector<int> *selection);
    static void _copyObjects(std::vector<int> *selection);
    static CShape *_groupShapes(const std::vector<CShape *> &shapesToGroup);
    static void _fullUngroupShape(CShape *it, std::vector<CShape *> &newShapes);
    static void _ungroupShape(CShape *it, std::vector<CShape *> &newShapes);
    static CShape *_mergeShapes(const std::vector<CShape *> &allShapesToMerge);
    static bool _divideShape(CShape *it, std::vector<CShape *> &newShapes);
    static CShape *_morphToConvexDecomposed(CShape *it, size_t nClusters, double maxConcavity, bool addExtraDistPoints,
                                            bool addFacesPoints, double maxConnectDist,
                                            size_t maxTrianglesInDecimatedMesh, size_t maxHullVertices,
                                            double smallClusterThreshold, bool useHACD, int resolution_VHACD,
                                            int depth_VHACD_old, double concavity_VHACD, int planeDownsampling_VHACD,
                                            int convexHullDownsampling_VHACD, double alpha_VHACD, double beta_VHACD,
                                            double gamma_VHACD_old, bool pca_VHACD, bool voxelBased_VHACD,
                                            int maxVerticesPerCH_VHACD, double minVolumePerCH_VHACD);

#ifdef SIM_WITH_GUI
  public:
    static void keyPress(int key);
    static bool processCommand(int commandID);
    static void addMenu(VMenu *menu);
#endif
};
