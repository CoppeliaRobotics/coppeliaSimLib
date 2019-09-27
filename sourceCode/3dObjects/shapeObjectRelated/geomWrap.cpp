
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "geomWrap.h"
#include "geometric.h"
#include "tt.h"
#include "global.h"
#include "viewableBase.h"
#include "app.h"

CGeomWrap::CGeomWrap()
{
    _mass=1.0f;
    _name="sub__0";

    _dynMaterialId_OLD=-1; // not used anymore since V3.4.0

    _convex=false;

    setDefaultInertiaParams();

    _transformationsSinceGrouping.setIdentity();
}

CGeomWrap::~CGeomWrap()
{
    for (int i=0;i<int(childList.size());i++)
        delete childList[i];
}

C7Vector CGeomWrap::getTransformationsSinceGrouping()
{
    return(_transformationsSinceGrouping);
}

void CGeomWrap::setTransformationsSinceGrouping(const C7Vector& tr)
{
    _transformationsSinceGrouping=tr;
}

void CGeomWrap::display(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->display(geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,multishapeEditSelected);
}

void CGeomWrap::display_extRenderer(CGeomProxy* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->display_extRenderer(geomData,displayAttrib,tr,shapeHandle,componentIndex);
}

void CGeomWrap::display_colorCoded(CGeomProxy* geomData,int objectId,int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->display_colorCoded(geomData,objectId,displayAttrib);
}


void CGeomWrap::displayForCutting(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->displayForCutting(geomData,displayAttrib,collisionColor);
}

void CGeomWrap::displayGhost(CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->displayGhost(geomData,displayAttrib,originalColors,backfaceCulling,transparency,newColors);
}

void CGeomWrap::perform3DObjectLoadingMapping(std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->perform3DObjectLoadingMapping(map);
}

void CGeomWrap::performTextureObjectLoadingMapping(std::vector<int>* map)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->performTextureObjectLoadingMapping(map);
}

void CGeomWrap::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    _dynMaterialId_OLD=App::ct->objCont->getLoadingMapping(map,_dynMaterialId_OLD);
}

void CGeomWrap::announce3DObjectWillBeErased(int objectID)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->announce3DObjectWillBeErased(objectID);
}

void CGeomWrap::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->setTextureDependencies(shapeID);
}

int CGeomWrap::getTextureCount()
{ // function has virtual/non-virtual counterpart!
    int retVal=0;
    for (int i=0;i<int(childList.size());i++)
        retVal+=childList[i]->getTextureCount();
    return(retVal);
}

bool CGeomWrap::hasTextureThatUsesFixedTextureCoordinates()
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
    {
        if (childList[i]->hasTextureThatUsesFixedTextureCoordinates())
            return(true);
    }
    return(false);
}

void CGeomWrap::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->removeAllTextures();
}

void CGeomWrap::getColorStrings(std::string& colorStrings)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->getColorStrings(colorStrings);
}

bool CGeomWrap::getContainsTransparentComponents()
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
    {
        if (childList[i]->getContainsTransparentComponents())
            return(true);
    }
    return(false);
}

float CGeomWrap::getGouraudShadingAngle()
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getGouraudShadingAngle()); // we just return the first angle we encounter! Normally never used
}

void CGeomWrap::setGouraudShadingAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->setGouraudShadingAngle(angle);
}

float CGeomWrap::getEdgeThresholdAngle()
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getEdgeThresholdAngle()); // we just return the first angle we encounter! Normally never used
}

void CGeomWrap::setEdgeThresholdAngle(float angle)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->setEdgeThresholdAngle(angle);
}

void CGeomWrap::setHideEdgeBorders(bool v)
{  // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->setHideEdgeBorders(v);
}

bool CGeomWrap::getHideEdgeBorders()
{  // function has virtual/non-virtual counterpart!
    return(childList[0]->getHideEdgeBorders()); // we just return the first setting we encounter! Normally never used
}

