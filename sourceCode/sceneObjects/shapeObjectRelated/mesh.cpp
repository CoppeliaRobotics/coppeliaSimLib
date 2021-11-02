#include "mesh.h"
#include "global.h"
#include "tt.h"
#include "meshRoutines.h"
#include "meshManip.h"
#include "ttUtil.h"
#include "app.h"
#include "pluginContainer.h"
#include "shapeRendering.h"
#include "tt.h"
#include "base64.h"
#include "simFlavor.h"

int CMesh::_nextUniqueID=0;
unsigned int CMesh::_extRendererUniqueObjectID=0;
unsigned int CMesh::_extRendererUniqueMeshID=0;
unsigned int CMesh::_extRendererUniqueTextureID=0;

std::vector<std::vector<float>*> CMesh::_tempVertices;
std::vector<std::vector<int>*> CMesh::_tempIndices;
std::vector<std::vector<float>*> CMesh::_tempNormals;
std::vector<std::vector<unsigned char>*> CMesh::_tempEdges;

CMesh::CMesh()
{
    color.setDefaultValues();
    color.setColor(0.9f,0.9f,0.9f,sim_colorcomponent_ambient_diffuse);
    edgeColor_DEPRECATED.setColorsAllBlack();
    insideColor_DEPRECATED.setDefaultValues();

    _purePrimitive=sim_pure_primitive_none;
    _purePrimitiveXSizeOrDiameter=0.0f;
    _purePrimitiveYSize=0.0f;
    _purePrimitiveZSizeOrHeight=0.0f;
    _purePrimitiveInsideScaling=0.0f; // no inside part

    _heightfieldXCount=0;
    _heightfieldYCount=0;

    _verticeLocalFrame.setIdentity();
    _textureProperty=nullptr;

    _vertexBufferId=-1;
    _normalBufferId=-1;
    _edgeBufferId=-1;

    _edgeWidth_DEPRERCATED=1;
    _visibleEdges=false;
    _hideEdgeBorders=false;
    _culling=false;
    _displayInverted_DEPRECATED=false;
    _wireframe=false;
    _insideAndOutsideFacesSameColor_DEPRECATED=true;
    _gouraudShadingAngle=0.5f*degToRad_f;
    _edgeThresholdAngle=_gouraudShadingAngle;
    _uniqueID=_nextUniqueID++;

    _extRendererObjectId=0;
    _extRendererMeshId=0;
    _extRendererTextureId=0;
}

CMesh::~CMesh()
{
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);
    delete _textureProperty;
}

void CMesh::display_extRenderer(CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex)
{ // function has virtual/non-virtual counterpart!
    if (!_wireframe)
    {
        // Mesh change:
        if (_extRendererMeshId==0)
        { // first time we render this item
            _extRendererMeshId=_extRendererUniqueMeshID++;
            _extRendererMesh_lastVertexBufferId=_vertexBufferId;
        }
        else
        { // we already rendered this item. Did it change?
            if (_extRendererMesh_lastVertexBufferId!=_vertexBufferId)
            {
                _extRendererMeshId=_extRendererUniqueMeshID++;
                _extRendererMesh_lastVertexBufferId=_vertexBufferId;
            }
        }

        // texture change:
        if (_extRendererTextureId==0)
        { // first time we render this item
            _extRendererTexture_lastTextureId=(unsigned int)-1;
            if (_textureProperty!=nullptr)
            {
                CTextureObject* to=_textureProperty->getTextureObject();
                if (to!=nullptr)
                    _extRendererTexture_lastTextureId=to->getCurrentTextureContentUniqueId();
            }
            _extRendererTextureId=_extRendererUniqueTextureID++;
        }
        else
        { // we already rendered this item. Did it change?
            unsigned int tex=(unsigned int)-1;
            if (_textureProperty!=nullptr)
            {
                CTextureObject* to=_textureProperty->getTextureObject();
                if (to!=nullptr)
                    tex=to->getCurrentTextureContentUniqueId();
            }
            if (tex!=_extRendererTexture_lastTextureId)
            {

                _extRendererTexture_lastTextureId=tex;
                _extRendererTextureId=_extRendererUniqueTextureID++;
            }
        }

        // Object change:
        if (_extRendererObjectId==0)
        { // first time we render this item
            _extRendererObjectId=_extRendererUniqueObjectID++;
            _extRendererObject_lastMeshId=_extRendererMeshId;
            _extRendererObject_lastTextureId=_extRendererTextureId;
        }
        else
        { // we already rendered this item. Did it change?
            if ((_extRendererObject_lastMeshId!=_extRendererMeshId)||(_extRendererObject_lastTextureId!=_extRendererTextureId))
            {
                _extRendererObjectId=_extRendererUniqueObjectID++;
                _extRendererObject_lastMeshId=_extRendererMeshId;
                _extRendererObject_lastTextureId=_extRendererTextureId;
            }
        }

        C7Vector tr2(tr*_verticeLocalFrame);
        static int a=0;
        a++;
        void* data[40];
        data[0]=&_vertices[0];
        int vs=(int)_vertices.size()/3;
        data[1]=&vs;
        data[2]=&_indices[0];
        int is=(int)_indices.size()/3;
        data[3]=&is;
        data[4]=&_normals[0];
        int ns=(int)_normals.size()/3;
        data[5]=&ns;
        data[6]=tr2.X.data;

//      C3Vector euler(tr2.Q.getEulerAngles());
//      data[7]=euler.data;
        data[7]=tr2.Q.data;

        data[8]=color.getColorsPtr();
        data[19]=&_gouraudShadingAngle;
        data[20]=&_extRendererObjectId;
        bool translucid=color.getTranslucid();
        data[21]=&translucid;
        float transparencyF=color.getTransparencyFactor();
        data[22]=&transparencyF;
        data[23]=&_culling;
        data[24]=&_extRendererMeshId;
        data[25]=&_extRendererTextureId;
        data[26]=&_edges[0];
        bool visibleEdges=_visibleEdges;
        if (displayAttrib&sim_displayattribute_forbidedges)
            visibleEdges=false;
        data[27]=&visibleEdges;
        // FREE data[28]=edgeColor_DEPRECATED.colors;
        data[30]=&displayAttrib;
        data[31]=(void*)color.getColorName().c_str();
        data[32]=&shapeHandle;
        data[33]=&componentIndex;

        // Following actually free since CoppeliaSim 3.3.0
        // But the older PovRay plugin version crash without this:
        int povMaterial=0;
        data[29]=&povMaterial;

        CTextureProperty* tp=_textureProperty;
        if ((!App::currentWorld->environment->getShapeTexturesEnabled())||CEnvironment::getShapeTexturesTemporarilyDisabled())
            tp=nullptr;
        bool textured=false;
        std::vector<float>* textureCoords=nullptr;
        if (tp!=nullptr)
        {
            textured=true;
            textureCoords=tp->getTextureCoordinates(geomData->getMeshModificationCounter(),_verticeLocalFrame,_vertices,_indices);
            if (textureCoords==nullptr)
                return; // Should normally never happen
            data[9]=&(textureCoords[0])[0];
            int texCoordSize=(int)textureCoords->size()/2;
            data[10]=&texCoordSize;
            CTextureObject* to=tp->getTextureObject();
            if (to==nullptr)
                return; // should normally never happen
            data[11]=(unsigned char*)to->getTextureBufferPointer();
            int sx,sy;
            to->getTextureSize(sx,sy);
            data[12]=&sx;
            data[13]=&sy;
            bool repeatU=tp->getRepeatU();
            bool repeatV=tp->getRepeatU();
            bool interpolateColors=tp->getInterpolateColors();
            int applyMode=tp->getApplyMode();
            data[14]=&repeatU;
            data[15]=&repeatV;
            data[16]=&interpolateColors;
            data[17]=&applyMode;
            data[18]=&textured;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_mesh,data);
        }
        else
        {
            data[18]=&textured;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_mesh,data);
        }
    }
    componentIndex++;
}

void CMesh::performSceneObjectLoadingMapping(const std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->performObjectLoadingMapping(map);
}

void CMesh::performTextureObjectLoadingMapping(const std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->performTextureObjectLoadingMapping(map);
}

void CMesh::announceSceneObjectWillBeErased(int objectID)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
    {
        if (_textureProperty->announceObjectWillBeErased(objectID))
        {
            delete _textureProperty;
            _textureProperty=nullptr;
        }
    }
}

void CMesh::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->addTextureDependencies(shapeID,_uniqueID);
}

