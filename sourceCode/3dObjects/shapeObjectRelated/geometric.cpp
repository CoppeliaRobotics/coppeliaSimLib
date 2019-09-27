#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "geometric.h"
#include "global.h"
#include "tt.h"
#include "meshRoutines.h"
#include "meshManip.h"
#include "ttUtil.h"
#include "app.h"
#include "pluginContainer.h"
#include "shapeRendering.h"
#include "tt.h"

int CGeometric::_nextUniqueID=0;
unsigned int CGeometric::_extRendererUniqueObjectID=0;
unsigned int CGeometric::_extRendererUniqueMeshID=0;
unsigned int CGeometric::_extRendererUniqueTextureID=0;

std::vector<std::vector<float>*> CGeometric::_tempVertices;
std::vector<std::vector<int>*> CGeometric::_tempIndices;
std::vector<std::vector<float>*> CGeometric::_tempNormals;
std::vector<std::vector<unsigned char>*> CGeometric::_tempEdges;

CGeometric::CGeometric()
{
    color.setDefaultValues();
    color.setColor(0.6f+0.4f*(float)(rand()/(float)RAND_MAX),0.6f+0.4f*(float)(rand()/(float)RAND_MAX),0.6f+0.4f*(float)(rand()/(float)RAND_MAX),sim_colorcomponent_ambient_diffuse);
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

CGeometric::~CGeometric()
{
    decreaseVertexBufferRefCnt(_vertexBufferId);
    decreaseNormalBufferRefCnt(_normalBufferId);
    decreaseEdgeBufferRefCnt(_edgeBufferId);
    delete _textureProperty;
}

void CGeometric::display_extRenderer(CGeomProxy* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex)
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

        data[8]=color.colors;
        data[19]=&_gouraudShadingAngle;
        data[20]=&_extRendererObjectId;
        data[21]=&color.translucid;
        data[22]=&color.transparencyFactor;
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
        data[31]=(void*)color.colorName.c_str();
        data[32]=&shapeHandle;
        data[33]=&componentIndex;

        // Following actually free since V-REP 3.3.0
        // But the older PovRay plugin version crash without this:
        int povMaterial=0;
        data[29]=&povMaterial;

        CTextureProperty* tp=_textureProperty;
        if ((!App::ct->environment->getShapeTexturesEnabled())||CEnvironment::getShapeTexturesTemporarilyDisabled())
            tp=nullptr;
        bool textured=false;
        std::vector<float>* textureCoords=nullptr;
        if (tp!=nullptr)
        {
            textured=true;
            textureCoords=tp->getTextureCoordinates(geomData->getGeomDataModificationCounter(),_verticeLocalFrame,_vertices,_indices);
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

void CGeometric::perform3DObjectLoadingMapping(std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->performObjectLoadingMapping(map);
}

void CGeometric::performTextureObjectLoadingMapping(std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->performTextureObjectLoadingMapping(map);
}

void CGeometric::announce3DObjectWillBeErased(int objectID)
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

void CGeometric::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        _textureProperty->addTextureDependencies(shapeID,_uniqueID);
}

int CGeometric::getTextureCount()
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        return(1);
    return(0);
}

bool CGeometric::hasTextureThatUsesFixedTextureCoordinates()
{ // function has virtual/non-virtual counterpart!
    if (_textureProperty!=nullptr)
        return(_textureProperty->getFixedCoordinates());
    return(false);
}

bool CGeometric::getContainsTransparentComponents()
{ // function has virtual/non-virtual counterpart!
    return(color.translucid||insideColor_DEPRECATED.translucid);
}

CGeometric* CGeometric::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CGeometric* newIt=new CGeometric();
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

void CGeometric::scale(float xVal,float yVal,float zVal)
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

    if ((_textureProperty!=nullptr)&&(fabs(xVal-yVal)<fabs(xVal*0.01f))&&(fabs(xVal-zVal)<fabs(xVal*0.01f))) // we only scale texture prop if scaling is isometric, otherwise difficult to adjust geom size for texture!
        _textureProperty->scaleObject(xVal);

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

