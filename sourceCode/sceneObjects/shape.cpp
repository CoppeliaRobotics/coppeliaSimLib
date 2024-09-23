#include <simInternal.h>
#include <shape.h>
#include <tt.h>
#include <algos.h>
#include <app.h>
#include <meshWrapper.h>
#include <mesh.h>
#include <meshManip.h>
#include <base64.h>
#include <imgLoaderSaver.h>
#include <utils.h>
#include <meshRoutines.h>
#ifdef SIM_WITH_GUI
#include <shapeRendering.h>
#endif

CShape::CShape()
{
    commonInit();
}

CShape::CShape(const std::vector<double> &allHeights, int xSize, int ySize, double dx, double minH, double maxH)
{ // heightfields
    commonInit();

    std::vector<double> vertices;
    std::vector<int> indices;
    double yPos = -double(ySize - 1) * dx * 0.5;
    for (int i = 0; i < ySize; i++)
    {
        double xPos = -double(xSize - 1) * dx * 0.5;
        for (int j = 0; j < xSize; j++)
        {
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(allHeights[i * xSize + j]);
            xPos += dx;
        }
        yPos += dx;
    }

    for (int i = 0; i < ySize - 1; i++)
    {
        for (int j = 0; j < xSize - 1; j++)
        {
            indices.push_back((i + 0) * xSize + (j + 0));
            indices.push_back((i + 0) * xSize + (j + 1));
            indices.push_back((i + 1) * xSize + (j + 0));

            indices.push_back((i + 0) * xSize + (j + 1));
            indices.push_back((i + 1) * xSize + (j + 1));
            indices.push_back((i + 1) * xSize + (j + 0));
        }
    }

    CMesh *newMesh = new CMesh(_localTransformation, vertices, indices, nullptr, nullptr, 0);
    replaceMesh(newMesh, false);
    newMesh->setPurePrimitiveType(sim_primitiveshape_heightfield, double(xSize - 1) * dx, double(ySize - 1) * dx, maxH - minH);
    std::vector<double> heightsInCorrectOrder;
    for (int i = 0; i < ySize; i++)
    {
        // Following doesn't work correctly somehow...
        // for (int j=xSize-1;j>=0;j--)
        //  heightsInCorrectOrder.push_back(allHeights[i*xSize+j]);
        for (int j = 0; j < xSize; j++)
            heightsInCorrectOrder.push_back(allHeights[i * xSize + j]); //  + (minH + maxH) * 0.5
    }
    newMesh->setHeightfieldData(heightsInCorrectOrder, xSize, ySize);
}

CMeshWrapper *CShape::detachMesh()
{
    CMeshWrapper *retVal = _mesh;
    _mesh = nullptr;
    return (retVal);
}

void CShape::replaceMesh(CMeshWrapper *newMesh, bool keepMeshAttributes)
{
    removeMeshCalculationStructure();
    if (_mesh != nullptr)
    {
        if (keepMeshAttributes)
        {
            _mesh->copyAttributesTo(newMesh);
            // textureObj->addDependentObject(h,shape->getSingleMesh()->getUniqueID());
        }
        delete _mesh;
    }
    _mesh = newMesh;
    computeBoundingBox();
    _meshModificationCounter++;
    actualizeContainsTransparentComponent();
    pushObjectRefreshEvent();
}

CShape::CShape(const C7Vector &transformation, const std::vector<double> &vertices, const std::vector<int> &indices,
               const std::vector<double> *optNormals, const std::vector<float> *optTexCoords, int options)
{ // all types of meshes, except heightfields
    commonInit();
    _localTransformation = transformation;
    CMesh *newMesh = new CMesh(_localTransformation, vertices, indices, optNormals, optTexCoords, options);
    replaceMesh(newMesh, false);
}

CShape::~CShape()
{
    removeMeshCalculationStructure();
    delete _mesh;
    delete _dynMaterial;
}

void CShape::invertFrontBack()
{
    getMesh()->flipFaces();
    removeMeshCalculationStructure(); // proximity sensors might check for the side!
}

int CShape::getMeshModificationCounter()
{
    return (_meshModificationCounter);
}

CMeshWrapper *CShape::getMesh() const
{
    return (_mesh);
}

CMesh *CShape::getSingleMesh() const
{
    CMesh *retVal = nullptr;
    if (_mesh->isMesh())
        retVal = (CMesh *)_mesh;
    return (retVal);
}

CMesh* CShape::getMeshFromUid(int meshUid, const C7Vector& parentCumulTr, C7Vector& shapeRelTr) const
{
    return _mesh->getMeshFromUid(meshUid, parentCumulTr, shapeRelTr);
}

CDynMaterialObject *CShape::getDynMaterial()
{
    return (_dynMaterial);
}

void CShape::setDynMaterial(CDynMaterialObject *mat)
{
    delete _dynMaterial;
    _dynMaterial = mat;
    if (_isInScene)
        _dynMaterial->setShapeHandleForEvents(_objectHandle);
    else
        _dynMaterial->setShapeHandleForEvents(-1);
}

C3Vector CShape::getInitialDynamicLinearVelocity()
{
    return (_initialDynamicLinearVelocity);
}