int CMesh::getTextureCount()
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        return(1);
    return(0);
}

bool CMesh::hasTextureThatUsesFixedTextureCoordinates()
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        return(_textureProperty->getFixedCoordinates());
    return(false);
}

bool CMesh::getContainsTransparentComponents()
{ // function has virtual/non-virtual counterpart!
    return(color.getTranslucid()||insideColor_DEPRECATED.getTranslucid());
}

CMesh* CMesh::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CMesh* newIt=new CMesh();
    copyWrapperInfos(newIt);

    color.copyYourselfInto(&newIt->color);
    insideColor_DEPRECATED.copyYourselfInto(&newIt->insideColor_DEPRECATED);
    edgeColor_DEPRECATED.copyYourselfInto(&newIt->edgeColor_DEPRECATED);

    newIt->_purePrimitive=_purePrimitive;
    newIt->_purePrimitiveXSizeOrDiameter=_purePrimitiveXSizeOrDiameter;
    newIt->_purePrimitiveYSize=_purePrimitiveYSize;
    newIt->_purePrimitiveZSizeOrHeight=_purePrimitiveZSizeOrHeight;
    newIt->_purePrimitiveInsideScaling=_purePrimitiveInsideScaling;
    newIt->_verticeLocalFrame=_verticeLocalFrame;

    newIt->_heightfieldXCount=_heightfieldXCount;
    newIt->_heightfieldYCount=_heightfieldYCount;
    newIt->_heightfieldHeights.insert(newIt->_heightfieldHeights.end(),_heightfieldHeights.begin(),_heightfieldHeights.end());

    newIt->_visibleEdges=_visibleEdges;
    newIt->_hideEdgeBorders=_hideEdgeBorders;
    newIt->_culling=_culling;
    newIt->_displayInverted_DEPRECATED=_displayInverted_DEPRECATED;
    newIt->_insideAndOutsideFacesSameColor_DEPRECATED=_insideAndOutsideFacesSameColor_DEPRECATED;
    newIt->_wireframe=_wireframe;
    newIt->_gouraudShadingAngle=_gouraudShadingAngle;
    newIt->_edgeThresholdAngle=_edgeThresholdAngle;
    newIt->_edgeWidth_DEPRERCATED=_edgeWidth_DEPRERCATED;

    newIt->_vertices.assign(_vertices.begin(),_vertices.end());
    newIt->_indices.assign(_indices.begin(),_indices.end());
    newIt->_normals.assign(_normals.begin(),_normals.end());
    newIt->_edges.assign(_edges.begin(),_edges.end());

    newIt->_vertexBufferId=_vertexBufferId;
    newIt->_normalBufferId=_normalBufferId;
    newIt->_edgeBufferId=_edgeBufferId;

    increaseVertexBufferRefCnt(_vertexBufferId);
    increaseNormalBufferRefCnt(_normalBufferId);
    increaseEdgeBufferRefCnt(_edgeBufferId);

    if (_textureProperty!=nullptr)
        newIt->_textureProperty=_textureProperty->copyYourself();

    return(newIt);
}

void CMesh::scale(float xVal,float yVal,float zVal)
{ // function has virtual/non-virtual counterpart!
    // Following should not really be needed (normally already done by the calling function)
    //--------------------
    if (_purePrimitive==sim_pure_primitive_plane)
        zVal=1.0f;
    if (_purePrimitive==sim_pure_primitive_disc)
    {
        zVal=1.0f;
        yVal=xVal;
    }
    if ( (_purePrimitive==sim_pure_primitive_cylinder)||(_purePrimitive==sim_pure_primitive_cone)||(_purePrimitive==sim_pure_primitive_heightfield) )
        yVal=xVal;

    scaleWrapperInfos(xVal,yVal,zVal);

    _purePrimitiveXSizeOrDiameter*=xVal;
    _purePrimitiveYSize*=yVal;
    _purePrimitiveZSizeOrHeight*=zVal;

    _verticeLocalFrame.X(0)*=xVal;
    _verticeLocalFrame.X(1)*=yVal;
    _verticeLocalFrame.X(2)*=zVal;

    C7Vector inverse(_verticeLocalFrame.getInverse());
    for (int i=0;i<int(_vertices.size())/3;i++)
    {
        C3Vector v(&_vertices[3*i+0]);
        v=_verticeLocalFrame.Q*v;
        v(0)*=xVal;
        v(1)*=yVal;
        v(2)*=zVal;
        v=inverse.Q*v;
        _vertices[3*i+0]=v(0);
        _vertices[3*i+1]=v(1);
        _vertices[3*i+2]=v(2);
    }
    
    if (_purePrimitive==sim_pure_primitive_heightfield)
    {
        for (int i=0;i<_heightfieldXCount*_heightfieldYCount;i++)
            _heightfieldHeights[i]*=zVal;
    }

    if (_textureProperty!=nullptr)
    {
        //if ( (fabs(xVal-yVal)>fabs(xVal*0.01f))||(fabs(xVal-zVal)>fabs(xVal*0.01f)) ) // if we do not have iso scaling, we transform the texture from text. coord. calculated into fixed text. coords:
        //    _textureProperty->transformToFixedTextureCoordinates(_verticeLocalFrame,_vertices,_indices);
        _textureProperty->scaleObject(xVal);
    }
    if ((xVal!=yVal)||(xVal!=zVal))
        actualizeGouraudShadingAndVisibleEdges(); // we need to recompute the normals and edges
    if ((xVal<0.0f)||(yVal<0.0f)||(zVal<0.0f)) // that effectively flips faces!
        checkIfConvex();

    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId=-1;
    _normalBufferId=-1;
    _edgeBufferId=-1;
}

void CMesh::setMeshDataDirect(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>& normals,const std::vector<unsigned char>& edges)
{
    _vertices.assign(vertices.begin(),vertices.end());
    _indices.assign(indices.begin(),indices.end());
    _normals.assign(normals.begin(),normals.end());
    _edges.assign(edges.begin(),edges.end());
    checkIfConvex();

    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId=-1;
    _normalBufferId=-1;
    _edgeBufferId=-1;
}

void CMesh::setMesh(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>* normals,const C7Vector& transformation)
{
    _vertices.assign(vertices.begin(),vertices.end());
    _indices.assign(indices.begin(),indices.end());
    if (normals==nullptr)
    {
        CMeshManip::getNormals(&_vertices,&_indices,&_normals);
        _recomputeNormals();
    }
    else
        _normals.assign(normals->begin(),normals->end());
    _verticeLocalFrame=transformation;
    _computeVisibleEdges();
    checkIfConvex();

    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);

    _vertexBufferId=-1;
    _normalBufferId=-1;
    _edgeBufferId=-1;
}

void CMesh::setPurePrimitiveType(int theType,float xOrDiameter,float y,float zOrHeight)
{ // function has virtual/non-virtual counterpart!
    _purePrimitive=theType;
    _purePrimitiveXSizeOrDiameter=xOrDiameter;
    _purePrimitiveYSize=y;
    _purePrimitiveZSizeOrHeight=zOrHeight;

    // Set the convex flag only to true!! never to false, at least not here
    if ((theType!=sim_pure_primitive_heightfield)&&(theType!=sim_pure_primitive_none)&&(theType!=sim_pure_primitive_plane)&&(theType!=sim_pure_primitive_disc))
        _convex=true;
    if (theType!=sim_pure_primitive_heightfield)
    {
        _heightfieldHeights.clear();
        _heightfieldXCount=0;
        _heightfieldYCount=0;
    }

    if ((theType!=sim_pure_primitive_cuboid)&&(theType!=sim_pure_primitive_spheroid)&&(theType!=sim_pure_primitive_cylinder))
        _purePrimitiveInsideScaling=0.0f; // no inside part!
}

int CMesh::getPurePrimitiveType()
{ // function has virtual/non-virtual counterpart!
    return(_purePrimitive);
}

bool CMesh::isMesh()
{ // function has virtual/non-virtual counterpart!
    return(true);
}

bool CMesh::isPure()
{ // function has virtual/non-virtual counterpart!
    return(_purePrimitive!=sim_pure_primitive_none);
}

bool CMesh::isConvex()
{ // function has virtual/non-virtual counterpart!
    return(_convex);
}

