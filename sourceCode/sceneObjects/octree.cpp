#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <tt.h>
#include <pointCloud.h>
#include <sceneObjectOperations.h>
#include <global.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <octreeRendering.h>
#endif

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "sceneObject",
    "namespaces": {
        "refs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "origRefs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "customData": {},
        "signal": {}
    },
    "methods": {
        )" OCTREE_META_METHODS R"(,
        )" SCENEOBJECT_META_METHODS R"(
    }
}
)";

COcTree::COcTree()
{
    TRACE_INTERNAL;
    _objectType = sim_sceneobject_octree;

    _refreshDisplay = true;
    _cellSize = 0.025;
    color.setDefaultValues();
    color.setColor(1.0, 1.0, 1.0, sim_colorcomponent_ambient_diffuse);
    _visibilityLayer = OCTREE_LAYER;
    _localObjectSpecialProperty = sim_objectspecialproperty_collidable | sim_objectspecialproperty_measurable |
                                  sim_objectspecialproperty_detectable | sim_objectspecialproperty_renderable;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    _octreeInfo = nullptr;
    _showOctreeStructure = false;
    _useRandomColors = false;
    _colorIsEmissive = false;
    _usePointsInsteadOfCubes = false;

    _pointSize = 2;
    _cellSizeForDisplay = 0;
    _vertexBufferId = -1;
    _normalBufferId = -1;

    clear(); // also resets the BB
}

COcTree::~COcTree()
{
    TRACE_INTERNAL;
    clear();
}

CColorObject* COcTree::getColor()
{
    return (&color);
}

void COcTree::setVertexBufferId(int id)
{
    _vertexBufferId = id;
}

int COcTree::getVertexBufferId() const
{
    return (_vertexBufferId);
}

void COcTree::setNormalBufferId(int id)
{
    _normalBufferId = id;
}

int COcTree::getNormalBufferId() const
{
    return (_normalBufferId);
}

float* COcTree::getCubeVertices()
{
    return (_cubeVertices);
}

float* COcTree::getColors()
{
    return (&_colors[0]);
}

void COcTree::_readPositionsAndColorsAndSetDimensions(bool incrementalDisplayUpdate)
{
    std::vector<double> _voxelPositions_old;
    _voxelPositions_old.swap(_voxelPositions);
    std::vector<unsigned char> _colorsByte_old;
    _colorsByte_old.swap(_colorsByte);
    _colors.clear();
    bool generateEvent = true;
    if (_octreeInfo != nullptr)
    {
        App::worldContainer->pluginContainer->geomPlugin_getOctreeVoxelPositions(_octreeInfo, _voxelPositions);
        App::worldContainer->pluginContainer->geomPlugin_getOctreeVoxelColors(_octreeInfo, _colors);
        if (_useRandomColors)
        {
            _colors.clear();
            for (size_t i = 0; i < _voxelPositions.size() / 3; i++)
            {
                _colors.push_back(0.2 + SIM_RAND_FLOAT * 0.8);
                _colors.push_back(0.2 + SIM_RAND_FLOAT * 0.8);
                _colors.push_back(0.2 + SIM_RAND_FLOAT * 0.8);
                _colors.push_back(1.0);
            }
        }
        for (size_t i = 0; i < _colors.size(); i++)
            _colorsByte.push_back((unsigned char)(_colors[i] * 255.1));

        computeBoundingBox();
    }
    else
    {
        clear();
        generateEvent = false;
    }
    if (generateEvent && _isInScene && App::worldContainer->getEventsEnabled())
    {
        if (_voxelPositions_old.size() == _voxelPositions.size())
        {
            unsigned char* v = (unsigned char*)_voxelPositions.data();
            unsigned char* w = (unsigned char*)_voxelPositions_old.data();
            unsigned long long vv = 0;
            unsigned long long ww = 0;
            for (size_t i = 0; i < _voxelPositions_old.size() * 4; i++)
            {
                vv += v[i];
                ww += w[i];
            }
            if (vv == ww)
            {

                v = _colorsByte.data();
                w = _colorsByte_old.data();
                vv = 0;
                ww = 0;
                for (size_t i = 0; i < _colorsByte.size(); i++)
                {
                    vv += v[i];
                    ww += w[i];
                }
                if (vv == ww)
                    generateEvent = false;
            }
        }
        if (generateEvent)
            _updateOctreeEvent(incrementalDisplayUpdate);
    }
}

