
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
#include "app.h"
#include "octreeRendering.h"

COctree::COctree()
{
    FUNCTION_DEBUG;
    _objectType=sim_object_octree_type;

    _cellSize=0.025f;
    color.setDefaultValues();
    color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
    layer=OCTREE_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable;
    _objectName=IDSOGL_OCTREE;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
    _octreeInfo=nullptr;
    _showOctreeStructure=false;
    _useRandomColors=false;
    _colorIsEmissive=false;
    _usePointsInsteadOfCubes=false;
    _saveCalculationStructure=true; // takes actually less disk space when true!

    _pointSize=2;
    _cellSizeForDisplay=0;
    _vertexBufferId=-1;
    _normalBufferId=-1;

    clear(); // also sets the _minDim and _maxDim values
}

COctree::~COctree()
{
    FUNCTION_DEBUG;
    clear();
}

void COctree::getMatrixAndHalfSizeOfBoundingBox(C4X4Matrix& m,C3Vector& hs) const
{
    hs=(_maxDim-_minDim)*0.5f;
    m=getCumulativeTransformation().getMatrix();
    C3Vector center((_minDim+_maxDim)*0.5);
    m.X+=m.M*center;
}

CVisualParam* COctree::getColor()
{
    return(&color);
}

void COctree::setVertexBufferId(int id)
{
    _vertexBufferId=id;
}

int COctree::getVertexBufferId() const
{
    return(_vertexBufferId);
}

void COctree::setNormalBufferId(int id)
{
    _normalBufferId=id;
}

int COctree::getNormalBufferId() const
{
    return(_normalBufferId);
}

void COctree::getMaxMinDims(C3Vector& ma,C3Vector& mi) const
{
    ma=_maxDim;
    mi=_minDim;
}

float* COctree::getCubeVertices()
{
    return(_cubeVertices);
}

float* COctree::getColors()
{
    return(&_colors[0]);
}

void COctree::_readPositionsAndColorsAndSetDimensions()
{
    _voxelPositions.clear();
    _colors.clear();
    if (_octreeInfo!=nullptr)
    {
        CPluginContainer::mesh_getOctreeVoxels(_octreeInfo,_voxelPositions,_colors);
        if (_useRandomColors)
        {
            _colors.clear();
            for (size_t i=0;i<_voxelPositions.size()/3;i++)
            {
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.0);
            }
        }

        for (size_t i=0;i<_voxelPositions.size()/3;i++)
        {
            C3Vector p(&_voxelPositions[3*i]);
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
        _minDim(0)-=_cellSize*0.5;
        _minDim(1)-=_cellSize*0.5;
        _minDim(2)-=_cellSize*0.5;
        _maxDim(0)+=_cellSize*0.5;
        _maxDim(1)+=_cellSize*0.5;
        _maxDim(2)+=_cellSize*0.5;
    }
    else
        clear();
}

