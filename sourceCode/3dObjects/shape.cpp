#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "shape.h"
#include "tt.h"
#include "algos.h"
#include "statDistObj.h"
#include "app.h"
#include "geomWrap.h"
#include "geometric.h"
#include "easyLock.h"
#include "pluginContainer.h"
#include "shapeRendering.h"

bool CShape::_visualizeObbStructures=false;

bool CShape::getDebugObbStructures()
{
    return(_visualizeObbStructures);
}

void CShape::setDebugObbStructures(bool d)
{
    _visualizeObbStructures=d;
}


CShape::CShape()
{
    geomData=nullptr;
    commonInit();
}

CShape::~CShape()
{
    delete geomData;
    delete _dynMaterial;
}

CDynMaterialObject* CShape::getDynMaterial()
{
    return(_dynMaterial);
}

void CShape::setDynMaterial(CDynMaterialObject* mat)
{
    delete _dynMaterial;
    _dynMaterial=mat;
}

C3Vector CShape::getInitialDynamicLinearVelocity()
{
    return(_initialDynamicLinearVelocity);
}

void CShape::setInitialDynamicLinearVelocity(const C3Vector& vel)
{
    _initialDynamicLinearVelocity=vel;
}

C3Vector CShape::getInitialDynamicAngularVelocity()
{
    return(_initialDynamicAngularVelocity);
}

void CShape::setInitialDynamicAngularVelocity(const C3Vector& vel)
{
    _initialDynamicAngularVelocity=vel;
}

void CShape::setRigidBodyWasAlreadyPutToSleepOnce(bool s)
{
    _rigidBodyWasAlreadyPutToSleepOnce=s;
}

bool CShape::getRigidBodyWasAlreadyPutToSleepOnce()
{
    return(_rigidBodyWasAlreadyPutToSleepOnce);
}

void CShape::actualizeContainsTransparentComponent()
{
    if (geomData!=nullptr) // important
        _containsTransparentComponents=geomData->geomInfo->getContainsTransparentComponents();
}

bool CShape::getContainsTransparentComponent()
{
    return(_containsTransparentComponents);
}

void CShape::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{
    if (geomData!=nullptr)
        geomData->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CShape::setShapeIsStaticAndNotRespondableButDynamicTag(bool f)
{
    _shapeIsStaticAndNotRespondableButDynamicTag=f;
}

bool CShape::getShapeIsStaticAndNotRespondableButDynamicTag()
{
    return(_shapeIsStaticAndNotRespondableButDynamicTag);
}

void CShape::setDynamicCollisionMask(unsigned short m)
{
    _dynamicCollisionMask=m;
}

unsigned short CShape::getDynamicCollisionMask()
{
    return(_dynamicCollisionMask);
}

C3DObject* CShape::getLastParentForLocalGlobalCollidable()
{
    if (_lastParentForLocalGlobalCollidable!=nullptr)
        return(_lastParentForLocalGlobalCollidable);
    C3DObject* it=this;
    while (it->getParentObject()!=nullptr)
        it=it->getParentObject();
    _lastParentForLocalGlobalCollidable=it;
    return(_lastParentForLocalGlobalCollidable);
}

void CShape::clearLastParentForLocalGlobalCollidable()
{
    _lastParentForLocalGlobalCollidable=nullptr;
}

void CShape::setParentFollowsDynamic(bool f)
{ // careful, is also called from makeObjectChildOf routine
    _parentFollowsDynamic=f;
}

bool CShape::getParentFollowsDynamic()
{
    return(_parentFollowsDynamic);
}

std::string CShape::getObjectTypeInfo() const
{
    return("Shape");
}

std::string CShape::getObjectTypeInfoExtended() const
{
    if (geomData->geomInfo->isGeometric())
    {
        int pureType=((CGeometric*)geomData->geomInfo)->getPurePrimitiveType();
        if (pureType==sim_pure_primitive_none)
            return("Shape (simple, non-pure)");
        if (pureType==sim_pure_primitive_plane)
            return("Shape (simple, pure (plane))");
        if (pureType==sim_pure_primitive_disc)
            return("Shape (simple, pure (disc))");
        if (pureType==sim_pure_primitive_cuboid)
            return("Shape (simple, pure (cuboid))");
        if (pureType==sim_pure_primitive_spheroid)
            return("Shape (simple, pure (sphere))");
        if (pureType==sim_pure_primitive_cylinder)
            return("Shape (simple, pure (cylinder))");
        if (pureType==sim_pure_primitive_cone)
            return("Shape (simple, pure (cone))");
        if (pureType==sim_pure_primitive_heightfield)
            return("Shape (simple, pure (heightfield))");
    }
    else
    {
        if (!geomData->geomInfo->isPure())
            return("Shape (multishape, non-pure)");
        else
            return("Shape (multishape, pure)");
    }
    return("ERROR");
}
bool CShape::isPotentiallyCollidable() const
{
    return(true);
}
bool CShape::isPotentiallyMeasurable() const
{
    return(true);
}
bool CShape::isPotentiallyDetectable() const
{
    return(true);
}
bool CShape::isPotentiallyRenderable() const
{
    return(true);
}
bool CShape::isPotentiallyCuttable() const
{
    return ((!isCompound())&&(!geomData->geomInfo->isPure()));
}

bool CShape::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    maxV=((CShape*)this)->geomData->getBoundingBoxHalfSizes();
    minV=maxV*-1.0f;
    return(true);
}