void COcTree::_updateOctreeEvent(bool incremental, CCbor* evv /*= nullptr*/)
{
    CCbor* ev = evv;
    if ((evv != nullptr) || (_isInScene && App::worldContainer->getEventsEnabled()))
    {
#if SIM_EVENT_PROTOCOL_VERSION == 2
        const char* cmd = "voxels";
        if (evv == nullptr)
            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
        ev->openKeyMap(cmd);
        ev->appendKeyDoubleArray("positions", _voxelPositions.data(), _voxelPositions.size());
        ev->appendKeyUCharArray("colors", _colorsByte.data(), _colorsByte.size());
        if (evv == nullptr)
            App::worldContainer->pushEvent();
#else
        /*
        const char* cmd = propOctree_voxels.name;
        if (evv == nullptr)
            ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->appendKeyDoubleArray(cmd, _voxelPositions.data(), _voxelPositions.size());
        ev->appendKeyBuff(propOctree_colors.name, _colorsByte.data(), _colorsByte.size());
        ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
        ev->appendKeyBool(propOctree_randomColors.name, _useRandomColors);
        ev->appendKeyBool(propOctree_showPoints.name, _usePointsInsteadOfCubes);
        if (evv == nullptr)
            App::worldContainer->pushEvent();
        */

        if (!incremental)
            _refreshDisplay = true;
        if (_octreeInfo == nullptr)
        {
            if (evv == nullptr)
                ev = App::worldContainer->createSceneObjectChangedEvent(this, false, "set", true);
            ev->openKeyMap("set");
            ev->appendKeyBuff("pts", nullptr, 0);
            ev->appendKeyBuff("rgb", nullptr, 0);
            ev->appendKeyBuff("ids", nullptr, 0);
            ev->closeArrayOrMap();
            ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
            ev->appendKeyBool(propOctree_randomColors.name, _useRandomColors);
            ev->appendKeyBool(propOctree_showPoints.name, _usePointsInsteadOfCubes);
            if (evv == nullptr)
                App::worldContainer->pushEvent();
        }
        else
        {
            if (_refreshDisplay)
            {
                App::worldContainer->pluginContainer->geomPlugin_refreshDisplayOctreeData(_octreeInfo);
                _refreshDisplay = false;
            }
            float* pts;
            unsigned char* cols;
            unsigned int* ids;
            unsigned int* remIds;
            int newCnt, remCnt;
            int r = App::worldContainer->pluginContainer->geomPlugin_getDisplayOctreeData(_octreeInfo, &pts, &cols, &ids, &newCnt, &remIds, &remCnt);
            if (r >= 0)
            {
                if (r == 1)
                {
                    if (evv == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(this, false, "set", true);
                    ev->openKeyMap("set");
                    ev->appendKeyBuff("pts", (unsigned char*)pts, newCnt * 3 * sizeof(float));
                    ev->appendKeyBuff("rgb", cols, newCnt * 3);
                    ev->appendKeyBuff("ids", (unsigned char*)ids, newCnt * sizeof(unsigned int));
                    ev->closeArrayOrMap();
                    ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
                    ev->appendKeyBool(propOctree_randomColors.name, _useRandomColors);
                    ev->appendKeyBool(propOctree_showPoints.name, _usePointsInsteadOfCubes);
                    if (evv == nullptr)
                        App::worldContainer->pushEvent();
                }
                else
                {
                    if (evv == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(this, false, "addRemove", true);
                    ev->openKeyMap("add");
                    ev->appendKeyBuff("pts", (unsigned char*)pts, newCnt * 3 * sizeof(float));
                    ev->appendKeyBuff("rgb", cols, newCnt * 3);
                    ev->appendKeyBuff("ids", (unsigned char*)ids, newCnt * sizeof(unsigned int));
                    ev->closeArrayOrMap();
                    ev->openKeyMap("rem");
                    ev->appendKeyBuff("ids", (unsigned char*)remIds, remCnt * sizeof(unsigned int));
                    ev->closeArrayOrMap();
                    ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
                    ev->appendKeyBool(propOctree_randomColors.name, _useRandomColors);
                    ev->appendKeyBool(propOctree_showPoints.name, _usePointsInsteadOfCubes);
                    if (evv == nullptr)
                        App::worldContainer->pushEvent();
                }
                delete[] pts;
                delete[] cols;
                delete[] ids;
                delete[] remIds;
            }
        }

#endif
    }
}

void COcTree::insertPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToOctree,
                           const unsigned char* optionalColors3, bool colorsAreIndividual,
                           const unsigned int* optionalTags, unsigned int theTagWhenOptionalTagsIsNull)
{
    TRACE_INTERNAL;
    const double* _pts = pts;
    std::vector<double> __pts;
    if (!ptsAreRelativeToOctree)
    {
        C7Vector tr(getFullCumulativeTransformation().getInverse());
        for (int i = 0; i < ptsCnt; i++)
        {
            C3Vector p(pts + 3 * i);
            p = tr * p;
            __pts.push_back(p(0));
            __pts.push_back(p(1));
            __pts.push_back(p(2));
        }
        _pts = &__pts[0];
    }
    if (_octreeInfo == nullptr)
    {
        if (optionalColors3 == nullptr)
        {
            unsigned char cols[3] = {(unsigned char)(color.getColorsPtr()[0] * 255.1),
                                     (unsigned char)(color.getColorsPtr()[1] * 255.1),
                                     (unsigned char)(color.getColorsPtr()[2] * 255.1)};
            _octreeInfo = App::worldContainer->pluginContainer->geomPlugin_createOctreeFromPoints(
                _pts, ptsCnt, nullptr, _cellSize, cols, theTagWhenOptionalTagsIsNull);
        }
        else
        {
            if (colorsAreIndividual)
                _octreeInfo = App::worldContainer->pluginContainer->geomPlugin_createOctreeFromColorPoints(
                    _pts, ptsCnt, nullptr, _cellSize, optionalColors3, optionalTags);
            else
                _octreeInfo = App::worldContainer->pluginContainer->geomPlugin_createOctreeFromPoints(
                    _pts, ptsCnt, nullptr, _cellSize, optionalColors3, optionalTags[0]);
        }
    }
    else
    {
        if (optionalColors3 == nullptr)
        {
            unsigned char cols[3] = {(unsigned char)(color.getColorsPtr()[0] * 255.1),
                                     (unsigned char)(color.getColorsPtr()[1] * 255.1),
                                     (unsigned char)(color.getColorsPtr()[2] * 255.1)};
            App::worldContainer->pluginContainer->geomPlugin_insertPointsIntoOctree(
                _octreeInfo, C7Vector::identityTransformation, _pts, ptsCnt, cols, theTagWhenOptionalTagsIsNull);
        }
        else
        {
            if (colorsAreIndividual)
                App::worldContainer->pluginContainer->geomPlugin_insertColorPointsIntoOctree(
                    _octreeInfo, C7Vector::identityTransformation, _pts, ptsCnt, optionalColors3, optionalTags);
            else
                App::worldContainer->pluginContainer->geomPlugin_insertPointsIntoOctree(
                    _octreeInfo, C7Vector::identityTransformation, _pts, ptsCnt, optionalColors3, optionalTags[0]);
        }
    }
    _readPositionsAndColorsAndSetDimensions(true);
}

void COcTree::insertShape(CShape* shape, unsigned int theTag)
{
    TRACE_INTERNAL;
    shape->initializeMeshCalculationStructureIfNeeded();

    C7Vector octreeTr(getCumulativeTransformation());
    C7Vector shapeTr(((CShape*)shape)->getCumulCenteredMeshFrame());
    unsigned char cols[3] = {(unsigned char)(color.getColorsPtr()[0] * 255.1),
                             (unsigned char)(color.getColorsPtr()[1] * 255.1),
                             (unsigned char)(color.getColorsPtr()[2] * 255.1)};
    if (_octreeInfo == nullptr)
        _octreeInfo = App::worldContainer->pluginContainer->geomPlugin_createOctreeFromMesh(
            shape->_meshCalculationStructure, shapeTr, &octreeTr, _cellSize, cols, theTag);
    else
        App::worldContainer->pluginContainer->geomPlugin_insertMeshIntoOctree(
            _octreeInfo, octreeTr, shape->_meshCalculationStructure, shapeTr, cols, theTag);
    _readPositionsAndColorsAndSetDimensions(true);
}

void COcTree::insertOctree(const COcTree* octree, unsigned int theTag)
{
    TRACE_INTERNAL;
    if (octree->_octreeInfo != nullptr)
        insertOctree(octree->_octreeInfo, ((COcTree*)octree)->getFullCumulativeTransformation().getMatrix(), theTag);
}

void COcTree::insertDummy(const CDummy* dummy, unsigned int theTag)
{
    TRACE_INTERNAL;
    insertPoints(dummy->getFullCumulativeTransformation().X.data, 1, false, nullptr, false, nullptr, theTag);
}

void COcTree::insertPointCloud(const CPointCloud* pointCloud, unsigned int theTag)
{
    TRACE_INTERNAL;
    if (pointCloud->getPointCloudInfo() != nullptr)
    {
        const std::vector<double>* _pts = pointCloud->getPoints();
        C7Vector tr(pointCloud->getFullCumulativeTransformation());
        std::vector<double> pts;
        for (size_t i = 0; i < _pts->size() / 3; i++)
        {
            C3Vector v(&_pts->at(3 * i));
            v *= tr;
            pts.push_back(v(0));
            pts.push_back(v(1));
            pts.push_back(v(2));
        }
        insertPoints(&pts[0], (int)pts.size() / 3, false, nullptr, false, nullptr, theTag);
    }
}

void COcTree::insertOctree(const void* octree2Info, const C7Vector& octree2Tr, unsigned int theTag)
{
    TRACE_INTERNAL;

    unsigned char cols[3] = {(unsigned char)(color.getColorsPtr()[0] * 255.1),
                             (unsigned char)(color.getColorsPtr()[1] * 255.1),
                             (unsigned char)(color.getColorsPtr()[2] * 255.1)};
    if (_octreeInfo == nullptr)
    {
        const C7Vector tr(getFullCumulativeTransformation());
        _octreeInfo = App::worldContainer->pluginContainer->geomPlugin_createOctreeFromOctree(
            octree2Info, octree2Tr, &tr, _cellSize, cols, theTag);
    }
    else
        App::worldContainer->pluginContainer->geomPlugin_insertOctreeIntoOctree(
            _octreeInfo, getFullCumulativeTransformation(), octree2Info, octree2Tr, cols, theTag);
    _readPositionsAndColorsAndSetDimensions(true);
}

void COcTree::insertObjects(const std::vector<int>& sel)
{
    for (size_t i = 0; i < sel.size(); i++)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it != nullptr) && (it != this))
            insertObject(it, 0);
    }
}

