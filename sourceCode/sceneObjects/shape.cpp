#include "simInternal.h"
#include "shape.h"
#include "tt.h"
#include "algos.h"
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

CSceneObject* CShape::getLastParentForLocalGlobalRespondable()
{
    CSceneObject* retVal;
    if (_lastParentForLocalGlobalRespondable==nullptr)
    {
        retVal=this;
        while (retVal->getParent()!=nullptr)
            retVal=retVal->getParent();
        _lastParentForLocalGlobalRespondable=retVal;
    }
    else
        retVal=_lastParentForLocalGlobalRespondable;
    return(retVal);
}

void CShape::clearLastParentForLocalGlobalRespondable()
{
    _lastParentForLocalGlobalRespondable=nullptr;
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
    _lastParentForLocalGlobalRespondable=nullptr;
    _initialDynamicLinearVelocity.clear();
    _initialDynamicAngularVelocity.clear();

    _visibilityLayer=SHAPE_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable;
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

        C7Vector m(getCumulativeTransformation());
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
    return(App::currentWorld->drawingCont->getExportableMeshAtIndex(getObjectHandle(),index-1,vertices,indices));
}

void CShape::display_extRenderer(CViewableBase* renderingObject,int displayAttrib)
{
    if (getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if (renderingObject->isObjectInsideView(getFullCumulativeTransformation(),geomData->getBoundingBoxHalfSizes()))
        { // the bounding box is inside of the view (at least some part of it!)
            C7Vector tr=getCumulativeTransformation();
            int componentIndex=0;
            geomData->geomInfo->display_extRenderer(geomData,displayAttrib,tr,_objectHandle,componentIndex);
        }
    }
}

void CShape::scaleObject(float scalingFactor)
{   
    geomData->scale(scalingFactor,scalingFactor,scalingFactor); // will set the geomObject dynamics full refresh flag!
    CSceneObject::scaleObject(scalingFactor);
    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true;
}

void CShape::scaleObjectNonIsometrically(float x,float y,float z)
{
    float xp,yp,zp;
    geomData->scale(x,y,z,xp,yp,zp); // will set the geomObject dynamics full refresh flag!
    CSceneObject::scaleObjectNonIsometrically(xp,yp,zp);
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
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    geomData->announceSceneObjectWillBeErased(objectHandle); // for textures based on vision sensors
    return(retVal);
}

void CShape::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CShape::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CShape::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}
void CShape::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CShape::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    geomData->performSceneObjectLoadingMapping(map);
}
void CShape::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CShape::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CShape::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}
void CShape::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CShape::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
    geomData->performTextureObjectLoadingMapping(map);
}

void CShape::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
    geomData->geomInfo->performDynMaterialObjectLoadingMapping(map);
}

void CShape::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationIsRunning);
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
    CSceneObject::simulationAboutToStart();
}

void CShape::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _initialDynamicLinearVelocity=_initialInitialDynamicLinearVelocity;
        _initialDynamicAngularVelocity=_initialInitialDynamicAngularVelocity;
    }
    _initialValuesInitialized=false;

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();
    CSceneObject::simulationEnded();
}

