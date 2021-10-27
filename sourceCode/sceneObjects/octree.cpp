#include "simInternal.h"
#include "simStrings.h"
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
    TRACE_INTERNAL;
    _objectType=sim_object_octree_type;

    _cellSize=0.025f;
    color.setDefaultValues();
    color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
    _visibilityLayer=OCTREE_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable|sim_objectspecialproperty_renderable;
    _objectAlias=IDSOGL_OCTREE;
    _objectName_old=IDSOGL_OCTREE;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
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
    TRACE_INTERNAL;
    clear();
}

void COctree::getTransfAndHalfSizeOfBoundingBox(C7Vector& tr,C3Vector& hs) const
{
    hs=(_maxDim-_minDim)*0.5f;
    C4X4Matrix m=getFullCumulativeTransformation().getMatrix();
    C3Vector center((_minDim+_maxDim)*0.5);
    m.X+=m.M*center;
    tr=m.getTransformation();
}

CColorObject* COctree::getColor()
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
        CPluginContainer::geomPlugin_getOctreeVoxelPositions(_octreeInfo,_voxelPositions);
        CPluginContainer::geomPlugin_getOctreeVoxelColors(_octreeInfo,_colors);
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
    TRACE_INTERNAL;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToOctree)
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
    if (_octreeInfo==nullptr)
    {
        if (optionalColors3==nullptr)
        {
            unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
            _octreeInfo=CPluginContainer::geomPlugin_createOctreeFromPoints(_pts,ptsCnt,nullptr,_cellSize,cols,theTagWhenOptionalTagsIsNull);
        }
        else
        {
            if (colorsAreIndividual)
                _octreeInfo=CPluginContainer::geomPlugin_createOctreeFromColorPoints(_pts,ptsCnt,nullptr,_cellSize,optionalColors3,optionalTags);
            else
                _octreeInfo=CPluginContainer::geomPlugin_createOctreeFromPoints(_pts,ptsCnt,nullptr,_cellSize,optionalColors3,optionalTags[0]);
        }
    }
    else
    {
        if (optionalColors3==nullptr)
        {
            unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
            CPluginContainer::geomPlugin_insertPointsIntoOctree(_octreeInfo,C7Vector::identityTransformation,_pts,ptsCnt,cols,theTagWhenOptionalTagsIsNull);
        }
        else
        {
            if (colorsAreIndividual)
                CPluginContainer::geomPlugin_insertColorPointsIntoOctree(_octreeInfo,C7Vector::identityTransformation,_pts,ptsCnt,optionalColors3,optionalTags);
            else
                CPluginContainer::geomPlugin_insertPointsIntoOctree(_octreeInfo,C7Vector::identityTransformation,_pts,ptsCnt,optionalColors3,optionalTags[0]);
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertShape(CShape* shape,unsigned int theTag)
{
    TRACE_INTERNAL;
    shape->initializeMeshCalculationStructureIfNeeded();

    C7Vector octreeTr(getFullCumulativeTransformation());
    C7Vector shapeTr(((CShape*)shape)->getFullCumulativeTransformation());
    unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
    if (_octreeInfo==nullptr)
        _octreeInfo=CPluginContainer::geomPlugin_createOctreeFromMesh(shape->_meshCalculationStructure,shapeTr,&octreeTr,_cellSize,cols,theTag);
    else
        CPluginContainer::geomPlugin_insertMeshIntoOctree(_octreeInfo,octreeTr,shape->_meshCalculationStructure,shapeTr,cols,theTag);
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertOctree(const COctree* octree,unsigned int theTag)
{
    TRACE_INTERNAL;
    if (octree->_octreeInfo!=nullptr)
        insertOctree(octree->_octreeInfo,((COctree*)octree)->getFullCumulativeTransformation().getMatrix(),theTag);
}

void COctree::insertDummy(const CDummy* dummy,unsigned int theTag)
{
    TRACE_INTERNAL;
    insertPoints(dummy->getFullCumulativeTransformation().X.data,1,false,nullptr,false,nullptr,theTag);
}

void COctree::insertPointCloud(const CPointCloud* pointCloud,unsigned int theTag)
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
        insertPoints(&pts[0],(int)pts.size()/3,false,nullptr,false,nullptr,theTag);
    }
}


void COctree::insertOctree(const void* octree2Info,const C7Vector& octree2Tr,unsigned int theTag)
{
    TRACE_INTERNAL;

    unsigned char cols[3]={(unsigned char)(color.getColorsPtr()[0]*255.1f),(unsigned char)(color.getColorsPtr()[1]*255.1f),(unsigned char)(color.getColorsPtr()[2]*255.1f)};
    if (_octreeInfo==nullptr)
    {
        const C7Vector tr(getFullCumulativeTransformation());
        _octreeInfo=CPluginContainer::geomPlugin_createOctreeFromOctree(octree2Info,octree2Tr,&tr,_cellSize,cols,theTag);
    }
    else
        CPluginContainer::geomPlugin_insertOctreeIntoOctree(_octreeInfo,getFullCumulativeTransformation(),octree2Info,octree2Tr,cols,theTag);
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::insertObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            insertObject(it,0);
    }
}