CGeomWrap* CGeomWrap::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CGeomWrap* newIt=new CGeomWrap();
    copyWrapperInfos(newIt);
    return(newIt);
}

void CGeomWrap::copyWrapperInfos(CGeomWrap* target)
{
    target->_mass=_mass;
    target->_name=_name;
    target->_convex=_convex;

    target->_localInertiaFrame=_localInertiaFrame;
    target->_principalMomentsOfInertia=_principalMomentsOfInertia;

    target->_transformationsSinceGrouping=_transformationsSinceGrouping;

    target->_dynMaterialId_OLD=_dynMaterialId_OLD;

    for (size_t i=0;i<childList.size();i++)
        target->childList.push_back(childList[i]->copyYourself());
}

void CGeomWrap::setMass(float m)
{
    _mass=tt::getLimitedFloat(0.000000001f,100000.0f,m);
}

float CGeomWrap::getMass()
{
    return(_mass);
}

void CGeomWrap::setDefaultInertiaParams()
{
    _localInertiaFrame.setIdentity();
    _principalMomentsOfInertia(0)=0.001f;
    _principalMomentsOfInertia(1)=0.001f;
    _principalMomentsOfInertia(2)=0.001f;
}

void CGeomWrap::setName(std::string newName)
{
    _name=newName;
}

std::string CGeomWrap::getName()
{
    return(_name);
}

int CGeomWrap::getDynMaterialId_OLD()
{
    return(_dynMaterialId_OLD);
}

void CGeomWrap::setDynMaterialId_OLD(int id)
{
    _dynMaterialId_OLD=id;
}

C7Vector CGeomWrap::getLocalInertiaFrame()
{
    return (_localInertiaFrame);
}

void CGeomWrap::setLocalInertiaFrame(const C7Vector& li)
{
    _localInertiaFrame=li;
}

C3Vector CGeomWrap::getPrincipalMomentsOfInertia()
{
    return (_principalMomentsOfInertia);
}

void CGeomWrap::setPrincipalMomentsOfInertia(const C3Vector& inertia)
{
    _principalMomentsOfInertia=inertia;
    _principalMomentsOfInertia(0)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(0));
    _principalMomentsOfInertia(1)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(1));
    _principalMomentsOfInertia(2)=tt::getLimitedFloat(0.0f,10000.0f,_principalMomentsOfInertia(2));
    if (_principalMomentsOfInertia.getLength()==0.0f)
        _principalMomentsOfInertia(0)=0.001f; // make sure we don't have a zero vector (problems with Bullet? and V-REP!)
}

void CGeomWrap::scale(float xVal,float yVal,float zVal)
{ // function has virtual/non-virtual counterpart!
    // iso-scaling for compound shapes!! (should normally already be xVal=yVal=zVal)
    scaleWrapperInfos(xVal,xVal,xVal);
}

void CGeomWrap::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CGeomWrap::scaleWrapperInfos(float xVal,float yVal,float zVal)
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

void CGeomWrap::scaleMassAndInertia(float xVal,float yVal,float zVal)
{
    _mass*=xVal*yVal*zVal;
    _principalMomentsOfInertia(0)*=yVal*zVal;
    _principalMomentsOfInertia(1)*=xVal*zVal;
    _principalMomentsOfInertia(2)*=xVal*yVal;
}

void CGeomWrap::setPurePrimitiveType(int theType,float xOrDiameter,float y,float zOrHeight)
{ // function has virtual/non-virtual counterpart!
    // Following added on 14/03/2011 because a compound shape composed by pure and non pure shapes would decompose as pure shapes with wrong orientation!
    if (theType==sim_pure_primitive_none)
    {
        for (int i=0;i<int(childList.size());i++)
            childList[i]->setPurePrimitiveType(theType,xOrDiameter,y,zOrHeight);
        // _convex=false;  NO!!!
    }
}

