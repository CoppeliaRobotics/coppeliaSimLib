#include "simInternal.h"
#include "meshWrapper.h"
#include "mesh.h"
#include "tt.h"
#include "global.h"
#include "viewableBase.h"
#include "app.h"
#include <Eigen/Eigenvalues>
#include <iostream>

CMeshWrapper::CMeshWrapper()
{
    _mass=1.0f;
    _name="sub__0";

    _dynMaterialId_old=-1; // not used anymore since V3.4.0

    _convex=false;

    setDefaultInertiaParams();

    _transformationsSinceGrouping.setIdentity();
}

CMeshWrapper::~CMeshWrapper()
{
    for (size_t i=0;i<childList.size();i++)
        delete childList[i];
}

C7Vector CMeshWrapper::getTransformationsSinceGrouping() const
{
    return(_transformationsSinceGrouping);
}

void CMeshWrapper::setTransformationsSinceGrouping(const C7Vector& tr)
{
    _transformationsSinceGrouping=tr;
}

void CMeshWrapper::display(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display(geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,multishapeEditSelected);
}

void CMeshWrapper::display_extRenderer(CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display_extRenderer(geomData,displayAttrib,tr,shapeHandle,componentIndex);
}

void CMeshWrapper::display_colorCoded(CShape* geomData,int objectId,int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display_colorCoded(geomData,objectId,displayAttrib);
}

void CMeshWrapper::displayGhost(CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,floatDouble transparency,const floatDouble* newColors)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->displayGhost(geomData,displayAttrib,originalColors,backfaceCulling,transparency,newColors);
}

void CMeshWrapper::performSceneObjectLoadingMapping(const std::map<int,int>* map)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->performSceneObjectLoadingMapping(map);
}

void CMeshWrapper::performTextureObjectLoadingMapping(const std::map<int,int>* map)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->performTextureObjectLoadingMapping(map);
}

void CMeshWrapper::performDynMaterialObjectLoadingMapping(const std::map<int,int>* map)
{
    _dynMaterialId_old=CWorld::getLoadingMapping(map,_dynMaterialId_old);
}

void CMeshWrapper::announceSceneObjectWillBeErased(const CSceneObject* object)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->announceSceneObjectWillBeErased(object);
}

void CMeshWrapper::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setTextureDependencies(shapeID);
}

int CMeshWrapper::getTextureCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal=0;
    for (size_t i=0;i<childList.size();i++)
        retVal+=childList[i]->getTextureCount();
    return(retVal);
}

bool CMeshWrapper::hasTextureThatUsesFixedTextureCoordinates() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
    {
        if (childList[i]->hasTextureThatUsesFixedTextureCoordinates())
            return(true);
    }
    return(false);
}

void CMeshWrapper::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->removeAllTextures();
}

void CMeshWrapper::getColorStrings(std::string& colorStrings) const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getColorStrings(colorStrings);
}

bool CMeshWrapper::getContainsTransparentComponents() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
    {
        if (childList[i]->getContainsTransparentComponents())
            return(true);
    }
    return(false);
}

floatDouble CMeshWrapper::getShadingAngle() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getShadingAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setShadingAngle(floatDouble angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setShadingAngle(angle);
}

floatDouble CMeshWrapper::getEdgeThresholdAngle() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getEdgeThresholdAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setEdgeThresholdAngle(floatDouble angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setEdgeThresholdAngle(angle);
}

void CMeshWrapper::setHideEdgeBorders_OLD(bool v)
{  // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setHideEdgeBorders_OLD(v);
}

bool CMeshWrapper::getHideEdgeBorders_OLD() const
{  // function has virtual/non-virtual counterpart!
    return(childList[0]->getHideEdgeBorders_OLD()); // we just return the first setting we encounter! Normally never used
}

CMeshWrapper* CMeshWrapper::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CMeshWrapper* newIt=new CMeshWrapper();
    copyWrapperInfos(newIt);
    return(newIt);
}