void COctree::insertObject(const CSceneObject* obj,unsigned int theTag)
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
    TRACE_INTERNAL;
    const float* _pts=pts;
    std::vector<float> __pts;
    if (!ptsAreRelativeToOctree)
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
    if (_octreeInfo!=nullptr)
    {
        if (CPluginContainer::geomPlugin_removePointsFromOctree(_octreeInfo,C7Vector::identityTransformation,_pts,ptsCnt))
        {
            CPluginContainer::geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
    }
    _readPositionsAndColorsAndSetDimensions();
}

void COctree::subtractShape(CShape* shape)
{
    TRACE_INTERNAL;
    if (_octreeInfo!=nullptr)
    {
        shape->initializeMeshCalculationStructureIfNeeded();

        C4X4Matrix octreeM(getFullCumulativeTransformation().getMatrix());
        C4X4Matrix shapeM(((CShape*)shape)->getFullCumulativeTransformation().getMatrix());
        if (CPluginContainer::geomPlugin_removeMeshFromOctree(_octreeInfo,octreeM,shape->_meshCalculationStructure,shapeM))
        {
            CPluginContainer::geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void COctree::subtractOctree(const COctree* octree)
{
    TRACE_INTERNAL;
    if (octree->_octreeInfo!=nullptr)
        subtractOctree(octree->_octreeInfo,((COctree*)octree)->getFullCumulativeTransformation());
}

void COctree::subtractDummy(const CDummy* dummy)
{
    TRACE_INTERNAL;
    subtractPoints(dummy->getFullCumulativeTransformation().X.data,1,false);
}

void COctree::subtractPointCloud(const CPointCloud* pointCloud)
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
        subtractPoints(&pts[0],(int)pts.size()/3,false);
    }
}


void COctree::subtractOctree(const void* octree2Info,const C7Vector& octree2Tr)
{
    TRACE_INTERNAL;
    if (_octreeInfo!=nullptr)
    {
        if (CPluginContainer::geomPlugin_removeOctreeFromOctree(_octreeInfo,getFullCumulativeTransformation(),octree2Info,octree2Tr))
        {
            CPluginContainer::geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo=nullptr;
        }
        _readPositionsAndColorsAndSetDimensions();
    }
}

void COctree::subtractObjects(const std::vector<int>& sel)
{
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it!=nullptr)&&(it!=this))
            subtractObject(it);
    }
}

void COctree::subtractObject(const CSceneObject* obj)
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
    TRACE_INTERNAL;
    if (_octreeInfo!=nullptr)
    {
        CPluginContainer::geomPlugin_destroyOctree(_octreeInfo);
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
                _colors.push_back(color.getColorsPtr()[0]);
                _colors.push_back(color.getColorsPtr()[1]);
                _colors.push_back(color.getColorsPtr()[2]);
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
    TRACE_INTERNAL;
    return(&_voxelPositions);
}

std::vector<float>* COctree::getCubePositions()
{
    TRACE_INTERNAL;
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
    CSceneObject::scaleObject(scalingFactor);
    _minDim*=scalingFactor;
    _maxDim*=scalingFactor;
    for (size_t i=0;i<_voxelPositions.size();i++)
        _voxelPositions[i]*=scalingFactor;
    if (_octreeInfo!=nullptr)
        CPluginContainer::geomPlugin_scaleOctree(_octreeInfo,scalingFactor);
}

void COctree::scaleObjectNonIsometrically(float x,float y,float z)
{
    float s=cbrt(x*y*z);
    scaleObject(s);
}

void COctree::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void COctree::pushCreationEvent() const
{
    auto [event,data]=App::worldContainer->createEvent(EVENTTYPE_OBJECTADDED,nullptr,this,true);
    CSceneObject::_pushObjectCreationEventData(data);

    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("octree",subC);
    data=subC;

    // todo

    App::worldContainer->pushEvent(event);
}

CSceneObject* COctree::copyYourself()
{
    COctree* newOctree=(COctree*)CSceneObject::copyYourself();

    newOctree->_cellSize=_cellSize;
    color.copyYourselfInto(&newOctree->color);

    if (_octreeInfo!=nullptr)
        newOctree->_octreeInfo=CPluginContainer::geomPlugin_copyOctree(_octreeInfo);
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
            insertOctree(octree2Info,getFullCumulativeTransformation().getMatrix(),0);
            CPluginContainer::geomPlugin_destroyOctree(octree2Info);
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
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void COctree::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void COctree::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}

