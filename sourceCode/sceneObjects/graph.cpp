#include "simInternal.h"
#include "graph.h"
#include "tt.h"
#include "gV.h"
#include "graphingRoutines_old.h"
#include "simStrings.h"
#include <boost/lexical_cast.hpp>
#include "vVarious.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "graphRendering.h"
#include "simFlavor.h"

CGraph::CGraph()
{
    _objectType=sim_object_graph_type;
    justDrawCurves=false;
    _needsRefresh=true;
    _explicitHandling=true; // changed to true on 29.11.2020: graphs are not handled anymore by the main script since CoppeliaSim V4.2.0 on
    bufferSize=1000;
    numberOfPoints=0;
    startingPoint=0;
    curves3d_old.reserve(4);
    curves3d_old.clear();
    curves2d_old.reserve(4);
    curves2d_old.clear();
    dataStreams_old.reserve(16);
    dataStreams_old.clear();
    times.reserve(bufferSize);
    times.clear();
    for (int i=0;i<bufferSize;i++)
        times.push_back(0.0f);

    _localObjectSpecialProperty=0; // actually also renderable, but turned off by default!
    cyclic=true;
    xYZPlanesDisplay=true;
    graphGrid=true;
    graphValues=true;
    _graphSize=0.1f;
    color.setDefaultValues();
    color.setColor(0.15f,0.15f,0.15f,sim_colorcomponent_ambient_diffuse);

    backgroundColor[0]=0.1f;
    backgroundColor[1]=0.1f;
    backgroundColor[2]=0.1f;

    textColor[0]=0.8f;
    textColor[1]=0.8f;
    textColor[2]=0.8f;
    _visibilityLayer=GRAPH_LAYER;
    _objectAlias=IDSOGL_GRAPH;
    _objectName_old=IDSOGL_GRAPH;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

CGraph::~CGraph()
{
    removeAllStreamsAndCurves();

    // Old:
    removeAllStreamsAndCurves_old();
}

void CGraph::removeAllStreamsAndCurves_old()
{
    for (size_t i=0;i<curves3d_old.size();i++)
        delete curves3d_old[i];
    curves3d_old.clear();
    for (size_t i=0;i<curves2d_old.size();i++)
        delete curves2d_old[i];
    curves2d_old.clear();
    for (size_t i=0;i<dataStreams_old.size();i++)
        delete dataStreams_old[i];
    dataStreams_old.clear();
    for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
        delete staticStreamsAndCurves_old[i];
    staticStreamsAndCurves_old.clear();
}

std::string CGraph::getObjectTypeInfo() const
{
    return(IDSOGL_GRAPH);
}
std::string CGraph::getObjectTypeInfoExtended() const
{
    return(IDSOGL_GRAPH);
}
bool CGraph::isPotentiallyCollidable() const
{
    return(false);
}
bool CGraph::isPotentiallyMeasurable() const
{
    return(false);
}
bool CGraph::isPotentiallyDetectable() const
{
    return(false);
}
bool CGraph::isPotentiallyRenderable() const
{
    return(false);
}

void CGraph::computeBoundingBox()
{
    C3Vector maxV(_graphSize/2.0f,_graphSize/2.0f,_graphSize/2.0f);
    _setBoundingBox(maxV*-1.0f,maxV);
}

void CGraph::setJustDrawCurves(bool justCurves)
{
    justDrawCurves=justCurves;
}

bool CGraph::getJustDrawCurves() const
{
    return(justDrawCurves);
}

int CGraph::getDataStreamCount() const
{
    return((int)dataStreams_old.size());
}
int CGraph::get2DCurveCount() const
{
    return((int)curves2d_old.size());
}
int CGraph::get3DCurveCount() const
{
    return((int)curves3d_old.size());
}

CGraphData_old* CGraph::getGraphData(int id) const
{
    for (size_t i=0;i<dataStreams_old.size();i++)
    {
        if (dataStreams_old[i]->getIdentifier()==id)
            return(dataStreams_old[i]);
    }
    return(nullptr);
}
CGraphData_old* CGraph::getGraphData(std::string theName) const
{
    for (size_t i=0;i<dataStreams_old.size();i++)
    {
        if (dataStreams_old[i]->getName().compare(theName)==0)
            return(dataStreams_old[i]);
    }
    return(nullptr);
}
CGraphDataComb_old* CGraph::getGraphData3D(int id) const
{
    for (size_t i=0;i<curves3d_old.size();i++)
    {
        if (curves3d_old[i]->getIdentifier()==id)
        return(curves3d_old[i]);
    }
    return(nullptr);
}
CGraphDataComb_old* CGraph::getGraphData3D(std::string theName) const
{
    for (size_t i=0;i<curves3d_old.size();i++)
    {
        if (curves3d_old[i]->getName().compare(theName)==0)
            return(curves3d_old[i]);
    }
    return(nullptr);
}
CGraphDataComb_old* CGraph::getGraphData2D(int id) const
{
    for (size_t i=0;i<curves2d_old.size();i++)
    {
        if (curves2d_old[i]->getIdentifier()==id)
            return(curves2d_old[i]);
    }
    return(nullptr);
}
CGraphDataComb_old* CGraph::getGraphData2D(std::string theName) const
{
    for (size_t i=0;i<curves2d_old.size();i++)
    {
        if (curves2d_old[i]->getName().compare(theName)==0)
            return(curves2d_old[i]);
    }
    return(nullptr);
}



CGraphDataStream* CGraph::getGraphDataStream(int id) const
{
    CGraphDataStream* retVal=nullptr;
    for (size_t i=0;i<_dataStreams.size();i++)
    {
        if (_dataStreams[i]->getId()==id)
        {
            retVal=_dataStreams[i];
            break;
        }
    }
    return(retVal);
}

CGraphDataStream* CGraph::getGraphDataStream(const char* name,bool staticStream) const
{
    CGraphDataStream* retVal=nullptr;
    for (size_t i=0;i<_dataStreams.size();i++)
    {
        if ( (_dataStreams[i]->getStreamName().compare(name)==0)&&(_dataStreams[i]->getIsStatic()==staticStream) )
        {
            retVal=_dataStreams[i];
            break;
        }
    }
    return(retVal);
}

void CGraph::getGraphDataStreamsFromIds(const int ids[3],CGraphDataStream* streams[3]) const
{
    for (size_t i=0;i<3;i++)
    {
        streams[i]=nullptr;
        if (ids[i]!=-1)
            streams[i]=getGraphDataStream(ids[i]);
    }
}

CGraphCurve* CGraph::getGraphCurve(int id) const
{
    CGraphCurve* retVal=nullptr;
    for (size_t i=0;i<_curves.size();i++)
    {
        if (_curves[i]->getId()==id)
        {
            retVal=_curves[i];
            break;
        }
    }
    return(retVal);
}

CGraphCurve* CGraph::getGraphCurve(const char* name,bool staticCurve) const
{
    CGraphCurve* retVal=nullptr;
    for (size_t i=0;i<_curves.size();i++)
    {
        if ( (_curves[i]->getCurveName().compare(name)==0)&&(_curves[i]->getIsStatic()==staticCurve) )
        {
            retVal=_curves[i];
            break;
        }
    }
    return(retVal);
}

int CGraph::addOrUpdateDataStream(CGraphDataStream* dataStream)
{
    int retVal=-1; // error, curve already exists and is static
    CGraphDataStream* stream=getGraphDataStream(dataStream->getStreamName().c_str(),true);
    if (stream==nullptr)
    {
        stream=getGraphDataStream(dataStream->getStreamName().c_str(),false);
        if (stream!=nullptr)
        { // such a stream already exists
            stream->setBasics(dataStream->getUnitStr().c_str(),dataStream->getOptions(),dataStream->getColorPtr(),dataStream->getCyclicRange(),dataStream->getScriptHandle());
            delete dataStream;
        }
        else
        {
            int id=0;
            while (getGraphDataStream(id)!=nullptr)
                id++;
            dataStream->setId(id);
            _dataStreams.push_back(dataStream);
            dataStream->reset(bufferSize);
            stream=dataStream;
        }
        retVal=stream->getId();
        _needsRefresh=true;
    }

    // Old:
    // --------
    // Make sure we don't have twice that name (old and new streams)
    CGraphData_old* oldStream=getGraphData(stream->getStreamName());
    if (oldStream!=nullptr)
        removeGraphData(oldStream->getIdentifier());
    // --------

    return(retVal);
}

int CGraph::addOrUpdateCurve(CGraphCurve* curve)
{
    int retVal=-1; // error, static curve with same name already exists
    CGraphCurve* theCurve=getGraphCurve(curve->getCurveName().c_str(),true);
    if (theCurve==nullptr)
    {
        theCurve=getGraphCurve(curve->getCurveName().c_str(),false);
        std::vector<int> allStreamIds;
        getAllStreamIds(allStreamIds);
        curve->updateStreamIds(allStreamIds);
        if (theCurve!=nullptr)
        { // such a curve already exists
            theCurve->setBasics(curve->getDim(),curve->getStreamIdsPtr(),curve->getDefaultValsPtr(),curve->getUnitStr().c_str(),curve->getOptions(),curve->getColorPtr(),curve->getCurveWidth(),curve->getScriptHandle());
            delete curve;
        }
        else
        {
            int id=10000;
            while (getGraphCurve(id)!=nullptr)
                id++;
            curve->setId(id);
            _curves.push_back(curve);
            theCurve=curve;
        }
        retVal=theCurve->getId();
        _needsRefresh=true;
    }

    // Old:
    // --------
    // Make sure we don't have twice that name (old and new curves)
    if (theCurve->getDim()==2)
    {
        CGraphDataComb_old* oldCurve=getGraphData2D(theCurve->getCurveName());
        if (oldCurve!=nullptr)
            remove2DPartners(oldCurve->getIdentifier());
    }
    if (theCurve->getDim()==3)
    {
        CGraphDataComb_old* oldCurve=getGraphData3D(theCurve->getCurveName());
        if (oldCurve!=nullptr)
            remove3DPartners(oldCurve->getIdentifier());
    }
    // --------

    return(retVal);
}

bool CGraph::setDataStreamTransformation(int streamId,int trType,floatDouble mult,floatDouble off,int movAvgPeriod)
{
    CGraphDataStream* stream=getGraphDataStream(streamId);
    if (stream!=nullptr)
    {
        if (stream->setTransformation(trType,mult,off,movAvgPeriod))
            stream->reset(bufferSize);
        _needsRefresh=true;
    }
    return(stream!=nullptr);
}

bool CGraph::setNextValueToInsert(int streamId,floatDouble v)
{
    CGraphDataStream* stream=getGraphDataStream(streamId);
    if (stream!=nullptr)
        stream->setNextValueToInsert(v);
    return(stream!=nullptr);
}

bool CGraph::removeGraphDataStream(int id)
{
    bool retVal=false;
    for (size_t i=0;i<_dataStreams.size();i++)
    {
        if (_dataStreams[i]->getId()==id)
        {
            delete _dataStreams[i];
            _dataStreams.erase(_dataStreams.begin()+i);
            retVal=true;
            break;
        }
    }

    // Update dependent curves:
    std::vector<int> allStreamIds;
    getAllStreamIds(allStreamIds);
    for (size_t i=0;i<_curves.size();i++)
        _curves[i]->updateStreamIds(allStreamIds);

    _needsRefresh=true;
    return(retVal);
}

bool CGraph::removeGraphCurve(int id)
{
    bool retVal=false;
    for (size_t i=0;i<_curves.size();i++)
    {
        if (_curves[i]->getId()==id)
        {
            delete _curves[i];
            _curves.erase(_curves.begin()+i);
            retVal=true;
            break;
        }
    }
    _needsRefresh=true;
    return(retVal);
}

void CGraph::removeAllStreamsAndCurves()
{
    for (size_t i=0;i<_curves.size();i++)
        delete _curves[i];
    _curves.clear();
    for (size_t i=0;i<_dataStreams.size();i++)
        delete _dataStreams[i];
    _dataStreams.clear();
    _needsRefresh=true;
}

void CGraph::getAllStreamIds(std::vector<int>& allStreamIds)
{
    allStreamIds.clear();
    for (size_t i=0;i<_dataStreams.size();i++)
        allStreamIds.push_back(_dataStreams[i]->getId());
}

int CGraph::addNewGraphData(CGraphData_old* graphData)
{   // Returns the graphData identifier
    // We don't care if already present, because we could scale one but not the other
    // for instance.
    _needsRefresh=true;
    std::string theName=graphData->getName();
    while (getGraphData(theName)!=nullptr)
        theName=tt::generateNewName_noHash(theName.c_str());
    graphData->setName(theName);
    int id=0;
    while (getGraphData(id)!=nullptr)
        id++;
    graphData->setIdentifier(id);
    dataStreams_old.push_back(graphData);
    graphData->resetData(bufferSize);
    return(graphData->getIdentifier());
}
void CGraph::removeGraphData(int id)
{
    _needsRefresh=true;
    announceGraphDataObjectWillBeDestroyed(id);
    for (int i=0;i<int(dataStreams_old.size());i++)
    {
        if (dataStreams_old[i]->getIdentifier()==id)
        {
            delete dataStreams_old[i];
            dataStreams_old.erase(dataStreams_old.begin()+i);
            break;
        }
    }
}
void CGraph::remove2DPartners(int id)
{
    _needsRefresh=true;
    for (int i=0;i<int(curves2d_old.size());i++)
    {
        if (curves2d_old[i]->getIdentifier()==id)
        {
            delete curves2d_old[i];
            curves2d_old.erase(curves2d_old.begin()+i);
            break;
        }
    }
}
void CGraph::remove3DPartners(int id)
{
    _needsRefresh=true;
    for (int i=0;i<int(curves3d_old.size());i++)
    {
        if (curves3d_old[i]->getIdentifier()==id)
        {
            delete curves3d_old[i];
            curves3d_old.erase(curves3d_old.begin()+i);
            break;
        }
    }
}
void CGraph::add2DPartners(CGraphDataComb_old* it)
{
    std::string tmp=it->getName();
    while (getGraphData2D(tmp)!=nullptr)
        tmp=tt::generateNewName_noHash(tmp.c_str());
    it->setName(tmp);
    int id=0;
    while (getGraphData2D(id)!=nullptr)
        id++;
    it->setIdentifier(id);
    curves2d_old.push_back(it);
}
void CGraph::add3DPartners(CGraphDataComb_old* it)
{
    std::string tmp=it->getName();
    while (getGraphData3D(tmp)!=nullptr)
        tmp=tt::generateNewName_noHash(tmp.c_str());
    it->setName(tmp);
    int id=0;
    while (getGraphData3D(id)!=nullptr)
        id++;
    it->setIdentifier(id);
    curves3d_old.push_back(it);
}

void CGraph::addNextPoint(floatDouble time)
{
    int nextEntryPosition;
    if (numberOfPoints>=bufferSize)
    { // We reached the maximum of points
        if (!cyclic)
            return; // The buffer is not cyclic, we leave here
        else
        { // The buffer is cyclic
            nextEntryPosition=startingPoint;
            startingPoint++;
            if (startingPoint>=bufferSize)
                startingPoint=startingPoint-bufferSize;
        }
    }
    else
    {
        nextEntryPosition=startingPoint+numberOfPoints;
        if (nextEntryPosition>=bufferSize)
            nextEntryPosition=nextEntryPosition-bufferSize;
        numberOfPoints++;
    }
    times[nextEntryPosition]=time;
    for (size_t i=0;i<_dataStreams.size();i++)
        _dataStreams[i]->insertNextValue(nextEntryPosition,nextEntryPosition==startingPoint,times);

    // Old:
    C7Vector m(getCumulativeTransformation());
    for (size_t i=0;i<dataStreams_old.size();i++)
    {
        bool cyclic;
        floatDouble range;
        CGraphingRoutines_old::getCyclicAndRangeValues(dataStreams_old[i],cyclic,range);
        dataStreams_old[i]->setValue(&m,nextEntryPosition,nextEntryPosition==startingPoint,cyclic,range,times);
        // Here we have to handle a special case: GRAPH_VARIOUS_TIME
        if (dataStreams_old[i]->getDataType()==GRAPH_NOOBJECT_TIME)
            dataStreams_old[i]->setValueDirect(nextEntryPosition,time,nextEntryPosition==startingPoint,cyclic,range,times);
        dataStreams_old[i]->clearUserData();
    }
}
bool CGraph::getAbsIndexOfPosition(int pos,int& absIndex) const
{   // Use this function to loop through all absolute indices until it returns false.
    if (pos>=numberOfPoints)
        return(false);
    absIndex=startingPoint+pos;
    if (absIndex>=bufferSize)
        absIndex=absIndex-bufferSize;
    return(true);
}
int CGraph::getNumberOfPoints() const
{
    return(numberOfPoints);
}

void CGraph::scaleObject(floatDouble scalingFactor)
{
    setGraphSize(_graphSize*scalingFactor);
    CSceneObject::scaleObject(scalingFactor);
}

void CGraph::scaleObjectNonIsometrically(floatDouble x,floatDouble y,floatDouble z)
{
    scaleObject(cbrt(x*y*z));
}

void CGraph::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CGraph::addSpecializedObjectEventData(CInterfaceStackTable* data) const
{
    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("graph",subC);
    data=subC;

    data->appendMapObject_stringFloat("size",_graphSize);

    // todo
}

CSceneObject* CGraph::copyYourself()
{   
    CGraph* newGraph=(CGraph*)CSceneObject::copyYourself();
    color.copyYourselfInto(&newGraph->color);
    newGraph->_graphSize=_graphSize;
    newGraph->setCyclic(getCyclic());
    newGraph->setBufferSize(getBufferSize());
    newGraph->numberOfPoints=numberOfPoints;
    newGraph->startingPoint=startingPoint;
    newGraph->xYZPlanesDisplay=xYZPlanesDisplay;
    newGraph->times.assign(times.begin(),times.end());

    for (size_t i=0;i<_dataStreams.size();i++)
        newGraph->_dataStreams.push_back(_dataStreams[i]->copyYourself());

    // Old:
    // ----------------
    newGraph->dataStreams_old.clear();
    for (int i=0;i<int(dataStreams_old.size());i++)
        newGraph->dataStreams_old.push_back(dataStreams_old[i]->copyYourself());
    newGraph->curves3d_old.clear();
    for (int i=0;i<int(curves3d_old.size());i++)
        newGraph->curves3d_old.push_back(curves3d_old[i]->copyYourself());
    newGraph->curves2d_old.clear();
    for (int i=0;i<int(curves2d_old.size());i++)
        newGraph->curves2d_old.push_back(curves2d_old[i]->copyYourself());
    for (int i=0;i<int(staticStreamsAndCurves_old.size());i++)
        newGraph->staticStreamsAndCurves_old.push_back(staticStreamsAndCurves_old[i]->copyYourself());
    // ----------------

    newGraph->backgroundColor[0]=backgroundColor[0];
    newGraph->backgroundColor[1]=backgroundColor[1];
    newGraph->backgroundColor[2]=backgroundColor[2];
    newGraph->textColor[0]=textColor[0];
    newGraph->textColor[1]=textColor[1];
    newGraph->textColor[2]=textColor[2];
    newGraph->graphGrid=graphGrid;
    newGraph->graphValues=graphValues;
    newGraph->setExplicitHandling(getExplicitHandling());

    newGraph->_initialValuesInitialized=_initialValuesInitialized;
    newGraph->_initialExplicitHandling=_initialExplicitHandling;

    return(newGraph);
}

int CGraph::getTrackingValueIndex() const
{
    return(trackingValueIndex);
}

void CGraph::removeAllStatics()
{
    size_t i=0;
    while (i<_dataStreams.size())
    {
        CGraphDataStream* str=_dataStreams[i];
        if (str->getIsStatic())
            removeGraphDataStream(str->getId());
        else
            i++;
    }
    i=0;
    while (i<_curves.size())
    {
        CGraphCurve* curve=_curves[i];
        if (curve->getIsStatic())
            removeGraphCurve(curve->getId());
        else
            i++;
    }

    // Old:
    for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
        delete staticStreamsAndCurves_old[i];
    staticStreamsAndCurves_old.clear();
}

void CGraph::makeCurveStatic(int curveIndex,int dimensionIndex)
{
    _needsRefresh=true;
    if (dimensionIndex==0)
    { // time graph curves:
        if (curveIndex<int(dataStreams_old.size()))
        {
            std::vector<floatDouble> timeValues;
            std::vector<floatDouble> staticValues;
            CGraphData_old* it=dataStreams_old[curveIndex];
            int pos=0;
            int absIndex;
            floatDouble yVal,xVal;
            bool cyclic;
            floatDouble range;
            CGraphingRoutines_old::getCyclicAndRangeValues(it,cyclic,range);
            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                xVal=times[absIndex];
                bool dataIsValid=getData(it,absIndex,yVal,cyclic,range,true);
                if (dataIsValid)
                {
                    timeValues.push_back(xVal);
                    staticValues.push_back(yVal);
                }
            }
            if (timeValues.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(0,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(0,&timeValues,&staticValues,nullptr);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->ambientColor[0];
                curve->ambientColor[1]=it->ambientColor[1];
                curve->ambientColor[2]=it->ambientColor[2];
            }
        }
    }
    if (dimensionIndex==1)
    { // x/y graph curves:
        if (curveIndex<int(curves2d_old.size()))
        {
            std::vector<floatDouble> values0;
            std::vector<floatDouble> values1;
            CGraphDataComb_old* it=curves2d_old[curveIndex];
            int pos=0;
            int absIndex;
            floatDouble val[3];
            CGraphData_old* number1=getGraphData(it->data[0]);
            CGraphData_old* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            floatDouble range1,range2;
            if (number1!=nullptr)  
                CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)  
                CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);

            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                bool dataIsValid=true;
                if (number1!=nullptr)
                {
                    if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number2!=nullptr)
                {
                    if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (dataIsValid)
                {
                    values0.push_back(val[0]);
                    values1.push_back(val[1]);
                }
            }
            if (values0.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(1,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(1,&values0,&values1,nullptr);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.getColorsPtr()[0];
                curve->ambientColor[1]=it->curveColor.getColorsPtr()[1];
                curve->ambientColor[2]=it->curveColor.getColorsPtr()[2];
                curve->emissiveColor[0]=it->curveColor.getColorsPtr()[9];
                curve->emissiveColor[1]=it->curveColor.getColorsPtr()[10];
                curve->emissiveColor[2]=it->curveColor.getColorsPtr()[11];
            }
        }
    }
    if (dimensionIndex==2)
    { // 3D graph curves:
        if (curveIndex<int(curves3d_old.size()))
        {
            std::vector<floatDouble> values0;
            std::vector<floatDouble> values1;
            std::vector<floatDouble> values2;
            CGraphDataComb_old* it=curves3d_old[curveIndex];
            int pos=0;
            int absIndex;
            floatDouble val[3];
            CGraphData_old* number1=getGraphData(it->data[0]);
            CGraphData_old* number2=getGraphData(it->data[1]);
            CGraphData_old* number3=getGraphData(it->data[2]);
            bool cyclic1,cyclic2,cyclic3;
            floatDouble range1,range2,range3;
            if (number1!=nullptr)  
                CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)  
                CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);
            if (number3!=nullptr)  
                CGraphingRoutines_old::getCyclicAndRangeValues(number3,cyclic3,range3);

            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                bool dataIsValid=true;
                if (number1!=nullptr)
                {
                    if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number2!=nullptr)
                {
                    if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number3!=nullptr)
                {
                    if(!getData(number3,absIndex,val[2],cyclic3,range3,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (dataIsValid)
                {
                    C3Vector pp(val[0],val[1],val[2]);
                    values0.push_back(val[0]);
                    values1.push_back(val[1]);
                    values2.push_back(val[2]);
                }
            }
            if (values0.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(2,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(2,&values0,&values1,&values2);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.getColorsPtr()[0];
                curve->ambientColor[1]=it->curveColor.getColorsPtr()[1];
                curve->ambientColor[2]=it->curveColor.getColorsPtr()[2];
                curve->emissiveColor[0]=it->curveColor.getColorsPtr()[9];
                curve->emissiveColor[1]=it->curveColor.getColorsPtr()[10];
                curve->emissiveColor[2]=it->curveColor.getColorsPtr()[11];
                curve->setCurveWidth(it->get3DCurveWidth());
                curve->setRelativeToWorld(curves3d_old[curveIndex]->getCurveRelativeToWorld());
            }
        }
    }
}

CStaticGraphCurve_old* CGraph::getStaticCurveFromName(int type,const char* name)
{
    for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
    {
        CStaticGraphCurve_old* it=staticStreamsAndCurves_old[i];
        if (it->getCurveType()==type)
        {
            if (it->getName().compare(name)==0)
                return(it);
        }
    }
    return(nullptr);
}

void CGraph::setGraphSize(floatDouble theNewSize)
{
    tt::limitValue(0.001f,10.0f,theNewSize);
    if (_graphSize!=theNewSize)
    {
        _graphSize=theNewSize;
        computeBoundingBox();
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="size";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringFloat(cmd,_graphSize);
            App::worldContainer->pushEvent(event);
        }
    }
}

