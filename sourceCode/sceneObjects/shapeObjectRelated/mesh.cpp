#include <mesh.h>
#include <global.h>
#include <tt.h>
#include <meshRoutines.h>
#include <meshManip.h>
#include <utils.h>
#include <app.h>
#include <tt.h>
#include <base64.h>
#include <simFlavor.h>
#include <algos.h>
#ifdef SIM_WITH_GUI
#include <shapeRendering.h>
#endif

int CMesh::_nextUniqueID = 0;
unsigned int CMesh::_extRendererUniqueObjectID = 0;
unsigned int CMesh::_extRendererUniqueMeshID = 0;
unsigned int CMesh::_extRendererUniqueTextureID = 0;

// temp, for serialization purpose:
std::vector<std::vector<float> *> CMesh::_tempVerticesForDisk;
std::vector<std::vector<int> *> CMesh::_tempIndicesForDisk;
std::vector<std::vector<float> *> CMesh::_tempNormalsForDisk;
std::vector<std::vector<unsigned char> *> CMesh::_tempEdgesForDisk;

CMesh::CMesh()
{
    _commonInit();
}

CMesh::CMesh(const C7Vector &meshFrame, const std::vector<double> &vertices, const std::vector<int> &indices,
             const std::vector<double> *optNormals, const std::vector<float> *optTexCoords, int options)
{
    _commonInit();
    _vertices.assign(vertices.begin(), vertices.end());
    _indices.assign(indices.begin(), indices.end());
    if (optNormals == nullptr)
    {
        CMeshManip::getNormals(&_vertices, &_indices, &_normals);
        _recomputeNormals();
    }
    else
        _normals.assign(optNormals->begin(), optNormals->end());

    // Do not modify meshes at this stage (other than rotation/translation). If you must do it, do it before
    // This is however ok:
    CMeshRoutines::removeNonReferencedVertices(_vertices, _indices);

    std::vector<float> texCoords;
    if (optTexCoords != nullptr)
        texCoords.assign(optTexCoords->begin(), optTexCoords->end());

    // Express everything in the meshFrame:
    C7Vector inv(meshFrame.getInverse());
    _transformMesh(inv);

    // Find the _bbFrame, and express everything in that frame:
    _bbFrame = CAlgos::getMeshBoundingBox(_vertices, _indices, false);
    inv = _bbFrame.getInverse();
    _transformMesh(inv);
    _bbSize = _computeBBSize();
    _culling = ((options & 1) != 0);
    _visibleEdges = ((options & 2) != 0);

    delete _textureProperty;
    _textureProperty = nullptr;
    if (optTexCoords != nullptr)
    {
        _textureProperty = new CTextureProperty();
        _textureProperty->setInterpolateColors((options & 4) == 0);
        if ((options & 8) != 0)
            _textureProperty->setApplyMode(1);
        else
            _textureProperty->setApplyMode(0);
        _textureProperty->setFixedCoordinates(&texCoords);
    }

    _computeVisibleEdges();
    checkIfConvex();

#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId = -1;
    _normalBufferId = -1;
    _edgeBufferId = -1;
#endif
}

CMesh::~CMesh()
{
#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);
#endif
    delete _textureProperty;
}

void CMesh::_commonInit()
{
    color.setDefaultValues();
    color.setColor(0.9f, 0.9f, 0.9f, sim_colorcomponent_ambient_diffuse);
    edgeColor_DEPRECATED.setColorsAllBlack();
    insideColor_DEPRECATED.setDefaultValues();

    _purePrimitive = sim_primitiveshape_none;
    _purePrimitiveXSizeOrDiameter = 0.0;
    _purePrimitiveYSize = 0.0;
    _purePrimitiveZSizeOrHeight = 0.0;
    _purePrimitiveInsideScaling = 0.0; // no inside part

    _heightfieldXCount = 0;
    _heightfieldYCount = 0;

    _textureProperty = nullptr;

#ifdef SIM_WITH_GUI
    _vertexBufferId = -1;
    _normalBufferId = -1;
    _edgeBufferId = -1;
#endif

    _edgeWidth_DEPRERCATED = 1;
    _visibleEdges = false;
    _hideEdgeBorders_OLD = false;
    _culling = false;
    _convex = false;
    _displayInverted_DEPRECATED = false;
    _wireframe_OLD = false;
    _insideAndOutsideFacesSameColor_DEPRECATED = true;
    _shadingAngle = 0.0;
    _edgeThresholdAngle = 0.0;
    _uniqueID = _nextUniqueID++;

    _extRendererObjectId = 0;
    _extRendererMeshId = 0;
    _extRendererTextureId = 0;
}

void CMesh::performSceneObjectLoadingMapping(const std::map<int, int> *map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
        _textureProperty->performObjectLoadingMapping(map);
}

void CMesh::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
        _textureProperty->performTextureObjectLoadingMapping(map);
}

void CMesh::announceSceneObjectWillBeErased(const CSceneObject *object)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
    {
        if (_textureProperty->announceObjectWillBeErased(object))
        {
            delete _textureProperty;
            _textureProperty = nullptr;
        }
    }
}

void CMesh::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
        _textureProperty->addTextureDependencies(shapeID, _uniqueID);
}

int CMesh::getTextureCount() const
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
        return (1);
    return (0);
}

bool CMesh::hasTextureThatUsesFixedTextureCoordinates() const
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty != nullptr)
        return (_textureProperty->getFixedCoordinates());
    return (false);
}

bool CMesh::getContainsTransparentComponents() const
{ // function has virtual/non-virtual counterpart!
    return (color.getTranslucid() || insideColor_DEPRECATED.getTranslucid());
}

CMesh *CMesh::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CMesh *newIt = new CMesh();
    copyWrapperData(newIt);

    color.copyYourselfInto(&newIt->color);
    insideColor_DEPRECATED.copyYourselfInto(&newIt->insideColor_DEPRECATED);
    edgeColor_DEPRECATED.copyYourselfInto(&newIt->edgeColor_DEPRECATED);

    newIt->_purePrimitive = _purePrimitive;
    newIt->_purePrimitiveXSizeOrDiameter = _purePrimitiveXSizeOrDiameter;
    newIt->_purePrimitiveYSize = _purePrimitiveYSize;
    newIt->_purePrimitiveZSizeOrHeight = _purePrimitiveZSizeOrHeight;
    newIt->_purePrimitiveInsideScaling = _purePrimitiveInsideScaling;

    newIt->_heightfieldXCount = _heightfieldXCount;
    newIt->_heightfieldYCount = _heightfieldYCount;
    newIt->_heightfieldHeights.insert(newIt->_heightfieldHeights.end(), _heightfieldHeights.begin(),
                                      _heightfieldHeights.end());

    newIt->_visibleEdges = _visibleEdges;
    newIt->_hideEdgeBorders_OLD = _hideEdgeBorders_OLD;
    newIt->_culling = _culling;
    newIt->_displayInverted_DEPRECATED = _displayInverted_DEPRECATED;
    newIt->_insideAndOutsideFacesSameColor_DEPRECATED = _insideAndOutsideFacesSameColor_DEPRECATED;
    newIt->_wireframe_OLD = _wireframe_OLD;
    newIt->_shadingAngle = _shadingAngle;
    newIt->_edgeThresholdAngle = _edgeThresholdAngle;
    newIt->_edgeWidth_DEPRERCATED = _edgeWidth_DEPRERCATED;

    newIt->_convex = _convex;
    newIt->_vertices.assign(_vertices.begin(), _vertices.end());
    newIt->_indices.assign(_indices.begin(), _indices.end());
    newIt->_normals.assign(_normals.begin(), _normals.end());
    newIt->_edges.assign(_edges.begin(), _edges.end());

    newIt->_verticesForDisplayAndDisk.assign(_verticesForDisplayAndDisk.begin(), _verticesForDisplayAndDisk.end());
    newIt->_normalsForDisplayAndDisk.assign(_normalsForDisplayAndDisk.begin(), _normalsForDisplayAndDisk.end());

#ifdef SIM_WITH_GUI
    newIt->_vertexBufferId = _vertexBufferId;
    newIt->_normalBufferId = _normalBufferId;
    newIt->_edgeBufferId = _edgeBufferId;

    increaseVertexBufferRefCnt(_vertexBufferId);
    increaseNormalBufferRefCnt(_normalBufferId);
    increaseEdgeBufferRefCnt(_edgeBufferId);
#endif

    if (_textureProperty != nullptr)
        newIt->_textureProperty = _textureProperty->copyYourself();

    return (newIt);
}

void CMesh::scale(double isoVal)
{ // Function has virtual/non-virtual counterpart
    CMeshWrapper::scale(isoVal);

    _purePrimitiveXSizeOrDiameter *= isoVal;
    _purePrimitiveYSize *= isoVal;
    _purePrimitiveZSizeOrHeight *= isoVal;
    if (_purePrimitive == sim_primitiveshape_heightfield)
    {
        for (int i = 0; i < _heightfieldXCount * _heightfieldYCount; i++)
            _heightfieldHeights[i] *= isoVal;
    }

    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        C3Vector v(_vertices.data() + 3 * i);
        v *= isoVal;
        _vertices[3 * i + 0] = v(0);
        _vertices[3 * i + 1] = v(1);
        _vertices[3 * i + 2] = v(2);
        _verticesForDisplayAndDisk[3 * i + 0] = (float)v(0);
        _verticesForDisplayAndDisk[3 * i + 1] = (float)v(1);
        _verticesForDisplayAndDisk[3 * i + 2] = (float)v(2);
    }

    if (_textureProperty != nullptr)
        _textureProperty->scaleObject(isoVal);
    if (isoVal < 0.0) // flip faces
        checkIfConvex();

#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId = -1;
    _normalBufferId = -1;
    _edgeBufferId = -1;
#endif
}

