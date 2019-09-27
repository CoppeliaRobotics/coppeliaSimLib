
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "graph.h"
#include "tt.h"
#include "gV.h"
#include "graphingRoutines.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "vVarious.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "graphRendering.h"

CGraph::CGraph()
{
    setObjectType(sim_object_graph_type);
    justDrawCurves=false;
    _explicitHandling=false;
    bufferSize=1000;
    numberOfPoints=0;
    startingPoint=0;
    threeDPartners.reserve(4);
    threeDPartners.clear();
    twoDPartners.reserve(4);
    twoDPartners.clear();
    daten.reserve(16);
    daten.clear();
    times.reserve(bufferSize);
    times.clear();
    for (int i=0;i<bufferSize;i++)
        times.push_back(0.0f);

    _initialValuesInitialized=false;

    _localObjectSpecialProperty=0; // actually also renderable, but turned off by default!
    cyclic=true;
    xYZPlanesDisplay=true;
    graphGrid=true;
    graphValues=true;
    size=0.1f;
    color.setDefaultValues();
    color.setColor(0.15f,0.15f,0.15f,sim_colorcomponent_ambient_diffuse);

    backgroundColor[0]=0.1f;
    backgroundColor[1]=0.1f;
    backgroundColor[2]=0.1f;

    textColor[0]=0.8f;
    textColor[1]=0.8f;
    textColor[2]=0.8f;
    layer=GRAPH_LAYER;
    _objectName=IDSOGL_GRAPH;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

CGraph::~CGraph()
{
    for (int i=0;i<int(threeDPartners.size());i++)
        delete threeDPartners[i];
    threeDPartners.clear();
    for (int i=0;i<int(twoDPartners.size());i++)
        delete twoDPartners[i];
    twoDPartners.clear();
    for (int i=0;i<int(daten.size());i++)
        delete daten[i];
    daten.clear();

    removeAllStatics();
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
    return(true);
}
bool CGraph::isPotentiallyCuttable() const
{
    return(false);
}

bool CGraph::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    getMarkingBoundingBox(minV,maxV);
    C7Vector thisInv(getCumulativeTransformation().getInverse());
    for (int i=0;i<int(threeDPartners.size());i++)
    {
        const CGraphData* part0=getGraphData(threeDPartners[i]->data[0]);
        const CGraphData* part1=getGraphData(threeDPartners[i]->data[1]);
        const CGraphData* part2=getGraphData(threeDPartners[i]->data[2]);
        int pos=0;
        int absIndex;
        float point[3];
        bool cyclic0,cyclic1,cyclic2;
        float range0,range1,range2;
        if (part0!=nullptr)    
            CGraphingRoutines::getCyclicAndRangeValues(part0,cyclic0,range0);
        if (part1!=nullptr)    
            CGraphingRoutines::getCyclicAndRangeValues(part1,cyclic1,range1);
        if (part2!=nullptr)    
            CGraphingRoutines::getCyclicAndRangeValues(part2,cyclic2,range2);
        while (getAbsIndexOfPosition(pos++,absIndex))
        {
            bool dataIsValid=true;
            if (part0!=nullptr)
            {
                if(!getData(part0,absIndex,point[0],cyclic0,range0,true))
                    dataIsValid=false;
            }
            else
                dataIsValid=false;
            if (part1!=nullptr)
            {
                if(!getData(part1,absIndex,point[1],cyclic1,range1,true))
                    dataIsValid=false;
            }
            else
                dataIsValid=false;
            if (part2!=nullptr)
            {
                if(!getData(part2,absIndex,point[2],cyclic2,range2,true))
                    dataIsValid=false;
            }
            else
                dataIsValid=false;
            if (dataIsValid)
            {
                C3Vector pp(point);
                if (threeDPartners[i]->getCurveRelativeToWorld())
                    pp=thisInv*pp;
                minV.keepMin(pp);
                maxV.keepMax(pp);
            }
        }
    }

    // Static 3D curves now:
    for (int i=0;i<int(_staticCurves.size());i++)
    {
        CStaticGraphCurve* it=_staticCurves[i];
        if (it->getCurveType()==2)
        {
            for (int j=0;j<int(it->values.size()/3);j++)
            {
                C3Vector pp(it->values[3*j+0],it->values[3*j+1],it->values[3*j+2]);
                if (it->getRelativeToWorld())
                    pp=thisInv*pp;
                minV.keepMin(pp);
                maxV.keepMax(pp);
            }
        }
    }

    return(true);
}

bool CGraph::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    maxV(0)=maxV(1)=maxV(2)=size/2.0f;
    minV=maxV*-1.0f;
    return(true);
}

bool CGraph::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
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
    return((int)daten.size());
}
int CGraph::get2DCurveCount() const
{
    return((int)twoDPartners.size());
}
int CGraph::get3DCurveCount() const
{
    return((int)threeDPartners.size());
}

CGraphData* CGraph::getGraphData(int id) const
{
    for (size_t i=0;i<daten.size();i++)
    {
        if (daten[i]->getIdentifier()==id)
            return(daten[i]);
    }
    return(nullptr);
}
CGraphData* CGraph::getGraphData(std::string theName) const
{
    for (size_t i=0;i<daten.size();i++)
    {
        if (daten[i]->getName().compare(theName)==0)
            return(daten[i]);
    }
    return(nullptr);
}
CGraphDataComb* CGraph::getGraphData3D(int id) const
{
    for (size_t i=0;i<threeDPartners.size();i++)
    {
        if (threeDPartners[i]->getIdentifier()==id)
        return(threeDPartners[i]);
    }
    return(nullptr);
}
CGraphDataComb* CGraph::getGraphData3D(std::string theName) const
{
    for (size_t i=0;i<threeDPartners.size();i++)
    {
        if (threeDPartners[i]->getName().compare(theName)==0)
            return(threeDPartners[i]);
    }
    return(nullptr);
}
CGraphDataComb* CGraph::getGraphData2D(int id) const
{
    for (size_t i=0;i<twoDPartners.size();i++)
    {
        if (twoDPartners[i]->getIdentifier()==id)
            return(twoDPartners[i]);
    }
    return(nullptr);
}
CGraphDataComb* CGraph::getGraphData2D(std::string theName) const
{
    for (size_t i=0;i<twoDPartners.size();i++)
    {
        if (twoDPartners[i]->getName().compare(theName)==0)
            return(twoDPartners[i]);
    }
    return(nullptr);
}