void CShape::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
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
            SIM_SET_CLEAR_BIT(nothing,2,false); // removed on 01/05/2020
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
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                    }
                    if (theName=="Ss3")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable=SIM_IS_BIT_SET(nothing,0);
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                        _setAutomaticallyToNonStaticIfGetsParent=SIM_IS_BIT_SET(nothing,5);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            actualizeContainsTransparentComponent();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (exhaustiveXml)
        { // for non-exhaustive, is done in CSceneObjectContainer
            if (ar.isStoring())
            {
                ar.xmlPushNewNode("meshProxy");
                geomData->serialize(ar,_objectName.c_str());
                ar.xmlPopNode();

                ar.xmlPushNewNode("dynamics");
                ar.xmlAddNode_int("respondableMask",_dynamicCollisionMask);
                C3Vector vel=_initialDynamicLinearVelocity;
                vel*=180.0f/piValue_f;
                ar.xmlAddNode_floats("initialLinearVelocity",vel.data,3);
                vel=_initialDynamicAngularVelocity;
                vel*=180.0f/piValue_f;
                ar.xmlAddNode_floats("initialAngularVelocity",vel.data,3);
                ar.xmlPushNewNode("switches");
                ar.xmlAddNode_bool("static",_shapeIsDynamicallyStatic);
                ar.xmlAddNode_bool("respondable",_shapeIsDynamicallyRespondable);
                // ar.xmlAddNode_bool("parentFollows",_parentFollowsDynamic); removed on 01/05/2020
                ar.xmlAddNode_bool("startSleeping",_startInDynamicSleeping);
                ar.xmlAddNode_bool("setToDynamicIfGetsParent",_setAutomaticallyToNonStaticIfGetsParent);
                ar.xmlPopNode();
                ar.xmlPushNewNode("material");
                _dynMaterial->serialize(ar);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            else
            {
                if (ar.xmlPushChildNode("meshProxy"))
                {
                    geomData=new CGeomProxy();
                    geomData->serialize(ar,_objectName.c_str());
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("dynamics"))
                {
                    int m;
                    ar.xmlGetNode_int("respondableMask",m);
                    _dynamicCollisionMask=(unsigned short)m;
                    C3Vector vel;
                    ar.xmlGetNode_floats("initialLinearVelocity",vel.data,3);
                    _initialDynamicLinearVelocity=vel*piValue_f/180.0f;
                    ar.xmlGetNode_floats("initialAngularVelocity",vel.data,3);
                    _initialDynamicAngularVelocity=vel*piValue_f/180.0f;
                    if (ar.xmlPushChildNode("switches"))
                    {
                        ar.xmlGetNode_bool("static",_shapeIsDynamicallyStatic);
                        ar.xmlGetNode_bool("respondable",_shapeIsDynamicallyRespondable);
                        // ar.xmlGetNode_bool("parentFollows",_parentFollowsDynamic);  removed on 01/05/2020
                        ar.xmlGetNode_bool("startSleeping",_startInDynamicSleeping);
                        ar.xmlGetNode_bool("setToDynamicIfGetsParent",_setAutomaticallyToNonStaticIfGetsParent);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("material"))
                    {
                        _dynMaterial->serialize(ar);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
            }
        }
    }
}

void CShape::serializeWExtIk(CExtIkSer& ar)
{
    CSceneObject::serializeWExtIk(ar);
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
    C7Vector m(getCumulativeTransformation());
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
    setLocalTransformation(getLocalTransformation()*mCorr);
    // we have to correct the attached drawing objects:
    App::currentWorld->drawingCont->adjustForFrameChange(getObjectHandle(),mCorrInv);
    incrementMemorizedConfigurationValidCounter();
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        child->setLocalTransformation(mCorrInv*child->getLocalTransformation());
        child->incrementMemorizedConfigurationValidCounter();
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
    if ( CPluginContainer::geomPlugin_getMeshMeshCollision(geomData->collInfo,getFullCumulativeTransformation(),collidee->geomData->collInfo,collidee->getFullCumulativeTransformation(),_intersectP,nullptr,nullptr))
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

    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector rayPart0;
    if (CPluginContainer::geomPlugin_getMeshPointDistanceIfSmaller(geomData->collInfo,getFullCumulativeTransformation(),dummyPos,dist,&rayPart0,&buffer))
    {
        rayPart0.getInternalData(ray+0);
        dummyPos.getInternalData(ray+3);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

bool CShape::getShapeShapeDistance_IfSmaller(CShape* it,float &dist,float ray[7],int buffer[2])
{   // this is shape number 1, 'it' is shape number 2 (for ordering (measured from 1 to 2))
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false

    CShape* shapeA=this;
    CShape* shapeB=it;
    C7Vector shapeATr=shapeA->getFullCumulativeTransformation();
    C7Vector shapeBTr=shapeB->getFullCumulativeTransformation();
    shapeA->initializeCalculationStructureIfNeeded();
    shapeB->initializeCalculationStructureIfNeeded();
    C3Vector ptOnShapeA;
    C3Vector ptOnShapeB;

    bool smaller=false;
    if (CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeA->geomData->collInfo)<CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeB->geomData->collInfo))
        smaller=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeA->geomData->collInfo,shapeATr,shapeB->geomData->collInfo,shapeBTr,dist,&ptOnShapeA,&ptOnShapeB,&buffer[0],&buffer[1]);
    else
        smaller=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeB->geomData->collInfo,shapeBTr,shapeA->geomData->collInfo,shapeATr,dist,&ptOnShapeB,&ptOnShapeA,&buffer[1],&buffer[0]);

    if (smaller)
    {
        ray[0]=ptOnShapeA(0);
        ray[1]=ptOnShapeA(1);
        ray[2]=ptOnShapeA(2);
        ray[3]=ptOnShapeB(0);
        ray[4]=ptOnShapeB(1);
        ray[5]=ptOnShapeB(2);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

void CShape::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

CSceneObject* CShape::copyYourself()
{   
    CShape* newShape=(CShape*)CSceneObject::copyYourself();

    if (geomData!=nullptr)
        newShape->geomData=geomData->copyYourself();

    delete newShape->_dynMaterial;
    newShape->_dynMaterial=_dynMaterial->copyYourself();

    // Various:
    newShape->_startInDynamicSleeping=_startInDynamicSleeping;
    newShape->_shapeIsDynamicallyStatic=_shapeIsDynamicallyStatic;
    newShape->_shapeIsDynamicallyRespondable=_shapeIsDynamicallyRespondable;
    newShape->_dynamicCollisionMask=_dynamicCollisionMask;
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
    EASYLOCK(_objectMutex);
    displayShape(this,renderingObject,displayAttrib);
}

bool CShape::setParent(CSceneObject* newParent,bool keepObjectInPlace)
{ // Overridden from CSceneObject
    bool retVal=CSceneObject::setParent(newParent,keepObjectInPlace);
    if ( retVal&&(newParent!=nullptr) )
    {
        CSceneObject* thisObject=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandle);
        CSceneObject* _newParent=App::currentWorld->sceneObjects->getObjectFromHandle(newParent->getObjectHandle());
        if ( (thisObject!=nullptr)&&(_newParent!=nullptr) )
        { // both objects are in the world
            if (getSetAutomaticallyToNonStaticIfGetsParent())
                setShapeIsDynamicallyStatic(false);
        }
    }
    return(retVal);
}
