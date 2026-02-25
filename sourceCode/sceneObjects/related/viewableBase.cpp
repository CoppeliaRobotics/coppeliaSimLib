#include <viewableBase.h>
#include <tt.h>
#include <simInternal.h>
#include <camera.h>
#include <vDateTime.h>
#include <simMath/4X4Matrix.h>
#include <app.h>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

bool CViewableBase::fogWasActivated = false;
bool CViewableBase::_frustumCulling = true;

bool CViewableBase::getFrustumCullingEnabled()
{
    return (_frustumCulling);
}

void CViewableBase::setFrustumCullingEnabled(bool e)
{
    _frustumCulling = e;
}

CViewableBase::CViewableBase()
{
    _planesCalculated = false;
    _fogTimer = (int)VDateTime::getTimeInMs() - 1;
    _fogTimerDuration = 0.0;
    _frustumCullingTemporarilyDisabled = false;
    _disabledColorComponents = 0;
    _perspective = true;
    _viewAngle = 60.0 * piValue / 180.0;
    _orthoViewSize = 1.0;
    _resolution[0] = 1280;
    _resolution[1] = 720;
    _volumeVectorNear.clear();
    _volumeVectorFar.clear();
}

CViewableBase::~CViewableBase()
{
}

int CViewableBase::getDisabledColorComponents()
{
    return (_disabledColorComponents);
}

void CViewableBase::setDisabledColorComponents(int c)
{
    _disabledColorComponents = c;
}

CSceneObject* CViewableBase::copyYourself()
{
    return (nullptr);
}
void CViewableBase::display(CViewableBase* renderingObject, int displayAttrib)
{
}
void CViewableBase::scaleObject(double scalingFactor)
{
}
void CViewableBase::simulationAboutToStart()
{
}
void CViewableBase::simulationEnded()
{
    //  if (_initialValuesInitialized)
    //  {
    //  }
}

void CViewableBase::setUseLocalLights(bool l)
{
    _useLocalLights = l;
}

bool CViewableBase::getuseLocalLights()
{
    return (_useLocalLights);
}

void CViewableBase::serialize(CSer& ar)
{
}
void CViewableBase::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{
}
void CViewableBase::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{
}
void CViewableBase::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{
}
void CViewableBase::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{
}
void CViewableBase::performObjectLoadingMapping(const std::map<int, int>* map)
{
}
void CViewableBase::performCollectionLoadingMapping(const std::map<int, int>* map)
{
}
void CViewableBase::performCollisionLoadingMapping(const std::map<int, int>* map)
{
}
void CViewableBase::performDistanceLoadingMapping(const std::map<int, int>* map)
{
}
std::string CViewableBase::getObjectTypeInfo() const
{
    return ("");
}
std::string CViewableBase::getObjectTypeInfoExtended() const
{
    return ("");
}
bool CViewableBase::isPotentiallyCollidable() const
{
    return (false);
}
bool CViewableBase::isPotentiallyMeasurable() const
{
    return (false);
}
bool CViewableBase::isPotentiallyDetectable() const
{
    return (false);
}
bool CViewableBase::isPotentiallyRenderable() const
{
    return (false);
}

void CViewableBase::setClippingPlanes(double nearPlane, double farPlane)
{
    if (nearPlane < 0.0001)
        nearPlane = 0.0001;
    if (farPlane > 100000.0)
        farPlane = 100000.0;
    if (farPlane < nearPlane)
        farPlane = nearPlane;
    bool diff = ((_nearClippingPlane != nearPlane) || (_farClippingPlane != farPlane));
    if (diff)
    {
        _nearClippingPlane = nearPlane;
        _farClippingPlane = farPlane;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            if (App::getEventProtocolVersion() == 2)
            {
                const char* cmd = "nearClippingPlane";
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                ev->appendKeyDouble(cmd, _nearClippingPlane);
                ev->appendKeyDouble("farClippingPlane", _farClippingPlane);
            }
            else
            {
                const char* cmd = propViewable_clippingPlanes.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                double arr[2] = {_nearClippingPlane, _farClippingPlane};
                ev->appendKeyDoubleArray(cmd, arr, 2);
            }
            App::worldContainer->pushEvent();
        }
        computeVolumeVectors();
    }
}

