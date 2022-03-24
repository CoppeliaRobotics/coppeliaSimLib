#include "simInternal.h"
#include "simStrings.h"
#include "ttUtil.h"
#include "tt.h"
#include "pointCloud.h"
#include "pluginContainer.h"
#include "sceneObjectOperations.h"
#include "easyLock.h"
#include "global.h"
#include "vDateTime.h"
#include "app.h"
#include "pointCloudRendering.h"

CPointCloud::CPointCloud()
{
    TRACE_INTERNAL;
    _objectType=sim_object_pointcloud_type;

    _cellSize=0.02f;
    _maxPointCountPerCell=20;
    color.setDefaultValues();
    color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
    _visibilityLayer=POINTCLOUD_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable|sim_objectspecialproperty_renderable;
    _objectAlias=IDSOGL_POINTCLOUD;
    _objectName_old=IDSOGL_POINTCLOUD;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    _pointCloudInfo=nullptr;
    _showOctreeStructure=false;
    _useRandomColors=false;
    _colorIsEmissive=false;
    _saveCalculationStructure=true; // when true, the file size is actually smaller!
    _doNotUseOctreeStructure=false;
    _pointSize=4;
    _buildResolution=0.01f;
    _removalDistanceTolerance=0.01f;
    _insertionDistanceTolerance=0.0;
    _nonEmptyCells=0;
    _pointDisplayRatio=1.0;

    clear(); // also sets the _minDim and _maxDim values
    computeBoundingBox();
}

CPointCloud::~CPointCloud()
{
    TRACE_INTERNAL;
    clear();
}

void CPointCloud::getTransfAndHalfSizeOfBoundingBox(C7Vector& tr,C3Vector& hs) const
{
    hs=(_boundingBoxMax-_boundingBoxMin)*0.5f;
    C4X4Matrix m=getFullCumulativeTransformation().getMatrix();
    C3Vector center((_boundingBoxMin+_boundingBoxMax)*0.5);
    m.X+=m.M*center;
    tr=m.getTransformation();
}

CColorObject* CPointCloud::getColor()
{
    return(&color);
}

std::vector<float>* CPointCloud::getColors()
{
    return(&_colors);
}

std::vector<float>* CPointCloud::getDisplayPoints()
{
    return(&_displayPoints);
}

std::vector<float>* CPointCloud::getDisplayColors()
{
    return(&_displayColors);
}

void CPointCloud::_readPositionsAndColorsAndSetDimensions()
{
    std::vector<float> displayPoints_old;
    std::vector<unsigned char> displayColorsByte_old;
    displayPoints_old.swap(_displayPoints);
    displayColorsByte_old.swap(_displayColorsByte);
    _displayColors.clear();
    bool generateEvent=true;
    if (_doNotUseOctreeStructure)
    {
        _nonEmptyCells=0;
        if (_useRandomColors)
        {
            _colors.clear();
            for (size_t i=0;i<_points.size()/3;i++)
            {
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(1.0f);
            }
        }
        C3Vector minDim,maxDim;
        for (size_t i=0;i<_points.size()/3;i++)
        {
            C3Vector p(&_points[3*i]);
            if (i==0)
            {
                minDim=p;
                maxDim=p;
            }
            else
            {
                minDim.keepMin(p);
                maxDim.keepMax(p);
            }
        }
        /*
        minDim(0)-=_cellSize; // not *0.5 here! The point could lie on the other side of the cube (i.e. not centered)
        minDim(1)-=_cellSize;
        minDim(2)-=_cellSize;
        maxDim(0)+=_cellSize;
        maxDim(1)+=_cellSize;
        maxDim(2)+=_cellSize;
        */
        _setBoundingBox(minDim,maxDim);
        _displayPoints.assign(_points.begin(),_points.end());
        _displayColors.assign(_colors.begin(),_colors.end());
        for (size_t i=0;i<_displayColors.size();i++)
            _displayColorsByte.push_back((unsigned char)(_displayColors[i]*255.1f));
    }
    else
    {
        _points.clear();
        _colors.clear();
        if (_pointCloudInfo!=nullptr)
        {
            _nonEmptyCells=CPluginContainer::geomPlugin_getPtcloudNonEmptyCellCount(_pointCloudInfo);

            CPluginContainer::geomPlugin_getPtcloudPoints(_pointCloudInfo,_points,&_colors);
            if (_pointDisplayRatio<0.99f)
                CPluginContainer::geomPlugin_getPtcloudPoints(_pointCloudInfo,_displayPoints,&_displayColors,_pointDisplayRatio);
            else
            {
                _displayPoints.assign(_points.begin(),_points.end());
                _displayColors.assign(_colors.begin(),_colors.end());
            }
            if (_useRandomColors)
            {
                _colors.clear();
                for (size_t i=0;i<_points.size()/3;i++)
                {
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(1.0f);
                }
                _displayColors.clear();
                for (size_t i=0;i<_displayPoints.size()/3;i++)
                {
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(1.0f);
                }
            }
            for (size_t i=0;i<_displayColors.size();i++)
                _displayColorsByte.push_back((unsigned char)(_displayColors[i]*255.1f));
            C3Vector minDim,maxDim;
            for (size_t i=0;i<_points.size()/3;i++)
            {
                C3Vector p(&_points[3*i]);
                if (i==0)
                {
                    minDim=p;
                    maxDim=p;
                }
                else
                {
                    minDim.keepMin(p);
                    maxDim.keepMax(p);
                }
            }
            /*
            minDim(0)-=_cellSize; // not *0.5 here! The point could lie on the other side of the cube (i.e. not centered)
            minDim(1)-=_cellSize;
            minDim(2)-=_cellSize;
            maxDim(0)+=_cellSize;
            maxDim(1)+=_cellSize;
            maxDim(2)+=_cellSize;
            */
            _setBoundingBox(minDim,maxDim);
        }
        else
        {
            clear();
            generateEvent=false;
        }
    }

    if ( generateEvent&&_isInScene&&App::worldContainer->getEventsEnabled() )
    {
        if (displayPoints_old.size()==_displayPoints.size())
        {
            unsigned char* v=(unsigned char*)_displayPoints.data();
            unsigned char* w=(unsigned char*)displayPoints_old.data();
            unsigned long long vv=0;
            unsigned long long ww=0;
            for (size_t i=0;i<displayPoints_old.size()*4;i++)
            {
                vv+=v[i];
                ww+=w[i];
            }
            if (vv==ww)
            {

                v=_displayColorsByte.data();
                w=displayColorsByte_old.data();
                vv=0;
                ww=0;
                for (size_t i=0;i<_displayColorsByte.size();i++)
                {
                    vv+=v[i];
                    ww+=w[i];
                }
                if (vv==ww)
                    generateEvent=false;
            }
        }
        if (generateEvent)
            _updatePointCloudEvent();
    }
}