void CShape::setInitialDynamicLinearVelocity(const C3Vector &vel)
{
    bool diff = (_initialDynamicLinearVelocity != vel);
    if (diff)
    {
        _initialDynamicLinearVelocity = vel;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_initLinearVelocity.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _initialDynamicLinearVelocity.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

C3Vector CShape::getInitialDynamicAngularVelocity()
{
    return (_initialDynamicAngularVelocity);
}

void CShape::setInitialDynamicAngularVelocity(const C3Vector &vel)
{
    bool diff = (_initialDynamicAngularVelocity != vel);
    if (diff)
    {
        _initialDynamicAngularVelocity = vel;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_initAngularVelocity.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _initialDynamicAngularVelocity.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CShape::setRigidBodyWasAlreadyPutToSleepOnce(bool s)
{
    _rigidBodyWasAlreadyPutToSleepOnce = s;
}

bool CShape::getRigidBodyWasAlreadyPutToSleepOnce()
{
    return (_rigidBodyWasAlreadyPutToSleepOnce);
}

void CShape::actualizeContainsTransparentComponent()
{
    _containsTransparentComponents = getMesh()->getContainsTransparentComponents();
}

bool CShape::getContainsTransparentComponent()
{
    return (_containsTransparentComponents);
}

void CShape::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{
    getMesh()->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CShape::setRespondableMask(int m)
{
    bool diff = (_respondableMask != m);
    if (diff)
    {
        _respondableMask = m;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_respondableMask.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _respondableMask);
            App::worldContainer->pushEvent();
        }
    }
}

int CShape::getRespondableMask()
{
    return _respondableMask;
}

CSceneObject *CShape::getLastParentForLocalGlobalRespondable()
{
    CSceneObject *retVal;
    if (_lastParentForLocalGlobalRespondable == nullptr)
    {
        retVal = this;
        while (retVal->getParent() != nullptr)
            retVal = retVal->getParent();
        _lastParentForLocalGlobalRespondable = retVal;
    }
    else
        retVal = _lastParentForLocalGlobalRespondable;
    return (retVal);
}

void CShape::clearLastParentForLocalGlobalRespondable()
{
    _lastParentForLocalGlobalRespondable = nullptr;
}

std::string CShape::getObjectTypeInfo() const
{
    return ("shape");
}

std::string CShape::getObjectTypeInfoExtended() const
{
    if (getMesh()->isMesh())
    {
        int pureType = getSingleMesh()->getPurePrimitiveType();
        if (pureType == sim_primitiveshape_none)
            return ("shape");
        if (pureType == sim_primitiveshape_plane)
            return ("shape (plane)");
        if (pureType == sim_primitiveshape_disc)
            return ("shape (disc)");
        if (pureType == sim_primitiveshape_cuboid)
            return ("shape (cuboid)");
        if (pureType == sim_primitiveshape_spheroid)
            return ("shape (spheroid)");
        if (pureType == sim_primitiveshape_cylinder)
            return ("shape (cylinder)");
        if (pureType == sim_primitiveshape_cone)
            return ("shape (cone)");
        if (pureType == sim_primitiveshape_capsule)
            return ("shape (capsule)");
        return ("shape (heightfield)");
    }
    else
    {
        if (!getMesh()->isPure())
            return ("shape (compound)");
        return ("shape (compound primitive)");
    }
}
bool CShape::isPotentiallyCollidable() const
{
    return (true);
}
bool CShape::isPotentiallyMeasurable() const
{
    return (true);
}
bool CShape::isPotentiallyDetectable() const
{
    return (true);
}
bool CShape::isPotentiallyRenderable() const
{
    return (true);
}

void CShape::computeBoundingBox()
{
    C3Vector s;
    C7Vector fr(getMesh()->getBB(&s));
    s *= 0.5;
    _setBB(fr, s);
}

void CShape::commonInit()
{
    _objectType = sim_sceneobject_shape;
    _containsTransparentComponents = false;
    _startInDynamicSleeping = false;
    _shapeIsDynamicallyStatic = true;
    _shapeIsDynamicallyKinematic = true;
    _setAutomaticallyToNonStaticIfGetsParent = false;
    _shapeIsDynamicallyRespondable = false; // keep false, otherwise too many "default" problems
    _respondableSuspendCount = 0;
    _respondableMask = 0xffff;
    _lastParentForLocalGlobalRespondable = nullptr;
    _initialDynamicLinearVelocity.clear();
    _initialDynamicAngularVelocity.clear();

    _visibilityLayer = SHAPE_LAYER;
    _localObjectSpecialProperty = sim_objectspecialproperty_collidable | sim_objectspecialproperty_measurable |
                                  sim_objectspecialproperty_detectable | sim_objectspecialproperty_renderable;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();

    _meshCalculationStructure = nullptr;
    _mesh = nullptr;
    _meshModificationCounter = 0;

    _dynMaterial = new CDynMaterialObject();
}

void CShape::setDynamicVelocity(const C3Vector &linearV, const C3Vector &angularV)
{
    bool diff = ( (_dynamicLinearVelocity != linearV) || (_dynamicAngularVelocity != angularV) );
    if (diff)
    {
        _dynamicLinearVelocity = linearV;
        _dynamicAngularVelocity = angularV;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_dynLinearVelocity.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _dynamicLinearVelocity.data, 3);
            ev->appendKeyDoubleArray(propShape_dynAngularVelocity.name, _dynamicAngularVelocity.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

C3Vector CShape::getDynamicLinearVelocity()
{
    return (_dynamicLinearVelocity);
}

C3Vector CShape::getDynamicAngularVelocity()
{
    return (_dynamicAngularVelocity);
}

void CShape::addAdditionalForceAndTorque(const C3Vector &f, const C3Vector &t)
{
    _additionalForce += f;
    _additionalTorque += t;
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
    return (_additionalForce);
}

C3Vector CShape::getAdditionalTorque()
{
    return (_additionalTorque);
}

void CShape::setRespondable(bool r)
{
    bool diff = (_shapeIsDynamicallyRespondable != r);
    if (diff)
    {
        _shapeIsDynamicallyRespondable = r;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_respondable.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _shapeIsDynamicallyRespondable);
            App::worldContainer->pushEvent();
        }
    }
}

bool CShape::getRespondable()
{
    return (_shapeIsDynamicallyRespondable && (_respondableSuspendCount == 0));
}

bool CShape::getSetAutomaticallyToNonStaticIfGetsParent()
{
    return (_setAutomaticallyToNonStaticIfGetsParent);
}

void CShape::setSetAutomaticallyToNonStaticIfGetsParent(bool autoNonStatic)
{
    bool diff = (_setAutomaticallyToNonStaticIfGetsParent != autoNonStatic);
    if (diff)
    {
        _setAutomaticallyToNonStaticIfGetsParent = autoNonStatic;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_setToDynamicWithParent.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _setAutomaticallyToNonStaticIfGetsParent);
            App::worldContainer->pushEvent();
        }
    }
}

bool CShape::getStartInDynamicSleeping() const
{
    return (_startInDynamicSleeping);
}

void CShape::setStartInDynamicSleeping(bool sleeping)
{
    bool diff = (_startInDynamicSleeping != sleeping);
    if (diff)
    {
        _startInDynamicSleeping = sleeping;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_startInDynSleepMode.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _startInDynamicSleeping);
            App::worldContainer->pushEvent();
        }
    }
}

bool CShape::getStatic() const
{
    return (_shapeIsDynamicallyStatic);
}

void CShape::setStatic(bool sta)
{
    bool diff = (_shapeIsDynamicallyStatic != sta);
    if (diff)
    {
        _shapeIsDynamicallyStatic = sta;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_dynamic.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, !_shapeIsDynamicallyStatic);
            App::worldContainer->pushEvent();
        }
        if (!sta)
            setSetAutomaticallyToNonStaticIfGetsParent(false);
    }
}

bool CShape::getDynKinematic() const
{
    bool retVal = _shapeIsDynamicallyKinematic;
    // commented on 12.06.2024
    // if (_objectAlias == "Floor")
    //    retVal = false;
    return (retVal);
}

void CShape::setDynKinematic(bool kin)
{
    // commented on 12.06.2024
    // if (_objectAlias == "Floor")
    //    kin = false;
    bool diff = (_shapeIsDynamicallyKinematic != kin);
    if (diff)
    {
        _shapeIsDynamicallyKinematic = kin;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propShape_kinematic.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _shapeIsDynamicallyKinematic);
            App::worldContainer->pushEvent();
        }
    }
}

void CShape::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    if (getMesh()->isMesh())
        getSingleMesh()->setInsideAndOutsideFacesSameColor_DEPRECATED(s);
}
bool CShape::getInsideAndOutsideFacesSameColor_DEPRECATED()
{
    if (getMesh()->isMesh())
        return (getSingleMesh()->getInsideAndOutsideFacesSameColor_DEPRECATED());
    return (true);
}

bool CShape::isCompound() const
{
    return (!getMesh()->isMesh());
}

int CShape::getEdgeWidth_DEPRECATED() const
{
    if (getMesh()->isMesh())
        return (getSingleMesh()->getEdgeWidth_DEPRECATED());
    return (0);
}

void CShape::setEdgeWidth_DEPRECATED(int w)
{
    w = tt::getLimitedInt(1, 4, w);
    if (getMesh()->isMesh())
        getSingleMesh()->setEdgeWidth_DEPRECATED(w);
}

void CShape::display_extRenderer(CViewableBase *renderingObject, int displayAttrib)
{
    if (getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if (renderingObject->isObjectInsideView(getFullCumulativeTransformation() * getBB(nullptr), getBBHSize()))
        { // the bounding box is inside of the view (at least some part of it!)
            C7Vector tr = getCumulativeTransformation();
            int componentIndex = 0;
            getMesh()->display_extRenderer(C7Vector::identityTransformation, this, displayAttrib, tr, _objectHandle,
                                           componentIndex);
        }
    }
}

void CShape::scaleObject(double scalingFactor)
{
    _meshModificationCounter++;
    // Scale collision info
    if (_meshCalculationStructure != nullptr)
        App::worldContainer->pluginContainer->geomPlugin_scaleMesh(_meshCalculationStructure, scalingFactor);
    // Scale meshes and adjust textures:
    getMesh()->scale(scalingFactor);
    _dynamicsResetFlag = true;

    CSceneObject::scaleObject(scalingFactor);
}