void CViewableBase::getClippingPlanes(double& nearPlane, double& farPlane) const
{
    nearPlane = _nearClippingPlane;
    farPlane = _farClippingPlane;
}

void CViewableBase::setViewAngle(double angle)
{
    tt::limitValue(1.0 * degToRad, 135.0 * degToRad, angle); // with 90 degrees, objects disappear!! Really??? Changed to 135 on 2010/11/12
    bool diff = (_viewAngle != angle);
    if (diff)
    {
        _viewAngle = angle;
        computeVolumeVectors();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propViewable_viewAngle.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _viewAngle);
            App::worldContainer->pushEvent();
        }
    }
}

double CViewableBase::getViewAngle() const
{
    return _viewAngle;
}

void CViewableBase::setOrthoViewSize(double theSize)
{
    tt::limitValue(0.001, 200000.0, theSize);
    bool diff = (_orthoViewSize != theSize);
    if (diff)
    {
        _orthoViewSize = theSize;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            std::string cmd;
            if (App::getEventProtocolVersion() == 2)
                cmd = "orthoSize";
            else
                cmd = propViewable_viewSize.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd.c_str(), true);
            ev->appendKeyDouble(cmd.c_str(), _orthoViewSize);
            App::worldContainer->pushEvent();
        }
        computeVolumeVectors();
    }
}

double CViewableBase::getOrthoViewSize() const
{
    return _orthoViewSize;
}

void CViewableBase::setHideFog(bool hideFog)
{
    _hideFog = hideFog;
}

bool CViewableBase::getHideFog() const
{
    return _hideFog;
}

void CViewableBase::setFogTimer(double seconds)
{
    _fogTimerDuration = seconds;
    _fogTimer = (int)VDateTime::getTimeInMs() + int(seconds * 1000.0);
}

double CViewableBase::getFogStrength()
{
    if (_fogTimerDuration == 0.0)
        return (1.0);
    int t = (int)VDateTime::getTimeInMs();
    if (_fogTimer < t)
    {
        _fogTimerDuration = 0.0;
        return (1.0);
    }
    if (_fogTimer > t + 100000)
    {
        _fogTimerDuration = 0.0;
        return (1.0);
    }
    double dt = double(_fogTimer - t) / 1000.0;
    return (1.0 - (dt / _fogTimerDuration));
}

void CViewableBase::setFrustumCullingTemporarilyDisabled(bool d)
{
    _frustumCullingTemporarilyDisabled = d;
}

