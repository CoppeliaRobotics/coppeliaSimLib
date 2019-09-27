
#include "vrepMainHeader.h"
#include "viewableBase.h"

#include "tt.h"
#include "v_rep_internal.h"
#include "camera.h"
#include "vDateTime.h"
#include "4X4Matrix.h"
#include "app.h"

bool CViewableBase::fogWasActivated=false;
bool CViewableBase::_frustumCulling=true;

bool CViewableBase::getFrustumCullingEnabled()
{
    return(_frustumCulling);
}

void CViewableBase::setFrustumCullingEnabled(bool e)
{
    _frustumCulling=e;
}

CViewableBase::CViewableBase()
{
    _planesCalculated=false;
    _fogTimer=VDateTime::getTimeInMs()-1;
    _fogTimerDuration=0.0f;
    _frustumCullingTemporarilyDisabled=false;
    _disabledColorComponents=0;
}

CViewableBase::~CViewableBase()
{
}

int CViewableBase::getDisabledColorComponents()
{
    return(_disabledColorComponents);
}

void CViewableBase::setDisabledColorComponents(int c)
{
    _disabledColorComponents=c;
}

C3DObject* CViewableBase::copyYourself() 
{
    return(nullptr);
}
void CViewableBase::display(CViewableBase* renderingObject,int displayAttrib)
{
}
void CViewableBase::scaleObject(float scalingFactor)
{
}
void CViewableBase::simulationAboutToStart()
{
}
void CViewableBase::simulationEnded()
{
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

void CViewableBase::setUseLocalLights(bool l)
{
    _useLocalLights=l;
}

bool CViewableBase::getuseLocalLights()
{
    return(_useLocalLights);
}

void CViewableBase::serialize(CSer& ar)
{
}
bool CViewableBase::announceObjectWillBeErased(int objID,bool copyBuffer)
{
    // Return value true means 'this' has to be erased too!
    return(false);
}
void CViewableBase::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{
}
void CViewableBase::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{
}
void CViewableBase::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{
}
void CViewableBase::performObjectLoadingMapping(std::vector<int>* map)
{
}
void CViewableBase::performCollectionLoadingMapping(std::vector<int>* map)
{
}
void CViewableBase::performCollisionLoadingMapping(std::vector<int>* map)
{
}
void CViewableBase::performDistanceLoadingMapping(std::vector<int>* map)
{
}
std::string CViewableBase::getObjectTypeInfo() const
{
    return("");
}
std::string CViewableBase::getObjectTypeInfoExtended() const
{
    return("");
}
bool CViewableBase::isPotentiallyCollidable() const
{
    return(false);
}
bool CViewableBase::isPotentiallyMeasurable() const
{
    return(false);
}
bool CViewableBase::isPotentiallyDetectable() const
{
    return(false);
}
bool CViewableBase::isPotentiallyRenderable() const
{
    return(false);
}
bool CViewableBase::isPotentiallyCuttable() const
{
    return(false);
}

void CViewableBase::setNearClippingPlane(float nearPlane)
{
    tt::limitValue(0.0001f,_farClippingPlane,nearPlane);
    _nearClippingPlane=nearPlane;
}

float CViewableBase::getNearClippingPlane()
{
    return(_nearClippingPlane);
}

void CViewableBase::setFarClippingPlane(float farPlane)
{
    tt::limitValue(_nearClippingPlane,100000.0f,farPlane);
    _farClippingPlane=farPlane;
}

float CViewableBase::getFarClippingPlane()
{
    return(_farClippingPlane);
}

void CViewableBase::setViewAngle(float angle)
{
    _viewAngle=angle;
    tt::limitValue(1.0f*degToRad_f,135.0f*degToRad_f,_viewAngle); // with 90 degrees, objects disappear!! Really??? Changed to 135 on 2010/11/12
}

float CViewableBase::getViewAngle()
{
    return(_viewAngle);
}

void CViewableBase::setOrthoViewSize(float theSize)
{
    _orthoViewSize=theSize;
    tt::limitValue(0.001f,200000.0f,_orthoViewSize);
}

float CViewableBase::getOrthoViewSize()
{
    return(_orthoViewSize);
}

void CViewableBase::setShowFogIfAvailable(bool showFog)
{
    _showFogIfAvailable=showFog;
}

bool CViewableBase::getShowFogIfAvailable()
{
    return(_showFogIfAvailable);
}

void CViewableBase::setFogTimer(float seconds)
{
    _fogTimerDuration=seconds;
    _fogTimer=VDateTime::getTimeInMs()+int(seconds*1000.0f);
}

float CViewableBase::getFogStrength()
{
    if (_fogTimerDuration==0.0f)
        return(1.0f);
    int t=VDateTime::getTimeInMs();
    if (_fogTimer<t)
    {
        _fogTimerDuration=0.0f;
        return(1.0f);
    }
    if (_fogTimer>t+100000)
    {
        _fogTimerDuration=0.0f;
        return(1.0f);
    }
    float dt=float(_fogTimer-t)/1000.0f;
    return(1.0f-(dt/_fogTimerDuration));
}

void CViewableBase::setFrustumCullingTemporarilyDisabled(bool d)
{
    _frustumCullingTemporarilyDisabled=d;
}

void CViewableBase::computeViewFrustumIfNeeded()
{
    if (!_planesCalculated)
    {
        _planesCalculated=true;
        C7Vector m;
        C7Vector upperPlane;
        C7Vector lowerPlane;
        C7Vector rightPlane;
        C7Vector leftPlane;
        C7Vector nearPlane;
        C7Vector farPlane;
        float winXSize=(float)_currentViewSize[0];
        float winYSize=(float)_currentViewSize[1];
        C7Vector viewableCumulTransf;

        if (getObjectType()==sim_object_camera_type)
        {
            viewableCumulTransf=getCumulativeTransformation_forDisplay(true);
#ifdef SIM_WITH_GUI
            if (App::mainWindow->getHasStereo())
            { // handle stereo cameras correctly
                C7Vector displ;
                displ.setIdentity();
                if (App::mainWindow->getLeftEye())
                    displ.X(0)=App::mainWindow->getStereoDistance()/2;
                else
                    displ.X(0)=-App::mainWindow->getStereoDistance()/2;
                viewableCumulTransf=viewableCumulTransf*displ;
            }
#endif
        }
        else
            viewableCumulTransf=getCumulativeTransformation();

        if (_currentPerspective)
        {
            float xAngle,yAngle;
            if (winXSize<winYSize)
            {
                yAngle=_viewAngle*0.5f;
                xAngle=(float)atan(tan(_viewAngle*0.5f)*winXSize/winYSize);
            }
            else
            {
                yAngle=(float)atan(tan(_viewAngle*0.5f)*winYSize/winXSize);
                xAngle=_viewAngle*0.5f;
            }
            xAngle*=1.05f; // a little bit of tolerance!
            yAngle*=1.05f;
            upperPlane.X=viewableCumulTransf.X;
            lowerPlane.X=viewableCumulTransf.X;
            rightPlane.X=viewableCumulTransf.X;
            leftPlane.X=viewableCumulTransf.X;
            upperPlane.Q=viewableCumulTransf.Q*C4Vector(-yAngle,C3Vector(1.0f,0.0f,0.0f));
            lowerPlane.Q=viewableCumulTransf.Q*C4Vector(+yAngle,C3Vector(1.0f,0.0f,0.0f));
            rightPlane.Q=viewableCumulTransf.Q*C4Vector(-xAngle,C3Vector(0.0f,1.0f,0.0f));
            leftPlane.Q=viewableCumulTransf.Q*C4Vector(+xAngle,C3Vector(0.0f,1.0f,0.0f));
            nearPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,_nearClippingPlane)));
            farPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,_farClippingPlane)));
        }
        else
        {
            float xSize,ySize;
            if (winXSize<winYSize)
            {
                xSize=_orthoViewSize*0.5f*winXSize/winYSize;
                ySize=_orthoViewSize*0.5f;
            }
            else
            {
                xSize=_orthoViewSize*0.5f;
                ySize=_orthoViewSize*0.5f*winYSize/winXSize;
            }
            xSize*=1.05f; // a little bit of tolerance!
            ySize*=1.05f;
            upperPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,+ySize,0.0f)));
            lowerPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,-ySize,0.0f)));
            rightPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(-xSize,0.0f,0.0f)));
            leftPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(+xSize,0.0f,0.0f)));
            if (getObjectType()==sim_object_camera_type)
            { // we are in othoview!
                nearPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,ORTHO_CAMERA_NEAR_CLIPPING_PLANE)));
                farPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,ORTHO_CAMERA_FAR_CLIPPING_PLANE)));
            }
            else
            {
                nearPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,_nearClippingPlane)));
                farPlane.setMultResult(viewableCumulTransf,C7Vector(C3Vector(0.0f,0.0f,_farClippingPlane)));
            }
        }
        viewFrustum.clear();
        
        // If the plane ordering changes, check also the volumeNode class!!
        C4X4Matrix upperP(upperPlane);
        viewFrustum.push_back(upperP.M.axis[1](0));
        viewFrustum.push_back(upperP.M.axis[1](1));
        viewFrustum.push_back(upperP.M.axis[1](2));
        viewFrustum.push_back(-(upperP.X*upperP.M.axis[1]));

        C4X4Matrix lowerP(lowerPlane);
        viewFrustum.push_back(-lowerP.M.axis[1](0));
        viewFrustum.push_back(-lowerP.M.axis[1](1));
        viewFrustum.push_back(-lowerP.M.axis[1](2));
        viewFrustum.push_back(lowerP.X*lowerP.M.axis[1]);
    
        C4X4Matrix rightP(rightPlane);
        viewFrustum.push_back(-rightP.M.axis[0](0));
        viewFrustum.push_back(-rightP.M.axis[0](1));
        viewFrustum.push_back(-rightP.M.axis[0](2));
        viewFrustum.push_back(rightP.X*rightP.M.axis[0]);
    
        C4X4Matrix leftP(leftPlane);
        viewFrustum.push_back(leftP.M.axis[0](0));
        viewFrustum.push_back(leftP.M.axis[0](1));
        viewFrustum.push_back(leftP.M.axis[0](2));
        viewFrustum.push_back(-(leftP.X*leftP.M.axis[0]));
    
        C4X4Matrix nearP(nearPlane);
        viewFrustum.push_back(-nearP.M.axis[2](0));
        viewFrustum.push_back(-nearP.M.axis[2](1));
        viewFrustum.push_back(-nearP.M.axis[2](2));
        viewFrustum.push_back(nearP.X*nearP.M.axis[2]);
    
        C4X4Matrix farP(farPlane);
        viewFrustum.push_back(farP.M.axis[2](0));
        viewFrustum.push_back(farP.M.axis[2](1));
        viewFrustum.push_back(farP.M.axis[2](2));
        viewFrustum.push_back(-(farP.X*farP.M.axis[2]));
    }
}