floatDouble CGraph::getGraphSize() const
{
    return(_graphSize);
}

bool CGraph::getNeedsRefresh()
{
    bool retVal=_needsRefresh;
    _needsRefresh=false;
    return(retVal);
}

void CGraph::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CGraph::getExplicitHandling() const
{
    return(_explicitHandling);
}

CColorObject* CGraph::getColor()
{
    return(&color);
}


void CGraph::announceGraphDataObjectWillBeDestroyed(int graphDataID)
{
// We first erase all the data entries from curves3d_old and twoDPartners
// of graph graphID which contain graphDataID.
    // We first set all invalid entries to -1:  
    for (int j=0;j<3;j++)
    {
        for (int i=0;i<int(curves3d_old.size());i++)
        {
            if (curves3d_old[i]->data[j]==graphDataID)
                curves3d_old[i]->data[j]=-1;
        }
        for (int i=0;i<int(curves2d_old.size());i++)
        {
            if (curves2d_old[i]->data[j]==graphDataID)
                curves2d_old[i]->data[j]=-1;
        }
    }
    // Now we remove all curves3d_old and twoDPartners which have 0 valid
    // entries:
    int i=0;
    while (i<int(curves3d_old.size()))
    {
        int objID=curves3d_old[i]->getIdentifier();
        if ( (curves3d_old[i]->data[0]==-1)&&
            (curves3d_old[i]->data[1]==-1)&&
            (curves3d_old[i]->data[2]==-1) )
        {
            remove3DPartners(objID);
            i=0; // We start at 0 again: ordering may have changed
        }
        else
            i++;
    }
    i=0;
    while (i<int(curves2d_old.size()))
    {
        int objID=curves2d_old[i]->getIdentifier();
        if ( (curves2d_old[i]->data[0]==-1)&&
            (curves2d_old[i]->data[1]==-1)&&
            (curves2d_old[i]->data[2]==-1) )
        {
            remove2DPartners(objID);
            i=0; // We start at 0 again: ordering may have changed
        }
        else
            i++;
    }
}