void CViewableBase::computeViewFrustumIfNeeded()
{
    if (!_planesCalculated)
    {
        _planesCalculated = true;
        C7Vector m;
        C7Vector upperPlane;
        C7Vector lowerPlane;
        C7Vector rightPlane;
        C7Vector leftPlane;
        C7Vector nearPlane;
        C7Vector farPlane;
        double winXSize = (double)_currentViewSize[0];
        double winYSize = (double)_currentViewSize[1];
        C7Vector viewableCumulTransf;

        if (getObjectType() == sim_sceneobject_camera)
        {
            viewableCumulTransf = getFullCumulativeTransformation();
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow->getHasStereo())
            { // handle stereo cameras correctly
                C7Vector displ;
                displ.setIdentity();
                if (GuiApp::mainWindow->getLeftEye())
                    displ.X(0) = GuiApp::mainWindow->getStereoDistance() / 2;
                else
                    displ.X(0) = -GuiApp::mainWindow->getStereoDistance() / 2;
                viewableCumulTransf = viewableCumulTransf * displ;
            }
#endif
        }
        else
            viewableCumulTransf = getFullCumulativeTransformation();

        if (_currentPerspective)
        {
            double xAngle, yAngle;
            if (winXSize < winYSize)
            {
                yAngle = _viewAngle * 0.5;
                xAngle = atan(tan(_viewAngle * 0.5) * winXSize / winYSize);
            }
            else
            {
                yAngle = atan(tan(_viewAngle * 0.5) * winYSize / winXSize);
                xAngle = _viewAngle * 0.5;
            }
            xAngle *= 1.05; // a little bit of tolerance!
            yAngle *= 1.05;
            upperPlane.X = viewableCumulTransf.X;
            lowerPlane.X = viewableCumulTransf.X;
            rightPlane.X = viewableCumulTransf.X;
            leftPlane.X = viewableCumulTransf.X;
            upperPlane.Q = viewableCumulTransf.Q * C4Vector(-yAngle, C3Vector(1.0, 0.0, 0.0));
            lowerPlane.Q = viewableCumulTransf.Q * C4Vector(+yAngle, C3Vector(1.0, 0.0, 0.0));
            rightPlane.Q = viewableCumulTransf.Q * C4Vector(-xAngle, C3Vector(0.0, 1.0, 0.0));
            leftPlane.Q = viewableCumulTransf.Q * C4Vector(+xAngle, C3Vector(0.0, 1.0, 0.0));
            nearPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, 0.0, _nearClippingPlane)));
            farPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, 0.0, _farClippingPlane)));
        }
        else
        {
            double xSize, ySize;
            if (winXSize < winYSize)
            {
                xSize = _orthoViewSize * 0.5 * winXSize / winYSize;
                ySize = _orthoViewSize * 0.5;
            }
            else
            {
                xSize = _orthoViewSize * 0.5;
                ySize = _orthoViewSize * 0.5 * winYSize / winXSize;
            }
            xSize *= 1.05; // a little bit of tolerance!
            ySize *= 1.05;
            upperPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, +ySize, 0.0)));
            lowerPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, -ySize, 0.0)));
            rightPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(-xSize, 0.0, 0.0)));
            leftPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(+xSize, 0.0, 0.0)));
            if (getObjectType() == sim_sceneobject_camera)
            { // we are in othoview!
                nearPlane.setMultResult(viewableCumulTransf,
                                        C7Vector(C3Vector(0.0, 0.0, ORTHO_CAMERA_NEAR_CLIPPING_PLANE)));
                farPlane.setMultResult(viewableCumulTransf,
                                       C7Vector(C3Vector(0.0, 0.0, ORTHO_CAMERA_FAR_CLIPPING_PLANE)));
            }
            else
            {
                nearPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, 0.0, _nearClippingPlane)));
                farPlane.setMultResult(viewableCumulTransf, C7Vector(C3Vector(0.0, 0.0, _farClippingPlane)));
            }
        }
        viewFrustum.clear();

        // If the plane ordering changes, check also the volumeNode class!!
        C4X4Matrix upperP(upperPlane);
        viewFrustum.push_back(upperP.M.axis[1](0));
        viewFrustum.push_back(upperP.M.axis[1](1));
        viewFrustum.push_back(upperP.M.axis[1](2));
        viewFrustum.push_back(-(upperP.X * upperP.M.axis[1]));

        C4X4Matrix lowerP(lowerPlane);
        viewFrustum.push_back(-lowerP.M.axis[1](0));
        viewFrustum.push_back(-lowerP.M.axis[1](1));
        viewFrustum.push_back(-lowerP.M.axis[1](2));
        viewFrustum.push_back(lowerP.X * lowerP.M.axis[1]);

        C4X4Matrix rightP(rightPlane);
        viewFrustum.push_back(-rightP.M.axis[0](0));
        viewFrustum.push_back(-rightP.M.axis[0](1));
        viewFrustum.push_back(-rightP.M.axis[0](2));
        viewFrustum.push_back(rightP.X * rightP.M.axis[0]);

        C4X4Matrix leftP(leftPlane);
        viewFrustum.push_back(leftP.M.axis[0](0));
        viewFrustum.push_back(leftP.M.axis[0](1));
        viewFrustum.push_back(leftP.M.axis[0](2));
        viewFrustum.push_back(-(leftP.X * leftP.M.axis[0]));

        C4X4Matrix nearP(nearPlane);
        viewFrustum.push_back(-nearP.M.axis[2](0));
        viewFrustum.push_back(-nearP.M.axis[2](1));
        viewFrustum.push_back(-nearP.M.axis[2](2));
        viewFrustum.push_back(nearP.X * nearP.M.axis[2]);

        C4X4Matrix farP(farPlane);
        viewFrustum.push_back(farP.M.axis[2](0));
        viewFrustum.push_back(farP.M.axis[2](1));
        viewFrustum.push_back(farP.M.axis[2](2));
        viewFrustum.push_back(-(farP.X * farP.M.axis[2]));
    }
}