void CMeshWrapper::copyWrapperInfos(CMeshWrapper* target)
{
    target->_mass=_mass;
    target->_name=_name;
    target->_convex=_convex;

    target->_localInertiaFrame=_localInertiaFrame;
    target->_principalMomentsOfInertia=_principalMomentsOfInertia;

    target->_transformationsSinceGrouping=_transformationsSinceGrouping;

    target->_dynMaterialId_old=_dynMaterialId_old;

    for (size_t i=0;i<childList.size();i++)
        target->childList.push_back(childList[i]->copyYourself());
}

void CMeshWrapper::setMass(floatDouble m)
{
    _mass=tt::getLimitedFloat(0.000000001f,100000.0f,m);
}

floatDouble CMeshWrapper::getMass() const
{
    return(_mass);
}

void CMeshWrapper::setDefaultInertiaParams()
{
    _localInertiaFrame.setIdentity();
    _principalMomentsOfInertia(0)=0.001f;
    _principalMomentsOfInertia(1)=0.001f;
    _principalMomentsOfInertia(2)=0.001f;
}

void CMeshWrapper::setName(std::string newName)
{
    _name=newName;
}

std::string CMeshWrapper::getName() const
{
    return(_name);
}

int CMeshWrapper::getDynMaterialId_old() const
{
    return(_dynMaterialId_old);
}

void CMeshWrapper::setDynMaterialId_old(int id)
{
    _dynMaterialId_old=id;
}

C7Vector CMeshWrapper::getLocalInertiaFrame() const
{
    return (_localInertiaFrame);
}

void CMeshWrapper::setLocalInertiaFrame(const C7Vector& li)
{
    _localInertiaFrame=li;
}

C3Vector CMeshWrapper::getPrincipalMomentsOfInertia() const
{
    return (_principalMomentsOfInertia);
}

void CMeshWrapper::setPrincipalMomentsOfInertia(const C3Vector& inertia)
{ // massless inertia
    _principalMomentsOfInertia=inertia;
    _principalMomentsOfInertia(0)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(0));
    _principalMomentsOfInertia(1)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(1));
    _principalMomentsOfInertia(2)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(2));
    if (_principalMomentsOfInertia.getLength()==0.0f)
        _principalMomentsOfInertia(0)=0.001f; // make sure we don't have a zero vector (problems with Bullet? and CoppeliaSim!)
}

void CMeshWrapper::scale(floatDouble xVal,floatDouble yVal,floatDouble zVal)
{ // function has virtual/non-virtual counterpart!
    // iso-scaling for compound shapes!! (should normally already be xVal=yVal=zVal)
    scaleWrapperInfos(xVal,xVal,xVal);
}

void CMeshWrapper::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CMeshWrapper::scaleWrapperInfos(floatDouble xVal,floatDouble yVal,floatDouble zVal)
{
    scaleMassAndInertia(xVal,yVal,zVal);

    _localInertiaFrame.X(0)*=xVal;
    _localInertiaFrame.X(1)*=yVal;
    _localInertiaFrame.X(2)*=zVal;

    _transformationsSinceGrouping.X(0)*=xVal;
    _transformationsSinceGrouping.X(1)*=yVal;
    _transformationsSinceGrouping.X(2)*=zVal;

    for (size_t i=0;i<childList.size();i++)
        childList[i]->scale(xVal,yVal,zVal);
    if ((xVal<0.0f)||(yVal<0.0f)||(zVal<0.0f)) // that effectively flips faces!
        checkIfConvex();
}

void CMeshWrapper::scaleMassAndInertia(floatDouble xVal,floatDouble yVal,floatDouble zVal)
{
    _mass*=xVal*yVal*zVal;
    _principalMomentsOfInertia(0)*=yVal*zVal;
    _principalMomentsOfInertia(1)*=xVal*zVal;
    _principalMomentsOfInertia(2)*=xVal*yVal;
}

void CMeshWrapper::setPurePrimitiveType(int theType,floatDouble xOrDiameter,floatDouble y,floatDouble zOrHeight)
{ // function has virtual/non-virtual counterpart!
    // Following added on 14/03/2011 because a compound shape composed by pure and non pure shapes would decompose as pure shapes with wrong orientation!
    if (theType==sim_primitiveshape_none)
    {
        for (size_t i=0;i<childList.size();i++)
            childList[i]->setPurePrimitiveType(theType,xOrDiameter,y,zOrHeight);
        // _convex=false;  NO!!!
    }
}