bool CViewableBase::isObjectInsideView(const C7Vector& objectM,const C3Vector& maxBB)
{   // This is to optimize display speed:
    // objects which we can't see are not rendered
    // We have to make distinction between perspective mode and orthogonal mode
    // Until now, only shape-objects are speed-optimized
    // (And only objects with symmetric BB can be processed now!)

    if (_frustumCullingTemporarilyDisabled)
        return(true);

    computeViewFrustumIfNeeded();

    if (!_frustumCulling)
        return(true);

    C3Vector size(maxBB*1.1f);  // BB is 1.05 times bigger... but still: we display bounding boxes even if the shape is culled because of the text that might still be visible!
    return(!_isBoxOutsideVolumeApprox(objectM.getMatrix(),size,&viewFrustum));
}

bool CViewableBase::_isBoxOutsideVolumeApprox(const C4X4Matrix& tr,
                            const C3Vector& s,std::vector<float>* planes)
{   // Planes contain a collection of plane definitions:
    // Each plane is defined by 4 values a, b, c & d (consecutive in the array):
    // ax+by+cz+d=0
    // The normal vector for each plane (a,b,c) should point outside of the volume
    // The volume has to be closed and convex
    // boxTransf is the transformation matrix of the box
    // boxSize is the size of the box (in fact half-sizes)
    // Return value is true if the box is definetely outside of the volume.
    // If return value is false, the box might also be outside of the volume!
    C3Vector edges[8];
    C3Vector v0(tr.M.axis[0]*s(0));
    C3Vector v1(tr.M.axis[1]*s(1));
    C3Vector v2(tr.M.axis[2]*s(2));
    edges[0]=v0+v1+v2+tr.X;
    edges[1]=v0+v1-v2+tr.X;
    edges[2]=v0-v1+v2+tr.X;
    edges[3]=v0-v1-v2+tr.X;
    edges[4]=v1+v2+tr.X-v0;
    edges[5]=v1-v2+tr.X-v0;
    edges[6]=v2+tr.X-v0-v1;
    edges[7]=tr.X-v0-v1-v2;
    for (int i=0;i<int(planes->size())/4;i++)
    {
        C3Vector abc(planes->at(4*i+0),planes->at(4*i+1),planes->at(4*i+2));
        float d=planes->at(4*i+3);
        if ((abc*edges[0]+d)>=0.0f)
        {
            if ((abc*edges[1]+d)>=0.0f)
            {
                if ((abc*edges[2]+d)>=0.0f)
                {
                    if ((abc*edges[3]+d)>=0.0f)
                    {
                        if ((abc*edges[4]+d)>=0.0f)
                        {
                            if ((abc*edges[5]+d)>=0.0f)
                            {
                                if ((abc*edges[6]+d)>=0.0f)
                                {
                                    if ((abc*edges[7]+d)>=0.0f)
                                        return(true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(false);
}