void COcTree::insertObject(const CSceneObject* obj, unsigned int theTag)
{
    if (obj->getObjectType() == sim_sceneobject_shape)
        insertShape((CShape*)obj, theTag);
    if (obj->getObjectType() == sim_sceneobject_octree)
        insertOctree((COcTree*)obj, theTag);
    if (obj->getObjectType() == sim_sceneobject_dummy)
        insertDummy((CDummy*)obj, theTag);
    if (obj->getObjectType() == sim_sceneobject_pointcloud)
        insertPointCloud((CPointCloud*)obj, theTag);
}

void COcTree::subtractPoints(const double* pts, int ptsCnt, bool ptsAreRelativeToOctree)
{
    TRACE_INTERNAL;
    const double* _pts = pts;
    std::vector<double> __pts;
    if (!ptsAreRelativeToOctree)
    {
        C7Vector tr(getFullCumulativeTransformation().getInverse());
        for (int i = 0; i < ptsCnt; i++)
        {
            C3Vector p(pts + 3 * i);
            p = tr * p;
            __pts.push_back(p(0));
            __pts.push_back(p(1));
            __pts.push_back(p(2));
        }
        _pts = &__pts[0];
    }
    if (_octreeInfo != nullptr)
    {
        if (App::worldContainer->pluginContainer->geomPlugin_removePointsFromOctree(
                _octreeInfo, C7Vector::identityTransformation, _pts, ptsCnt))
        {
            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo = nullptr;
        }
    }
    _readPositionsAndColorsAndSetDimensions(true);
}