bool CShape::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

void CShape::commonInit()
{
    setObjectType(sim_object_shape_type);
    _initialValuesInitialized=false;
    _containsTransparentComponents=false;
    _startInDynamicSleeping=false;
    _shapeIsDynamicallyStatic=true;
    _setAutomaticallyToNonStaticIfGetsParent=false;
    _shapeIsDynamicallyRespondable=false; // keep false, otherwise too many "default" problems
    _dynamicCollisionMask=0xffff;
    _parentFollowsDynamic=false;
    _lastParentForLocalGlobalCollidable=nullptr;
    _initialDynamicLinearVelocity.clear();
    _initialDynamicAngularVelocity.clear();

    layer=SHAPE_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable;// by DEFAULT not cuttable! |sim_objectspecialproperty_cuttable;
    _objectName="Shape";
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();

    _dynMaterial=new CDynMaterialObject();
}

void CShape::setDynamicVelocity(const C3Vector& linearV,const C3Vector& angularV)
{
    _dynamicLinearVelocity=linearV;
    _dynamicAngularVelocity=angularV;
}

C3Vector CShape::getDynamicLinearVelocity()
{
    return(_dynamicLinearVelocity);
}

C3Vector CShape::getDynamicAngularVelocity()
{
    return(_dynamicAngularVelocity);
}

void CShape::addAdditionalForceAndTorque(const C3Vector& f,const C3Vector& t)
{
    _additionalForce+=f;
    _additionalTorque+=t;
}

void CShape::clearAdditionalForceAndTorque()
{
    clearAdditionalForce();
    clearAdditionalTorque();
}

void CShape::clearAdditionalForce()
{
    _additionalForce.clear();
}

void CShape::clearAdditionalTorque()
{
    _additionalTorque.clear();
}

C3Vector CShape::getAdditionalForce()
{
    return(_additionalForce);
}

C3Vector CShape::getAdditionalTorque()
{
    return(_additionalTorque);
}

void CShape::setRespondable(bool r)
{
    _shapeIsDynamicallyRespondable=r;
}

bool CShape::getRespondable()
{
    return(_shapeIsDynamicallyRespondable);
}



bool CShape::getSetAutomaticallyToNonStaticIfGetsParent()
{
    return(_setAutomaticallyToNonStaticIfGetsParent);
}

void CShape::setSetAutomaticallyToNonStaticIfGetsParent(bool autoNonStatic)
{
    _setAutomaticallyToNonStaticIfGetsParent=autoNonStatic;
}

bool CShape::getStartInDynamicSleeping()
{
    return(_startInDynamicSleeping);
}

void CShape::setStartInDynamicSleeping(bool sleeping)
{
    _startInDynamicSleeping=sleeping;
}

bool CShape::getShapeIsDynamicallyStatic()
{
    return(_shapeIsDynamicallyStatic);
}

void CShape::setShapeIsDynamicallyStatic(bool sta)
{
    _shapeIsDynamicallyStatic=sta;
    if (!sta)
        _setAutomaticallyToNonStaticIfGetsParent=false;
}

