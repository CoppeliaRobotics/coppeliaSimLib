
#include "simInternal.h"
#include "textureProperty.h"
#include "tt.h"
#include "simStrings.h"
#include "app.h"
#include "rendering.h"

CTextureProperty::CTextureProperty()
{
    _commonInit();
}

CTextureProperty::CTextureProperty(int textureOrVisionSensorObjectID)
{
    _commonInit();
    _textureOrVisionSensorObjectID=textureOrVisionSensorObjectID;
}

CTextureProperty::~CTextureProperty()
{
    decreaseTexCoordBufferRefCnt(_texCoordBufferId);
}

void CTextureProperty::setTextureMapMode(int mode)
{
    if ((_textureCoordinateMode!=mode)||getFixedCoordinates())
    {
        setFixedCoordinates(nullptr);
        _calculatedTextureCoordinates.clear();
        _textureCoordinateMode=mode;
        if (mode==sim_texturemap_cylinder)
        {
            _textureScalingX=1.0;
            _repeatU=true;
        }
        if (mode==sim_texturemap_sphere)
        {
            _textureScalingX=1.0;
            _textureScalingY=1.0;
            _repeatU=true;
            _repeatV=true;
        }
        decreaseTexCoordBufferRefCnt(_texCoordBufferId);
        _texCoordBufferId=-1;
    }
}

int CTextureProperty::getTextureMapMode()
{
    return(_textureCoordinateMode);
}

void CTextureProperty::_commonInit()
{
    _startedTexture=nullptr;
    _interpolateColor=false;
    _applyMode=0; // modulate
    _repeatU=false;
    _repeatV=false;
    _textureOrVisionSensorObjectID=-1;
    _objectStateId=-1; // uninitialized
    _textureCoordinateMode=sim_texturemap_plane;

    _textureRelativeConfig.setIdentity();
    _textureScalingX=1.0f;
    _textureScalingY=1.0f;

    _texCoordBufferId=-1;
}

void CTextureProperty::setRepeatU(bool r)
{
    _repeatU=r;
    _objectStateId=-1; // Force a new calculation of texture coordinates
}

bool CTextureProperty::getRepeatU()
{
    return(_repeatU);
}

void CTextureProperty::setRepeatV(bool r)
{
    _repeatV=r;
    _objectStateId=-1; // Force a new calculation of texture coordinates
}

bool CTextureProperty::getRepeatV()
{
    return(_repeatV);
}

void CTextureProperty::addTextureDependencies(int objID,int objSubID)
{
    CTextureObject* it=App::currentWorld->textureContainer->getObject(_textureOrVisionSensorObjectID);
    if (it!=nullptr)
        it->addDependentObject(objID,objSubID);
}

void CTextureProperty::scaleObject(float scalingFactor)
{
    _objectStateId=-1; // Force a new calculation of texture coordinates

    decreaseTexCoordBufferRefCnt(_texCoordBufferId);
    _texCoordBufferId=-1;

    _textureRelativeConfig.X*=scalingFactor;
    if ((_textureCoordinateMode==sim_texturemap_plane)||(_textureCoordinateMode==sim_texturemap_cube))
    {
        _textureScalingX*=scalingFactor;
        _textureScalingY*=scalingFactor;
    }
    if (_textureCoordinateMode==sim_texturemap_cylinder)
    {
        _textureScalingY*=scalingFactor;
    }
}

void CTextureProperty::adjustForFrameChange(const C7Vector& mCorrection)
{
    _textureRelativeConfig=mCorrection*_textureRelativeConfig;
}

int* CTextureProperty::getTexCoordBufferIdPointer()
{
    return(&_texCoordBufferId);
}

void CTextureProperty::setStartedTextureObject(CTextureObject* it)
{
    _startedTexture=it;
}

CTextureObject* CTextureProperty::getStartedTextureObject()
{
    return(_startedTexture);
}

std::vector<floatFloat>* CTextureProperty::getFixedTextureCoordinates()
{
    if (_fixedTextureCoordinates.size()!=0)
        return(&_fixedTextureCoordinates);
    return(nullptr);
}

void CTextureProperty::transformToFixedTextureCoordinates(const C7Vector& transf,const std::vector<float>& vertices,const std::vector<int>& triangles)
{
    std::vector<floatFloat>* textCoords=getTextureCoordinates(-1,transf,vertices,triangles);
    setFixedCoordinates(textCoords);
}