void CGeometric::setMeshDataDirect(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>& normals,const std::vector<unsigned char>& edges)
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

void CGeometric::setMesh(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>* normals,const C7Vector& transformation)
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

void CGeometric::setPurePrimitiveType(int theType,float xOrDiameter,float y,float zOrHeight)
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

int CGeometric::getPurePrimitiveType()
{ // function has virtual/non-virtual counterpart!
    return(_purePrimitive);
}

bool CGeometric::isGeometric()
{ // function has virtual/non-virtual counterpart!
    return(true);
}

bool CGeometric::isPure()
{ // function has virtual/non-virtual counterpart!
    return(_purePrimitive!=sim_pure_primitive_none);
}

bool CGeometric::isConvex()
{ // function has virtual/non-virtual counterpart!
    return(_convex);
}

bool CGeometric::containsOnlyPureConvexShapes()
{ // function has virtual/non-virtual counterpart!
    bool retVal=((_purePrimitive!=sim_pure_primitive_none)&&(_purePrimitive!=sim_pure_primitive_heightfield)&&(_purePrimitive!=sim_pure_primitive_plane)&&(_purePrimitive!=sim_pure_primitive_disc));
    if (retVal)
        _convex=retVal; // needed since there was a bug where pure planes and pure discs were considered as convex
    return(retVal);
}

void CGeometric::setConvex(bool convex)
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

void CGeometric::getCumulativeMeshes(std::vector<float>& vertices,std::vector<int>* indices,std::vector<float>* normals)
{ // function has virtual/non-virtual counterpart!
    int offset=(int)vertices.size()/3;
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
        for (int i=0;i<int(_indices.size());i++)
            indices->push_back(_indices[i]+offset);
    }
    if (normals!=nullptr)
    {
        C4Vector rot(_verticeLocalFrame.Q);
        for (int i=0;i<int(_normals.size())/3;i++)
        {
            C3Vector v(&_normals[3*i]);
            v=rot*v;
            normals->push_back(v(0));
            normals->push_back(v(1));
            normals->push_back(v(2));
        }
    }
}

void CGeometric::setColor(const char* colorName,int colorComponent,const float* rgbData)
{ // function has virtual/non-virtual counterpart!
    if ( (colorName==nullptr)||(color.colorName.compare(colorName)==0) )
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                color.colors[colorComponent*3+i]=rgbData[i];
        }
        if (colorComponent==4)
        {
            color.transparencyFactor=rgbData[0];
            color.translucid=(rgbData[0]<1.0f);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                color.colors[12+i]=rgbData[i];
        }
    }
    if ((colorName!=nullptr)&&(insideColor_DEPRECATED.colorName.compare(colorName)==0))
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                insideColor_DEPRECATED.colors[colorComponent*3+i]=rgbData[i];
        }
        if (colorComponent==4)
        {
            insideColor_DEPRECATED.transparencyFactor=rgbData[0];
            insideColor_DEPRECATED.translucid=(rgbData[0]<1.0f);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                insideColor_DEPRECATED.colors[12+i]=rgbData[i];
        }
    }
    if ((colorName!=nullptr)&&(edgeColor_DEPRECATED.colorName.compare(colorName)==0))
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                edgeColor_DEPRECATED.colors[colorComponent*3+i]=rgbData[i];
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                edgeColor_DEPRECATED.colors[12+i]=rgbData[i];
        }
    }


    if ( (colorName!=nullptr)&&(strlen(colorName)==2)&&(colorName[0]=='@') )
    { // operations in the HSL space
        if (colorName[1]=='0')
        { // outside color
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(color.colors+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,color.colors+colorComponent*3);
            }
            if (colorComponent==4)
            {
                color.transparencyFactor=tt::getLimitedFloat(0.0f,1.0f,color.transparencyFactor+rgbData[0]);
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(color.colors+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,color.colors+12);
            }
        }
        if (colorName[1]=='1')
        { // inside color
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.colors+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,insideColor_DEPRECATED.colors+colorComponent*3);
            }
            if (colorComponent==4)
            {
                insideColor_DEPRECATED.transparencyFactor=tt::getLimitedFloat(0.0f,1.0f,insideColor_DEPRECATED.transparencyFactor+rgbData[0]);
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(insideColor_DEPRECATED.colors+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,insideColor_DEPRECATED.colors+12);
            }
        }
        if (colorName[1]=='2')
        { // edge color
            if (colorComponent<4)
            { // regular components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.colors+colorComponent*3,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,edgeColor_DEPRECATED.colors+colorComponent*3);
            }
            if (colorComponent==5)
            { // auxiliary components
                float hsl[3];
                tt::rgbToHsl(edgeColor_DEPRECATED.colors+12,hsl);
                hsl[0]=fmod(hsl[0]+rgbData[0],1.0f);
                hsl[1]=tt::getLimitedFloat(0.0f,1.0f,hsl[1]+rgbData[1]);
                hsl[2]=tt::getLimitedFloat(0.0f,1.0f,hsl[2]+rgbData[2]);
                tt::hslToRgb(hsl,edgeColor_DEPRECATED.colors+12);
            }
        }
    }

}

