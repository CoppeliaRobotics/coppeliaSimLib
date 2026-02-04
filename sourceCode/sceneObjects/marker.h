#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <array>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
MARKER_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_marker;
// ----------------------------------------------------------------------------------------------

struct CMultiSIt
{
    std::multiset<float>::iterator itX;
    std::multiset<float>::iterator itY;
    std::multiset<float>::iterator itZ;
};

struct CItemPointIts {
    std::array<CMultiSIt, 3> its;
};

class CMarker : public CSceneObject
{
  public:
    CMarker(int type = sim_markertype_points, unsigned char col[3] = nullptr, double size[3] = nullptr, int maxCnt = 0, int options = 0, float duplicateTol = 0.0f);

    virtual ~CMarker();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setLongProperty(const char* pName, long long int pState) override;
    int getLongProperty(const char* pName, long long int& pState) const override;
    int getHandleProperty(const char* pName, long long int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setStringProperty(const char* pName, const char* pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;

    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    int getMarkerOptions() const;

    void addItems(const std::vector<float>* pts, const std::vector<float>* quats, const std::vector<unsigned char>* rgbas, const std::vector<float>* sizes, bool transform = true, std::vector<long long int>* newIds = nullptr);
    void remItems(int itemCnt, bool triggerEvent = true);
    void remItems(const std::vector<long long int>* ids);

  protected:
    void _updateMarkerEvent(bool incremental, CCbor* evv = nullptr);
    void _initialize();
    void _rebuildMarkerBoundingBox();

    int _itemType;
    unsigned char _itemCol[4];
    double _itemSize[3];
    int _itemMaxCnt;
    int _itemOptions;
    float _itemDuplicateTol;
    int _itemPointCnt; // 1, 2 (lines) or 3 (triangles)

    std::vector<float> _pts; // 3 * _itemPointCnt per item
    std::vector<float> _quats; // 4 per item (qx, qy, qz, qw) (or 0 with lines and triangles, pts and similar)
    std::vector<unsigned char> _rgba; // 4 * _itemPointCnt per item
    std::vector<float> _sizes; // 3 per item (or 0 with lines and triangles, pts and similar)
    std::vector<long long int> _ids; // 1 item

    bool _sendFullEvent;
    long long int _nextId;
    int _newItemsCnt;
    std::vector<long long int> _remIds;

    std::multiset<float> _xs;
    std::multiset<float> _ys;
    std::multiset<float> _zs;
    std::unordered_map<long long int, CItemPointIts> itemIts;


#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
    void displayOverlay(CViewableBase* renderingObject, int displayAttrib);
    void drawItems(int displayAttrib, const double normalVectorForLinesAndPoints[3], bool overlay) const;
    void _drawPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawLines(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawTriangles(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawQuadPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawDiscPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawCubePoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
    void _drawSpherePoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const;
#endif
};