void CPointCloud::_updatePointCloudEvent() const
{
    if ( _isInScene&&App::worldContainer->getEventsEnabled() )
    {
        const char* cmd="points";
        auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);

        CInterfaceStackTable* subC=new CInterfaceStackTable();
        data->appendMapObject_stringObject(cmd,subC);
        data=subC;

        CCbor obj(nullptr,0);
        size_t l;
        obj.appendFloatArray(_displayPoints.data(),_displayPoints.size());
        const char* buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("points",buff,l,true);

        obj.clear();
        obj.appendBuff(_displayColorsByte.data(),_displayColorsByte.size());
        buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("colors",buff,l,true);
        App::worldContainer->pushEvent(event);
    }
}

void CPointCloud::_getCharRGB3Colors(const std::vector<float>& floatRGBA,std::vector<unsigned char>& charRGB)
{
    charRGB.resize(floatRGBA.size()*3/4);
    for (size_t i=0;i<floatRGBA.size()/4;i++)
    {
        charRGB[3*i+0]=(unsigned char)(floatRGBA[4*i+0]*255.1f);
        charRGB[3*i+1]=(unsigned char)(floatRGBA[4*i+1]*255.1f);
        charRGB[3*i+2]=(unsigned char)(floatRGBA[4*i+2]*255.1f);
    }
}