void CMesh::scale(double xVal, double yVal, double zVal)
{ // We scale along _bbFrame, not _iFrame, since non-iso scaling of inertia is delicate
  // So we leave inertia items untouched
  // Only for non-compound shapes
    if (_purePrimitive == sim_primitiveshape_plane)
        zVal = 1.0;
    if (_purePrimitive == sim_primitiveshape_disc)
    {
        zVal = 1.0;
        yVal = xVal;
    }
    if ((_purePrimitive == sim_primitiveshape_cylinder) || (_purePrimitive == sim_primitiveshape_cone) ||
        (_purePrimitive == sim_primitiveshape_heightfield))
        yVal = xVal;
    if (_purePrimitive == sim_primitiveshape_capsule)
    {
        yVal = xVal;
        zVal = xVal;
    }
    if (_purePrimitive == sim_primitiveshape_heightfield)
    {
        for (int i = 0; i < _heightfieldXCount * _heightfieldYCount; i++)
            _heightfieldHeights[i] *= zVal;
    }

    _purePrimitiveXSizeOrDiameter *= xVal;
    _purePrimitiveYSize *= yVal;
    _purePrimitiveZSizeOrHeight *= zVal;

    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        C3Vector v(_vertices.data() + 3 * i);
        v(0) *= xVal;
        v(1) *= yVal;
        v(2) *= zVal;
        _vertices[3 * i + 0] = v(0);
        _vertices[3 * i + 1] = v(1);
        _vertices[3 * i + 2] = v(2);
        _verticesForDisplayAndDisk[3 * i + 0] = (float)v(0);
        _verticesForDisplayAndDisk[3 * i + 1] = (float)v(1);
        _verticesForDisplayAndDisk[3 * i + 2] = (float)v(2);
    }

    _bbSize(0) *= xVal;
    _bbSize(1) *= yVal;
    _bbSize(2) *= zVal;

    C7Vector inv(_bbFrame.getInverse());
    inv.X(0) *= xVal;
    inv.X(1) *= yVal;
    inv.X(2) *= zVal;
    _bbFrame = inv.getInverse();

    //    ignore the texture scaling...
    //    if (_textureProperty!=nullptr)
    //        _textureProperty->scaleObject(xVal);
    actualizeGouraudShadingAndVisibleEdges();         // we need to recompute the normals and edges
    if ((xVal < 0.0) || (yVal < 0.0) || (zVal < 0.0)) // that effectively flips faces!
        checkIfConvex();

#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId = -1;
    _normalBufferId = -1;
    _edgeBufferId = -1;
#endif
}

void CMesh::_transformMesh(const C7Vector &tr)
{
    if (_textureProperty != nullptr)
        _textureProperty->transformTexturePose(tr);
    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        C3Vector v(_vertices.data() + 3 * i);
        v = tr * v;
        _vertices[3 * i + 0] = v(0);
        _vertices[3 * i + 1] = v(1);
        _vertices[3 * i + 2] = v(2);
    }
    for (size_t i = 0; i < _normals.size() / 3; i++)
    {
        C3Vector n(&_normals[3 * i]);
        n.normalize();
        n = tr.Q * n;
        _normals[3 * i + 0] = n(0);
        _normals[3 * i + 1] = n(1);
        _normals[3 * i + 2] = n(2);
    }
    _updateDisplayAndDiskValues();

#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);
    _vertexBufferId = -1;
    _normalBufferId = -1;
    _edgeBufferId = -1;
#endif
}

void CMesh::setBBFrame(const C7Vector &bbFrame)
{ // function has virtual/non-virtual counterpart!
    CMeshWrapper::setBBFrame(bbFrame);
}

bool CMesh::reorientBB(const C4Vector *rot)
{ // function has virtual/non-virtual counterpart!
    bool retVal = false;
    if (!isPure())
    {
        C7Vector initialFrame(_bbFrame);
        if (rot != nullptr)
        {
            C7Vector tr;
            tr.X.clear();
            tr.Q = rot[0];
            _transformMesh(tr.getInverse() * _bbFrame);
            C7Vector bbc;
            bbc.setIdentity();
            _bbSize = _computeBBSize(&bbc.X);
            _bbFrame = tr * bbc;
            _transformMesh(bbc.getInverse());
        }
        else
        {
            _transformMesh(_bbFrame);
            _bbFrame = CAlgos::getMeshBoundingBox(_vertices, _indices, true);
            _transformMesh(_bbFrame.getInverse());
            _bbSize = _computeBBSize();
        }
        retVal = true;
    }
    return (retVal);
}

void CMesh::setPurePrimitiveType(int theType, double xOrDiameter, double y, double zOrHeight)
{ // function has virtual/non-virtual counterpart!
    _purePrimitive = theType;
    _purePrimitiveXSizeOrDiameter = xOrDiameter;
    _purePrimitiveYSize = y;
    _purePrimitiveZSizeOrHeight = zOrHeight;

    // Set the convex flag only to true!! never to false, at least not here
    if ((theType != sim_primitiveshape_heightfield) && (theType != sim_primitiveshape_none) &&
        (theType != sim_primitiveshape_plane) && (theType != sim_primitiveshape_disc))
        _convex = true;
    if (theType != sim_primitiveshape_heightfield)
    {
        _heightfieldHeights.clear();
        _heightfieldXCount = 0;
        _heightfieldYCount = 0;
    }

    if ((theType != sim_primitiveshape_cuboid) && (theType != sim_primitiveshape_spheroid) &&
        (theType != sim_primitiveshape_cylinder) && (theType != sim_primitiveshape_capsule))
        _purePrimitiveInsideScaling = 0.0; // no inside part!
}

int CMesh::getPurePrimitiveType() const
{ // function has virtual/non-virtual counterpart!
    return (_purePrimitive);
}

bool CMesh::isMesh() const
{ // function has virtual/non-virtual counterpart!
    return (true);
}

bool CMesh::isPure() const
{ // function has virtual/non-virtual counterpart!
    return (_purePrimitive != sim_primitiveshape_none);
}

bool CMesh::isConvex() const
{ // function has virtual/non-virtual counterpart!
    return (_convex);
}

CMesh *CMesh::getFirstMesh()
{ // function has virtual/non-virtual counterpart!
    return (this);
}

int CMesh::countTriangles() const
{
    return (int(_indices.size() / 3));
}

void CMesh::getCumulativeMeshes(const C7Vector &parentCumulTr, std::vector<double> &vertices, std::vector<int> *indices,
                                std::vector<double> *normals)
{ // function has virtual/non-virtual counterpart!
    size_t offset = vertices.size() / 3;
    C7Vector tr(parentCumulTr * _iFrame * _bbFrame);
    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        C3Vector v(_vertices.data() + 3 * i);
        v *= tr;
        vertices.push_back(v(0));
        vertices.push_back(v(1));
        vertices.push_back(v(2));
    }
    if (indices != nullptr)
    {
        for (size_t i = 0; i < _indices.size(); i++)
            indices->push_back(_indices[i] + int(offset));
    }
    if (normals != nullptr)
    {
        for (size_t i = 0; i < _normals.size() / 3; i++)
        {
            C3Vector v(_normals.data() + 3 * i);
            v = tr.Q * v;
            normals->push_back(v(0));
            normals->push_back(v(1));
            normals->push_back(v(2));
        }
    }
}

void CMesh::getCumulativeMeshes(const C7Vector &parentCumulTr, const CMeshWrapper *wrapper,
                                std::vector<double> &vertices, std::vector<int> *indices, std::vector<double> *normals)
{ // function has virtual/non-virtual counterpart!
    if ((wrapper == this) || (wrapper == nullptr))
    {
        size_t offset = vertices.size() / 3;
        C7Vector tr(parentCumulTr * _iFrame * _bbFrame);
        for (size_t i = 0; i < _vertices.size() / 3; i++)
        {
            C3Vector v(_vertices.data() + 3 * i);
            v *= tr;
            vertices.push_back(v(0));
            vertices.push_back(v(1));
            vertices.push_back(v(2));
        }
        if (indices != nullptr)
        {
            for (size_t i = 0; i < _indices.size(); i++)
                indices->push_back(_indices[i] + int(offset));
        }
        if (normals != nullptr)
        {
            for (size_t i = 0; i < _normals.size() / 3; i++)
            {
                C3Vector v(_normals.data() + 3 * i);
                v = tr.Q * v;
                normals->push_back(v(0));
                normals->push_back(v(1));
                normals->push_back(v(2));
            }
        }
    }
}

void CMesh::setColor(const CShape *shape, int &elementIndex, const char *colorName, int colorComponent,
                     const float *rgbData, int &rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    if ((colorName == nullptr) || (color.getColorName().compare(colorName) == 0) ||
        (strcmp(colorName, "@compound") == 0))
    {
        bool compoundColors = (colorName != nullptr) && (strcmp(colorName, "@compound") == 0);
        if (colorComponent < sim_colorcomponent_transparency)
        { // regular components
            for (int i = 0; i < 3; i++)
                color.getColorsPtr()[colorComponent * 3 + i] = rgbData[rgbDataOffset + i];
            if (compoundColors)
                rgbDataOffset += 3;
        }
        if (colorComponent == sim_colorcomponent_transparency)
        {
            color.setOpacity(rgbData[rgbDataOffset + 0]);
            color.setTranslucid(rgbData[rgbDataOffset + 0] < 1.0);
            if (compoundColors)
                rgbDataOffset += 1;
        }
        if (colorComponent == sim_colorcomponent_auxiliary)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                color.getColorsPtr()[12 + i] = rgbData[rgbDataOffset + i];
            if (compoundColors)
                rgbDataOffset += 3;
        }
        if (shape != nullptr)
            color.pushShapeColorChangeEvent(shape->getObjectHandle(), elementIndex);
    }
    if ((colorName != nullptr) && (insideColor_DEPRECATED.getColorName().compare(colorName) == 0))
    { // OLD, deprecated
        if (colorComponent < 4)
        { // regular components
            for (int i = 0; i < 3; i++)
                insideColor_DEPRECATED.getColorsPtr()[colorComponent * 3 + i] = rgbData[rgbDataOffset + i];
        }
        if (colorComponent == 4)
        {
            insideColor_DEPRECATED.setOpacity(rgbData[rgbDataOffset + 0]);
            insideColor_DEPRECATED.setTranslucid(rgbData[rgbDataOffset + 0] < 1.0);
        }
        if (colorComponent == 5)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                insideColor_DEPRECATED.getColorsPtr()[12 + i] = rgbData[rgbDataOffset + i];
        }
    }
    if ((colorName != nullptr) && (edgeColor_DEPRECATED.getColorName().compare(colorName) == 0))
    { // OLD, deprecated
        if (colorComponent < 4)
        { // regular components
            for (int i = 0; i < 3; i++)
                edgeColor_DEPRECATED.getColorsPtr()[colorComponent * 3 + i] = rgbData[rgbDataOffset + i];
        }
        if (colorComponent == 5)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                edgeColor_DEPRECATED.getColorsPtr()[12 + i] = rgbData[rgbDataOffset + i];
        }
    }

    if ((colorName != nullptr) && (strlen(colorName) == 2) && (colorName[0] == '@'))
    { // operations in the HSL space
        if (colorName[1] == '0')
        { // color
            if (colorComponent < 4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(color.getColorsPtr() + colorComponent * 3, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, color.getColorsPtr() + colorComponent * 3);
            }
            if (colorComponent == 4)
            {
                color.setOpacity(tt::getLimitedFloat(0.0, 1.0, color.getOpacity() + rgbData[rgbDataOffset + 0]));
            }
            if (colorComponent == 5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(color.getColorsPtr() + 12, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, color.getColorsPtr() + 12);
            }
        }
        if (colorName[1] == '1')
        { // OLD, deprecated (inside color)
            if (colorComponent < 4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.getColorsPtr() + colorComponent * 3, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, insideColor_DEPRECATED.getColorsPtr() + colorComponent * 3);
            }
            if (colorComponent == 4)
            {
                insideColor_DEPRECATED.setOpacity(
                    tt::getLimitedFloat(0.0, 1.0, insideColor_DEPRECATED.getOpacity() + rgbData[rgbDataOffset + 0]));
            }
            if (colorComponent == 5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.getColorsPtr() + 12, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, insideColor_DEPRECATED.getColorsPtr() + 12);
            }
        }
        if (colorName[1] == '2')
        { // OLD, deprecated (edge color)
            if (colorComponent < 4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.getColorsPtr() + colorComponent * 3, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, edgeColor_DEPRECATED.getColorsPtr() + colorComponent * 3);
            }
            if (colorComponent == 5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.getColorsPtr() + 12, hsl);
                hsl[0] = fmod(hsl[0] + rgbData[rgbDataOffset + 0], 1.0);
                hsl[1] = tt::getLimitedFloat(0.0, 1.0, hsl[1] + rgbData[rgbDataOffset + 1]);
                hsl[2] = tt::getLimitedFloat(0.0, 1.0, hsl[2] + rgbData[rgbDataOffset + 2]);
                tt::hslToRgb(hsl, edgeColor_DEPRECATED.getColorsPtr() + 12);
            }
        }
    }
    elementIndex++;
}