bool CShape::scaleObjectNonIsometrically(double x, double y, double z)
{ // only shapes (and only certain shapes) can be scaled in an non-iso fashion
    bool retVal = true;
    if (getMesh()->isMesh())
    {
        if (getMesh()->isPure())
        { // we have some constraints in case we have a primitive mesh
            int purePrim = getSingleMesh()->getPurePrimitiveType();
            if ((purePrim == sim_primitiveshape_plane) || (purePrim == sim_primitiveshape_disc))
                z = 1.0;
            if ((x >= 0.00001) && (y >= 0.00001) && (z >= 0.00001))
            { // no x/y/z flipping for primitives
                if (purePrim == sim_primitiveshape_disc)
                    y = x;
                if ((purePrim == sim_primitiveshape_spheroid) || (purePrim == sim_primitiveshape_capsule))
                {
                    y = x;
                    z = x;
                }
                if ((purePrim == sim_primitiveshape_cylinder) || (purePrim == sim_primitiveshape_cone) ||
                    (purePrim == sim_primitiveshape_heightfield))
                    y = x;
            }
            else
                retVal = false;
        }

        if (retVal)
        {
            C3Vector s;
            _mesh->getBB(&s);
            if (s(0) == 0.0)
                x = 1.0;
            if (s(1) == 0.0)
                y = 1.0;
            if (s(2) == 0.0)
                z = 1.0;
            if ((fabs(x) >= 0.00001) && (fabs(y) >= 0.00001) && (fabs(z) >= 0.00001))
            { // Make sure we do not have too small scalings
                _meshModificationCounter++;
                removeMeshCalculationStructure(); // we have to recompute it!

                // Scale meshes and adjust textures:
                getSingleMesh()->scale(x, y, z);
                _dynamicsResetFlag = true;

                _sizeFactor *= cbrt(x * y * z);
                _sizeValues[0] *= x;
                _sizeValues[1] *= y;
                _sizeValues[2] *= z;

                computeBoundingBox();
                pushObjectRefreshEvent();
            }
            else
                retVal = false;
        }
    }
    else
        retVal = CSceneObject::scaleObjectNonIsometrically(x, y, z); // we have a compound, do an iso scaling
    return (retVal);
}

void CShape::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
    if (getMesh() != nullptr)
        getMesh()->announceSceneObjectWillBeErased(object); // for textures based on vision sensors
}

void CShape::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void CShape::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CShape::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CShape::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CShape::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
    getMesh()->performSceneObjectLoadingMapping(map);
}
void CShape::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}
void CShape::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}
void CShape::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}
void CShape::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CShape::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
    getMesh()->performTextureObjectLoadingMapping(map);
}

void CShape::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
    getMesh()->performDynMaterialObjectLoadingMapping(map);
}

void CShape::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    setDynamicVelocity(C3Vector::zeroVector, C3Vector::zeroVector);
    _additionalForce.clear();
    _additionalTorque.clear();
    _initialInitialDynamicLinearVelocity = _initialDynamicLinearVelocity;
    _initialInitialDynamicAngularVelocity = _initialDynamicAngularVelocity;
    _respondableSuspendCount = 0;

    actualizeContainsTransparentComponent(); // added on 2010/11/22 to correct at least each time a simulation starts,
                                             // when those values where not set correctly
}

void CShape::simulationAboutToStart()
{
    initializeInitialValues(false);
    _rigidBodyWasAlreadyPutToSleepOnce = false;
    CSceneObject::simulationAboutToStart();
}

void CShape::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
            setInitialDynamicLinearVelocity(_initialInitialDynamicLinearVelocity);
            setInitialDynamicAngularVelocity(_initialInitialDynamicAngularVelocity);
        }
    }

    setDynamicVelocity(C3Vector::zeroVector, C3Vector::zeroVector);
    _additionalForce.clear();
    _additionalTorque.clear();
    _respondableSuspendCount = 0;
    CSceneObject::simulationEnded();
}