int CMeshWrapper::getPurePrimitiveType() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getPurePrimitiveType()); // we just return the first type we encounter! Normally never used
}

bool CMeshWrapper::isMesh() const
{ // function has virtual/non-virtual counterpart!
    return(false);
}

bool CMeshWrapper::isPure() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->isPure());
}

bool CMeshWrapper::isConvex() const
{ // function has virtual/non-virtual counterpart!
    return(_convex);
}

bool CMeshWrapper::containsOnlyPureConvexShapes()
{ // function has virtual/non-virtual counterpart!
    bool retVal=true;
    for (size_t i=0;i<childList.size();i++)
        retVal=retVal&&childList[i]->containsOnlyPureConvexShapes();
    if (retVal)
        _convex=retVal; // needed since there was a bug where pure planes and pure discs were considered as convex
    return(retVal);
}

bool CMeshWrapper::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex=true;
    for (size_t i=0;i<childList.size();i++)
        _convex=_convex&&childList[i]->checkIfConvex();
    setConvex(_convex);
    return(_convex);
}

void CMeshWrapper::setConvex(bool convex)
{ // function has virtual/non-virtual counterpart!
    _convex=convex; // This is just for the wrapper!
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

void CMeshWrapper::getCumulativeMeshes(std::vector<floatDouble>& vertices,std::vector<int>* indices,std::vector<floatDouble>* normals)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getCumulativeMeshes(vertices,indices,normals);
}

void CMeshWrapper::setColor(const CShape* shape,int& elementIndex,const char* colorName,int colorComponent,const floatDouble* rgbData,int& rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setColor(shape,elementIndex,colorName,colorComponent,rgbData,rgbDataOffset);
}

bool CMeshWrapper::getColor(const char* colorName,int colorComponent,floatDouble* rgbData,int& rgbDataOffset) const
{ // function has virtual/non-virtual counterpart!
    bool retVal=false;
    for (size_t i=0;i<childList.size();i++)
        retVal=childList[i]->getColor(colorName,colorComponent,rgbData,rgbDataOffset)||retVal;
    return(retVal);
}

void CMeshWrapper::getAllShapeComponentsCumulative(std::vector<CMesh*>& shapeComponentList)
{   // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getAllShapeComponentsCumulative(shapeComponentList);
}

CMesh* CMeshWrapper::getShapeComponentAtIndex(int& index)
{ // function has virtual/non-virtual counterpart!
    if (index<0)
        return(nullptr);
    for (size_t i=0;i<childList.size();i++)
    {
        CMesh* retVal=childList[i]->getShapeComponentAtIndex(index);
        if (retVal!=nullptr)
            return(retVal);
        if (index<0)
            return(nullptr);
    }
    return(nullptr);
}

int CMeshWrapper::getComponentCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal=0;
    for (size_t i=0;i<childList.size();i++)
        retVal+=childList[i]->getComponentCount();
    return(retVal);
}

void CMeshWrapper::preMultiplyAllVerticeLocalFrames(const C7Vector& preTr)
{ // function has virtual/non-virtual counterpart!

    _transformationsSinceGrouping=preTr*_transformationsSinceGrouping;
    _localInertiaFrame=preTr*_localInertiaFrame;

    for (size_t i=0;i<childList.size();i++)
        childList[i]->preMultiplyAllVerticeLocalFrames(preTr);
}

void CMeshWrapper::flipFaces()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->flipFaces();
    checkIfConvex();
}

void CMeshWrapper::serialize(CSer& ar,const char* shapeName)
{ // function has virtual/non-virtual counterpart!
    serializeWrapperInfos(ar,shapeName);
}

void CMeshWrapper::serializeWrapperInfos(CSer& ar,const char* shapeName)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << _name;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Mas");
            ar.flt() << (floatFloat)_mass;
            ar.flush();
#endif
#ifdef DOUBLESERIALIZATIONOPERATION
            ar.storeDataName("_as");
            ar.dbl() << _mass;
            ar.flush();