int CPointCloud::removePoints(const float* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,float distanceTolerance)
{
    TRACE_INTERNAL;
    int pointCntRemoved=0;
    if (_pointCloudInfo!=nullptr)
    {
        const float* _pts=pts;
        std::vector<float> __pts;
        if (!ptsAreRelativeToPointCloud)
        {
            C7Vector tr(getFullCumulativeTransformation().getInverse());
            for (int i=0;i<ptsCnt;i++)
            {
                C3Vector p(pts+3*i);
                p=tr*p;
                __pts.push_back(p(0));
                __pts.push_back(p(1));
                __pts.push_back(p(2));
            }
            _pts=&__pts[0];
        }
        if (CPluginContainer::geomPlugin_removePointsFromPtcloud(_pointCloudInfo,C7Vector::identityTransformation,_pts,ptsCnt,distanceTolerance,&pointCntRemoved))
        {
            CPluginContainer::geomPlugin_destroyPtcloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
    return(pointCntRemoved);
}

void CPointCloud::subtractOctree(const COctree* octree)
{
    TRACE_INTERNAL;
    if (octree->getOctreeInfo()!=nullptr)
        subtractOctree(octree->getOctreeInfo(),((COctree*)octree)->getFullCumulativeTransformation());
}

void CPointCloud::subtractDummy(const CDummy* dummy,float distanceTolerance)
{
    TRACE_INTERNAL;
    removePoints(dummy->getFullCumulativeTransformation().X.data,1,false,distanceTolerance);
}

void CPointCloud::subtractPointCloud(const CPointCloud* pointCloud,float distanceTolerance)
{
    TRACE_INTERNAL;
    if (pointCloud->getPointCloudInfo()!=nullptr)
    {
        const std::vector<float>* _pts=pointCloud->getPoints();
        C7Vector tr(pointCloud->getFullCumulativeTransformation());
        std::vector<float> pts;
        for (size_t i=0;i<_pts->size()/3;i++)
        {
            C3Vector v(&_pts->at(3*i));
            v*=tr;
            pts.push_back(v(0));
            pts.push_back(v(1));
            pts.push_back(v(2));
        }
        removePoints(&pts[0],(int)pts.size()/3,false,distanceTolerance);
    }
}


void CPointCloud::subtractOctree(const void* octree2Info,const C7Vector& octree2Tr)
{
    TRACE_INTERNAL;
    if (_pointCloudInfo!=nullptr)
    {
        int ptCntRemoved;
        if (CPluginContainer::geomPlugin_removeOctreeFromPtcloud(_pointCloudInfo,getFullCumulativeTransformation(),octree2Info,octree2Tr,&ptCntRemoved))
        {
            CPluginContainer::geomPlugin_destroyPtcloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void CPointCloud::subtractObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            subtractObject(it,_removalDistanceTolerance);
    }
}

void CPointCloud::subtractObject(const CSceneObject* obj,float distanceTolerance)
{
    if (obj->getObjectType()==sim_object_octree_type)
        subtractOctree((COctree*)obj);
    if (obj->getObjectType()==sim_object_dummy_type)
        subtractDummy((CDummy*)obj,distanceTolerance);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        subtractPointCloud((CPointCloud*)obj,distanceTolerance);
}

int CPointCloud::intersectPoints(const float* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,float distanceTolerance)
{
    TRACE_INTERNAL;
    if (_pointCloudInfo!=nullptr)
    {
        const float* _pts=pts;
        std::vector<float> __pts;
        if (!ptsAreRelativeToPointCloud)
        {
            C7Vector tr(getFullCumulativeTransformation().getInverse());
            for (int i=0;i<ptsCnt;i++)
            {
                C3Vector p(pts+3*i);
                p=tr*p;
                __pts.push_back(p(0));
                __pts.push_back(p(1));
                __pts.push_back(p(2));
            }
            _pts=&__pts[0];
        }
        if (CPluginContainer::geomPlugin_intersectPointsWithPtcloud(_pointCloudInfo,C7Vector::identityTransformation,_pts,ptsCnt,distanceTolerance))
        {
            CPluginContainer::geomPlugin_destroyPtcloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
    return(int(_points.size()/3));
}

void CPointCloud::insertPoints(const float* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,const unsigned char* optionalColors3,bool colorsAreIndividual)
{
    TRACE_INTERNAL;
    if (ptsCnt<=0)
        return;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToPointCloud)
    {
        __pts.resize(ptsCnt*3);
        C7Vector tr(getFullCumulativeTransformation().getInverse());
        for (int i=0;i<ptsCnt;i++)
        {
            C3Vector p(pts+3*i);
            p*=tr;
            __pts[3*i+0]=p(0);
            __pts[3*i+1]=p(1);
            __pts[3*i+2]=p(2);
        }
        _pts=&__pts[0];
    }
    if (_doNotUseOctreeStructure)
    {
        _points.insert(_points.end(),_pts,_pts+ptsCnt*3);
        if (optionalColors3==nullptr)
        {
            for (int i=0;i<ptsCnt;i++)
            {
                _colors.push_back(color.getColorsPtr()[0]);
                _colors.push_back(color.getColorsPtr()[1]);
                _colors.push_back(color.getColorsPtr()[2]);
                _colors.push_back(1.0f);
            }
        }
        else
        {
            if (colorsAreIndividual)
            {
                for (int i=0;i<ptsCnt;i++)
                {
                    _colors.push_back(float(optionalColors3[3*i+0])/255.1f);
                    _colors.push_back(float(optionalColors3[3*i+1])/255.1f);
                    _colors.push_back(float(optionalColors3[3*i+2])/255.1f);
                    _colors.push_back(1.0f);
                }
            }
            else
            {
                for (int i=0;i<ptsCnt;i++)
                {
                    _colors.push_back(float(optionalColors3[0])/255.1f);
                    _colors.push_back(float(optionalColors3[1])/255.1f);
                    _colors.push_back(float(optionalColors3[2])/255.1f);
                    _colors.push_back(1.0f);
                }
            }
        }
    }
    else
    {
        if (_pointCloudInfo==nullptr)
        {
            if (optionalColors3==nullptr)
            {
                unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
                _pointCloudInfo=CPluginContainer::geomPlugin_createPtcloudFromPoints(_pts,ptsCnt,nullptr,_cellSize,_maxPointCountPerCell,cols,_insertionDistanceTolerance);
            }
            else
            {
                if (colorsAreIndividual)
                    _pointCloudInfo=CPluginContainer::geomPlugin_createPtcloudFromColorPoints(_pts,ptsCnt,nullptr,_cellSize,_maxPointCountPerCell,optionalColors3,_insertionDistanceTolerance);
                else
                    _pointCloudInfo=CPluginContainer::geomPlugin_createPtcloudFromPoints(_pts,ptsCnt,nullptr,_cellSize,_maxPointCountPerCell,optionalColors3,_insertionDistanceTolerance);
            }
        }
        else
        {
            if (optionalColors3==nullptr)
            {
                unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
                CPluginContainer::geomPlugin_insertPointsIntoPtcloud(_pointCloudInfo,C7Vector::identityTransformation,_pts,ptsCnt,cols,_insertionDistanceTolerance);
            }
            else
            {
                if (colorsAreIndividual)
                    CPluginContainer::geomPlugin_insertColorPointsIntoPtcloud(_pointCloudInfo,C7Vector::identityTransformation,_pts,ptsCnt,optionalColors3,_insertionDistanceTolerance);
                else
                    CPluginContainer::geomPlugin_insertPointsIntoPtcloud(_pointCloudInfo,C7Vector::identityTransformation,_pts,ptsCnt,optionalColors3,_insertionDistanceTolerance);
            }
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void CPointCloud::insertShape(CShape* shape)
{
    TRACE_INTERNAL;
    // We first build an octree from the shape, then insert the octree cube points:
    shape->initializeMeshCalculationStructureIfNeeded();
    C4X4Matrix m(getFullCumulativeTransformation().getMatrix());
    unsigned char dummyColor[3];
    const C7Vector tr(getFullCumulativeTransformation());
    void* octree=CPluginContainer::geomPlugin_createOctreeFromMesh(shape->_meshCalculationStructure,shape->getFullCumulativeTransformation(),&tr,_buildResolution,dummyColor,0);
    std::vector<float> pts;
    CPluginContainer::geomPlugin_getOctreeVoxelPositions(octree,pts);
    CPluginContainer::geomPlugin_destroyOctree(octree);
    insertPoints(&pts[0],(int)pts.size()/3,true,nullptr,false);
}

void CPointCloud::insertOctree(const COctree* octree)
{
    TRACE_INTERNAL;
    if (octree->getOctreeInfo()!=nullptr)
    {
        const std::vector<float>* _pts=octree->getCubePositions();
        C7Vector tr(octree->getFullCumulativeTransformation());
        std::vector<float> pts;
        for (size_t i=0;i<_pts->size()/3;i++)
        {
            C3Vector v(&_pts->at(3*i));
            v*=tr;
            pts.push_back(v(0));
            pts.push_back(v(1));
            pts.push_back(v(2));
        }
        insertPoints(&pts[0],(int)pts.size()/3,false,nullptr,false);
    }
}

void CPointCloud::insertDummy(const CDummy* dummy)
{
    TRACE_INTERNAL;
    insertPoints(dummy->getFullCumulativeTransformation().X.data,1,false,nullptr,false);
}

void CPointCloud::insertPointCloud(const CPointCloud* pointCloud)
{
    TRACE_INTERNAL;
    const std::vector<float>* _pts=pointCloud->getPoints();
    std::vector<unsigned char> _cols;
    _cols.resize(_pts->size());

    C7Vector tr(pointCloud->getFullCumulativeTransformation());
    std::vector<float> pts;
    for (size_t i=0;i<_pts->size()/3;i++)
    {
        C3Vector v(&_pts->at(3*i));
        v*=tr;
        pts.push_back(v(0));
        pts.push_back(v(1));
        pts.push_back(v(2));
        _cols[3*i+0]=(unsigned char)(pointCloud->_colors[4*i+0]*255.1f);
        _cols[3*i+1]=(unsigned char)(pointCloud->_colors[4*i+1]*255.1f);
        _cols[3*i+2]=(unsigned char)(pointCloud->_colors[4*i+2]*255.1f);
    }
    insertPoints(&pts[0],(int)pts.size()/3,false,&_cols[0],true);
}

void CPointCloud::insertObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            insertObject(it);
    }
}

void CPointCloud::insertObject(const CSceneObject* obj)
{
    if (obj->getObjectType()==sim_object_shape_type)
        insertShape((CShape*)obj);
    if (obj->getObjectType()==sim_object_octree_type)
        insertOctree((COctree*)obj);
    if (obj->getObjectType()==sim_object_dummy_type)
        insertDummy((CDummy*)obj);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        insertPointCloud((CPointCloud*)obj);
}

void CPointCloud::clear()
{
    TRACE_INTERNAL;
    _points.clear();
    _colors.clear();
    _displayPoints.clear();
    _displayColors.clear();
    _displayColorsByte.clear();
    if (_pointCloudInfo!=nullptr)
    {
        CPluginContainer::geomPlugin_destroyPtcloud(_pointCloudInfo);
        _pointCloudInfo=nullptr;
    }
    _setBoundingBox(C3Vector(-0.1f,-0.1f,-0.1f),C3Vector(+0.1f,+0.1f,+0.1f));
    _nonEmptyCells=0;
    _updatePointCloudEvent();
}

const std::vector<float>* CPointCloud::getPoints() const
{
    TRACE_INTERNAL;
    return(&_points);
}

std::vector<float>* CPointCloud::getPoints()
{
    TRACE_INTERNAL;
    return(&_points);
}

const void* CPointCloud::getPointCloudInfo() const
{
    return(_pointCloudInfo);
}

void* CPointCloud::getPointCloudInfo()
{
    return(_pointCloudInfo);
}

std::string CPointCloud::getObjectTypeInfo() const
{
    return(IDSOGL_POINTCLOUD);
}

std::string CPointCloud::getObjectTypeInfoExtended() const
{
    return(IDSOGL_POINTCLOUD);
}
bool CPointCloud::isPotentiallyCollidable() const
{
    return(!_doNotUseOctreeStructure);
}
bool CPointCloud::isPotentiallyMeasurable() const
{
    return(!_doNotUseOctreeStructure);
}
bool CPointCloud::isPotentiallyDetectable() const
{
    return(!_doNotUseOctreeStructure);
}
bool CPointCloud::isPotentiallyRenderable() const
{
    return(true);
}

void CPointCloud::computeBoundingBox()
{ // handled elsewhere
}

void CPointCloud::scaleObject(float scalingFactor)
{
    _cellSize*=scalingFactor;
    _buildResolution*=scalingFactor;
    _removalDistanceTolerance*=scalingFactor;
    _insertionDistanceTolerance*=scalingFactor;
    _setBoundingBox(_boundingBoxMin*scalingFactor,_boundingBoxMax*scalingFactor);
    for (size_t i=0;i<_points.size();i++)
        _points[i]*=scalingFactor;
    for (size_t i=0;i<_displayPoints.size();i++)
        _displayPoints[i]*=scalingFactor;
    if (_pointCloudInfo!=nullptr)
        CPluginContainer::geomPlugin_scalePtcloud(_pointCloudInfo,scalingFactor);
    _updatePointCloudEvent();
    CSceneObject::scaleObject(scalingFactor);
}

void CPointCloud::scaleObjectNonIsometrically(float x,float y,float z)
{
    float s=cbrt(x*y*z);
    scaleObject(s);
}

void CPointCloud::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CPointCloud::addSpecializedObjectEventData(CInterfaceStackTable* data) const
{
    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("pointCloud",subC);
    data=subC;

    data->appendMapObject_stringInt32("pointSize",_pointSize);

    subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("points",subC);
    data=subC;

    CCbor obj(nullptr,0);
    size_t l;
    obj.appendFloatArray(_displayPoints.data(),_displayPoints.size());
    const char* buff=(const char*)obj.getBuff(l);
    data->appendMapObject_stringString("points",buff,l,true);

    obj.clear();
    obj.appendBuff(_displayColorsByte.data(),_displayColorsByte.size());
    buff=(const char*)obj.getBuff(l);
    data->appendMapObject_stringString("colors",buff,l,true);
}

CSceneObject* CPointCloud::copyYourself()
{   
    CPointCloud* newPointcloud=(CPointCloud*)CSceneObject::copyYourself();

    newPointcloud->_cellSize=_cellSize;
    newPointcloud->_maxPointCountPerCell=_maxPointCountPerCell;
    color.copyYourselfInto(&newPointcloud->color);

    if (_pointCloudInfo!=nullptr)
        newPointcloud->_pointCloudInfo=CPluginContainer::geomPlugin_copyPtcloud(_pointCloudInfo);
    newPointcloud->_points.assign(_points.begin(),_points.end());
    newPointcloud->_colors.assign(_colors.begin(),_colors.end());
    newPointcloud->_displayPoints.assign(_displayPoints.begin(),_displayPoints.end());
    newPointcloud->_displayColors.assign(_displayColors.begin(),_displayColors.end());
    newPointcloud->_displayColorsByte.assign(_displayColorsByte.begin(),_displayColorsByte.end());
    newPointcloud->_showOctreeStructure=_showOctreeStructure;
    newPointcloud->_useRandomColors=_useRandomColors;
    newPointcloud->_colorIsEmissive=_colorIsEmissive;
    newPointcloud->_saveCalculationStructure=_saveCalculationStructure;
    newPointcloud->_pointSize=_pointSize;
    newPointcloud->_nonEmptyCells=_nonEmptyCells;
    newPointcloud->_doNotUseOctreeStructure=_doNotUseOctreeStructure;
    newPointcloud->_buildResolution=_buildResolution;
    newPointcloud->_removalDistanceTolerance=_removalDistanceTolerance;
    newPointcloud->_insertionDistanceTolerance=_insertionDistanceTolerance;
    newPointcloud->_pointDisplayRatio=_pointDisplayRatio;

    return(newPointcloud);
}

void CPointCloud::setCellSize(float theNewSize)
{
    theNewSize=tt::getLimitedFloat(0.001f,1.0,theNewSize);
    if (theNewSize!=_cellSize)
    {
        _cellSize=theNewSize;
        std::vector<float> pts(_points);
        std::vector<unsigned char> cols;
        _getCharRGB3Colors(_colors,cols);
        clear();
        if (pts.size()>0)
            insertPoints(&pts[0],(int)pts.size()/3,true,&cols[0],true);
    }
}

float CPointCloud::getCellSize() const
{ 
    return(_cellSize);
}

void CPointCloud::setMaxPointCountPerCell(int cnt)
{
    cnt=tt::getLimitedInt(1,100,cnt);
    if (cnt!=_maxPointCountPerCell)
    {
        _maxPointCountPerCell=cnt;
        std::vector<float> pts(_points);
        std::vector<unsigned char> cols;
        _getCharRGB3Colors(_colors,cols);
        clear();
        if (pts.size()>0)
            insertPoints(&pts[0],(int)pts.size()/3,true,&cols[0],true);
    }
}

int CPointCloud::getMaxPointCountPerCell() const
{
    return(_maxPointCountPerCell);
}

bool CPointCloud::getShowOctree() const
{
    return(_showOctreeStructure);
}

void CPointCloud::setShowOctree(bool show)
{
    _showOctreeStructure=show;
}

float CPointCloud::getAveragePointCountInCell()
{
    if ( (_points.size()==0)||_doNotUseOctreeStructure )
        return(-1.0);
    return(float(_points.size()/3)/float(_nonEmptyCells));
}

int CPointCloud::getPointSize() const
{
    return(_pointSize);
}

void CPointCloud::setPointSize(int s)
{
    s=tt::getLimitedInt(1,8,s);
    bool diff=(_pointSize!=s);
    if (diff)
    {
        _pointSize=s;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="pointSize";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
            data->appendMapObject_stringInt32(cmd,_pointSize);
            App::worldContainer->pushEvent(event);
        }
    }
}

float CPointCloud::getBuildResolution() const
{
    return(_buildResolution);
}

void CPointCloud::setBuildResolution(float r)
{
    _buildResolution=tt::getLimitedFloat(0.001f,1.0,r);
}

float CPointCloud::getRemovalDistanceTolerance() const
{
    return(_removalDistanceTolerance);
}

void CPointCloud::setRemovalDistanceTolerance(float t)
{
    _removalDistanceTolerance=tt::getLimitedFloat(0.0001f,1.0,t);
}

float CPointCloud::getInsertionDistanceTolerance() const
{
    return(_insertionDistanceTolerance);
}

void CPointCloud::setInsertionDistanceTolerance(float t)
{
    _insertionDistanceTolerance=tt::getLimitedFloat(0.0,1.0,t);
}

bool CPointCloud::getUseRandomColors() const
{
    return(_useRandomColors);
}

void CPointCloud::setUseRandomColors(bool r)
{
    if (r!=_useRandomColors)
    {
        _useRandomColors=r;
        _readPositionsAndColorsAndSetDimensions();
    }
}

bool CPointCloud::getColorIsEmissive() const
{
    return(_colorIsEmissive);
}

void CPointCloud::setColorIsEmissive(bool e)
{
    _colorIsEmissive=e;
}

bool CPointCloud::getSaveCalculationStructure() const
{
    return(_saveCalculationStructure);
}

void CPointCloud::setSaveCalculationStructure(bool s)
{
    _saveCalculationStructure=s;
}

bool CPointCloud::getDoNotUseCalculationStructure() const
{
    return(_doNotUseOctreeStructure);
}

void CPointCloud::setDoNotUseCalculationStructure(bool s)
{
    if (s!=_doNotUseOctreeStructure)
    {
        _doNotUseOctreeStructure=s;
        if (_points.size()>0)
        {
            std::vector<float> p(_points);
            std::vector<unsigned char> c;
            c.reserve(p.size());
            for (size_t i=0;i<p.size()/3;i++)
            {
                c[3*i+0]=(unsigned char)(_colors[4*i+0]*255.1f);
                c[3*i+1]=(unsigned char)(_colors[4*i+1]*255.1f);
                c[3*i+2]=(unsigned char)(_colors[4*i+2]*255.1f);
            }
            clear();
            insertPoints(&p[0],(int)p.size()/3,true,&c[0],true);
        }
    }
}

float CPointCloud::getPointDisplayRatio() const
{
    return(_pointDisplayRatio);
}

void CPointCloud::setPointDisplayRatio(float r)
{
    r=tt::getLimitedFloat(0.01f,1.0,r);
    if (r!=_pointDisplayRatio)
    {
        _pointDisplayRatio=r;
        _readPositionsAndColorsAndSetDimensions();
    }
}

void CPointCloud::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CPointCloud::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CPointCloud::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}

void CPointCloud::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}
void CPointCloud::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CPointCloud::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CPointCloud::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}

void CPointCloud::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CPointCloud::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CPointCloud::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void CPointCloud::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CPointCloud::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
        }
    }
    CSceneObject::simulationEnded();
}