void CShape::serialize(CSer &ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Ge2");
            ar.setCountingMode();
            _serializeMesh(ar);
            if (ar.setWritingMode())
                _serializeMesh(ar);

            ar.storeDataName("Mat");
            ar.setCountingMode();
            _dynMaterial->serialize(ar);
            if (ar.setWritingMode())
                _dynMaterial->serialize(ar);

            ar.storeDataName("Dc2");
            ar << (unsigned short)_respondableMask;
            ar.flush();

            ar.storeDataName("_dv");
            ar << _initialDynamicLinearVelocity(0) << _initialDynamicLinearVelocity(1)
               << _initialDynamicLinearVelocity(2);
            ar << _initialDynamicAngularVelocity(0) << _initialDynamicAngularVelocity(1)
               << _initialDynamicAngularVelocity(2);
            ar.flush();

            ar.storeDataName("Sss");
            unsigned char nothing = 0;
            //        SIM_SET_CLEAR_BIT(nothing,0,_shapeIsDynamicallyKinematic); reserved since 2022
            SIM_SET_CLEAR_BIT(nothing, 1, !_shapeIsDynamicallyKinematic);
            //      SIM_SET_CLEAR_BIT(nothing,2,_culling); removed on 11/11/2012
            //      SIM_SET_CLEAR_BIT(nothing,3,tracing); removed on 13/09/2011
            //      SIM_SET_CLEAR_BIT(nothing,4,_shapeWireframe); removed on 11/11/2012
            //      SIM_SET_CLEAR_BIT(nothing,5,_displayInvertedFaces); removed on 2010/04/19
            SIM_SET_CLEAR_BIT(nothing, 6, _startInDynamicSleeping);
            SIM_SET_CLEAR_BIT(nothing, 7, !_shapeIsDynamicallyStatic);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Ss3");
            nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _shapeIsDynamicallyRespondable);
            //      SIM_SET_CLEAR_BIT(nothing,1,_visualizeInertia); // removed on 16/12/2012
            SIM_SET_CLEAR_BIT(nothing, 2, false); // removed on 01/05/2020
            //      SIM_SET_CLEAR_BIT(nothing,3,!_insideAndOutsideFacesSameColor); // removed on 11/11/2012
            //      SIM_SET_CLEAR_BIT(nothing,4,_containsTransparentComponents); // removed on 11/11/2012
            SIM_SET_CLEAR_BIT(nothing, 5, _setAutomaticallyToNonStaticIfGetsParent);
            ar << nothing;
            ar.flush();

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
                    if (theName.compare("Ge2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _serializeMesh(ar);
                    }

                    if (theName.compare("Mat") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _dynMaterial->serialize(ar);
                    }

                    if (theName.compare("Dc2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned short m;
                        ar >> m;
                        _respondableMask = m;
                    }

                    if (theName.compare("Idv") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        _initialDynamicLinearVelocity(0) = (double)bla;
                        _initialDynamicLinearVelocity(1) = (double)bli;
                        _initialDynamicLinearVelocity(2) = (double)blo;
                        ar >> bla >> bli >> blo;
                        _initialDynamicAngularVelocity(0) = (double)bla;
                        _initialDynamicAngularVelocity(1) = (double)bli;
                        _initialDynamicAngularVelocity(2) = (double)blo;
                    }

                    if (theName.compare("_dv") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _initialDynamicLinearVelocity(0) >> _initialDynamicLinearVelocity(1) >>
                            _initialDynamicLinearVelocity(2);
                        ar >> _initialDynamicAngularVelocity(0) >> _initialDynamicAngularVelocity(1) >>
                            _initialDynamicAngularVelocity(2);
                    }

                    if (theName == "Sss")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyKinematic = !SIM_IS_BIT_SET(nothing, 1);
                        _startInDynamicSleeping = SIM_IS_BIT_SET(nothing, 6);
                        _shapeIsDynamicallyStatic = !SIM_IS_BIT_SET(nothing, 7);
                    }
                    if (theName == "Ss2")
                    { // keep for backward compatibility (2010/07/12)
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable = SIM_IS_BIT_SET(nothing, 0);
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                    }
                    if (theName == "Ss3")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable = SIM_IS_BIT_SET(nothing, 0);
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                        _setAutomaticallyToNonStaticIfGetsParent = SIM_IS_BIT_SET(nothing, 5);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            actualizeContainsTransparentComponent();
            computeBoundingBox();
            //if (_objectAlias == "Floor")
            //    _shapeIsDynamicallyKinematic = false;
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (exhaustiveXml)
        { // for non-exhaustive, is done in CSceneObjectContainer
            if (ar.isStoring())
            {
                ar.xmlPushNewNode("meshProxy");
                _serializeMesh(ar);
                ar.xmlPopNode();

                ar.xmlPushNewNode("dynamics");
                ar.xmlAddNode_int("respondableMask", _respondableMask);
                C3Vector vel = _initialDynamicLinearVelocity;
                vel *= 180.0 / piValue;
                ar.xmlAddNode_floats("initialLinearVelocity", vel.data, 3);
                vel = _initialDynamicAngularVelocity;
                vel *= 180.0 / piValue;
                ar.xmlAddNode_floats("initialAngularVelocity", vel.data, 3);
                ar.xmlPushNewNode("switches");
                ar.xmlAddNode_bool("static", _shapeIsDynamicallyStatic);
                ar.xmlAddNode_bool("kinematic", _shapeIsDynamicallyKinematic);
                ar.xmlAddNode_bool("respondable", _shapeIsDynamicallyRespondable);
                // ar.xmlAddNode_bool("parentFollows",_parentFollowsDynamic); removed on 01/05/2020
                ar.xmlAddNode_bool("startSleeping", _startInDynamicSleeping);
                ar.xmlAddNode_bool("setToDynamicIfGetsParent", _setAutomaticallyToNonStaticIfGetsParent);
                ar.xmlPopNode();
                ar.xmlPushNewNode("material");
                _dynMaterial->serialize(ar);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            else
            {
                bool meshProxyNotPresent = true;
                if (ar.xmlPushChildNode("meshProxy", false))
                {
                    _serializeMesh(ar);
                    ar.xmlPopNode();
                    meshProxyNotPresent = false;
                }

                if (ar.xmlPushChildNode("dynamics"))
                {
                    int m;
                    ar.xmlGetNode_int("respondableMask", m);
                    _respondableMask = m;
                    C3Vector vel;
                    ar.xmlGetNode_floats("initialLinearVelocity", vel.data, 3);
                    _initialDynamicLinearVelocity = vel * piValue / 180.0;
                    ar.xmlGetNode_floats("initialAngularVelocity", vel.data, 3);
                    _initialDynamicAngularVelocity = vel * piValue / 180.0;
                    if (ar.xmlPushChildNode("switches"))
                    {
                        ar.xmlGetNode_bool("static", _shapeIsDynamicallyStatic);
                        ar.xmlGetNode_bool("kinematic", _shapeIsDynamicallyKinematic);
                        ar.xmlGetNode_bool("respondable", _shapeIsDynamicallyRespondable);
                        // ar.xmlGetNode_bool("parentFollows",_parentFollowsDynamic);  removed on 01/05/2020
                        ar.xmlGetNode_bool("startSleeping", _startInDynamicSleeping);
                        ar.xmlGetNode_bool("setToDynamicIfGetsParent", _setAutomaticallyToNonStaticIfGetsParent);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("material"))
                    {
                        _dynMaterial->serialize(ar);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                computeBoundingBox();
            }
        }
    }
}

void CShape::_serializeMesh(CSer &ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            // geomInfo has to be stored before collInfo!!!
            if (_mesh->isMesh())
                ar.storeDataName("Gst");
            else
                ar.storeDataName("Gsg");
            ar.setCountingMode();
            _mesh->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
            if (ar.setWritingMode())
                _mesh->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);

            // (if undo/redo under way, getSaveExistingCalculationStructuresTemp is false)
            if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp() &&
                isMeshCalculationStructureInitialized())
            {
                std::vector<unsigned char> serializationData;
                App::worldContainer->pluginContainer->geomPlugin_getMeshSerializationData(_meshCalculationStructure,
                                                                                          serializationData);
                ar.storeDataName("Coj");
                ar.setCountingMode(true);
                for (int i = 0; i < serializationData.size(); i++)
                    ar << serializationData[i];
                ar.flush(false);
                if (ar.setWritingMode(true))
                {
                    for (int i = 0; i < serializationData.size(); i++)
                        ar << serializationData[i];
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

                    if (theName.compare("Gst") == 0)
                    { // geometric
                        noHit = false;
                        ar >> byteQuantity;
                        delete _mesh;
                        _mesh = new CMesh();
                        ((CMesh *)_mesh)
                            ->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
                    }
                    if (theName.compare("Gsg") == 0)
                    { // geomWrap
                        noHit = false;
                        ar >> byteQuantity;
                        delete _mesh;
                        _mesh = new CMeshWrapper();
                        _mesh->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
                    }

                    if (theName.compare("Coj") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo never
                                            // stores calc structures)

                        std::vector<unsigned char> data;
                        data.reserve(byteQuantity);
                        unsigned char dummy;
                        for (int i = 0; i < byteQuantity; i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        _meshCalculationStructure =
                            App::worldContainer->pluginContainer->geomPlugin_getMeshFromSerializationData(&data[0]);
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
        if (ar.isStoring())
        {
            ar.clearIncrementCounter();
            if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp() &&
                isMeshCalculationStructureInitialized())
            {
                std::vector<unsigned char> collInfoData;
                App::worldContainer->pluginContainer->geomPlugin_getMeshSerializationData(_meshCalculationStructure,
                                                                                          collInfoData);
                ar.xmlPushNewNode("calculationStructure");
                if (ar.xmlSaveDataInline(int(collInfoData.size())))
                {
                    std::string str(base64_encode(&collInfoData[0], (unsigned int)collInfoData.size()));
                    ar.xmlAddNode_string("data_base64Coded", str.c_str());
                }
                else
                    ar.xmlAddNode_binFile("file", (std::string("calcStruct_") + getObjectAliasAndHandle()).c_str(),
                                          &collInfoData[0], collInfoData.size());
                ar.xmlPopNode();
            }

            if (_mesh->isMesh())
                ar.xmlPushNewNode("mesh");
            else
                ar.xmlPushNewNode("compound");
            _mesh->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
            ar.xmlPopNode();
        }
        else
        {
            if (ar.xmlPushChildNode("calculationStructure", false))
            {
                std::string str;
                if (ar.xmlGetNode_string("data_base64Coded", str, false))
                    str = base64_decode(str);
                else
                    ar.xmlGetNode_binFile("file", str);

                std::vector<double> wvert;
                std::vector<int> wind;
                _mesh->getCumulativeMeshes(C7Vector::identityTransformation, wvert, &wind, nullptr);
                _meshCalculationStructure =
                    App::worldContainer->pluginContainer->geomPlugin_getMeshFromSerializationData(
                        (unsigned char *)str.c_str());
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("mesh", false))
            {
                delete _mesh;
                _mesh = new CMesh();
                ((CMesh *)_mesh)
                    ->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
                ar.xmlPopNode();
            }
            else
            {
                if (ar.xmlPushChildNode("compound"))
                {
                    delete _mesh;
                    _mesh = new CMeshWrapper();
                    _mesh->serialize(ar, getObjectAliasAndHandle().c_str(), C7Vector::identityTransformation, true);
                    ar.xmlPopNode();
                }
            }
            computeBoundingBox();
        }
    }
}

bool CShape::computeMassAndInertia(double density)
{
    bool retVal = false;
    double mass = 0.0;
    C7Vector localTr;
    C3Vector diagI;

    if (_mesh->isPure())
        mass = App::worldContainer->pluginContainer->dyn_computeInertia(_objectHandle, localTr, diagI);
    else
    { // we use the convex hull
        std::vector<double> vert;
        _mesh->getCumulativeMeshes(C7Vector::identityTransformation, vert, nullptr, nullptr);
        std::vector<double> hull;
        std::vector<int> indices;
        if (CMeshRoutines::getConvexHull(vert, hull, indices))
            mass = App::worldContainer->pluginContainer->dyn_computePMI(hull, indices, localTr, diagI);
    }
    if (mass > 0.0)
    {
        retVal = true;
        C3X3Matrix im;
        im.clear();
        im(0, 0) = diagI(0);
        im(1, 1) = diagI(1);
        im(2, 2) = diagI(2);
        im = CMeshWrapper::getInertiaInNewFrame(localTr.Q, im, C4Vector::identityRotation);
        _mesh->setMass(density * mass / 1000.0);
        _mesh->setInertia(im);
        _mesh->setCOM(localTr.X);
    }

    return (retVal);
}

bool CShape::alignBB(const char *mode, const C7Vector *tr /*=nullptr*/)
{
    bool retVal = false;
    if (std::string(mode) == "world")
    {
        C7Vector shapeCumulTrInv(getCumulativeTransformation().getInverse());
        retVal = _mesh->reorientBB(&shapeCumulTrInv.Q);
    }
    if (std::string(mode) == "mesh")
        retVal = _mesh->reorientBB(nullptr);
    if ((std::string(mode) == "custom") && (tr != nullptr))
    {
        C7Vector shapeCumulTrInv(getCumulativeTransformation().getInverse() * tr[0]);
        retVal = _mesh->reorientBB(&shapeCumulTrInv.Q);
    }
    if (retVal)
    {
        computeBoundingBox();
        _meshModificationCounter++;
        pushObjectRefreshEvent();
    }
    return (retVal);
}

bool CShape::relocateFrame(const char *mode, const C7Vector *tr /*=nullptr*/)
{
    bool retVal = false;
    if ((!_mesh->isMesh()) || (!_mesh->isPure()))
    { // we have a compound, or a non-primitive
        retVal = true;
        if (std::string(mode) == "world")
        {
            C7Vector shapeCumulTr(getCumulativeTransformation());
            _mesh->setCOM(shapeCumulTr * _mesh->getCOM());
            _mesh->setInertia(
                CMeshWrapper::getInertiaInNewFrame(shapeCumulTr.Q, _mesh->getInertia(), C4Vector::identityRotation));
            _mesh->setBBFrame(shapeCumulTr * _mesh->getBB(nullptr));
            if (getSingleMesh() == nullptr)
            { // we have a compound
                for (size_t i = 0; i < _mesh->childList.size(); i++)
                    _mesh->childList[i]->setIFrame(shapeCumulTr * _mesh->childList[i]->getIFrame());
            }
            for (size_t i = 0; i < getChildCount(); i++)
            {
                CSceneObject *child = getChildFromIndex(i);
                child->setLocalTransformation(shapeCumulTr * child->getLocalTransformation());
            }
            setLocalTransformation(getFullParentCumulativeTransformation().getInverse());
        }
        if (std::string(mode) == "parent")
        {
            C7Vector localOld(_localTransformation);
            _mesh->setCOM(localOld * _mesh->getCOM());
            _mesh->setInertia(
                CMeshWrapper::getInertiaInNewFrame(localOld.Q, _mesh->getInertia(), C4Vector::identityRotation));
            _mesh->setBBFrame(localOld * _mesh->getBB(nullptr));
            if (getSingleMesh() == nullptr)
            { // we have a compound
                for (size_t i = 0; i < _mesh->childList.size(); i++)
                    _mesh->childList[i]->setIFrame(localOld * _mesh->childList[i]->getIFrame());
            }
            for (size_t i = 0; i < getChildCount(); i++)
            {
                CSceneObject *child = getChildFromIndex(i);
                child->setLocalTransformation(localOld * child->getLocalTransformation());
            }
            setLocalTransformation(C7Vector::identityTransformation);
        }
        if (std::string(mode) == "mesh")
        {
            C7Vector oldBBFrame(_mesh->getBB(nullptr));
            _mesh->setCOM(oldBBFrame.getInverse() * _mesh->getCOM());
            _mesh->setInertia(CMeshWrapper::getInertiaInNewFrame(oldBBFrame.getInverse().Q, _mesh->getInertia(),
                                                                 C4Vector::identityRotation));
            _mesh->setBBFrame(C7Vector::identityTransformation);
            if (getSingleMesh() == nullptr)
            { // we have a compound
                for (size_t i = 0; i < _mesh->childList.size(); i++)
                    _mesh->childList[i]->setIFrame(oldBBFrame.getInverse() * _mesh->childList[i]->getIFrame());
            }
            for (size_t i = 0; i < getChildCount(); i++)
            {
                CSceneObject *child = getChildFromIndex(i);
                child->setLocalTransformation(oldBBFrame.getInverse() * child->getLocalTransformation());
            }
            setLocalTransformation(_localTransformation * oldBBFrame);
        }
        if ((std::string(mode) == "custom") && (tr != nullptr))
        {
            C7Vector x(getCumulativeTransformation().getInverse() * tr[0]);
            _mesh->setCOM(x.getInverse() * _mesh->getCOM());
            _mesh->setInertia(
                CMeshWrapper::getInertiaInNewFrame(x.getInverse().Q, _mesh->getInertia(), C4Vector::identityRotation));
            _mesh->setBBFrame(C7Vector::identityTransformation);
            if (getSingleMesh() == nullptr)
            { // we have a compound
                for (size_t i = 0; i < _mesh->childList.size(); i++)
                    _mesh->childList[i]->setIFrame(x.getInverse() * _mesh->childList[i]->getIFrame());
            }
            for (size_t i = 0; i < getChildCount(); i++)
            {
                CSceneObject *child = getChildFromIndex(i);
                child->setLocalTransformation(x.getInverse() * child->getLocalTransformation());
            }
            setLocalTransformation(_localTransformation * x);
        }
        computeBoundingBox();
        _meshModificationCounter++;
        pushObjectRefreshEvent();
    }
    return (retVal);
}

void CShape::removeMeshCalculationStructure()
{
    TRACE_INTERNAL;
    if (_meshCalculationStructure != nullptr)
    {
        App::worldContainer->pluginContainer->geomPlugin_destroyMesh(_meshCalculationStructure);
        _meshCalculationStructure = nullptr;
    }
}

bool CShape::isMeshCalculationStructureInitialized()
{
    return (_meshCalculationStructure != nullptr);
}

C7Vector CShape::getCumulCenteredMeshFrame() const
{
    return (getCumulativeTransformation() * _mesh->getBB(nullptr));
}

void CShape::initializeMeshCalculationStructureIfNeeded()
{
    if ((_meshCalculationStructure == nullptr) && (_mesh != nullptr))
    {
        std::vector<double> wvert;
        std::vector<int> wind;
        // use a centered mesh (required by the geom plugin)
        _mesh->getCumulativeMeshes(_mesh->getBB(nullptr).getInverse(), wvert, &wind, nullptr);
        double maxTriSize = App::currentWorld->environment->getCalculationMaxTriangleSize();
        double minTriSize = (std::max<double>(std::max<double>(_bbHalfSize(0), _bbHalfSize(1)), _bbHalfSize(2))) * 2.0 *
                            App::currentWorld->environment->getCalculationMinRelTriangleSize();
        if (maxTriSize < minTriSize)
            maxTriSize = minTriSize;
        _meshCalculationStructure = App::worldContainer->pluginContainer->geomPlugin_createMesh(
            &wvert[0], (int)wvert.size(), &wind[0], (int)wind.size(), nullptr, maxTriSize,
            App::userSettings->triCountInOBB);
    }
}

bool CShape::getCulling() const
{
    if (getMesh()->isMesh())
        return (getSingleMesh()->getCulling());
    return (false);
}

void CShape::setCulling(bool culState)
{
    _mesh->setCulling(culState);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        const char *cmd = "color";
        CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->openKeyMap(cmd);
        ev->appendKeyBool("culling", culState);
        ev->appendKeyInt("index", 0);
        App::worldContainer->pushEvent();
    }
#endif
}

bool CShape::getVisibleEdges() const
{
    if (getMesh()->isMesh())
        return (getSingleMesh()->getVisibleEdges());
    return (false);
}

void CShape::setVisibleEdges(bool v)
{
    _mesh->setVisibleEdges(v);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        const char *cmd = "color";
        CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, false);
        ev->openKeyMap(cmd);
        ev->appendKeyBool("showEdges", v);
        ev->appendKeyInt("index", 0);
        App::worldContainer->pushEvent();
    }