void COcTree::subtractShape(CShape* shape)
{
    TRACE_INTERNAL;
    if (_octreeInfo != nullptr)
    {
        shape->initializeMeshCalculationStructureIfNeeded();

        C4X4Matrix octreeM(getCumulativeTransformation().getMatrix());
        C4X4Matrix shapeM(((CShape*)shape)->getCumulCenteredMeshFrame().getMatrix());
        if (App::worldContainer->pluginContainer->geomPlugin_removeMeshFromOctree(
                _octreeInfo, octreeM, shape->_meshCalculationStructure, shapeM))
        {
            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo = nullptr;
        }
        _readPositionsAndColorsAndSetDimensions(true);
    }
}

void COcTree::subtractOctree(const COcTree* octree)
{
    TRACE_INTERNAL;
    if (octree->_octreeInfo != nullptr)
        subtractOctree(octree->_octreeInfo, ((COcTree*)octree)->getFullCumulativeTransformation());
}

void COcTree::subtractDummy(const CDummy* dummy)
{
    TRACE_INTERNAL;
    subtractPoints(dummy->getFullCumulativeTransformation().X.data, 1, false);
}

void COcTree::subtractPointCloud(const CPointCloud* pointCloud)
{
    TRACE_INTERNAL;
    if (pointCloud->getPointCloudInfo() != nullptr)
    {
        const std::vector<double>* _pts = pointCloud->getPoints();
        C7Vector tr(pointCloud->getFullCumulativeTransformation());
        std::vector<double> pts;
        for (size_t i = 0; i < _pts->size() / 3; i++)
        {
            C3Vector v(&_pts->at(3 * i));
            v *= tr;
            pts.push_back(v(0));
            pts.push_back(v(1));
            pts.push_back(v(2));
        }
        subtractPoints(&pts[0], (int)pts.size() / 3, false);
    }
}

void COcTree::subtractOctree(const void* octree2Info, const C7Vector& octree2Tr)
{
    TRACE_INTERNAL;
    if (_octreeInfo != nullptr)
    {
        if (App::worldContainer->pluginContainer->geomPlugin_removeOctreeFromOctree(
                _octreeInfo, getFullCumulativeTransformation(), octree2Info, octree2Tr))
        {
            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
            _octreeInfo = nullptr;
        }
        _readPositionsAndColorsAndSetDimensions(true);
    }
}

void COcTree::subtractObjects(const std::vector<int>& sel)
{
    for (size_t i = 0; i < sel.size(); i++)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if ((it != nullptr) && (it != this))
            subtractObject(it);
    }
}

void COcTree::subtractObject(const CSceneObject* obj)
{
    if (obj->getObjectType() == sim_sceneobject_shape)
        subtractShape((CShape*)obj);
    if (obj->getObjectType() == sim_sceneobject_octree)
        subtractOctree((COcTree*)obj);
    if (obj->getObjectType() == sim_sceneobject_dummy)
        subtractDummy((CDummy*)obj);
    if (obj->getObjectType() == sim_sceneobject_pointcloud)
        subtractPointCloud((CPointCloud*)obj);
}

void COcTree::clear()
{
    TRACE_INTERNAL;
    if (_octreeInfo != nullptr)
    {
        App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
        _octreeInfo = nullptr;
    }
    _voxelPositions.clear();
    _colors.clear();
    _colorsByte.clear();
    computeBoundingBox();
    _updateOctreeEvent(false);
}

bool COcTree::getUseRandomColors() const
{
    return (_useRandomColors);
}

void COcTree::setUseRandomColors(bool r)
{
    bool diff = (_useRandomColors != r);
    if (diff)
    {
        _useRandomColors = r;
        _readPositionsAndColorsAndSetDimensions(false);
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propOctree_randomColors.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _useRandomColors);
            App::worldContainer->pushEvent();
        }
    }
}