bool CGeometric::getColor(const char* colorName,int colorComponent,float* rgbData)
{ // function has virtual/non-virtual counterpart!
    if ( (colorName==nullptr)||(color.colorName.compare(colorName)==0) )
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[i]=color.colors[colorComponent*3+i];
            return(true);
        }
        if (colorComponent==4)
        {
            rgbData[0]=color.transparencyFactor;
            return(true);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[i]=color.colors[12+i];
            return(true);
        }
        return(false);
    }
    if ((colorName!=nullptr)&&(insideColor_DEPRECATED.colorName.compare(colorName)==0))
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[i]=insideColor_DEPRECATED.colors[colorComponent*3+i];
            return(true);
        }
        if (colorComponent==4)
        {
            rgbData[0]=insideColor_DEPRECATED.transparencyFactor;
            return(true);
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[i]=insideColor_DEPRECATED.colors[12+i];
            return(true);
        }
        return(false);
    }
    if ((colorName!=nullptr)&&(edgeColor_DEPRECATED.colorName.compare(colorName)==0))
    {
        if (colorComponent<4)
        { // regular components
            for (int i=0;i<3;i++)
                rgbData[i]=edgeColor_DEPRECATED.colors[colorComponent*3+i];
        }
        if (colorComponent==5)
        { // auxiliary components
            for (int i=0;i<3;i++)
                rgbData[i]=edgeColor_DEPRECATED.colors[12+i];
        }
        return(false);
    }
    return(false);
}

void CGeometric::getAllShapeComponentsCumulative(std::vector<CGeometric*>& shapeComponentList)
{   // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    shapeComponentList.push_back(this);
}

CGeometric* CGeometric::getShapeComponentAtIndex(int& index)
{ // function has virtual/non-virtual counterpart!
    if (index<0)
        return(nullptr);
    if (index==0)
        return(this);
    index--;
    return(nullptr);
}

int CGeometric::getUniqueID()
{
    return(_uniqueID);
}

void CGeometric::setHeightfieldData(const std::vector<float>& heights,int xCount,int yCount)
{
    _heightfieldHeights.clear();
    _heightfieldHeights.insert(_heightfieldHeights.end(),heights.begin(),heights.end());
    _heightfieldXCount=xCount;
    _heightfieldYCount=yCount;
}