#endif
}

double CShape::getShadingAngle() const
{
    double retVal = 0.0;
    if (getMesh()->isMesh())
        retVal = getSingleMesh()->getShadingAngle();
    return (retVal);
}

void CShape::setShadingAngle(double a)
{
    _mesh->setShadingAngle(a);
}

void CShape::setRespondableSuspendCount(int cnt)
{
    _respondableSuspendCount = cnt;
}

void CShape::decrementRespondableSuspendCount()
{
    if (_respondableSuspendCount > 0)
        _respondableSuspendCount--;
}

bool CShape::getHideEdgeBorders_OLD() const
{
    return (getMesh()->getHideEdgeBorders_OLD());
}

void CShape::setHideEdgeBorders_OLD(bool v)
{
    getMesh()->setHideEdgeBorders_OLD(v);
}

bool CShape::getShapeWireframe_OLD() const
{
    if (getMesh()->isMesh())
        return (getSingleMesh()->getWireframe_OLD());
    return (false);
}

void CShape::setShapeWireframe_OLD(bool w)
{
    if (getMesh()->isMesh())
        getSingleMesh()->setWireframe_OLD(w);
}

bool CShape::doesShapeCollideWithShape(CShape *collidee, std::vector<double> *intersections)
{ // If intersections is different from nullptr, we check for all intersections and
    // intersection segments are appended to the vector

    std::vector<double> _intersect;
    std::vector<double> *_intersectP = nullptr;
    if (intersections != nullptr)
        _intersectP = &_intersect;
    if (App::worldContainer->pluginContainer->geomPlugin_getMeshMeshCollision(
            _meshCalculationStructure, getCumulCenteredMeshFrame(), collidee->_meshCalculationStructure,
            collidee->getCumulCenteredMeshFrame(), _intersectP, nullptr, nullptr))
    { // There was a collision
        if (intersections != nullptr)
            intersections->insert(intersections->end(), _intersect.begin(), _intersect.end());
        return (true);
    }
    return (false);
}