bool COcTree::getColorIsEmissive() const
{
    return (_colorIsEmissive);
}

void COcTree::setColorIsEmissive(bool e)
{
    _colorIsEmissive = e;
}

bool COcTree::getUsePointsInsteadOfCubes() const
{
    return (_usePointsInsteadOfCubes);
}

void COcTree::setUsePointsInsteadOfCubes(bool r)
{
    bool diff = (_usePointsInsteadOfCubes != r);
    if (diff)
    {
        _usePointsInsteadOfCubes = r;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propOctree_showPoints.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _usePointsInsteadOfCubes);
            App::worldContainer->pushEvent();
        }
    }
}

int COcTree::getPointSize() const
{
    return (_pointSize);
}

void COcTree::setPointSize(int s)
{
    _pointSize = s;
}

const std::vector<double>* COcTree::getCubePositions() const
{
    TRACE_INTERNAL;
    return (&_voxelPositions);
}

std::vector<double>* COcTree::getCubePositions()
{
    TRACE_INTERNAL;
    return (&_voxelPositions);
}

std::string COcTree::getObjectTypeInfo() const
{
    return "ocTree";
}

std::string COcTree::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
}

bool COcTree::isPotentiallyCollidable() const
{
    return (true);
}
bool COcTree::isPotentiallyMeasurable() const
{
    return (true);
}
bool COcTree::isPotentiallyDetectable() const
{
    return (true);
}
bool COcTree::isPotentiallyRenderable() const
{
    return (true);
}

void COcTree::computeBoundingBox()
{
    if (_voxelPositions.size() >= 6)
    {
        C3Vector minDim(C3Vector::inf);
        C3Vector maxDim(C3Vector::ninf);
        for (size_t i = 0; i < _voxelPositions.size() / 3; i++)
        {
            C3Vector p(&_voxelPositions[3 * i]);
            minDim.keepMin(p);
            maxDim.keepMax(p);
        }
        minDim(0) -= _cellSize * 0.5;
        minDim(1) -= _cellSize * 0.5;
        minDim(2) -= _cellSize * 0.5;
        maxDim(0) += _cellSize * 0.5;
        maxDim(1) += _cellSize * 0.5;
        maxDim(2) += _cellSize * 0.5;
        C7Vector fr;
        fr.setIdentity();
        fr.X = (maxDim + minDim) * 0.5;
        _setBB(fr, (maxDim - minDim) * 0.5);
    }
    else
        _setBB(C7Vector::identityTransformation, C3Vector(0.1, 0.1, 0.1));
}

void COcTree::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        color.setEventParams(true, _objectHandle);
    else
        color.setEventParams(true, -1);
}

void COcTree::scaleObject(double scalingFactor)
{
    _cellSize *= scalingFactor;
    for (size_t i = 0; i < _voxelPositions.size(); i++)
        _voxelPositions[i] *= scalingFactor;
    if (_octreeInfo != nullptr)
        App::worldContainer->pluginContainer->geomPlugin_scaleOctree(_octreeInfo, scalingFactor);
    _updateOctreeEvent(false);

    CSceneObject::scaleObject(scalingFactor);
}

void COcTree::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void COcTree::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->openKeyMap("voxels");
    ev->appendKeyDoubleArray("positions", _voxelPositions.data(), _voxelPositions.size());
    ev->appendKeyUCharArray("colors", _colorsByte.data(), _colorsByte.size());
    ev->closeArrayOrMap(); // voxels
    ev->appendKeyDouble(propOctree_voxelSize.name, _cellSize);
    ev->appendKeyBool(propOctree_showPoints.name, _usePointsInsteadOfCubes);
    ev->appendKeyBool(propOctree_randomColors.name, _useRandomColors);
    ev->closeArrayOrMap(); // octree
#else
    color.addGenesisEventData(ev);
    _updateOctreeEvent(false, ev);
#endif
}

CSceneObject* COcTree::copyYourself()
{
    COcTree* newOctree = (COcTree*)CSceneObject::copyYourself();

    newOctree->_cellSize = _cellSize;
    color.copyYourselfInto(&newOctree->color);

    if (_octreeInfo != nullptr)
        newOctree->_octreeInfo = App::worldContainer->pluginContainer->geomPlugin_copyOctree(_octreeInfo);
    newOctree->_voxelPositions.assign(_voxelPositions.begin(), _voxelPositions.end());
    newOctree->_colors.assign(_colors.begin(), _colors.end());
    newOctree->_colorsByte.assign(_colorsByte.begin(), _colorsByte.end());
    newOctree->_showOctreeStructure = _showOctreeStructure;
    newOctree->_useRandomColors = _useRandomColors;
    newOctree->_colorIsEmissive = _colorIsEmissive;
    newOctree->_usePointsInsteadOfCubes = _usePointsInsteadOfCubes;

    return (newOctree);
}