bool CMesh::containsOnlyPureConvexShapes()
{ // function has virtual/non-virtual counterpart!
    bool retVal=((_purePrimitive!=sim_pure_primitive_none)&&(_purePrimitive!=sim_pure_primitive_heightfield)&&(_purePrimitive!=sim_pure_primitive_plane)&&(_purePrimitive!=sim_pure_primitive_disc));
    if (retVal)
        _convex=retVal; // needed since there was a bug where pure planes and pure discs were considered as convex
    return(retVal);
}

void CMesh::setConvex(bool convex)
{ // function has virtual/non-virtual counterpart!
    _convex=convex;

/* removed on 24/3/2013
    if (_convex)
    { // convex shape handling in Bullet includes a very large margin. We can:
        // 1. shrink the dynamic model using _bulletAutoShrinkConvexMesh. This adds some initial preprocessing time, can lead to crashes, and edges and points appear shifted inwards. Faces react correctly.
        // 2. reduce the margin (what we do here). Erwin from Bullet doesn't recommend it (doesn't say why), but I got the best (still not good!!) results with it
        _bulletNonDefaultCollisionMarginFactor=0.002f;
        _bulletNonDefaultCollisionMargin=true;
    }
    else
        _bulletNonDefaultCollisionMargin=false;
        */
}

void CMesh::getCumulativeMeshes(std::vector<float>& vertices,std::vector<int>* indices,std::vector<float>* normals)
{ // function has virtual/non-virtual counterpart!
    size_t offset=vertices.size()/3;
    for (size_t i=0;i<_vertices.size()/3;i++)
    {
        C3Vector v(&_vertices[3*i]);
        v*=_verticeLocalFrame;
        vertices.push_back(v(0));
        vertices.push_back(v(1));
        vertices.push_back(v(2));
    }
    if (indices!=nullptr)
    {
        for (size_t i=0;i<_indices.size();i++)
            indices->push_back(_indices[i]+int(offset));
    }
    if (normals!=nullptr)
    {
        C4Vector rot(_verticeLocalFrame.Q);
        for (size_t i=0;i<_normals.size()/3;i++)
        {
            C3Vector v(&_normals[3*i]);
            v=rot*v;
            normals->push_back(v(0));
            normals->push_back(v(1));
            normals->push_back(v(2));
        }
    }
}

void CMesh::setColor(const CShape* shape,int& elementIndex,const char* colorName,int colorComponent,const float* rgbData,int& rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    if ( (colorName==nullptr)||(color.getColorName().compare(colorName)==0)||(strcmp(colorName,"@compound")==0) )
    {
        bool compoundColors=(colorName!=nullptr)&&(strcmp(colorName,"@compound")==0);
        if (colorComponent<sim_colorcomponent_transparency)
        { // regular components
            for (int i=0;i<3;i++)
                color.getColorsPtr()[colorComponent*3+i]=rgbData[rgbDataOffset+i];
            if (compoundColors)
                rgbDataOffset+=3;
        }
        if (colorComponent==sim_colorcomponent_transparency)
        {
            color.setTransparencyFactor(rgbData[rgbDataOffset+0]);
            color.setTranslucid(rgbData[rgbDataOffset+0]<1.0f);
            if (compoundColors)
                rgbDataOffset+=1;
        }
        if (colorComponent==sim_colorcomponent_auxiliary)
        { // auxiliary components
            for (int i=0;i<3;i++)
                color.getColorsPtr()[12+i]=rgbData[rgbDataOffset+i];
            if (compoundColors)
                rgbDataOffset+=3;
        }
        if (shape!=nullptr)
            color.pushColorChangeEvent(shape->getObjectHandle(),elementIndex);
    }
    if ((colorName!=nullptr)&&(insideColor_DEPRECATED.getColorName().compare(colorName)==0))
    { // OLD, deprecated
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                insideColor_DEPRECATED.getColorsPtr()[colorComponent*3+i]=rgbData[rgbDataOffset+i];
        }
        if (colorComponent==4)
        {
            insideColor_DEPRECATED.setTransparencyFactor(rgbData[rgbDataOffset+0]);
            insideColor_DEPRECATED.setTranslucid(rgbData[rgbDataOffset+0]<1.0f);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                insideColor_DEPRECATED.getColorsPtr()[12+i]=rgbData[rgbDataOffset+i];
        }
    }
    if ((colorName!=nullptr)&&(edgeColor_DEPRECATED.getColorName().compare(colorName)==0))
    { // OLD, deprecated
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                edgeColor_DEPRECATED.getColorsPtr()[colorComponent*3+i]=rgbData[rgbDataOffset+i];
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                edgeColor_DEPRECATED.getColorsPtr()[12+i]=rgbData[rgbDataOffset+i];
        }
    }


    if ( (colorName!=nullptr)&&(strlen(colorName)==2)&&(colorName[0]=='@') )
    { // operations in the HSL space
        if (colorName[1]=='0')
        { // color
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(color.getColorsPtr()+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,color.getColorsPtr()+colorComponent*3);
            }
            if (colorComponent==4)
            {
                color.setTransparencyFactor(tt::getLimitedFloat(0.0f,1.0f,color.getTransparencyFactor()+rgbData[rgbDataOffset+0]));
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(color.getColorsPtr()+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,color.getColorsPtr()+12);
            }
        }
        if (colorName[1]=='1')
        { // OLD, deprecated (inside color)
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.getColorsPtr()+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,insideColor_DEPRECATED.getColorsPtr()+colorComponent*3);
            }
            if (colorComponent==4)
            {
                insideColor_DEPRECATED.setTransparencyFactor(tt::getLimitedFloat(0.0f,1.0f,insideColor_DEPRECATED.getTransparencyFactor()+rgbData[rgbDataOffset+0]));
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.getColorsPtr()+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,insideColor_DEPRECATED.getColorsPtr()+12);
            }
        }
        if (colorName[1]=='2')
        { // OLD, deprecated (edge color)
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.getColorsPtr()+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,edgeColor_DEPRECATED.getColorsPtr()+colorComponent*3);
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.getColorsPtr()+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[rgbDataOffset+0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[rgbDataOffset+1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[rgbDataOffset+2]);
                tt::hslToRgb(hsl,edgeColor_DEPRECATED.getColorsPtr()+12);
            }
        }
    }
    elementIndex++;
}

bool CMesh::getColor(const char* colorName,int colorComponent,float* rgbData,int& rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    if ( (colorName==nullptr)||(color.getColorName().compare(colorName)==0)||(strcmp(colorName,"@compound")==0) )
    {
        bool compoundColors=(colorName!=nullptr)&&(strcmp(colorName,"@compound")==0);
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=color.getColorsPtr()[colorComponent*3+i];
            if (compoundColors)
                rgbDataOffset+=3;
            return(true);
        }
        if (colorComponent==4)
        {
            rgbData[rgbDataOffset+0]=color.getTransparencyFactor();
            if (compoundColors)
                rgbDataOffset+=1;
            return(true);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=color.getColorsPtr()[12+i];
            if (compoundColors)
                rgbDataOffset+=3;
            return(true);
        }
        return(false);
    }
    if ((colorName!=nullptr)&&(insideColor_DEPRECATED.getColorName().compare(colorName)==0))
    { // OLD, deprecated
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=insideColor_DEPRECATED.getColorsPtr()[colorComponent*3+i];
            return(true);
        }
        if (colorComponent==4)
        {
            rgbData[rgbDataOffset+0]=insideColor_DEPRECATED.getTransparencyFactor();
            return(true);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=insideColor_DEPRECATED.getColorsPtr()[12+i];
            return(true);
        }
        return(false);
    }
    if ((colorName!=nullptr)&&(edgeColor_DEPRECATED.getColorName().compare(colorName)==0))
    { // OLD, deprecated
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=edgeColor_DEPRECATED.getColorsPtr()[colorComponent*3+i];
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[rgbDataOffset+i]=edgeColor_DEPRECATED.getColorsPtr()[12+i];
        }
        return(false);
    }
    return(false);
}

void CMesh::getAllShapeComponentsCumulative(std::vector<CMesh*>& shapeComponentList)
{   // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    shapeComponentList.push_back(this);
}

int CMesh::countAllShapeComponentsCumulative()
{   // function has virtual/non-virtual counterpart!
    return(1);
}

CMesh* CMesh::getShapeComponentAtIndex(int& index)
{ // function has virtual/non-virtual counterpart!
    if (index<0)
        return(nullptr);
    if (index==0)
        return(this);
    index--;
    return(nullptr);
}

int CMesh::getComponentCount() const
{ // function has virtual/non-virtual counterpart!
    return(1);
}