bool CShape::getDistanceToDummy_IfSmaller(CDummy *dummy, double &dist, double ray[7], int &buffer)
{ // Distance is measured from this to dummy
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // Build the node only when needed. So do it right here!
    initializeMeshCalculationStructureIfNeeded();

    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector rayPart0;
    if (App::worldContainer->pluginContainer->geomPlugin_getMeshPointDistanceIfSmaller(
            _meshCalculationStructure, getCumulCenteredMeshFrame(), dummyPos, dist, &rayPart0, &buffer))
    {
        rayPart0.getData(ray + 0);
        dummyPos.getData(ray + 3);
        ray[6] = dist;
        return (true);
    }
    return (false);
}

bool CShape::getShapeShapeDistance_IfSmaller(CShape *it, double &dist, double ray[7], int buffer[2])
{ // this is shape number 1, 'it' is shape number 2 (for ordering (measured from 1 to 2))
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false

    CShape *shapeA = this;
    CShape *shapeB = it;
    C7Vector shapeATr = shapeA->getCumulCenteredMeshFrame();
    C7Vector shapeBTr = shapeB->getCumulCenteredMeshFrame();
    shapeA->initializeMeshCalculationStructureIfNeeded();
    shapeB->initializeMeshCalculationStructureIfNeeded();
    C3Vector ptOnShapeA;
    C3Vector ptOnShapeB;

    bool smaller = false;
    if (App::worldContainer->pluginContainer->geomPlugin_getMeshRootObbVolume(shapeA->_meshCalculationStructure) <
        App::worldContainer->pluginContainer->geomPlugin_getMeshRootObbVolume(shapeB->_meshCalculationStructure))
        smaller = App::worldContainer->pluginContainer->geomPlugin_getMeshMeshDistanceIfSmaller(
            shapeA->_meshCalculationStructure, shapeATr, shapeB->_meshCalculationStructure, shapeBTr, dist, &ptOnShapeA,
            &ptOnShapeB, &buffer[0], &buffer[1]);
    else
        smaller = App::worldContainer->pluginContainer->geomPlugin_getMeshMeshDistanceIfSmaller(
            shapeB->_meshCalculationStructure, shapeBTr, shapeA->_meshCalculationStructure, shapeATr, dist, &ptOnShapeB,
            &ptOnShapeA, &buffer[1], &buffer[0]);

    if (smaller)
    {
        ray[0] = ptOnShapeA(0);
        ray[1] = ptOnShapeA(1);
        ray[2] = ptOnShapeA(2);
        ray[3] = ptOnShapeB(0);
        ray[4] = ptOnShapeB(1);
        ray[5] = ptOnShapeB(2);
        ray[6] = dist;
        return (true);
    }
    return (false);
}