void CShape::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        ((CGeometric*)geomData->geomInfo)->setInsideAndOutsideFacesSameColor_DEPRECATED(s);
}
bool CShape::getInsideAndOutsideFacesSameColor_DEPRECATED()
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        return(((CGeometric*)geomData->geomInfo)->getInsideAndOutsideFacesSameColor_DEPRECATED());
    return(true);
}

bool CShape::isCompound() const
{
    return(!geomData->geomInfo->isGeometric());
}

int CShape::getEdgeWidth_DEPRECATED()
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        return(((CGeometric*)geomData->geomInfo)->getEdgeWidth_DEPRECATED());
    return(0);
}

void CShape::setEdgeWidth_DEPRECATED(int w)
{
    w=tt::getLimitedInt(1,4,w);
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        ((CGeometric*)geomData->geomInfo)->setEdgeWidth_DEPRECATED(w);
}

bool CShape::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    if (index==0)
    {
        std::vector<float> visibleVertices;
        std::vector<int> visibleIndices;
        geomData->geomInfo->getCumulativeMeshes(visibleVertices,&visibleIndices,nullptr);

        C7Vector m(getCumulativeTransformationPart1());
        C3Vector v;
        for (int j=0;j<int(visibleVertices.size())/3;j++)
        {
            v(0)=visibleVertices[3*j+0];
            v(1)=visibleVertices[3*j+1];
            v(2)=visibleVertices[3*j+2];
            v=m*v;
            vertices.push_back(v(0));
            vertices.push_back(v(1));
            vertices.push_back(v(2));
        }
        indices.assign(visibleIndices.begin(),visibleIndices.end());
        return(true);
    }
    return(App::ct->drawingCont->getExportableMeshAtIndex(getObjectHandle(),index-1,vertices,indices));
}

void CShape::display_extRenderer(CViewableBase* renderingObject,int displayAttrib)
{
    if (getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if (renderingObject->isObjectInsideView(getCumulativeTransformation(),geomData->getBoundingBoxHalfSizes()))
        { // the bounding box is inside of the view (at least some part of it!)
            C7Vector tr=getCumulativeTransformationPart1_forDisplay((displayAttrib&sim_displayattribute_forvisionsensor)==0);
            int componentIndex=0;
            geomData->geomInfo->display_extRenderer(geomData,displayAttrib,tr,_objectHandle,componentIndex);
        }
    }
}

void CShape::scaleObject(float scalingFactor)
{   
    geomData->scale(scalingFactor,scalingFactor,scalingFactor); // will set the geomObject dynamics full refresh flag!
    scaleObjectMain(scalingFactor);
    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;
}

void CShape::scaleObjectNonIsometrically(float x,float y,float z)
{
    float xp,yp,zp;
    geomData->scale(x,y,z,xp,yp,zp); // will set the geomObject dynamics full refresh flag!
    scaleObjectNonIsometricallyMain(xp,yp,zp);
    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;
}

bool CShape::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    geomData->announce3DObjectWillBeErased(objectHandle); // for textures based on vision sensors
    return(retVal);
}

void CShape::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CShape::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CShape::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CShape::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CShape::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CShape::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    geomData->perform3DObjectLoadingMapping(map);
}
void CShape::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CShape::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CShape::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CShape::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CShape::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CShape::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
    geomData->performTextureObjectLoadingMapping(map);
}

void CShape::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
    geomData->geomInfo->performDynMaterialObjectLoadingMapping(map);
}

void CShape::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CShape::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CShape::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();

    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialInitialDynamicLinearVelocity=_initialDynamicLinearVelocity;
        _initialInitialDynamicAngularVelocity=_initialDynamicAngularVelocity;
    }

    actualizeContainsTransparentComponent(); // added on 2010/11/22 to correct at least each time a simulation starts, when those values where not set correctly
}

void CShape::simulationAboutToStart()
{
    initializeInitialValues(true);
    _rigidBodyWasAlreadyPutToSleepOnce=false;
}

void CShape::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _initialDynamicLinearVelocity=_initialInitialDynamicLinearVelocity;
        _initialDynamicAngularVelocity=_initialInitialDynamicAngularVelocity;
    }
    _initialValuesInitialized=false;

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();
    simulationEndedMain();
}

void CShape::resetMilling()
{
    if ((_localObjectSpecialProperty&sim_objectspecialproperty_cuttable)&&geomData->isCollisionInformationInitialized())
        geomData->removeCollisionInformation();
}