std::vector<floatFloat>* CTextureProperty::getTextureCoordinates(int objectStateId,const C7Vector& transf,const std::vector<float>& vertices,const std::vector<int>& triangles)
{ // can return nullptr if texture needs to be destroyed!
    if (_fixedTextureCoordinates.size()!=0)
    { // We have fixed coordinates!
        if ( (triangles.size()==_fixedTextureCoordinates.size()/2) )
            return(&_fixedTextureCoordinates);
        return(nullptr); // texture needs to be destroyed
    }

    if ( (objectStateId==_objectStateId)&&(triangles.size()==_calculatedTextureCoordinates.size()/2) )
        return(&_calculatedTextureCoordinates);

    if (vertices.size()==0)
    {
        _calculatedTextureCoordinates.clear();
        return(&_calculatedTextureCoordinates);
    }

    // we need to compute the texture coordinates!

    decreaseTexCoordBufferRefCnt(_texCoordBufferId);
    _texCoordBufferId=-1;

    _calculatedTextureCoordinates.clear();
    C7Vector tr(_textureRelativeConfig.getInverse()*transf);
    CTextureObject* it=nullptr;
    if ( (_textureOrVisionSensorObjectID>=SIM_IDSTART_TEXTURE)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_TEXTURE) )
        it=App::currentWorld->textureContainer->getObject(_textureOrVisionSensorObjectID);
    else
    {
#ifdef SIM_WITH_OPENGL
        CVisionSensor* rend=App::currentWorld->sceneObjects->getVisionSensorFromHandle(_textureOrVisionSensorObjectID);
        if (rend!=nullptr)
            it=rend->getTextureObject();
#endif
    }
    float xs=1.0;
    float ys=1.0;
    if (it!=nullptr)
    {
        _objectStateId=objectStateId;
        int sizeX,sizeY;
        it->getTextureSize(sizeX,sizeY);
        if (sizeX>=sizeY)
            ys=float(sizeY)/float(sizeX);
        else
            xs=float(sizeX)/float(sizeY);
    }

    for (size_t i=0;i<triangles.size()/3;i++)
    {
        int ind3[3]={triangles[3*i+0],triangles[3*i+1],triangles[3*i+2]};
        float tc[3][2];

        C3Vector v[3]={&vertices[3*ind3[0]],&vertices[3*ind3[1]],&vertices[3*ind3[2]]};
        v[0]*=tr;
        v[1]*=tr;
        v[2]*=tr;
        float x=0.0;
        float y=0.0;

        if ((_textureCoordinateMode==sim_texturemap_cylinder)||(_textureCoordinateMode==sim_texturemap_sphere))
        {
            int verticesInCenterCnt=0;
            int verticesInCenterIndex[3];
            int verticesNotInCenterCnt=0;
            int verticesNotInCenterIndex[3];
            for (int ot3=0;ot3<3;ot3++)
            {
                if ( (fabs(v[ot3](1))<0.00001)&&(fabs(v[ot3](0))<0.00001) )
                { // this vertex is in the center!
                    verticesInCenterIndex[verticesInCenterCnt]=ot3;
                    verticesInCenterCnt++;
                }
                else
                { // this vertex is not in the center!
                    verticesNotInCenterIndex[verticesNotInCenterCnt]=ot3;
                    verticesNotInCenterCnt++;
                }
                if (_textureCoordinateMode==sim_texturemap_cylinder)
                {
                    x=((atan2(v[ot3](1),v[ot3](0))/piValue)+1.0)/2.0;// _textureScalingX doesn't make sense here!  (2.0f*_textureScalingX);
                    y=(v[ot3](2)/_textureScalingY*ys)+0.5;
                }
                if (_textureCoordinateMode==sim_texturemap_sphere)
                {
                    x=((atan2(v[ot3](1),v[ot3](0))/piValue)+1.0)/2.0; // _textureScalingX doesn't make sense here!   (2.0f*_textureScalingX);
                    float a2=C3Vector::unitZVector.getAngle(v[ot3]);
                    y=(1.0-(a2/piValue)); // _textureScalingX doesn't make sense here!  /_textureScalingY;
                }
                tc[ot3][0]=x;
                tc[ot3][1]=y;
            }

            if (verticesInCenterCnt==2)
            {
                tc[verticesInCenterIndex[0]][0]=tc[verticesNotInCenterIndex[0]][0];
                tc[verticesInCenterIndex[1]][0]=tc[verticesNotInCenterIndex[0]][0];
            }
            else
            {
                if (verticesInCenterCnt==1)
                {
                    float am=tc[verticesNotInCenterIndex[0]][0];
                    float bm=tc[verticesNotInCenterIndex[1]][0];
                    if (am>=bm)
                    {
                        if (am-bm>0.5)
                            tc[verticesInCenterIndex[0]][0]=(am+bm+1.0)*0.5;
                        else
                            tc[verticesInCenterIndex[0]][0]=(am+bm)*0.5;
                    }
                    else
                    {
                        if (bm-am>0.5)
                            tc[verticesInCenterIndex[0]][0]=(am+bm+1.0)*0.5;
                        else
                            tc[verticesInCenterIndex[0]][0]=(am+bm)*0.5;
                    }
                }
            }

            int sorted[3];
            if (tc[0][0]>=tc[1][0])
            {
                if (tc[0][0]>=tc[2][0])
                {
                    if (tc[1][0]>=tc[2][0])
                    {
                        sorted[0]=0;
                        sorted[1]=1;
                        sorted[2]=2;
                    }
                    else
                    {
                        sorted[0]=0;
                        sorted[1]=2;
                        sorted[2]=1;
                    }

                }
                else
                { // 1>2, 1<3
                    sorted[0]=2;
                    sorted[1]=0;
                    sorted[2]=1;
                }
            }
            else
            { // 1<2
                if (tc[0][0]>=tc[2][0])
                { // 1<2, 1>3
                    sorted[0]=1;
                    sorted[1]=0;
                    sorted[2]=2;
                }
                else
                {
                    if (tc[1][0]>=tc[2][0])
                    {
                        sorted[0]=1;
                        sorted[1]=2;
                        sorted[2]=0;
                    }
                    else
                    {
                        sorted[0]=2;
                        sorted[1]=1;
                        sorted[2]=0;
                    }
                }
            }
            
            if (tc[sorted[0]][0]-tc[sorted[2]][0]>0.5)
            {
                tc[sorted[2]][0]+=1.0;
                if (tc[sorted[0]][0]-tc[sorted[1]][0]>0.5)
                    tc[sorted[1]][0]+=1.0;
            }

        }
        if (_textureCoordinateMode==sim_texturemap_cube)
        { // sim_texturemap_cube
            C3Vector n((v[1]-v[0])^(v[2]-v[0]));
            n.normalize();
            int xIndex;
            int yIndex;
            if ( (fabs(n(0)))>=(fabs(n(1))) )
            { // x>y
                if ( (fabs(n(0)))>=(fabs(n(2))) )
                { // x>z
                    xIndex=1;
                    yIndex=2;
                }
                else
                { // z>x
                    xIndex=0;
                    yIndex=1;
                }
            }
            else
            { // y>x
                if ( (fabs(n(1)))>=(fabs(n(2))) )
                { // y>z
                    xIndex=2;
                    yIndex=0;
                }
                else
                { // z>y
                    xIndex=0;
                    yIndex=1;
                }
            }
            tc[0][0]=(v[0](xIndex)/(_textureScalingX*xs))+0.5;
            tc[0][1]=(v[0](yIndex)/(_textureScalingY*ys))+0.5;
            tc[1][0]=(v[1](xIndex)/(_textureScalingX*xs))+0.5;
            tc[1][1]=(v[1](yIndex)/(_textureScalingY*ys))+0.5;
            tc[2][0]=(v[2](xIndex)/(_textureScalingX*xs))+0.5;
            tc[2][1]=(v[2](yIndex)/(_textureScalingY*ys))+0.5;
        }
        if (_textureCoordinateMode==sim_texturemap_plane)
        { // sim_texturemap_plane
            tc[0][0]=(v[0](0)/(_textureScalingX*xs))+0.5;
            tc[0][1]=(v[0](1)/(_textureScalingY*ys))+0.5;
            tc[1][0]=(v[1](0)/(_textureScalingX*xs))+0.5;
            tc[1][1]=(v[1](1)/(_textureScalingY*ys))+0.5;
            tc[2][0]=(v[2](0)/(_textureScalingX*xs))+0.5;
            tc[2][1]=(v[2](1)/(_textureScalingY*ys))+0.5;
        }

        _calculatedTextureCoordinates.push_back(tc[0][0]);
        _calculatedTextureCoordinates.push_back(tc[0][1]);
        _calculatedTextureCoordinates.push_back(tc[1][0]);
        _calculatedTextureCoordinates.push_back(tc[1][1]);
        _calculatedTextureCoordinates.push_back(tc[2][0]);
        _calculatedTextureCoordinates.push_back(tc[2][1]);
    }

    return(&_calculatedTextureCoordinates);
}