void COctree::insertPoints(const float* pts,int ptsCnt,bool ptsAreRelativeToOctree,const unsigned char* optionalColors3,bool colorsAreIndividual,const unsigned int* optionalTags,unsigned int theTagWhenOptionalTagsIsNull)
{
    FUNCTION_DEBUG;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToOctree)
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
    if (_octreeInfo==nullptr)
    {
        if (optionalColors3==nullptr)
            _octreeInfo=CPluginContainer::mesh_createOctreeFromPoints(_pts,ptsCnt,_cellSize,color.colors,theTagWhenOptionalTagsIsNull);
        else
        {
            if (colorsAreIndividual)
                _octreeInfo=CPluginContainer::mesh_createOctreeFromColorPoints(_pts,ptsCnt,_cellSize,optionalColors3,optionalTags);
            else
            {
                const float c[3]={float(optionalColors3[0])/255.1f,float(optionalColors3[1])/255.1f,float(optionalColors3[2])/255.1f};
                _octreeInfo=CPluginContainer::mesh_createOctreeFromPoints(_pts,ptsCnt,_cellSize,c,optionalTags[0]);
            }
        }
    }
    else
    {
        if (optionalColors3==nullptr)
            CPluginContainer::mesh_insertPointsIntoOctree(_octreeInfo,_pts,ptsCnt,color.colors,theTagWhenOptionalTagsIsNull);
        else
        {
            if (colorsAreIndividual)
                CPluginContainer::mesh_insertColorPointsIntoOctree(_octreeInfo,_pts,ptsCnt,optionalColors3,optionalTags);
            else
            {
                const float c[3]={float(optionalColors3[0])/255.1f,float(optionalColors3[1])/255.1f,float(optionalColors3[2])/255.1f};
                CPluginContainer::mesh_insertPointsIntoOctree(_octreeInfo,_pts,ptsCnt,c,optionalTags[0]);
            }
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertShape(const CShape* shape,unsigned int theTag)
{
    FUNCTION_DEBUG;
    shape->geomData->initializeCalculationStructureIfNeeded();

    C4X4Matrix octreeM(getCumulativeTransformation().getMatrix());
    C4X4Matrix shapeM(((CShape*)shape)->getCumulativeTransformation().getMatrix());
    if (_octreeInfo==nullptr)
        _octreeInfo=CPluginContainer::mesh_createOctreeFromShape(octreeM,shape->geomData->collInfo,shapeM,_cellSize,color.colors,theTag);
    else
        CPluginContainer::mesh_insertShapeIntoOctree(_octreeInfo,octreeM,shape->geomData->collInfo,shapeM,color.colors,theTag);
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertOctree(const COctree* octree,unsigned int theTag)
{
    FUNCTION_DEBUG;
    if (octree->_octreeInfo!=nullptr)
        insertOctree(octree->_octreeInfo,((COctree*)octree)->getCumulativeTransformation().getMatrix(),theTag);
}

void COctree::insertDummy(const CDummy* dummy,unsigned int theTag)
{
    FUNCTION_DEBUG;
    insertPoints(dummy->getCumulativeTransformation().X.data,1,false,nullptr,false,nullptr,theTag);
}

void COctree::insertPointCloud(const CPointCloud* pointCloud,unsigned int theTag)
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
        insertPoints(&pts[0],(int)pts.size()/3,false,nullptr,false,nullptr,theTag);
    }
}


void COctree::insertOctree(const void* octree2Info,const C4X4Matrix& octree2CTM,unsigned int theTag)
{
    FUNCTION_DEBUG;

    C4X4Matrix octreeM(getCumulativeTransformation().getMatrix());
    if (_octreeInfo==nullptr)
        _octreeInfo=CPluginContainer::mesh_createOctreeFromOctree(octreeM,octree2Info,octree2CTM,_cellSize,color.colors,theTag);
    else
        CPluginContainer::mesh_insertOctreeIntoOctree(_octreeInfo,octreeM,octree2Info,octree2CTM,color.colors,theTag);
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            insertObject(it,0);
    }
}

void COctree::insertObject(const C3DObject* obj,unsigned int theTag)
{
    if (obj->getObjectType()==sim_object_shape_type)
        insertShape((CShape*)obj,theTag);
    if (obj->getObjectType()==sim_object_octree_type)
        insertOctree((COctree*)obj,theTag);
    if (obj->getObjectType()==sim_object_dummy_type)
        insertDummy((CDummy*)obj,theTag);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        insertPointCloud((CPointCloud*)obj,theTag);
}

