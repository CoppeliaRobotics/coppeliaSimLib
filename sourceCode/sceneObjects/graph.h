#pragma once

#include <graphDataStream.h>
#include <graphCurve.h>

// Old:
#include <sceneObject.h>
#include <graphData_old.h>
#include <graphDataComb_old.h>
#include <staticGraphCurve_old.h>
#include <sView.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                        \
    FUNCX(propGraph_size, "graphSize", sim_propertytype_float, 0, "Size", "Size of the 3D graph representation") \
    FUNCX(propGraph_bufferSize, "bufferSize", sim_propertytype_int, 0, "Buffer size", "")                        \
    FUNCX(propGraph_cyclic, "cyclic", sim_propertytype_bool, 0, "Cyclic", "Buffer is cyclic")                    \
    FUNCX(propGraph_backgroundColor, "backgroundColor", sim_propertytype_color, 0, "Background color", "")       \
    FUNCX(propGraph_foregroundColor, "foregroundColor", sim_propertytype_color, 0, "Foreground color", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_graph = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CGraph : public CSceneObject
{
  public:
    CGraph();
    virtual ~CGraph();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;

    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performScriptLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void setIsInScene(bool s) override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    // Old:
    // ---------
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    // ---------

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

    // Various
    bool getGraphCurveData(int graphType, int index, std::string& label, std::vector<double>& xVals,
                           std::vector<double>& yVals, int& curveType, float col[3], double minMax[6], int& curveId,
                           int& curveWidth, long long int& curveUid) const;
    void curveToClipboard(int graphType, const char* curveName) const;
    void curveToStatic(int graphType, const char* curveName);
    void removeStaticCurve(int graphType, const char* curveName);

    int addOrUpdateDataStream(CGraphDataStream* dataStream);
    int addOrUpdateCurve(CGraphCurve* curve);
    bool setDataStreamTransformation(int streamId, int trType, double mult, double off, int movAvgPeriod);
    bool setNextValueToInsert(int streamId, double v);
    CGraphDataStream* getGraphDataStream(int id) const;
    CGraphDataStream* getGraphDataStream(const char* name, bool staticStream) const;
    void getGraphDataStreamsFromIds(const int ids[3], CGraphDataStream* streams[3]) const;
    CGraphCurve* getGraphCurve(int id) const;
    CGraphCurve* getGraphCurve(const char* name, bool staticCurve) const;
    bool removeGraphDataStream(int id);
    bool removeGraphCurve(int id);
    void removeAllStreamsAndCurves();
    int duplicateCurveToStatic(int curveId, const char* curveName);
    void getAllStreamIds(std::vector<int>& allStreamIds);

    void setGraphSize(double theNewSize);
    double getGraphSize() const;
    bool getNeedsRefresh();
    void setBufferSize(int buffSize);
    int getBufferSize() const;
    void setCyclic(bool isCyclic);
    bool getCyclic() const;
    void resetGraph();
    void addNextPoint(double time);
    bool getAbsIndexOfPosition(int pos, int& absIndex) const;
    int getNumberOfPoints() const;

    void exportGraphData(VArchive& ar);

    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling() const;

    void removeAllStatics();

    void announceGraphDataObjectWillBeDestroyed(int graphDataID);

    CColorObject* getColor();

    bool xYZPlanesDisplay;
    bool graphGrid;
    bool graphValues;
    float backgroundColor[3];
    float foregroundColor[3];

    // Old:
    void removeAllStreamsAndCurves_old();
    void makeCurveStatic(int curveIndex, int dimensionIndex);
    int addNewGraphData(CGraphData_old* graphData);
    void removeGraphData(int id);
    int getDataStreamCount() const;
    int get2DCurveCount() const;
    int get3DCurveCount() const;
    CGraphData_old* getGraphData(int id) const;
    CGraphData_old* getGraphData(std::string theName) const;
    CGraphDataComb_old* getGraphData2D(int id) const;
    CGraphDataComb_old* getGraphData2D(std::string theName) const;
    CGraphDataComb_old* getGraphData3D(int id) const;
    CGraphDataComb_old* getGraphData3D(std::string theName) const;
    void add2DPartners(CGraphDataComb_old* it);
    void add3DPartners(CGraphDataComb_old* it);
    void remove2DPartners(int id);
    void remove3DPartners(int id);
    bool set3DDataName(int identifier, std::string newName);
    bool set2DDataName(int identifier, std::string newName);
    void setJustDrawCurves(bool justCurves);
    bool getJustDrawCurves() const;
    bool getData(const CGraphData_old* it, int pos, double& outputValue, bool cyclic, double range,
                 bool doUnitConversion) const;
    int getTrackingValueIndex() const;
    std::vector<CGraphData_old*> dataStreams_old;
    std::vector<CGraphDataComb_old*> curves3d_old;
    std::vector<CGraphDataComb_old*> curves2d_old;
    std::vector<CStaticGraphCurve_old*> staticStreamsAndCurves_old;

  protected:
    void _setBackgroundColor(const float col[3]);
    void _setForegroundColor(const float col[3]);
    CStaticGraphCurve_old* getStaticCurveFromName(int type, const char* name);

    std::vector<CGraphDataStream*> _dataStreams;
    std::vector<CGraphCurve*> _curves;

    CColorObject color;
    double _graphSize;
    int bufferSize;
    bool cyclic;
    bool _explicitHandling;
    int numberOfPoints;
    int startingPoint;
    std::vector<double> times;
    bool _needsRefresh;

    bool _initialExplicitHandling;

    // Old:
    bool justDrawCurves;
    int trackingValueIndex;
    bool trackingValueIsStatic;
    double trackingValue[2];
    double squareDistFromTrackingValue;
#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib);
    void lookAt(int windowSize[2], CSView* subView, bool timeGraph, bool drawText, bool passiveSubView,
                bool oneOneProportionForXYGraph);
    void validateViewValues(int windowSize[2], double graphPosition[2], double graphSize[2], bool timeGraph,
                            bool shiftOnly = false, bool keepProp = false, bool autoModeForTimeGraphXaxis = true);

  private:
    void drawGrid(int windowSize[2], double graphPosition[2], double graphSize[2]);
    void drawOverlay(int windowSize[2], double graphPosition[2], double graphSize[2], int mouseMode, CSView* subView,
                     bool passiveSubView);
    void drawValues(int windowSize[2], double graphPosition[2], double graphSize[2], int mousePosition[2],
                    bool mouseIsDown, bool dontRender, bool autoMode, bool timeGraphYaxisAutoMode, bool drawText,
                    bool passiveSubView, bool timeGraph, CSView* subView);
#endif
};
