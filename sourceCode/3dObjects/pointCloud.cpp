
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "v_repStrings.h"
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
    FUNCTION_DEBUG;
    _objectType=sim_object_pointcloud_type;

    _cellSize=0.02f;
    _maxPointCountPerCell=20;
    color.setDefaultValues();
    color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
    layer=POINTCLOUD_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable;
    _objectName=IDSOGL_POINTCLOUD;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
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
}

CPointCloud::~CPointCloud()
{
    FUNCTION_DEBUG;
    clear();
}

void CPointCloud::getMatrixAndHalfSizeOfBoundingBox(C4X4Matrix& m,C3Vector& hs) const
{
    hs=(_maxDim-_minDim)*0.5f;
    m=getCumulativeTransformation().getMatrix();
    C3Vector center((_minDim+_maxDim)*0.5);
    m.X+=m.M*center;
}

CVisualParam* CPointCloud::getColor()
{
    return(&color);
}

void CPointCloud::getMaxMinDims(C3Vector& ma,C3Vector& mi) const
{
    ma=_maxDim;
    mi=_minDim;
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
    _displayPoints.clear();
    _displayColors.clear();
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
                _colors.push_back(0.0);
            }
        }

        for (size_t i=0;i<_points.size()/3;i++)
        {
            C3Vector p(&_points[3*i]);
            if (i==0)
            {
                _minDim=p;
                _maxDim=p;
            }
            else
            {
                _minDim.keepMin(p);
                _maxDim.keepMax(p);
            }
        }
        /*
        _minDim(0)-=_cellSize; // not *0.5 here! The point could lie on the other side of the cube (i.e. not centered)
        _minDim(1)-=_cellSize;
        _minDim(2)-=_cellSize;
        _maxDim(0)+=_cellSize;
        _maxDim(1)+=_cellSize;
        _maxDim(2)+=_cellSize;
        */
    }
    else
    {
        _points.clear();
        _colors.clear();
        if (_pointCloudInfo!=nullptr)
        {
            _nonEmptyCells=CPluginContainer::mesh_getPointCloudNonEmptyCellCount(_pointCloudInfo);

            CPluginContainer::mesh_getPointCloudPointData(_pointCloudInfo,_points,_colors);
            if (_pointDisplayRatio<0.99f)
                CPluginContainer::mesh_getPartialPointCloudPointData(_pointCloudInfo,_displayPoints,_displayColors,_pointDisplayRatio);
            if (_useRandomColors)
            {
                _colors.clear();
                for (size_t i=0;i<_points.size()/3;i++)
                {
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _colors.push_back(0.0);
                }
                _displayColors.clear();
                for (size_t i=0;i<_displayPoints.size()/3;i++)
                {
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                    _displayColors.push_back(0.0);
                }
            }

            for (size_t i=0;i<_points.size()/3;i++)
            {
                C3Vector p(&_points[3*i]);
                if (i==0)
                {
                    _minDim=p;
                    _maxDim=p;
                }
                else
                {
                    _minDim.keepMin(p);
                    _maxDim.keepMax(p);
                }
            }
            /*
            _minDim(0)-=_cellSize; // not *0.5 here! The point could lie on the other side of the cube (i.e. not centered)
            _minDim(1)-=_cellSize;
            _minDim(2)-=_cellSize;
            _maxDim(0)+=_cellSize;
            _maxDim(1)+=_cellSize;
            _maxDim(2)+=_cellSize;
            */
        }
        else
            clear();
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
    FUNCTION_DEBUG;
    int pointCntRemoved=0;
    if (_pointCloudInfo!=nullptr)
    {
        const float* _pts=pts;
        std::vector<float> __pts;
        if (!ptsAreRelativeToPointCloud)
        {
            C7Vector tr(getCumulativeTransformation().getInverse());
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
        if (CPluginContainer::mesh_removePointCloudPoints(_pointCloudInfo,_pts,ptsCnt,distanceTolerance,pointCntRemoved))
        {
            CPluginContainer::mesh_destroyPointCloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
    return(pointCntRemoved);
}

void CPointCloud::subtractOctree(const COctree* octree)
{
    FUNCTION_DEBUG;
    if (octree->getOctreeInfo()!=nullptr)
        subtractOctree(octree->getOctreeInfo(),((COctree*)octree)->getCumulativeTransformation().getMatrix());
}

void CPointCloud::subtractDummy(const CDummy* dummy,float distanceTolerance)
{
    FUNCTION_DEBUG;
    removePoints(dummy->getCumulativeTransformation().X.data,1,false,distanceTolerance);
}

void CPointCloud::subtractPointCloud(const CPointCloud* pointCloud,float distanceTolerance)
{
    FUNCTION_DEBUG;
    if (pointCloud->getPointCloudInfo()!=nullptr)
    {
        const std::vector<float>* _pts=pointCloud->getPoints();
        C7Vector tr(pointCloud->getCumulativeTransformation());
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


void CPointCloud::subtractOctree(const void* octree2Info,const C4X4Matrix& octree2CTM)
{
    FUNCTION_DEBUG;
    if (_pointCloudInfo!=nullptr)
    {
        C4X4Matrix pointCloudM(getCumulativeTransformation().getMatrix());
        int ptCntRemoved;
        if (CPluginContainer::mesh_removePointCloudPointsFromOctree(_pointCloudInfo,pointCloudM,octree2Info,octree2CTM,ptCntRemoved))
        {
            CPluginContainer::mesh_destroyPointCloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void CPointCloud::subtractObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            subtractObject(it,_removalDistanceTolerance);
    }
}

void CPointCloud::subtractObject(const C3DObject* obj,float distanceTolerance)
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
    FUNCTION_DEBUG;
    if (_pointCloudInfo!=nullptr)
    {
        const float* _pts=pts;
        std::vector<float> __pts;
        if (!ptsAreRelativeToPointCloud)
        {
            C7Vector tr(getCumulativeTransformation().getInverse());
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
        if (CPluginContainer::mesh_intersectPointCloudPoints(_pointCloudInfo,_pts,ptsCnt,distanceTolerance))
        {
            CPluginContainer::mesh_destroyPointCloud(_pointCloudInfo);
            _pointCloudInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
    return(int(_points.size()/3));
}

void CPointCloud::insertPoints(const float* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,const unsigned char* optionalColors3,bool colorsAreIndividual)
{
    FUNCTION_DEBUG;
    if (ptsCnt<=0)
        return;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToPointCloud)
    {
        __pts.resize(ptsCnt*3);
        C7Vector tr(getCumulativeTransformation().getInverse());
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
                _colors.push_back(color.colors[0]);
                _colors.push_back(color.colors[1]);
                _colors.push_back(color.colors[2]);
                _colors.push_back(0.0);
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
                    _colors.push_back(0.0);
                }
            }
            else
            {
                for (int i=0;i<ptsCnt;i++)
                {
                    _colors.push_back(float(optionalColors3[0])/255.1f);
                    _colors.push_back(float(optionalColors3[1])/255.1f);
                    _colors.push_back(float(optionalColors3[2])/255.1f);
                    _colors.push_back(0.0);
                }
            }
        }
    }
    else
    {
        if (_pointCloudInfo==nullptr)
        {
            if (optionalColors3==nullptr)
                _pointCloudInfo=CPluginContainer::mesh_createPointCloud(_pts,ptsCnt,_cellSize,_maxPointCountPerCell,color.colors,_insertionDistanceTolerance);
            else
            {
                if (colorsAreIndividual)
                    _pointCloudInfo=CPluginContainer::mesh_createColorPointCloud(_pts,ptsCnt,_cellSize,_maxPointCountPerCell,optionalColors3,_insertionDistanceTolerance);
                else
                {
                    const float c[3]={float(optionalColors3[0])/255.1f,float(optionalColors3[1])/255.1f,float(optionalColors3[2])/255.1f};
                    _pointCloudInfo=CPluginContainer::mesh_createPointCloud(_pts,ptsCnt,_cellSize,_maxPointCountPerCell,c,_insertionDistanceTolerance);
                }
            }
        }
        else
        {
            if (optionalColors3==nullptr)
                CPluginContainer::mesh_insertPointsIntoPointCloud(_pointCloudInfo,_pts,ptsCnt,color.colors,_insertionDistanceTolerance);
            else
            {
                if (colorsAreIndividual)
                    CPluginContainer::mesh_insertColorPointsIntoPointCloud(_pointCloudInfo,_pts,ptsCnt,optionalColors3,_insertionDistanceTolerance);
                else
                {
                    const float c[3]={float(optionalColors3[0])/255.1f,float(optionalColors3[1])/255.1f,float(optionalColors3[2])/255.1f};
                    CPluginContainer::mesh_insertPointsIntoPointCloud(_pointCloudInfo,_pts,ptsCnt,c,_insertionDistanceTolerance);
                }
            }
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void CPointCloud::insertShape(const CShape* shape)
{
    FUNCTION_DEBUG;
    // We first build an octree from the shape, then insert the octree cube points:
    ((CShape*)shape)->geomData->initializeCalculationStructureIfNeeded();
    C4X4Matrix m(getCumulativeTransformation().getMatrix());
    float dummyColor[3];
    void* octree=CPluginContainer::mesh_createOctreeFromShape(m,shape->geomData->collInfo,shape->getCumulativeTransformation().getMatrix(),_buildResolution,dummyColor,0);
    std::vector<float> pts;
    std::vector<float> cols;
    CPluginContainer::mesh_getOctreeVoxels(octree,pts,cols);
    CPluginContainer::mesh_destroyOctree(octree);
    insertPoints(&pts[0],(int)pts.size()/3,true,nullptr,false);
}

void CPointCloud::insertOctree(const COctree* octree)
{
    FUNCTION_DEBUG;
    if (octree->getOctreeInfo()!=nullptr)
    {
        const std::vector<float>* _pts=octree->getCubePositions();
        C7Vector tr(octree->getCumulativeTransformation());
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
    FUNCTION_DEBUG;
    insertPoints(dummy->getCumulativeTransformation().X.data,1,false,nullptr,false);
}

void CPointCloud::insertPointCloud(const CPointCloud* pointCloud)
{
    FUNCTION_DEBUG;
    const std::vector<float>* _pts=pointCloud->getPoints();
    std::vector<unsigned char> _cols;
    _cols.resize(_pts->size());

    C7Vector tr(pointCloud->getCumulativeTransformation());
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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            insertObject(it);
    }
}

void CPointCloud::insertObject(const C3DObject* obj)
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
    FUNCTION_DEBUG;
    _points.clear();
    _colors.clear();
    _displayPoints.clear();
    _displayColors.clear();
    if (_pointCloudInfo!=nullptr)
    {
        CPluginContainer::mesh_destroyPointCloud(_pointCloudInfo);
        _pointCloudInfo=nullptr;
    }
    _minDim.set(-0.1f,-0.1f,-0.1f);
    _maxDim.set(+0.1f,+0.1f,+0.1f);
    _nonEmptyCells=0;
}

const std::vector<float>* CPointCloud::getPoints() const
{
    FUNCTION_DEBUG;
    return(&_points);
}

std::vector<float>* CPointCloud::getPoints()
{
    FUNCTION_DEBUG;
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
bool CPointCloud::isPotentiallyCuttable() const
{
    return(false);
}

bool CPointCloud::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    minV=_minDim;
    maxV=_maxDim;
    return(true);
}

bool CPointCloud::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

bool CPointCloud::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CPointCloud::scaleObject(float scalingFactor)
{
    _cellSize*=scalingFactor;
    _buildResolution*=scalingFactor;
    _removalDistanceTolerance*=scalingFactor;
    _insertionDistanceTolerance*=scalingFactor;
    scaleObjectMain(scalingFactor);
    _minDim*=scalingFactor;
    _maxDim*=scalingFactor;
    for (size_t i=0;i<_points.size();i++)
        _points[i]*=scalingFactor;
    for (size_t i=0;i<_displayPoints.size();i++)
        _displayPoints[i]*=scalingFactor;
    if (_pointCloudInfo!=nullptr)
        CPluginContainer::mesh_scalePointCloud(_pointCloudInfo,scalingFactor);
}

void CPointCloud::scaleObjectNonIsometrically(float x,float y,float z)
{
    float s=cbrt(x*y*z);
    scaleObject(s);
}

void CPointCloud::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CPointCloud::copyYourself()
{   
    CPointCloud* newPointcloud=(CPointCloud*)copyYourselfMain();

    newPointcloud->_cellSize=_cellSize;
    newPointcloud->_maxPointCountPerCell=_maxPointCountPerCell;
    color.copyYourselfInto(&newPointcloud->color);

    if (_pointCloudInfo!=nullptr)
        newPointcloud->_pointCloudInfo=CPluginContainer::mesh_copyPointCloud(_pointCloudInfo);
    newPointcloud->_points.assign(_points.begin(),_points.end());
    newPointcloud->_colors.assign(_colors.begin(),_colors.end());
    newPointcloud->_minDim=_minDim;
    newPointcloud->_maxDim=_maxDim;
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
    newPointcloud->_displayPoints.assign(_displayPoints.begin(),_displayPoints.end());
    newPointcloud->_displayColors.assign(_displayColors.begin(),_displayColors.end());

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
    _pointSize=tt::getLimitedInt(1,8,s);
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
        _colors.clear();
        if (r)
        {
            for (size_t i=0;i<_points.size()/3;i++)
            {
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.0);
            }
        }
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
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CPointCloud::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CPointCloud::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}

void CPointCloud::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CPointCloud::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CPointCloud::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CPointCloud::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CPointCloud::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CPointCloud::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CPointCloud::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CPointCloud::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CPointCloud::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CPointCloud::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CPointCloud::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
}

void CPointCloud::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CPointCloud::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
//  {
//  }
    simulationEndedMain();
}

void CPointCloud::serialize(CSer& ar)
{
    FUNCTION_INSIDE_DEBUG("CPointCloud::serialize");
    serializeMain(ar);
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
                    ar << _minDim(0) << _minDim(1) << _minDim(2);
                    ar << _maxDim(0) << _maxDim(1) << _maxDim(2);
                    ar.flush();

                    std::vector<unsigned char> data;
                    CPluginContainer::mesh_getPointCloudSerializationData(_pointCloudInfo,data);
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
                        ar >> _minDim(0) >> _minDim(1) >> _minDim(2);
                        ar >> _maxDim(0) >> _maxDim(1) >> _maxDim(2);
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
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
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
                        _pointCloudInfo=CPluginContainer::mesh_getPointCloudFromSerializationData(data);
                        _readPositionsAndColorsAndSetDimensions();
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CPointCloud::serializeWExtIk(CExtIkSer& ar)
{ // make sure to do similar in the serializeWExtIkStatic routine
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CPointCloud::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
}

bool CPointCloud::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void CPointCloud::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}

void CPointCloud::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CPointCloud::display");
    EASYLOCK(_objectMutex);
    displayPointCloud(this,renderingObject,displayAttrib);
}