bool CMesh::getColor(const char *colorName, int colorComponent, float *rgbData, int &rgbDataOffset) const
{ // function has virtual/non-virtual counterpart!
    if ((colorName == nullptr) || (color.getColorName().compare(colorName) == 0) ||
        (strcmp(colorName, "@compound") == 0))
    {
        bool compoundColors = (colorName != nullptr) && (strcmp(colorName, "@compound") == 0);
        if (colorComponent < 4)
        { // regular components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = color.getColorsPtr()[colorComponent * 3 + i];
            if (compoundColors)
                rgbDataOffset += 3;
            return (true);
        }
        if (colorComponent == 4)
        {
            rgbData[rgbDataOffset + 0] = color.getOpacity();
            if (compoundColors)
                rgbDataOffset += 1;
            return (true);
        }
        if (colorComponent == 5)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = color.getColorsPtr()[12 + i];
            if (compoundColors)
                rgbDataOffset += 3;
            return (true);
        }
        return (false);
    }
    if ((colorName != nullptr) && (insideColor_DEPRECATED.getColorName().compare(colorName) == 0))
    { // OLD, deprecated
        if (colorComponent < 4)
        { // regular components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = insideColor_DEPRECATED.getColorsPtr()[colorComponent * 3 + i];
            return (true);
        }
        if (colorComponent == 4)
        {
            rgbData[rgbDataOffset + 0] = insideColor_DEPRECATED.getOpacity();
            return (true);
        }
        if (colorComponent == 5)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = insideColor_DEPRECATED.getColorsPtr()[12 + i];
            return (true);
        }
        return (false);
    }
    if ((colorName != nullptr) && (edgeColor_DEPRECATED.getColorName().compare(colorName) == 0))
    { // OLD, deprecated
        if (colorComponent < 4)
        { // regular components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = edgeColor_DEPRECATED.getColorsPtr()[colorComponent * 3 + i];
        }
        if (colorComponent == 5)
        { // auxiliary components
            for (int i = 0; i < 3; i++)
                rgbData[rgbDataOffset + i] = edgeColor_DEPRECATED.getColorsPtr()[12 + i];
        }
        return (false);
    }
    return (false);
}

void CMesh::getAllMeshComponentsCumulative(const C7Vector &parentCumulTr, std::vector<CMesh *> &shapeComponentList,
                                           std::vector<C7Vector> *OptParentCumulTrList /*=nullptr*/)
{ // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    shapeComponentList.push_back(this);
    if (OptParentCumulTrList != nullptr)
        OptParentCumulTrList->push_back(parentCumulTr * _iFrame * _bbFrame);
}

CMesh *CMesh::getMeshComponentAtIndex(const C7Vector &parentCumulTr, int &index,
                                      C7Vector *optParentCumulTrOut /*=nullptr*/)
{ // function has virtual/non-virtual counterpart!
    CMesh *retVal = nullptr;
    if (index >= 0)
    {
        if (index == 0)
        {
            retVal = this;
            if (optParentCumulTrOut != nullptr)
                optParentCumulTrOut[0] = parentCumulTr;
        }
        else
            index--;
    }
    return (retVal);
}

int CMesh::getComponentCount() const
{ // function has virtual/non-virtual counterpart!
    return (1);
}

int CMesh::getUniqueID() const
{
    return (_uniqueID);
}

void CMesh::setHeightfieldData(const std::vector<double> &heights, int xCount, int yCount)
{
    _heightfieldHeights.clear();
    _heightfieldHeights.insert(_heightfieldHeights.end(), heights.begin(), heights.end());
    _heightfieldXCount = xCount;
    _heightfieldYCount = yCount;
}

double *CMesh::getHeightfieldData(int &xCount, int &yCount, double &minHeight, double &maxHeight)
{
    setHeightfieldDiamonds(App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr) == sim_physics_mujoco);
    if ((_purePrimitive != sim_primitiveshape_heightfield) || (_heightfieldHeights.size() == 0))
        return (nullptr);
    xCount = _heightfieldXCount;
    yCount = _heightfieldYCount;
    minHeight = +99999999.0;
    maxHeight = -99999999.0;
    for (int i = 0; i < int(_heightfieldHeights.size()); i++)
    {
        if (_heightfieldHeights[i] < minHeight)
            minHeight = _heightfieldHeights[i];
        if (_heightfieldHeights[i] > maxHeight)
            maxHeight = _heightfieldHeights[i];
    }
    return (&_heightfieldHeights[0]);
}

void CMesh::setHeightfieldDiamonds(bool d)
{
    if (_purePrimitive == sim_primitiveshape_heightfield)
    {
        for (size_t i = 0; i < _indices.size() / 6; i++)
        {
            if (d)
            {
                _indices[6 * i + 1] = _indices[6 * i + 4];
                _indices[6 * i + 5] = _indices[6 * i + 0];
            }
            else
            {
                _indices[6 * i + 1] = _indices[6 * i + 3];
                _indices[6 * i + 5] = _indices[6 * i + 2];
            }
        }
#ifdef SIM_WITH_GUI
        decreaseVertexBufferRefCnt(_vertexBufferId);
        decreaseNormalBufferRefCnt(_normalBufferId);
        decreaseEdgeBufferRefCnt(_edgeBufferId);

        _vertexBufferId = -1;
        _normalBufferId = -1;
        _edgeBufferId = -1;
#endif
    }
}

void CMesh::getPurePrimitiveSizes(C3Vector &s) const
{
    s(0) = _purePrimitiveXSizeOrDiameter;
    s(1) = _purePrimitiveYSize;
    s(2) = _purePrimitiveZSizeOrHeight;
}

void CMesh::setPurePrimitiveInsideScaling_OLD(double s)
{
    _purePrimitiveInsideScaling = tt::getLimitedFloat(0.0, 0.99999, s);
}

double CMesh::getPurePrimitiveInsideScaling_OLD() const
{
    return (_purePrimitiveInsideScaling);
}

CTextureProperty *CMesh::getTextureProperty()
{
    return (_textureProperty);
}

void CMesh::setTextureProperty(CTextureProperty *tp)
{ // careful, this doesn't check if a _textureProperty already exists! Has to be done and destroyed outside!
    _textureProperty = tp;
}

void CMesh::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    _insideAndOutsideFacesSameColor_DEPRECATED = s;
}

bool CMesh::getInsideAndOutsideFacesSameColor_DEPRECATED() const
{
    return (_insideAndOutsideFacesSameColor_DEPRECATED);
}

bool CMesh::getVisibleEdges() const
{
    return (_visibleEdges);
}

void CMesh::setVisibleEdges(bool v)
{
    _visibleEdges = v;
}

void CMesh::setHideEdgeBorders_OLD(bool v)
{
    if (_hideEdgeBorders_OLD != v)
    {
        _hideEdgeBorders_OLD = v;
        _computeVisibleEdges();
    }
}

bool CMesh::getHideEdgeBorders_OLD() const
{
    return (_hideEdgeBorders_OLD);
}

int CMesh::getEdgeWidth_DEPRECATED() const
{
    return (_edgeWidth_DEPRERCATED);
}

void CMesh::setEdgeWidth_DEPRECATED(int w)
{
    w = tt::getLimitedInt(1, 4, w);
    _edgeWidth_DEPRERCATED = w;
}

bool CMesh::getCulling() const
{
    return (_culling);
}

void CMesh::setCulling(bool c)
{
    _culling = c;
}

bool CMesh::getDisplayInverted_DEPRECATED() const
{
    return (_displayInverted_DEPRECATED);
}

void CMesh::setDisplayInverted_DEPRECATED(bool di)
{
    _displayInverted_DEPRECATED = di;
}

void CMesh::takeVisualAttributesFrom(CMesh *origin)
{
    origin->color.copyYourselfInto(&color);
    origin->insideColor_DEPRECATED.copyYourselfInto(&insideColor_DEPRECATED);
    origin->edgeColor_DEPRECATED.copyYourselfInto(&edgeColor_DEPRECATED);
    _visibleEdges = origin->_visibleEdges;
    _hideEdgeBorders_OLD = origin->_hideEdgeBorders_OLD;
    _culling = origin->_culling;
    _displayInverted_DEPRECATED = origin->_displayInverted_DEPRECATED;
    _insideAndOutsideFacesSameColor_DEPRECATED = origin->_insideAndOutsideFacesSameColor_DEPRECATED;
    _wireframe_OLD = origin->_wireframe_OLD;
    _edgeWidth_DEPRERCATED = origin->_edgeWidth_DEPRERCATED;
    if ((origin->_textureProperty != nullptr) && (_textureProperty != nullptr))
    {
        _textureProperty->setInterpolateColors(origin->_textureProperty->getInterpolateColors());
        _textureProperty->setApplyMode(origin->_textureProperty->getApplyMode());
        _textureProperty->setTextureObjectID(origin->_textureProperty->getTextureObjectID());
    }
    if (_shadingAngle != origin->_shadingAngle)
    {
        _shadingAngle = origin->_shadingAngle;
        _recomputeNormals();
    }
    if (_edgeThresholdAngle != origin->_edgeThresholdAngle)
    {
        _edgeThresholdAngle = origin->_edgeThresholdAngle;
        _computeVisibleEdges();
    }
}