bool CViewableBase::isObjectInsideView(const C7Vector& objectM, const C3Vector& maxBB)
{ // This is to optimize display speed:
    // objects which we can't see are not rendered
    // We have to make distinction between perspective mode and orthogonal mode
    // Until now, only shape-objects are speed-optimized
    // (And only objects with symmetric BB can be processed now!)

    if (_frustumCullingTemporarilyDisabled)
        return (true);

    computeViewFrustumIfNeeded();

    if (!_frustumCulling)
        return (true);

    C3Vector size(maxBB * 1.1); // BB is 1.05 times bigger... but still: we display bounding boxes even if the shape is
                                // culled because of the text that might still be visible!
    return (!_isBoxOutsideVolumeApprox(objectM.getMatrix(), size, &viewFrustum));
}

bool CViewableBase::_isBoxOutsideVolumeApprox(const C4X4Matrix& tr, const C3Vector& s, std::vector<double>* planes)
{ // Planes contain a collection of plane definitions:
    // Each plane is defined by 4 values a, b, c & d (consecutive in the array):
    // ax+by+cz+d=0
    // The normal vector for each plane (a,b,c) should point outside of the volume
    // The volume has to be closed and convex
    // boxTransf is the transformation matrix of the box
    // boxSize is the size of the box (in fact half-sizes)
    // Return value is true if the box is definetely outside of the volume.
    // If return value is false, the box might also be outside of the volume!
    C3Vector edges[8];
    C3Vector v0(tr.M.axis[0] * s(0));
    C3Vector v1(tr.M.axis[1] * s(1));
    C3Vector v2(tr.M.axis[2] * s(2));
    edges[0] = v0 + v1 + v2 + tr.X;
    edges[1] = v0 + v1 - v2 + tr.X;
    edges[2] = v0 - v1 + v2 + tr.X;
    edges[3] = v0 - v1 - v2 + tr.X;
    edges[4] = v1 + v2 + tr.X - v0;
    edges[5] = v1 - v2 + tr.X - v0;
    edges[6] = v2 + tr.X - v0 - v1;
    edges[7] = tr.X - v0 - v1 - v2;
    for (int i = 0; i < int(planes->size()) / 4; i++)
    {
        C3Vector abc(planes->at(4 * i + 0), planes->at(4 * i + 1), planes->at(4 * i + 2));
        double d = planes->at(4 * i + 3);
        if ((abc * edges[0] + d) >= 0.0)
        {
            if ((abc * edges[1] + d) >= 0.0)
            {
                if ((abc * edges[2] + d) >= 0.0)
                {
                    if ((abc * edges[3] + d) >= 0.0)
                    {
                        if ((abc * edges[4] + d) >= 0.0)
                        {
                            if ((abc * edges[5] + d) >= 0.0)
                            {
                                if ((abc * edges[6] + d) >= 0.0)
                                {
                                    if ((abc * edges[7] + d) >= 0.0)
                                        return (true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return (false);
}

void CViewableBase::setPerspective(bool p)
{
    bool diff = (_perspective != p);
    if (diff)
    {
        _perspective = p;
        computeVolumeVectors();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            std::string cmd;
            if (App::getEventProtocolVersion() == 2)
                cmd = "perspectiveMode";
            else
                cmd = propViewable_perspective.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd.c_str(), true);
            ev->appendKeyBool(cmd.c_str(), _perspective);
            App::worldContainer->pushEvent();
        }
    }
}

void CViewableBase::setResolution(const int r[2])
{
    if ((_resolution[0] != r[0]) || (_resolution[1] != r[1]))
    {
        _resolution[0] = r[0];
        _resolution[1] = r[1];
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propViewable_resolution.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt32Array(cmd, _resolution, 2);
            App::worldContainer->pushEvent();
        }
        computeVolumeVectors();
    }
}
void CViewableBase::getResolution(int r[2]) const
{
    r[0] = _resolution[0];
    r[1] = _resolution[1];
}

bool CViewableBase::getPerspective() const
{
    return _perspective;
}

void CViewableBase::computeVolumeVectors()
{
    C3Vector nearV, farV;
    double resYoverResX = double(_resolution[1]) / double(_resolution[0]);
    if (_perspective)
    {
        if (resYoverResX <= 1.0)
        { // x is bigger
            farV(0) = tan(_viewAngle / 2.0) * _farClippingPlane;
            farV(1) = farV(0) * resYoverResX;
            farV(2) = _farClippingPlane;

            nearV(0) = tan(_viewAngle / 2.0) * _nearClippingPlane;
            nearV(1) = nearV(0) * resYoverResX;
            nearV(2) = _nearClippingPlane;
        }
        else
        { // y is bigger
            farV(1) = tan(_viewAngle / 2.0) * _farClippingPlane;
            farV(0) = farV(1) / resYoverResX;
            farV(2) = _farClippingPlane;

            nearV(1) = tan(_viewAngle / 2.0) * _nearClippingPlane;
            nearV(0) = nearV(1) / resYoverResX;
            nearV(2) = _nearClippingPlane;
        }
    }
    else
    {
        if (resYoverResX <= 1.0)
            farV(0) = _orthoViewSize / 2.0; // x is bigger
        else
            farV(0) = _orthoViewSize / (2.0 * resYoverResX); // y is bigger
        farV(1) = farV(0) * resYoverResX;
        farV(2) = _farClippingPlane;

        nearV(0) = farV(0);
        nearV(1) = farV(1);
        nearV(2) = _nearClippingPlane;
    }
    if ((nearV - _volumeVectorNear).getLength() + (farV - _volumeVectorFar).getLength() > 0.0001)
    {
        if ((_volumeVectorNear != nearV) || (_volumeVectorFar != farV))
        {
            _volumeVectorNear = nearV;
            _volumeVectorFar = farV;
            if (_isInScene && App::worldContainer->getEventsEnabled())
            {
                if (App::getEventProtocolVersion() == 2)
                {
                    const char* cmd = "frustumVectors";
                    CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                    ev->openKeyMap(cmd);
                    ev->appendKeyDoubleArray("near", _volumeVectorNear.data, 3);
                    ev->appendKeyDoubleArray("far", _volumeVectorFar.data, 3);
                }
                else
                {
                    const char* cmd = propViewable_frustumCornerNear.name;
                    CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                    if (App::getEventProtocolVersion() <= 3)
                    {
                        ev->appendKeyDoubleArray(cmd, _volumeVectorNear.data, 3);
                        ev->appendKeyDoubleArray(propViewable_frustumCornerFar.name, _volumeVectorFar.data, 3);
                    }
                    else
                    {
                        ev->appendKeyVector3(cmd, _volumeVectorNear);
                        ev->appendKeyVector3(propViewable_frustumCornerFar.name, _volumeVectorFar);
                    }
                }
                App::worldContainer->pushEvent();
            }
        }
    }
}

void CViewableBase::setShowVolume(bool s)
{
    bool diff = (_showVolume != s);
    if (diff)
    {
        _showVolume = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propViewable_showFrustum.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _showVolume);
            App::worldContainer->pushEvent();
        }
    }
}

bool CViewableBase::getShowVolume() const
{
    return _showVolume;
}

void CViewableBase::getVolumeVectors(C3Vector& n, C3Vector& f) const
{
    n = _volumeVectorNear;
    f = _volumeVectorFar;
}

void CViewableBase::addSpecializedObjectEventData(CCbor* ev)
{
    ev->appendKeyDouble(propViewable_viewAngle.name, _viewAngle);
    ev->appendKeyDouble(propViewable_viewSize.name, _orthoViewSize);
    double arr[2] = {_nearClippingPlane, _farClippingPlane};
    ev->appendKeyDoubleArray(propViewable_clippingPlanes.name, arr, 2);
    ev->appendKeyBool(propViewable_perspective.name, _perspective);
    ev->appendKeyBool(propViewable_showFrustum.name, _showVolume);
    if (App::getEventProtocolVersion() <= 3)
    {
        ev->appendKeyDoubleArray(propViewable_frustumCornerNear.name, _volumeVectorNear.data, 3);
        ev->appendKeyDoubleArray(propViewable_frustumCornerFar.name, _volumeVectorFar.data, 3);
    }
    else
    {
        ev->appendKeyVector3(propViewable_frustumCornerNear.name, _volumeVectorNear);
        ev->appendKeyVector3(propViewable_frustumCornerFar.name, _volumeVectorFar);
    }
    ev->appendKeyInt32Array(propViewable_resolution.name, _resolution, 2);
}

int CViewableBase::setBoolProperty(const char* pName, bool pState)
{
    int retVal = CSceneObject::setBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_showFrustum.name, pName) == 0)
        {
            retVal = 1;
            setShowVolume(pState);
        }
    }

    return retVal;
}

int CViewableBase::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = CSceneObject::getBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_perspective.name, pName) == 0)
        {
            retVal = 1;
            pState = _perspective;
        }
        else if (strcmp(propViewable_showFrustum.name, pName) == 0)
        {
            retVal = 1;
            pState = _showVolume;
        }
    }

    return retVal;
}