int CGraph::addNewGraphData(CGraphData* graphData)
{   // Returns the graphData identifier
    // We don't care if already present, because we could scale one but not the other
    // for instance.
    std::string theName=graphData->getName();
    while (getGraphData(theName)!=nullptr)
        theName=tt::generateNewName_noDash(theName);
    graphData->setName(theName);
    int id=0;
    while (getGraphData(id)!=nullptr)
        id++;
    graphData->setIdentifier(id);
    daten.push_back(graphData);
    graphData->resetData(bufferSize);
    return(graphData->getIdentifier());
}
void CGraph::removeGraphData(int id)
{
    announceGraphDataObjectWillBeDestroyed(id);
    for (int i=0;i<int(daten.size());i++)
    {
        if (daten[i]->getIdentifier()==id)
        {
            delete daten[i];
            daten.erase(daten.begin()+i);
            break;
        }
    }
}
void CGraph::remove2DPartners(int id)
{
    for (int i=0;i<int(twoDPartners.size());i++)
    {
        if (twoDPartners[i]->getIdentifier()==id)
        {
            delete twoDPartners[i];
            twoDPartners.erase(twoDPartners.begin()+i);
            break;
        }
    }
}
void CGraph::remove3DPartners(int id)
{
    for (int i=0;i<int(threeDPartners.size());i++)
    {
        if (threeDPartners[i]->getIdentifier()==id)
        {
            delete threeDPartners[i];
            threeDPartners.erase(threeDPartners.begin()+i);
            break;
        }
    }
}
void CGraph::add2DPartners(CGraphDataComb* it)
{
    std::string tmp=it->getName();
    while (getGraphData2D(tmp)!=nullptr)
        tmp=tt::generateNewName_noDash(tmp);
    it->setName(tmp);
    int id=0;
    while (getGraphData2D(id)!=nullptr)
        id++;
    it->setIdentifier(id);
    twoDPartners.push_back(it);
}
void CGraph::add3DPartners(CGraphDataComb* it)
{
    std::string tmp=it->getName();
    while (getGraphData3D(tmp)!=nullptr)
        tmp=tt::generateNewName_noDash(tmp);
    it->setName(tmp);
    int id=0;
    while (getGraphData3D(id)!=nullptr)
        id++;
    it->setIdentifier(id);
    threeDPartners.push_back(it);
}