void CTextureProperty::setInterpolateColors(bool ic)
{
    _interpolateColor=ic;
}

bool CTextureProperty::getInterpolateColors()
{
    return(_interpolateColor);
}

void CTextureProperty::setApplyMode(int dt)
{
    _applyMode=dt;
}

int CTextureProperty::getApplyMode()
{
    return(_applyMode);
}


CTextureProperty* CTextureProperty::copyYourself()
{
    CTextureProperty* newObj=new CTextureProperty();
    newObj->_interpolateColor=_interpolateColor;
    newObj->_applyMode=_applyMode;
    newObj->_repeatU=_repeatU;
    newObj->_repeatV=_repeatV;
    newObj->_textureOrVisionSensorObjectID=_textureOrVisionSensorObjectID;
    newObj->_textureCoordinateMode=_textureCoordinateMode;
    newObj->_textureRelativeConfig=_textureRelativeConfig;
    newObj->_textureScalingX=_textureScalingX;
    newObj->_textureScalingY=_textureScalingY;
    newObj->_fixedTextureCoordinates.assign(_fixedTextureCoordinates.begin(),_fixedTextureCoordinates.end());

    return(newObj);
}

int CTextureProperty::getTextureObjectID()
{
    return(_textureOrVisionSensorObjectID);
}