int CViewableBase::setFloatProperty(const char* pName, double pState)
{
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_viewAngle.name, pName) == 0)
        {
            retVal = 1;
            setViewAngle(pState);
        }
        else if (strcmp(propViewable_viewSize.name, pName) == 0)
        {
            retVal = 1;
            setOrthoViewSize(pState);
        }
    }

    return retVal;
}

int CViewableBase::getFloatProperty(const char* pName, double& pState) const
{
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_viewAngle.name, pName) == 0)
        {
            retVal = 1;
            pState = _viewAngle;
        }
        else if (strcmp(propViewable_viewSize.name, pName) == 0)
        {
            retVal = 1;
            pState = _orthoViewSize;
        }
    }

    return retVal;
}

int CViewableBase::setIntArray2Property(const char* pName, const int* pState)
{
    int retVal = CSceneObject::setIntArray2Property(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_resolution.name, pName) == 0)
        {
            retVal = 1;
            setResolution(pState);
        }
    }

    return retVal;
}

int CViewableBase::getIntArray2Property(const char* pName, int* pState) const
{
    int retVal = CSceneObject::getIntArray2Property(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_resolution.name, pName) == 0)
        {
            retVal = 1;
            pState[0] = _resolution[0];
            pState[1] = _resolution[1];
        }
    }

    return retVal;
}