void CMesh::copyVisualAttributesTo(CMeshWrapper *target)
{
    target->takeVisualAttributesFrom(this);
}

double CMesh::getShadingAngle() const
{ // function has virtual/non-virtual counterpart!
    return (_shadingAngle);
}

void CMesh::setShadingAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0, 89.0 * degToRad, angle);
    if (_shadingAngle != angle)
    {
        _shadingAngle = angle;
        _recomputeNormals();
        _edgeThresholdAngle = angle;
        _computeVisibleEdges();
    }
}

double CMesh::getEdgeThresholdAngle() const
{ // function has virtual/non-virtual counterpart!
    return (_edgeThresholdAngle);
}

void CMesh::setEdgeThresholdAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0, 89.0 * degToRad, angle);
    if (_edgeThresholdAngle != angle)
    {
        _edgeThresholdAngle = angle;
        _computeVisibleEdges();
    }
}

void CMesh::setWireframe_OLD(bool w)
{
    _wireframe_OLD = w;
}

bool CMesh::getWireframe_OLD() const
{
    return (_wireframe_OLD);
}

std::vector<double> *CMesh::getVertices()
{
    return (&_vertices);
}

std::vector<float> *CMesh::getVerticesForDisplayAndDisk()
{
    return (&_verticesForDisplayAndDisk);
}

std::vector<int> *CMesh::getIndices()
{
    return (&_indices);
}

std::vector<double> *CMesh::getNormals()
{
    return (&_normals);
}

std::vector<float> *CMesh::getNormalsForDisplayAndDisk()
{
    return (&_normalsForDisplayAndDisk);
}

std::vector<unsigned char> *CMesh::getEdges()
{
    return (&_edges);
}

void CMesh::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    delete _textureProperty;
    _textureProperty = nullptr;
}

void CMesh::getColorStrings(std::string &colorStrings, bool onlyNamed) const
{ // function has virtual/non-virtual counterpart!
    if (!onlyNamed)
    {
        if (colorStrings.length() != 0)
            colorStrings += " ";
        if (color.getColorName().length() > 0)
            colorStrings += color.getColorName();
        else
            colorStrings += "*";
    }
    else
    {
        if ((color.getColorName().length() > 0) && (colorStrings.find(color.getColorName()) == std::string::npos))
        {
            if (colorStrings.length() != 0)
                colorStrings += " ";
            colorStrings += color.getColorName();
        }
        if ((insideColor_DEPRECATED.getColorName().length() > 0) &&
            (colorStrings.find(insideColor_DEPRECATED.getColorName()) == std::string::npos))
        {
            if (colorStrings.length() != 0)
                colorStrings += " ";
            colorStrings += insideColor_DEPRECATED.getColorName();
        }
        if ((edgeColor_DEPRECATED.getColorName().length() > 0) &&
            (colorStrings.find(edgeColor_DEPRECATED.getColorName()) == std::string::npos))
        {
            if (colorStrings.length() != 0)
                colorStrings += " ";
            colorStrings += edgeColor_DEPRECATED.getColorName();
        }
    }
}

void CMesh::flipFaces()
{ // function has virtual/non-virtual counterpart!
    int save;
    double normSave;
    for (size_t i = 0; i < _indices.size() / 3; i++)
    {
        save = _indices[3 * i + 0];
        _indices[3 * i + 0] = _indices[3 * i + 2];
        _indices[3 * i + 2] = save;

        normSave = -_normals[3 * (3 * i + 0) + 0];
        _normals[3 * (3 * i + 0) + 0] = -_normals[3 * (3 * i + 2) + 0];
        _normals[3 * (3 * i + 1) + 0] *= -1.0;
        _normals[3 * (3 * i + 2) + 0] = normSave;

        normSave = -_normals[3 * (3 * i + 0) + 1];
        _normals[3 * (3 * i + 0) + 1] = -_normals[3 * (3 * i + 2) + 1];
        _normals[3 * (3 * i + 1) + 1] *= -1.0;
        _normals[3 * (3 * i + 2) + 1] = normSave;

        normSave = -_normals[3 * (3 * i + 0) + 2];
        _normals[3 * (3 * i + 0) + 2] = -_normals[3 * (3 * i + 2) + 2];
        _normals[3 * (3 * i + 1) + 2] *= -1.0;
        _normals[3 * (3 * i + 2) + 2] = normSave;
    }
    _computeVisibleEdges();
    checkIfConvex();

#ifdef SIM_WITH_GUI
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);

    _normalBufferId = -1;
    _vertexBufferId = -1;
#endif

    _normalsForDisplayAndDisk.resize(_normals.size());
    for (size_t i = 0; i < _normals.size(); i++)
        _normalsForDisplayAndDisk[i] = (float)_normals[i];
}

void CMesh::actualizeGouraudShadingAndVisibleEdges()
{
    _recomputeNormals();
    _computeVisibleEdges();
}

void CMesh::_recomputeNormals()
{
    _normals.resize(3 * _indices.size());
    double maxAngle = _shadingAngle;
    C3Vector v[3];
    for (size_t i = 0; i < _indices.size() / 3; i++)
    { // Here we restore first all the normal vectors
        v[0] = C3Vector(&_vertices[3 * (_indices[3 * i + 0])]);
        v[1] = C3Vector(&_vertices[3 * (_indices[3 * i + 1])]);
        v[2] = C3Vector(&_vertices[3 * (_indices[3 * i + 2])]);

        C3Vector v1(v[1] - v[0]);
        C3Vector v2(v[2] - v[0]);
        C3Vector n((v1 ^ v2).getNormalized());

        _normals[9 * i + 0] = n(0);
        _normals[9 * i + 1] = n(1);
        _normals[9 * i + 2] = n(2);
        _normals[9 * i + 3] = n(0);
        _normals[9 * i + 4] = n(1);
        _normals[9 * i + 5] = n(2);
        _normals[9 * i + 6] = n(0);
        _normals[9 * i + 7] = n(1);
        _normals[9 * i + 8] = n(2);
    }

    std::vector<std::vector<int> *> indexToNormals;
    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        std::vector<int> *sharingNormals = new std::vector<int>;
        indexToNormals.push_back(sharingNormals);
    }
    for (size_t i = 0; i < _indices.size() / 3; i++)
    {
        indexToNormals[_indices[3 * i + 0]]->push_back(int(3 * i + 0));
        indexToNormals[_indices[3 * i + 1]]->push_back(int(3 * i + 1));
        indexToNormals[_indices[3 * i + 2]]->push_back(int(3 * i + 2));
    }
    std::vector<double> changedNorm(_normals.size());

    for (size_t i = 0; i < indexToNormals.size(); i++)
    {
        for (size_t j = 0; j < indexToNormals[i]->size(); j++)
        {
            C3Vector totN;
            double nb = 1.0;
            C3Vector nActual;
            nActual.setData(&_normals[3 * (indexToNormals[i]->at(j))]);
            totN = nActual;
            for (size_t k = 0; k < indexToNormals[i]->size(); k++)
            {
                if (j != k)
                {
                    C3Vector nToCompare(&_normals[3 * (indexToNormals[i]->at(k))]);
                    if (nActual.getAngle(nToCompare) < maxAngle)
                    {
                        totN += nToCompare;
                        nb = nb + 1.0;
                    }
                }
            }
            changedNorm[3 * indexToNormals[i]->at(j) + 0] = (totN(0) / nb);
            changedNorm[3 * indexToNormals[i]->at(j) + 1] = (totN(1) / nb);
            changedNorm[3 * indexToNormals[i]->at(j) + 2] = (totN(2) / nb);
        }
        indexToNormals[i]->clear();
        delete indexToNormals[i];
    }
    // Now we have to replace the modified normals:
    for (size_t i = 0; i < _indices.size() / 3; i++)
    {
        for (int j = 0; j < 9; j++)
            _normals[9 * i + j] = changedNorm[9 * i + j];
    }

#ifdef SIM_WITH_GUI
    decreaseNormalBufferRefCnt(_normalBufferId);
    _normalBufferId = -1;
#endif

    _normalsForDisplayAndDisk.resize(_normals.size());
    for (size_t i = 0; i < _normals.size(); i++)
        _normalsForDisplayAndDisk[i] = (float)_normals[i];
}

C3Vector CMesh::_computeBBSize(C3Vector *optBBCenter /*=nullptr*/)
{
    C3Vector retBBSize;
    C3Vector mmin(DBL_MAX, DBL_MAX, DBL_MAX);
    C3Vector mmax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
    for (size_t i = 0; i < _vertices.size() / 3; i++)
    {
        C3Vector v(_vertices.data() + 3 * i);
        mmin.keepMin(v);
        mmax.keepMax(v);
    }
    retBBSize = mmax - mmin;
    if (optBBCenter != nullptr)
        optBBCenter[0] = (mmax + mmin) * 0.5;
    return (retBBSize);
}

void CMesh::_computeVisibleEdges()
{
    if (_indices.size() == 0)
        return;
    double softAngle = _edgeThresholdAngle;
    _edges.clear();
    std::vector<int> eIDs;
    CMeshRoutines::getEdgeFeatures(&_vertices[0], (int)_vertices.size(), &_indices[0], (int)_indices.size(), nullptr,
                                   &eIDs, nullptr, softAngle, true, _hideEdgeBorders_OLD);
    _edges.assign((_indices.size() / 8) + 1, 0);
    std::vector<bool> usedEdges(_indices.size(), false);
    for (int i = 0; i < int(eIDs.size()); i++)
    {
        if (eIDs[i] != -1)
        {
            _edges[i >> 3] |= (1 << (i & 7));
            usedEdges[eIDs[i]] = true;
        }
    }
#ifdef SIM_WITH_GUI
    _edgeBufferId = -1;
#endif
}

bool CMesh::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex = (CMeshRoutines::getConvexType(_vertices, _indices, 0.015) ==
               0); // 1.5% tolerance of the average bounding box side length
    return (_convex);
}