void COcTree::setCellSize(double theNewSize)
{
    if (_octreeInfo != nullptr)
        theNewSize = tt::getLimitedFloat(_cellSize, 1.0, theNewSize); // we can't reduce the cell size for an existing octree, because it doesn't make sense!
    else
        theNewSize = tt::getLimitedFloat(0.001, 1.0, theNewSize);
    if (theNewSize != _cellSize)
    {
        _cellSize = theNewSize;
        if (_octreeInfo != nullptr)
        { // we reconstruct the octree from this octree:
            void* octree2Info = _octreeInfo;
            _octreeInfo = nullptr;
            clear();
            insertOctree(octree2Info, getFullCumulativeTransformation().getMatrix(), 0);
            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(octree2Info);
        }
    }
}

double COcTree::getCellSize() const
{
    return _cellSize;
}

void COcTree::setCellSizeForDisplay(double theNewSizeForDisplay)
{
    _cellSizeForDisplay = theNewSizeForDisplay;
}

double COcTree::getCellSizeForDisplay() const
{
    return _cellSizeForDisplay;
}

bool COcTree::getShowOctree() const
{
    return _showOctreeStructure;
}

void COcTree::setShowOctree(bool show)
{
    _showOctreeStructure = show;
}

const void* COcTree::getOctreeInfo() const
{
    return (_octreeInfo);
}

void* COcTree::getOctreeInfo()
{
    return (_octreeInfo);
}

void COcTree::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void COcTree::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void COcTree::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void COcTree::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}
void COcTree::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}
void COcTree::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}
void COcTree::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}

void COcTree::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void COcTree::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void COcTree::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void COcTree::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void COcTree::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    CSceneObject::simulationEnded();
}