float* CGeometric::getHeightfieldData(int& xCount,int& yCount,float& minHeight,float& maxHeight)
{
    if (App::ct->dynamicsContainer->getDynamicEngineType(nullptr)==sim_physics_bullet)
        setHeightfieldDiamonds(0);
    if (App::ct->dynamicsContainer->getDynamicEngineType(nullptr)==sim_physics_ode)
        setHeightfieldDiamonds(1);
    if (App::ct->dynamicsContainer->getDynamicEngineType(nullptr)==sim_physics_vortex)
        setHeightfieldDiamonds(0);
    if (App::ct->dynamicsContainer->getDynamicEngineType(nullptr)==sim_physics_newton)
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

void CGeometric::setHeightfieldDiamonds(int d)
{ 
    if (_purePrimitive==sim_pure_primitive_heightfield)
    {
        for (int i=0;i<int(_indices.size())/6;i++)
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

void CGeometric::getPurePrimitiveSizes(C3Vector& s)
{
    s(0)=_purePrimitiveXSizeOrDiameter;
    s(1)=_purePrimitiveYSize;
    s(2)=_purePrimitiveZSizeOrHeight;
}

void CGeometric::setPurePrimitiveInsideScaling(float s)
{
    _purePrimitiveInsideScaling=tt::getLimitedFloat(0.0f,0.99999f,s);
}

float CGeometric::getPurePrimitiveInsideScaling()
{
    return(_purePrimitiveInsideScaling);
}

void CGeometric::setConvexVisualAttributes()
{
    _hideEdgeBorders=false;
    setGouraudShadingAngle(0.0f);
    setEdgeThresholdAngle(0.0f);
    setVisibleEdges(true);
    color.setConvexColors();
    edgeColor_DEPRECATED.setColorsAllBlack();
    _insideAndOutsideFacesSameColor_DEPRECATED=true;
}

CTextureProperty* CGeometric::getTextureProperty()
{
    return(_textureProperty);
}

void CGeometric::setTextureProperty(CTextureProperty* tp)
{ // careful, this doesn't check if a _textureProperty already exists! Has to be done and destroyed outside!
    _textureProperty=tp;
}

void CGeometric::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    _insideAndOutsideFacesSameColor_DEPRECATED=s;
}

bool CGeometric::getInsideAndOutsideFacesSameColor_DEPRECATED()
{
    return(_insideAndOutsideFacesSameColor_DEPRECATED);
}

bool CGeometric::getVisibleEdges()
{
    return(_visibleEdges);
}

void CGeometric::setVisibleEdges(bool v)
{
    _visibleEdges=v;
}

void CGeometric::setHideEdgeBorders(bool v)
{
    if (_hideEdgeBorders!=v)
    {
        _hideEdgeBorders=v;
        _computeVisibleEdges();
    }
}

bool CGeometric::getHideEdgeBorders()
{
    return(_hideEdgeBorders);
}

int CGeometric::getEdgeWidth_DEPRECATED()
{
    return(_edgeWidth_DEPRERCATED);
}

void CGeometric::setEdgeWidth_DEPRECATED(int w)
{
    w=tt::getLimitedInt(1,4,w);
    _edgeWidth_DEPRERCATED=w;
}

bool CGeometric::getCulling()
{
    return(_culling);
}

void CGeometric::setCulling(bool c)
{
    _culling=c;
}

bool CGeometric::getDisplayInverted_DEPRECATED()
{
    return(_displayInverted_DEPRECATED);
}

void CGeometric::setDisplayInverted_DEPRECATED(bool di)
{
    _displayInverted_DEPRECATED=di;
}

void CGeometric::copyVisualAttributesTo(CGeometric* target)
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

float CGeometric::getGouraudShadingAngle()
{ // function has virtual/non-virtual counterpart!
    return(_gouraudShadingAngle);
}

void CGeometric::setGouraudShadingAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0f,89.0f*degToRad_f,angle);
    if (_gouraudShadingAngle!=angle)
    {
        _gouraudShadingAngle=angle;
        _recomputeNormals();
    }
}

float CGeometric::getEdgeThresholdAngle()
{ // function has virtual/non-virtual counterpart!
    return(_edgeThresholdAngle);
}

void CGeometric::setEdgeThresholdAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    tt::limitValue(0.0f,89.0f*degToRad_f,angle);
    if (_edgeThresholdAngle!=angle)
    {
        _edgeThresholdAngle=angle;
        _computeVisibleEdges();
    }
}

void CGeometric::setWireframe(bool w)
{
    _wireframe=w;
}

bool CGeometric::getWireframe()
{
    return(_wireframe);
}

C7Vector CGeometric::getVerticeLocalFrame()
{
    return(_verticeLocalFrame);
}