void CShape::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CShape::addSpecializedObjectEventData(CCbor *ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
#endif
    _dynMaterial->setBoolProperty(nullptr, false, ev);
    _dynMaterial->setIntProperty(nullptr, 0, ev);
    _dynMaterial->setFloatProperty(nullptr, 0.0, ev);
    _dynMaterial->setVector3Property(nullptr, nullptr, ev);
    _dynMaterial->setVectorProperty(nullptr, nullptr, 0, ev);
    _dynMaterial->sendEngineString(ev);
    ev->openKeyArray(propShape_meshes.name);
    std::vector<CMesh *> all;
    std::vector<C7Vector> allTr;
    getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, &allTr);
    for (size_t i = 0; i < all.size(); i++)
    {
        CMesh *geom = all[i];
#if SIM_EVENT_PROTOCOL_VERSION == 2
        C7Vector tr(allTr[i]);
        ev->openMap();

        const std::vector<float> *wvert = geom->getVerticesForDisplayAndDisk();
        const std::vector<int> *wind = geom->getIndices();
        const std::vector<float> *wnorm = geom->getNormalsForDisplayAndDisk();
        std::vector<float> vertices;
        vertices.resize(wvert->size());
        for (size_t j = 0; j < wvert->size() / 3; j++)
        {
            C3Vector v;
            v.setData(wvert->data() + j * 3);
            v = tr * v;
            vertices[3 * j + 0] = (float)v(0);
            vertices[3 * j + 1] = (float)v(1);
            vertices[3 * j + 2] = (float)v(2);
        }
        ev->appendKeyFloatArray("vertices", vertices.data(), vertices.size());
        ev->appendKeyIntArray("indices", wind->data(), wind->size());

        std::vector<float> normals;
        normals.resize(wind->size() * 3);
        for (size_t j = 0; j < wind->size(); j++)
        {
            C3Vector n;
            n.setData(&(wnorm[0])[0] + j * 3);
            n = tr.Q * n; // only orientation
            normals[3 * j + 0] = (float)n(0);
            normals[3 * j + 1] = (float)n(1);
            normals[3 * j + 2] = (float)n(2);
        }
        ev->appendKeyFloatArray("normals", normals.data(), normals.size());

        float c[9];
        geom->color.getColor(c + 0, sim_colorcomponent_ambient_diffuse);
        geom->color.getColor(c + 3, sim_colorcomponent_specular);
        geom->color.getColor(c + 6, sim_colorcomponent_emission);
        ev->appendKeyFloatArray("color", c, 9);
        ev->appendKeyDouble("shadingAngle", geom->getShadingAngle());
        ev->appendKeyBool("showEdges", geom->getVisibleEdges());
        ev->appendKeyBool("culling", geom->getCulling());
        double transp = 0.0;
        if (geom->color.getTranslucid())
            transp = 1.0 - geom->color.getOpacity();
        ev->appendKeyDouble("transparency", transp);

        int options = 0;
        if (geom->getCulling())
            options |= 1;
        if (geom->getWireframe_OLD())
            options |= 2;
        ev->appendKeyInt("options", options);

        CTextureProperty *tp = geom->getTextureProperty();
        CTextureObject *to = nullptr;
        const std::vector<float> *tc = nullptr;
        if (tp != nullptr)
        {
            to = tp->getTextureObject();
            tc = tp->getTextureCoordinates(-1, wvert[0], wind[0]);
        }

        if ((to != nullptr) && (tc != nullptr))
        {
            int tRes[2];
            to->getTextureSize(tRes[0], tRes[1]);
            ev->openKeyMap("texture");
            ev->appendKeyBuff("rawTexture", to->getTextureBufferPointer(), tRes[1] * tRes[0] * 4);
            ev->appendKeyIntArray("resolution", tRes, 2);
            ev->appendKeyFloatArray("coordinates", tc->data(), tc->size());
            ev->appendKeyInt("applyMode", tp->getApplyMode());

            int options = 0;
            if (tp->getRepeatU())
                options |= 1;
            if (tp->getRepeatV())
                options |= 2;
            if (tp->getInterpolateColors())
                options |= 4;
            ev->appendKeyInt("options", options);
            ev->appendKeyInt("id", tp->getTextureObjectID());
            ev->closeArrayOrMap(); // texture
        }
        ev->closeArrayOrMap(); // one mesh
#else
        ev->appendInt(geom->getUniqueID());
#endif
    }
    ev->closeArrayOrMap(); // meshes
    ev->appendKeyInt(propShape_respondableMask.name, _respondableMask);
    ev->appendKeyBool(propShape_startInDynSleepMode.name, _startInDynamicSleeping);
    ev->appendKeyBool(propShape_dynamic.name, !_shapeIsDynamicallyStatic);
    ev->appendKeyBool(propShape_kinematic.name, _shapeIsDynamicallyKinematic);
    ev->appendKeyBool(propShape_respondable.name, _shapeIsDynamicallyRespondable);
    ev->appendKeyBool(propShape_setToDynamicWithParent.name, _setAutomaticallyToNonStaticIfGetsParent);
    ev->appendKeyDoubleArray(propShape_initLinearVelocity.name, _initialDynamicLinearVelocity.data, 3);
    ev->appendKeyDoubleArray(propShape_initAngularVelocity.name, _initialDynamicAngularVelocity.data, 3);
    ev->appendKeyDoubleArray(propShape_dynLinearVelocity.name, _dynamicLinearVelocity.data, 3);
    ev->appendKeyDoubleArray(propShape_dynAngularVelocity.name, _dynamicAngularVelocity.data, 3);

#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // shape
#endif
}

void CShape::copyAttributesTo(CShape *target)
{ // target should not yet be in the scene. Used when generating a shape from another
    delete target->_dynMaterial;
    target->_dynMaterial = _dynMaterial->copyYourself();
    target->_shapeIsDynamicallyKinematic = _shapeIsDynamicallyKinematic;
    target->_startInDynamicSleeping = _startInDynamicSleeping;
    target->_shapeIsDynamicallyStatic = _shapeIsDynamicallyStatic;
    target->_shapeIsDynamicallyRespondable = _shapeIsDynamicallyRespondable;
    target->_respondableMask = _respondableMask;
    target->_setAutomaticallyToNonStaticIfGetsParent = _setAutomaticallyToNonStaticIfGetsParent;

    target->setVisibilityLayer(getVisibilityLayer()); // actually a CSceneObject property
}

CSceneObject *CShape::copyYourself()
{
    CShape *newShape = (CShape *)CSceneObject::copyYourself();

    if (_mesh != nullptr)
        newShape->_mesh = _mesh->copyYourself();

    if (_meshCalculationStructure != nullptr)
        newShape->_meshCalculationStructure =
            App::worldContainer->pluginContainer->geomPlugin_copyMesh(_meshCalculationStructure);

    delete newShape->_dynMaterial;
    newShape->_dynMaterial = _dynMaterial->copyYourself();

    // Various:
    newShape->_shapeIsDynamicallyKinematic = _shapeIsDynamicallyKinematic;
    newShape->_startInDynamicSleeping = _startInDynamicSleeping;
    newShape->_shapeIsDynamicallyStatic = _shapeIsDynamicallyStatic;
    newShape->_shapeIsDynamicallyRespondable = _shapeIsDynamicallyRespondable;
    newShape->_respondableMask = _respondableMask;
    newShape->_containsTransparentComponents = _containsTransparentComponents;
    newShape->_rigidBodyWasAlreadyPutToSleepOnce = _rigidBodyWasAlreadyPutToSleepOnce;
    newShape->_setAutomaticallyToNonStaticIfGetsParent = _setAutomaticallyToNonStaticIfGetsParent;
    newShape->_initialDynamicLinearVelocity = _initialDynamicLinearVelocity;
    newShape->_initialDynamicAngularVelocity = _initialDynamicAngularVelocity;

    newShape->_initialValuesInitialized = _initialValuesInitialized;

    return (newShape);
}

void CShape::setColor(const char *colorName, int colorComponent, float r, float g, float b)
{
    float rgb[3] = {r, g, b};
    setColor(colorName, colorComponent, rgb);
}

void CShape::setColor(const char *colorName, int colorComponent, const float *rgbData)
{
    int rgbDataOffset = 0;
    int cnt = 0;
    const CShape *s = nullptr;
    if (_isInScene)
        s = this;
    getMesh()->setColor(s, cnt, colorName, colorComponent, rgbData, rgbDataOffset);
    if (colorComponent == sim_colorcomponent_transparency)
        actualizeContainsTransparentComponent();
}

bool CShape::getColor(const char *colorName, int colorComponent, float *rgbData)
{
    int rgbDataOffset = 0;
    return (getMesh()->getColor(colorName, colorComponent, rgbData, rgbDataOffset));
}

int CShape::getComponentCount() const
{
    return (getMesh()->getComponentCount());
}

bool CShape::setParent(CSceneObject *newParent)
{ // Overridden from CSceneObject
    bool retVal = CSceneObject::setParent(newParent);
    if (retVal && getSetAutomaticallyToNonStaticIfGetsParent())
        setStatic(false);
    return (retVal);
}