void COcTree::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_iz");
            ar << _cellSize;
            ar << _pointSize;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 1, _showOctreeStructure);
            SIM_SET_CLEAR_BIT(dummy, 2, _useRandomColors);
            SIM_SET_CLEAR_BIT(dummy, 3, _usePointsInsteadOfCubes);
            SIM_SET_CLEAR_BIT(dummy, 4, true);
            SIM_SET_CLEAR_BIT(dummy, 5, _colorIsEmissive);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            color.serialize(ar, 0);
            if (ar.setWritingMode())
                color.serialize(ar, 0);

            // Keep this at the end
            if (_octreeInfo != nullptr)
            {
                std::vector<unsigned char> data;

                App::worldContainer->pluginContainer->geomPlugin_getOctreeSerializationData(_octreeInfo, data);
                ar.storeDataName("_o2");
                ar.setCountingMode(true);
                for (size_t i = 0; i < data.size(); i++)
                    ar << data[i];
                ar.flush(false);
                if (ar.setWritingMode(true))
                {
                    for (size_t i = 0; i < data.size(); i++)
                        ar << data[i];
                    ar.flush(false);
                }
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Siz") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        ar >> _pointSize;
                        _cellSize = (double)bla;
                    }

                    if (theName.compare("_iz") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _cellSize;
                        ar >> _pointSize;
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _showOctreeStructure = SIM_IS_BIT_SET(dummy, 1);
                        _useRandomColors = SIM_IS_BIT_SET(dummy, 2);
                        _usePointsInsteadOfCubes = SIM_IS_BIT_SET(dummy, 3);
                        // _saveCalculationStructure = SIM_IS_BIT_SET(dummy, 4);
                        _colorIsEmissive = SIM_IS_BIT_SET(dummy, 5);
                    }
                    if (theName.compare("Col") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        color.serialize(ar, 0);
                    }
                    if (theName.compare("Pt2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        std::vector<double> pts;
                        pts.resize(cnt * 3);
                        std::vector<unsigned char> cols;
                        cols.resize(cnt * 3);
                        std::vector<unsigned int> tags;
                        tags.resize(cnt, 0);
                        float bla;
                        for (int i = 0; i < cnt; i++)
                        {
                            ar >> bla;
                            pts[3 * i + 0] = (double)bla;
                            ar >> bla;
                            pts[3 * i + 1] = (double)bla;
                            ar >> bla;
                            pts[3 * i + 2] = (double)bla;
                            ar >> cols[3 * i + 0];
                            ar >> cols[3 * i + 1];
                            ar >> cols[3 * i + 2];
                        }
                        // Now we need to rebuild the octree:
                        if (cnt > 0)
                            insertPoints(&pts[0], cnt, true, &cols[0], true, &tags[0], 0);
                        else
                            _readPositionsAndColorsAndSetDimensions(false);
                    }

                    if (theName.compare("_t2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        std::vector<double> pts;
                        pts.resize(cnt * 3);
                        std::vector<unsigned char> cols;
                        cols.resize(cnt * 3);
                        std::vector<unsigned int> tags;
                        tags.resize(cnt, 0);
                        for (int i = 0; i < cnt; i++)
                        {
                            ar >> pts[3 * i + 0];
                            ar >> pts[3 * i + 1];
                            ar >> pts[3 * i + 2];
                            ar >> cols[3 * i + 0];
                            ar >> cols[3 * i + 1];
                            ar >> cols[3 * i + 2];
                        }
                        // Now we need to rebuild the octree:
                        clear(); // we might also have read "Pt2"
                        if (cnt > 0)
                            insertPoints(&pts[0], cnt, true, &cols[0], true, &tags[0], 0);
                        else
                            _readPositionsAndColorsAndSetDimensions(false);
                    }

                    if (theName.compare("Co2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        std::vector<unsigned char> data;
                        data.reserve(byteQuantity);
                        unsigned char dummy;
                        for (int i = 0; i < byteQuantity; i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        if (_octreeInfo != nullptr)
                            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
                        _octreeInfo =
                            App::worldContainer->pluginContainer->geomPlugin_getOctreeFromSerializationData_float(
                                &data[0]);
                        _readPositionsAndColorsAndSetDimensions(false);
                    }

                    if (theName.compare("_o2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        std::vector<unsigned char> data;
                        unsigned char dummy;
                        for (int i = 0; i < byteQuantity; i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        if (_octreeInfo != nullptr) // we could have also read "Co2"
                            App::worldContainer->pluginContainer->geomPlugin_destroyOctree(_octreeInfo);
                        _octreeInfo =
                            App::worldContainer->pluginContainer->geomPlugin_getOctreeFromSerializationData(&data[0]);
                        _readPositionsAndColorsAndSetDimensions(false);
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
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("cellSize", _cellSize);
            ar.xmlAddNode_int("pointSize", _pointSize);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showStructure", _showOctreeStructure);
            ar.xmlAddNode_bool("randomColors", _useRandomColors);
            ar.xmlAddNode_bool("pointsInsteadOfCubes", _usePointsInsteadOfCubes);
            ar.xmlAddNode_bool("emissiveColor", _colorIsEmissive);
            ar.xmlPopNode();

            ar.xmlPushNewNode("applyColor");
            if (exhaustiveXml)
                color.serialize(ar, 0);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(color.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("points", rgb, 3);
            }
            ar.xmlPopNode();

            if (exhaustiveXml)
                ar.xmlAddNode_int("voxelCount", int(_voxelPositions.size() / 3));

            if (_octreeInfo != nullptr)
            {
                if (ar.xmlSaveDataInline(_voxelPositions.size() * 4 + _colors.size() * 3 / 4) || (!exhaustiveXml))
                {
                    ar.xmlAddNode_floats("voxelPositions", _voxelPositions);
                    std::vector<int> tmp;
                    for (size_t i = 0; i < _voxelPositions.size() / 3; i++)
                    {
                        tmp.push_back((unsigned int)(_colors[4 * i + 0] * 255.1));
                        tmp.push_back((unsigned int)(_colors[4 * i + 1] * 255.1));
                        tmp.push_back((unsigned int)(_colors[4 * i + 2] * 255.1));
                    }
                    ar.xmlAddNode_ints("voxelColors", tmp);
                }
                else
                {
                    CSer* w = ar.xmlAddNode_binFile(
                        "file", (_objectAlias + "-octree-" + std::to_string(_objectHandle)).c_str());
                    w[0] << int(_voxelPositions.size());
                    for (size_t i = 0; i < _voxelPositions.size(); i++)
                        w[0] << (float)_voxelPositions[i]; // keep this as float

                    for (size_t i = 0; i < _voxelPositions.size() / 3; i++)
                    {
                        w[0] << (unsigned char)(_colors[4 * i + 0] * 255.1);
                        w[0] << (unsigned char)(_colors[4 * i + 1] * 255.1);
                        w[0] << (unsigned char)(_colors[4 * i + 2] * 255.1);
                    }
                    w->flush();
                    w->writeClose();
                    delete w;
                }
            }
        }
        else
        {
            ar.xmlGetNode_float("cellSize", _cellSize, exhaustiveXml);
            ar.xmlGetNode_int("pointSize", _pointSize, exhaustiveXml);

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("showStructure", _showOctreeStructure, exhaustiveXml);
                ar.xmlGetNode_bool("randomColors", _useRandomColors, exhaustiveXml);
                ar.xmlGetNode_bool("pointsInsteadOfCubes", _usePointsInsteadOfCubes, exhaustiveXml);
                ar.xmlGetNode_bool("emissiveColor", _colorIsEmissive, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("applyColor", exhaustiveXml))
            {
                if (exhaustiveXml)
                    color.serialize(ar, 0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                       sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                int voxelCnt = 0;
                ar.xmlGetNode_int("voxelCount", voxelCnt);

                if (voxelCnt > 0)
                {
                    std::vector<double> pts;
                    std::vector<unsigned char> cols;
                    std::vector<unsigned int> tags;
                    if (ar.xmlGetNode_floats("voxelPositions", pts, false))
                        ar.xmlGetNode_uchars("voxelColors", cols);
                    else
                    {
                        CSer* w = ar.xmlGetNode_binFile("file");
                        int cnt;
                        w[0] >> cnt;
                        pts.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                        {
                            float bla; // keep as float
                            w[0] >> bla;
                            pts[i] = (double)bla;
                        }
                        cols.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            w[0] >> cols[i];
                        w->readClose();
                        delete w;
                    }
                    tags.resize(voxelCnt, 0);
                    insertPoints(&pts[0], voxelCnt, true, &cols[0], true, &tags[0], 0);
                }
                else
                    _readPositionsAndColorsAndSetDimensions(false);
            }
            else
            {
                std::vector<double> pts;
                std::vector<unsigned char> cols;
                std::vector<unsigned int> tags;
                if (ar.xmlGetNode_floats("voxelPositions", pts, exhaustiveXml))
                    ar.xmlGetNode_uchars("voxelColors", cols, exhaustiveXml);
                if (pts.size() >= 3)
                {
                    while ((pts.size() % 3) != 0)
                        pts.pop_back();
                    if (cols.size() < pts.size())
                    {
                        cols.resize(pts.size());
                        for (size_t i = 0; i < pts.size() / 3; i++)
                        {
                            cols[3 * i + 0] = (unsigned char)(color.getColorsPtr()[0] * 255.1);
                            cols[3 * i + 1] = (unsigned char)(color.getColorsPtr()[1] * 255.1);
                            cols[3 * i + 2] = (unsigned char)(color.getColorsPtr()[2] * 255.1);
                        }
                    }
                    tags.resize(pts.size() / 3, 0);
                    insertPoints(&pts[0], int(pts.size() / 3), true, &cols[0], true, &tags[0], 0);
                }
                else
                    _readPositionsAndColorsAndSetDimensions(false);
            }
            computeBoundingBox();
        }
    }
}

void COcTree::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
}

void COcTree::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void COcTree::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

#ifdef SIM_WITH_GUI
void COcTree::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayOctree(this, renderingObject, displayAttrib);
}
#endif

int COcTree::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_randomColors.name)
        {
            setUseRandomColors(pState);
            retVal = 1;
        }
        else if (_pName == propOctree_showPoints.name)
        {
            setUsePointsInsteadOfCubes(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int COcTree::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_randomColors.name)
        {
            pState = _useRandomColors;
            retVal = 1;
        }
        else if (_pName == propOctree_showPoints.name)
        {
            pState = _usePointsInsteadOfCubes;
            retVal = 1;
        }
    }

    return retVal;
}

int COcTree::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = color.setFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_voxelSize.name)
        {
            setCellSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int COcTree::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = color.getFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_voxelSize.name)
        {
            pState = _cellSize;
            retVal = 1;
        }
    }

    return retVal;
}