int CGeomWrap::getPurePrimitiveType()
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getPurePrimitiveType()); // we just return the first type we encounter! Normally never used
}

bool CGeomWrap::isGeometric()
{ // function has virtual/non-virtual counterpart!
    return(false);
}

bool CGeomWrap::isPure()
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->isPure());
}

bool CGeomWrap::isConvex()
{ // function has virtual/non-virtual counterpart!
    return(_convex);
}

bool CGeomWrap::containsOnlyPureConvexShapes()
{ // function has virtual/non-virtual counterpart!
    bool retVal=true;
    for (int i=0;i<int(childList.size());i++)
        retVal&=childList[i]->containsOnlyPureConvexShapes();
    if (retVal)
        _convex=retVal; // needed since there was a bug where pure planes and pure discs were considered as convex
    return(retVal);
}

bool CGeomWrap::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex=true;
    for (int i=0;i<int(childList.size());i++)
        _convex&=childList[i]->checkIfConvex();
    setConvex(_convex);
    return(_convex);
}

void CGeomWrap::setConvex(bool convex)
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

void CGeomWrap::getCumulativeMeshes(std::vector<float>& vertices,std::vector<int>* indices,std::vector<float>* normals)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->getCumulativeMeshes(vertices,indices,normals);
}

void CGeomWrap::setColor(const char* colorName,int colorComponent,const float* rgbData)
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->setColor(colorName,colorComponent,rgbData);
}

bool CGeomWrap::getColor(const char* colorName,int colorComponent,float* rgbData)
{ // function has virtual/non-virtual counterpart!
    bool retVal=false;
    for (int i=0;i<int(childList.size());i++)
        retVal|=childList[i]->getColor(colorName,colorComponent,rgbData);
    return(retVal);
}

void CGeomWrap::getAllShapeComponentsCumulative(std::vector<CGeometric*>& shapeComponentList)
{   // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->getAllShapeComponentsCumulative(shapeComponentList);
}

CGeometric* CGeomWrap::getShapeComponentAtIndex(int& index)
{ // function has virtual/non-virtual counterpart!
    if (index<0)
        return(nullptr);
    for (int i=0;i<int(childList.size());i++)
    {
        CGeometric* retVal=childList[i]->getShapeComponentAtIndex(index);
        if (retVal!=nullptr)
            return(retVal);
        if (index<0)
            return(nullptr);
    }
    return(nullptr);
}

void CGeomWrap::preMultiplyAllVerticeLocalFrames(const C7Vector& preTr)
{ // function has virtual/non-virtual counterpart!

    _transformationsSinceGrouping=preTr*_transformationsSinceGrouping;
    _localInertiaFrame=preTr*_localInertiaFrame;

    for (int i=0;i<int(childList.size());i++)
        childList[i]->preMultiplyAllVerticeLocalFrames(preTr);
}

void CGeomWrap::flipFaces()
{ // function has virtual/non-virtual counterpart!
    for (int i=0;i<int(childList.size());i++)
        childList[i]->flipFaces();
    checkIfConvex();
}

void CGeomWrap::serialize(CSer& ar,const char* shapeName)
{ // function has virtual/non-virtual counterpart!
    serializeWrapperInfos(ar,shapeName);
}