void CGraph::addNextPoint(float time)
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
    C7Vector m(getCumulativeTransformationPart1());
    for (int i=0;i<int(daten.size());i++)
    {
        bool cyclic;
        float range;
        CGraphingRoutines::getCyclicAndRangeValues(daten[i],cyclic,range);
        daten[i]->setValue(&m,nextEntryPosition,nextEntryPosition==startingPoint,cyclic,range,times);
        // Here we have to handle a special case: GRAPH_VARIOUS_TIME
        if (daten[i]->getDataType()==GRAPH_NOOBJECT_TIME)
            daten[i]->setValueDirect(nextEntryPosition,time,nextEntryPosition==startingPoint,cyclic,range,times);
        daten[i]->clearUserData();
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

void CGraph::scaleObject(float scalingFactor)
{
    size=size*scalingFactor;
    scaleObjectMain(scalingFactor);
}

void CGraph::scaleObjectNonIsometrically(float x,float y,float z)
{
    scaleObject(cbrt(x*y*z));
}

void CGraph::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CGraph::copyYourself()
{   
    CGraph* newGraph=(CGraph*)copyYourselfMain();
    color.copyYourselfInto(&newGraph->color);
    newGraph->setSize(getSize());
    newGraph->setCyclic(getCyclic());
    newGraph->setBufferSize(getBufferSize());
    newGraph->numberOfPoints=numberOfPoints;
    newGraph->startingPoint=startingPoint;
    newGraph->xYZPlanesDisplay=xYZPlanesDisplay;
    newGraph->times.reserve(times.size());
    newGraph->times.clear();
    for (int i=0;i<int(times.size());i++)
        newGraph->times.push_back(times[i]);
    newGraph->daten.reserve(daten.size());
    newGraph->daten.clear();
    for (int i=0;i<int(daten.size());i++)
        newGraph->daten.push_back(daten[i]->copyYourself());
    newGraph->threeDPartners.reserve(threeDPartners.size());
    newGraph->threeDPartners.clear();
    for (int i=0;i<int(threeDPartners.size());i++)
        newGraph->threeDPartners.push_back(threeDPartners[i]->copyYourself());
    newGraph->twoDPartners.reserve(twoDPartners.size());
    newGraph->twoDPartners.clear();
    for (int i=0;i<int(twoDPartners.size());i++)
        newGraph->twoDPartners.push_back(twoDPartners[i]->copyYourself());

    for (int i=0;i<int(_staticCurves.size());i++)
        newGraph->_staticCurves.push_back(_staticCurves[i]->copyYourself());

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
    for (size_t i=0;i<_staticCurves.size();i++)
        delete _staticCurves[i];
    _staticCurves.clear();
}

void CGraph::makeCurveStatic(int curveIndex,int dimensionIndex)
{
    if (dimensionIndex==0)
    { // time graph curves:
        if (curveIndex<int(daten.size()))
        {
            std::vector<float> timeValues;
            std::vector<float> staticValues;
            CGraphData* it=daten[curveIndex];
            int pos=0;
            int absIndex;
            float yVal,xVal;
            bool cyclic;
            float range;
            CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);
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
                while (getStaticCurveFromName(0,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(0,&timeValues,&staticValues,nullptr);
                _staticCurves.push_back(curve);
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
        if (curveIndex<int(twoDPartners.size()))
        {
            std::vector<float> values0;
            std::vector<float> values1;
            CGraphDataComb* it=twoDPartners[curveIndex];
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            float range1,range2;
            if (number1!=nullptr)  
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)  
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);

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
                while (getStaticCurveFromName(1,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(1,&values0,&values1,nullptr);
                _staticCurves.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.colors[0];
                curve->ambientColor[1]=it->curveColor.colors[1];
                curve->ambientColor[2]=it->curveColor.colors[2];
                curve->emissiveColor[0]=it->curveColor.colors[9];
                curve->emissiveColor[1]=it->curveColor.colors[10];
                curve->emissiveColor[2]=it->curveColor.colors[11];
            }
        }
    }
    if (dimensionIndex==2)
    { // 3D graph curves:
        if (curveIndex<int(threeDPartners.size()))
        {
            std::vector<float> values0;
            std::vector<float> values1;
            std::vector<float> values2;
            CGraphDataComb* it=threeDPartners[curveIndex];
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            CGraphData* number3=getGraphData(it->data[2]);
            bool cyclic1,cyclic2,cyclic3;
            float range1,range2,range3;
            if (number1!=nullptr)  
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)  
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);
            if (number3!=nullptr)  
                CGraphingRoutines::getCyclicAndRangeValues(number3,cyclic3,range3);

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
                while (getStaticCurveFromName(2,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(2,&values0,&values1,&values2);
                _staticCurves.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.colors[0];
                curve->ambientColor[1]=it->curveColor.colors[1];
                curve->ambientColor[2]=it->curveColor.colors[2];
                curve->emissiveColor[0]=it->curveColor.colors[9];
                curve->emissiveColor[1]=it->curveColor.colors[10];
                curve->emissiveColor[2]=it->curveColor.colors[11];
                curve->setCurveWidth(it->get3DCurveWidth());
                curve->setRelativeToWorld(threeDPartners[curveIndex]->getCurveRelativeToWorld());
            }
        }
    }
}

CStaticGraphCurve* CGraph::getStaticCurveFromName(int type,const std::string& name)
{
    for (size_t i=0;i<_staticCurves.size();i++)
    {
        CStaticGraphCurve* it=_staticCurves[i];
        if (it->getCurveType()==type)
        {
            if (it->getName().compare(name)==0)
                return(it);
        }
    }
    return(nullptr);
}

void CGraph::setSize(float theNewSize)
{
    tt::limitValue(0.001f,10.0f,theNewSize);
    size=theNewSize;
}

float CGraph::getSize() const
{
    return(size);
}

void CGraph::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CGraph::getExplicitHandling() const
{
    return(_explicitHandling);
}

CVisualParam* CGraph::getColor()
{
    return(&color);
}


void CGraph::announceGraphDataObjectWillBeDestroyed(int graphDataID)
{
// We first erase all the data entries from threeDPartners and twoDPartners
// of graph graphID which contain graphDataID.
    // We first set all invalid entries to -1:  
    for (int j=0;j<3;j++)
    {
        for (int i=0;i<int(threeDPartners.size());i++)
        {
            if (threeDPartners[i]->data[j]==graphDataID)
                threeDPartners[i]->data[j]=-1;
        }
        for (int i=0;i<int(twoDPartners.size());i++)
        {
            if (twoDPartners[i]->data[j]==graphDataID)
                twoDPartners[i]->data[j]=-1;
        }
    }
    // Now we remove all threeDPartners and twoDPartners which have 0 valid
    // entries:
    int i=0;
    while (i<int(threeDPartners.size()))
    {
        int objID=threeDPartners[i]->getIdentifier();
        if ( (threeDPartners[i]->data[0]==-1)&&
            (threeDPartners[i]->data[1]==-1)&&
            (threeDPartners[i]->data[2]==-1) )
        {
            remove3DPartners(objID);
            i=0; // We start at 0 again: ordering may have changed
        }
        else
            i++;
    }
    i=0;
    while (i<int(twoDPartners.size()))
    {
        int objID=twoDPartners[i]->getIdentifier();
        if ( (twoDPartners[i]->data[0]==-1)&&
            (twoDPartners[i]->data[1]==-1)&&
            (twoDPartners[i]->data[2]==-1) )
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
    ar.writeString(getObjectName());
    ar << (unsigned char)13;
    ar << (unsigned char)10;
    // The first line:
    std::string tmp("Time ("+gv::getTimeUnitStr()+")");
    if (daten.size()!=0)
        tmp+=",";
    ar.writeString(tmp);
    for (int k=0;k<int(daten.size());k++)
    {
        CGraphData* gr=daten[k];
        tmp=gr->getName()+" ("+CGraphingRoutines::getDataUnit(gr)+")";
        if (k<(int(daten.size())-1))
            tmp+=",";
        ar.writeString(tmp);
    }
    ar << (unsigned char)13;
    ar << (unsigned char)10;

    // Now the data:
    int pos=0;
    int absIndex;
    while (getAbsIndexOfPosition(pos++,absIndex))
    {
        float value=times[absIndex];
        tmp=tt::FNb(0,value,6,false);
        if (daten.size()!=0)
            tmp+=",";
        ar.writeString(tmp);
        for (int k=0;k<int(daten.size());k++)
        {
            CGraphData* gr=daten[k];
            bool cyclic;
            float range;
            CGraphingRoutines::getCyclicAndRangeValues(gr,cyclic,range);
            bool dataIsValid=getData(gr,absIndex,value,cyclic,range,true);
            if (dataIsValid)
                tmp=tt::FNb(0,value,6,false);
            else
                tmp="Null";
            if (k<(int(daten.size())-1))
                tmp+=",";
            ar.writeString(tmp);
        }
        ar << (unsigned char)13;
        ar << (unsigned char)10;
    }
    ar << (unsigned char)13;
    ar << (unsigned char)10;
}

bool CGraph::getGraphCurve(int graphType,int index,std::string& label,std::vector<float>& xVals,std::vector<float>& yVals,std::vector<float>& zVals,int& curveType,float col[3],float minMax[6]) const
{
    if (graphType==0)
    { // time
        for (size_t ind=0;ind<daten.size();ind++)
        {
            CGraphData* gr=daten[ind];
            if (gr->getVisible())
            {
                if (index==0)
                {
                    label=gr->getName()+" ("+CGraphingRoutines::getDataUnit(gr)+")";
                    if (gr->getLinkPoints())
                        curveType=0;
                    else
                        curveType=1;
                    col[0]=gr->ambientColor[0];
                    col[1]=gr->ambientColor[1];
                    col[2]=gr->ambientColor[2];

                    int pos=0;
                    int absIndex;
                    while (getAbsIndexOfPosition(pos++,absIndex))
                    {
                        float xVal=times[absIndex];
                        float yVal;
                        bool cyclic;
                        float range;
                        CGraphingRoutines::getCyclicAndRangeValues(gr,cyclic,range);
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
        for (size_t ind=0;ind<_staticCurves.size();ind++)
        {
            CStaticGraphCurve* gr=_staticCurves[ind];
            if (gr->getCurveType()==0)
            { // time
                if (index==0)
                {
                    label=gr->getName()+" (STATIC)";
                    if (gr->getLinkPoints())
                        curveType=2;
                    else
                        curveType=2+1;
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
        for (size_t ind=0;ind<twoDPartners.size();ind++)
        {
            CGraphDataComb* it=twoDPartners[ind];
            if (it->getVisible())
            {
                if (index==0)
                {
                    col[0]=it->curveColor.colors[0];
                    col[1]=it->curveColor.colors[1];
                    col[2]=it->curveColor.colors[2];
                    int pos=0;
                    int absIndex;
                    float val[3];
                    CGraphData* number1=getGraphData(it->data[0]);
                    CGraphData* number2=getGraphData(it->data[1]);
                    bool cyclic1,cyclic2;
                    float range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);
                    label=it->getName()+" (x: ";
                    if (number1!=nullptr)
                        label+=CGraphingRoutines::getDataUnit(number1)+") (y: ";
                    else
                        label+="0.0) (y: ";
                    if (number2!=nullptr)
                        label+=CGraphingRoutines::getDataUnit(number2)+")";
                    else
                        label+="0.0)";
                    if (it->getLinkPoints())
                        curveType=0;
                    else
                        curveType=1;

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

        for (size_t ind=0;ind<_staticCurves.size();ind++)
        {
            CStaticGraphCurve* gr=_staticCurves[ind];
            if (gr->getCurveType()==1)
            { // x/y
                if (index==0)
                {
                    label=gr->getName()+" (STATIC)";
                    if (gr->getLinkPoints())
                        curveType=2;
                    else
                        curveType=2+1;
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
    if (graphType==2)
    { // x/y/z
        for (size_t ind=0;ind<threeDPartners.size();ind++)
        {
            CGraphDataComb* it=threeDPartners[ind];
            if (it->getVisible())
            {
                if (index==0)
                {
                    col[0]=it->curveColor.colors[0];
                    col[1]=it->curveColor.colors[1];
                    col[2]=it->curveColor.colors[2];
                    int pos=0;
                    int absIndex;
                    float val[3];
                    CGraphData* number1=getGraphData(it->data[0]);
                    CGraphData* number2=getGraphData(it->data[1]);
                    CGraphData* number3=getGraphData(it->data[2]);
                    bool cyclic1,cyclic2,cyclic3;
                    float range1,range2,range3;
                    if (number1!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);
                    if (number3!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number3,cyclic3,range3);
                    label=it->getName()+" (x: ";
                    if (number1!=nullptr)
                        label+=CGraphingRoutines::getDataUnit(number1)+") (y: ";
                    else
                        label+="0.0) (y: ";
                    if (number2!=nullptr)
                        label+=CGraphingRoutines::getDataUnit(number2)+") (z: ";
                    else
                        label+="0.0) (z: ";
                    if (number3!=nullptr)
                        label+=CGraphingRoutines::getDataUnit(number3)+")";
                    else
                        label+="0.0)";
                    if (it->getLinkPoints())
                        curveType=0;
                    else
                        curveType=1;

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
                        if (number3!=nullptr)
                        {
                            if(!getData(number3,absIndex,val[2],cyclic3,range3,true))
                                dataIsValid=false;
                        }
                        else
                            val[2]=0.0;
                        if (dataIsValid)
                        {
                            xVals.push_back(val[0]);
                            yVals.push_back(val[1]);
                            zVals.push_back(val[2]);
                            if (xVals.size()==1)
                            {
                                minMax[0]=val[0];
                                minMax[1]=val[0];
                                minMax[2]=val[1];
                                minMax[3]=val[1];
                                minMax[4]=val[2];
                                minMax[5]=val[2];
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
                                if (val[2]<minMax[4])
                                    minMax[4]=val[2];
                                if (val[2]>minMax[5])
                                    minMax[5]=val[2];
                            }
                        }
                    }
                    return(true);
                }
                index--;
            }
        }

        for (size_t ind=0;ind<_staticCurves.size();ind++)
        {
            CStaticGraphCurve* gr=_staticCurves[ind];
            if (gr->getCurveType()==2)
            { // x/y/z
                if (index==0)
                {
                    label=gr->getName()+" (STATIC)";
                    if (gr->getLinkPoints())
                        curveType=2;
                    else
                        curveType=2+1;
                    col[0]=gr->ambientColor[0];
                    col[1]=gr->ambientColor[1];
                    col[2]=gr->ambientColor[2];
                    for (size_t i=0;i<gr->values.size()/3;i++)
                    {
                        xVals.push_back(gr->values[3*i+0]);
                        yVals.push_back(gr->values[3*i+1]);
                        zVals.push_back(gr->values[3*i+2]);
                        if (xVals.size()==1)
                        {
                            minMax[0]=gr->values[3*i+0];
                            minMax[1]=gr->values[3*i+0];
                            minMax[2]=gr->values[3*i+1];
                            minMax[3]=gr->values[3*i+1];
                            minMax[4]=gr->values[3*i+2];
                            minMax[5]=gr->values[3*i+2];
                        }
                        else
                        {
                            if (gr->values[3*i+0]<minMax[0])
                                minMax[0]=gr->values[3*i+0];
                            if (gr->values[3*i+0]>minMax[1])
                                minMax[1]=gr->values[3*i+0];
                            if (gr->values[3*i+1]<minMax[2])
                                minMax[2]=gr->values[3*i+1];
                            if (gr->values[3*i+1]>minMax[3])
                                minMax[3]=gr->values[3*i+1];
                            if (gr->values[3*i+2]<minMax[4])
                                minMax[4]=gr->values[3*i+2];
                            if (gr->values[3*i+2]>minMax[5])
                                minMax[5]=gr->values[3*i+2];
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

void CGraph::curveToClipboard(int graphType,const std::string& curveName) const
{
    std::string txt;
    if (graphType==0)
    { // time graph curves:
        CGraphData* it=nullptr;
        for (size_t ind=0;ind<daten.size();ind++)
        {
            if (daten[ind]->getName().compare(curveName)==0)
            {
                it=daten[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            int pos=0;
            int absIndex;
            float yVal,xVal;
            bool cyclic;
            float range;
            CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);
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
        CGraphDataComb* it=nullptr;
        for (size_t ind=0;ind<twoDPartners.size();ind++)
        {
            if (twoDPartners[ind]->getName().compare(curveName)==0)
            {
                it=twoDPartners[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            float range1,range2;
            if (number1!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);

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
        for (size_t i=0;i<_staticCurves.size();i++)
        {
            CStaticGraphCurve* it=_staticCurves[i];
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

void CGraph::curveToStatic(int graphType,const std::string& curveName)
{
    if (graphType==0)
    { // time graph curves:
        CGraphData* it=nullptr;
        for (size_t ind=0;ind<daten.size();ind++)
        {
            if (daten[ind]->getName().compare(curveName)==0)
            {
                it=daten[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<float> timeValues;
            std::vector<float> staticValues;
            int pos=0;
            int absIndex;
            float yVal,xVal;
            bool cyclic;
            float range;
            CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);
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
                while (getStaticCurveFromName(0,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(0,&timeValues,&staticValues,nullptr);
                _staticCurves.push_back(curve);
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
        CGraphDataComb* it=nullptr;
        for (size_t ind=0;ind<twoDPartners.size();ind++)
        {
            if (twoDPartners[ind]->getName().compare(curveName)==0)
            {
                it=twoDPartners[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<float> values0;
            std::vector<float> values1;
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            float range1,range2;
            if (number1!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);

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
                while (getStaticCurveFromName(1,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(1,&values0,&values1,nullptr);
                _staticCurves.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.colors[0];
                curve->ambientColor[1]=it->curveColor.colors[1];
                curve->ambientColor[2]=it->curveColor.colors[2];
                curve->emissiveColor[0]=it->curveColor.colors[9];
                curve->emissiveColor[1]=it->curveColor.colors[10];
                curve->emissiveColor[2]=it->curveColor.colors[11];
            }
        }
    }
    if (graphType==2)
    { // 3D graph curves:
        CGraphDataComb* it=nullptr;
        for (size_t ind=0;ind<threeDPartners.size();ind++)
        {
            if (threeDPartners[ind]->getName().compare(curveName)==0)
            {
                it=threeDPartners[ind];
                break;
            }
        }
        if (it!=nullptr)
        {
            std::vector<float> values0;
            std::vector<float> values1;
            std::vector<float> values2;
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            CGraphData* number3=getGraphData(it->data[2]);
            bool cyclic1,cyclic2,cyclic3;
            float range1,range2,range3;
            if (number1!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);
            if (number3!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number3,cyclic3,range3);

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
                while (getStaticCurveFromName(2,nm)!=nullptr)
                    nm=tt::generateNewName_noDash(nm);
                CStaticGraphCurve* curve=new CStaticGraphCurve(2,&values0,&values1,&values2);
                _staticCurves.push_back(curve);
                curve->setName(nm);
                curve->setLabel(it->getLabel());
                curve->setLinkPoints(it->getLinkPoints());
                curve->ambientColor[0]=it->curveColor.colors[0];
                curve->ambientColor[1]=it->curveColor.colors[1];
                curve->ambientColor[2]=it->curveColor.colors[2];
                curve->emissiveColor[0]=it->curveColor.colors[9];
                curve->emissiveColor[1]=it->curveColor.colors[10];
                curve->emissiveColor[2]=it->curveColor.colors[11];
                curve->setCurveWidth(it->get3DCurveWidth());
                curve->setRelativeToWorld(it->getCurveRelativeToWorld());
            }
        }
    }
}

void CGraph::removeStaticCurve(int graphType,const std::string& curveName)
{
    for (size_t i=0;i<_staticCurves.size();i++)
    {
        if (_staticCurves[i]->getCurveType()==graphType-3)
        {
            if (_staticCurves[i]->getName().compare(curveName)==0)
            {
                delete _staticCurves[i];
                _staticCurves.erase(_staticCurves.begin()+i);
                break;
            }
        }
    }
}

bool CGraph::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(daten.size()))
    {
        CGraphData* gr=daten[i];
        if (gr->announceObjectWillBeErased(objectHandle,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
    return(retVal);
}

void CGraph::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CGraph::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(daten.size()))
    {
        CGraphData* gr=daten[i];
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
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(daten.size()))
    {
        CGraphData* gr=daten[i];
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
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(daten.size()))
    {
        CGraphData* gr=daten[i];
        if (gr->announceIkObjectWillBeErased(ikGroupID,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}
void CGraph::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
    // Remove the object which will be erased from the daten's list
    int i=0;
    while (i<int(daten.size()))
    {
        CGraphData* gr=daten[i];
        if (gr->announceGcsObjectWillBeErased(gcsObjectID,copyBuffer))
        { // We have to remove this graphData:
            removeGraphData(gr->getIdentifier());
            i=0; // We start at 0 again
        }
        else
            i++;
    }
}
void CGraph::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    for (int i=0;i<int(daten.size());i++)
        daten[i]->performObjectLoadingMapping(map);
}
void CGraph::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CGraph::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
    for (int i=0;i<int(daten.size());i++)
        daten[i]->performCollisionLoadingMapping(map);
}
void CGraph::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
    for (int i=0;i<int(daten.size());i++)
        daten[i]->performDistanceLoadingMapping(map);
}
void CGraph::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
    for (int i=0;i<int(daten.size());i++)
        daten[i]->performIkLoadingMapping(map);
}
void CGraph::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
    for (int i=0;i<int(daten.size());i++)
        daten[i]->performGcsLoadingMapping(map);
}

void CGraph::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CGraph::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CGraph::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CGraph::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CGraph::setBufferSize(int buffSize)
{
    tt::limitValue(10,1000000,buffSize);
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

bool CGraph::getData(const CGraphData* it,int pos,float& outputValue,bool cyclic,float range,bool doUnitConversion) const
{
    float cumulativeValue=0.0f;
    int cumulativeValueCount=0;
    int movingAverageCount=it->getMovingAverageCount();
    for (int i=0;i<movingAverageCount;i++)
    {
        float tmpVal;
        if (it->getValue(pos,tmpVal))
        {
            if (doUnitConversion)
                CGraphingRoutines::adjustDataToUserMetric(it,tmpVal,it->getDerivativeIntegralAndCumulative());
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
        outputValue=cumulativeValue/float(cumulativeValueCount);
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
    for (int i=0;i<int(daten.size());i++)
        daten[i]->resetData(bufferSize);
}

void CGraph::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
        resetGraph();
    }
}

void CGraph::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CGraph::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

void CGraph::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Ghg");
            ar << size;
            ar.flush();

            ar.storeDataName("Cl0"); // Colors
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            ar.storeDataName("Cl1");
            ar << backgroundColor[0] << backgroundColor[1] << backgroundColor[2];
            ar << textColor[0] << textColor[1] << textColor[2];
            ar.flush();

            ar.storeDataName("Gbv");
            ar << bufferSize << numberOfPoints;
            ar.flush();

            ar.storeDataName("Gtd"); // Should always come after bufferSize!!!
            for (int i=0;i<numberOfPoints;i++)
            {
                int absIndex;
                getAbsIndexOfPosition(i,absIndex);
                ar << times[absIndex];
            }
            ar.flush();

            for (int i=0;i<int(daten.size());i++)
            {
                ar.storeDataName("Ghd");
                ar.setCountingMode();
                daten[i]->serialize(ar,this);
                if (ar.setWritingMode())
                    daten[i]->serialize(ar,this);
            }
            for (int i=0;i<int(threeDPartners.size());i++)
            {
                ar.storeDataName("Gh3");
                ar.setCountingMode();
                threeDPartners[i]->serialize(ar);
                if (ar.setWritingMode())
                    threeDPartners[i]->serialize(ar);
            }
            for (int i=0;i<int(twoDPartners.size());i++)
            {
                ar.storeDataName("Gh2");
                ar.setCountingMode();
                twoDPartners[i]->serialize(ar);
                if (ar.setWritingMode())
                    twoDPartners[i]->serialize(ar);
            }

            for (int i=0;i<int(_staticCurves.size());i++)
            {
                ar.storeDataName("Sta");
                ar.setCountingMode();
                _staticCurves[i]->serialize(ar);
                if (ar.setWritingMode())
                    _staticCurves[i]->serialize(ar);
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
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> size;
                    }
                    if (theName.compare("Cl0")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> backgroundColor[0] >> backgroundColor[1] >> backgroundColor[2];
                        ar >> textColor[0] >> textColor[1] >> textColor[2];
                    }
                    if (theName.compare("Gbv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> bufferSize >> numberOfPoints;
                    }
                    if (theName.compare("Gtd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        times.reserve(bufferSize);
                        times.clear();
                        for (int i=0;i<bufferSize;i++)
                            times.push_back(0.0f);
                        for (int i=0;i<byteQuantity/int(sizeof(float));i++)
                        {
                            float aVal;
                            ar >> aVal;
                            times[i]=aVal;
                        }
                    }
                    if (theName.compare("Ghd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CGraphData* it=new CGraphData();
                        it->serialize(ar,this);
                        daten.push_back(it);
                    }
                    if (theName.compare("Gh3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CGraphDataComb* it=new CGraphDataComb();
                        it->serialize(ar);
                        threeDPartners.push_back(it);
                    }
                    if (theName.compare("Gh2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CGraphDataComb* it=new CGraphDataComb();
                        it->serialize(ar);
                        twoDPartners.push_back(it);
                    }
                    if (theName.compare("Sta")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CStaticGraphCurve* it=new CStaticGraphCurve();
                        it->serialize(ar);
                        // Following 4 on 16/3/2017: duplicate names for static curves can cause problems
                        std::string nm(it->getName());
                        while (getStaticCurveFromName(it->getCurveType(),nm)!=nullptr)
                            nm=tt::generateNewName_noDash(nm);
                        it->setName(nm);
                        _staticCurves.push_back(it);
                    }
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
                CTTUtil::scaleColorUp_(color.colors);
            }
        }
    }
}

void CGraph::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

VPoint CGraph::currentWinSize;

void CGraph::display(CViewableBase* renderingObject,int displayAttrib)
{ // This is a quite ugly routine which requires refactoring!
    FUNCTION_INSIDE_DEBUG("CGraph::display");
    EASYLOCK(_objectMutex);
    displayGraph(this,renderingObject,displayAttrib);
}

void CGraph::copyCurveToClipboard(int curveIndex,int dimensionIndex)
{
    std::string txt;
    if (dimensionIndex==0)
    { // time graph curves:
        if (curveIndex<int(daten.size()))
        {
            CGraphData* it=daten[curveIndex];
            int pos=0;
            int absIndex;
            float yVal,xVal;
            bool cyclic;
            float range;
            CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);
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
    if (dimensionIndex==1)
    { // x/y graph curves:
        if (curveIndex<int(twoDPartners.size()))
        {
            CGraphDataComb* it=twoDPartners[curveIndex];
            int pos=0;
            int absIndex;
            float val[3];
            CGraphData* number1=getGraphData(it->data[0]);
            CGraphData* number2=getGraphData(it->data[1]);
            bool cyclic1,cyclic2;
            float range1,range2;
            if (number1!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
            if (number2!=nullptr)
                CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);

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
    VVarious::copyTextToClipboard(txt);
}

#ifdef SIM_WITH_GUI
void CGraph::lookAt(int windowSize[2],CSView* subView,bool timeGraph,bool drawText,bool passiveSubView,bool oneOneProportionForXYGraph)
{ // drawText is false and passiveSubView is true by default
    // Default values (used for instance in view selection mode)
    int currentWinSize[2];
    int mouseRelativePosition[2];
    int mouseDownRelativePosition[2];
    float graphPos[2]={-2.0f,-2.0f};
    float graphSize[2]={4.0f,4.0f};
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
            autoMode=subView->getTimeGraphXAutoModeDuringSimulation()&&App::ct->simulation->isSimulationRunning();
            timeGraphYaxisAutoMode=subView->getTimeGraphYAutoModeDuringSimulation()&&App::ct->simulation->isSimulationRunning();
            oneOneProportionForXYGraph=false;
        }
        else
        {
            autoMode=subView->getXYGraphAutoModeDuringSimulation()&&App::ct->simulation->isSimulationRunning();
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
        if (!App::ct->simulation->isSimulationRunning())
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

void CGraph::drawGrid(int windowSize[2],float graphPosition[2],float graphSize[2])
{
    float interline=((float)ogl::getInterline())*graphSize[1]/(float)windowSize[1];
    float labelPos[2];
    labelPos[0]=graphPosition[0]+3.0f*graphSize[0]/(float)windowSize[0];
    labelPos[1]=graphPosition[1]+0.3f*interline;

    // First compute the x- and y-grid start and the x- and y-grid spacing
    int minNbOfLines[2]={8*windowSize[0]/1024,8*windowSize[1]/768};
    int maxNbOfLines[2]={2*minNbOfLines[0],2*minNbOfLines[1]};

    float gridStartX=graphPosition[0];
    float a=(float)((int)log10f(graphSize[0]));
    if (graphSize[0]<1.0f)
        a=a-1.0f;
    float gridSpacingX=(float)pow(10.0f,a);
    while((graphSize[0]/gridSpacingX)<minNbOfLines[0])
        gridSpacingX=gridSpacingX/2.0f;
    while((graphSize[0]/gridSpacingX)>maxNbOfLines[0])
        gridSpacingX=gridSpacingX*2.0f;
    gridStartX=(((int)(gridStartX/gridSpacingX))*gridSpacingX)-gridSpacingX;

    float gridStartY=graphPosition[1];
    a=(float)((int)log10f(graphSize[1]));
    if (graphSize[1]<1.0f)
        a=a-1.0f;
    float gridSpacingY=(float)pow(10.0f,a);
    while ((graphSize[1]/gridSpacingY)<minNbOfLines[1])
        gridSpacingY=gridSpacingY/2.0f;
    while ((graphSize[1]/gridSpacingY)>maxNbOfLines[1])
        gridSpacingY=gridSpacingY*2.0f;
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
                ogl::addBuffer2DPoints(gridStartX+gridSpacingX*i,gridStartY+60.0f*gridSpacingY);
                ogl::addBuffer2DPoints(gridStartX,gridStartY+gridSpacingY*i);
                ogl::addBuffer2DPoints(gridStartX+60.0f*gridSpacingX,gridStartY+gridSpacingY*i);
        }
        ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,false,0.0f);
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
            if (fabs(gridSpacingY)>0.0f)
            {
                float l=log10f(fabs(gridSpacingY));
                d=int(1.5f-l);
                if (d<0)
                    d=0;
                tmp=tt::FNb(0,gridStartY+gridSpacingY*i,d,false);
                ogl::drawBitmapTextTo2dPosition(labelPos[0],gridStartY+gridSpacingY*i,tmp.c_str());
            }
        }
    }
}

void CGraph::drawOverlay(int windowSize[2],float graphPosition[2],float graphSize[2],int mouseMode,CSView* subView,bool passiveSubView)
{
    // Draw the selection square
    if ( (subView!=nullptr)&&(!passiveSubView)&&subView->isMouseDown()&&(subView->getSelectionStatus()==SHIFTSELECTION) )
    {
        float downRelPos[2];
        float relPos[2];
        int aux[2];
        subView->getMouseDownRelativePosition(aux);
        downRelPos[0]=graphPosition[0]+((float)aux[0]/(float)windowSize[0])*graphSize[0];
        downRelPos[1]=graphPosition[1]+((float)aux[1]/(float)windowSize[1])*graphSize[1];
        subView->getMouseRelativePosition(aux);
        relPos[0]=graphPosition[0]+((float)aux[0]/(float)windowSize[0])*graphSize[0];
        relPos[1]=graphPosition[1]+((float)aux[1]/(float)windowSize[1])*graphSize[1];
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




void CGraph::drawValues(int windowSize[2],float graphPosition[2],float graphSize[2],
                        int mousePosition[2],bool mouseIsDown,bool dontRender,
                        bool autoMode,bool timeGraphYaxisAutoMode,bool drawText,bool passiveSubView,bool timeGraph,CSView* subView)
{ // This is a quite ugly routine which requires refactoring!
    static bool markSelectedStream=false;
    float interline=((float)ogl::getInterline())*graphSize[1]/(float)windowSize[1];
    float labelPos[2];
    float pixelSizeCoeff=graphSize[0]/(float)windowSize[0];
    labelPos[0]=graphPosition[0]+graphSize[0]-3.0f*pixelSizeCoeff;
    labelPos[1]=graphPosition[1]+graphSize[1]-interline;

    float maxVal[2]={-SIM_MAX_FLOAT,-SIM_MAX_FLOAT};
    float minVal[2]={+SIM_MAX_FLOAT,+SIM_MAX_FLOAT};
    if (dontRender)
        trackingValueIndex=-1;
    float ratio=graphSize[1]/graphSize[0];
    float relMousePos[2];
    relMousePos[0]=graphPosition[0]+graphSize[0]*((float)mousePosition[0]/(float)windowSize[0]);
    relMousePos[1]=graphPosition[1]+graphSize[1]*((float)mousePosition[1]/(float)windowSize[1]);
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
        float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
        ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
        labelPos[1]=labelPos[1]-interline;
        for (int i=0;i<int(daten.size());i++)
        {
            if (daten[i]->getVisible())
            {
                CGraphData* it=daten[i];
                ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                if (!dontRender)
                { // We display that curve
                    ogl::buffer.clear();
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glLineWidth(3.0f); // we are tracking that curve!
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glPointSize(6.0f); // we are tracking that curve!
                        else
                            glPointSize(4.0f);
                    }
                    int pos=0;
                    int absIndex;
                    float yVal,xVal;

                    bool cyclic;
                    float range;
                    CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);

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
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0f);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0f);
                    }
                    ogl::buffer.clear();
                    glPointSize(1.0f);
                    glLineWidth(1.0f);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" ("+CGraphingRoutines::getDataUnit(it)+")";
                        float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
                        labelPos[1]=labelPos[1]-interline;
                    }

                }
                else
                { // We don't display the curve, we just get its min-max values
                    int pos=0;
                    int absIndex;
                    float yVal,xVal;
                    bool cyclic;
                    float range;
                    CGraphingRoutines::getCyclicAndRangeValues(it,cyclic,range);
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
                                        float d1=(relMousePos[0]-xVal)*ratio;
                                        float d2=relMousePos[1]-yVal;
                                        float dist=d1*d1+d2*d2;
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
                                        float d1=(relMousePos[0]-xVal)*ratio;
                                        float d2=relMousePos[1]-yVal;
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
        float vOffset=2.0f*graphSize[1]/float(windowSize[1]);
        if (dontRender)
            vOffset=0.0f;
        for (int i=0;i<int(_staticCurves.size());i++)
        {
            if (_staticCurves[i]->getCurveType()==0)
            {
                CStaticGraphCurve* it=_staticCurves[i];
                if (!dontRender)
                {
                    ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glLineWidth(3.0f); // we are tracking that curve!
                        glLineStipple(1,0xE187);
                        glEnable(GL_LINE_STIPPLE);
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glPointSize(6.0f); // we are tracking that curve!
                        else
                            glPointSize(4.0f);
                    }
                }
                ogl::buffer.clear();
                for (int ka=0;ka<int(it->values.size()/2);ka++)
                {
                    float xVal=it->values[2*ka+0];
                    float yVal=it->values[2*ka+1]+vOffset;
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
                                        float d1=(relMousePos[0]-xVal)*ratio;
                                        float d2=relMousePos[1]-yVal;
                                        float dist=d1*d1+d2*d2;
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
                                        float d1=(relMousePos[0]-xVal)*ratio;
                                        float d2=relMousePos[1]-yVal;
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
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0f);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0f);
                    }
                    ogl::buffer.clear();

                    glPointSize(1.0f);
                    glLineWidth(1.0f);
                    glDisable(GL_LINE_STIPPLE);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" [STATIC]";
                        float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
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
        float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
        ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
        labelPos[1]=labelPos[1]-interline;
        for (int i=0;i<int(twoDPartners.size());i++)
        {
            if (twoDPartners[i]->getVisible())
            {
                CGraphDataComb* it=twoDPartners[i];
                ogl::setMaterialColor(sim_colorcomponent_emission,it->curveColor.colors);
                if (!dontRender)
                {
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glLineWidth(3.0f); // we are tracking that curve!
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&(!trackingValueIsStatic))
                            glPointSize(6.0f); // we are tracking that curve!
                        else
                            glPointSize(4.0f);
                    }
                    ogl::buffer.clear();
                    int pos=0;
                    int absIndex;
                    float val[3];
                    CGraphData* number1=getGraphData(it->data[0]);
                    CGraphData* number2=getGraphData(it->data[1]);

                    bool cyclic1,cyclic2;
                    float range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);

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
                            ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0f);
                        else
                            ogl::drawRandom2dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/2,0.0f);
                    }
                    ogl::buffer.clear();

                    glPointSize(1.0f);
                    glLineWidth(1.0f);
                    if ( it->getLabel() )
                    {
                        if ( (number1!=nullptr)&&(number2!=nullptr) )
                        {
                            tmp=it->getName()+" (x: "+CGraphingRoutines::getDataUnit(number1)+") (y: "+CGraphingRoutines::getDataUnit(number2)+")";
                            float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
                            ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
                            labelPos[1]=labelPos[1]-interline;
                        }
                    }
                }
                else
                { // We don't display the curve, we just take the max-min values:
                    int pos=0;
                    int absIndex;
                    float val[3];
                    CGraphData* number1=getGraphData(it->data[0]);
                    CGraphData* number2=getGraphData(it->data[1]);
                    bool cyclic1,cyclic2;
                    float range1,range2;
                    if (number1!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number1,cyclic1,range1);
                    if (number2!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(number2,cyclic2,range2);
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
                                            float d1=(relMousePos[0]-val[0])*ratio;
                                            float d2=relMousePos[1]-val[1];
                                            float dist=d1*d1+d2*d2;
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
                                            float d1=(relMousePos[0]-val[0])*ratio;
                                            float d2=relMousePos[1]-val[1];
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
        float vOffset=2.0f*graphSize[1]/float(windowSize[1]);
        if (dontRender)
            vOffset=0.0f;
        for (int i=0;i<int(_staticCurves.size());i++)
        {
            if (_staticCurves[i]->getCurveType()==1)
            {
                CStaticGraphCurve* it=_staticCurves[i];
                if (!dontRender)
                {
                    ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    if (it->getLinkPoints())
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glLineWidth(3.0f); // we are tracking that curve!
                        glLineStipple(1,0xE187);
                        glEnable(GL_LINE_STIPPLE);
                    }
                    else
                    {
                        if ((trackingValueIndex==i)&&markSelectedStream&&trackingValueIsStatic)
                            glPointSize(6.0f); // we are tracking that curve!
                        else
                            glPointSize(4.0f);
                    }
                }
                ogl::buffer.clear();
                for (int ka=0;ka<int(it->values.size()/2);ka++)
                {
                    float xVal=it->values[2*ka+0];
                    float yVal=it->values[2*ka+1]+vOffset;
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
                                    float d1=(relMousePos[0]-xVal)*ratio;
                                    float d2=relMousePos[1]-yVal;
                                    float dist=d1*d1+d2*d2;
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
                                    float d1=(relMousePos[0]-xVal)*ratio;
                                    float d2=relMousePos[1]-yVal;
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
                    glPointSize(1.0f);
                    glLineWidth(1.0f);
                    glDisable(GL_LINE_STIPPLE);
                    if (it->getLabel())
                    {
                        tmp=it->getName()+" [STATIC]";
                        float tl=float(ogl::getTextLengthInPixels(tmp))*pixelSizeCoeff;
                        ogl::drawBitmapTextTo2dPosition(labelPos[0]-tl,labelPos[1],tmp);
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
    float sensitivity=30.0f;
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
                float squareSize=5.0f;
                float r[2];
                r[0]=squareSize*graphSize[0]/(float)windowSize[0];
                r[1]=squareSize*graphSize[1]/(float)windowSize[1];
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                ogl::setMaterialColor(sim_colorcomponent_emission,textColor);
                std::string tmp;
                if (subView!=nullptr)
                    subView->setTrackedGraphCurveIndex(-1);
                if (timeGraph)
                {
                    if (!trackingValueIsStatic)
                    {
                        CGraphData* it=daten[trackingValueIndex];
                        tmp=" ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        float l0=r[0]*2.5f;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5f)
                            l0=-r[0]*2.5f-graphSize[0]*float(ogl::getTextLengthInPixels(tmp))/float(windowSize[0]);
                        float l1=r[1]*2.5f;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5f)
                            l1=-r[1]*2.5f;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp);
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                        if (markSelectedStreamSaved&&(subView!=nullptr))
                            subView->setTrackedGraphCurveIndex(trackingValueIndex);
                    }
                    else
                    { // Tracking a static curve here!
                        CStaticGraphCurve* it=_staticCurves[trackingValueIndex];
                        tmp=" [STATIC] ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        float l0=r[0]*2.5f;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5f)
                            l0=-r[0]*2.5f-graphSize[0]*float(ogl::getTextLengthInPixels(tmp))/float(windowSize[0]);
                        float l1=r[1]*2.5f;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5f)
                            l1=-r[1]*2.5f;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp);
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    }
                }
                else
                {
                    if (!trackingValueIsStatic)
                    {
                        CGraphDataComb* it=twoDPartners[trackingValueIndex];
                        tmp=" ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        float l0=r[0]*2.5f;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5f)
                            l0=-r[0]*2.5f-graphSize[0]*float(ogl::getTextLengthInPixels(tmp))/float(windowSize[0]);
                        float l1=r[1]*2.5f;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5f)
                            l1=-r[1]*2.5f;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp);
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->curveColor.colors);
                        if (markSelectedStreamSaved&&(subView!=nullptr))
                            subView->setTrackedGraphCurveIndex(trackingValueIndex);
                    }
                    else
                    { // tracking a static curve here!
                        CStaticGraphCurve* it=_staticCurves[trackingValueIndex];
                        tmp=" [STATIC] ("+tt::getEString(false,trackingValue[0],5)+" ; ";
                        tmp+=tt::getEString(false,trackingValue[1],5)+")";
                        tmp=it->getName()+tmp;
                        float l0=r[0]*2.5f;
                        if (trackingValue[0]-graphPosition[0]>graphSize[0]*0.5f)
                            l0=-r[0]*2.5f-graphSize[0]*float(ogl::getTextLengthInPixels(tmp))/float(windowSize[0]);
                        float l1=r[1]*2.5f;
                        if (trackingValue[1]-graphPosition[1]>graphSize[1]*0.5f)
                            l1=-r[1]*2.5f;
                        ogl::drawBitmapTextTo2dPosition(trackingValue[0]+l0,trackingValue[1]+l1,tmp);
                        ogl::setMaterialColor(sim_colorcomponent_emission,it->ambientColor);
                    }
                }
                ogl::buffer.clear();
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]-r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]+r[0],trackingValue[1]-r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]+r[0],trackingValue[1]+r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]+r[1]);
                ogl::addBuffer2DPoints(trackingValue[0]-r[0],trackingValue[1]-r[1]);
                ogl::drawRandom2dLines(&ogl::buffer[0],(int)ogl::buffer.size()/2,true,0.0f);
                ogl::buffer.clear();
            }
        }
    }

    if (dontRender)
    { // into a running time graph!
        float size[2]={maxVal[0]-minVal[0],maxVal[1]-minVal[1]};
        if (timeGraph)
        {
            if (autoMode)
            { // for the x axis:
                graphPosition[0]=minVal[0]-0.05f*size[0];
                graphSize[0]=1.1f*size[0];
            }
            if (timeGraphYaxisAutoMode)
            { // for the y axis:
                graphPosition[1]=minVal[1]-0.05f*size[1];
                graphSize[1]=1.1f*size[1];
            }
        }
        else
        {
            if (autoMode)
            {
                graphPosition[0]=minVal[0]-0.05f*size[0];
                graphPosition[1]=minVal[1]-0.05f*size[1];
                graphSize[0]=1.1f*size[0];
                graphSize[1]=1.1f*size[1];
            }
        }
        validateViewValues(windowSize,graphPosition,graphSize,timeGraph,false,false,autoMode);
    }
}

void CGraph::validateViewValues(int windowSize[2],float graphPosition[2],float graphSize[2],
            bool timeGraph,bool shiftOnly,bool keepProp,bool autoModeForTimeGraphXaxis)
{ // keepProp is false by default, shiftOnly also
    float minValues[2]={-SIM_MAX_FLOAT,-SIM_MAX_FLOAT};
    float maxValues[2]={+SIM_MAX_FLOAT,+SIM_MAX_FLOAT};
    float minGraphSize[2]={0.00001f,0.00001f};
    if (timeGraph)
    {
        if (autoModeForTimeGraphXaxis)
        {
            if (numberOfPoints<2)
            {
                minValues[0]=0.0f;
                maxValues[0]=1.0f;
                minGraphSize[0]=1.0f;
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
    float maxGraphSize[2]={maxValues[0]-minValues[0],maxValues[1]-minValues[1]};
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
            graphPosition[0]=graphPosition[0]+0.5f*(graphSize[0]-minGraphSize[0]);
        if (graphSize[1]<minGraphSize[1])
            graphPosition[1]=graphPosition[1]+0.5f*(graphSize[1]-minGraphSize[1]);

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
        float prop[2]={((float)windowSize[0])/graphSize[0],((float)windowSize[1])/graphSize[1]};
        float coeff=prop[1]/prop[0];
        float nys=graphSize[1]*coeff;
        float nxs=graphSize[0]/coeff;
        if (coeff>1.0f)
        {   // We have to zoom out on the y-axis
            if (nys<maxGraphSize[1])
            {   // We zoom out on the y-axis
                graphPosition[1]=graphPosition[1]-(nys-graphSize[1])/2.0f;
                if (graphPosition[1]<minValues[1])
                    graphPosition[1]=minValues[1];
                if ((graphPosition[1]+nys)>maxValues[1])
                    graphPosition[1]=graphPosition[1]+maxValues[1]-(graphPosition[1]+nys);
                graphSize[1]=nys;
            }
            else
            {   // We zoom in on the x-axis
                graphPosition[0]=graphPosition[0]-(nxs-graphSize[0])/2.0f;
                graphSize[0]=nxs;
            }
        }
        else
        {   // We have to zoom in on the y-axis
            if (nxs<maxGraphSize[0])
            {   // We zoom out on the x-axis
                graphPosition[0]=graphPosition[0]-(nxs-graphSize[0])/2.0f;
                if (graphPosition[0]<minValues[0])
                    graphPosition[0]=minValues[0];
                if ((graphPosition[0]+nxs)>maxValues[0])
                    graphPosition[0]=graphPosition[0]+maxValues[0]-(graphPosition[0]+nxs);
                graphSize[0]=nxs;
            }
            else
            {   // We zoom in on the y-axis
                graphPosition[1]=graphPosition[1]-(nys-graphSize[1])/2.0f;
                graphSize[1]=nys;
            }
        }
    }
}
#endif