void CMesh::_loadPackedIntegers_OLD(CSer &ar, std::vector<int> &data)
{
    int dataLength;
    ar >> dataLength;
    data.clear();
    unsigned char b0, b1, b2, b3, storageB;
    int prevInd = 0;
    for (int i = 0; i < dataLength; i++)
    {
        b1 = 0;
        b2 = 0;
        b3 = 0;
        ar >> b0;
        storageB = ((b0 & 0xc0) >> 6);
        b0 &= 0x3f; // we remove the storage byte info
        if (storageB >= 1)
            ar >> b1; // this index takes 2 or more storage bytes
        if (storageB >= 2)
            ar >> b2; // this index takes 3 or more storage bytes
        if (storageB >= 3)
            ar >> b3; // this index takes 4 storage bytes
        int diff = b0 + (b1 << 6) + (b2 << 14) + (b3 << 22);
        if (storageB == 0)
            diff -= 31;
        if (storageB == 1)
            diff -= 8191;
        if (storageB == 2)
            diff -= 2097151;
        if (storageB == 3)
            diff -= 536870911;
        int currInd = prevInd + diff;
        data.push_back(currInd);
        prevInd = currInd;
    }
}

void CMesh::clearTempVerticesIndicesNormalsAndEdges()
{
    for (size_t i = 0; i < _tempVerticesForDisk.size(); i++)
        delete _tempVerticesForDisk[i];
    _tempVerticesForDisk.clear();

    for (size_t i = 0; i < _tempIndicesForDisk.size(); i++)
        delete _tempIndicesForDisk[i];
    _tempIndicesForDisk.clear();

    for (size_t i = 0; i < _tempNormalsForDisk.size(); i++)
        delete _tempNormalsForDisk[i];
    _tempNormalsForDisk.clear();

    for (size_t i = 0; i < _tempEdgesForDisk.size(); i++)
        delete _tempEdgesForDisk[i];
    _tempEdgesForDisk.clear();
}

void CMesh::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    _tempVerticesIndexForSerialization = getBufferIndexOfVertices(_verticesForDisplayAndDisk);
    if (_tempVerticesIndexForSerialization == -1)
        _tempVerticesIndexForSerialization = addVerticesToBufferAndReturnIndex(_verticesForDisplayAndDisk);

    _tempIndicesIndexForSerialization = getBufferIndexOfIndices(_indices);
    if (_tempIndicesIndexForSerialization == -1)
        _tempIndicesIndexForSerialization = addIndicesToBufferAndReturnIndex(_indices);

    _tempNormalsIndexForSerialization = getBufferIndexOfNormals(_normalsForDisplayAndDisk);
    if (_tempNormalsIndexForSerialization == -1)
        _tempNormalsIndexForSerialization = addNormalsToBufferAndReturnIndex(_normalsForDisplayAndDisk);

    _tempEdgesIndexForSerialization = getBufferIndexOfEdges(_edges);
    if (_tempEdgesIndexForSerialization == -1)
        _tempEdgesIndexForSerialization = addEdgesToBufferAndReturnIndex(_edges);
}