CTextureObject* CTextureProperty::getTextureObject()
{
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_TEXTURE)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_TEXTURE))
        return(App::currentWorld->textureContainer->getObject(_textureOrVisionSensorObjectID));
#ifdef SIM_WITH_OPENGL
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_SCENEOBJECT)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_SCENEOBJECT))
    {
        CVisionSensor* rs=App::currentWorld->sceneObjects->getVisionSensorFromHandle(_textureOrVisionSensorObjectID);
        if (rs!=nullptr)
            return(rs->getTextureObject());
    }
#endif
    return(nullptr);
}

bool CTextureProperty::announceObjectWillBeErased(const CSceneObject* object)
{
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_SCENEOBJECT)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_SCENEOBJECT))
        return(_textureOrVisionSensorObjectID==object->getObjectHandle());
    return(false);
}

void CTextureProperty::performObjectLoadingMapping(const std::map<int,int>* map)
{
    if (_textureOrVisionSensorObjectID<=SIM_IDEND_SCENEOBJECT)
        _textureOrVisionSensorObjectID=CWorld::getLoadingMapping(map,_textureOrVisionSensorObjectID); // texture is a vision sensor texture object
}

void CTextureProperty::performTextureObjectLoadingMapping(const std::map<int,int>* map)
{
    if (_textureOrVisionSensorObjectID>=SIM_IDSTART_TEXTURE)
        _textureOrVisionSensorObjectID=CWorld::getLoadingMapping(map,_textureOrVisionSensorObjectID); // texture is a regular texture object
}

C7Vector CTextureProperty::getTextureRelativeConfig()
{
    return(_textureRelativeConfig);
}

void CTextureProperty::setTextureRelativeConfig(const C7Vector& c)
{
    _textureRelativeConfig=c;
    _objectStateId=-1; // Force a new calculation of texture coordinates
    decreaseTexCoordBufferRefCnt(_texCoordBufferId);
    _texCoordBufferId=-1;
}

void CTextureProperty::setFixedCoordinates(const std::vector<floatFloat>* coords)
{ // nullptr to remove them and have calculated coords
    _fixedTextureCoordinates.clear();
    if ( (coords!=nullptr)&&(coords->size()!=0) )
    {
        _fixedTextureCoordinates.assign(coords->begin(),coords->end());
        _repeatU=true;
        _repeatV=true;
    }
    decreaseTexCoordBufferRefCnt(_texCoordBufferId);
    _texCoordBufferId=-1;
}

bool CTextureProperty::getFixedCoordinates()
{
    return(_fixedTextureCoordinates.size()!=0);
}

void CTextureProperty::getTextureScaling(float& x,float& y)
{
    x=_textureScalingX;
    y=_textureScalingY;
}

void CTextureProperty::setTextureScaling(float x,float y)
{
    _textureScalingX=x;
    _textureScalingY=y;
    _objectStateId=-1; // Force a new calculation of texture coordinates
}