void CGeomWrap::serializeWrapperInfos(CSer& ar,const char* shapeName)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << _name;
            ar.flush();

            ar.storeDataName("Mas");
            ar << _mass;
            ar.flush();

            ar.storeDataName("Dmi");
            ar << _dynMaterialId_OLD;
            ar.flush();

            ar.storeDataName("Ine");
            ar << _localInertiaFrame(0) << _localInertiaFrame(1) << _localInertiaFrame(2) << _localInertiaFrame(3);
            ar << _localInertiaFrame(4) << _localInertiaFrame(5) << _localInertiaFrame(6);
            ar << _principalMomentsOfInertia(0) << _principalMomentsOfInertia(1) << _principalMomentsOfInertia(2);
            ar.flush();

            ar.storeDataName("Vtb");
            ar << _transformationsSinceGrouping(0) << _transformationsSinceGrouping(1) << _transformationsSinceGrouping(2) << _transformationsSinceGrouping(3);
            ar << _transformationsSinceGrouping(4) << _transformationsSinceGrouping(5) << _transformationsSinceGrouping(6);
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            // reserved (12/9/2013) SIM_SET_CLEAR_BIT(nothing,0,_bulletNonDefaultCollisionMargin);
            // reserved (12/9/2013) SIM_SET_CLEAR_BIT(nothing,1,_bulletStickyContact);
            SIM_SET_CLEAR_BIT(nothing,2,_convex);
            // reserved (12/9/2013) SIM_SET_CLEAR_BIT(nothing,3,!_bulletAutoShrinkConvexMesh);
            // reserved (12/9/2013) SIM_SET_CLEAR_BIT(nothing,4,_bulletNonDefaultCollisionMargin_forConvexAndNonPureShape); // since 24/3/2013
            SIM_SET_CLEAR_BIT(nothing,5,true); // means: we do not have to make the convectivity test for this shape (was already done). Added this on 28/1/2013
            ar << nothing;
            ar.flush();

            for (int i=0;i<int(childList.size());i++)
            {
                if (childList[i]->isGeometric())
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
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _mass;
                        if (_mass==0.0f) // to catch an old bug
                            _mass=0.001f;
                    }
                    if (theName.compare("Dmi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynMaterialId_OLD;
                    }
                    if (theName.compare("Ine")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _localInertiaFrame(0) >> _localInertiaFrame(1) >> _localInertiaFrame(2) >> _localInertiaFrame(3);
                        ar >> _localInertiaFrame(4) >> _localInertiaFrame(5) >> _localInertiaFrame(6);
                        ar >> _principalMomentsOfInertia(0) >> _principalMomentsOfInertia(1) >> _principalMomentsOfInertia(2);
                    }

                    if (theName.compare("Vtb")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _transformationsSinceGrouping(0) >> _transformationsSinceGrouping(1) >> _transformationsSinceGrouping(2) >> _transformationsSinceGrouping(3);
                        ar >> _transformationsSinceGrouping(4) >> _transformationsSinceGrouping(5) >> _transformationsSinceGrouping(6);
                    }

                    if (theName=="Var")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        // 0 reserved
                        // 1 reserved
                        _convex=SIM_IS_BIT_SET(nothing,2);
                        // 3 reserved
                        // 4 reserved
                        // 5 reserved
                    }
                    if (theName.compare("Geo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CGeometric* it=new CGeometric();
                        it->serialize(ar,shapeName);
                        childList.push_back(it);
                    }
                    if (theName.compare("Wrp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CGeomWrap* it=new CGeomWrap();
                        it->serialize(ar,shapeName);
                        childList.push_back(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}


void CGeomWrap::findPrincipalMomentOfInertia(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments)
{ // This routine is iterative and not elegant. But we do not need speed here anyway ;)
    C3X3Matrix rot;
    C3X3Matrix tens(tensor);
    rot.setIdentity();
    float w=_getTensorNonDiagonalMeasure(tensor);

    // With below settings, we will get an orientational precision of about 0.000128 degrees
    float stepSize=10.0f*degToRad_f;
    const float stepSizeMultiplier=0.2f;
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
                float w2=_getTensorNonDiagonalMeasure(tens);
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
}

float CGeomWrap::_getTensorNonDiagonalMeasure(const C3X3Matrix& tensor)
{
    C3Vector v(tensor.axis[1](0),tensor.axis[2](0),tensor.axis[2](1));
    return(v*v);
}

C3X3Matrix CGeomWrap::getNewTensor(const C3Vector& principalMoments,const C7Vector& newFrame)
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
