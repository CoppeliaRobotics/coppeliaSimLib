#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
POINTCLOUD_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_pointCloud;
// ----------------------------------------------------------------------------------------------

class CDummy;
class COcTree;

class CPointCloud : public CSceneObject
{
  public:
    CPointCloud();
    virtual ~CPointCloud();

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
    void setIsInScene(bool s) override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;

    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
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
    void setMaxPointCountPerCell(int cnt);
    int getMaxPointCountPerCell() const;
    void insertPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToPointCloud,
                      const unsigned char* optionalColors3, bool colorsAreIndividual);
    void insertShape(CShape* shape);
    void insertOctree(const COcTree* octree);
    void insertDummy(const CDummy* dummy);
    void insertPointCloud(const CPointCloud* pointCloud);
    void insertObjects(const std::vector<int>& sel);
    void insertObject(const CSceneObject* obj);

    int removePoints(const double* pts, int ptsCnt, bool ptsAreRelativeToPointCloud, double distanceTolerance);
    void subtractOctree(const COcTree* octree);
    void subtractDummy(const CDummy* dummy, double distanceTolerance);
    void subtractPointCloud(const CPointCloud* pointCloud, double distanceTolerance);
    void subtractOctree(const void* octree2Info, const C7Vector& octree2Tr);
    void subtractObjects(const std::vector<int>& sel);
    void subtractObject(const CSceneObject* obj, double distanceTolerance);

    int intersectPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToPointCloud, double distanceTolerance);

    void clear();
    bool getShowOctree() const;
    void setShowOctree(bool show);
    double getAveragePointCountInCell();
    int getPointSize() const;
    void setPointSize(int s);
    double getBuildResolution() const;
    void setBuildResolution(double r);
    double getRemovalDistanceTolerance() const;
    void setRemovalDistanceTolerance(double t);
    double getInsertionDistanceTolerance() const;
    void setInsertionDistanceTolerance(double t);
    bool getUseRandomColors() const;
    void setUseRandomColors(bool r);
    bool getColorIsEmissive() const;
    void setColorIsEmissive(bool e);
    bool getDoNotUseCalculationStructure() const;
    void setDoNotUseCalculationStructure(bool s);
    double getPointDisplayRatio() const;
    void setPointDisplayRatio(double r);
    const std::vector<double>* getPoints() const;
    std::vector<double>* getPoints();
    const void* getPointCloudInfo() const;
    void* getPointCloudInfo();

    CColorObject* getColor();
    std::vector<double>* getColors();
    std::vector<double>* getDisplayPoints();
    std::vector<double>* getDisplayColors();

  protected:
    void _updatePointCloudEvent(bool incremental, CCbor* evv = nullptr);
    void _readPositionsAndColorsAndSetDimensions(bool incrementalDisplayUpdate);
    void _getCharRGB3Colors(const std::vector<double>& floatRGBA, std::vector<unsigned char>& charRGB);

    // Variables which need to be serialized & copied
    CColorObject color;
    double _cellSize;
    int _maxPointCountPerCell;
    void* _pointCloudInfo;
    std::vector<double> _points;
    std::vector<double> _colors;
    std::vector<double> _displayPoints;
    std::vector<double> _displayColors;
    std::vector<unsigned char> _displayColorsByte;
    bool _showOctreeStructure;
    bool _useRandomColors;
    int _pointSize;
    int _nonEmptyCells;
    double _buildResolution;
    double _removalDistanceTolerance;
    double _insertionDistanceTolerance;
    double _pointDisplayRatio;
    bool _doNotUseOctreeStructure;
    bool _colorIsEmissive;
    bool _refreshDisplay;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
