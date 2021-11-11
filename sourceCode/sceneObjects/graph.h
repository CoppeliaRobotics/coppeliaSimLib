#pragma once

#include "graphDataStream.h"
#include "graphCurve.h"

// Old:
#include "sceneObject.h"
#include "graphData_old.h"
#include "graphDataComb_old.h"
#include "staticGraphCurve_old.h"
#include "sView.h"
#include "VPoint.h"

class CGraph : public CSceneObject  
{
public:

    CGraph();
    virtual ~CGraph();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);

    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performScriptLoadingMapping(const std::vector<int>* map);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);

    // Old:
    // ---------
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    // ---------

    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    // Various
    bool getGraphCurveData(int graphType,int index,std::string& label,std::vector<float>& xVals,std::vector<float>& yVals,int& curveType,float col[3],float minMax[6],int& curveId,int& curveWidth) const;
    void curveToClipboard(int graphType,const char* curveName) const;
    void curveToStatic(int graphType,const char* curveName);
    void removeStaticCurve(int graphType,const char* curveName);

    int addOrUpdateDataStream(CGraphDataStream* dataStream);
    int addOrUpdateCurve(CGraphCurve* curve);
    bool setDataStreamTransformation(int streamId,int trType,float mult,float off,int movAvgPeriod);
    bool setNextValueToInsert(int streamId,float v);
    CGraphDataStream* getGraphDataStream(int id) const;
    CGraphDataStream* getGraphDataStream(const char* name,bool staticStream) const;
    void getGraphDataStreamsFromIds(const int ids[3],CGraphDataStream* streams[3]) const;
    CGraphCurve* getGraphCurve(int id) const;
    CGraphCurve* getGraphCurve(const char* name,bool staticCurve) const;
    bool removeGraphDataStream(int id);
    bool removeGraphCurve(int id);
    void removeAllStreamsAndCurves();
    int duplicateCurveToStatic(int curveId,const char* curveName);
    void getAllStreamIds(std::vector<int>& allStreamIds);

    void setGraphSize(float theNewSize);
    float getGraphSize() const;
    bool getNeedsRefresh();
    void setBufferSize(int buffSize);
    int getBufferSize() const;
    void setCyclic(bool isCyclic);
    bool getCyclic() const;
    void resetGraph();
    void addNextPoint(float time);
    bool getAbsIndexOfPosition(int pos,int& absIndex) const;
    int getNumberOfPoints() const;

    void exportGraphData(VArchive &ar);

    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling() const;

    void removeAllStatics();

    void announceGraphDataObjectWillBeDestroyed(int graphDataID);

    CColorObject* getColor();

    bool xYZPlanesDisplay;
    bool graphGrid;
    bool graphValues;
    float backgroundColor[3];
    float textColor[3];

    // Old:
    void removeAllStreamsAndCurves_old();
    void makeCurveStatic(int curveIndex,int dimensionIndex);
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
    bool set3DDataName(int identifier,std::string newName);
    bool set2DDataName(int identifier,std::string newName);
    void setJustDrawCurves(bool justCurves);
    bool getJustDrawCurves() const;
    bool getData(const CGraphData_old* it,int pos,float& outputValue,bool cyclic,float range,bool doUnitConversion) const;
    int getTrackingValueIndex() const;
    std::vector <CGraphData_old*> dataStreams_old;
    std::vector <CGraphDataComb_old*> curves3d_old;
    std::vector <CGraphDataComb_old*> curves2d_old;
    std::vector <CStaticGraphCurve_old*> staticStreamsAndCurves_old;

protected:
    CStaticGraphCurve_old* getStaticCurveFromName(int type,const char* name);

    std::vector <CGraphDataStream*> _dataStreams;
    std::vector <CGraphCurve*> _curves;

    CColorObject color;
    float _graphSize;
    int bufferSize;
    bool cyclic;
    bool _explicitHandling;
    int numberOfPoints;
    int startingPoint;
    std::vector <float> times;
    bool _needsRefresh;

    bool _initialExplicitHandling;

    // Old:
    bool justDrawCurves;
    int trackingValueIndex;
    bool trackingValueIsStatic;
    float trackingValue[2];
    float squareDistFromTrackingValue;
    static VPoint currentWinSize;
#ifdef SIM_WITH_GUI
public:
    void lookAt(int windowSize[2],CSView* subView,bool timeGraph,bool drawText,bool passiveSubView,bool oneOneProportionForXYGraph);
    void validateViewValues(int windowSize[2],float graphPosition[2],float graphSize[2],
                bool timeGraph,bool shiftOnly=false,bool keepProp=false,bool autoModeForTimeGraphXaxis=true);

private:
    void drawGrid(int windowSize[2],float graphPosition[2],float graphSize[2]);
    void drawOverlay(int windowSize[2],float graphPosition[2],float graphSize[2],
                    int mouseMode,CSView* subView,bool passiveSubView);
    void drawValues(int windowSize[2],float graphPosition[2],float graphSize[2],
                    int mousePosition[2],bool mouseIsDown,bool dontRender,bool autoMode,bool timeGraphYaxisAutoMode,
                    bool drawText,bool passiveSubView,bool timeGraph,CSView* subView);
#endif
};