int COcTree::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_objectMetaInfo.name)
        {
            pState = OBJECT_META_INFO;
            retVal = 1;
        }
    }

    return retVal;
}

int COcTree::setBufferProperty(const char* ppName, const char* buffer, int bufferL)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBufferProperty(ppName, buffer, bufferL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int COcTree::getBufferProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    pState.clear();
    int retVal = CSceneObject::getBufferProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_colors.name)
        {
            retVal = 1;
            pState.assign(_colorsByte.begin(), _colorsByte.end());
        }
    }

    return retVal;
}

int COcTree::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = color.setColorProperty(ppName, pState);
    return retVal;
}

int COcTree::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = color.getColorProperty(ppName, pState);
    return retVal;
}

int COcTree::setFloatArrayProperty(const char* ppName, const double* v, int vL)
{
    std::string _pName(ppName);
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setFloatArrayProperty(ppName, v, vL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int COcTree::getFloatArrayProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName(ppName);
    pState.clear();
    int retVal = CSceneObject::getFloatArrayProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propOctree_voxels.name)
        {
            retVal = 1;
            pState.assign(_voxelPositions.begin(), _voxelPositions.end());
        }
    }

    return retVal;
}

int COcTree::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "ocTree";
        retVal = color.getPropertyName(index, pName, excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_ocTree.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_ocTree[i].name, pName.c_str()))
            {
                if ((allProps_ocTree[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_ocTree[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int COcTree::getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "ocTree";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "", excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_ocTree.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_ocTree[i].name, pName.c_str()))
            {
                if ((allProps_ocTree[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_ocTree[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int COcTree::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = color.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_ocTree.size(); i++)
        {
            if (strcmp(allProps_ocTree[i].name, ppName) == 0)
            {
                retVal = allProps_ocTree[i].type;
                info = allProps_ocTree[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_ocTree[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_ocTree[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
        if (retVal != -1)
        {
            if (_pName == propOctree_voxels.name)
            {
                if (_voxelPositions.size() > LARGE_PROPERTY_SIZE)
                    info = info | sim_propertyinfo_largedata;
            }
            if (_pName == propOctree_colors.name)
            {
                if (_colorsByte.size() * 3 > LARGE_PROPERTY_SIZE)
                    info = info | sim_propertyinfo_largedata;
            }
        }
    }
    return retVal;
}

int COcTree::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo_bstatic(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_ocTree.size(); i++)
        {
            if (strcmp(allProps_ocTree[i].name, ppName) == 0)
            {
                retVal = allProps_ocTree[i].type;
                info = allProps_ocTree[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_ocTree[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_ocTree[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}

std::string COcTree::getObjectState() const
{
    long long int h = 0;
    for (size_t i = 0; i < _voxelPositions.size(); i++)
        h += ((long long int*)&_voxelPositions[i])[0];
    for (size_t i = 0; i < _colorsByte.size(); i++)
        h += _colorsByte[i];
    return std::string(reinterpret_cast<const char*>(&h), sizeof(h));
}