void CPointCloud::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Siz");
            ar << _cellSize << _pointSize;
            ar.flush();

            ar.storeDataName("Sut");
            ar << _removalDistanceTolerance;
            ar.flush();

            ar.storeDataName("adt");
            ar << _insertionDistanceTolerance;
            ar.flush();

            ar.storeDataName("Nec");
            ar << _nonEmptyCells;
            ar.flush();

            ar.storeDataName("Pdr");
            ar << _pointDisplayRatio;
            ar.flush();

            ar.storeDataName("Pcc");
            ar << _maxPointCountPerCell << _buildResolution;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,1,_showOctreeStructure);
            SIM_SET_CLEAR_BIT(dummy,2,_useRandomColors);
            SIM_SET_CLEAR_BIT(dummy,3,_saveCalculationStructure);
            SIM_SET_CLEAR_BIT(dummy,4,_doNotUseOctreeStructure);
            SIM_SET_CLEAR_BIT(dummy,5,_colorIsEmissive);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            if ( (!_saveCalculationStructure)||_doNotUseOctreeStructure )
            {
                ar.storeDataName("Pt2");
                ar << int(_points.size()/3);
                for (size_t i=0;i<_points.size()/3;i++)
                {
                    ar << _points[3*i+0];
                    ar << _points[3*i+1];
                    ar << _points[3*i+2];
                    ar << (unsigned char)(_colors[4*i+0]*255.1f);
                    ar << (unsigned char)(_colors[4*i+1]*255.1f);
                    ar << (unsigned char)(_colors[4*i+2]*255.1f);
                }
                ar.flush();
            }
            else
            {
                if (_pointCloudInfo!=nullptr)
                {
                    ar.storeDataName("Mmd");
                    ar << _boundingBoxMin(0) << _boundingBoxMin(1) << _boundingBoxMin(2);
                    ar << _boundingBoxMax(0) << _boundingBoxMax(1) << _boundingBoxMax(2);
                    ar.flush();

                    std::vector<unsigned char> data;
                    CPluginContainer::geomPlugin_getPtcloudSerializationData(_pointCloudInfo,data);
                    ar.storeDataName("Co2");
                    ar.setCountingMode(true);
                    for (size_t i=0;i<data.size();i++)
                        ar << data[i];
                    ar.flush(false);
                    if (ar.setWritingMode(true))
                    {
                        for (size_t i=0;i<data.size();i++)
                            ar << data[i];
                        ar.flush(false);
                    }
                }
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Siz")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _cellSize >> _pointSize;
                    }
                    if (theName.compare("Sut")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _removalDistanceTolerance;
                    }
                    if (theName.compare("adt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _insertionDistanceTolerance;
                    }
                    if (theName.compare("Nec")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _nonEmptyCells;
                    }
                    if (theName.compare("Pdr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _pointDisplayRatio;
                    }
                    if (theName.compare("Pcc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxPointCountPerCell >> _buildResolution;
                    }
                    if (theName.compare("Pt2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        std::vector<float> pts;
                        pts.resize(cnt*3);
                        std::vector<unsigned char> cols;
                        cols.resize(cnt*3);
                        for (int i=0;i<cnt;i++)
                        {
                            ar >> pts[3*i+0];
                            ar >> pts[3*i+1];
                            ar >> pts[3*i+2];
                            ar >> cols[3*i+0];
                            ar >> cols[3*i+1];
                            ar >> cols[3*i+2];
                        }
                        // Now we need to rebuild the pointCloud:
                        if (cnt>0)
                            insertPoints(&pts[0],cnt,true,&cols[0],true);
                        else
                            clear();
                    }
                    if (theName.compare("Mmd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _boundingBoxMin(0) >> _boundingBoxMin(1) >> _boundingBoxMin(2);
                        ar >> _boundingBoxMax(0) >> _boundingBoxMax(1) >> _boundingBoxMax(2);
                    }

                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _showOctreeStructure=SIM_IS_BIT_SET(dummy,1);
                        _useRandomColors=SIM_IS_BIT_SET(dummy,2);
                        _saveCalculationStructure=SIM_IS_BIT_SET(dummy,3);
                        _doNotUseOctreeStructure=SIM_IS_BIT_SET(dummy,4);
                        _colorIsEmissive=SIM_IS_BIT_SET(dummy,5);
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Co2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo never stores calc structures)

                        std::vector<unsigned char> data;
                        data.reserve(byteQuantity);
                        unsigned char dummy;
                        for (int i=0;i<byteQuantity;i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        _pointCloudInfo=CPluginContainer::geomPlugin_getPtcloudFromSerializationData(&data[0]);
                        _readPositionsAndColorsAndSetDimensions();
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("cellSize",_cellSize);
            ar.xmlAddNode_int("pointSize",_pointSize);

            if (exhaustiveXml)
                ar.xmlAddNode_int("occupiedCells",_nonEmptyCells);

            ar.xmlAddNode_int("maxPointsPerCell",_maxPointCountPerCell);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showStructure",_showOctreeStructure);
            ar.xmlAddNode_bool("randomColors",_useRandomColors);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("saveCalculationStructure",_saveCalculationStructure);
            ar.xmlAddNode_bool("emissiveColor",_colorIsEmissive);
            ar.xmlAddNode_bool("useOctreeStructure",!_doNotUseOctreeStructure);
            ar.xmlPopNode();

            ar.xmlPushNewNode("tolerances");
            ar.xmlAddNode_float("insertion",_insertionDistanceTolerance);
            ar.xmlAddNode_float("removal",_removalDistanceTolerance);
            ar.xmlPopNode();

            ar.xmlAddNode_float("buildResolution",_buildResolution);
            ar.xmlAddNode_float("displayRatio",_pointDisplayRatio);

            ar.xmlPushNewNode("applyColor");
            if (exhaustiveXml)
                color.serialize(ar,0);
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(color.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("points",rgb,3);
            }
            ar.xmlPopNode();

            if (exhaustiveXml)
                ar.xmlAddNode_int("pointCount",int(_points.size()/3));

            if (ar.xmlSaveDataInline(_points.size()*4+_colors.size()*3/4)||(!exhaustiveXml))
            {
                ar.xmlAddNode_floats("points",_points);
                std::vector<int> tmp;
                for (size_t i=0;i<_points.size()/3;i++)
                {
                    tmp.push_back((unsigned int)(_colors[4*i+0]*255.1f));
                    tmp.push_back((unsigned int)(_colors[4*i+1]*255.1f));
                    tmp.push_back((unsigned int)(_colors[4*i+2]*255.1f));
                }
                ar.xmlAddNode_ints("pointColors",tmp);
            }
            else
            {
//                CSer* w=ar.xmlAddNode_binFile("file",(std::string("ptcloud_")+_objectName).c_str());
                CSer* w=ar.xmlAddNode_binFile("file",(_objectAlias+"-ptcloud-"+std::to_string(_objectHandle)).c_str());
                w[0] << int(_points.size());
                for (size_t i=0;i<_points.size();i++)
                    w[0] << _points[i];

                for (size_t i=0;i<_points.size()/3;i++)
                {
                    w[0] << (unsigned char)(_colors[4*i+0]*255.1f);
                    w[0] << (unsigned char)(_colors[4*i+1]*255.1f);
                    w[0] << (unsigned char)(_colors[4*i+2]*255.1f);
                }
                w->flush();
                w->writeClose();
                delete w;
            }
        }
        else
        {
            ar.xmlGetNode_float("cellSize",_cellSize,exhaustiveXml);
            ar.xmlGetNode_int("pointSize",_pointSize,exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_int("occupiedCells",_nonEmptyCells);

            ar.xmlGetNode_int("maxPointsPerCell",_maxPointCountPerCell,exhaustiveXml);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("showStructure",_showOctreeStructure,exhaustiveXml);
                ar.xmlGetNode_bool("randomColors",_useRandomColors,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("saveCalculationStructure",_saveCalculationStructure);
                ar.xmlGetNode_bool("emissiveColor",_colorIsEmissive,exhaustiveXml);
                if (ar.xmlGetNode_bool("useOctreeStructure",_doNotUseOctreeStructure,exhaustiveXml))
                    _doNotUseOctreeStructure=!_doNotUseOctreeStructure;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("tolerances",exhaustiveXml))
            {
                ar.xmlGetNode_float("insertion",_insertionDistanceTolerance,exhaustiveXml);
                ar.xmlGetNode_float("removal",_removalDistanceTolerance,exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_float("buildResolution",_buildResolution,exhaustiveXml);
            ar.xmlGetNode_float("displayRatio",_pointDisplayRatio,exhaustiveXml);

            if (ar.xmlPushChildNode("applyColor",exhaustiveXml))
            {
                if (exhaustiveXml)
                    color.serialize(ar,0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object",rgb,3,exhaustiveXml))
                        color.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                int ptCnt=0;
                ar.xmlGetNode_int("pointCount",ptCnt);

                std::vector<float> pts;
                std::vector<unsigned char> cols;
                if (ar.xmlGetNode_floats("points",pts,false))
                    ar.xmlGetNode_uchars("pointColors",cols);
                else
                {
                    CSer* w=ar.xmlGetNode_binFile("file");
                    int cnt;
                    w[0] >> cnt;
                    pts.resize(cnt);
                    for (int i=0;i<cnt;i++)
                        w[0] >> pts[i];
                    cols.resize(cnt);
                    for (int i=0;i<cnt;i++)
                        w[0] >> cols[i];
                    w->readClose();
                    delete w;
                }
                if (ptCnt>0)
                    insertPoints(&pts[0],ptCnt,true,&cols[0],true);
                else
                    clear();
            }
            else
            {
                std::vector<float> pts;
                std::vector<unsigned char> cols;
                if (ar.xmlGetNode_floats("points",pts,exhaustiveXml))
                    ar.xmlGetNode_uchars("pointColors",cols,exhaustiveXml);
                if (pts.size()>=3)
                {
                    while ((pts.size() % 3)!=0)
                        pts.pop_back();
                    if (cols.size()<pts.size())
                    {
                        cols.resize(pts.size());
                        for (size_t i=0;i<pts.size()/3;i++)
                        {
                            cols[3*i+0]=(unsigned char)(color.getColorsPtr()[0]*255.1f);
                            cols[3*i+1]=(unsigned char)(color.getColorsPtr()[1]*255.1f);
                            cols[3*i+2]=(unsigned char)(color.getColorsPtr()[2]*255.1f);
                        }
                    }
                    insertPoints(&pts[0],pts.size()/3,true,&cols[0],true);
                }
                else
                    clear();
            }
            computeBoundingBox();
        }
    }
}

void CPointCloud::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
}

void CPointCloud::announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object,copyBuffer);
}

void CPointCloud::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CPointCloud::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayPointCloud(this,renderingObject,displayAttrib);
}