int CMesh::getUniqueID()
{
    return(_uniqueID);
}

void CMesh::setHeightfieldData(const std::vector<float>& heights,int xCount,int yCount)
{
    _heightfieldHeights.clear();
    _heightfieldHeights.insert(_heightfieldHeights.end(),heights.begin(),heights.end());
    _heightfieldXCount=xCount;
    _heightfieldYCount=yCount;
}

float* CMesh::getHeightfieldData(int& xCount,int& yCount,float& minHeight,float& maxHeight)
{
    setHeightfieldDiamonds(0);
    if ( (_purePrimitive!=sim_pure_primitive_heightfield)||(_heightfieldHeights.size()==0) )
        return(nullptr);
    xCount=_heightfieldXCount;
    yCount=_heightfieldYCount;
    minHeight=+99999999.0f;
    maxHeight=-99999999.0f;
    for (int i=0;i<int(_heightfieldHeights.size());i++)
    {
        if (_heightfieldHeights[i]<minHeight)
            minHeight=_heightfieldHeights[i];
        if (_heightfieldHeights[i]>maxHeight)
            maxHeight=_heightfieldHeights[i];
    }
    return(&_heightfieldHeights[0]);
}

void CMesh::setHeightfieldDiamonds(int d)
{ 
    if (_purePrimitive==sim_pure_primitive_heightfield)
    {
        for (size_t i=0;i<_indices.size()/6;i++)
        {
            if (d==0)
            {
                _indices[6*i+1]=_indices[6*i+3];
                _indices[6*i+5]=_indices[6*i+2];
            }
            if (d==1)
            {
                _indices[6*i+1]=_indices[6*i+4];
                _indices[6*i+5]=_indices[6*i+0];
            }
        }
    }
}

void CMesh::getPurePrimitiveSizes(C3Vector& s)
{
    s(0)=_purePrimitiveXSizeOrDiameter;
    s(1)=_purePrimitiveYSize;
    s(2)=_purePrimitiveZSizeOrHeight;
}

void CMesh::setPurePrimitiveInsideScaling(float s)
{
    _purePrimitiveInsideScaling=tt::getLimitedFloat(0.0f,0.99999f,s);
}

float CMesh::getPurePrimitiveInsideScaling()
{
    return(_purePrimitiveInsideScaling);
}

void CMesh::setConvexVisualAttributes()
{
    _hideEdgeBorders=false;
    setGouraudShadingAngle(0.0f);
    setEdgeThresholdAngle(0.0f);
    setVisibleEdges(false);
    color.setConvexColors();
    edgeColor_DEPRECATED.setColorsAllBlack();
    _insideAndOutsideFacesSameColor_DEPRECATED=true;
}

CTextureProperty* CMesh::getTextureProperty()
{
    return(_textureProperty);
}

void CMesh::setTextureProperty(CTextureProperty* tp)
{ // careful, this doesn't check if a _textureProperty already exists! Has to be done and destroyed outside!
    _textureProperty=tp;
}

void CMesh::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    _insideAndOutsideFacesSameColor_DEPRECATED=s;
}

bool CMesh::getInsideAndOutsideFacesSameColor_DEPRECATED()
{
    return(_insideAndOutsideFacesSameColor_DEPRECATED);
}

bool CMesh::getVisibleEdges()
{
    return(_visibleEdges);
}

void CMesh::setVisibleEdges(bool v)
{
    _visibleEdges=v;
}

void CMesh::setHideEdgeBorders(bool v)
{
    if (_hideEdgeBorders!=v)
    {
        _hideEdgeBorders=v;
        _computeVisibleEdges();
    }
}

bool CMesh::getHideEdgeBorders()
{
    return(_hideEdgeBorders);
}

int CMesh::getEdgeWidth_DEPRECATED()
{
    return(_edgeWidth_DEPRERCATED);
}

void CMesh::setEdgeWidth_DEPRECATED(int w)
{
    w=tt::getLimitedInt(1,4,w);
    _edgeWidth_DEPRERCATED=w;
}

bool CMesh::getCulling()
{
    return(_culling);
}

void CMesh::setCulling(bool c)
{
    _culling=c;
}

bool CMesh::getDisplayInverted_DEPRECATED()
{
    return(_displayInverted_DEPRECATED);
}

void CMesh::setDisplayInverted_DEPRECATED(bool di)
{
    _displayInverted_DEPRECATED=di;
}

void CMesh::copyVisualAttributesTo(CMesh* target)
{
    color.copyYourselfInto(&target->color);
    insideColor_DEPRECATED.copyYourselfInto(&target->insideColor_DEPRECATED);
    edgeColor_DEPRECATED.copyYourselfInto(&target->edgeColor_DEPRECATED);

    target->_visibleEdges=_visibleEdges;
    target->_hideEdgeBorders=_hideEdgeBorders;
    target->_culling=_culling;
    target->_displayInverted_DEPRECATED=_displayInverted_DEPRECATED;
    target->_insideAndOutsideFacesSameColor_DEPRECATED=_insideAndOutsideFacesSameColor_DEPRECATED;
    target->_wireframe=_wireframe;
    target->_edgeWidth_DEPRERCATED=_edgeWidth_DEPRERCATED;
    target->_gouraudShadingAngle=_gouraudShadingAngle;
    target->_edgeThresholdAngle=_edgeThresholdAngle;
}

float CMesh::getGouraudShadingAngle()
{ // function has virtual/non-virtual counterpart!
    return(_gouraudShadingAngle);
}

void CMesh::setGouraudShadingAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0f,89.0f*degToRad_f,angle);
    if (_gouraudShadingAngle!=angle)
    {
        _gouraudShadingAngle=angle;
        _recomputeNormals();
    }
}

float CMesh::getEdgeThresholdAngle()
{ // function has virtual/non-virtual counterpart!
    return(_edgeThresholdAngle);
}

void CMesh::setEdgeThresholdAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0f,89.0f*degToRad_f,angle);
    if (_edgeThresholdAngle!=angle)
    {
        _edgeThresholdAngle=angle;
        _computeVisibleEdges();
    }
}

void CMesh::setWireframe(bool w)
{
    _wireframe=w;
}

bool CMesh::getWireframe()
{
    return(_wireframe);
}

C7Vector CMesh::getVerticeLocalFrame()
{
    return(_verticeLocalFrame);
}

void CMesh::setVerticeLocalFrame(const C7Vector& tr)
{
    _verticeLocalFrame=tr;
}

std::vector<float>* CMesh::getVertices()
{
    return(&_vertices);
}

std::vector<int>* CMesh::getIndices()
{
    return(&_indices);
}

std::vector<float>* CMesh::getNormals()
{
    return(&_normals);
}

std::vector<unsigned char>* CMesh::getEdges()
{
    return(&_edges);
}

int* CMesh::getVertexBufferIdPtr()
{
    return(&_vertexBufferId);
}

int* CMesh::getNormalBufferIdPtr()
{
    return(&_normalBufferId);
}

int* CMesh::getEdgeBufferIdPtr()
{
    return(&_edgeBufferId);
}


void CMesh::preMultiplyAllVerticeLocalFrames(const C7Vector& preTr)
{ // function has virtual/non-virtual counterpart!

    _transformationsSinceGrouping=preTr*_transformationsSinceGrouping;
    _localInertiaFrame=preTr*_localInertiaFrame;

    _verticeLocalFrame=preTr*_verticeLocalFrame;

    if (_textureProperty!=nullptr)
        _textureProperty->adjustForFrameChange(preTr);
}

void CMesh::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    delete _textureProperty;
    _textureProperty=nullptr;
}