void CGraph::exportGraphData(VArchive &ar)
{ // STATIC streams are not exported!! (they might have a different time interval, etc.)

    // The graph name:
    std::string txt(getObjectAlias_shortPath());
    txt+="\n";

    if (numberOfPoints>0)
    {
        // The labels:
        txt+="Time (s)";
        for (size_t i=0;i<_dataStreams.size();i++)
        {
            txt+=",";
            std::string label;
            _dataStreams[i]->getExportValue(0,0,nullptr,&label);
            txt+=label;
        }
        // The labels of the old curves:
        for (size_t i=0;i<dataStreams_old.size();i++)
        {
            txt+=",";
            CGraphData_old* gr=dataStreams_old[i];
            txt+=gr->getName()+" ("+CGraphingRoutines_old::getDataUnit(gr)+")";
        }
        txt+="\n";

        // The values:
        int pos=0;
        int absIndex;
        while (getAbsIndexOfPosition(pos,absIndex))
        {
            floatDouble time=times[absIndex];
            txt+=tt::getFString(false,time,5);
            for (size_t i=0;i<_dataStreams.size();i++)
            {
                txt+=",";
                floatDouble val;
                if (_dataStreams[i]->getExportValue(startingPoint,pos,&val,nullptr))
                    txt+=tt::getEString(true,val,4);
                else
                    txt+="Null";
            }
            // The values of the old curves:
            for (size_t i=0;i<dataStreams_old.size();i++)
            {
                txt+=",";
                CGraphData_old* gr=dataStreams_old[i];

                bool cyclic;
                floatDouble range;
                CGraphingRoutines_old::getCyclicAndRangeValues(gr,cyclic,range);
                floatDouble val;
                bool dataIsValid=getData(gr,absIndex,val,cyclic,range,true);
                if (dataIsValid)
                    txt+=tt::getEString(true,val,4);
                else
                    txt+="Null";
            }
            txt+="\n";
            pos++;
        }
    }
    txt+="\n";
    ar.writeString(txt);
}

bool CGraph::getGraphCurveData(int graphType,int index,std::string& label,std::vector<floatDouble>& xVals,std::vector<floatDouble>& yVals,int& curveType,floatDouble col[3],floatDouble minMax[6],int& curveId,int& curveWidth) const
{
    if (graphType==0)
    { // time curves (dyn then static curves)
        for (size_t i=0;i<_dataStreams.size();i++)
        {
            if (_dataStreams[i]->getCurveData(false,&index,startingPoint,numberOfPoints,times,&label,xVals,yVals,&curveType,col,minMax))
            {
                curveId=_dataStreams[i]->getId();
                return(true);
            }
        }
        for (size_t i=0;i<_dataStreams.size();i++)
        {
            if (_dataStreams[i]->getCurveData(true,&index,startingPoint,numberOfPoints,times,&label,xVals,yVals,&curveType,col,minMax))
            {
                curveId=_dataStreams[i]->getId();
                return(true);
            }
        }
    }
    if (graphType==1)
    { // x/y curves (dyn then static curves)
        for (size_t i=0;i<_curves.size();i++)
        {
            CGraphDataStream* streams[3];
            getGraphDataStreamsFromIds(_curves[i]->getStreamIdsPtr(),streams);
            if (_curves[i]->getCurveData_xy(streams,&index,bufferSize,startingPoint,numberOfPoints,&label,xVals,yVals,&curveType,col,minMax))
            {
                curveId=_curves[i]->getId();
                return(true);
            }
        }
        for (size_t i=0;i<_curves.size();i++)
        {
            if (_curves[i]->getCurveData_xy(nullptr,&index,bufferSize,startingPoint,numberOfPoints,&label,xVals,yVals,&curveType,col,minMax))
            {
                curveId=_curves[i]->getId();
                return(true);
            }
        }
    }
    if (graphType==2)
    { // xyz curves (dyn then static curves)
        for (size_t i=0;i<_curves.size();i++)
        {
            CGraphDataStream* streams[3];
            getGraphDataStreamsFromIds(_curves[i]->getStreamIdsPtr(),streams);
            if (_curves[i]->getCurveData_xyz(streams,&index,bufferSize,startingPoint,numberOfPoints,&label,xVals,&curveType,col,minMax,&curveWidth))
            {
                curveId=_curves[i]->getId();
                return(true);
            }
        }
        for (size_t i=0;i<_curves.size();i++)
        {
            if (_curves[i]->getCurveData_xyz(nullptr,&index,bufferSize,startingPoint,numberOfPoints,&label,xVals,&curveType,col,minMax,&curveWidth))
            {
                curveId=_curves[i]->getId();
                return(true);
            }
        }
    }

    curveId=-1;
    // Old, Gui-created curves:
    if (graphType==0)
    { // time
        for (size_t ind=0;ind<dataStreams_old.size();ind++)
        {
            CGraphData_old* gr=dataStreams_old[ind];
            if (gr->getVisible())
            {
                if (index==0)
                {
                    label=gr->getName()+" ("+CGraphingRoutines_old::getDataUnit(gr)+")";
                    if (gr->getLinkPoints())
                        curveType=0;
                    else
                        curveType=1;
                    if (!gr->getLabel())
                        curveType+=4;
                    col[0]=gr->ambientColor[0];
                    col[1]=gr->ambientColor[1];
                    col[2]=gr->ambientColor[2];

                    int pos=0;
                    int absIndex;
                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        floatDouble xVal=times[absIndex];
                        floatDouble yVal;
                        bool cyclic;
                        floatDouble range;
                        CGraphingRoutines_old::getCyclicAndRangeValues(gr,cyclic,range);
                        bool dataIsValid=getData(gr,absIndex,yVal,cyclic,range,true);
                        if (dataIsValid)
                        {
                            xVals.push_back(xVal);
                            yVals.push_back(yVal);
                            if (xVals.size()==1)
                            {
                                minMax[0]=xVal;
                                minMax[1]=xVal;
                                minMax[2]=yVal;
                                minMax[3]=yVal;
                            }
                            else
                            {
                                if (xVal<minMax[0])
                                    minMax[0]=xVal;
                                if (xVal>minMax[1])
                                    minMax[1]=xVal;
                                if (yVal<minMax[2])
                                    minMax[2]=yVal;
                                if (yVal>minMax[3])
                                    minMax[3]=yVal;
                            }
                        }
                    }
                    return(true);
                }
                index--;
            }
        }
        for (size_t ind=0;ind<staticStreamsAndCurves_old.size();ind++)
        {
            CStaticGraphCurve_old* gr=staticStreamsAndCurves_old[ind];
            if (gr->getCurveType()==0)
            { // time
                if (index==0)
                {
                    label=gr->getName()+" (STATIC)";
                    if (gr->getLinkPoints())
                        curveType=2;
                    else
                        curveType=2+1;
                    if (!gr->getLabel())
                        curveType+=4;
                    col[0]=gr->ambientColor[0];
                    col[1]=gr->ambientColor[1];
                    col[2]=gr->ambientColor[2];
                    for (size_t i=0;i<gr->values.size()/2;i++)
                    {
                        xVals.push_back(gr->values[2*i+0]);
                        yVals.push_back(gr->values[2*i+1]);
                        if (xVals.size()==1)
                        {
                            minMax[0]=gr->values[2*i+0];
                            minMax[1]=gr->values[2*i+0];
                            minMax[2]=gr->values[2*i+1];
                            minMax[3]=gr->values[2*i+1];
                        }
                        else
                        {
                            if (gr->values[2*i+0]<minMax[0])
                                minMax[0]=gr->values[2*i+0];
                            if (gr->values[2*i+0]>minMax[1])
                                minMax[1]=gr->values[2*i+0];
                            if (gr->values[2*i+1]<minMax[2])
                                minMax[2]=gr->values[2*i+1];
                            if (gr->values[2*i+1]>minMax[3])
                                minMax[3]=gr->values[2*i+1];
                        }
                    }
                    return(true);
                }
                index--;
            }
        }
        return(false);
    }
    if (graphType==1)
    { // x/y
        for (size_t ind=0;ind<curves2d_old.size();ind++)
        {
            CGraphDataComb_old* it=curves2d_old[ind];
            if (it->getVisible())
            {
                if (index==0)
                {
                    col[0]=it->curveColor.getColorsPtr()[0];
                    col[1]=it->curveColor.getColorsPtr()[1];
                    col[2]=it->curveColor.getColorsPtr()[2];
                    int pos=0;
                    int absIndex;
                    floatDouble val[3];
                    CGraphData_old* number1=getGraphData(it->data[0]);
                    CGraphData_old* number2=getGraphData(it->data[1]);
                    bool cyclic1,cyclic2;
                    floatDouble range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);
                    label=it->getName()+" (x: ";
                    if (number1!=nullptr)
                        label+=CGraphingRoutines_old::getDataUnit(number1)+") (y: ";
                    else
                        label+="0.0) (y: ";
                    if (number2!=nullptr)
                        label+=CGraphingRoutines_old::getDataUnit(number2)+")";
                    else
                        label+="0.0)";
                    if (it->getLinkPoints())
                        curveType=0;
                    else
                        curveType=1;
                    if (!it->getLabel())
                        curveType+=4;

                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        bool dataIsValid=true;
                        if (number1!=nullptr)
                        {
                            if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                                dataIsValid=false;
                        }
                        else
                            val[0]=0.0;
                        if (number2!=nullptr)
                        {
                            if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                                dataIsValid=false;
                        }
                        else
                            val[1]=0.0;
                        if (dataIsValid)
                        {
                            xVals.push_back(val[0]);
                            yVals.push_back(val[1]);
                            if (xVals.size()==1)
                            {
                                minMax[0]=val[0];
                                minMax[1]=val[0];
                                minMax[2]=val[1];
                                minMax[3]=val[1];
                            }
                            else
                            {
                                if (val[0]<minMax[0])
                                    minMax[0]=val[0];
                                if (val[0]>minMax[1])
                                    minMax[1]=val[0];
                                if (val[1]<minMax[2])
                                    minMax[2]=val[1];
                                if (val[1]>minMax[3])
                                    minMax[3]=val[1];
                            }
                        }
                    }
                    return(true);
                }
                index--;
            }
        }

        for (size_t ind=0;ind<staticStreamsAndCurves_old.size();ind++)
        {
            CStaticGraphCurve_old* gr=staticStreamsAndCurves_old[ind];
            if (gr->getCurveType()==1)
            { // x/y
                if (index==0)
                {
                    label=gr->getName()+" (STATIC)";
                    if (gr->getLinkPoints())
                        curveType=2;
                    else
                        curveType=2+1;
                    if (!gr->getLabel())
                        curveType+=4;
                    col[0]=gr->ambientColor[0];
                    col[1]=gr->ambientColor[1];
                    col[2]=gr->ambientColor[2];
                    for (size_t i=0;i<gr->values.size()/2;i++)
                    {
                        xVals.push_back(gr->values[2*i+0]);
                        yVals.push_back(gr->values[2*i+1]);
                        if (xVals.size()==1)
                        {
                            minMax[0]=gr->values[2*i+0];
                            minMax[1]=gr->values[2*i+0];
                            minMax[2]=gr->values[2*i+1];
                            minMax[3]=gr->values[2*i+1];
                        }
                        else
                        {
                            if (gr->values[2*i+0]<minMax[0])
                                minMax[0]=gr->values[2*i+0];
                            if (gr->values[2*i+0]>minMax[1])
                                minMax[1]=gr->values[2*i+0];
                            if (gr->values[2*i+1]<minMax[2])
                                minMax[2]=gr->values[2*i+1];
                            if (gr->values[2*i+1]>minMax[3])
                                minMax[3]=gr->values[2*i+1];
                        }
                    }
                    return(true);
                }
                index--;
            }
        }
        return(false);
    }
    return(false);
}