void CMesh::serializeTempVerticesIndicesNormalsAndEdges(CSer &ar)
{
    if (ar.isStoring())
    { // Storing
        for (size_t c = 0; c < _tempVerticesForDisk.size(); c++)
        {
            ar.storeDataName("Ver");
            if (_tempVerticesForDisk[c]->size() != 0)
            {
                std::vector<unsigned char> *serBuffer = ar.getBufferPointer();
                unsigned char *ptr = reinterpret_cast<unsigned char *>(&_tempVerticesForDisk[c]->at(0));
                serBuffer->insert(serBuffer->end(), ptr, ptr + _tempVerticesForDisk[c]->size() * sizeof(float));
            }
            ar.flush();
        }

        for (size_t c = 0; c < _tempIndicesForDisk.size(); c++)
        {
            ar.storeDataName("Ind");
            for (size_t i = 0; i < _tempIndicesForDisk[c]->size(); i++)
                ar << _tempIndicesForDisk[c]->at(i);
            ar.flush();
        }

        for (size_t c = 0; c < _tempNormalsForDisk.size(); c++)
        {
            ar.storeDataName("Nor");
            for (size_t i = 0; i < _tempNormalsForDisk[c]->size(); i++)
                ar << (float)_tempNormalsForDisk[c]->at(i);
            ar.flush();
        }

        for (size_t c = 0; c < _tempEdgesForDisk.size(); c++)
        {
            ar.storeDataName("Ved"); // Recomputing edges at load takes too much time
            if (_tempEdgesForDisk[c]->size() != 0)
            {
                std::vector<unsigned char> *serBuffer = ar.getBufferPointer();
                unsigned char *ptr = &_tempEdgesForDisk[c]->at(0);
                serBuffer->insert(serBuffer->end(), ptr, ptr + _tempEdgesForDisk[c]->size());
            }
            ar.flush();
        }

        ar.storeDataName(SER_END_OF_OBJECT);
    }
    else
    { // loading
        int byteQuantity;
        std::string theName = "";
        while (theName.compare(SER_END_OF_OBJECT) != 0)
        {
            theName = ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                bool noHit = true;
                if (theName.compare("Ver") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<float> *arr = new std::vector<float>;
                    _tempVerticesForDisk.push_back(arr);
                    arr->resize(byteQuantity / sizeof(float), 0.0);
                    for (size_t i = 0; i < arr->size(); i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("Ind") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<int> *arr = new std::vector<int>;
                    _tempIndicesForDisk.push_back(arr);
                    arr->resize(byteQuantity / sizeof(int), 0);
                    for (size_t i = 0; i < arr->size(); i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("In2") == 0)
                { // deprecated in 2023
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<int> *arr = new std::vector<int>;
                    _tempIndicesForDisk.push_back(arr);
                    _loadPackedIntegers_OLD(ar, *arr);
                }
                if (theName.compare("Nor") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<float> *arr = new std::vector<float>;
                    _tempNormalsForDisk.push_back(arr);
                    arr->resize(byteQuantity / sizeof(float), 0.0);
                    for (size_t i = 0; i < arr->size(); i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("No2") == 0)
                { // approx. normals. For backward compatibility
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<float> *arr = new std::vector<float>;
                    _tempNormalsForDisk.push_back(arr);
                    arr->resize(byteQuantity * 6 / sizeof(float), 0.0);
                    for (int i = 0; i < byteQuantity / 2; i++)
                    {
                        unsigned short w;
                        ar >> w;
                        char x = (w & 0x001f) - 15;
                        char y = ((w >> 5) & 0x001f) - 15;
                        char z = ((w >> 10) & 0x001f) - 15;
                        C3Vector n((double)x, (double)y, (double)z);
                        n.normalize();
                        arr->at(3 * i + 0) = (float)n(0);
                        arr->at(3 * i + 1) = (float)n(1);
                        arr->at(3 * i + 2) = (float)n(2);
                    }
                }
                if (theName.compare("Ved") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    std::vector<unsigned char> *arr = new std::vector<unsigned char>;
                    _tempEdgesForDisk.push_back(arr);
                    arr->resize(byteQuantity, 0);
                    for (int i = 0; i < byteQuantity; i++)
                        ar >> arr->at(i);
                }
            }
        }
    }
}

int CMesh::getBufferIndexOfVertices(const std::vector<float> &vert)
{
    int vertl = (int)vert.size();
    for (int i = 0; i < int(_tempVerticesForDisk.size()); i++)
    {
        if (int(_tempVerticesForDisk[i]->size()) == vertl)
        { // might be same!
            bool same = true;
            for (int j = 0; j < vertl; j++)
            {
                if (vert[j] != _tempVerticesForDisk[i]->at(j))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                return (i);
        }
    }

    return (-1); // not found
}

int CMesh::addVerticesToBufferAndReturnIndex(const std::vector<float> &vert)
{
    std::vector<float> *nvert = new std::vector<float>;
    nvert->assign(vert.begin(), vert.end());
    _tempVerticesForDisk.push_back(nvert);
    return ((int)_tempVerticesForDisk.size() - 1);
}

void CMesh::getVerticesFromBufferBasedOnIndex(int index, std::vector<float> &vert)
{
    vert.assign(_tempVerticesForDisk[index]->begin(), _tempVerticesForDisk[index]->end());
}

int CMesh::getBufferIndexOfIndices(const std::vector<int> &ind)
{
    int indl = (int)ind.size();
    for (int i = 0; i < int(_tempIndicesForDisk.size()); i++)
    {
        if (int(_tempIndicesForDisk[i]->size()) == indl)
        { // might be same!
            bool same = true;
            for (int j = 0; j < indl; j++)
            {
                if (ind[j] != _tempIndicesForDisk[i]->at(j))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                return (i);
        }
    }

    return (-1); // not found
}

int CMesh::addIndicesToBufferAndReturnIndex(const std::vector<int> &ind)
{
    std::vector<int> *nind = new std::vector<int>;
    nind->assign(ind.begin(), ind.end());
    _tempIndicesForDisk.push_back(nind);
    return ((int)_tempIndicesForDisk.size() - 1);
}

void CMesh::getIndicesFromBufferBasedOnIndex(int index, std::vector<int> &ind)
{
    ind.assign(_tempIndicesForDisk[index]->begin(), _tempIndicesForDisk[index]->end());
}

int CMesh::getBufferIndexOfNormals(const std::vector<float> &norm)
{
    int norml = (int)norm.size();
    for (int i = 0; i < int(_tempNormalsForDisk.size()); i++)
    {
        if (int(_tempNormalsForDisk[i]->size()) == norml)
        { // might be same!
            bool same = true;
            for (int j = 0; j < norml; j++)
            {
                if (norm[j] != _tempNormalsForDisk[i]->at(j))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                return (i);
        }
    }

    return (-1); // not found
}

int CMesh::addNormalsToBufferAndReturnIndex(const std::vector<float> &norm)
{
    std::vector<float> *nnorm = new std::vector<float>;
    nnorm->assign(norm.begin(), norm.end());
    _tempNormalsForDisk.push_back(nnorm);
    return ((int)_tempNormalsForDisk.size() - 1);
}

void CMesh::getNormalsFromBufferBasedOnIndex(int index, std::vector<float> &norm)
{
    norm.assign(_tempNormalsForDisk[index]->begin(), _tempNormalsForDisk[index]->end());
}

int CMesh::getBufferIndexOfEdges(const std::vector<unsigned char> &edges)
{
    int edgesl = (int)edges.size();
    for (int i = 0; i < int(_tempEdgesForDisk.size()); i++)
    {
        if (int(_tempEdgesForDisk[i]->size()) == edgesl)
        { // might be same!
            bool same = true;
            for (int j = 0; j < edgesl; j++)
            {
                if (edges[j] != _tempEdgesForDisk[i]->at(j))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                return (i);
        }
    }

    return (-1); // not found
}

int CMesh::addEdgesToBufferAndReturnIndex(const std::vector<unsigned char> &edges)
{
    std::vector<unsigned char> *nedges = new std::vector<unsigned char>;
    nedges->assign(edges.begin(), edges.end());
    _tempEdgesForDisk.push_back(nedges);
    return ((int)_tempEdgesForDisk.size() - 1);
}

void CMesh::getEdgesFromBufferBasedOnIndex(int index, std::vector<unsigned char> &edges)
{
    edges.assign(_tempEdgesForDisk[index]->begin(), _tempEdgesForDisk[index]->end());
}

bool CMesh::serialize(CSer &ar, const char *shapeName, const C7Vector &parentCumulIFrame, bool rootLevel)
{ // function has virtual/non-virtual counterpart!
    bool hasNewBBFrameAndSize = CMeshWrapper::serialize(ar, shapeName, parentCumulIFrame, rootLevel);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Cl0");
            ar.setCountingMode();
            color.serialize(ar, 0);
            if (ar.setWritingMode())
                color.serialize(ar, 0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            insideColor_DEPRECATED.serialize(ar, 0);
            if (ar.setWritingMode())
                insideColor_DEPRECATED.serialize(ar, 0);

            ar.storeDataName("Ecl");
            ar.setCountingMode();
            edgeColor_DEPRECATED.serialize(ar, 1);
            if (ar.setWritingMode())
                edgeColor_DEPRECATED.serialize(ar, 1);

            if (App::currentWorld->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
            { // undo/redo serialization:
                ar.storeDataName("Ver");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addVertexBuffer(
                    _verticesForDisplayAndDisk, App::currentWorld->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Ind");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addIndexBuffer(
                    _indices, App::currentWorld->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Nor");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addNormalsBuffer(
                    _normalsForDisplayAndDisk, App::currentWorld->undoBufferContainer->getNextBufferId());
                ar.flush();
            }
            else
            { // normal serialization:
                ar.storeDataName("Vev");
                ar << _tempVerticesIndexForSerialization;
                ar.flush();

                ar.storeDataName("Inv");
                ar << _tempIndicesIndexForSerialization;
                ar.flush();

                ar.storeDataName("Nov");
                ar << _tempNormalsIndexForSerialization;
                ar.flush();
            }

            ar.storeDataName("Vvd");
            ar << _tempEdgesIndexForSerialization;
            ar.flush();

            ar.storeDataName("_pr");
            ar << _purePrimitive;
            ar << _purePrimitiveXSizeOrDiameter << _purePrimitiveYSize << _purePrimitiveZSizeOrHeight;
            ar.flush();

            ar.storeDataName("_p2");
            ar << _purePrimitiveInsideScaling;
            ar.flush();

            ar.storeDataName("_pf"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame * _iFrame * _bbFrame);
            ar << w(0) << w(1) << w(2) << w(3);
            ar << w(4) << w(5) << w(6);
            ar.flush();

            ar.storeDataName("_sa"); // write this always before Gs2
            ar << _shadingAngle;
            ar << _edgeWidth_DEPRERCATED;
            ar.flush();

            ar.storeDataName("_s2");
            ar << _edgeThresholdAngle;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _visibleEdges);
            SIM_SET_CLEAR_BIT(nothing, 1, _culling);
            SIM_SET_CLEAR_BIT(nothing, 2, _wireframe_OLD);
            SIM_SET_CLEAR_BIT(nothing, 3, _insideAndOutsideFacesSameColor_DEPRECATED);
            // RESERVED... DO NOT USE  // SIM_SET_CLEAR_BIT(nothing,4,true); // means: we do not have to make the
            // convectivity test for this shape (was already done). Added this on 16/1/2013
            SIM_SET_CLEAR_BIT(nothing, 5, true); // means: we do not have to make the convectivity test for this shape
                                                 // (was already done). Added this on 28/1/2013
            SIM_SET_CLEAR_BIT(nothing, 6, _displayInverted_DEPRECATED);
            SIM_SET_CLEAR_BIT(nothing, 7, _hideEdgeBorders_OLD);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Va2"); // Since V4.5
            nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _convex);
            ar << nothing;
            ar.flush();

            if (_textureProperty != nullptr)
            {
                ar.storeDataName("Toj");
                ar.setCountingMode();
                _textureProperty->serialize(ar);
                if (ar.setWritingMode())
                    _textureProperty->serialize(ar);
            }

            ar.storeDataName("_fd"); // Has to come after PURE TYPE!
            ar << _heightfieldXCount << _heightfieldYCount;
            for (size_t i = 0; i < _heightfieldHeights.size(); i++)
                ar << _heightfieldHeights[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool fixFrame_prior4_5_2 = false;
            bool hasConvexFlag = false;
            C7Vector verticesLocalFrame_old; // prev. _verticesLocalframe
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Cl0") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        color.serialize(ar, 0);
                    }
                    if (theName.compare("Cl2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        insideColor_DEPRECATED.serialize(ar, 0);
                    }
                    if (theName.compare("Ecl") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        edgeColor_DEPRECATED.serialize(ar, 1);
                    }
                    if (App::currentWorld->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
                    { // undo/redo serialization
                        if (theName.compare("Ver") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getVertexBuffer(
                                id, _verticesForDisplayAndDisk);
                            _vertices.resize(_verticesForDisplayAndDisk.size());
                            for (size_t i = 0; i < _verticesForDisplayAndDisk.size(); i++)
                                _vertices[i] = (double)_verticesForDisplayAndDisk[i];
                        }
                        if (theName.compare("Ind") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getIndexBuffer(id, _indices);
                        }
                        if (theName.compare("Nor") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getNormalsBuffer(
                                id, _normalsForDisplayAndDisk);
                            _normals.resize(_normalsForDisplayAndDisk.size());
                            for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
                                _normals[i] = (double)_normalsForDisplayAndDisk[i];
                        }
                    }
                    else
                    { // normal serialization
                        if (theName.compare("Ver") == 0)
                        { // for backward compatibility (1/7/2014)
                            noHit = false;
                            ar >> byteQuantity;
                            _verticesForDisplayAndDisk.resize(byteQuantity / sizeof(float), 0.0);
                            for (size_t i = 0; i < _verticesForDisplayAndDisk.size(); i++)
                                ar >> _verticesForDisplayAndDisk[i];
                            _vertices.resize(_verticesForDisplayAndDisk.size());
                            for (size_t i = 0; i < _verticesForDisplayAndDisk.size(); i++)
                                _vertices[i] = (double)_verticesForDisplayAndDisk[i];
                        }
                        if (theName.compare("Ind") == 0)
                        { // for backward compatibility (1/7/2014)
                            noHit = false;
                            ar >> byteQuantity;
                            _indices.resize(byteQuantity / sizeof(int), 0);
                            for (size_t i = 0; i < _indices.size(); i++)
                                ar >> _indices[i];
                        }
                        if (theName.compare("Nor") == 0)
                        { // for backward compatibility (1/7/2014)
                            noHit = false;
                            ar >> byteQuantity;
                            _normalsForDisplayAndDisk.resize(byteQuantity / sizeof(float), 0.0);
                            for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
                                ar >> _normalsForDisplayAndDisk[i];
                            _normals.resize(_normalsForDisplayAndDisk.size());
                            for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
                                _normals[i] = (double)_normalsForDisplayAndDisk[i];
                        }

                        if (theName.compare("Vev") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getVerticesFromBufferBasedOnIndex(index, _verticesForDisplayAndDisk);
                            _vertices.resize(_verticesForDisplayAndDisk.size());
                            for (size_t i = 0; i < _verticesForDisplayAndDisk.size(); i++)
                                _vertices[i] = (double)_verticesForDisplayAndDisk[i];
                        }
                        if (theName.compare("Inv") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getIndicesFromBufferBasedOnIndex(index, _indices);
                        }
                        if (theName.compare("Nov") == 0)
                        {
                            noHit = false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getNormalsFromBufferBasedOnIndex(index, _normalsForDisplayAndDisk);
                            _normals.resize(_normalsForDisplayAndDisk.size());
                            for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
                                _normals[i] = (double)_normalsForDisplayAndDisk[i];
                        }
                    }

                    if (theName.compare("In2") == 0)
                    { // for backward compatibility (1/7/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        _loadPackedIntegers_OLD(ar, _indices);
                    }
                    if (theName.compare("No2") == 0)
                    { // for backward compatibility (1/7/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        _normalsForDisplayAndDisk.resize(byteQuantity * 6 / sizeof(float), 0.0);
                        for (int i = 0; i < byteQuantity / 2; i++)
                        {
                            unsigned short w;
                            ar >> w;
                            char x = (w & 0x001f) - 15;
                            char y = ((w >> 5) & 0x001f) - 15;
                            char z = ((w >> 10) & 0x001f) - 15;
                            C3Vector n((double)x, (double)y, (double)z);
                            n.normalize();
                            _normalsForDisplayAndDisk[3 * i + 0] = (float)n(0);
                            _normalsForDisplayAndDisk[3 * i + 1] = (float)n(1);
                            _normalsForDisplayAndDisk[3 * i + 2] = (float)n(2);
                        }
                        _normals.resize(_normalsForDisplayAndDisk.size());
                        for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
                            _normals[i] = (double)_normalsForDisplayAndDisk[i];
                    }
                    if (theName.compare("Ved") == 0)
                    { // for backward compatibility (1/7/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        _edges.resize(byteQuantity, 0);
                        for (int i = 0; i < byteQuantity; i++)
                            ar >> _edges[i];
                    }
                    if (theName.compare("Vvd") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int index;
                        ar >> index;
                        getEdgesFromBufferBasedOnIndex(index, _edges);
                    }
                    if (theName.compare("Ppr") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _purePrimitive;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        _purePrimitiveXSizeOrDiameter = (double)bla;
                        _purePrimitiveYSize = (double)bli;
                        _purePrimitiveZSizeOrHeight = (double)blo;
                    }

                    if (theName.compare("_pr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _purePrimitive;
                        ar >> _purePrimitiveXSizeOrDiameter >> _purePrimitiveYSize >> _purePrimitiveZSizeOrHeight;
                    }

                    if (theName.compare("Pp2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _purePrimitiveInsideScaling = (double)bla;
                    }

                    if (theName.compare("_p2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _purePrimitiveInsideScaling;
                    }

                    if (theName.compare("Ppf") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {                                // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        for (size_t i = 0; i < 7; i++)
                        {
                            ar >> bla;
                            verticesLocalFrame_old(i) = (double)bla;
                        }
                        verticesLocalFrame_old.Q
                            .normalize(); // we read from float. Make sure we are perfectly normalized!
                        if (!hasNewBBFrameAndSize)
                            fixFrame_prior4_5_2 = true;
                    }

                    if (theName.compare("_pf") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> verticesLocalFrame_old(0) >> verticesLocalFrame_old(1) >> verticesLocalFrame_old(2) >>
                            verticesLocalFrame_old(3);
                        ar >> verticesLocalFrame_old(4) >> verticesLocalFrame_old(5) >> verticesLocalFrame_old(6);
                        verticesLocalFrame_old.Q.normalize();
                        if (!hasNewBBFrameAndSize)
                            fixFrame_prior4_5_2 = true;
                    }

                    if (theName.compare("Gsa") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        ar >> _edgeWidth_DEPRERCATED;
                        _shadingAngle = (double)bla;
                        _edgeThresholdAngle = _shadingAngle;
                    }

                    if (theName.compare("_sa") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _shadingAngle;
                        ar >> _edgeWidth_DEPRERCATED;
                        _edgeThresholdAngle = _shadingAngle;
                    }

                    if (theName.compare("Gs2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _edgeThresholdAngle = (double)bla;
                    }

                    if (theName.compare("_s2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _edgeThresholdAngle;
                    }

                    if (theName == "Var")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _visibleEdges = SIM_IS_BIT_SET(nothing, 0);
                        _culling = SIM_IS_BIT_SET(nothing, 1);
                        _wireframe_OLD = SIM_IS_BIT_SET(nothing, 2);
                        _insideAndOutsideFacesSameColor_DEPRECATED = SIM_IS_BIT_SET(nothing, 3);
                        // reserved   doTheConvectivityTest=!SIM_IS_BIT_SET(nothing,4); // version 3.0.1 was buggy
                        // reserved doTheConvectivityTest=!SIM_IS_BIT_SET(nothing,5); // since version 3.0.2
                        // (version 3.0.1 was buggy)
                        _displayInverted_DEPRECATED = SIM_IS_BIT_SET(nothing, 6);
                        _hideEdgeBorders_OLD = SIM_IS_BIT_SET(nothing, 7);
                        // if (CSimFlavor::getBoolVal(18))
                        //    _visibleEdges=false;
                    }
                    if (theName == "Va2")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _convex = SIM_IS_BIT_SET(nothing, 0);
                        hasConvexFlag = true;
                    }
                    if (theName.compare("Toj") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _textureProperty = new CTextureProperty();
                        _textureProperty->serialize(ar);
                    }
                    if (theName.compare("Hfd") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _heightfieldXCount >> _heightfieldYCount;
                        for (int i = 0; i < _heightfieldXCount * _heightfieldYCount; i++)
                        {
                            float dummy;
                            ar >> dummy;
                            _heightfieldHeights.push_back((double)dummy);
                        }
                    }

                    if (theName.compare("_fd") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _heightfieldXCount >> _heightfieldYCount;
                        _heightfieldHeights.clear(); // could be non-empty, if "Hfd" tag is present
                        for (int i = 0; i < _heightfieldXCount * _heightfieldYCount; i++)
                        {
                            double dummy;
                            ar >> dummy;
                            _heightfieldHeights.push_back(dummy);
                        }
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (fixFrame_prior4_5_2)
            { // to accomodate new shapes (V4.5, rev2+)
                _transformMesh((parentCumulIFrame * _iFrame).getInverse() * verticesLocalFrame_old);
                _bbSize = _computeBBSize(&_bbFrame.X);
                _bbFrame.setIdentity();
                _transformMesh(_bbFrame.getInverse());
                _bbFrame.X.clear();
            }
            if (!hasConvexFlag)
            {
                checkIfConvex();
                if ((!_convex) && _convex_OLD)
                { // in some rare cases, old shapes won't be detected as convex anymore. Make them convex in that case!
                    if (CMeshRoutines::getConvexHull(_vertices, _vertices, _indices))
                    {
                        actualizeGouraudShadingAndVisibleEdges();
                        _updateDisplayAndDiskValues();
                        checkIfConvex();
                    }
                }
            }
            /* Needed to fix models and scenes just after release V4.5.1, rev0
            if ( _convex && (CMeshRoutines::getConvexType(_vertices,_indices,0.015)!=0) )
            {
                static int cnt=0;
                static int fcnt=0;
                if (CMeshRoutines::getConvexHull(_vertices,_vertices,_indices))
                {
                    actualizeGouraudShadingAndVisibleEdges();
                    _updateDisplayAndDiskValues();
                    checkIfConvex();
                    if (_convex)
                        fcnt++;
                }
                printf("bad,fixed: %i,%i\n",++cnt,fcnt);
            }
            //*/
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("shadingAngle", _shadingAngle * 180.0 / piValue);
            ar.xmlAddNode_float("edgeThresholdAngle", _edgeThresholdAngle * 180.0 / piValue);

            ar.xmlPushNewNode("color");
            color.serialize(ar, 0);
            ar.xmlPopNode();

            ar.xmlPushNewNode("primitive");
            ar.xmlAddNode_enum("type", _purePrimitive, sim_primitiveshape_none, "none", sim_primitiveshape_plane,
                               "plane", sim_primitiveshape_disc, "disc", sim_primitiveshape_cuboid, "cuboid",
                               sim_primitiveshape_spheroid, "spheroid", sim_primitiveshape_cylinder, "cylinder",
                               sim_primitiveshape_cone, "cone", sim_primitiveshape_heightfield, "heightfield",
                               sim_primitiveshape_capsule, "capsule");
            ar.xmlAddNode_float("insideScaling", _purePrimitiveInsideScaling);
            ar.xmlAddNode_3float("sizes", _purePrimitiveXSizeOrDiameter, _purePrimitiveYSize,
                                 _purePrimitiveZSizeOrHeight);
            ar.xmlPopNode();

            ar.xmlAddNode_comment(" 'verticesLocalFrame' tag: deprecated, for backward compatibility ", false);
            ar.xmlPushNewNode("verticesLocalFrame"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame * _iFrame * _bbFrame);
            ar.xmlAddNode_floats("position", w.X.data, 3);
            ar.xmlAddNode_floats("quaternion", w.Q.data, 4);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("edgesVisible", _visibleEdges);
            ar.xmlAddNode_bool("culling", _culling);
            ar.xmlAddNode_bool("wireframe", _wireframe_OLD);
            ar.xmlAddNode_comment(" 'hideEdgeBorders' tag: deprecated, for backward compatibility ", false);
            ar.xmlAddNode_bool("hideEdgeBorders", _hideEdgeBorders_OLD);
            ar.xmlPopNode();

            if (_textureProperty != nullptr)
            {
                ar.xmlPushNewNode("texture");
                _textureProperty->serialize(ar);
                ar.xmlPopNode();
            }

            ar.xmlPushNewNode("heightfield");
            ar.xmlAddNode_2int("sizes", _heightfieldXCount, _heightfieldYCount);
            ar.xmlAddNode_floats("data", _heightfieldHeights);
            ar.xmlPopNode();

            ar.xmlPushNewNode("meshData");
            if (ar.xmlSaveDataInline(_verticesForDisplayAndDisk.size() * 4 + _indices.size() * 4 +
                                     _normalsForDisplayAndDisk.size() * 4 + _edges.size()))
            {
                ar.xmlAddNode_floats("vertices", _verticesForDisplayAndDisk); // keep float
                ar.xmlAddNode_ints("indices", _indices);
                ar.xmlAddNode_floats("normals", _normalsForDisplayAndDisk); // keep float
                ar.xmlAddNode_uchars("edges", _edges);
            }
            else
                ar.xmlAddNode_meshFile("file",
                                       (std::string("mesh_") + std::string(shapeName) + "_" +
                                        utils::getIntString(false, ar.getIncrementCounter()))
                                           .c_str(),
                                       &_verticesForDisplayAndDisk[0], (int)_verticesForDisplayAndDisk.size(),
                                       &_indices[0], (int)_indices.size(), &_normalsForDisplayAndDisk[0],
                                       (int)_normalsForDisplayAndDisk.size(), &_edges[0], (int)_edges.size());
            ar.xmlPopNode();
        }
        else
        {
            bool fixFrame_prior4_5_2 = false;
            C7Vector verticesLocalFrame_old; // prev. _verticesLocalframe
            ar.xmlGetNode_float("shadingAngle", _shadingAngle);
            _shadingAngle *= piValue / 180.0;
            ar.xmlGetNode_float("edgeThresholdAngle", _edgeThresholdAngle);
            _edgeThresholdAngle *= piValue / 180.0;

            if (ar.xmlPushChildNode("color"))
            {
                color.serialize(ar, 0);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("primitive"))
            {
                ar.xmlGetNode_enum("type", _purePrimitive, true, "none", sim_primitiveshape_none, "plane",
                                   sim_primitiveshape_plane, "disc", sim_primitiveshape_disc, "cuboid",
                                   sim_primitiveshape_cuboid, "spheroid", sim_primitiveshape_spheroid, "cylinder",
                                   sim_primitiveshape_cylinder, "cone", sim_primitiveshape_cone, "heightfield",
                                   sim_primitiveshape_heightfield, "capsule", sim_primitiveshape_capsule);
                ar.xmlGetNode_float("insideScaling", _purePrimitiveInsideScaling);
                ar.xmlGetNode_3float("sizes", _purePrimitiveXSizeOrDiameter, _purePrimitiveYSize,
                                     _purePrimitiveZSizeOrHeight);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("verticesLocalFrame",
                                    false)) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            {
                ar.xmlGetNode_floats("position", verticesLocalFrame_old.X.data, 3);
                ar.xmlGetNode_floats("quaternion", verticesLocalFrame_old.Q.data, 4);
                verticesLocalFrame_old.Q.normalize(); // just in case
                if (!hasNewBBFrameAndSize)
                    fixFrame_prior4_5_2 = true;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("edgesVisible", _visibleEdges);
                ar.xmlGetNode_bool("culling", _culling);
                ar.xmlGetNode_bool("wireframe", _wireframe_OLD);
                ar.xmlGetNode_bool("hideEdgeBorders", _hideEdgeBorders_OLD, false);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("texture", false))
            {
                _textureProperty = new CTextureProperty();
                _textureProperty->serialize(ar);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("heightfield"))
            {
                ar.xmlGetNode_2int("sizes", _heightfieldXCount, _heightfieldYCount);
                ar.xmlGetNode_floats("data", _heightfieldHeights);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("meshData"))
            {
                if (ar.xmlGetNode_floats("vertices", _verticesForDisplayAndDisk, false)) // float
                {
                    ar.xmlGetNode_ints("indices", _indices);
                    ar.xmlGetNode_floats("normals", _normalsForDisplayAndDisk); // float
                    ar.xmlGetNode_uchars("edges", _edges);
                }
                else
                {
                    ar.xmlGetNode_meshFile("file", _verticesForDisplayAndDisk, _indices, _normalsForDisplayAndDisk,
                                           _edges);
                    actualizeGouraudShadingAndVisibleEdges();
                }

                _updateNonDisplayAndNonDiskValues();

                ar.xmlPopNode();
            }
            if (fixFrame_prior4_5_2)
            { // to accomodate new shapes (V4.5, rev2+)
                _transformMesh((parentCumulIFrame * _iFrame).getInverse() * verticesLocalFrame_old);
                _bbSize = _computeBBSize(&_bbFrame.X);
                _bbFrame.setIdentity();
                _transformMesh(_bbFrame.getInverse());
                _bbFrame.X.clear();
            }

            checkIfConvex(); // with XML, we do not serialize the _convex flag. So we recompute it
            if ((!_convex) && _convex_OLD)
            { // in some rare cases, old shapes won't be detected as convex anymore. Make them convex in that case!
                if (CMeshRoutines::getConvexHull(_vertices, _vertices, _indices))
                {
                    actualizeGouraudShadingAndVisibleEdges();
                    _updateDisplayAndDiskValues();
                    checkIfConvex();
                }
            }
        }
    }
    return (hasNewBBFrameAndSize);
}

void CMesh::_updateNonDisplayAndNonDiskValues()
{
    _vertices.resize(_verticesForDisplayAndDisk.size());
    for (size_t i = 0; i < _verticesForDisplayAndDisk.size(); i++)
        _vertices[i] = (double)_verticesForDisplayAndDisk[i];
    _normals.resize(_normalsForDisplayAndDisk.size());
    for (size_t i = 0; i < _normalsForDisplayAndDisk.size(); i++)
        _normals[i] = (double)_normalsForDisplayAndDisk[i];
}

void CMesh::_updateDisplayAndDiskValues()
{
    _verticesForDisplayAndDisk.resize(_vertices.size());
    for (size_t i = 0; i < _vertices.size(); i++)
        _verticesForDisplayAndDisk[i] = (float)_vertices[i];
    _normalsForDisplayAndDisk.resize(_normals.size());
    for (size_t i = 0; i < _normals.size(); i++)
        _normalsForDisplayAndDisk[i] = (float)_normals[i];
}

#ifdef SIM_WITH_GUI
void CMesh::display(const C7Vector &cumulIFrameTr, CShape *geomData, int displayAttrib, CColorObject *collisionColor,
                    int dynObjFlag_forVisualization, int transparencyHandling, bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
    displayGeometric(cumulIFrameTr * _iFrame, this, geomData, displayAttrib, collisionColor,
                     dynObjFlag_forVisualization, transparencyHandling, multishapeEditSelected);
}

void CMesh::display_colorCoded(const C7Vector &cumulIFrameTr, CShape *geomData, int objectId, int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    displayGeometric_colorCoded(cumulIFrameTr * _iFrame, this, geomData, objectId, displayAttrib);
}

void CMesh::displayGhost(const C7Vector &cumulIFrameTr, CShape *geomData, int displayAttrib, bool originalColors,
                         bool backfaceCulling, double transparency, const float *newColors)
{ // function has virtual/non-virtual counterpart!
    displayGeometricGhost(cumulIFrameTr * _iFrame, this, geomData, displayAttrib, originalColors, backfaceCulling,
                          transparency, newColors);
}

bool CMesh::getNonCalculatedTextureCoordinates(std::vector<float> &texCoords)
{
    if (_textureProperty == nullptr)
        return (false);
    std::vector<float> *tc = _textureProperty->getTextureCoordinates(-1, _verticesForDisplayAndDisk, _indices);
    if (tc == nullptr)
        return (false);
    if (!_textureProperty->getFixedCoordinates())
        return (false);
    texCoords.assign(tc->begin(), tc->end());
    return (true);
}

void CMesh::display_extRenderer(const C7Vector &cumulIFrameTr, CShape *geomData, int displayAttrib, const C7Vector &tr,
                                int shapeHandle, int &componentIndex)
{ // function has virtual/non-virtual counterpart!
    if (!_wireframe_OLD)
    {
        // Mesh change:
        if (_extRendererMeshId == 0)
        { // first time we render this item
            _extRendererUniqueMeshID++;
            _extRendererMeshId = _extRendererUniqueMeshID;
            _extRendererMesh_lastVertexBufferId = _vertexBufferId;
        }
        else
        { // we already rendered this item. Did it change?
            if (_extRendererMesh_lastVertexBufferId != _vertexBufferId)
            {
                _extRendererUniqueMeshID++;
                _extRendererMeshId = _extRendererUniqueMeshID;
                _extRendererMesh_lastVertexBufferId = _vertexBufferId;
            }
        }

        // texture change:
        if (_extRendererTextureId == 0)
        { // first time we render this item
            _extRendererTexture_lastTextureId = (unsigned int)-1;
            if (_textureProperty != nullptr)
            {
                CTextureObject *to = _textureProperty->getTextureObject();
                if (to != nullptr)
                    _extRendererTexture_lastTextureId = to->getCurrentTextureContentUniqueId();
            }
            _extRendererUniqueTextureID++;
            _extRendererTextureId = _extRendererUniqueTextureID;
        }
        else
        { // we already rendered this item. Did it change?
            unsigned int tex = (unsigned int)-1;
            if (_textureProperty != nullptr)
            {
                CTextureObject *to = _textureProperty->getTextureObject();
                if (to != nullptr)
                    tex = to->getCurrentTextureContentUniqueId();
            }
            if (tex != _extRendererTexture_lastTextureId)
            {
                _extRendererTexture_lastTextureId = tex;
                _extRendererUniqueTextureID++;
                _extRendererTextureId = _extRendererUniqueTextureID;
            }
        }

        // Object change:
        if (_extRendererObjectId == 0)
        { // first time we render this item
            _extRendererUniqueObjectID++;
            _extRendererObjectId = _extRendererUniqueObjectID;
            _extRendererObject_lastMeshId = _extRendererMeshId;
            _extRendererObject_lastTextureId = _extRendererTextureId;
        }
        else
        { // we already rendered this item. Did it change?
            if ((_extRendererObject_lastMeshId != _extRendererMeshId) ||
                (_extRendererObject_lastTextureId != _extRendererTextureId))
            {
                _extRendererUniqueObjectID++;
                _extRendererObjectId = _extRendererUniqueObjectID;
                _extRendererObject_lastMeshId = _extRendererMeshId;
                _extRendererObject_lastTextureId = _extRendererTextureId;
            }
        }

        C7Vector tr2(tr * cumulIFrameTr * _iFrame * _bbFrame);
        static int a = 0;
        a++;
        void *data[40];
        data[0] = &_verticesForDisplayAndDisk[0];
        int vs = (int)_verticesForDisplayAndDisk.size() / 3;
        data[1] = &vs;
        data[2] = &_indices[0];
        int is = (int)_indices.size() / 3;
        data[3] = &is;
        data[4] = &_normalsForDisplayAndDisk[0];
        int ns = (int)_normalsForDisplayAndDisk.size() / 3;
        data[5] = &ns;
        float x[3] = {(float)tr2.X(0), (float)tr2.X(1), (float)tr2.X(2)};
        data[6] = x;
        float q[4] = {(float)tr2.Q(0), (float)tr2.Q(1), (float)tr2.Q(2), (float)tr2.Q(3)};
        data[7] = q;
        data[8] = color.getColorsPtr();
        float sa = (float)_shadingAngle;
        data[19] = &sa;
        data[20] = &_extRendererObjectId;
        bool translucid = color.getTranslucid();
        data[21] = &translucid;
        float transparencyF = color.getOpacity();
        data[22] = &transparencyF;
        data[23] = &_culling;
        data[24] = &_extRendererMeshId;
        data[25] = &_extRendererTextureId;
        data[26] = &_edges[0];
        bool visibleEdges = _visibleEdges;
        if (displayAttrib & sim_displayattribute_forbidedges)
            visibleEdges = false;
        data[27] = &visibleEdges;
        // FREE data[28]=edgeColor_DEPRECATED.colors;
        data[30] = &displayAttrib;
        data[31] = (void *)color.getColorName().c_str();
        data[32] = &shapeHandle;
        data[33] = &componentIndex;

        // Following actually free since CoppeliaSim 3.3.0
        // But the older PovRay plugin version crash without this:
        int povMaterial = 0;
        data[29] = &povMaterial;

        CTextureProperty *tp = _textureProperty;
        if ((!App::currentWorld->environment->getShapeTexturesEnabled()) ||
            CEnvironment::getShapeTexturesTemporarilyDisabled())
            tp = nullptr;
        bool textured = false;
        std::vector<float> *textureCoords = nullptr;
        if (tp != nullptr)
        {
            textured = true;
            textureCoords =
                tp->getTextureCoordinates(geomData->getMeshModificationCounter(), _verticesForDisplayAndDisk, _indices);
            if (textureCoords == nullptr)
                return; // Should normally never happen
            data[9] = &(textureCoords[0])[0];
            int texCoordSize = (int)textureCoords->size() / 2;
            data[10] = &texCoordSize;
            CTextureObject *to = tp->getTextureObject();
            if (to == nullptr)
                return; // should normally never happen
            data[11] = (unsigned char *)to->getTextureBufferPointer();
            int sx, sy;
            to->getTextureSize(sx, sy);
            data[12] = &sx;
            data[13] = &sy;
            bool repeatU = tp->getRepeatU();
            bool repeatV = tp->getRepeatU();
            bool interpolateColors = tp->getInterpolateColors();
            int applyMode = tp->getApplyMode();
            data[14] = &repeatU;
            data[15] = &repeatV;
            data[16] = &interpolateColors;
            data[17] = &applyMode;
            data[18] = &textured;
            App::worldContainer->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_mesh, data);
        }
        else
        {
            data[18] = &textured;
            App::worldContainer->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_mesh, data);
        }
    }
    componentIndex++;
}

int *CMesh::getVertexBufferIdPtr()
{
    return (&_vertexBufferId);
}

int *CMesh::getNormalBufferIdPtr()
{
    return (&_normalBufferId);
}

int *CMesh::getEdgeBufferIdPtr()
{
    return (&_edgeBufferId);
}
#endif