bool CShape::applyMilling()
{
    if ((_localObjectSpecialProperty&sim_objectspecialproperty_cuttable)&&geomData->isCollisionInformationInitialized())
    {
        if (geomData->applyCuttingChanges(getCumulativeTransformation()))
            return(true); // means that this shape has to be destroyed!
        incrementMemorizedConfigurationValidCounter();
        geomData->removeCollisionInformation(); // is probably not needed
        C7Vector newCTM(geomData->getCreationTransformation());
        C7Vector xTr(getCumulativeTransformation().getInverse()*newCTM);
        geomData->setCreationTransformation(C7Vector::identityTransformation);

        setLocalTransformation(getLocalTransformation()*xTr);
        // we have to correct the attached drawing objects:
        App::ct->drawingCont->adjustForFrameChange(getObjectHandle(),xTr.getInverse());
        for (int j=0;j<int(childList.size());j++) // adjust for the children:
        {
            childList[j]->setLocalTransformation(xTr.getInverse()*childList[j]->getLocalTransformationPart1());
            childList[j]->incrementMemorizedConfigurationValidCounter();
        }
        
    }
    return(false);
}

void CShape::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {   // Storing
            // Following tags are reserved (11/11/2012): Sco, Sc2, Eco, Ewt

            ar.storeDataName("Ge2");
            ar.setCountingMode();
            geomData->serialize(ar,_objectName.c_str());
            if (ar.setWritingMode())
                geomData->serialize(ar,_objectName.c_str());

            ar.storeDataName("Mat");
            ar.setCountingMode();
            _dynMaterial->serialize(ar);
            if (ar.setWritingMode())
                _dynMaterial->serialize(ar);

            ar.storeDataName("Dc2");
            ar << _dynamicCollisionMask;
            ar.flush();

            ar.storeDataName("Idv");
            ar << _initialDynamicLinearVelocity(0) << _initialDynamicLinearVelocity(1) << _initialDynamicLinearVelocity(2);
            ar << _initialDynamicAngularVelocity(0) << _initialDynamicAngularVelocity(1) << _initialDynamicAngularVelocity(2);
            ar.flush();

            ar.storeDataName("Sss");
            unsigned char nothing=0;
    //      SIM_SET_CLEAR_BIT(nothing,0,_explicitTracing); removed on 13/09/2011
    //      SIM_SET_CLEAR_BIT(nothing,1,_visibleEdges); removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,2,_culling); removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,3,tracing); removed on 13/09/2011
    //      SIM_SET_CLEAR_BIT(nothing,4,_shapeWireframe); removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,5,_displayInvertedFaces); removed on 2010/04/19
            SIM_SET_CLEAR_BIT(nothing,6,_startInDynamicSleeping);
            SIM_SET_CLEAR_BIT(nothing,7,!_shapeIsDynamicallyStatic);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Ss3");
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_shapeIsDynamicallyRespondable);
    //      SIM_SET_CLEAR_BIT(nothing,1,_visualizeInertia); // removed on 16/12/2012
            SIM_SET_CLEAR_BIT(nothing,2,_parentFollowsDynamic);
    //      SIM_SET_CLEAR_BIT(nothing,3,!_insideAndOutsideFacesSameColor); // removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,4,_containsTransparentComponents); // removed on 11/11/2012
            SIM_SET_CLEAR_BIT(nothing,5,_setAutomaticallyToNonStaticIfGetsParent);
            ar << nothing;
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
                    if (theName.compare("Ge2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        geomData=new CGeomProxy();
                        geomData->serialize(ar,_objectName.c_str());
                        geomData->geomInfo->containsOnlyPureConvexShapes(); // needed since there was a bug where pure planes and pure discs were considered as convex
                    }
                    if (theName.compare("Mat")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _dynMaterial->serialize(ar);
                    }

                    if (theName.compare("Dc2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicCollisionMask;
                    }

                    if (theName.compare("Idv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _initialDynamicLinearVelocity(0) >> _initialDynamicLinearVelocity(1) >> _initialDynamicLinearVelocity(2);
                        ar >> _initialDynamicAngularVelocity(0) >> _initialDynamicAngularVelocity(1) >> _initialDynamicAngularVelocity(2);
                    }

                    if (theName=="Sss")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _startInDynamicSleeping=SIM_IS_BIT_SET(nothing,6);
                        _shapeIsDynamicallyStatic=!SIM_IS_BIT_SET(nothing,7);
                    }
                    if (theName=="Ss2")
                    { // keep for backward compatibility (2010/07/12)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable=SIM_IS_BIT_SET(nothing,0);
                        _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName=="Ss3")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable=SIM_IS_BIT_SET(nothing,0);
                        _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2);
                        _setAutomaticallyToNonStaticIfGetsParent=SIM_IS_BIT_SET(nothing,5);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            actualizeContainsTransparentComponent();
        }
    }
}