void CGraph::curveToClipboard(int graphType,const char* curveName) const
{
    std::vector<floatDouble> xVals;
    std::vector<floatDouble> yVals;
    if (graphType==0)
    { // time curves, non-static
        CGraphDataStream* stream=getGraphDataStream(curveName,false);
        if (stream!=nullptr)
            stream->getCurveData(false,nullptr,startingPoint,numberOfPoints,times,nullptr,xVals,yVals,nullptr,nullptr,nullptr);
    }
    if (graphType==3)
    { // time curves, static
        CGraphDataStream* stream=getGraphDataStream((std::string(curveName)+" [STATIC]").c_str(),true);
        if (stream!=nullptr)
            stream->getCurveData(true,nullptr,startingPoint,numberOfPoints,times,nullptr,xVals,yVals,nullptr,nullptr,nullptr);
    }
    if (graphType==1)
    { // x/y curves, non-static
        CGraphCurve* curve=getGraphCurve(curveName,false);
        if (curve!=nullptr)
        {
            CGraphDataStream* streams[3];
            getGraphDataStreamsFromIds(curve->getStreamIdsPtr(),streams);
            curve->getCurveData_xy(streams,nullptr,bufferSize,startingPoint,numberOfPoints,nullptr,xVals,yVals,nullptr,nullptr,nullptr);
        }
    }
    if (graphType==4)
    { // x/y curves, static
        CGraphCurve* curve=getGraphCurve((std::string(curveName)+" [STATIC]").c_str(),true);
        if (curve!=nullptr)
            curve->getCurveData_xy(nullptr,nullptr,bufferSize,startingPoint,numberOfPoints,nullptr,xVals,yVals,nullptr,nullptr,nullptr);
    }
    if (xVals.size()>0)
    {
        std::string txt;
        for (size_t i=0;i<xVals.size();i++)
        {
            txt+=boost::lexical_cast<std::string>(xVals[i])+char(9);
            txt+=boost::lexical_cast<std::string>(yVals[i])+char(13);
            txt+=char(10);
        }
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=COPY_TEXT_TO_CLIPBOARD_UITHREADCMD;
        cmdIn.stringParams.push_back(txt);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        return;
    }


    // Old:
    std::string txt;
    if (graphType==0)
    { // time graph curves:
        CGraphData_old* it=nullptr;
        for (size_t ind=0;ind<dataStreams_old.size();ind++)
        {
            if (dataStreams_old[ind]->getName().compare(curveName)==0)
            {
                it=dataStreams_old[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            int pos=0;
            int absIndex;
            floatDouble yVal,xVal;
            bool cyclic;
            floatDouble range;
            CGraphingRoutines_old::getCyclicAndRangeValues(it,cyclic,range);
            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                xVal=times[absIndex];
                bool dataIsValid=getData(it,absIndex,yVal,cyclic,range,true);
                if (dataIsValid)
                {
                    txt+=boost::lexical_cast<std::string>(xVal)+char(9);
                    txt+=boost::lexical_cast<std::string>(yVal)+char(13);
                    txt+=char(10);
                }
            }
        }
    }
    if (graphType==1)
    { // x/y graph curves:
        CGraphDataComb_old* it=nullptr;
        for (size_t ind=0;ind<curves2d_old.size();ind++)
        {
            if (curves2d_old[ind]->getName().compare(curveName)==0)
            {
                it=curves2d_old[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            int pos=0;
            int absIndex;
            floatDouble val[3];
            CGraphData_old* number1=getGraphData(it->data[0]);
            CGraphData_old* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            floatDouble range1,range2;
            if (number1!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);

            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                bool dataIsValid=true;
                if (number1!=nullptr)
                {
                    if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number2!=nullptr)
                {
                    if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (dataIsValid)
                {
                    txt+=boost::lexical_cast<std::string>(val[0])+char(9);
                    txt+=boost::lexical_cast<std::string>(val[1])+char(13);
                    txt+=char(10);
                }
            }
        }
    }
    if ( (graphType==3)||(graphType==4) )
    { // static time and xy graph curves:
        for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
        {
            CStaticGraphCurve_old* it=staticStreamsAndCurves_old[i];
            if ( (it->getCurveType()==graphType-3)&&(it->getName().compare(curveName)==0) )
            {
                for (size_t j=0;j<it->values.size()/2;j++)
                {
                    txt+=boost::lexical_cast<std::string>(it->values[2*j+0])+char(9);
                    txt+=boost::lexical_cast<std::string>(it->values[2*j+1])+char(13);
                    txt+=char(10);
                }
            }
        }
    }
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=COPY_TEXT_TO_CLIPBOARD_UITHREADCMD;
    cmdIn.stringParams.push_back(txt);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
}

int CGraph::duplicateCurveToStatic(int curveId,const char* curveName)
{
    int retVal=-1;
    CGraphDataStream* stream=getGraphDataStream(curveId);
    if ( (stream!=nullptr)&&(!stream->getIsStatic()) )
    {
        std::string nm(curveName);
        while (getGraphDataStream((nm+" [STATIC]").c_str(),true)!=nullptr)
            nm=tt::generateNewName_noHash(nm.c_str());
        CGraphDataStream* staticStream=stream->copyYourself();
        staticStream->setStreamName((nm+" [STATIC]").c_str());
        staticStream->makeStatic(startingPoint,numberOfPoints,times);
        retVal=addOrUpdateDataStream(staticStream);
    }
    else
    {
        CGraphCurve* curve=getGraphCurve(curveId);
        if ( (curve!=nullptr)&&(!curve->getIsStatic()) )
        {
            std::string nm(curveName);
            while (getGraphCurve((nm+" [STATIC]").c_str(),true)!=nullptr)
                nm=tt::generateNewName_noHash(nm.c_str());
            CGraphCurve* staticCurve=curve->copyYourself();
            staticCurve->setCurveName((nm+" [STATIC]").c_str());
            CGraphDataStream* streams[3];
            getGraphDataStreamsFromIds(staticCurve->getStreamIdsPtr(),streams);
            staticCurve->makeStatic(streams,bufferSize,startingPoint,numberOfPoints);
            retVal=addOrUpdateCurve(staticCurve);
        }
    }
    return(retVal);
}

void CGraph::curveToStatic(int graphType,const char* curveName)
{
    if (graphType==0)
    { // time, non-static
        CGraphDataStream* stream=getGraphDataStream(curveName,false);
        if (stream!=nullptr)
            duplicateCurveToStatic(stream->getId(),stream->getStreamName().c_str());
    }
    if (graphType==1)
    { // xy, non-static
        CGraphCurve* curve=getGraphCurve(curveName,false);
        if (curve!=nullptr)
            duplicateCurveToStatic(curve->getId(),curve->getCurveName().c_str());
    }

    // Old:
    if (graphType==0)
    { // time graph curves:
        CGraphData_old* it=nullptr;
        for (size_t ind=0;ind<dataStreams_old.size();ind++)
        {
            if (dataStreams_old[ind]->getName().compare(curveName)==0)
            {
                it=dataStreams_old[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<floatDouble> timeValues;
            std::vector<floatDouble> staticValues;
            int pos=0;
            int absIndex;
            floatDouble yVal,xVal;
            bool cyclic;
            floatDouble range;
            CGraphingRoutines_old::getCyclicAndRangeValues(it,cyclic,range);
            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                xVal=times[absIndex];
                bool dataIsValid=getData(it,absIndex,yVal,cyclic,range,true);
                if (dataIsValid)
                {
                    timeValues.push_back(xVal);
                    staticValues.push_back(yVal);
                }
            }
            if (timeValues.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(0,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(0,&timeValues,&staticValues,nullptr);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->ambientColor[0];
                curve->ambientColor[1]=it->ambientColor[1];
                curve->ambientColor[2]=it->ambientColor[2];
            }
        }
    }
    if (graphType==1)
    { // x/y graph curves:
        CGraphDataComb_old* it=nullptr;
        for (size_t ind=0;ind<curves2d_old.size();ind++)
        {
            if (curves2d_old[ind]->getName().compare(curveName)==0)
            {
                it=curves2d_old[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<floatDouble> values0;
            std::vector<floatDouble> values1;
            int pos=0;
            int absIndex;
            floatDouble val[3];
            CGraphData_old* number1=getGraphData(it->data[0]);
            CGraphData_old* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            floatDouble range1,range2;
            if (number1!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);

            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                bool dataIsValid=true;
                if (number1!=nullptr)
                {
                    if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number2!=nullptr)
                {
                    if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (dataIsValid)
                {
                    values0.push_back(val[0]);
                    values1.push_back(val[1]);
                }
            }
            if (values0.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(1,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(1,&values0,&values1,nullptr);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.getColorsPtr()[0];
                curve->ambientColor[1]=it->curveColor.getColorsPtr()[1];
                curve->ambientColor[2]=it->curveColor.getColorsPtr()[2];
                curve->emissiveColor[0]=it->curveColor.getColorsPtr()[9];
                curve->emissiveColor[1]=it->curveColor.getColorsPtr()[10];
                curve->emissiveColor[2]=it->curveColor.getColorsPtr()[11];
            }
        }
    }
    if (graphType==2)
    { // 3D graph curves:
        CGraphDataComb_old* it=nullptr;
        for (size_t ind=0;ind<curves3d_old.size();ind++)
        {
            if (curves3d_old[ind]->getName().compare(curveName)==0)
            {
                it=curves3d_old[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<floatDouble> values0;
            std::vector<floatDouble> values1;
            std::vector<floatDouble> values2;
            int pos=0;
            int absIndex;
            floatDouble val[3];
            CGraphData_old* number1=getGraphData(it->data[0]);
            CGraphData_old* number2=getGraphData(it->data[1]);
            CGraphData_old* number3=getGraphData(it->data[2]);
            bool cyclic1,cyclic2,cyclic3;
            floatDouble range1,range2,range3;
            if (number1!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);
            if (number3!=nullptr)
                CGraphingRoutines_old::getCyclicAndRangeValues(number3,cyclic3,range3);

            while (getAbsIndexOfPosition(pos++,absIndex))
            {
                bool dataIsValid=true;
                if (number1!=nullptr)
                {
                    if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number2!=nullptr)
                {
                    if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (number3!=nullptr)
                {
                    if(!getData(number3,absIndex,val[2],cyclic3,range3,true))
                        dataIsValid=false;
                }
                else
                    dataIsValid=false;
                if (dataIsValid)
                {
                    C3Vector pp(val[0],val[1],val[2]);
                    values0.push_back(val[0]);
                    values1.push_back(val[1]);
                    values2.push_back(val[2]);
                }
            }
            if (values0.size()!=0)
            {
                std::string nm(it->getName());
                while (getStaticCurveFromName(2,nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm.c_str());
                CStaticGraphCurve_old* curve=new CStaticGraphCurve_old(2,&values0,&values1,&values2);
                staticStreamsAndCurves_old.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.getColorsPtr()[0];
                curve->ambientColor[1]=it->curveColor.getColorsPtr()[1];
                curve->ambientColor[2]=it->curveColor.getColorsPtr()[2];
                curve->emissiveColor[0]=it->curveColor.getColorsPtr()[9];
                curve->emissiveColor[1]=it->curveColor.getColorsPtr()[10];
                curve->emissiveColor[2]=it->curveColor.getColorsPtr()[11];
                curve->setCurveWidth(it->get3DCurveWidth());
                curve->setRelativeToWorld(it->getCurveRelativeToWorld());
            }
        }
    }
}

void CGraph::removeStaticCurve(int graphType,const char* curveName)
{
    if (graphType==3)
    { // time, static
        CGraphDataStream* stream=getGraphDataStream((std::string(curveName)+" [STATIC]").c_str(),true);
        if (stream!=nullptr)
            removeGraphDataStream(stream->getId());
    }
    if (graphType==4)
    { // xy, static
        CGraphCurve* curve=getGraphCurve((std::string(curveName)+" [STATIC]").c_str(),true);
        if (curve!=nullptr)
            removeGraphCurve(curve->getId());
    }


    // Old:
    for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
    {
        if (staticStreamsAndCurves_old[i]->getCurveType()==graphType-3)
        {
            if (staticStreamsAndCurves_old[i]->getName().compare(curveName)==0)
            {
                delete staticStreamsAndCurves_old[i];
                staticStreamsAndCurves_old.erase(staticStreamsAndCurves_old.begin()+i);
                break;
            }
        }
    }
}

void CGraph::announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object,copyBuffer);
    // OLD:
    int i=0;
    while (i<int(dataStreams_old.size()))
    {
        CGraphData_old* gr=dataStreams_old[i];
        if (gr->announceObjectWillBeErased(object->getObjectHandle(),copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}

void CGraph::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer)
{
    CSceneObject::announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript,copyBuffer);
    // When a script that created a stream/curve gets removed, that stream/curve should also be removed:
    size_t i=0;
    while (i<_dataStreams.size())
    {
        if (_dataStreams[i]->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript,copyBuffer))
            removeGraphDataStream(_dataStreams[i]->getId());
        else
            i++;
    }
    i=0;
    while (i<_curves.size())
    {
        if (_curves[i]->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript,copyBuffer))
            removeGraphCurve(_curves[i]->getId());
        else
            i++;
    }
}

void CGraph::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CGraph::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(dataStreams_old.size()))
    {
        CGraphData_old* gr=dataStreams_old[i];
        if (gr->announceCollisionWillBeErased(collisionID,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}
void CGraph::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(dataStreams_old.size()))
    {
        CGraphData_old* gr=dataStreams_old[i];
        if (gr->announceDistanceWillBeErased(distanceID,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}
void CGraph::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(dataStreams_old.size()))
    {
        CGraphData_old* gr=dataStreams_old[i];
        if (gr->announceIkObjectWillBeErased(ikGroupID,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}

void CGraph::performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    for (size_t i=0;i<dataStreams_old.size();i++)
        dataStreams_old[i]->performObjectLoadingMapping(map);
}

void CGraph::performScriptLoadingMapping(const std::map<int,int>* map)
{
    CSceneObject::performScriptLoadingMapping(map);
    for (size_t i=0;i<_dataStreams.size();i++)
        _dataStreams[i]->performScriptLoadingMapping(map);
    for (size_t i=0;i<_curves.size();i++)
        _curves[i]->performScriptLoadingMapping(map);
}

void CGraph::performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CGraph::performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
    for (int i=0;i<int(dataStreams_old.size());i++)
        dataStreams_old[i]->performCollisionLoadingMapping(map);
}
void CGraph::performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
    for (int i=0;i<int(dataStreams_old.size());i++)
        dataStreams_old[i]->performDistanceLoadingMapping(map);
}
void CGraph::performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
    for (int i=0;i<int(dataStreams_old.size());i++)
        dataStreams_old[i]->performIkLoadingMapping(map);
}

void CGraph::performTextureObjectLoadingMapping(const std::map<int,int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CGraph::performDynMaterialObjectLoadingMapping(const std::map<int,int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CGraph::setBufferSize(int buffSize)
{
    tt::limitValue(10,100000000,buffSize);
    bufferSize=buffSize;
    resetGraph();
}
int CGraph::getBufferSize() const
{
    return(bufferSize);
}
void CGraph::setCyclic(bool isCyclic)
{
    if (isCyclic!=cyclic)
    {
        cyclic=isCyclic;
        resetGraph();
    }
}
bool CGraph::getCyclic() const
{
    return(cyclic);
}

bool CGraph::getData(const CGraphData_old* it,int pos,floatDouble& outputValue,bool cyclic,floatDouble range,bool doUnitConversion) const
{
    floatDouble cumulativeValue=0.0f;
    int cumulativeValueCount=0;
    int movingAverageCount=it->getMovingAverageCount();
    for (int i=0;i<movingAverageCount;i++)
    {
        floatDouble tmpVal;
        if (it->getValue(pos,tmpVal))
        {
            if (doUnitConversion)
                CGraphingRoutines_old::adjustDataToUserMetric(it,tmpVal,it->getDerivativeIntegralAndCumulative());
            cumulativeValue+=tmpVal;
            cumulativeValueCount++;
        }
        if ((i==0)&&(cumulativeValueCount==0))
            return(false); // the current point is not valid, we leave here
        if (i!=movingAverageCount-1)
        { // we are not finished looping!
            if (pos==startingPoint)
                return(false); // we reached the starting pos but didn't finish looping! (not enough values from current point!)
            pos--;
            if (pos<0)
                pos+=bufferSize;
        }
    }

    if (cumulativeValueCount>0)
        outputValue=cumulativeValue/floatDouble(cumulativeValueCount);
    return(cumulativeValueCount>0);
}

void CGraph::resetGraph()
{
    numberOfPoints=0;
    startingPoint=0;
    times.reserve(bufferSize);
    times.clear();
    for (int i=0;i<bufferSize;i++)
        times.push_back(0.0f);
    for (size_t i=0;i<_dataStreams.size();i++)
        _dataStreams[i]->reset(bufferSize);
    // Old:
    for (int i=0;i<int(dataStreams_old.size());i++)
        dataStreams_old[i]->resetData(bufferSize);
}

void CGraph::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialExplicitHandling=_explicitHandling;
    if (!_explicitHandling)
        resetGraph();
}

void CGraph::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CGraph::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            _explicitHandling=_initialExplicitHandling;
        }
    }
    CSceneObject::simulationEnded();
}

void CGraph::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
#ifdef TMPOPERATION
            ar.storeDataName("Ghg");
            ar.flt() << (floatFloat)_graphSize;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_hg");
            ar.dbl() << _graphSize;
            ar.flush();
#endif

            ar.storeDataName("Cl0"); // Colors
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

#ifdef TMPOPERATION
            ar.storeDataName("Cl1");
            ar.flt() << (floatFloat)backgroundColor[0] << (floatFloat)backgroundColor[1] << (floatFloat)backgroundColor[2];
            ar.flt() << (floatFloat)textColor[0] << (floatFloat)textColor[1] << (floatFloat)textColor[2];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_l1");
            ar.dbl() << backgroundColor[0] << backgroundColor[1] << backgroundColor[2];
            ar.dbl() << textColor[0] << textColor[1] << textColor[2];
            ar.flush();
#endif

            ar.storeDataName("Gbv");
            ar << bufferSize << numberOfPoints;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Gtd"); // Should always come after bufferSize!!!
            for (int i=0;i<numberOfPoints;i++)
            {
                int absIndex;
                getAbsIndexOfPosition(i,absIndex);
                ar.flt() << (floatFloat)times[absIndex];
            }
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_td"); // Should always come after bufferSize!!!
            for (int i=0;i<numberOfPoints;i++)
            {
                int absIndex;
                getAbsIndexOfPosition(i,absIndex);
                ar.dbl() << times[absIndex];
            }
            ar.flush();
#endif

            for (size_t i=0;i<_dataStreams.size();i++)
            {
                ar.storeDataName("Dst");
                ar.setCountingMode();
                _dataStreams[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                if (ar.setWritingMode())
                    _dataStreams[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
            }

            for (size_t i=0;i<_curves.size();i++)
            {
                ar.storeDataName("Cuv");
                ar.setCountingMode();
                _curves[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                if (ar.setWritingMode())
                    _curves[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
            }

            for (int i=0;i<int(dataStreams_old.size());i++)
            { // old
                ar.storeDataName("Ghd");
                ar.setCountingMode();
                dataStreams_old[i]->serialize(ar,this);
                if (ar.setWritingMode())
                    dataStreams_old[i]->serialize(ar,this);
            }
            for (int i=0;i<int(curves3d_old.size());i++)
            { // old
                ar.storeDataName("Gh3");
                ar.setCountingMode();
                curves3d_old[i]->serialize(ar);
                if (ar.setWritingMode())
                    curves3d_old[i]->serialize(ar);
            }
            for (int i=0;i<int(curves2d_old.size());i++)
            { // old
                ar.storeDataName("Gh2");
                ar.setCountingMode();
                curves2d_old[i]->serialize(ar);
                if (ar.setWritingMode())
                    curves2d_old[i]->serialize(ar);
            }
            for (int i=0;i<int(staticStreamsAndCurves_old.size());i++)
            { // old
                ar.storeDataName("Sta");
                ar.setCountingMode();
                staticStreamsAndCurves_old[i]->serialize(ar);
                if (ar.setWritingMode())
                    staticStreamsAndCurves_old[i]->serialize(ar);
            }

            ar.storeDataName("Gps");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,cyclic);
            SIM_SET_CLEAR_BIT(nothing,1,xYZPlanesDisplay);
            SIM_SET_CLEAR_BIT(nothing,2,graphGrid);
            SIM_SET_CLEAR_BIT(nothing,3,graphValues);
            SIM_SET_CLEAR_BIT(nothing,4,_explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            startingPoint=0;
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Ghg")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla;
                        ar.flt() >> bla;
                        _graphSize=(floatDouble)bla;
                    }
                    if (theName.compare("_hg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _graphSize;
                    }
                    if (theName.compare("Cl0")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Cl1")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla,bli,blo;
                        ar.flt() >> bla >> bli >> blo;
                        backgroundColor[0]=(floatDouble)bla;
                        backgroundColor[1]=(floatDouble)bli;
                        backgroundColor[2]=(floatDouble)blo;
                        ar.flt() >> bla >> bli >> blo;
                        textColor[0]=(floatDouble)bla;
                        textColor[1]=(floatDouble)bli;
                        textColor[2]=(floatDouble)blo;
                    }
                    if (theName.compare("_l1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> backgroundColor[0] >> backgroundColor[1] >> backgroundColor[2];
                        ar.dbl() >> textColor[0] >> textColor[1] >> textColor[2];
                    }
                    if (theName.compare("Gbv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> bufferSize >> numberOfPoints;
                    }
                    if (theName.compare("Gtd")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        times.clear();
                        for (int i=0;i<bufferSize;i++)
                            times.push_back(0.0);
                        for (int i=0;i<byteQuantity/int(sizeof(floatFloat));i++)
                        {
                            floatFloat aVal;
                            ar.flt() >> aVal;
                            times[i]=(floatDouble)aVal;
                        }
                    }
                    if (theName.compare("_td")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        times.clear();
                        for (int i=0;i<bufferSize;i++)
                            times.push_back(0.0);
                        for (int i=0;i<byteQuantity/int(sizeof(floatDouble));i++)
                        {
                            floatDouble aVal;
                            ar.dbl() >> aVal;
                            times[i]=aVal;
                        }
                    }

                    if (theName.compare("Dst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        CGraphDataStream* it=new CGraphDataStream();
                        it->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                        _dataStreams.push_back(it);
                    }

                    if (theName.compare("Cuv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        CGraphCurve* it=new CGraphCurve();
                        it->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                        _curves.push_back(it);
                    }

                    // Old:
                    //------------
                    if (theName.compare("Ghd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CGraphData_old* it=new CGraphData_old();
                        it->serialize(ar,this);
                        dataStreams_old.push_back(it);
                    }
                    if (theName.compare("Gh3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CGraphDataComb_old* it=new CGraphDataComb_old();
                        it->serialize(ar);
                        curves3d_old.push_back(it);
                    }
                    if (theName.compare("Gh2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CGraphDataComb_old* it=new CGraphDataComb_old();
                        it->serialize(ar);
                        curves2d_old.push_back(it);
                    }
                    if (theName.compare("Sta")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CStaticGraphCurve_old* it=new CStaticGraphCurve_old();
                        it->serialize(ar);
                        // Following 4 on 16/3/2017: duplicate names for static curves can cause problems
                        std::string nm(it->getName());
                        while (getStaticCurveFromName(it->getCurveType(),nm.c_str())!=nullptr)
                            nm=tt::generateNewName_noHash(nm.c_str());
                        it->setName(nm);
                        staticStreamsAndCurves_old.push_back(it);
                    }
                    //------------
                    if (theName=="Gps")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        cyclic=SIM_IS_BIT_SET(nothing,0);
                        xYZPlanesDisplay=SIM_IS_BIT_SET(nothing,1);
                        graphGrid=SIM_IS_BIT_SET(nothing,2);
                        graphValues=SIM_IS_BIT_SET(nothing,3);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,4);
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(color.getColorsPtr());
            }

            if (CSimFlavor::getBoolVal(18))
            {
                if (dataStreams_old.size()+curves3d_old.size()+curves2d_old.size()+staticStreamsAndCurves_old.size()!=0)
                    App::logMsg(sim_verbosity_errors,"Contains old graph streams/curves...");
            }
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("size",_graphSize);

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("object");
                color.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlAddNode_floats("background",backgroundColor,3);
                ar.xmlAddNode_floats("text",textColor,3);
            }
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(color.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("object",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(backgroundColor[l]*255.1f);
                ar.xmlAddNode_ints("background",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(textColor[l]*255.1f);
                ar.xmlAddNode_ints("text",rgb,3);
            }
            ar.xmlPopNode();

            ar.xmlAddNode_int("bufferSize",bufferSize);

            if (exhaustiveXml)
                ar.xmlAddNode_int("pointCount",numberOfPoints);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("cyclic",cyclic);
            ar.xmlAddNode_bool("showXYZ",xYZPlanesDisplay);
            ar.xmlAddNode_bool("showGrid",graphGrid);
            ar.xmlAddNode_bool("showValues",graphValues);
            ar.xmlAddNode_bool("explicitHandling",_explicitHandling);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                std::vector<floatDouble> tmp;
                for (int i=0;i<numberOfPoints;i++)
                {
                    int absIndex;
                    getAbsIndexOfPosition(i,absIndex);
                    tmp.push_back(times[absIndex]);
                }
                ar.xmlAddNode_floats("times",tmp);

                for (size_t i=0;i<_dataStreams.size();i++)
                {
                    ar.xmlPushNewNode("stream");
                    _dataStreams[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                    ar.xmlPopNode();
                }

                for (size_t i=0;i<_curves.size();i++)
                {
                    ar.xmlPushNewNode("curve");
                    _curves[i]->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                    ar.xmlPopNode();
                }

                // Old:
                // ---------------------
                for (size_t i=0;i<dataStreams_old.size();i++)
                {
                    ar.xmlPushNewNode("dataStream");
                    dataStreams_old[i]->serialize(ar,this);
                    ar.xmlPopNode();
                }
                for (size_t i=0;i<curves3d_old.size();i++)
                {
                    ar.xmlPushNewNode("3dCurve");
                    curves3d_old[i]->serialize(ar);
                    ar.xmlPopNode();
                }
                for (size_t i=0;i<curves2d_old.size();i++)
                {
                    ar.xmlPushNewNode("2dCurve");
                    curves2d_old[i]->serialize(ar);
                    ar.xmlPopNode();
                }
                for (size_t i=0;i<staticStreamsAndCurves_old.size();i++)
                {
                    ar.xmlPushNewNode("staticCurve");
                    staticStreamsAndCurves_old[i]->serialize(ar);
                    ar.xmlPopNode();
                }
                // ---------------------
            }
        }
        else
        {
            ar.xmlGetNode_float("size",_graphSize,exhaustiveXml);

            if (ar.xmlPushChildNode("color",exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("object"))
                    {
                        color.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    ar.xmlGetNode_floats("background",backgroundColor,3);
                    ar.xmlGetNode_floats("text",textColor,3);
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object",rgb,3,exhaustiveXml))
                        color.setColor(floatDouble(rgb[0])/255.1,floatDouble(rgb[1])/255.1,floatDouble(rgb[2])/255.1,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("background",rgb,3,exhaustiveXml))
                    {
                        backgroundColor[0]=floatDouble(rgb[0])/255.1;
                        backgroundColor[1]=floatDouble(rgb[1])/255.1;
                        backgroundColor[2]=floatDouble(rgb[2])/255.1;
                    }
                    if (ar.xmlGetNode_ints("text",rgb,3,exhaustiveXml))
                    {
                        textColor[0]=floatDouble(rgb[0])/255.1;
                        textColor[1]=floatDouble(rgb[1])/255.1;
                        textColor[2]=floatDouble(rgb[2])/255.1;
                    }
                }
                ar.xmlPopNode();
            }
            ar.xmlGetNode_int("bufferSize",bufferSize,exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_int("pointCount",numberOfPoints);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("cyclic",cyclic,exhaustiveXml);
                ar.xmlGetNode_bool("showXYZ",xYZPlanesDisplay,exhaustiveXml);
                ar.xmlGetNode_bool("showGrid",graphGrid,exhaustiveXml);
                ar.xmlGetNode_bool("showValues",graphValues,exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling",_explicitHandling,exhaustiveXml);
                ar.xmlPopNode();
            }

            times.clear();
            if (exhaustiveXml)
            {
                ar.xmlGetNode_floats("times",times);

                if (ar.xmlPushChildNode("stream",false))
                {
                    while (true)
                    {
                        CGraphDataStream* it=new CGraphDataStream();
                        it->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                        _dataStreams.push_back(it);
                        if (!ar.xmlPushSiblingNode("stream",false))
                            break;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("curve",false))
                {
                    while (true)
                    {
                        CGraphCurve* it=new CGraphCurve();
                        it->serialize(ar,startingPoint,numberOfPoints,bufferSize);
                        _curves.push_back(it);
                        if (!ar.xmlPushSiblingNode("curve",false))
                            break;
                    }
                    ar.xmlPopNode();
                }

                // Old:
                // --------------
                if (ar.xmlPushChildNode("dataStream",false))
                {
                    while (true)
                    {
                        CGraphData_old* it=new CGraphData_old();
                        it->serialize(ar,this);
                        dataStreams_old.push_back(it);
                        if (!ar.xmlPushSiblingNode("dataStream",false))
                            break;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("3dCurve",false))
                {
                    while (true)
                    {
                        CGraphDataComb_old* it=new CGraphDataComb_old();
                        it->serialize(ar);
                        curves3d_old.push_back(it);
                        if (!ar.xmlPushSiblingNode("3dCurve",false))
                            break;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("2dCurve",false))
                {
                    while (true)
                    {
                        CGraphDataComb_old* it=new CGraphDataComb_old();
                        it->serialize(ar);
                        curves2d_old.push_back(it);
                        if (!ar.xmlPushSiblingNode("2dCurve",false))
                            break;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("staticCurve",false))
                {
                    while (true)
                    {
                        CStaticGraphCurve_old* it=new CStaticGraphCurve_old();
                        it->serialize(ar);
                        // Following 4 on 16/3/2017: duplicate names for static curves can cause problems
                        std::string nm(it->getName());
                        while (getStaticCurveFromName(it->getCurveType(),nm.c_str())!=nullptr)
                            nm=tt::generateNewName_noHash(nm.c_str());
                        it->setName(nm);
                        staticStreamsAndCurves_old.push_back(it);
                        if (!ar.xmlPushSiblingNode("staticCurve",false))
                            break;
                    }
                    ar.xmlPopNode();
                }
                // --------------
            }
            computeBoundingBox();
        }
    }
}

void CGraph::display(CViewableBase* renderingObject,int displayAttrib)
{ // This is a quite ugly routine which requires refactoring!
    EASYLOCK(_objectMutex);
    displayGraph(this,renderingObject,displayAttrib);
}

#ifdef SIM_WITH_GUI
void CGraph::lookAt(int windowSize[2],CSView* subView,bool timeGraph,bool drawText,bool passiveSubView,bool oneOneProportionForXYGraph)
{ // drawText is false and passiveSubView is true by default
    // Default values (used for instance in view selection mode)
    int currentWinSize[2];
    int mouseRelativePosition[2];
    int mouseDownRelativePosition[2];
    floatDouble graphPos[2]={-2.0,-2.0};
    floatDouble graphSize[2]={4.0,4.0};
    int selectionStatus=NOSELECTION;
    bool autoMode=true;
    bool timeGraphYaxisAutoMode=true;
    bool mouseIsDown=false;
    int mouseMode=sim_navigation_passive;
    if (windowSize!=nullptr)
    {
        currentWinSize[0]=windowSize[0];
        currentWinSize[1]=windowSize[1];
    }
    if (subView!=nullptr)
    {
        subView->getViewSize(currentWinSize);
        subView->getMouseRelativePosition(mouseRelativePosition);
        subView->getMouseDownRelativePosition(mouseDownRelativePosition);
        if (timeGraph)
        {
            autoMode=subView->getTimeGraphXAutoModeDuringSimulation()&&App::currentWorld->simulation->isSimulationRunning();
            timeGraphYaxisAutoMode=subView->getTimeGraphYAutoModeDuringSimulation()&&App::currentWorld->simulation->isSimulationRunning();
            oneOneProportionForXYGraph=false;
        }
        else
        {
            autoMode=subView->getXYGraphAutoModeDuringSimulation()&&App::currentWorld->simulation->isSimulationRunning();
            oneOneProportionForXYGraph=subView->getXYGraphIsOneOneProportional();
            timeGraphYaxisAutoMode=true;
        }
        subView->getGraphPosition(graphPos);
        subView->getGraphSize(graphSize);
        timeGraph=subView->getTimeGraph();
        if (!passiveSubView)
        {
            selectionStatus=subView->getSelectionStatus();
            mouseIsDown=subView->isMouseDown();
        }
    }

    // We go a first time through the rendering process without displaying anything (we check the view size etc.)
    drawValues(currentWinSize,graphPos,graphSize,mouseRelativePosition,false,true,autoMode,timeGraphYaxisAutoMode,drawText,passiveSubView,timeGraph,subView);

    if ((!timeGraph)&&oneOneProportionForXYGraph)
        validateViewValues(currentWinSize,graphPos,graphSize,false,false,true,autoMode);

    // We set up the view:
    glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(backgroundColor[0],backgroundColor[1],backgroundColor[2],1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(graphPos[0],graphPos[0]+graphSize[0],graphPos[1],graphPos[1]+graphSize[1],-1.0f,1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRenderMode(GL_RENDER);
    glDisable(GL_DEPTH_TEST);
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    // Now we render the graph:
    drawValues(currentWinSize,graphPos,graphSize,mouseRelativePosition,mouseIsDown,false,autoMode,timeGraphYaxisAutoMode,drawText,passiveSubView,timeGraph,subView);
    drawGrid(currentWinSize,graphPos,graphSize);
    if (!passiveSubView)
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
            drawOverlay(currentWinSize,graphPos,graphSize,mouseMode,subView,passiveSubView);
    }

    // Once the scene is rendered, we have to report changes of some values:
    if (subView!=nullptr)
    {
        subView->setGraphPosition(graphPos[0],graphPos[1]);
        subView->setGraphSize(graphSize[0],graphSize[1]);
    }
    glEnable(GL_DEPTH_TEST);
}

void CGraph::drawGrid(int windowSize[2],floatDouble graphPosition[2],floatDouble graphSize[2])
{
    floatDouble interline=((floatDouble)ogl::getInterline())*graphSize[1]/(floatDouble)windowSize[1];
    floatDouble labelPos[2];
    labelPos[0]=graphPosition[0]+3.0*graphSize[0]/(floatDouble)windowSize[0];
    labelPos[1]=graphPosition[1]+0.3*interline;

    // First compute the x- and y-grid start and the x- and y-grid spacing
    int minNbOfLines[2]={8*windowSize[0]/1024,8*windowSize[1]/768};
    int maxNbOfLines[2]={2*minNbOfLines[0],2*minNbOfLines[1]};

    floatDouble gridStartX=graphPosition[0];
    floatDouble a=(floatDouble)((int)log10f(graphSize[0]));
    if (graphSize[0]<1.0)
        a=a-1.0;
    floatDouble gridSpacingX=(floatDouble)pow(10.0,a);
    while((graphSize[0]/gridSpacingX)<minNbOfLines[0])
        gridSpacingX=gridSpacingX/2.0;
    while((graphSize[0]/gridSpacingX)>maxNbOfLines[0])
        gridSpacingX=gridSpacingX*2.0;
    gridStartX=(((int)(gridStartX/gridSpacingX))*gridSpacingX)-gridSpacingX;

    floatDouble gridStartY=graphPosition[1];
    a=(floatDouble)((int)log10f(graphSize[1]));
    if (graphSize[1]<1.0)
        a=a-1.0;
    floatDouble gridSpacingY=(floatDouble)pow(10.0,a);
    while ((graphSize[1]/gridSpacingY)<minNbOfLines[1])
        gridSpacingY=gridSpacingY/2.0;
    while ((graphSize[1]/gridSpacingY)>maxNbOfLines[1])
        gridSpacingY=gridSpacingY*2.0;
    gridStartY=(((int)(gridStartY/gridSpacingY))*gridSpacingY)-gridSpacingY;

    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,textColor);
    if (graphGrid)
    {
        glLineStipple(1,0x1111);
        glEnable(GL_LINE_STIPPLE);
        ogl::buffer.clear();
        for (int i=0;i<60;i++)
        {
                ogl::addBuffer2DPoints(gridStartX+gridSpacingX*i,gridStartY);
                ogl::addBuffer2DPoints(gridStartX+gridSpacingX*i,gridStartY+60.0*gridSpacingY);
                ogl::addBuffer2DPoints(gridStartX,gridStartY+gridSpacingY*i);
                ogl::addBuffer2DPoints(gridStartX+60.0*gridSpacingX,gridStartY+gridSpacingY*i);
        }
        ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,false,0.0);
        ogl::buffer.clear();
        glDisable(GL_LINE_STIPPLE);
    }
    if (graphValues)
    {
        std::string tmp;
        for (int i=0;i<60;i++)
        {
            tmp=gv::getTimeStr(gridStartX+gridSpacingX*i);
            ogl::drawBitmapTextTo2dPosition(gridStartX+gridSpacingX*i,labelPos[1],tmp.c_str());
            int d=0;
            if (fabs(gridSpacingY)>0.0)
            {
                floatDouble l=log10f(fabs(gridSpacingY));
                d=int(1.5-l);
                if (d<0)
                    d=0;
                tmp=tt::FNb(0,gridStartY+gridSpacingY*i,d,false);
                ogl::drawBitmapTextTo2dPosition(labelPos[0],gridStartY+gridSpacingY*i,tmp.c_str());
            }
        }
    }
}

void CGraph::drawOverlay(int windowSize[2],floatDouble graphPosition[2],floatDouble graphSize[2],int mouseMode,CSView* subView,bool passiveSubView)
{
    // Draw the selection square
    if ( (subView!=nullptr)&&(!passiveSubView)&&subView->isMouseDown()&&(subView->getSelectionStatus()==SHIFTSELECTION) )
    {
        floatDouble downRelPos[2];
        floatDouble relPos[2];
        int aux[2];
        subView->getMouseDownRelativePosition(aux);
        downRelPos[0]=graphPosition[0]+((floatDouble)aux[0]/(floatDouble)windowSize[0])*graphSize[0];
        downRelPos[1]=graphPosition[1]+((floatDouble)aux[1]/(floatDouble)windowSize[1])*graphSize[1];
        subView->getMouseRelativePosition(aux);
        relPos[0]=graphPosition[0]+((floatDouble)aux[0]/(floatDouble)windowSize[0])*graphSize[0];
        relPos[1]=graphPosition[1]+((floatDouble)aux[1]/(floatDouble)windowSize[1])*graphSize[1];
        ogl::setAlpha(0.2f);
        if ((relPos[0]>downRelPos[0])&&(downRelPos[1]>relPos[1]))
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
                glVertex3f(downRelPos[0],downRelPos[1],0);
                glVertex3f(downRelPos[0],relPos[1],0);
                glVertex3f(relPos[0],relPos[1],0);
                glVertex3f(relPos[0],downRelPos[1],0);
                glVertex3f(downRelPos[0],downRelPos[1],0);
            glEnd();
            ogl::setBlending(false);
            ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
            glBegin(GL_LINE_STRIP);
                glVertex3f(downRelPos[0],downRelPos[1],0);
                glVertex3f(downRelPos[0],relPos[1],0);
                glVertex3f(relPos[0],relPos[1],0);
                glVertex3f(relPos[0],downRelPos[1],0);
                glVertex3f(downRelPos[0],downRelPos[1],0);
            glEnd();
            ogl::setTextColor(textColor);
            ogl::drawBitmapTextIntoScene((relPos[0]+downRelPos[0])/2.0f,(relPos[1]+downRelPos[1])/2.0f,0.0f,IDSOGL_ZOOMING_IN);
        }
        ogl::setTextColor(textColor);
        if ((relPos[0]>downRelPos[0])&&(relPos[1]>downRelPos[1]))
            ogl::drawBitmapTextIntoScene((relPos[0]+downRelPos[0])/2.0f,(relPos[1]+downRelPos[1])/2.0f,0.0f,IDSOGL_SETTING_PROPORTIONS_TO_1_1);
        if ((downRelPos[0]>relPos[0])&&(relPos[1]>downRelPos[1]))
            ogl::drawBitmapTextIntoScene((relPos[0]+downRelPos[0])/2.0f,(relPos[1]+downRelPos[1])/2.0f,0.0f,IDSOGL_ZOOMING_OUT);
        if ((downRelPos[0]>relPos[0])&&(relPos[1]<downRelPos[1]))
            ogl::drawBitmapTextIntoScene((relPos[0]+downRelPos[0])/2.0f,(relPos[1]+downRelPos[1])/2.0f,0.0f,IDSOGL_ZOOMING_IN);
    }
}




void CGraph::drawValues(int windowSize[2],floatDouble graphPosition[2],floatDouble graphSize[2],
                        int mousePosition[2],bool mouseIsDown,bool dontRender,
                        bool autoMode,bool timeGraphYaxisAutoMode,bool drawText,bool passiveSubView,bool timeGraph,CSView* subView)
{ // This is a quite ugly routine which requires refactoring!
    static bool markSelectedStream=false;
    floatDouble interline=((floatDouble)ogl::getInterline())*graphSize[1]/(floatDouble)windowSize[1];
    floatDouble labelPos[2];
    floatDouble pixelSizeCoeff=graphSize[0]/(floatDouble)windowSize[0];
    labelPos[0]=graphPosition[0]+graphSize[0]-3.0*pixelSizeCoeff;
    labelPos[1]=graphPosition[1]+graphSize[1]-interline;

    floatDouble maxVal[2]={-FLOAT_MAX,-FLOAT_MAX};
    floatDouble minVal[2]={+FLOAT_MAX,+FLOAT_MAX};
    if (dontRender)
        trackingValueIndex=-1;
    floatDouble ratio=graphSize[1]/graphSize[0];
    floatDouble relMousePos[2];
    relMousePos[0]=graphPosition[0]+graphSize[0]*((floatDouble)mousePosition[0]/(floatDouble)windowSize[0]);
    relMousePos[1]=graphPosition[1]+graphSize[1]*((floatDouble)mousePosition[1]/(floatDouble)windowSize[1]);
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    if (App::userSettings->antiAliasing)
    {
        glEnable (GL_LINE_SMOOTH);
        glEnable (GL_POINT_SMOOTH);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
    }
    if (timeGraph)
    { // Display of time graph curves here:
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        std::string tmp(IDSOGL_TIME_GRAPH_CURVES_);
        floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
        ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
        labelPos[1]=labelPos[1]-interline;
        for (int i=0;i<int(dataStreams_old.size());i++)
        {
            if (dataStreams_old[i]->getVisible())
            {
                CGraphData_old* it=dataStreams_old[i];
                ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                if (!dontRender)
                { // We display that curve
                    ogl::buffer.clear();
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glLineWidth(3.0); // we are tracking that curve!
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glPointSize(6.0); // we are tracking that curve!
                        else
                            glPointSize(4.0);
                    }
                    int pos=0;
                    int absIndex;
                    floatDouble yVal,xVal;

                    bool cyclic;
                    floatDouble range;
                    CGraphingRoutines_old::getCyclicAndRangeValues(it,cyclic,range);

                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        xVal=times[absIndex];
                        bool dataIsValid=getData(it,absIndex,yVal,cyclic,range,true);
                        if (dataIsValid)
                            ogl::addBuffer2DPoints(xVal,yVal);
                    }
                    if (ogl::buffer.size()!=0)
                    {
                        if (it->getLinkPoints())
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0);
                    }
                    ogl::buffer.clear();
                    glPointSize(1.0);
                    glLineWidth(1.0);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" ("+CGraphingRoutines_old::getDataUnit(it)+")";
                        floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
                        labelPos[1]=labelPos[1]-interline;
                    }

                }
                else
                { // We don't display the curve, we just get its min-max values
                    int pos=0;
                    int absIndex;
                    floatDouble yVal,xVal;
                    bool cyclic;
                    floatDouble range;
                    CGraphingRoutines_old::getCyclicAndRangeValues(it,cyclic,range);
                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        xVal=times[absIndex];
                        bool dataIsValid=getData(it,absIndex,yVal,cyclic,range,true);
                        if (xVal<minVal[0]) // keep those two outside of dataIsValid check!
                            minVal[0]=xVal;
                        if (xVal>maxVal[0])
                            maxVal[0]=xVal;
                        if (dataIsValid)
                        {
                            if (yVal<minVal[1])
                                minVal[1]=yVal;
                            if (yVal>maxVal[1])
                                maxVal[1]=yVal;
                            if ( (!passiveSubView)&&(!mouseIsDown) )
                            { // Here we have the value tracking part:
                                if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
                                {
                                    if (trackingValueIndex!=-1)
                                    {
                                        floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                        floatDouble d2=relMousePos[1]-yVal;
                                        floatDouble dist=d1*d1+d2*d2;
                                        if (dist<squareDistFromTrackingValue)
                                        {
                                            trackingValue[0]=xVal;
                                            trackingValue[1]=yVal;
                                            squareDistFromTrackingValue=dist;
                                            trackingValueIndex=i;
                                            trackingValueIsStatic=false;
                                        }
                                    }
                                    else
                                    {
                                        trackingValue[0]=xVal;
                                        trackingValue[1]=yVal;
                                        floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                        floatDouble d2=relMousePos[1]-yVal;
                                        squareDistFromTrackingValue=d1*d1+d2*d2;
                                        trackingValueIndex=i;
                                        trackingValueIsStatic=false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // We have the static curves here:
        floatDouble vOffset=2.0*graphSize[1]/floatDouble(windowSize[1]);
        if (dontRender)
            vOffset=0.0;
        for (int i=0;i<int(staticStreamsAndCurves_old.size());i++)
        {
            if (staticStreamsAndCurves_old[i]->getCurveType()==0)
            {
                CStaticGraphCurve_old* it=staticStreamsAndCurves_old[i];
                if (!dontRender)
                {
                    ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glLineWidth(3.0); // we are tracking that curve!
                        glLineStipple(1,0xE187);
                        glEnable(GL_LINE_STIPPLE);
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glPointSize(6.0); // we are tracking that curve!
                        else
                            glPointSize(4.0);
                    }
                }
                ogl::buffer.clear();
                for (int ka=0;ka<int(it->values.size()/2);ka++)
                {
                    floatDouble xVal=it->values[2*ka+0];
                    floatDouble yVal=it->values[2*ka+1]+vOffset;
                    ogl::addBuffer2DPoints(xVal,yVal);
                    if (dontRender)
                    { // min/max and value tracking
                        if ((yVal<minVal[1])&&(xVal>=minVal[0])&&(xVal<=maxVal[0])) // only in the active time slice!
                            minVal[1]=yVal;
                        if ((yVal>maxVal[1])&&(xVal>=minVal[0])&&(xVal<=maxVal[0])) // only in the active time slice!
                            maxVal[1]=yVal;
                        if ( (!passiveSubView)&&(!mouseIsDown) )
                        { // Here we have the value tracking part:
                            if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
                            {
                                if (trackingValueIndex!=-1)
                                {
                                    if ((xVal>=minVal[0])&&(xVal<=maxVal[0])) // only in the active time slice!
                                    {
                                        floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                        floatDouble d2=relMousePos[1]-yVal;
                                        floatDouble dist=d1*d1+d2*d2;
                                        if (dist<squareDistFromTrackingValue)
                                        {
                                            trackingValue[0]=xVal;
                                            trackingValue[1]=yVal-vOffset;
                                            squareDistFromTrackingValue=dist;
                                            trackingValueIndex=i;
                                            trackingValueIsStatic=true;
                                        }
                                    }
                                }
                                else
                                {
                                    if ((xVal>=minVal[0])&&(xVal<=maxVal[0])) // only in the active time slice!
                                    {
                                        trackingValue[0]=xVal;
                                        trackingValue[1]=yVal-vOffset;
                                        floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                        floatDouble d2=relMousePos[1]-yVal;
                                        squareDistFromTrackingValue=d1*d1+d2*d2;
                                        trackingValueIndex=i;
                                        trackingValueIsStatic=true;
                                    }
                                }
                            }
                        }
                    }
                }
                if (!dontRender)
                {
                    if (ogl::buffer.size()!=0)
                    {
                        if (it->getLinkPoints())
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0);
                    }
                    ogl::buffer.clear();

                    glPointSize(1.0);
                    glLineWidth(1.0);
                    glDisable(GL_LINE_STIPPLE);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" [STATIC]";
                        floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
                        labelPos[1]-=interline;
                    }
                }
            }
        }
    }
    else
    { // Display of xy graph curves here:
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        std::string tmp(IDSOGL_X_Y_GRAPH_CURVES_);
        floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
        ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
        labelPos[1]=labelPos[1]-interline;
        for (int i=0;i<int(curves2d_old.size());i++)
        {
            if (curves2d_old[i]->getVisible())
            {
                CGraphDataComb_old* it=curves2d_old[i];
                ogl::setMaterialColor(sim_colorcomponent_emission,it->curveColor.getColorsPtr());
                if (!dontRender)
                {
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glLineWidth(3.0); // we are tracking that curve!
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glPointSize(6.0); // we are tracking that curve!
                        else
                            glPointSize(4.0);
                    }
                    ogl::buffer.clear();
                    int pos=0;
                    int absIndex;
                    floatDouble val[3];
                    CGraphData_old* number1=getGraphData(it->data[0]);
                    CGraphData_old* number2=getGraphData(it->data[1]);

                    bool cyclic1,cyclic2;
                    floatDouble range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);

                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        bool dataIsValid=true;
                        if (number1!=nullptr)
                        {
                            if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (number2!=nullptr)
                        {
                            if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (dataIsValid)
                            ogl::addBuffer2DPoints(val[0],val[1]);
                    }

                    if (ogl::buffer.size()!=0)
                    {
                        if (it->getLinkPoints())
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0);
                    }
                    ogl::buffer.clear();

                    glPointSize(1.0);
                    glLineWidth(1.0);
                    if ( it->getLabel() )
                    {
                        if ( (number1!=nullptr)&&(number2!=nullptr) )
                        {
                            tmp=it->getName()+" (x: "+CGraphingRoutines_old::getDataUnit(number1)+") (y: "+CGraphingRoutines_old::getDataUnit(number2)+")";
                            floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
                            ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
                            labelPos[1]=labelPos[1]-interline;
                        }
                    }
                }
                else
                { // We don't display the curve, we just take the max-min values:
                    int pos=0;
                    int absIndex;
                    floatDouble val[3];
                    CGraphData_old* number1=getGraphData(it->data[0]);
                    CGraphData_old* number2=getGraphData(it->data[1]);
                    bool cyclic1,cyclic2;
                    floatDouble range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines_old::getCyclicAndRangeValues(number2,cyclic2,range2);
                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        bool dataIsValid=true;
                        if (number1!=nullptr)
                        {
                            if(!getData(number1,absIndex,val[0],cyclic1,range1,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (number2!=nullptr)
                        {
                            if(!getData(number2,absIndex,val[1],cyclic2,range2,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (dataIsValid)
                        {
                            if (val[0]<minVal[0])
                                minVal[0]=val[0];
                            if (val[0]>maxVal[0])
                                maxVal[0]=val[0];
                            if (val[1]<minVal[1])
                                minVal[1]=val[1];
                            if (val[1]>maxVal[1])
                                maxVal[1]=val[1];

                            if ( (!passiveSubView)&&(!mouseIsDown) )
                            { // Here we have the value tracking part:
                                if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
                                {
                                    if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
                                    {
                                        if (trackingValueIndex!=-1)
                                        {
                                            floatDouble d1=(relMousePos[0]-val[0])*ratio;
                                            floatDouble d2=relMousePos[1]-val[1];
                                            floatDouble dist=d1*d1+d2*d2;
                                            if (dist<squareDistFromTrackingValue)
                                            {
                                                trackingValue[0]=val[0];
                                                trackingValue[1]=val[1];
                                                squareDistFromTrackingValue=dist;
                                                trackingValueIndex=i;
                                                trackingValueIsStatic=false;
                                            }
                                        }
                                        else
                                        {
                                            trackingValue[0]=val[0];
                                            trackingValue[1]=val[1];
                                            floatDouble d1=(relMousePos[0]-val[0])*ratio;
                                            floatDouble d2=relMousePos[1]-val[1];
                                            squareDistFromTrackingValue=d1*d1+d2*d2;
                                            trackingValueIndex=i;
                                            trackingValueIsStatic=false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // We have the static curves here:
        floatDouble vOffset=2.0*graphSize[1]/floatDouble(windowSize[1]);
        if (dontRender)
            vOffset=0.0;
        for (int i=0;i<int(staticStreamsAndCurves_old.size());i++)
        {
            if (staticStreamsAndCurves_old[i]->getCurveType()==1)
            {
                CStaticGraphCurve_old* it=staticStreamsAndCurves_old[i];
                if (!dontRender)
                {
                    ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glLineWidth(3.0); // we are tracking that curve!
                        glLineStipple(1,0xE187);
                        glEnable(GL_LINE_STIPPLE);
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glPointSize(6.0); // we are tracking that curve!
                        else
                            glPointSize(4.0);
                    }
                }
                ogl::buffer.clear();
                for (int ka=0;ka<int(it->values.size()/2);ka++)
                {
                    floatDouble xVal=it->values[2*ka+0];
                    floatDouble yVal=it->values[2*ka+1]+vOffset;
                    ogl::addBuffer2DPoints(xVal,yVal);

                    if (dontRender)
                    { // Min/max and value tracking!
                        if (xVal<minVal[0])
                            minVal[0]=xVal;
                        if (xVal>maxVal[0])
                            maxVal[0]=xVal;
                        if (yVal<minVal[1])
                            minVal[1]=yVal;
                        if (yVal>maxVal[1])
                            maxVal[1]=yVal;
                        if ( (!passiveSubView)&&(!mouseIsDown) )
                        { // Here we have the value tracking part:
                            if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
                            {
                                if (trackingValueIndex!=-1)
                                {
                                    floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                    floatDouble d2=relMousePos[1]-yVal;
                                    floatDouble dist=d1*d1+d2*d2;
                                    if (dist<squareDistFromTrackingValue)
                                    {
                                        trackingValue[0]=xVal;
                                        trackingValue[1]=yVal-vOffset;
                                        squareDistFromTrackingValue=dist;
                                        trackingValueIndex=i;
                                        trackingValueIsStatic=true;
                                    }
                                }
                                else
                                {
                                    trackingValue[0]=xVal;
                                    trackingValue[1]=yVal-vOffset;
                                    floatDouble d1=(relMousePos[0]-xVal)*ratio;
                                    floatDouble d2=relMousePos[1]-yVal;
                                    squareDistFromTrackingValue=d1*d1+d2*d2;
                                    trackingValueIndex=i;
                                    trackingValueIsStatic=true;
                                }
                            }
                        }
                    }
                }
                if (!dontRender)
                {
                    if (ogl::buffer.size()!=0)
                    {
                        if (it->getLinkPoints())
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0f);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0f);
                    }
                    ogl::buffer.clear();
                    glPointSize(1.0);
                    glLineWidth(1.0);
                    glDisable(GL_LINE_STIPPLE);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" [STATIC]";
                        floatDouble tl=floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp.c_str());
                        labelPos[1]=labelPos[1]-interline;
                    }
                }
            }
        }
    }

    // following 3 to reset antialiasing:
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POINT_SMOOTH);
    glDisable (GL_BLEND);

    // Data tracking:
    bool markSelectedStreamSaved=markSelectedStream;
    markSelectedStream=false;
    floatDouble sensitivity=30.0;
    if ( (!passiveSubView)&&(!mouseIsDown)&&dontRender )
    {
        if ( (trackingValueIndex!=-1)&&((sqrtf(squareDistFromTrackingValue)*windowSize[1]/graphSize[1])<sensitivity) )
            markSelectedStream=true;
    }
    if (subView!=nullptr)
        subView->setTrackedGraphCurveIndex(-1);
    if ( (!passiveSubView)&&(!mouseIsDown)&&(!dontRender) )
    {
        if ( (trackingValueIndex!=-1)&&((sqrtf(squareDistFromTrackingValue)*windowSize[1]/graphSize[1])<sensitivity) )
        {
            if ( (mousePosition[0]>=0)&&(mousePosition[0]<windowSize[0])&&(mousePosition[1]>=0)&&(mousePosition[1]<windowSize[1]) )
            {
                floatDouble squareSize=5.0;
                floatDouble r[2];
                r[0]=squareSize*graphSize[0]/(floatDouble)windowSize[0];
                r[1]=squareSize*graphSize[1]/(floatDouble)windowSize[1];
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
                std::string tmp;
                if (subView!=nullptr)
                    subView->setTrackedGraphCurveIndex(-1);
                if (timeGraph)
                {
                    if (!trackingValueIsStatic)
                    {
                        CGraphData_old* it=dataStreams_old[trackingValueIndex];
                        tmp=" ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        floatDouble l0=r[0]*2.5;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5)
                            l0=-r[0]*2.5-graphSize[0]*floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))/floatDouble(windowSize[0]);
                        floatDouble l1=r[1]*2.5;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5)
                            l1=-r[1]*2.5;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp.c_str());
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                        if (markSelectedStreamSaved&&(subView!=nullptr))
                            subView->setTrackedGraphCurveIndex(trackingValueIndex);
                    }
                    else
                    { // Tracking a static curve here!
                        CStaticGraphCurve_old* it=staticStreamsAndCurves_old[trackingValueIndex];
                        tmp=" [STATIC] ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        floatDouble l0=r[0]*2.5;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5)
                            l0=-r[0]*2.5-graphSize[0]*floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))/floatDouble(windowSize[0]);
                        floatDouble l1=r[1]*2.5;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5)
                            l1=-r[1]*2.5;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp.c_str());
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    }
                }
                else
                {
                    if (!trackingValueIsStatic)
                    {
                        CGraphDataComb_old* it=curves2d_old[trackingValueIndex];
                        tmp=" ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        floatDouble l0=r[0]*2.5;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5)
                            l0=-r[0]*2.5-graphSize[0]*floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))/floatDouble(windowSize[0]);
                        floatDouble l1=r[1]*2.5;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5)
                            l1=-r[1]*2.5;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp.c_str());
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->curveColor.getColorsPtr());
                        if (markSelectedStreamSaved&&(subView!=nullptr))
                            subView->setTrackedGraphCurveIndex(trackingValueIndex);
                    }
                    else
                    { // tracking a static curve here!
                        CStaticGraphCurve_old* it=staticStreamsAndCurves_old[trackingValueIndex];
                        tmp=" [STATIC] ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        floatDouble l0=r[0]*2.5;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5)
                            l0=-r[0]*2.5-graphSize[0]*floatDouble(ogl::getTextLengthInPixels(tmp.c_str()))/floatDouble(windowSize[0]);
                        floatDouble l1=r[1]*2.5;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5)
                            l1=-r[1]*2.5;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp.c_str());
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    }
                }
                ogl::buffer.clear();
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]-r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]+r[0],trackingValue[1]-r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]+r[0],trackingValue[1]+r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]+r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]-r[1]);
                ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0);
                ogl::buffer.clear();
            }
        }
    }

    if (dontRender)
    { // into a running time graph!
        floatDouble size[2]={maxVal[0]-minVal[0],maxVal[1]-minVal[1]};
        if (timeGraph)
        {
            if (autoMode)
            { // for the x axis:
                graphPosition[0]=minVal[0]-0.05*size[0];
                graphSize[0]=1.1*size[0];
            }
            if (timeGraphYaxisAutoMode)
            { // for the y axis:
                graphPosition[1]=minVal[1]-0.05*size[1];
                graphSize[1]=1.1*size[1];
            }
        }
        else
        {
            if (autoMode)
            {
                graphPosition[0]=minVal[0]-0.05*size[0];
                graphPosition[1]=minVal[1]-0.05*size[1];
                graphSize[0]=1.1*size[0];
                graphSize[1]=1.1*size[1];
            }
        }
        validateViewValues(windowSize,graphPosition,graphSize,timeGraph,false,false,autoMode);
    }
}