void CTextureProperty::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Bst"); // Old, replaced by "Bs2". But keep (in that position) for backward compatibility (24/9/2014)
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_interpolateColor);
            SIM_SET_CLEAR_BIT(nothing,1,_applyMode==1);
            SIM_SET_CLEAR_BIT(nothing,2,_repeatU);
            SIM_SET_CLEAR_BIT(nothing,3,_repeatV);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Bs2");
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_interpolateColor);
            // FREE
            SIM_SET_CLEAR_BIT(nothing,2,_repeatU);
            SIM_SET_CLEAR_BIT(nothing,3,_repeatV);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Tob");
            ar << _textureOrVisionSensorObjectID << _textureCoordinateMode << _textureScalingX << _textureScalingY;
            ar.flush();

            ar.storeDataName("Trc");
            ar << _textureRelativeConfig.Q(0) << _textureRelativeConfig.Q(1) << _textureRelativeConfig.Q(2) << _textureRelativeConfig.Q(3);
            ar << _textureRelativeConfig.X(0) << _textureRelativeConfig.X(1) << _textureRelativeConfig.X(2);
            ar.flush();

            ar.storeDataName("Ftc");
            for (size_t i=0;i<_fixedTextureCoordinates.size();i++)
                ar << _fixedTextureCoordinates[i];
            ar.flush();

            ar.storeDataName("Apm");
            ar << _applyMode;
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
                    if (theName=="Bst")
                    { // Old, keep for backward compatibility (24/9/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _interpolateColor=SIM_IS_BIT_SET(nothing,0);
                        bool decal=SIM_IS_BIT_SET(nothing,1);
                        _repeatU=SIM_IS_BIT_SET(nothing,2);
                        _repeatV=SIM_IS_BIT_SET(nothing,3);
                        if (decal)
                            _applyMode=1;
                        else
                            _applyMode=0;
                    }
                    if (theName=="Bs2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _interpolateColor=SIM_IS_BIT_SET(nothing,0);
                        // FREE
                        _repeatU=SIM_IS_BIT_SET(nothing,2);
                        _repeatV=SIM_IS_BIT_SET(nothing,3);
                    }
                    if (theName.compare("Tob")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _textureOrVisionSensorObjectID >> _textureCoordinateMode >> _textureScalingX >> _textureScalingY;
                    }
                    if (theName.compare("Trc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _textureRelativeConfig.Q(0) >> _textureRelativeConfig.Q(1) >> _textureRelativeConfig.Q(2) >> _textureRelativeConfig.Q(3);
                        ar >> _textureRelativeConfig.X(0) >> _textureRelativeConfig.X(1) >> _textureRelativeConfig.X(2);
                    }

                    if (theName.compare("Ftc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _fixedTextureCoordinates.resize(byteQuantity/sizeof(floatFloat),0.0);
                        for (size_t i=0;i<_fixedTextureCoordinates.size();i++)
                            ar >> _fixedTextureCoordinates[i];
                    }
                    if (theName.compare("Apm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _applyMode;
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
            ar.xmlAddNode_int("id",_textureOrVisionSensorObjectID);
            ar.xmlAddNode_enum("applyMode",_applyMode,0,"modulate",1,"decal",2,"add");

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("interpolate",_interpolateColor);
            ar.xmlAddNode_bool("repeatU",_repeatU);
            ar.xmlAddNode_bool("repeatV",_repeatV);
            ar.xmlPopNode();

            ar.xmlPushNewNode("textureMapping");
            ar.xmlAddNode_enum("mode",_textureCoordinateMode,sim_texturemap_plane,"plane",sim_texturemap_cylinder,"cylinder",sim_texturemap_sphere,"sphere",sim_texturemap_cube,"cube");
            ar.xmlAddNode_2float("scaling",_textureScalingX,_textureScalingY);
            ar.xmlPopNode();

            ar.xmlPushNewNode("relativePose");
            ar.xmlAddNode_floats("position",_textureRelativeConfig.X.data,3);
            ar.xmlAddNode_floats("quaternion",_textureRelativeConfig.Q.data,4);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("textureCoordinates",_fixedTextureCoordinates);
        }
        else
        {
            ar.xmlGetNode_int("id",_textureOrVisionSensorObjectID);
            ar.xmlGetNode_enum("applyMode",_applyMode,true,"modulate",0,"decal",1,"add",2);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("interpolate",_interpolateColor);
                ar.xmlGetNode_bool("repeatU",_repeatU);
                ar.xmlGetNode_bool("repeatV",_repeatV);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("textureMapping"))
            {
                ar.xmlGetNode_enum("mode",_textureCoordinateMode,true,"plane",sim_texturemap_plane,"cylinder",sim_texturemap_cylinder,"sphere",sim_texturemap_sphere,"cube",sim_texturemap_cube);
                ar.xmlGetNode_2float("scaling",_textureScalingX,_textureScalingY);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("relativePose"))
            {
                ar.xmlGetNode_floats("position",_textureRelativeConfig.X.data,3);
                ar.xmlGetNode_floats("quaternion",_textureRelativeConfig.Q.data,4);
                _textureRelativeConfig.Q.normalize(); // just in case
                ar.xmlPopNode();
            }

            ar.xmlGetNode_floats("textureCoordinates",_fixedTextureCoordinates);
        }
    }
}