#endif

            ar.storeDataName("Dmi");
            ar << _dynMaterialId_old;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Ine");
            ar.flt() << (floatFloat)_localInertiaFrame(0) << (floatFloat)_localInertiaFrame(1) << (floatFloat)_localInertiaFrame(2) << (floatFloat)_localInertiaFrame(3);
            ar.flt() << (floatFloat)_localInertiaFrame(4) << (floatFloat)_localInertiaFrame(5) << (floatFloat)_localInertiaFrame(6);
            ar.flt() << (floatFloat)_principalMomentsOfInertia(0) << (floatFloat)_principalMomentsOfInertia(1) << (floatFloat)_principalMomentsOfInertia(2);
            ar.flush();
#endif
#ifdef DOUBLESERIALIZATIONOPERATION
            ar.storeDataName("_ne");
            ar.dbl() << _localInertiaFrame(0) << _localInertiaFrame(1) << _localInertiaFrame(2) << _localInertiaFrame(3);
            ar.dbl() << _localInertiaFrame(4) << _localInertiaFrame(5) << _localInertiaFrame(6);
            ar.dbl() << _principalMomentsOfInertia(0) << _principalMomentsOfInertia(1) << _principalMomentsOfInertia(2);
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Vtb");
            ar.flt() << (floatFloat)_transformationsSinceGrouping(0) << (floatFloat)_transformationsSinceGrouping(1) << (floatFloat)_transformationsSinceGrouping(2) << (floatFloat)_transformationsSinceGrouping(3);
            ar.flt() << (floatFloat)_transformationsSinceGrouping(4) << (floatFloat)_transformationsSinceGrouping(5) << (floatFloat)_transformationsSinceGrouping(6);
            ar.flush();
#endif
#ifdef DOUBLESERIALIZATIONOPERATION
            ar.storeDataName("_tb");
            ar.dbl() << _transformationsSinceGrouping(0) << _transformationsSinceGrouping(1) << _transformationsSinceGrouping(2) << _transformationsSinceGrouping(3);
            ar.dbl() << _transformationsSinceGrouping(4) << _transformationsSinceGrouping(5) << _transformationsSinceGrouping(6);
            ar.flush();