void CGeometric::setVerticeLocalFrame(const C7Vector& tr)
{
    _verticeLocalFrame=tr;
}

std::vector<float>* CGeometric::getVertices()
{
    return(&_vertices);
}

std::vector<int>* CGeometric::getIndices()
{
    return(&_indices);
}

std::vector<float>* CGeometric::getNormals()
{
    return(&_normals);
}

std::vector<unsigned char>* CGeometric::getEdges()
{
    return(&_edges);
}

int* CGeometric::getVertexBufferIdPtr()
{
    return(&_vertexBufferId);
}

int* CGeometric::getNormalBufferIdPtr()
{
    return(&_normalBufferId);
}

int* CGeometric::getEdgeBufferIdPtr()
{
    return(&_edgeBufferId);
}


void CGeometric::preMultiplyAllVerticeLocalFrames(const C7Vector& preTr)
{ // function has virtual/non-virtual counterpart!

    _transformationsSinceGrouping=preTr*_transformationsSinceGrouping;
    _localInertiaFrame=preTr*_localInertiaFrame;

    _verticeLocalFrame=preTr*_verticeLocalFrame;

    if (_textureProperty!=nullptr)
        _textureProperty->adjustForFrameChange(preTr);
}

void CGeometric::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    delete _textureProperty;
    _textureProperty=nullptr;
}