void COctree::subtractPoints(const float* pts,int ptsCnt,bool ptsAreRelativeToOctree)
{
    FUNCTION_DEBUG;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToOctree)
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
    if (_octreeInfo!=nullptr)
    {
        if (CPluginContainer::mesh_removeOctreeVoxelsFromPoints(_octreeInfo,_pts,ptsCnt))
        {
            CPluginContainer::mesh_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::subtractShape(const CShape* shape)
{
    FUNCTION_DEBUG;
    if (_octreeInfo!=nullptr)
    {
        shape->geomData->initializeCalculationStructureIfNeeded();

        C4X4Matrix octreeM(getCumulativeTransformation().getMatrix());
        C4X4Matrix shapeM(((CShape*)shape)->getCumulativeTransformation().getMatrix());
        if (CPluginContainer::mesh_removeOctreeVoxelsFromShape(_octreeInfo,octreeM,shape->geomData->collInfo,shapeM))
        {
            CPluginContainer::mesh_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void COctree::subtractOctree(const COctree* octree)
{
    FUNCTION_DEBUG;
    if (octree->_octreeInfo!=nullptr)
        subtractOctree(octree->_octreeInfo,((COctree*)octree)->getCumulativeTransformation().getMatrix());
}

void COctree::subtractDummy(const CDummy* dummy)
{
    FUNCTION_DEBUG;
    subtractPoints(dummy->getCumulativeTransformation().X.data,1,false);
}

void COctree::subtractPointCloud(const CPointCloud* pointCloud)
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
        subtractPoints(&pts[0],(int)pts.size()/3,false);
    }
}


void COctree::subtractOctree(const void* octree2Info,const C4X4Matrix& octree2CTM)
{
    FUNCTION_DEBUG;
    if (_octreeInfo!=nullptr)
    {
        C4X4Matrix octreeM(getCumulativeTransformation().getMatrix());
        if (CPluginContainer::mesh_removeOctreeVoxelsFromOctree(_octreeInfo,octreeM,octree2Info,octree2CTM))
        {
            CPluginContainer::mesh_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void COctree::subtractObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            subtractObject(it);
    }
}

void COctree::subtractObject(const C3DObject* obj)
{
    if (obj->getObjectType()==sim_object_shape_type)
        subtractShape((CShape*)obj);
    if (obj->getObjectType()==sim_object_octree_type)
        subtractOctree((COctree*)obj);
    if (obj->getObjectType()==sim_object_dummy_type)
        subtractDummy((CDummy*)obj);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        subtractPointCloud((CPointCloud*)obj);
}

void COctree::clear()
{
    FUNCTION_DEBUG;
    if (_octreeInfo!=nullptr)
    {
        CPluginContainer::mesh_destroyOctree(_octreeInfo);
        _octreeInfo=nullptr;
    }
    _voxelPositions.clear();
    _colors.clear();
    _minDim.set(-0.1f,-0.1f,-0.1f);
    _maxDim.set(+0.1f,+0.1f,+0.1f);
}

bool COctree::getUseRandomColors() const
{
    return(_useRandomColors);
}

void COctree::setUseRandomColors(bool r)
{
    if (r!=_useRandomColors)
    {
        _useRandomColors=r;
        _colors.clear();
        if (r)
        {
            for (size_t i=0;i<_voxelPositions.size()/3;i++)
            {
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.2f+SIM_RAND_FLOAT*0.8f);
                _colors.push_back(0.0);
            }
        }
        else
        {
            for (size_t i=0;i<_voxelPositions.size()/3;i++)
            {
                _colors.push_back(color.colors[0]);
                _colors.push_back(color.colors[1]);
                _colors.push_back(color.colors[2]);
                _colors.push_back(0.0);
            }
        }
    }
}

bool COctree::getColorIsEmissive() const
{
    return(_colorIsEmissive);
}

void COctree::setColorIsEmissive(bool e)
{
    _colorIsEmissive=e;
}

bool COctree::getUsePointsInsteadOfCubes() const
{
    return(_usePointsInsteadOfCubes);
}

void COctree::setUsePointsInsteadOfCubes(bool r)
{
    _usePointsInsteadOfCubes=r;
}

bool COctree::getSaveCalculationStructure() const
{
    return(_saveCalculationStructure);
}

void COctree::setSaveCalculationStructure(bool s)
{
    _saveCalculationStructure=s;
}

int COctree::getPointSize() const
{
    return(_pointSize);
}

void COctree::setPointSize(int s)
{
    _pointSize=s;
}

const std::vector<float>* COctree::getCubePositions() const
{
    FUNCTION_DEBUG;
    return(&_voxelPositions);
}

std::vector<float>* COctree::getCubePositions()
{
    FUNCTION_DEBUG;
    return(&_voxelPositions);
}

std::string COctree::getObjectTypeInfo() const
{
    return(IDSOGL_OCTREE);
}

std::string COctree::getObjectTypeInfoExtended() const
{
    return(IDSOGL_OCTREE);
}
bool COctree::isPotentiallyCollidable() const
{
    return(true);
}
bool COctree::isPotentiallyMeasurable() const
{
    return(true);
}
bool COctree::isPotentiallyDetectable() const
{
    return(true);
}
bool COctree::isPotentiallyRenderable() const
{
    return(true);
}
bool COctree::isPotentiallyCuttable() const
{
    return(false);
}

bool COctree::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    minV=_minDim;
    maxV=_maxDim;
    return(true);
}

bool COctree::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

bool COctree::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void COctree::scaleObject(float scalingFactor)
{
    _cellSize*=scalingFactor;
    scaleObjectMain(scalingFactor);
    _minDim*=scalingFactor;
    _maxDim*=scalingFactor;
    for (size_t i=0;i<_voxelPositions.size();i++)
        _voxelPositions[i]*=scalingFactor;
    if (_octreeInfo!=nullptr)
        CPluginContainer::mesh_scaleOctree(_octreeInfo,scalingFactor);
}

void COctree::scaleObjectNonIsometrically(float x,float y,float z)
{
    float s=cbrt(x*y*z);
    scaleObject(s);
}

void COctree::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* COctree::copyYourself()
{
    COctree* newOctree=(COctree*)copyYourselfMain();

    newOctree->_cellSize=_cellSize;
    color.copyYourselfInto(&newOctree->color);

    if (_octreeInfo!=nullptr)
        newOctree->_octreeInfo=CPluginContainer::mesh_copyOctree(_octreeInfo);
    newOctree->_voxelPositions.assign(_voxelPositions.begin(),_voxelPositions.end());
    newOctree->_colors.assign(_colors.begin(),_colors.end());
    newOctree->_minDim=_minDim;
    newOctree->_maxDim=_maxDim;
    newOctree->_showOctreeStructure=_showOctreeStructure;
    newOctree->_useRandomColors=_useRandomColors;
    newOctree->_colorIsEmissive=_colorIsEmissive;
    newOctree->_usePointsInsteadOfCubes=_usePointsInsteadOfCubes;
    newOctree->_saveCalculationStructure=_saveCalculationStructure;

    return(newOctree);
}

void COctree::setCellSize(float theNewSize)
{
    if (_octreeInfo!=nullptr)
        theNewSize=tt::getLimitedFloat(_cellSize,1.0,theNewSize); // we can't reduce the cell size for an existing octree, because it doesn't make sense!
    else
        theNewSize=tt::getLimitedFloat(0.001f,1.0,theNewSize);
    if (theNewSize!=_cellSize)
    {
        _cellSize=theNewSize;
        if (_octreeInfo!=nullptr)
        { // we reconstruct the octree from this octree:
            void* octree2Info=_octreeInfo;
            _octreeInfo=nullptr;
            clear();
            insertOctree(octree2Info,getCumulativeTransformation().getMatrix(),0);
            CPluginContainer::mesh_destroyOctree(octree2Info);
        }
    }
}

float COctree::getCellSize() const
{
    return(_cellSize);
}

void COctree::setCellSizeForDisplay(float theNewSizeForDisplay)
{
    _cellSizeForDisplay=theNewSizeForDisplay;
}

float COctree::getCellSizeForDisplay() const
{
    return(_cellSizeForDisplay);
}

bool COctree::getShowOctree() const
{
    return(_showOctreeStructure);
}

void COctree::setShowOctree(bool show)
{
    _showOctreeStructure=show;
}

const void* COctree::getOctreeInfo() const
{
    return(_octreeInfo);
}

void* COctree::getOctreeInfo()
{
    return(_octreeInfo);
}

void COctree::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void COctree::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void COctree::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}

void COctree::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void COctree::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void COctree::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void COctree::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void COctree::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void COctree::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void COctree::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void COctree::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void COctree::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void COctree::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void COctree::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
}