void CShape::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CShape::alignBoundingBoxWithMainAxis()
{
    reorientGeometry(0);
}
void CShape::alignBoundingBoxWithWorld()
{
    reorientGeometry(1);    
}

bool CShape::alignTubeBoundingBoxWithMainAxis()
{
    return(reorientGeometry(2));    
}

bool CShape::alignCuboidBoundingBoxWithMainAxis()
{
    return(reorientGeometry(3));    
}

bool CShape::reorientGeometry(int type)
{ // return value is the success state of the operation
    C7Vector m(getCumulativeTransformationPart1());
    C7Vector mNew;
    bool error=false;
    if (type<2)
        mNew=geomData->recomputeOrientation(m,type==0);
    if (type==2)
        mNew=geomData->recomputeTubeOrCuboidOrientation(m,true,error);
    if (type==3)
        mNew=geomData->recomputeTubeOrCuboidOrientation(m,false,error);

    if (error)
        return(false);

    C7Vector mCorr(m.getInverse()*mNew);
    C7Vector mCorrInv(mCorr.getInverse());

    // Now we have to compute the new local transformation:
    setLocalTransformation(getLocalTransformationPart1()*mCorr);
    // we have to correct the attached drawing objects:
    App::ct->drawingCont->adjustForFrameChange(getObjectHandle(),mCorrInv);
    incrementMemorizedConfigurationValidCounter();
    for (int i=0;i<int(childList.size());i++)
    {
        childList[i]->setLocalTransformation(mCorrInv*childList[i]->getLocalTransformationPart1());
        childList[i]->incrementMemorizedConfigurationValidCounter();
    }
    App::setFullDialogRefreshFlag(); // so that textures and other things get updated!
    return(true);
}

void CShape::removeCollisionInformation()
{
    geomData->removeCollisionInformation();
}

/*
void CShape::initializeCollisionDetection()
{
    geomData->initializeCollisionInformation();
}
*/
bool CShape::isCollisionInformationInitialized()
{
    return(geomData->isCollisionInformationInitialized());
}


void CShape::initializeCalculationStructureIfNeeded()
{
    geomData->initializeCalculationStructureIfNeeded();
}

bool CShape::getCulling()
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        return(((CGeometric*)geomData->geomInfo)->getCulling());
    return(false);
}

void CShape::setCulling(bool culState)
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        ((CGeometric*)geomData->geomInfo)->setCulling(culState);
}

bool CShape::getVisibleEdges()
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        return(((CGeometric*)geomData->geomInfo)->getVisibleEdges());
    return(false);
}

void CShape::setVisibleEdges(bool v)
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        ((CGeometric*)geomData->geomInfo)->setVisibleEdges(v);
}

bool CShape::getHideEdgeBorders()
{
    if (geomData!=nullptr)
        return(geomData->geomInfo->getHideEdgeBorders());
    return(false);
}

void CShape::setHideEdgeBorders(bool v)
{
    if (geomData!=nullptr)
        geomData->geomInfo->setHideEdgeBorders(v);
}

bool CShape::getShapeWireframe()
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        return(((CGeometric*)geomData->geomInfo)->getWireframe());
    return(false);
}

void CShape::setShapeWireframe(bool w)
{
    if ((geomData!=nullptr)&&(geomData->geomInfo->isGeometric()))
        ((CGeometric*)geomData->geomInfo)->setWireframe(w);
}