int CViewableBase::setVector2Property(const char* pName, const double* pState)
{
    int retVal = CSceneObject::setVector2Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CViewableBase::getVector2Property(const char* pName, double* pState) const
{
    int retVal = CSceneObject::getVector2Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CViewableBase::setVector3Property(const char* pName, const C3Vector& pState)
{
    int retVal = CSceneObject::setVector3Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CViewableBase::getVector3Property(const char* pName, C3Vector& pState) const
{
    int retVal = CSceneObject::getVector3Property(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_frustumCornerNear.name, pName) == 0)
        {
            pState = _volumeVectorNear;
            retVal = 1;
        }
        else if (strcmp(propViewable_frustumCornerFar.name, pName) == 0)
        {
            pState = _volumeVectorFar;
            retVal = 1;
        }
    }

    return retVal;
}

int CViewableBase::setFloatArrayProperty(const char* pName, const double* v, int vL)
{
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setFloatArrayProperty(pName, v, vL);
    if (retVal == -1)
    {
        if (strcmp(propViewable_clippingPlanes.name, pName) == 0)
        {
            if (vL >= 2)
            {
                retVal = 1;
                setClippingPlanes(v[0], v[1]);
            }
            else
                retVal = 0;
        }
    }

    return retVal;
}

int CViewableBase::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    pState.clear();
    int retVal = CSceneObject::getFloatArrayProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(propViewable_clippingPlanes.name, pName) == 0)
        {
            pState.push_back(_nearClippingPlane);
            pState.push_back(_farClippingPlane);
            retVal = 1;
        }
    }

    return retVal;
}

int CViewableBase::setIntArrayProperty(const char* pName, const int* v, int vL)
{
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setIntArrayProperty(pName, v, vL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CViewableBase::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    pState.clear();
    int retVal = CSceneObject::getIntArrayProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CViewableBase::getPropertyName_vstatic(int& index, std::string& pName, int excludeFlags)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_viewable.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_viewable[i].name, pName.c_str()))
        {
            if ((allProps_viewable[i].flags & excludeFlags) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_viewable[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CViewableBase::getPropertyInfo_vstatic(const char* pName, int& info, std::string& infoTxt)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_viewable.size(); i++)
    {
        if (strcmp(allProps_viewable[i].name, pName) == 0)
        {
            retVal = allProps_viewable[i].type;
            info = allProps_viewable[i].flags;
            if (infoTxt == "j")
                infoTxt = allProps_viewable[i].shortInfoTxt;
            else
            {
                auto w = QJsonDocument::fromJson(allProps_viewable[i].shortInfoTxt.c_str()).object();
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
    return retVal;
}