#ifdef SIM_WITH_GUI
void CShape::displayInertia(CViewableBase *renderingObject, double size, bool persp)
{
    if (persp)
    {
        C7Vector x(renderingObject->getCumulativeTransformation().getInverse() * getCumulativeTransformation());
        size *= x(2);
    }
    C3Vector diag;
    C7Vector tr(getCumulativeTransformation() * getMesh()->getDiagonalInertiaInfo(diag));
    _displayInertia(tr, diag, size * 0.008);
}

void CShape::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayShape(this, renderingObject, displayAttrib);
}

void CShape::displayFrames(CViewableBase *renderingObject, double size, bool persp)
{
    CSceneObject::displayFrames(renderingObject, size, persp);
    if (persp)
    {
        C7Vector x(renderingObject->getCumulativeTransformation().getInverse() * getCumulativeTransformation());
        size *= x(2);
    }
    C7Vector tr(getCumulativeTransformation());
    _displayFrame(tr * _bbFrame, size * 0.006, 1); // frame of the bounding box
}
#endif

void CShape::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (getMesh() != nullptr)
    {
        std::vector<CMesh *> all;
        getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, nullptr);
        for (size_t i = 0; i < all.size(); i++)
        {
            if (s)
                all[i]->color.setEventParams(false, all[i]->getUniqueID());
            else
                all[i]->color.setEventParams(false, -1);
        }
    }
    if (s)
        _dynMaterial->setShapeHandleForEvents(_objectHandle);
    else
        _dynMaterial->setShapeHandleForEvents(-1);
}

int CShape::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setBoolProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->setBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propShape_applyCulling.name)
        {
            setCulling(pState);
            retVal = 1;
        }
        else if (_pName == propShape_applyShowEdges.name)
        {
            setVisibleEdges(pState);
            retVal = 1;
        }
        else if (_pName == propShape_flipFaces.name)
        {
            invertFrontBack();
            retVal = 1;
        }
        else if (_pName == propShape_startInDynSleepMode.name)
        {
            setStartInDynamicSleeping(pState);
            retVal = 1;
        }
        else if (_pName == propShape_dynamic.name)
        {
            setStatic(!pState);
            retVal = 1;
        }
        else if (_pName == propShape_kinematic.name)
        {
            setDynKinematic(pState);
            retVal = 1;
        }
        else if (_pName == propShape_respondable.name)
        {
            setRespondable(pState);
            retVal = 1;
        }
        else if (_pName == propShape_setToDynamicWithParent.name)
        {
            setSetAutomaticallyToNonStaticIfGetsParent(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getBoolProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propShape_startInDynSleepMode.name)
        {
            pState = _startInDynamicSleeping;
            retVal = 1;
        }
        else if (_pName == propShape_dynamic.name)
        {
            pState = !_shapeIsDynamicallyStatic;
            retVal = 1;
        }
        else if (_pName == propShape_kinematic.name)
        {
            pState = _shapeIsDynamicallyKinematic;
            retVal = 1;
        }
        else if (_pName == propShape_respondable.name)
        {
            pState = _shapeIsDynamicallyRespondable;
            retVal = 1;
        }
        else if (_pName == propShape_setToDynamicWithParent.name)
        {
            pState = _setAutomaticallyToNonStaticIfGetsParent;
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::setIntProperty(const char* ppName, int pState)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setIntProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->setIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propShape_respondableMask.name)
        {
            setRespondableMask(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getIntProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propShape_respondableMask.name)
        {
            pState = _respondableMask;
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propShape_applyShadingAngle.name)
        {
            setShadingAngle(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getFloatProperty(pName, pState);

    return retVal;
}

int CShape::setStringProperty(const char* ppName, const char* pState)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setStringProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->setStringProperty(pName, pState);

    return retVal;
}

int CShape::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getStringProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getStringProperty(pName, pState);

    return retVal;
}

int CShape::setVector3Property(const char* ppName, const C3Vector& pState)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setVector3Property(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->setVector3Property(pName, &pState);
    if (retVal == -1)
    {
        if (_pName == propShape_initLinearVelocity.name)
        {
            setInitialDynamicLinearVelocity(pState);
            retVal = 1;
        }
        else if (_pName == propShape_initAngularVelocity.name)
        {
            setInitialDynamicAngularVelocity(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getVector3Property(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getVector3Property(pName, &pState);
    if (retVal == -1)
    {
        if (_pName == propShape_initLinearVelocity.name)
        {
            pState = _initialDynamicLinearVelocity;
            retVal = 1;
        }
        else if (_pName == propShape_initAngularVelocity.name)
        {
            pState = _initialDynamicAngularVelocity;
            retVal = 1;
        }
        else if (_pName == propShape_dynLinearVelocity.name)
        {
            pState = _dynamicLinearVelocity;
            retVal = 1;
        }
        else if (_pName == propShape_dynAngularVelocity.name)
        {
            pState = _dynamicAngularVelocity;
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::setVectorProperty(const char* ppName, const double* v, int vL)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setVectorProperty(pName, v, vL);
    if (retVal == -1)
        retVal = _dynMaterial->setVectorProperty(pName, v, vL);

    return retVal;
}

int CShape::getVectorProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    pState.clear();
    int retVal = CSceneObject::getVectorProperty(pName, pState);
    if (retVal == -1)
        retVal = _dynMaterial->getVectorProperty(pName, pState);

    return retVal;
}

int CShape::setIntVectorProperty(const char* ppName, const int* v, int vL)
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setIntVectorProperty(pName, v, vL);

    return retVal;
}

int CShape::getIntVectorProperty(const char* ppName, std::vector<int>& pState) const
{
    std::string _pName( utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    pState.clear();
    int retVal = CSceneObject::getIntVectorProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(pName, propShape_meshes.name) == 0)
        {
            std::vector<CMesh *> all;
            getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, nullptr);
            for (size_t i = 0; i < all.size(); i++)
                pState.push_back(all[i]->getUniqueID());
            retVal = 1;
        }
    }

    return retVal;
}

int CShape::getPropertyName(int& index, std::string& pName, std::string& appartenance) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".shape";
        retVal = _dynMaterial->getPropertyName(index, pName);
        if (retVal == -1)
        {
            for (size_t i = 0; i < allProps_shape.size(); i++)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_shape[i].name;
                    //pName = "shape." + pName;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CShape::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".shape";
        retVal = CDynMaterialObject::getPropertyName_static(index, pName);
        if (retVal == -1)
        {
            for (size_t i = 0; i < allProps_shape.size(); i++)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_shape[i].name;
                    //pName = "shape." + pName;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CShape::getPropertyInfo(const char* ppName, int& info) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info);
    if (retVal == -1)
    {
        retVal = _dynMaterial->getPropertyInfo(pName, info);
        if (retVal == -1)
        {
            for (size_t i = 0; i < allProps_shape.size(); i++)
            {
                if (strcmp(allProps_shape[i].name, pName) == 0)
                {
                    retVal = allProps_shape[i].type;
                    info = allProps_shape[i].flags;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CShape::getPropertyInfo_static(const char* ppName, int& info)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "shape."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info);
    if (retVal == -1)
    {
        retVal = CDynMaterialObject::getPropertyInfo_static(pName, info);
        if (retVal == -1)
        {
            for (size_t i = 0; i < allProps_shape.size(); i++)
            {
                if (strcmp(allProps_shape[i].name, pName) == 0)
                {
                    retVal = allProps_shape[i].type;
                    info = allProps_shape[i].flags;
                    break;
                }
            }
        }
    }
    return retVal;
}