void CMesh::getColorStrings(std::string& colorStrings)
{ // function has virtual/non-virtual counterpart!
    if ( (color.getColorName().length()>0)&&(colorStrings.find(color.getColorName())==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=color.getColorName();
    }
    if ( (insideColor_DEPRECATED.getColorName().length()>0)&&(colorStrings.find(insideColor_DEPRECATED.getColorName())==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=insideColor_DEPRECATED.getColorName();
    }
    if ( (edgeColor_DEPRECATED.getColorName().length()>0)&&(colorStrings.find(edgeColor_DEPRECATED.getColorName())==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=edgeColor_DEPRECATED.getColorName();
    }
}

void CMesh::flipFaces()
{ // function has virtual/non-virtual counterpart!
    int save;
    float normSave;
    for (int i=0;i<int(_indices.size())/3;i++)
    {
        save=_indices[3*i+0];
        _indices[3*i+0]=_indices[3*i+2];
        _indices[3*i+2]=save;

        normSave=-_normals[3*(3*i+0)+0];
        _normals[3*(3*i+0)+0]=-_normals[3*(3*i+2)+0];
        _normals[3*(3*i+1)+0]*=-1.0f;
        _normals[3*(3*i+2)+0]=normSave;

        normSave=-_normals[3*(3*i+0)+1];
        _normals[3*(3*i+0)+1]=-_normals[3*(3*i+2)+1];
        _normals[3*(3*i+1)+1]*=-1.0f;
        _normals[3*(3*i+2)+1]=normSave;

        normSave=-_normals[3*(3*i+0)+2];
        _normals[3*(3*i+0)+2]=-_normals[3*(3*i+2)+2];
        _normals[3*(3*i+1)+2]*=-1.0f;
        _normals[3*(3*i+2)+2]=normSave;  
    }
    _computeVisibleEdges();
    checkIfConvex();

    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);

    _normalBufferId=-1;
    _vertexBufferId=-1;
}

void CMesh::actualizeGouraudShadingAndVisibleEdges()
{
    _recomputeNormals();
    _computeVisibleEdges();
}

void CMesh::_recomputeNormals()
{
    _normals.resize(3*_indices.size());
    float maxAngle=_gouraudShadingAngle;
    C3Vector v[3];
    for (int i=0;i<int(_indices.size())/3;i++)
    {   // Here we restore first all the normal vectors
        v[0]=C3Vector(&_vertices[3*(_indices[3*i+0])]);
        v[1]=C3Vector(&_vertices[3*(_indices[3*i+1])]);
        v[2]=C3Vector(&_vertices[3*(_indices[3*i+2])]);

        C3Vector v1(v[1]-v[0]);
        C3Vector v2(v[2]-v[0]);
        C3Vector n((v1^v2).getNormalized());

        _normals[9*i+0]=n(0);
        _normals[9*i+1]=n(1);
        _normals[9*i+2]=n(2);
        _normals[9*i+3]=n(0);
        _normals[9*i+4]=n(1);
        _normals[9*i+5]=n(2);
        _normals[9*i+6]=n(0);
        _normals[9*i+7]=n(1);
        _normals[9*i+8]=n(2);
    }

    std::vector<std::vector<int>*> indexToNormals;
    for (int i=0;i<int(_vertices.size())/3;i++)
    {
        std::vector<int>* sharingNormals=new std::vector<int>;
        indexToNormals.push_back(sharingNormals);
    }
    for (int i=0;i<int(_indices.size())/3;i++)
    {
        indexToNormals[_indices[3*i+0]]->push_back(3*i+0);
        indexToNormals[_indices[3*i+1]]->push_back(3*i+1);
        indexToNormals[_indices[3*i+2]]->push_back(3*i+2);
    }
    std::vector<float> changedNorm(_normals.size());

    for (int i=0;i<int(indexToNormals.size());i++)
    {
        for (int j=0;j<int(indexToNormals[i]->size());j++)
        {
            C3Vector totN;
            float nb=1.0f;
            C3Vector nActual;
            nActual.set(&_normals[3*(indexToNormals[i]->at(j))]);
            totN=nActual;
            for (int k=0;k<int(indexToNormals[i]->size());k++)
            {
                if (j!=k)
                {
                    C3Vector nToCompare(&_normals[3*(indexToNormals[i]->at(k))]);
                    if (nActual.getAngle(nToCompare)<maxAngle)
                    {
                        totN+=nToCompare;
                        nb=nb+1.0f;
                    }
                }
            }
            changedNorm[3*indexToNormals[i]->at(j)+0]=totN(0)/nb;
            changedNorm[3*indexToNormals[i]->at(j)+1]=totN(1)/nb;
            changedNorm[3*indexToNormals[i]->at(j)+2]=totN(2)/nb;
        }
        indexToNormals[i]->clear();
        delete indexToNormals[i];
    }
    // Now we have to replace the modified normals:
    for (int i=0;i<int(_indices.size())/3;i++)
    {
        for (int j=0;j<9;j++)
            _normals[9*i+j]=changedNorm[9*i+j];
    }

    decreaseNormalBufferRefCnt(_normalBufferId);

    _normalBufferId=-1;
}

void CMesh::_computeVisibleEdges()
{
    if (_indices.size()==0)
        return;
    float softAngle=_edgeThresholdAngle;
    _edges.clear();
    std::vector<int> eIDs;
    CMeshRoutines::getEdgeFeatures(&_vertices[0],(int)_vertices.size(),&_indices[0],(int)_indices.size(),nullptr,&eIDs,nullptr,softAngle,true,_hideEdgeBorders);
    _edges.assign((_indices.size()/8)+1,0);
    std::vector<bool> usedEdges(_indices.size(),false);
    for (int i=0;i<int(eIDs.size());i++)
    {
        if (eIDs[i]!=-1)
        {
            _edges[i>>3]|=(1<<(i&7));
            usedEdges[eIDs[i]]=true;
        }
    }
    _edgeBufferId=-1;
}


bool CMesh::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex=CMeshRoutines::checkIfConvex(_vertices,_indices,0.015f); // 1.5% tolerance of the average bounding box side length
    setConvex(_convex);
    return(_convex);
}


void CMesh::_savePackedIntegers(CSer& ar,const std::vector<int>& data)
{
    ar << int(data.size());
    int prevInd=0;
    for (int i=0;i<int(data.size());i++)
    {
        int currInd=data[i];
        int diff=currInd-prevInd;
        if (abs(diff)<0x00000020) // 32
        { // this index takes only 1 byte storage!
            ar << (unsigned char)(diff+31); // we use the two msb to indicate the byte count (1 --> 0)
        }
        else
        { // this index takes more than 1 byte storage!
            if (abs(diff)<0x00002000) // 8192
            { // this index takes 2 byte storage!
                diff+=8191;
                ar << (unsigned char)((diff&0x0000003f)|0x00000040); // we use the two msb to indicate the byte count (2 --> 1)
                ar << (unsigned char)(diff>>6);
            }
            else
            { // this index takes more than 2 byte storage!
                if (abs(diff)<0x00200000) // 2097152
                { // this index takes 3 byte storage!
                    diff+=2097151;
                    ar << (unsigned char)((diff&0x0000003f)|0x00000080); // we use the two msb to indicate the byte count (3 --> 2)
                    ar << (unsigned char)(diff>>6);
                    ar << (unsigned char)(diff>>14);
                }
                else
                { // this index takes 4 byte storage!
                    diff+=536870911;
                    ar << (unsigned char)((diff&0x0000003f)|0x000000c0); // we use the two msb to indicate the byte count (4 --> 3)
                    ar << (unsigned char)(diff>>6);
                    ar << (unsigned char)(diff>>14);
                    ar << (unsigned char)(diff>>22);
                }
            }
        }
        prevInd=currInd;
    }
}

void CMesh::_loadPackedIntegers(CSer& ar,std::vector<int>& data)
{
    int dataLength;
    ar >> dataLength;
    data.clear();
    unsigned char b0,b1,b2,b3,storageB;
    int prevInd=0;
    for (int i=0;i<dataLength;i++)
    {
        b1=0;
        b2=0;
        b3=0;
        ar >> b0;
        storageB=((b0&0xc0)>>6); 
        b0&=0x3f; // we remove the storage byte info
        if (storageB>=1)
            ar >> b1; // this index takes 2 or more storage bytes
        if (storageB>=2)
            ar >> b2; // this index takes 3 or more storage bytes
        if (storageB>=3)
            ar >> b3; // this index takes 4 storage bytes
        int diff=b0+(b1<<6)+(b2<<14)+(b3<<22);
        if (storageB==0)
            diff-=31;
        if (storageB==1)
            diff-=8191;
        if (storageB==2)
            diff-=2097151;
        if (storageB==3)
            diff-=536870911;
        int currInd=prevInd+diff;
        data.push_back(currInd);
        prevInd=currInd;
    }
}

void CMesh::clearTempVerticesIndicesNormalsAndEdges()
{
    for (int i=0;i<int(_tempVertices.size());i++)
        delete _tempVertices[i];
    _tempVertices.clear();

    for (int i=0;i<int(_tempIndices.size());i++)
        delete _tempIndices[i];
    _tempIndices.clear();

    for (int i=0;i<int(_tempNormals.size());i++)
        delete _tempNormals[i];
    _tempNormals.clear();

    for (int i=0;i<int(_tempEdges.size());i++)
        delete _tempEdges[i];
    _tempEdges.clear();
}

void CMesh::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    _tempVerticesIndexForSerialization=getBufferIndexOfVertices(_vertices);
    if (_tempVerticesIndexForSerialization==-1)
        _tempVerticesIndexForSerialization=addVerticesToBufferAndReturnIndex(_vertices);

    _tempIndicesIndexForSerialization=getBufferIndexOfIndices(_indices);
    if (_tempIndicesIndexForSerialization==-1)
        _tempIndicesIndexForSerialization=addIndicesToBufferAndReturnIndex(_indices);

    _tempNormalsIndexForSerialization=getBufferIndexOfNormals(_normals);
    if (_tempNormalsIndexForSerialization==-1)
        _tempNormalsIndexForSerialization=addNormalsToBufferAndReturnIndex(_normals);

    _tempEdgesIndexForSerialization=getBufferIndexOfEdges(_edges);
    if (_tempEdgesIndexForSerialization==-1)
        _tempEdgesIndexForSerialization=addEdgesToBufferAndReturnIndex(_edges);
}