void COctree::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void COctree::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
//  {
//  }
    simulationEndedMain();
}

void COctree::serialize(CSer& ar)
{
    FUNCTION_INSIDE_DEBUG("COctree::serialize");
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Siz");
            ar << _cellSize << _pointSize;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,1,_showOctreeStructure);
            SIM_SET_CLEAR_BIT(dummy,2,_useRandomColors);
            SIM_SET_CLEAR_BIT(dummy,3,_usePointsInsteadOfCubes);
            SIM_SET_CLEAR_BIT(dummy,4,_saveCalculationStructure);
            SIM_SET_CLEAR_BIT(dummy,5,_colorIsEmissive);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            // Keep this at the end
            if (_octreeInfo!=nullptr)
            {
                if (!_saveCalculationStructure)
                {
                    ar.storeDataName("Pt2");
                    ar << int(_voxelPositions.size()/3);
                    for (size_t i=0;i<_voxelPositions.size()/3;i++)
                    {
                        ar << _voxelPositions[3*i+0];
                        ar << _voxelPositions[3*i+1];
                        ar << _voxelPositions[3*i+2];
                        ar << (unsigned char)(_colors[4*i+0]*255.1f);
                        ar << (unsigned char)(_colors[4*i+1]*255.1f);
                        ar << (unsigned char)(_colors[4*i+2]*255.1f);
                    }
                    ar.flush();
                }
                else
                {
                    ar.storeDataName("Mm2");
                    ar << _minDim(0) << _minDim(1) << _minDim(2);
                    ar << _maxDim(0) << _maxDim(1) << _maxDim(2);
                    ar.flush();

                    std::vector<unsigned char> data;
                    CPluginContainer::mesh_getOctreeSerializationData(_octreeInfo,data);
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
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _showOctreeStructure=SIM_IS_BIT_SET(dummy,1);
                        _useRandomColors=SIM_IS_BIT_SET(dummy,2);
                        _usePointsInsteadOfCubes=SIM_IS_BIT_SET(dummy,3);
                        _saveCalculationStructure=SIM_IS_BIT_SET(dummy,4);
                        _colorIsEmissive=SIM_IS_BIT_SET(dummy,5);
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        color.serialize(ar,0);
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
                        std::vector<unsigned int> tags;
                        tags.resize(cnt,0);
                        for (int i=0;i<cnt;i++)
                        {
                            ar >> pts[3*i+0];
                            ar >> pts[3*i+1];
                            ar >> pts[3*i+2];
                            ar >> cols[3*i+0];
                            ar >> cols[3*i+1];
                            ar >> cols[3*i+2];
                        }
                        // Now we need to rebuild the octree:
                        if (cnt>0)
                            insertPoints(&pts[0],cnt,true,&cols[0],true,&tags[0],0);
                        else
                            _readPositionsAndColorsAndSetDimensions();
                    }
                    if (theName.compare("Mm2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _minDim(0) >> _minDim(1) >> _minDim(2);
                        ar >> _maxDim(0) >> _maxDim(1) >> _maxDim(2);
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
                        _octreeInfo=CPluginContainer::mesh_getOctreeFromSerializationData(data);
                        _readPositionsAndColorsAndSetDimensions();
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void COctree::serializeWExtIk(CExtIkSer& ar)
{ // make sure to do similar in the serializeWExtIkStatic routine
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void COctree::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
}

bool COctree::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void COctree::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}

void COctree::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("COctree::display");
    EASYLOCK(_objectMutex);
    displayOctree(this,renderingObject,displayAttrib);
}

