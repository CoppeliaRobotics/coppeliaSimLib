#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
OCTREE_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_ocTree;
// ----------------------------------------------------------------------------------------------

class CDummy;
class CPointCloud;

class COcTree : public CSceneObject
{
  public:
    COcTree();
    virtual ~COcTree();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;

    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void setIsInScene(bool s) override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL) override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    std::string getObjectState() const;

    // Various functions
    void setCellSize(double theNewSize);
    double getCellSize() const;
    void setCellSizeForDisplay(double theNewSizeForDisplay);
    double getCellSizeForDisplay() const;

    void insertPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToOctree, const unsigned char* optionalColors3,
                      bool colorsAreIndividual, const unsigned int* optionalTags,
                      unsigned int theTagWhenOptionalTagsIsNull);
    void insertShape(CShape* shape, unsigned int theTag);
    void insertOctree(const COcTree* octree, unsigned int theTag);
    void insertDummy(const CDummy* dummy, unsigned int theTag);
    void insertPointCloud(const CPointCloud* pointCloud, unsigned int theTag);
    void insertOctree(const void* octree2Info, const C7Vector& octree2Tr, unsigned int theTag);
    void insertObjects(const std::vector<int>& sel);
    void insertObject(const CSceneObject* obj, unsigned int theTag);

    void subtractPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToOctree);
    void subtractShape(CShape* shape);
    void subtractOctree(const COcTree* octree);
    void subtractDummy(const CDummy* dummy);
    void subtractPointCloud(const CPointCloud* pointCloud);
    void subtractOctree(const void* octree2Info, const C7Vector& octree2Tr);
    void subtractObjects(const std::vector<int>& sel);
    void subtractObject(const CSceneObject* obj);

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
    const std::vector<double>* getCubePositions() const;
    std::vector<double>* getCubePositions();
    const void* getOctreeInfo() const;
    void* getOctreeInfo();
    CColorObject* getColor();

    void setVertexBufferId(int id);
    int getVertexBufferId() const;
    void setNormalBufferId(int id);
    int getNormalBufferId() const;
    float* getCubeVertices();
    float* getColors();

  protected:
    void _updateOctreeEvent() const;
    void _readPositionsAndColorsAndSetDimensions();

    // Variables which need to be serialized & copied
    CColorObject color;
    double _cellSize;
    int _pointSize;
    void* _octreeInfo;
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
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