bool CShape::doesShapeCollideWithShape(CShape* collidee,std::vector<float>* intersections)
{   // If intersections is different from nullptr, we check for all intersections and
    // intersection segments are appended to the vector

    std::vector<float> _intersect;
    std::vector<float>* _intersectP=nullptr;
    if (intersections!=nullptr)
        _intersectP=&_intersect;

    C4X4Matrix shapeACTM=getCumulativeTransformation().getMatrix();
    C4X4Matrix shapeBCTM=collidee->getCumulativeTransformation().getMatrix();

    if (!shapeACTM.isValid())
        return(false);
    if (!shapeBCTM.isValid())
        return(false);

    C4X4Matrix collObjMatr[2]={shapeACTM,shapeBCTM};
    const void* collInfos[2]={geomData->collInfo,collidee->geomData->collInfo};

    if ( CPluginContainer::mesh_getMeshMeshCollision(geomData->collInfo,collidee->geomData->collInfo,collObjMatr,collInfos,false,_intersectP,nullptr))
    { // There was a collision
        if (intersections!=nullptr)
            intersections->insert(intersections->end(),_intersect.begin(),_intersect.end());
        return(true);
    }
    return(false);
}

bool CShape::getDistanceToDummy_IfSmaller(CDummy* dummy,float &dist,float ray[7],int& buffer)
{   // Distance is measured from this to dummy
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // Build the node only when needed. So do it right here!
    initializeCalculationStructureIfNeeded();

    C3Vector dummyPos(dummy->getCumulativeTransformation().X);
    C4X4Matrix thisPCTM(getCumulativeTransformation());
    C3Vector rayPart0;
    C3Vector rayPart1;
    if (CPluginContainer::mesh_getDistanceAgainstDummy_ifSmaller(geomData->collInfo,dummyPos,thisPCTM,dist,rayPart0,rayPart1,buffer))
    {
        rayPart0.copyTo(ray);
        rayPart1.copyTo(ray+3);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

bool CShape::getShapeShapeDistance_IfSmaller(CShape* it,float &dist,float ray[7],int buffer[2])
{   // this is shape number 1, 'it' is shape number 2 (for ordering (measured from 1 to 2))
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false

    CStatDistObj distObj(this,it);
    if (distObj.measure(dist,buffer))
    {
        ray[0]=distObj.ptOnShapeA(0);
        ray[1]=distObj.ptOnShapeA(1);
        ray[2]=distObj.ptOnShapeA(2);
        ray[3]=distObj.ptOnShapeB(0);
        ray[4]=distObj.ptOnShapeB(1);
        ray[5]=distObj.ptOnShapeB(2);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

void CShape::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CShape::copyYourself()
{   
    CShape* newShape=(CShape*)copyYourselfMain();

    if (geomData!=nullptr)
        newShape->geomData=geomData->copyYourself();

    delete newShape->_dynMaterial;
    newShape->_dynMaterial=_dynMaterial->copyYourself();

    // Various:
    newShape->_startInDynamicSleeping=_startInDynamicSleeping;
    newShape->_shapeIsDynamicallyStatic=_shapeIsDynamicallyStatic;
    newShape->_shapeIsDynamicallyRespondable=_shapeIsDynamicallyRespondable;
    newShape->_dynamicCollisionMask=_dynamicCollisionMask;
    newShape->_parentFollowsDynamic=_parentFollowsDynamic;
    newShape->_containsTransparentComponents=_containsTransparentComponents;
    newShape->_rigidBodyWasAlreadyPutToSleepOnce=_rigidBodyWasAlreadyPutToSleepOnce;
    newShape->_setAutomaticallyToNonStaticIfGetsParent=_setAutomaticallyToNonStaticIfGetsParent;
    newShape->_initialDynamicLinearVelocity=_initialDynamicLinearVelocity;
    newShape->_initialDynamicAngularVelocity=_initialDynamicAngularVelocity;


    newShape->_initialValuesInitialized=_initialValuesInitialized;

    return(newShape);
}

void CShape::setColor(const char* colorName,int colorComponent,const float* rgbData)
{
    if (geomData!=nullptr)
    {
        geomData->geomInfo->setColor(colorName,colorComponent,rgbData);
        if (colorComponent==sim_colorcomponent_transparency)
            actualizeContainsTransparentComponent();
    }
}

bool CShape::getColor(const char* colorName,int colorComponent,float* rgbData)
{
    if (geomData!=nullptr)
        return(geomData->geomInfo->getColor(colorName,colorComponent,rgbData));
    return(false);
}

void CShape::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CShape::display");
    EASYLOCK(_objectMutex);
    displayShape(this,renderingObject,displayAttrib);
}