void CGeometric::getColorStrings(std::string& colorStrings)
{ // function has virtual/non-virtual counterpart!
    if ( (color.colorName.length()>0)&&(colorStrings.find(color.colorName)==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=color.colorName;
    }
    if ( (insideColor_DEPRECATED.colorName.length()>0)&&(colorStrings.find(insideColor_DEPRECATED.colorName)==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=insideColor_DEPRECATED.colorName;
    }
    if ( (edgeColor_DEPRECATED.colorName.length()>0)&&(colorStrings.find(edgeColor_DEPRECATED.colorName)==std::string::npos) )
    {
        if (colorStrings.length()!=0)
            colorStrings+=" ";
        colorStrings+=edgeColor_DEPRECATED.colorName;
    }
}

void CGeometric::flipFaces()
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

void CGeometric::actualizeGouraudShadingAndVisibleEdges()
{
    _recomputeNormals();
    _computeVisibleEdges();
}

void CGeometric::_recomputeNormals()
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

void CGeometric::_computeVisibleEdges()
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


bool CGeometric::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex=CMeshRoutines::checkIfConvex(_vertices,_indices,0.015f); // 1.5% tolerance of the average bounding box side length
    setConvex(_convex);
    return(_convex);
}


void CGeometric::_savePackedIntegers(CSer& ar,const std::vector<int>& data)
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

void CGeometric::_loadPackedIntegers(CSer& ar,std::vector<int>& data)
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

void CGeometric::clearTempVerticesIndicesNormalsAndEdges()
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

void CGeometric::prepareVerticesIndicesNormalsAndEdgesForSerialization()
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

void CGeometric::serializeTempVerticesIndicesNormalsAndEdges(CSer& ar)
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

int CGeometric::getBufferIndexOfVertices(const std::vector<float>& vert)
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

int CGeometric::addVerticesToBufferAndReturnIndex(const std::vector<float>& vert)
{
    std::vector<float>* nvert=new std::vector<float>;
    nvert->assign(vert.begin(),vert.end());
    _tempVertices.push_back(nvert);
    return((int)_tempVertices.size()-1);
}

void CGeometric::getVerticesFromBufferBasedOnIndex(int index,std::vector<float>& vert)
{
    vert.assign(_tempVertices[index]->begin(),_tempVertices[index]->end());
}



int CGeometric::getBufferIndexOfIndices(const std::vector<int>& ind)
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

int CGeometric::addIndicesToBufferAndReturnIndex(const std::vector<int>& ind)
{
    std::vector<int>* nind=new std::vector<int>;
    nind->assign(ind.begin(),ind.end());
    _tempIndices.push_back(nind);
    return((int)_tempIndices.size()-1);
}

void CGeometric::getIndicesFromBufferBasedOnIndex(int index,std::vector<int>& ind)
{
    ind.assign(_tempIndices[index]->begin(),_tempIndices[index]->end());
}

int CGeometric::getBufferIndexOfNormals(const std::vector<float>& norm)
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

int CGeometric::addNormalsToBufferAndReturnIndex(const std::vector<float>& norm)
{
    std::vector<float>* nnorm=new std::vector<float>;
    nnorm->assign(norm.begin(),norm.end());
    _tempNormals.push_back(nnorm);
    return((int)_tempNormals.size()-1);
}

void CGeometric::getNormalsFromBufferBasedOnIndex(int index,std::vector<float>& norm)
{
    norm.assign(_tempNormals[index]->begin(),_tempNormals[index]->end());
}

int CGeometric::getBufferIndexOfEdges(const std::vector<unsigned char>& edges)
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

int CGeometric::addEdgesToBufferAndReturnIndex(const std::vector<unsigned char>& edges)
{
    std::vector<unsigned char>* nedges=new std::vector<unsigned char>;
    nedges->assign(edges.begin(),edges.end());
    _tempEdges.push_back(nedges);
    return((int)_tempEdges.size()-1);
}

void CGeometric::getEdgesFromBufferBasedOnIndex(int index,std::vector<unsigned char>& edges)
{
    edges.assign(_tempEdges[index]->begin(),_tempEdges[index]->end());
}

void CGeometric::serialize(CSer& ar,const char* shapeName)
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

            if (App::ct->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
            { // undo/redo serialization:
                ar.storeDataName("Ver");
                ar << App::ct->undoBufferContainer->undoBufferArrays.addVertexBuffer(_vertices,App::ct->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Ind");
                ar << App::ct->undoBufferContainer->undoBufferArrays.addIndexBuffer(_indices,App::ct->undoBufferContainer->getNextBufferId());
                ar.flush();

                ar.storeDataName("Nor");
                ar << App::ct->undoBufferContainer->undoBufferArrays.addNormalsBuffer(_normals,App::ct->undoBufferContainer->getNextBufferId());
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
                    if (App::ct->undoBufferContainer->isUndoSavingOrRestoringUnderWay())
                    { // undo/redo serialization
                        if (theName.compare("Ver")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::ct->undoBufferContainer->undoBufferArrays.getVertexBuffer(id,_vertices);
                        }
                        if (theName.compare("Ind")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::ct->undoBufferContainer->undoBufferArrays.getIndexBuffer(id,_indices);
                        }
                        if (theName.compare("Nor")==0)
                        {
                            noHit=false;
                            ar >> byteQuantity;
                            int id;
                            ar >> id;
                            App::ct->undoBufferContainer->undoBufferArrays.getNormalsBuffer(id,_normals);
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
}

void CGeometric::display(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
//    printf("%f, %f, %f, %f\n",_transformationsSinceGrouping.Q.data[0],_transformationsSinceGrouping.Q.data[1],_transformationsSinceGrouping.Q.data[2],_transformationsSinceGrouping.Q.data[3]);
    displayGeometric(this,geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,multishapeEditSelected);
}

void CGeometric::display_colorCoded(CGeomProxy* geomData,int objectId,int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    displayGeometric_colorCoded(this,geomData,objectId,displayAttrib);
}

void CGeometric::displayForCutting(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,const float normalVectorForPointsAndLines[3])
{ // function has virtual/non-virtual counterpart!
    C_API_FUNCTION_DEBUG;
    CPluginContainer::mesh_lockUnlock(true); // to avoid accessing invalid data! Do not forget to unlock!!
    displayGeometricForCutting(this,geomData,displayAttrib,collisionColor,normalVectorForPointsAndLines);
    CPluginContainer::mesh_lockUnlock(false);
}

void CGeometric::displayGhost(CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors)
{ // function has virtual/non-virtual counterpart!
    displayGeometricGhost(this,geomData,displayAttrib,originalColors,backfaceCulling,transparency,newColors);
}

#ifdef SIM_WITH_GUI
bool CGeometric::getNonCalculatedTextureCoordinates(std::vector<float>& texCoords)
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