void CMesh::serializeTempVerticesIndicesNormalsAndEdges(CSer& ar)
{
    if (ar.isStoring())
    {       // Storing
        for (int c=0;c<int(_tempVertices.size());c++)
        {
            ar.storeDataName("Ver");
            if (_tempVertices[c]->size()!=0)
            {
                std::vector<unsigned char>* serBuffer=ar.getBufferPointer();
                unsigned char* ptr = reinterpret_cast<unsigned char*>(&_tempVertices[c]->at(0));
                serBuffer->insert(serBuffer->end(),ptr,ptr+_tempVertices[c]->size()* sizeof(float));
            }
            ar.flush();
        }

        for (int c=0;c<int(_tempIndices.size());c++)
        {
            if (App::userSettings->packIndices)
            { // to save storage space
                ar.storeDataName("In2");
                _savePackedIntegers(ar,*_tempIndices[c]);
                ar.flush();
            }
            else
            {
                ar.storeDataName("Ind");
                for (int i=0;i<int(_tempIndices[c]->size());i++)
                    ar << _tempIndices[c]->at(i);
                ar.flush();
            }
        }

        for (int c=0;c<int(_tempNormals.size());c++)
        {
            if (App::userSettings->saveApproxNormals)
            {
                ar.storeDataName("No2");
                for (int i=0;i<int(_tempNormals[c]->size())/3;i++)
                {
                    C3Vector n(&_tempNormals[c]->at(3*i));
                    n*=15.0f;
                    n(0)+=0.5f*n(0)/fabsf(n(0));
                    n(1)+=0.5f*n(1)/fabsf(n(1));
                    n(2)+=0.5f*n(2)/fabsf(n(2));
                    char x=char(n(0));
                    char y=char(n(1));
                    char z=char(n(2));
                    unsigned short w=(x+15)+((y+15)<<5)+((z+15)<<10);
                    ar << w;
                }
                ar.flush();
            }
            else
            {
                ar.storeDataName("Nor");
                for (int i=0;i<int(_tempNormals[c]->size());i++)
                    ar << _tempNormals[c]->at(i);
                ar.flush();
            }
        }

        for (int c=0;c<int(_tempEdges.size());c++)
        {
            ar.storeDataName("Ved"); // Recomputing edges at load takes too much time
            if (_tempEdges[c]->size()!=0)
            {
                std::vector<unsigned char>* serBuffer=ar.getBufferPointer();
                unsigned char* ptr = &_tempEdges[c]->at(0);
                serBuffer->insert(serBuffer->end(),ptr,ptr+_tempEdges[c]->size());
            }
            ar.flush();
        }

        ar.storeDataName(SER_END_OF_OBJECT);
    }
    else
    { // loading
        int byteQuantity;
        std::string theName="";
        while (theName.compare(SER_END_OF_OBJECT)!=0)
        {
            theName=ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                bool noHit=true;
                if (theName.compare("Ver")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<float>* arr=new std::vector<float>;
                    _tempVertices.push_back(arr);
                    arr->resize(byteQuantity/sizeof(float),0.0f);
                    for (int i=0;i<int(arr->size());i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("Ind")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<int>* arr=new std::vector<int>;
                    _tempIndices.push_back(arr);
                    arr->resize(byteQuantity/sizeof(int),0);
                    for (int i=0;i<int(arr->size());i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("In2")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<int>* arr=new std::vector<int>;
                    _tempIndices.push_back(arr);
                    _loadPackedIntegers(ar,*arr);
                }
                if (theName.compare("Nor")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<float>* arr=new std::vector<float>;
                    _tempNormals.push_back(arr);
                    arr->resize(byteQuantity/sizeof(float),0.0f);
                    for (int i=0;i<int(arr->size());i++)
                        ar >> arr->at(i);
                }
                if (theName.compare("No2")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<float>* arr=new std::vector<float>;
                    _tempNormals.push_back(arr);
                    arr->resize(byteQuantity*6/sizeof(float),0.0f);
                    for (int i=0;i<byteQuantity/2;i++)
                    {
                        unsigned short w;
                        ar >> w;
                        char x=(w&0x001f)-15;
                        char y=((w>>5)&0x001f)-15;
                        char z=((w>>10)&0x001f)-15;
                        C3Vector n((float)x,(float)y,(float)z);
                        n.normalize();
                        arr->at(3*i+0)=n(0);
                        arr->at(3*i+1)=n(1);
                        arr->at(3*i+2)=n(2);
                    }
                }
                if (theName.compare("Ved")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    std::vector<unsigned char>* arr=new std::vector<unsigned char>;
                    _tempEdges.push_back(arr);
                    arr->resize(byteQuantity,0);
                    for (int i=0;i<byteQuantity;i++)
                        ar >> arr->at(i);
                }
            }
        }
    }
}

int CMesh::getBufferIndexOfVertices(const std::vector<float>& vert)
{
    int vertl=(int)vert.size();
    for (int i=0;i<int(_tempVertices.size());i++)
    {
        if (int(_tempVertices[i]->size())==vertl)
        { // might be same!
            bool same=true;
            for (int j=0;j<vertl;j++)
            {
                if (vert[j]!=_tempVertices[i]->at(j))
                {
                    same=false;
                    break;
                }
            }
            if (same)
                return(i);
        }
    }

    return(-1); // not found
}

int CMesh::addVerticesToBufferAndReturnIndex(const std::vector<float>& vert)
{
    std::vector<float>* nvert=new std::vector<float>;
    nvert->assign(vert.begin(),vert.end());
    _tempVertices.push_back(nvert);
    return((int)_tempVertices.size()-1);
}

void CMesh::getVerticesFromBufferBasedOnIndex(int index,std::vector<float>& vert)
{
    vert.assign(_tempVertices[index]->begin(),_tempVertices[index]->end());
}



int CMesh::getBufferIndexOfIndices(const std::vector<int>& ind)
{
    int indl=(int)ind.size();
    for (int i=0;i<int(_tempIndices.size());i++)
    {
        if (int(_tempIndices[i]->size())==indl)
        { // might be same!
            bool same=true;
            for (int j=0;j<indl;j++)
            {
                if (ind[j]!=_tempIndices[i]->at(j))
                {
                    same=false;
                    break;
                }
            }
            if (same)
                return(i);
        }
    }

    return(-1); // not found
}

int CMesh::addIndicesToBufferAndReturnIndex(const std::vector<int>& ind)
{
    std::vector<int>* nind=new std::vector<int>;
    nind->assign(ind.begin(),ind.end());
    _tempIndices.push_back(nind);
    return((int)_tempIndices.size()-1);
}

void CMesh::getIndicesFromBufferBasedOnIndex(int index,std::vector<int>& ind)
{
    ind.assign(_tempIndices[index]->begin(),_tempIndices[index]->end());
}

int CMesh::getBufferIndexOfNormals(const std::vector<float>& norm)
{
    int norml=(int)norm.size();
    for (int i=0;i<int(_tempNormals.size());i++)
    {
        if (int(_tempNormals[i]->size())==norml)
        { // might be same!
            bool same=true;
            for (int j=0;j<norml;j++)
            {
                if (norm[j]!=_tempNormals[i]->at(j))
                {
                    same=false;
                    break;
                }
            }
            if (same)
                return(i);
        }
    }

    return(-1); // not found
}

int CMesh::addNormalsToBufferAndReturnIndex(const std::vector<float>& norm)
{
    std::vector<float>* nnorm=new std::vector<float>;
    nnorm->assign(norm.begin(),norm.end());
    _tempNormals.push_back(nnorm);
    return((int)_tempNormals.size()-1);
}

void CMesh::getNormalsFromBufferBasedOnIndex(int index,std::vector<float>& norm)
{
    norm.assign(_tempNormals[index]->begin(),_tempNormals[index]->end());
}

int CMesh::getBufferIndexOfEdges(const std::vector<unsigned char>& edges)
{
    int edgesl=(int)edges.size();
    for (int i=0;i<int(_tempEdges.size());i++)
    {
        if (int(_tempEdges[i]->size())==edgesl)
        { // might be same!
            bool same=true;
            for (int j=0;j<edgesl;j++)
            {
                if (edges[j]!=_tempEdges[i]->at(j))
                {
                    same=false;
                    break;
                }
            }
            if (same)
                return(i);
        }
    }

    return(-1); // not found
}

int CMesh::addEdgesToBufferAndReturnIndex(const std::vector<unsigned char>& edges)
{
    std::vector<unsigned char>* nedges=new std::vector<unsigned char>;
    nedges->assign(edges.begin(),edges.end());
    _tempEdges.push_back(nedges);
    return((int)_tempEdges.size()-1);
}

void CMesh::getEdgesFromBufferBasedOnIndex(int index,std::vector<unsigned char>& edges)
{
    edges.assign(_tempEdges[index]->begin(),_tempEdges[index]->end());
}

void CMesh::serialize(CSer& ar,const char* shapeName)
{ // function has virtual/non-virtual counterpart!
    serializeWrapperInfos(ar,shapeName);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Cl0");
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            insideColor_DEPRECATED.serialize(ar,0);
            if (ar.setWritingMode())
                insideColor_DEPRECATED.serialize(ar,0);

            ar.storeDataName("Ecl");
            ar.setCountingMode();
            edgeColor_DEPRECATED.serialize(ar,1);
            if (ar.setWritingMode())
                edgeColor_DEPRECATED.serialize(ar,1);

            if (App::currentWorld->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
            { // undo/redo serialization:
                ar.storeDataName("Ver");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addVertexBuffer(_vertices,App::currentWorld->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Ind");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addIndexBuffer(_indices,App::currentWorld->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Nor");
                ar << App::currentWorld->undoBufferContainer->undoBufferArrays.addNormalsBuffer(_normals,App::currentWorld->undoBufferContainer->getNextBufferId());
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

            ar.storeDataName("Ppr");
            ar << _purePrimitive << _purePrimitiveXSizeOrDiameter << _purePrimitiveYSize << _purePrimitiveZSizeOrHeight;
            ar.flush();

            ar.storeDataName("Pp2");
            ar << _purePrimitiveInsideScaling;
            ar.flush();

            ar.storeDataName("Ppf");
            ar << _verticeLocalFrame(0) << _verticeLocalFrame(1) << _verticeLocalFrame(2) << _verticeLocalFrame(3);
            ar << _verticeLocalFrame(4) << _verticeLocalFrame(5) << _verticeLocalFrame(6);
            ar.flush();

            ar.storeDataName("Gsa"); // write this always before Gs2
            ar << _gouraudShadingAngle << _edgeWidth_DEPRERCATED;
            ar.flush();

            ar.storeDataName("Gs2");
            ar << _edgeThresholdAngle;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_visibleEdges);
            SIM_SET_CLEAR_BIT(nothing,1,_culling);
            SIM_SET_CLEAR_BIT(nothing,2,_wireframe);
            SIM_SET_CLEAR_BIT(nothing,3,_insideAndOutsideFacesSameColor_DEPRECATED);
            // RESERVED... DO NOT USE  // SIM_SET_CLEAR_BIT(nothing,4,true); // means: we do not have to make the convectivity test for this shape (was already done). Added this on 16/1/2013
            SIM_SET_CLEAR_BIT(nothing,5,true); // means: we do not have to make the convectivity test for this shape (was already done). Added this on 28/1/2013
            SIM_SET_CLEAR_BIT(nothing,6,_displayInverted_DEPRECATED);
            SIM_SET_CLEAR_BIT(nothing,7,_hideEdgeBorders);
            ar << nothing;
            ar.flush();

            if (_textureProperty!=nullptr)
            {
                ar.storeDataName("Toj");
                ar.setCountingMode();
                _textureProperty->serialize(ar);
                if (ar.setWritingMode())
                    _textureProperty->serialize(ar);
            }

            ar.storeDataName("Hfd"); // Has to come after PURE TYPE!
            ar << _heightfieldXCount << _heightfieldYCount;
            for (int i=0;i<int(_heightfieldHeights.size());i++)
                ar << _heightfieldHeights[i];
            ar.flush();

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
                    if (theName.compare("Cl0")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        insideColor_DEPRECATED.serialize(ar,0);
                    }
                    if (theName.compare("Ecl")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        edgeColor_DEPRECATED.serialize(ar,1);
                    }
                    if (App::currentWorld->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
                    { // undo/redo serialization
                        if (theName.compare("Ver")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getVertexBuffer(id,_vertices);
                        }
                        if (theName.compare("Ind")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getIndexBuffer(id,_indices);
                        }
                        if (theName.compare("Nor")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::currentWorld->undoBufferContainer->undoBufferArrays.getNormalsBuffer(id,_normals);
                        }
                    }
                    else
                    { // normal serialization
                        if (theName.compare("Ver")==0)
                        { // for backward compatibility (1/7/2014)
                            noHit=false;
                            ar >> byteQuantity;
                            _vertices.resize(byteQuantity/sizeof(float),0.0f);
                            for (int i=0;i<int(_vertices.size());i++)
                                ar >> _vertices[i];
                        }
                        if (theName.compare("Ind")==0)
                        { // for backward compatibility (1/7/2014)
                            noHit=false;
                            ar >> byteQuantity;
                            _indices.resize(byteQuantity/sizeof(int),0);
                            for (int i=0;i<int(_indices.size());i++)
                                ar >> _indices[i];
                        }
                        if (theName.compare("Nor")==0)
                        { // for backward compatibility (1/7/2014)
                            noHit=false;
                            ar >> byteQuantity;
                            _normals.resize(byteQuantity/sizeof(float),0.0f);
                            for (int i=0;i<int(_normals.size());i++)
                                ar >> _normals[i];
                        }

                        if (theName.compare("Vev")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getVerticesFromBufferBasedOnIndex(index,_vertices);
                        }
                        if (theName.compare("Inv")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getIndicesFromBufferBasedOnIndex(index,_indices);
                        }
                        if (theName.compare("Nov")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int index;
                            ar >> index;
                            getNormalsFromBufferBasedOnIndex(index,_normals);
                        }
                    }

                    if (theName.compare("In2")==0)
                    { // for backward compatibility (1/7/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        _loadPackedIntegers(ar,_indices);
                    }
                    if (theName.compare("No2")==0)
                    { // for backward compatibility (1/7/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        _normals.resize(byteQuantity*6/sizeof(float),0.0f);
                        for (int i=0;i<byteQuantity/2;i++)
                        {
                            unsigned short w;
                            ar >> w;
                            char x=(w&0x001f)-15;
                            char y=((w>>5)&0x001f)-15;
                            char z=((w>>10)&0x001f)-15;
                            C3Vector n((float)x,(float)y,(float)z);
                            n.normalize();
                            _normals[3*i+0]=n(0);
                            _normals[3*i+1]=n(1);
                            _normals[3*i+2]=n(2);
                        }
                    }
                    if (theName.compare("Ved")==0)
                    { // for backward compatibility (1/7/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        _edges.resize(byteQuantity,0);
                        for (int i=0;i<byteQuantity;i++)
                            ar >> _edges[i];
                    }
                    if (theName.compare("Vvd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int index;
                        ar >> index;
                        getEdgesFromBufferBasedOnIndex(index,_edges);
                    }
                    if (theName.compare("Ppr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _purePrimitive >> _purePrimitiveXSizeOrDiameter >> _purePrimitiveYSize >> _purePrimitiveZSizeOrHeight;
                    }
                    if (theName.compare("Pp2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _purePrimitiveInsideScaling;
                    }
                    if (theName.compare("Ppf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _verticeLocalFrame(0) >> _verticeLocalFrame(1) >> _verticeLocalFrame(2) >> _verticeLocalFrame(3);
                        ar >> _verticeLocalFrame(4) >> _verticeLocalFrame(5) >> _verticeLocalFrame(6);
                    }
                    if (theName.compare("Gsa")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _gouraudShadingAngle >> _edgeWidth_DEPRERCATED;
                        _edgeThresholdAngle=_gouraudShadingAngle;
                    }
                    if (theName.compare("Gs2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _edgeThresholdAngle;
                    }
                    if (theName=="Var")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _visibleEdges=SIM_IS_BIT_SET(nothing,0);
                        _culling=SIM_IS_BIT_SET(nothing,1);
                        _wireframe=SIM_IS_BIT_SET(nothing,2);
                        _insideAndOutsideFacesSameColor_DEPRECATED=SIM_IS_BIT_SET(nothing,3);
                        // reserved   doTheConvectivityTest=!SIM_IS_BIT_SET(nothing,4); // version 3.0.1 was buggy
                        // reserved doTheConvectivityTest=!SIM_IS_BIT_SET(nothing,5); // since version 3.0.2 (version 3.0.1 was buggy)
                        _displayInverted_DEPRECATED=SIM_IS_BIT_SET(nothing,6);
                        _hideEdgeBorders=SIM_IS_BIT_SET(nothing,7);
                        if (CSimFlavor::getBoolVal(18))
                            _visibleEdges=false;
                    }
                    if (theName.compare("Toj")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _textureProperty=new CTextureProperty();
                        _textureProperty->serialize(ar);
                    }
                    if (theName.compare("Hfd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _heightfieldXCount >> _heightfieldYCount;
                        for (int i=0;i<_heightfieldXCount*_heightfieldYCount;i++)
                        {
                            float dummy;
                            ar >> dummy;
                            _heightfieldHeights.push_back(dummy);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("shadingAngle",_gouraudShadingAngle*180.0f/piValue_f);
            ar.xmlAddNode_float("edgeThresholdAngle",_edgeThresholdAngle*180.0f/piValue_f);

            ar.xmlPushNewNode("color");
            color.serialize(ar,0);
            ar.xmlPopNode();

            ar.xmlPushNewNode("primitive");
            ar.xmlAddNode_enum("type",_purePrimitive,sim_pure_primitive_none,"none",sim_pure_primitive_plane,"plane",sim_pure_primitive_disc,"disc",sim_pure_primitive_cuboid,"cuboid",sim_pure_primitive_spheroid,"spheroid",sim_pure_primitive_cylinder,"cylinder",sim_pure_primitive_cone,"cone",sim_pure_primitive_heightfield,"heightfield");
            ar.xmlAddNode_float("insideScaling",_purePrimitiveInsideScaling);
            ar.xmlAddNode_3float("sizes",_purePrimitiveXSizeOrDiameter,_purePrimitiveYSize,_purePrimitiveZSizeOrHeight);
            ar.xmlPopNode();

            ar.xmlPushNewNode("verticesLocalFrame");
            ar.xmlAddNode_floats("position",_verticeLocalFrame.X.data,3);
            ar.xmlAddNode_floats("quaternion",_verticeLocalFrame.Q.data,4);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("edgesVisible",_visibleEdges);
            ar.xmlAddNode_bool("culling",_culling);
            ar.xmlAddNode_bool("wireframe",_wireframe);
            ar.xmlAddNode_bool("hideEdgeBorders",_hideEdgeBorders);
            ar.xmlPopNode();

            if (_textureProperty!=nullptr)
            {
                ar.xmlPushNewNode("texture");
                _textureProperty->serialize(ar);
                ar.xmlPopNode();
            }

            ar.xmlPushNewNode("heightfield");
            ar.xmlAddNode_2int("sizes",_heightfieldXCount,_heightfieldYCount);
            ar.xmlAddNode_floats("data",_heightfieldHeights);
            ar.xmlPopNode();

            ar.xmlPushNewNode("meshData");
            if (ar.xmlSaveDataInline(_vertices.size()*4+_indices.size()*4+_normals.size()*4+_edges.size()))
            {
                ar.xmlAddNode_floats("vertices",_vertices);
                ar.xmlAddNode_ints("indices",_indices);
                ar.xmlAddNode_floats("normals",_normals);
                ar.xmlAddNode_uchars("edges",_edges);
            }
            else
                ar.xmlAddNode_meshFile("file",(std::string("mesh_")+std::string(shapeName)+"_"+tt::FNb(ar.getIncrementCounter())).c_str(),&_vertices[0],(int)_vertices.size(),&_indices[0],(int)_indices.size(),&_normals[0],(int)_normals.size(),&_edges[0],(int)_edges.size());
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_float("shadingAngle",_gouraudShadingAngle);
            _gouraudShadingAngle*=piValue_f/180.0f;
            ar.xmlGetNode_float("edgeThresholdAngle",_edgeThresholdAngle);
            _edgeThresholdAngle*=piValue_f/180.0f;

            if (ar.xmlPushChildNode("color"))
            {
                color.serialize(ar,0);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("primitive"))
            {
                ar.xmlGetNode_enum("type",_purePrimitive,true,"none",sim_pure_primitive_none,"plane",sim_pure_primitive_plane,"disc",sim_pure_primitive_disc,"cuboid",sim_pure_primitive_cuboid,"spheroid",sim_pure_primitive_spheroid,"cylinder",sim_pure_primitive_cylinder,"cone",sim_pure_primitive_cone,"heightfield",sim_pure_primitive_heightfield);
                ar.xmlGetNode_float("insideScaling",_purePrimitiveInsideScaling);
                ar.xmlGetNode_3float("sizes",_purePrimitiveXSizeOrDiameter,_purePrimitiveYSize,_purePrimitiveZSizeOrHeight);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("verticesLocalFrame"))
            {
                ar.xmlGetNode_floats("position",_verticeLocalFrame.X.data,3);
                ar.xmlGetNode_floats("quaternion",_verticeLocalFrame.Q.data,4);
                _verticeLocalFrame.Q.normalize(); // just in case
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("edgesVisible",_visibleEdges);
                ar.xmlGetNode_bool("culling",_culling);
                ar.xmlGetNode_bool("wireframe",_wireframe);
                ar.xmlGetNode_bool("hideEdgeBorders",_hideEdgeBorders);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("texture",false))
            {
                _textureProperty=new CTextureProperty();
                _textureProperty->serialize(ar);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("heightfield"))
            {
                ar.xmlGetNode_2int("sizes",_heightfieldXCount,_heightfieldYCount);
                ar.xmlGetNode_floats("data",_heightfieldHeights);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("meshData"))
            {
                if (ar.xmlGetNode_floats("vertices",_vertices,false))
                {
                    ar.xmlGetNode_ints("indices",_indices);
                    ar.xmlGetNode_floats("normals",_normals);
                    ar.xmlGetNode_uchars("edges",_edges);
                }
                else
                {
                    ar.xmlGetNode_meshFile("file",_vertices,_indices,_normals,_edges);
                    actualizeGouraudShadingAndVisibleEdges();
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CMesh::display(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
//    printf("%f, %f, %f, %f\n",_transformationsSinceGrouping.Q.data[0],_transformationsSinceGrouping.Q.data[1],_transformationsSinceGrouping.Q.data[2],_transformationsSinceGrouping.Q.data[3]);
//    printf("GeomX: %f, %f, %f\n",_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C3Vector e(_verticeLocalFrame.Q.getEulerAngles());
//    printf("GeomE: %f, %f, %f\n",e(0),e(1),e(2));
    displayGeometric(this,geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,multishapeEditSelected);
}

void CMesh::display_colorCoded(CShape* geomData,int objectId,int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    displayGeometric_colorCoded(this,geomData,objectId,displayAttrib);
}


void CMesh::displayGhost(CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors)
{ // function has virtual/non-virtual counterpart!
    displayGeometricGhost(this,geomData,displayAttrib,originalColors,backfaceCulling,transparency,newColors);
}

#ifdef SIM_WITH_GUI
bool CMesh::getNonCalculatedTextureCoordinates(std::vector<float>& texCoords)
{
    if (_textureProperty==nullptr)
        return(false);
    C7Vector dummyTr;
    dummyTr.setIdentity();
    std::vector<float>* tc=_textureProperty->getTextureCoordinates(-1,dummyTr,_vertices,_indices);
    if (tc==nullptr)
        return(false);
    if (!_textureProperty->getFixedCoordinates())
        return(false);
    texCoords.assign(tc->begin(),tc->end());
    return(true);
}
#endif