#endif

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,2,_convex);
            SIM_SET_CLEAR_BIT(nothing,5,true); // means: we do not have to make the convectivity test for this shape (was already done). Added this on 28/1/2013
            ar << nothing;
            ar.flush();

            for (size_t i=0;i<childList.size();i++)
            {
                if (childList[i]->isMesh())
                    ar.storeDataName("Geo");
                else
                    ar.storeDataName("Wrp");
                ar.setCountingMode();
                childList[i]->serialize(ar,shapeName);
                if (ar.setWritingMode())
                    childList[i]->serialize(ar,shapeName);
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
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _name;
                    }
                    if (theName.compare("Mas")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla;
                        ar.flt() >> bla;
                        _mass=(floatDouble)bla;
                        if (_mass==0.0f) // to catch an old bug
                            _mass=0.001;
                    }
                    if (theName.compare("_as")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _mass;
                        if (_mass==0.0f) // to catch an old bug
                            _mass=0.001;
                    }
                    if (theName.compare("Dmi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynMaterialId_old;
                    }
                    if (theName.compare("Ine")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla;
                        for (size_t i=0;i<7;i++)
                        {
                            ar.flt() >> bla;
                            _localInertiaFrame(i)=(floatDouble)bla;
                        }
                        for (size_t i=0;i<3;i++)
                        {
                            ar.flt() >> bla;
                            _principalMomentsOfInertia(i)=(floatDouble)bla;
                        }
                    }
                    if (theName.compare("_ne")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _localInertiaFrame(0) >> _localInertiaFrame(1) >> _localInertiaFrame(2) >> _localInertiaFrame(3);
                        ar.dbl() >> _localInertiaFrame(4) >> _localInertiaFrame(5) >> _localInertiaFrame(6);
                        ar.dbl() >> _principalMomentsOfInertia(0) >> _principalMomentsOfInertia(1) >> _principalMomentsOfInertia(2);
                    }

                    if (theName.compare("Vtb")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat bla;
                        for (size_t i=0;i<7;i++)
                        {
                            ar.flt() >> bla;
                            _transformationsSinceGrouping(i)=(floatDouble)bla;
                        }
                    }
                    if (theName.compare("_tb")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _transformationsSinceGrouping(0) >> _transformationsSinceGrouping(1) >> _transformationsSinceGrouping(2) >> _transformationsSinceGrouping(3);
                        ar.dbl() >> _transformationsSinceGrouping(4) >> _transformationsSinceGrouping(5) >> _transformationsSinceGrouping(6);
                    }

                    if (theName=="Var")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _convex=SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName.compare("Geo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CMesh* it=new CMesh();
                        it->serialize(ar,shapeName);
                        childList.push_back(it);
                    }
                    if (theName.compare("Wrp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CMeshWrapper* it=new CMeshWrapper();
                        it->serialize(ar,shapeName);
                        childList.push_back(it);
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
            ar.xmlPushNewNode("common");

            ar.xmlAddNode_string("name",_name.c_str());

            ar.xmlPushNewNode("dynamics");
            ar.xmlAddNode_float("mass",_mass);
            ar.xmlPushNewNode("localInertiaFrame");
            ar.xmlAddNode_floats("position",_localInertiaFrame.X.data,3);
            ar.xmlAddNode_floats("quaternion",_localInertiaFrame.Q.data,4);
            ar.xmlAddNode_floats("principalMomentsOfInertia",_principalMomentsOfInertia.data,3);
            ar.xmlPopNode();
            ar.xmlPopNode();

            ar.xmlPushNewNode("transformationSinceGrouping");
            ar.xmlAddNode_floats("position",_transformationsSinceGrouping.X.data,3);
            ar.xmlAddNode_floats("quaternion",_transformationsSinceGrouping.Q.data,4);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("convex",_convex);
            ar.xmlPopNode();

            for (size_t i=0;i<childList.size();i++)
            {
                ar.xmlPushNewNode("child");
                if (childList[i]->isMesh())
                    ar.xmlPushNewNode("mesh");
                else
                    ar.xmlPushNewNode("compound");
                childList[i]->serialize(ar,shapeName);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        else
        {
            if (ar.xmlPushChildNode("common"))
            {
                ar.xmlGetNode_string("name",_name);

                if (ar.xmlPushChildNode("dynamics"))
                {
                    ar.xmlGetNode_float("mass",_mass);
                    if (ar.xmlPushChildNode("localInertiaFrame"))
                    {
                        ar.xmlGetNode_floats("position",_localInertiaFrame.X.data,3);
                        ar.xmlGetNode_floats("quaternion",_localInertiaFrame.Q.data,4);
                        _localInertiaFrame.Q.normalize(); // just in case
                        ar.xmlGetNode_floats("principalMomentsOfInertia",_principalMomentsOfInertia.data,3);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("transformationSinceGrouping"))
                {
                    ar.xmlGetNode_floats("position",_transformationsSinceGrouping.X.data,3);
                    ar.xmlGetNode_floats("quaternion",_transformationsSinceGrouping.Q.data,4);
                    _transformationsSinceGrouping.Q.normalize(); // just in case
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("switches"))
                {
                    ar.xmlGetNode_bool("convex",_convex);
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("child",false))
                {
                    while (true)
                    {
                        if (ar.xmlPushChildNode("mesh",false))
                        {
                            CMesh* it=new CMesh();
                            it->serialize(ar,shapeName);
                            childList.push_back(it);
                            ar.xmlPopNode();
                        }
                        else
                        {
                            if (ar.xmlPushChildNode("compound"))
                            {
                                CMeshWrapper* it=new CMeshWrapper();
                                it->serialize(ar,shapeName);
                                childList.push_back(it);
                                ar.xmlPopNode();
                            }
                        }
                        if (!ar.xmlPushSiblingNode("child",false))
                            break;
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
        }
    }
}


void CMeshWrapper::findPrincipalMomentOfInertia(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments)
{
    //*
    Eigen::Matrix3f m;
    m(0,0)=tensor(0,0);
    m(1,1)=tensor(1,1);
    m(2,2)=tensor(2,2);
    m(0,1)=tensor(0,1);
    m(1,0)=tensor(0,1);
    m(0,2)=tensor(0,2);
    m(2,0)=tensor(0,2);
    m(1,2)=tensor(1,2);
    m(2,1)=tensor(1,2);
    Eigen::EigenSolver<Eigen::Matrix3f> es(m);
    Eigen::Vector3cf eigenVals=es.eigenvalues();
    principalMoments.setData(eigenVals(0).real(),eigenVals(1).real(),eigenVals(2).real());
    Eigen::Vector3cf eigenVect1=es.eigenvectors().col(0);
    C3Vector eVect1(eigenVect1(0).real(),eigenVect1(1).real(),eigenVect1(2).real());
    Eigen::Vector3cf eigenVect2=es.eigenvectors().col(1);
    C3Vector eVect2(eigenVect2(0).real(),eigenVect2(1).real(),eigenVect2(2).real());
    Eigen::Vector3cf eigenVect3=es.eigenvectors().col(2);
    C3Vector eVect3(eigenVect3(0).real(),eigenVect3(1).real(),eigenVect3(2).real());
    C3X3Matrix matr;
    matr.axis[0]=eVect1;
    matr.axis[1]=eVect2;
    matr.axis[2]=eVect3;
    rotation=matr.getQuaternion();
    //*/
    /*
    // This routine is iterative and not elegant. But we do not need speed here anyway
    C3X3Matrix rot;
    C3X3Matrix tens(tensor);
    rot.setIdentity();
    floatDouble w=_getTensorNonDiagonalMeasure(tensor);

    // With below settings, we will get an orientational precision of about 0.000128 degrees
    floatDouble stepSize=10.0f*degToRad;
    const floatDouble stepSizeMultiplier=0.2f;
    const int resolutionScalePasses=8;

    C3X3Matrix drot;
    for (int scale=0;scale<resolutionScalePasses;scale++)
    { // resolution loop
        for (int axis=0;axis<3;axis++)
        { // axis loop (x, y, z)
            if (axis==0)
                drot.buildXRotation(stepSize);
            if (axis==1)
                drot.buildYRotation(stepSize);
            if (axis==2)
                drot.buildZRotation(stepSize);
            int dirChangeCnt=0;
            while (dirChangeCnt<2)
            { // error reduction loop for a given axis
                rot=rot*drot;
                tens=drot.getTranspose()*tens*drot;
                floatDouble w2=_getTensorNonDiagonalMeasure(tens);
                if (w2>=w)
                { // it got worse
                    dirChangeCnt++;
                    drot.transpose();
                    rot=rot*drot; // reverse change
                    tens=drot.getTranspose()*tens*drot;
                }
                else
                    w=w2; // it got better
            }
        }
        stepSize*=stepSizeMultiplier;
    }

    principalMoments(0)=tens.axis[0](0);
    principalMoments(1)=tens.axis[1](1);
    principalMoments(2)=tens.axis[2](2);
    rotation=rot.getQuaternion();
    //*/
}

floatDouble CMeshWrapper::_getTensorNonDiagonalMeasure(const C3X3Matrix& tensor)
{
    C3Vector v(tensor.axis[1](0),tensor.axis[2](0),tensor.axis[2](1));
    return(v*v);
}

C3X3Matrix CMeshWrapper::getNewTensor(const C3Vector& principalMoments,const C7Vector& newFrame)
{ // remember that we always work with a massless tensor. The tensor is multiplied with the mass in the dynamics module!
    C3X3Matrix tensor;
    tensor.clear();
    tensor.axis[0](0)=principalMoments(0);
    tensor.axis[1](1)=principalMoments(1);
    tensor.axis[2](2)=principalMoments(2);
    // 1. reorient the frame:
    tensor=newFrame.Q.getMatrix()*tensor*newFrame.Q.getInverse().getMatrix();
    // 2. shift the frame:
    C3X3Matrix D;
    D.setIdentity();
    D*=(newFrame.X*newFrame.X);
    D.axis[0]-=newFrame.X*newFrame.X(0);
    D.axis[1]-=newFrame.X*newFrame.X(1);
    D.axis[2]-=newFrame.X*newFrame.X(2);
    tensor+=D;
    return(tensor);
}
