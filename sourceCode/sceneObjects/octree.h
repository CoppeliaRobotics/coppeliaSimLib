#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propOctree_voxelSize,                    "voxelSize",                                     sim_propertytype_float,     0) \

#define FUNCX(name, str, v1, v2) const SProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<SProperty> allProps_ocTree = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CDummy;
class CPointCloud;

class COcTree : public CSceneObject
{
  public:
    COcTree();
    virtual ~COcTree();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor *ev);
    CSceneObject *copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void serialize(CSer &ar);
    void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    void performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int, int> *map);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int> *map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    void announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);

    void performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void setIsInScene(bool s);
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance);
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info);
    static int getPropertyInfo_static(const char* pName, int& info);

    // Various functions
    void setCellSize(double theNewSize);
    double getCellSize() const;
    void setCellSizeForDisplay(double theNewSizeForDisplay);
    double getCellSizeForDisplay() const;

    void insertPoints(const double *pts, int ptsCnt, bool ptsAreRelativeToOctree, const unsigned char *optionalColors3,
                      bool colorsAreIndividual, const unsigned int *optionalTags,
                      unsigned int theTagWhenOptionalTagsIsNull);
    void insertShape(CShape *shape, unsigned int theTag);
    void insertOctree(const COcTree *octree, unsigned int theTag);
    void insertDummy(const CDummy *dummy, unsigned int theTag);
    void insertPointCloud(const CPointCloud *pointCloud, unsigned int theTag);
    void insertOctree(const void *octree2Info, const C7Vector &octree2Tr, unsigned int theTag);
    void insertObjects(const std::vector<int> &sel);
    void insertObject(const CSceneObject *obj, unsigned int theTag);

    void subtractPoints(const double *pts, int ptsCnt, bool ptsAreRelativeToOctree);
    void subtractShape(CShape *shape);
    void subtractOctree(const COcTree *octree);
    void subtractDummy(const CDummy *dummy);
    void subtractPointCloud(const CPointCloud *pointCloud);
    void subtractOctree(const void *octree2Info, const C7Vector &octree2Tr);
    void subtractObjects(const std::vector<int> &sel);
    void subtractObject(const CSceneObject *obj);

    void clear();
    bool getShowOctree() const;
    void setShowOctree(bool show);
    bool getUseRandomColors() const;
    void setUseRandomColors(bool r);
    bool getColorIsEmissive() const;
    void setColorIsEmissive(bool e);
    bool getUsePointsInsteadOfCubes() const;
    void setUsePointsInsteadOfCubes(bool r);
    int getPointSize() const;
    void setPointSize(int s);
    const std::vector<double> *getCubePositions() const;
    std::vector<double> *getCubePositions();
    const void *getOctreeInfo() const;
    void *getOctreeInfo();
    CColorObject *getColor();

    void setVertexBufferId(int id);
    int getVertexBufferId() const;
    void setNormalBufferId(int id);
    int getNormalBufferId() const;
    float *getCubeVertices();
    float *getColors();

  protected:
    void _updateOctreeEvent() const;
    void _readPositionsAndColorsAndSetDimensions();

    // Variables which need to be serialized & copied
    CColorObject color;
    double _cellSize;
    int _pointSize;
    void *_octreeInfo;
    std::vector<double> _voxelPositions;
    std::vector<float> _colors;
    std::vector<unsigned char> _colorsByte;
    bool _showOctreeStructure;
    bool _useRandomColors;
    bool _usePointsInsteadOfCubes;
    bool _colorIsEmissive;

    // following only for display:
    float _cubeVertices[24 * 3];
    double _cellSizeForDisplay;
    int _vertexBufferId;
    int _normalBufferId;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
#endif
};