void CGraph::validateViewValues(int windowSize[2],floatDouble graphPosition[2],floatDouble graphSize[2],
            bool timeGraph,bool shiftOnly,bool keepProp,bool autoModeForTimeGraphXaxis)
{ // keepProp is false by default, shiftOnly also
    floatDouble minValues[2]={-FLOAT_MAX,-FLOAT_MAX};
    floatDouble maxValues[2]={+FLOAT_MAX,+FLOAT_MAX};
    floatDouble minGraphSize[2]={0.00001,0.00001};
    if (timeGraph)
    {
        if (autoModeForTimeGraphXaxis)
        {
            if (numberOfPoints<2)
            {
                minValues[0]=0.0;
                maxValues[0]=1.0;
                minGraphSize[0]=1.0;
            }
            else
            {
                int absIndex;
                getAbsIndexOfPosition(0,absIndex);
                minValues[0]=times[absIndex];
                getAbsIndexOfPosition(numberOfPoints-1,absIndex);
                maxValues[0]=times[absIndex];
            }
        }
    }
    floatDouble maxGraphSize[2]={maxValues[0]-minValues[0],maxValues[1]-minValues[1]};
    if (shiftOnly)
    {
        if (graphPosition[0]<minValues[0])
            graphPosition[0]=minValues[0];
        if (graphPosition[1]<minValues[1])
            graphPosition[1]=minValues[1];
        if ((graphPosition[0]+graphSize[0])>maxValues[0])
            graphPosition[0]=graphPosition[0]-(graphPosition[0]+graphSize[0]-maxValues[0]);
        if ((graphPosition[1]+graphSize[1])>maxValues[1])
            graphPosition[1]=graphPosition[1]-(graphPosition[1]+graphSize[1]-maxValues[1]);
    }
    else
    {
        if (graphSize[0]<minGraphSize[0])
            graphPosition[0]=graphPosition[0]+0.5*(graphSize[0]-minGraphSize[0]);
        if (graphSize[1]<minGraphSize[1])
            graphPosition[1]=graphPosition[1]+0.5*(graphSize[1]-minGraphSize[1]);

        if (graphPosition[0]<minValues[0])
            graphPosition[0]=minValues[0];
        if (graphPosition[1]<minValues[1])
            graphPosition[1]=minValues[1];
        if (graphPosition[0]>(maxValues[0]-minGraphSize[0]))
            graphPosition[0]=maxValues[0]-minGraphSize[0];
        if (graphPosition[1]>(maxValues[1]-minGraphSize[1]))
            graphPosition[1]=maxValues[1]-minGraphSize[1];
        if (graphSize[0]<minGraphSize[0])
            graphSize[0]=minGraphSize[0];
        if (graphSize[1]<minGraphSize[1])
            graphSize[1]=minGraphSize[1];
        if ((graphPosition[0]+graphSize[0])>maxValues[0])
            graphSize[0]=maxValues[0]-graphPosition[0];
        if ((graphPosition[1]+graphSize[1])>maxValues[1])
            graphSize[1]=maxValues[1]-graphPosition[1];
    }

    if (keepProp)
    {
        floatDouble prop[2]={((floatDouble)windowSize[0])/graphSize[0],((floatDouble)windowSize[1])/graphSize[1]};
        floatDouble coeff=prop[1]/prop[0];
        floatDouble nys=graphSize[1]*coeff;
        floatDouble nxs=graphSize[0]/coeff;
        if (coeff>1.0)
        {   // We have to zoom out on the y-axis
            if (nys<maxGraphSize[1])
            {   // We zoom out on the y-axis
                graphPosition[1]=graphPosition[1]-(nys-graphSize[1])/2.0;
                if (graphPosition[1]<minValues[1])
                    graphPosition[1]=minValues[1];
                if ((graphPosition[1]+nys)>maxValues[1])
                    graphPosition[1]=graphPosition[1]+maxValues[1]-(graphPosition[1]+nys);
                graphSize[1]=nys;
            }
            else
            {   // We zoom in on the x-axis
                graphPosition[0]=graphPosition[0]-(nxs-graphSize[0])/2.0;
                graphSize[0]=nxs;
            }
        }
        else
        {   // We have to zoom in on the y-axis
            if (nxs<maxGraphSize[0])
            {   // We zoom out on the x-axis
                graphPosition[0]=graphPosition[0]-(nxs-graphSize[0])/2.0;
                if (graphPosition[0]<minValues[0])
                    graphPosition[0]=minValues[0];
                if ((graphPosition[0]+nxs)>maxValues[0])
                    graphPosition[0]=graphPosition[0]+maxValues[0]-(graphPosition[0]+nxs);
                graphSize[0]=nxs;
            }
            else
            {   // We zoom in on the y-axis
                graphPosition[1]=graphPosition[1]-(nys-graphSize[1])/2.0;
                graphSize[1]=nys;
            }
        }
    }
}
#endif