void COctree::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}
void COctree::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void COctree::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void COctree::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}

void COctree::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void COctree::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void COctree::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void COctree::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void COctree::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
        }
    }
    CSceneObject::simulationEnded();
}

void COctree::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
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
                    CPluginContainer::geomPlugin_getOctreeSerializationData(_octreeInfo,data);
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
                        _octreeInfo=CPluginContainer::geomPlugin_getOctreeFromSerializationData(&data[0]);
                        _readPositionsAndColorsAndSetDimensions();
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("cellSize",_cellSize);
            ar.xmlAddNode_int("pointSize",_pointSize);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showStructure",_showOctreeStructure);
            ar.xmlAddNode_bool("randomColors",_useRandomColors);
            ar.xmlAddNode_bool("pointsInsteadOfCubes",_usePointsInsteadOfCubes);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("saveCalculationStructure",_saveCalculationStructure);
            ar.xmlAddNode_bool("emissiveColor",_colorIsEmissive);
            ar.xmlPopNode();

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
                ar.xmlAddNode_int("voxelCount",int(_voxelPositions.size()/3));

            if (_octreeInfo!=nullptr)
            {
                if (ar.xmlSaveDataInline(_voxelPositions.size()*4+_colors.size()*3/4)||(!exhaustiveXml))
                {
                    ar.xmlAddNode_floats("voxelPositions",_voxelPositions);
                    std::vector<int> tmp;
                    for (size_t i=0;i<_voxelPositions.size()/3;i++)
                    {
                        tmp.push_back((unsigned int)(_colors[4*i+0]*255.1f));
                        tmp.push_back((unsigned int)(_colors[4*i+1]*255.1f));
                        tmp.push_back((unsigned int)(_colors[4*i+2]*255.1f));
                    }
                    ar.xmlAddNode_ints("voxelColors",tmp);
                }
                else
                {
//                    CSer* w=ar.xmlAddNode_binFile("file",(std::string("octree_")+_objectName).c_str());
                    CSer* w=ar.xmlAddNode_binFile("file",(_objectAlias+"-octree-"+std::to_string(_objectHandle)).c_str());
                    w[0] << int(_voxelPositions.size());
                    for (size_t i=0;i<_voxelPositions.size();i++)
                        w[0] << _voxelPositions[i];

                    for (size_t i=0;i<_voxelPositions.size()/3;i++)
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
        }
        else
        {
            ar.xmlGetNode_float("cellSize",_cellSize,exhaustiveXml);
            ar.xmlGetNode_int("pointSize",_pointSize,exhaustiveXml);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("showStructure",_showOctreeStructure,exhaustiveXml);
                ar.xmlGetNode_bool("randomColors",_useRandomColors,exhaustiveXml);
                ar.xmlGetNode_bool("pointsInsteadOfCubes",_usePointsInsteadOfCubes,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("saveCalculationStructure",_saveCalculationStructure);
                ar.xmlGetNode_bool("emissiveColor",_colorIsEmissive,exhaustiveXml);
                ar.xmlPopNode();
            }

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
                int voxelCnt=0;
                ar.xmlGetNode_int("voxelCount",voxelCnt);
    
                if (voxelCnt>0)
                {
                    std::vector<float> pts;
                    std::vector<unsigned char> cols;
                    std::vector<unsigned int> tags;
                    if (ar.xmlGetNode_floats("voxelPositions",pts,false))
                        ar.xmlGetNode_uchars("voxelColors",cols);
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
                    tags.resize(voxelCnt,0);
                    insertPoints(&pts[0],voxelCnt,true,&cols[0],true,&tags[0],0);
                }
                else
                    _readPositionsAndColorsAndSetDimensions();
            }
            else
            {
                std::vector<float> pts;
                std::vector<unsigned char> cols;
                std::vector<unsigned int> tags;
                if (ar.xmlGetNode_floats("voxelPositions",pts,exhaustiveXml))
                    ar.xmlGetNode_uchars("voxelColors",cols,exhaustiveXml);
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
                    tags.resize(pts.size()/3,0);
                    insertPoints(&pts[0],pts.size()/3,true,&cols[0],true,&tags[0],0);
                }
                else
                    _readPositionsAndColorsAndSetDimensions();
            }
        }
    }
}

void COctree::serializeWExtIk(CExtIkSer& ar)
{ // make sure to do similar in the serializeWExtIkStatic routine
    CSceneObject::serializeWExtIk(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void COctree::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
}

bool COctree::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    return(retVal);
}

void COctree::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void COctree::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayOctree(this,renderingObject,displayAttrib);
}

