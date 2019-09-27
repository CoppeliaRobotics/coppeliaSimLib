
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "pathCont.h"
#include "tt.h"
#include "v_rep_internal.h"
#include "meshRoutines.h"
#include "linMotionRoutines.h"
#include "v_repStrings.h"
#include "app.h"
#ifdef SIM_WITH_OPENGL
#include "oGL.h"
#endif
#ifdef SIM_WITH_GUI
#include "vMessageBox.h"
#endif

CPathCont::CPathCont()
{
    _position=0.0f;
    _maxAcceleration=0.01f;
    _nominalVelocity=0.0f;
    _targetNominalVelocity=0.0f;

    _initialValuesInitialized=false;
    _pathModifID=42; // anything except 0!

    _pathLengthCalculationMethod=sim_distcalcmethod_dl_if_nonzero;

    _attributes=sim_pathproperty_show_line|sim_pathproperty_show_orientation|sim_pathproperty_automatic_orientation|
                sim_pathproperty_show_position|sim_pathproperty_endpoints_at_zero_deprecated|sim_pathproperty_infinite_acceleration_deprecated;

    _lineSize=1;
    _squareSize=0.01f;
    _lineColor.setDefaultValues();
    _lineColor.setColor(0.1f,0.75f,1.0f,sim_colorcomponent_ambient_diffuse);

    _angleVarToDistanceCoeff=0.1f/(90.0f*degToRad_f);
    _onSpotDistanceToDistanceCoeff=1.0f;


    _avp_turningCircleRadiusForHalfVelocity=0.2f;
    _avp_relativeVelocityAtRotationAxisChange=0.5f;
    _avp_relativeAcceleration=0.1f;

    _actualizationEnabled=true;
}

CPathCont::~CPathCont()
{
    _removeAllBezierPathPoints();
    removeAllSimplePathPoints();
}

void CPathCont::setPathLengthCalculationMethod(int m)
{
    _pathLengthCalculationMethod=m;
    actualizePath();
}

int CPathCont::getPathLengthCalculationMethod()
{
    return(_pathLengthCalculationMethod);
}

void CPathCont::setAvpTurningCircleRadiusForHalfVelocity(float radius)
{
    radius=tt::getLimitedFloat(0.0001f,1000.0f,radius);
    _avp_turningCircleRadiusForHalfVelocity=radius;
    actualizePath();
}
float CPathCont::getAvpTurningCircleRadiusForHalfVelocity()
{
    return(_avp_turningCircleRadiusForHalfVelocity);
}

void CPathCont::setAvpRelativeVelocityAtRotationAxisChange(float relVel)
{
    relVel=tt::getLimitedFloat(0.01f,1.0f,relVel);
    _avp_relativeVelocityAtRotationAxisChange=relVel;
    actualizePath();
}
float CPathCont::getAvpRelativeVelocityAtRotationAxisChange()
{
    return(_avp_relativeVelocityAtRotationAxisChange);
}
void CPathCont::setAvpRelativeAcceleration(float relAccel)
{
    relAccel=tt::getLimitedFloat(0.00001f,10000.0f,relAccel);
    _avp_relativeAcceleration=relAccel;
    actualizePath();
}
float CPathCont::getAvpRelativeAcceleration()
{
    return(_avp_relativeAcceleration);
}

unsigned short CPathCont::getPathModifID()
{
    return(_pathModifID);
}

void CPathCont::setAngleVarToDistanceCoeff(float f)
{
    tt::limitValue(0.0001f,1000.0f,f);
    _angleVarToDistanceCoeff=f;
    actualizePath();
}

float CPathCont::getAngleVarToDistanceCoeff()
{
    return(_angleVarToDistanceCoeff);
}

void CPathCont::setOnSpotDistanceToDistanceCoeff(float f)
{
    tt::limitValue(0.0001f,1000.0f,f);
    _onSpotDistanceToDistanceCoeff=f;
    actualizePath();
}

float CPathCont::getOnSpotDistanceToDistanceCoeff()
{
    return(_onSpotDistanceToDistanceCoeff);
}

void CPathCont::scaleObject(float scalingFactor)
{
    for (int i=0;i<int(_simplePathPoints.size());i++)
        _simplePathPoints[i]->scaleYourself(scalingFactor);

    _squareSize*=scalingFactor;

    _position*=double(scalingFactor);
    _maxAcceleration*=scalingFactor;
    _nominalVelocity*=scalingFactor;
    _targetNominalVelocity*=scalingFactor;
    _angleVarToDistanceCoeff*=scalingFactor;
    _onSpotDistanceToDistanceCoeff*=scalingFactor;

    _avp_turningCircleRadiusForHalfVelocity*=scalingFactor;

    if (_initialValuesInitialized)
    {
        _initialPosition*=scalingFactor;
        _initialNominalVelocity*=scalingFactor;
        _initialTargetNominalVelocity*=scalingFactor;
    }

    actualizePath();
}

void CPathCont::scaleObjectNonIsometrically(float x,float y,float z)
{
    float xyz=cbrt(x*y*z);
    for (int i=0;i<int(_simplePathPoints.size());i++)
        _simplePathPoints[i]->scaleYourselfNonIsometrically(x,y,z);

    _squareSize*=xyz;

    _position*=double(xyz);
    _maxAcceleration*=xyz;
    _nominalVelocity*=xyz;
    _targetNominalVelocity*=xyz;
    _angleVarToDistanceCoeff*=xyz;
    _onSpotDistanceToDistanceCoeff*=xyz;

    _avp_turningCircleRadiusForHalfVelocity*=xyz;

    if (_initialValuesInitialized)
    {
        _initialPosition*=xyz;
        _initialNominalVelocity*=xyz;
        _initialTargetNominalVelocity*=xyz;
    }

    actualizePath();
}

void CPathCont::resetPath(CPath* it)
{
    if (_initialValuesInitialized)
    {
        setPosition(_initialPosition);
        setNominalVelocity(_initialNominalVelocity);
        setTargetNominalVelocity(_initialTargetNominalVelocity);
    }
    _handleAttachedDummies(it);
}

bool CPathCont::_getBezierPointsForVirtualDistance(double& l,int& index0,int& index1,bool forwardDirection)
{
    int c=getBezierPathPointCount();
    double pl=getBezierVirtualPathLength();
    if (pl==0.0f)
        return(false);
    if (_attributes&sim_pathproperty_closed_path)
    {
        l=CMath::robustmod(l,pl);
        while (l<0.0f)
            l+=pl;
    }
    else
        l=tt::getLimitedDouble(0.0,pl,l);
    for (int i=0;i<c;i++)
    {
        int j=i+1;
        if (j>=c)
        {
            j=0; // in case we have a closed path
        }

        if ((l==0.0f)&&(!forwardDirection)&&(_attributes&sim_pathproperty_closed_path)&&(i!=0))
            l=_bezierPathPoints[0]->virtualCumulativeLength;
        double l0=0.0f;
        if (i!=0)
            l0=_bezierPathPoints[i]->virtualCumulativeLength;
        double l1=_bezierPathPoints[j]->virtualCumulativeLength;
        if ((_attributes&sim_pathproperty_closed_path)==0)
        { // Exclusively open path cases:
            if ( ((i==0)&&(!forwardDirection)&&(l==l0))||
                ((i==c-2)&&forwardDirection&&(l==l1)) )
            { // we found the spot!
                index0=i;
                index1=j;
                return(true);
            }
        }

        if ((l>=l0)&&(l<=l1)&&(l0!=l1))
        {
            if ( ((l==l0)&&forwardDirection)||
                ((l>l0)&&(l<l1))||
                ((l==l1)&&(!forwardDirection)) )
            { // we found the spot!
                index0=i;
                index1=j;
                return(true);
            }
        }
    }
#ifdef SIM_WITH_GUI
    App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSNOTR_APPLICATION_ERROR),strTranslate(IDSNOTR_STRANGE_ERROR6),VMESSAGEBOX_OKELI);
#endif
    return(false);
}

void CPathCont::handlePath(CPath* it,float deltaTime)
{ // DEPRECATED
    if (App::ct->mainSettings->pathMotionHandlingEnabled_DEPRECATED)
    { // we should not forget to call _handleAttachedDummies even when the path motion is disabled!!
        handlePath_keepObjectUnchanged(deltaTime,_position,_nominalVelocity,_targetNominalVelocity,_maxAcceleration,(_attributes&sim_pathproperty_invert_velocity_deprecated)!=0,(_attributes&sim_pathproperty_infinite_acceleration_deprecated)!=0);
        setPosition(_position);
    }
    _handleAttachedDummies(it);
}


void CPathCont::handlePath_keepObjectUnchanged(float deltaTime,double& pos_,float& nomVel_,float& targetNomVel_,float maxAccel,bool invertVelocity,bool infiniteAccel)
{ // This function should not change any values in this object (so that the path can be handled by simFollowPath functions or such!)
    if (deltaTime<=0.0f)
        return;
    double newPos=pos_;
    double nominalVel=nomVel_;

    if ( ((fabs(newPos)<0.000000001f)&&(nominalVel<0.0f))||((fabs(newPos-getBezierVirtualPathLength())<0.000000001f)&&(nominalVel>0.0f)) )
    { // We are on an end-point.
        if (!invertVelocity)
        {
            if (nominalVel*targetNomVel_<0.0f)
                nominalVel=0.0f; 
        }
    }

    if (infiniteAccel)
        nominalVel=targetNomVel_;
    double timeSlice=double(deltaTime);
    int infLoopSafety=0;
    while (timeSlice!=0.0f)
    {
        int pIndex0,pIndex1;
        double v=nominalVel;
        if (v==0.0)
            v=targetNomVel_;
        if (_getBezierPointsForVirtualDistance(newPos,pIndex0,pIndex1,v>=0.0f))
        {
            CBezierPathPoint* lowP=_bezierPathPoints[pIndex0];
            CBezierPathPoint* highP=_bezierPathPoints[pIndex1];
            float lowPos=lowP->virtualCumulativeLength;
            float highPos=highP->virtualCumulativeLength;
            if (pIndex0==0)
                lowPos=0.0f;
            CLinMotionRoutines::getNextValuesForPath(newPos,nominalVel,targetNomVel_,maxAccel,lowPos,highPos,lowP->getMaxRelAbsVelocity(),highP->getMaxRelAbsVelocity(),timeSlice);
            if (_attributes&sim_pathproperty_closed_path)
            { // We normalize the position
                while (newPos<0.0f)
                    newPos+=getBezierVirtualPathLength();
                newPos=CMath::robustmod(newPos,getBezierVirtualPathLength());
            }
            else
            {
                v=targetNomVel_;
                if (v==0.0f)
                    v=nominalVel;
                newPos=tt::getLimitedDouble(0.0,getBezierVirtualPathLength(),newPos);
                if ( ((newPos==0.0f)&&(v<0.0f))||((newPos==getBezierVirtualPathLength())&&(v>0.0f)) )
                { // We arrived on an end-point. We might have to invert the velocity
                    if (invertVelocity)
                    {
                        targetNomVel_*=-1.0f; // We invert the target velocity and continue
                        nominalVel*=-1.0f;
                    }
                    else
                        break; // We leave here
                }
            }
            infLoopSafety++;
            if (infLoopSafety>100)
                break; // 2009-02-02 (to avoid infinite loops when subDeltaTime doesn't change at limits where target vel. is inverting but an outside app keeps target vel. same)
        }
        else
            timeSlice=0.0f; // Here we have a situation where the path has zero length
    }

    nomVel_=float(nominalVel);
    pos_=newPos;
    // Adjust the pos:
    if (_attributes&sim_pathproperty_closed_path)
    {
        pos_=CMath::robustmod(pos_,getBezierVirtualPathLength());
        while (pos_<0.0)
            pos_+=getBezierVirtualPathLength();
    }
    else
        pos_=tt::getLimitedDouble(0.0,getBezierVirtualPathLength(),pos_);

}

void CPathCont::_handleAttachedDummies(CPath* it)
{
    if (it==nullptr)
        return; // can happen during object creation
    std::vector<C3DObject*> childrensChildren;
    for (int i=0;i<int(it->childList.size());i++)
    { // Here we handle all dummies that are direct children of this path and that are fixed or free ON the path
        if (it->childList[i]->getObjectType()==sim_object_dummy_type)
        {
            CDummy* dum=(CDummy*)it->childList[i];
            if (dum->getAssignedToParentPath())
            {
                if (dum->getFreeOnPathTrajectory())
                { // The dummy is free. We search for the closest position on the path and then assign the dummy to that position:
                    C3Vector dp(dum->getLocalTransformationPart1().X);
                    C7Vector conf;
                    if (getConfigurationOnBezierCurveClosestTo(dp,conf))
                        dum->setLocalTransformation(conf);
                }
                else
                { // The dummy is fixed
                    float dl=float(_position)+dum->getVirtualDistanceOffsetOnPath();
                    int ind;
                    float t;
                    if (_getPointOnBezierCurveAtVirtualDistance(dl,ind,t))
                        dum->setLocalTransformation(_getInterpolatedBezierCurvePoint(ind,t));
                }
            }
        }
        it->childList[i]->getAllObjectsRecursive(&childrensChildren,false);
    }
    // Here we handle all dummies that are children (but not direct children) of this path and that follow the orientation on the path:
    C7Vector pctr(it->getCumulativeTransformation());
    C7Vector pctrInv(pctr.getInverse());
    for (int i=0;i<int(childrensChildren.size());i++)
    {
        if (childrensChildren[i]->getObjectType()==sim_object_dummy_type)
        {
            CDummy* d=(CDummy*)childrensChildren[i];
            if (d->getAssignedToParentPathOrientation())
            {
                C7Vector dctr(pctrInv*d->getCumulativeTransformation());
                C7Vector conf;
                if (getConfigurationOnBezierCurveClosestTo(dctr.X,conf))
                {
                    dctr.Q=pctr.Q*conf.Q;
                    d->setLocalTransformation(d->getParentCumulativeTransformation().Q.getInverse()*dctr.Q);
                }
            }
        }
    }
}

void CPathCont::_getDistinctConsecutivePoints(const std::vector<CPathPoint*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices)
{
    tr.clear();
    ptIndices.clear();
    // 1. we first handle special cases:
    if (ptCont.size()==0)
        return; // no points here
    if (ptCont.size()==1)
    { // a single point here
        tr.push_back(ptCont[0]->getTransformation().getMatrix());
        ptIndices.push_back(std::vector<int>());
        ptIndices[0].push_back(0);
        return;
    }
    // 2. We group consecutive coincident points:
    for (int i=0;i<int(ptCont.size());i++)
    {
        C4X4Matrix m(ptCont[i]->getTransformation().getMatrix());
        if (i==0)
        {
            tr.push_back(m);
            ptIndices.push_back(std::vector<int>());
        }
        C4X4Matrix previousM(tr[tr.size()-1]);
        if ((m.X-previousM.X).getLength()==0.0f)
            ptIndices[tr.size()-1].push_back(i); // the two points coincide
        else
        { // The two points don't coincide.
            tr.push_back(m);
            ptIndices.push_back(std::vector<int>());
            ptIndices[tr.size()-1].push_back(i);
        }
    }
    // 3. We need to check whether the last and first points coincide in case the path is closed:
    if ( ((_attributes&sim_pathproperty_closed_path)!=0)&&(tr.size()>1) )
    {
        if ((tr[0].X-tr[tr.size()-1].X).getLength()==0.0f)
        { // they coincide! We have to merge them:
            for (int i=0;i<int(ptIndices[tr.size()-1].size());i++)
                ptIndices[0].push_back(ptIndices[tr.size()-1][i]);
            tr.pop_back();
            ptIndices.pop_back();
        }
    }   
}

void CPathCont::_applyDistinctConsecutivePoints(const std::vector<CPathPoint*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices)
{
    for (int i=0;i<int(tr.size());i++)
    {
        for (int j=0;j<int(ptIndices[i].size());j++)
        {
            int index=ptIndices[i][j];
            ptCont[index]->setTransformation(tr[i].getTransformation(),_attributes);
        }
    }
}

void CPathCont::_computeAutomaticOrientation(const std::vector<CPathPoint*>& pc)
{ // z along the path in the direction of the path, y pointing outwards
    // However if the path is flat, x will point like path object z axis, and y will point inwards or outwards!
    std::vector<C4X4Matrix> tr;
    std::vector<std::vector<int> > ptIndices;
    // 1. we get all distinct consecutive points:
    _getDistinctConsecutivePoints(pc,tr,ptIndices);
    // 2. we handle special cases:
    if (tr.size()==0)
        return;
    if (tr.size()==1)
    {
        tr[0].M.setIdentity();
        _applyDistinctConsecutivePoints(pc,tr,ptIndices);
        return;
    }
    if (tr.size()==2)
    {
        C3Vector v(tr[1].X-tr[0].X);
        C3X3Matrix m;
        m.setIdentity();
        m.axis[2]=v.getNormalized();
        if (_attributes&sim_pathproperty_keep_x_up)
        {
            m.axis[0]=C3Vector::unitZVector;
            m.axis[1]=(m.axis[2]^m.axis[0]).getNormalized();
        }
        else
        {
            if (m.axis[2](2)<0.9f)
            {
                m.axis[1](0)=0.0f;
                m.axis[1](2)=1.0f;
            }
            else
            {
                m.axis[1](0)=1.0f;
                m.axis[1](2)=0.0f;
            }
            m.axis[1](1)=0.0f;
            m.axis[0]=(m.axis[1]^m.axis[2]).getNormalized();
            m.axis[1]=(m.axis[2]^m.axis[0]).getNormalized();
        }
        tr[0].M=m;
        tr[1].M=m;
        _applyDistinctConsecutivePoints(pc,tr,ptIndices);
        return;
    }
    // 3. Here we have 3 or more points. 
    // We start with the general case ("keep-x-up-paths" are treated afterwards): We tag each point as 1 or 2 and set its z-vector or full orientation:
    std::vector<unsigned char> fixedOrientationLevels(tr.size(),0); // 0=not fixed at all, 1=z-vector only fixed, 2=completely fixed 
    for (int i=0;i<int(tr.size());i++)
    {
        C3Vector vb,vm,va;
        vm=tr[i+0].X;
        bool checkAngle=true;
        C3Vector zVect;
        if (i==0)
        {
            if ((_attributes&sim_pathproperty_closed_path)!=0)
            { // Path is closed, first point
                vb=tr[tr.size()-1].X;
                va=tr[i+1].X;
            }
            else
            {
                va=tr[i+1].X;
                zVect=va-vm;
                checkAngle=false; // path is open, first point
            }
        }
        else if (i==int(tr.size())-1)
        {
            if ((_attributes&sim_pathproperty_closed_path)!=0)
            { // Path is closed, last point
                vb=tr[i-1].X;
                va=tr[0].X;
            }
            else
            {
                vb=tr[i-1].X;
                zVect=vm-vb;
                checkAngle=false; // path is open, last point
            }
        }
        else
        {
            vb=tr[i-1].X;
            va=tr[i+1].X;
        }
        if (!checkAngle)
        {
            fixedOrientationLevels[i]=1;
            tr[i].M.axis[2]=zVect.getNormalized();
        }
        else
        {
            C3Vector wwb((vm-vb).getNormalized());
            C3Vector wwa((va-vm).getNormalized());
            if (!wwb.isColinear(wwa,1.0f))
            {
                tr[i].M.axis[0]=(wwb^wwa).getNormalized();
                tr[i].M.axis[1]=(wwb-wwa).getNormalized();
                tr[i].M.axis[2]=(tr[i].M.axis[0]^tr[i].M.axis[1]).getNormalized();
                fixedOrientationLevels[i]=2;
            }
            else
            {
                fixedOrientationLevels[i]=1;
                tr[i].M.axis[2]=wwb.getNormalized();
            }
        }
    }
    // 4. We now have tagged points. We need to interpolate between points tagged as "2"
    int indexOfFirstFound=-1;
    for (int i=0;i<int(tr.size());i++)
    {
        if (fixedOrientationLevels[i]==2)
        {
            float cumulL=0.0f; // cumulative distance (colinear paths can go back and forth!!!)
            C4X4Matrix m0=tr[i];
            if (indexOfFirstFound==-1)
                indexOfFirstFound=i;
            C4X4Matrix mtemp=tr[i];
            int upBound=int(tr.size());
            if ((_attributes&sim_pathproperty_closed_path)!=0)
                upBound=int(tr.size())+indexOfFirstFound+1; // path is closed!
            for (int j=i+1;j<upBound;j++)
            {
                int adjustedJIndex=j;
                if (j>=int(tr.size()))
                    adjustedJIndex-=(int)tr.size();
                C4X4Matrix m1=tr[adjustedJIndex];
                cumulL+=(mtemp.X-m1.X).getLength();
                mtemp=m1;
                if (fixedOrientationLevels[adjustedJIndex]==2)
                {
                    if (j==i+1)
                        break; // the two points are neighbours!
                    m1=tr[adjustedJIndex];
                    // Now we go from m0 (i) to m1 (adjustedJIndex) and interpolating orientation according to the cumulative distance
                    mtemp=tr[i];
                    float totL=0.0f;
                    for (int k=i+1;k<j;k++)
                    {
                        int adjustedKIndex=k;
                        if (k>=int(tr.size()))
                            adjustedKIndex-=(int)tr.size();
                        C4X4Matrix mtemp2=tr[adjustedKIndex];
                        float l=(mtemp2.X-mtemp.X).getLength();
                        mtemp=mtemp2;
                        totL+=l;
                        float t=totL/cumulL;
                        C3Vector interpolatedY;
                        if (!m0.M.axis[1].isColinear(m1.M.axis[1],0.999f))
                        {
                            interpolatedY.buildInterpolation(m0.M.axis[1],m1.M.axis[1],t);
                            tr[adjustedKIndex].M.axis[0]=(interpolatedY^tr[adjustedKIndex].M.axis[2]).getNormalized();
                            tr[adjustedKIndex].M.axis[1]=(tr[adjustedKIndex].M.axis[2]^tr[adjustedKIndex].M.axis[0]).getNormalized();
                        }
                        else
                        { // the two vectors are colinear. Are they pointing into the same direction?
                            if (m0.M.axis[1]*m1.M.axis[1]>0.0f)
                            {
                                interpolatedY.buildInterpolation(m0.M.axis[1],m1.M.axis[1],t); // yes!!
                                tr[adjustedKIndex].M.axis[0]=(interpolatedY^tr[adjustedKIndex].M.axis[2]).getNormalized();
                                tr[adjustedKIndex].M.axis[1]=(tr[adjustedKIndex].M.axis[2]^tr[adjustedKIndex].M.axis[0]).getNormalized();
                            }
                            else
                            { // No!! They point into opposite direction. We need to turn! (clockwise, fixed!)
                                // We need an intermediate point in the middle:
                                C3Vector auxPt(m0.M.axis[0]*-1.0f);
                                if (t<=0.5f)
                                    interpolatedY.buildInterpolation(m0.M.axis[1],auxPt,t/0.5f);
                                else
                                    interpolatedY.buildInterpolation(auxPt,m1.M.axis[1],(t-0.5f)/0.5f);
                                tr[adjustedKIndex].M.axis[0]=(interpolatedY^tr[adjustedKIndex].M.axis[2]).getNormalized();
                                tr[adjustedKIndex].M.axis[1]=(tr[adjustedKIndex].M.axis[2]^tr[adjustedKIndex].M.axis[0]).getNormalized();
                            }
                        }
                        fixedOrientationLevels[adjustedKIndex]=2;
                    }
                    break; // we filled the missing info between the two points
                }
            }
        }
    }
    // 4.5 We have to handle the extremities of an open path:
    if ((_attributes&sim_pathproperty_closed_path)==0)
    { // We simply use the same Y vector as the next/preceeding point:
        tr[0].M.axis[0]=(tr[1].M.axis[1]^tr[0].M.axis[2]).getNormalized();
        tr[0].M.axis[1]=(tr[0].M.axis[2]^tr[0].M.axis[0]).getNormalized();
        
        int r=(int)tr.size()-1;
        tr[r].M.axis[0]=(tr[r-1].M.axis[1]^tr[r].M.axis[2]).getNormalized();
        tr[r].M.axis[1]=(tr[r].M.axis[2]^tr[r].M.axis[0]).getNormalized();
    }
    // 5. Now check if we have a special case (all points colinear):
    bool colinear=true;
    C3Vector v(tr[1].X-tr[0].X);
    for (int i=1;i<int(tr.size())-1;i++)
    {
        C3Vector v2(tr[i+1].X-tr[i+0].X);
        if (!v.isColinear(v2,0.99999f))
            colinear=false;
    }
    if (colinear)
    { // We have a special case.
        C3Vector v(tr[1].X-tr[0].X);
        C3X3Matrix m;
        m.setIdentity();
        m.axis[2]=v.getNormalized();
        if (m.axis[2](2)<0.9f)
        {
            m.axis[1](0)=0.0f;
            m.axis[1](2)=1.0f;
        }
        else
        {
            m.axis[1](0)=1.0f;
            m.axis[1](2)=0.0f;
        }
        m.axis[1](1)=0.0f;
        m.axis[0]=(m.axis[1]^m.axis[2]).getNormalized();
        m.axis[1]=(m.axis[2]^m.axis[0]).getNormalized();
        for (int i=0;i<int(tr.size());i++)
            tr[i].M=m;
    }
    if (_attributes&sim_pathproperty_keep_x_up)
    { // We treat the special where we try to keep x up
        // We keep each z-axis same, but also make sure the x-axis is aligned with the path object's z-axis:
        for (int i=0;i<int(tr.size());i++)
        {
            if (tr[i].M.axis[2]*C3Vector::unitZVector<1.0f)
            { // general case:
                tr[i].M.axis[0]=C3Vector::unitZVector;
                tr[i].M.axis[1]=(tr[i].M.axis[2]^tr[i].M.axis[0]).getNormalized();
                tr[i].M.axis[0]=(tr[i].M.axis[1]^tr[i].M.axis[2]).getNormalized();
            }
            else
            { // Special case with infinite solutions. We just pick the identity orientation!
                tr[i].M.setIdentity();
            }
        }
    }
    // 6. We apply the changes:
    _applyDistinctConsecutivePoints(pc,tr,ptIndices);
}

bool CPathCont::getConfigurationOnBezierCurveClosestTo(C3Vector& pt,C7Vector& conf)
{
    // 1. Handling of special cases:
    if (_bezierPathPoints.size()==0)
        return(false);
    if (_bezierPathPoints.size()==1)
    {
        conf=_bezierPathPoints[0]->getTransformation();
        return(true);
    }
    float d=SIM_MAX_FLOAT;
    // 2. We search for the closest bezier point (this gives us an approximate distance value to speed-up the calculations later on)
    for (int i=0;i<int(_bezierPathPoints.size());i++)
    {
        float dd=(pt-_bezierPathPoints[i]->getTransformation().X).getLength();
        if (dd<d)
        {
            d=dd;
            conf=_bezierPathPoints[i]->getTransformation();
        }
    }
    // 3. We search for the closest distance on the bezier path (only if d is different from 0!):
    if (d!=0.0f)
    {
        for (int i=0;i<int(_bezierPathPoints.size());i++)
        {
            CBezierPathPoint* bez0=_bezierPathPoints[i];
            CBezierPathPoint* bez1=nullptr;
            if (i==int(_bezierPathPoints.size())-1)
            {
                if ( (_attributes&sim_pathproperty_closed_path)&&(i!=2) )
                    bez1=_bezierPathPoints[0]; // closing the loop here (first point)
                else
                    break;
            }
            else
                bez1=_bezierPathPoints[i+1];
            C3Vector v0(bez0->getTransformation().X);
            C3Vector v1(bez1->getTransformation().X);
            C3Vector vd(v1-v0);
            C3Vector theSearchedPt;
            if (CMeshRoutines::getMinDistBetweenSegmentAndPoint_IfSmaller(v0,vd,pt,d,theSearchedPt))
            {
                float vdL=vd.getLength();
                if (vdL==0.0f)
                { // Coinciding points:
                    conf=bez0->getTransformation();
                }
                else
                {
                    float l=(theSearchedPt-v0).getLength();
                    conf.buildInterpolation(bez0->getTransformation(),bez1->getTransformation(),l/vdL);
                }
            }
        }
    }
    return(true);
}

bool CPathCont::getPositionOnPathClosestTo(const C3Vector& pt,float& distOnPath)
{
    // 1. Handling of special cases:
    if (_bezierPathPoints.size()==0)
        return(false);
    if (_bezierPathPoints.size()==1)
    {
        distOnPath=0.0f;
        return(true);
    }
    float d=SIM_MAX_FLOAT;
    C3Vector pppt(pt);
    for (int i=0;i<int(_bezierPathPoints.size());i++)
    {
        CBezierPathPoint* bez0=_bezierPathPoints[i];
        CBezierPathPoint* bez1=nullptr;
        if (i==int(_bezierPathPoints.size())-1)
        {
            if ( (_attributes&sim_pathproperty_closed_path)&&(i!=2) )
            {
                bez1=_bezierPathPoints[0]; // closing the loop here (first point)
            }
            else
                break;
        }
        else
            bez1=_bezierPathPoints[i+1];
        C3Vector v0(bez0->getTransformation().X);
        C3Vector v1(bez1->getTransformation().X);
        C3Vector vd(v1-v0);
        C3Vector theSearchedPt;
        if (CMeshRoutines::getMinDistBetweenSegmentAndPoint_IfSmaller(v0,vd,pppt,d,theSearchedPt))
        {
            float vdL=vd.getLength();
            if (vdL==0.0f)
            { // Coinciding points:
                distOnPath=bez0->virtualCumulativeLength;
            }
            else
            {
                float l=(theSearchedPt-v0).getLength();
                float c=l/vdL;
                if ( (i!=0)||((_attributes&sim_pathproperty_closed_path)==0) ) // added this condtion on 22/02/2012: bezier point 0 has the total virtual cumulative length (bezier path length!) for closed paths!
                    distOnPath=bez0->virtualCumulativeLength*(1.0f-c)+bez1->virtualCumulativeLength*c;
                else
                    distOnPath=bez1->virtualCumulativeLength*c; 
            }
        }
    }
    return(true);
}

void CPathCont::removeAllSimplePathPoints()
{
    for (int i=0;i<int(_simplePathPoints.size());i++)
        delete _simplePathPoints[i];
    _simplePathPoints.clear();
    _removeAllBezierPathPoints();
    actualizePath();
}

void CPathCont::rollPathPoints(bool forward)
{
    if (_simplePathPoints.size()<2)
        return;
    if (forward)
    {
        _simplePathPoints.insert(_simplePathPoints.begin(),_simplePathPoints[_simplePathPoints.size()-1]);
        _simplePathPoints.pop_back();
    }
    else
    {
        _simplePathPoints.push_back(_simplePathPoints[0]);
        _simplePathPoints.erase(_simplePathPoints.begin());
    }
    actualizePath();
}

void CPathCont::addSimplePathPoint(CSimplePathPoint* it)
{
    insertSimplePathPoint(it,int(_simplePathPoints.size()));
}

void CPathCont::insertSimplePathPoint(CSimplePathPoint* it,int position)
{
    tt::limitValue(0,int(_simplePathPoints.size()),position);
    _simplePathPoints.insert(_simplePathPoints.begin()+position,it);
    actualizePath();
}

void CPathCont::removeSimplePathPoint(int position)
{
    if ( (position<0)||(position>=int(_simplePathPoints.size())) )
        return;
    delete _simplePathPoints[position];
    _simplePathPoints.erase(_simplePathPoints.begin()+position);
    actualizePath();
}

CSimplePathPoint* CPathCont::getSimplePathPoint(int position)
{
    if ( (position<0)||(position>=int(_simplePathPoints.size())) )
        return(nullptr);
    return(_simplePathPoints[position]);
}
CBezierPathPoint* CPathCont::getBezierPathPoint(int position)
{
    if ( (position<0)||(position>=int(_bezierPathPoints.size())) )
        return(nullptr);
    return(_bezierPathPoints[position]);
}
int CPathCont::getSimplePathPointCount()
{
    return((int)_simplePathPoints.size());
}
int CPathCont::getBezierPathPointCount()
{
    return((int)_bezierPathPoints.size());
}

CBezierPathPoint* CPathCont::_addBezierPathPoint(const C7Vector& transf,float maxRelAbsVelocity,float onSpotDistance,unsigned short auxFlags,const float auxChannels[4])
{
    CBezierPathPoint* it=new CBezierPathPoint(transf);
    it->setMaxRelAbsVelocity(maxRelAbsVelocity);
    it->setOnSpotDistance(onSpotDistance);
    it->setAuxFlags(auxFlags);
    it->setAuxChannels(auxChannels);
    _bezierPathPoints.push_back(it);
    return(it);
}

void CPathCont::actualizePath()
{
    _pathModifID++;
    if (!_actualizationEnabled)
        return;
    _recomputeBezierPoints();
    _removeDoubleBezierPoints(0.0001f,0.5f*degToRad_f);
    if (getBezierPathPointCount()<3) // Path cannot be closed if less than 3 Bezier points
        _attributes=(_attributes|sim_pathproperty_closed_path)-sim_pathproperty_closed_path;
    if ((_attributes&sim_pathproperty_automatic_orientation)!=0)
    {
        _computeAutomaticOrientation(((std::vector<CPathPoint*>*)&_simplePathPoints)[0]);
        _computeAutomaticOrientation(((std::vector<CPathPoint*>*)&_bezierPathPoints)[0]);
    }
    _recomputeBezierPathElementLengths();
    _recomputeBezierPathMaxVelocities();
    setPosition(getPosition());
    setNominalVelocity(getNominalVelocity()); // Is this still needed??
    setTargetNominalVelocity(getTargetNominalVelocity());
}

void CPathCont::enableActualization(bool enable)
{
    _actualizationEnabled=enable;
}

int CPathCont::_removeDoubleBezierPoints(float linTol,float angTol)
{ // ret val is the number of removed points
    int retVal=0;
    for (int i=0;i<int(_bezierPathPoints.size())-1;i++)
    {
        CBezierPathPoint* it0=_bezierPathPoints[i+0];
        CBezierPathPoint* it1=_bezierPathPoints[i+1];
        C7Vector tr0(it0->getTransformation());
        C7Vector tr1(it1->getTransformation());
        if ((tr0.X-tr1.X).getLength()<linTol)
        { // same position!
            if (tr0.Q.getAngleBetweenQuaternions(tr1.Q)<angTol)
            { // same orientation!
                if (it0->getOnSpotDistance()==it1->getOnSpotDistance())
                { // same on spot distance!
                    // We have to remove this point, or rather merge both points:
                    it0->setMaxRelAbsVelocity(it0->getMaxRelAbsVelocity()+it1->getMaxRelAbsVelocity()*0.5f);
                    delete it1;
                    _bezierPathPoints.erase(_bezierPathPoints.begin()+i+1 );
                    retVal++;
                    i--;
                }
            }
        }
    }
    return(retVal);
}


void CPathCont::_recomputeBezierPathElementLengths()
{
    float cumulLength=0.0f;
    float cumulAngle=0.0f;
    float cumulOnSpotDistance=0.0f;
    float virtualCumulLength=0.0f;
    for (int i=0;i<int(_bezierPathPoints.size());i++)
    {
        CBezierPathPoint* it0=_bezierPathPoints[i+0];
        CBezierPathPoint* it1=nullptr;
        if (i==int(_bezierPathPoints.size())-1)
            it1=_bezierPathPoints[0];
        else
            it1=_bezierPathPoints[i+1];
        C7Vector tr0(it0->getTransformation());
        C7Vector tr1(it1->getTransformation());
        float dl=(tr0.X-tr1.X).getLength();
        float da=tr0.Q.getAngleBetweenQuaternions(tr1.Q)*_angleVarToDistanceCoeff;
        float onSpotDl=fabs(it0->getOnSpotDistance()-it1->getOnSpotDistance())*_onSpotDistanceToDistanceCoeff;
        float vdl=dl;
        if (_pathLengthCalculationMethod==sim_distcalcmethod_dl)
            vdl=dl;
        if (_pathLengthCalculationMethod==sim_distcalcmethod_dac)
            vdl=da;
        if (_pathLengthCalculationMethod==sim_distcalcmethod_max_dl_dac)
            vdl=SIM_MAX(dl,da);
        if (_pathLengthCalculationMethod==sim_distcalcmethod_dl_and_dac)
            vdl=dl+da;
        if (_pathLengthCalculationMethod==sim_distcalcmethod_sqrt_dl2_and_dac2)
            vdl=sqrtf(dl*dl+da*da);
        if (_pathLengthCalculationMethod==sim_distcalcmethod_dl_if_nonzero)
        {
            vdl=dl;
            if (dl<0.00005f) // Was dl==0.0f before (tolerance problem). Changed on 1/4/2011
                vdl=da;
        }
        if (_pathLengthCalculationMethod==sim_distcalcmethod_dac_if_nonzero)
        {
            vdl=da;
            if (da<0.01f*degToRad_f) // Was da==0.0f before (tolerance problem). Changed on 1/4/2011
                vdl=dl;
        }
        vdl+=onSpotDl;
        cumulLength+=dl;
        cumulAngle+=da;
        cumulOnSpotDistance+=onSpotDl;
        virtualCumulLength+=vdl;
        if ( (_attributes&sim_pathproperty_closed_path)||(i!=int(_bezierPathPoints.size())-1) )
        {
            it1->virtualCumulativeLength=virtualCumulLength;
            it1->virtualSegmentLength=vdl;
            it1->cumulativeLength=cumulLength;
            it1->segmentLength=dl;
            it1->cumulativeAngle=cumulAngle;
            it1->segmentAngleVariation=da;
            it1->cumulativeOnSpotDistance=cumulOnSpotDistance;
            it1->segmentOnSpotDistance=onSpotDl;
        }
        else
        { // This is the first point in an open path:
            it1->virtualCumulativeLength=0.0f;
            it1->virtualSegmentLength=0.0f;
            it1->cumulativeLength=0.0f;
            it1->segmentLength=0.0f;
            it1->cumulativeAngle=0.0f;
            it1->segmentAngleVariation=0.0f;
            it1->cumulativeOnSpotDistance=0.0f;
            it1->segmentOnSpotDistance=0.0f;
        }
    }
}

void CPathCont::_recomputeBezierPathMaxVelocities()
{
    if (getBezierPathPointCount()==0)
        return;

    // 1. Set all velocities as requested by the interpolation:
    std::vector<float> maxOk;
    for (int i=0;i<int(_bezierPathPoints.size());i++)
        maxOk.push_back(_bezierPathPoints[i]->getMaxRelAbsVelocity());


    // 2. Reset all velocities if an automatic velocity profile mode is active:
    if ( (_attributes&sim_pathproperty_auto_velocity_profile_translation_deprecated)||
        (_attributes&sim_pathproperty_auto_velocity_profile_rotation_deprecated) )
    {

        for (int i=0;i<int(maxOk.size());i++)
            maxOk[i]=1.0f; // We reset all values (we want automatic calculation!)
    }


    // 3. Take care of the extremities if present:
    if (_attributes&sim_pathproperty_endpoints_at_zero_deprecated)
    {
        if ( (_attributes&sim_pathproperty_closed_path)==0 )
        {
            maxOk[0]=0.0f; // first point
            maxOk[maxOk.size()-1]=0.0f; // last point
        }
    }
    
    // 4. For each point, have a non-zero vector for last lin. path, and next lin. path:
    // (this is to handle coincident points)
    if (_attributes&sim_pathproperty_auto_velocity_profile_translation_deprecated)
    {
        C3Vector prevVector(C3Vector::zeroVector);
        std::vector<C3Vector> previousVectors(_bezierPathPoints.size(),prevVector);
        std::vector<C3Vector> nextVectors(_bezierPathPoints.size(),prevVector);
        C7Vector prevTransf,currTransf;
        for (int i=0;i<int(_bezierPathPoints.size())*2;i++)
        {
            if ( ((_attributes&sim_pathproperty_closed_path)==0)&&(i==int(_bezierPathPoints.size())) )
                break;
            int j=i;
            if (j==0)
            {
                currTransf=_bezierPathPoints[j]->getTransformation();
                prevTransf=currTransf;
            }
            else
            {
                if (j>=int(_bezierPathPoints.size()))
                    j-=(int)_bezierPathPoints.size();
                currTransf=_bezierPathPoints[j]->getTransformation();
            }
            C3Vector nv(currTransf.X-prevTransf.X);
            if (nv.getLength()!=0.0f)
                prevVector=nv;
            prevTransf=currTransf;
            previousVectors[j]=prevVector;
        }
        for (int i=int(_bezierPathPoints.size())*2-1;i>=0;i--)
        {
            if ( ((_attributes&sim_pathproperty_closed_path)==0)&&(i==int(_bezierPathPoints.size())-1) )
                break;
            int j=i;
            if (j==int(_bezierPathPoints.size())*2-1)
            {
                j=int(_bezierPathPoints.size())-1;
                currTransf=_bezierPathPoints[j]->getTransformation();
                prevTransf=currTransf;
            }
            else
            {
                if (j>=int(_bezierPathPoints.size()))
                    j-=(int)_bezierPathPoints.size();
                currTransf=_bezierPathPoints[j]->getTransformation();
            }
            C3Vector nv(currTransf.X-prevTransf.X);
            if (nv.getLength()!=0.0f)
                prevVector=nv*-1.0f;
            prevTransf=currTransf;
            nextVectors[j]=prevVector;
        }

        // Take care of the velocity limitations linked to turning circle diameter:
        for (int i=0;i<int(_bezierPathPoints.size());i++)
        {
            if ( ((_attributes&sim_pathproperty_closed_path)==0)&&(i==int(_bezierPathPoints.size())-1) )
                break;
            if ( (_attributes&sim_pathproperty_closed_path)||(i!=0) )
            {
                CBezierPathPoint* bp_b=nullptr;
                if (i==0)
                    bp_b=_bezierPathPoints[_bezierPathPoints.size()-1];
                else
                    bp_b=_bezierPathPoints[i-1];
                CBezierPathPoint* bp_0=_bezierPathPoints[i+0];
                CBezierPathPoint* bp_a=nullptr;
                if (i==int(_bezierPathPoints.size())-1)
                    bp_a=_bezierPathPoints[0];
                else
                    bp_a=_bezierPathPoints[i+1];
                C7Vector tb(bp_b->getTransformation());
                C7Vector t0(bp_0->getTransformation());
                C7Vector ta(bp_a->getTransformation());
                C3Vector v0(t0.X-tb.X);
                C3Vector v1(ta.X-t0.X);
                if ((v0.getLength()!=0.0f)||(v1.getLength()!=0.0f))
                { // A point that is coincident with its previous and next points is not handled
                    if (v0.getLength()==0.0f)
                        v0=previousVectors[i+0]; // we get the last non-zero vector
                    if (v1.getLength()==0.0f)
                        v1=nextVectors[i+0]; // we get the next non-zero vector
                    if ((v0.getLength()!=0.0f)&&(v1.getLength()!=0.0f))
                    { // unless we have a completely coincident trajectory (e.g. on-spot rotation), this should always pass:
                        float angle=v0.getAngle(v1);
                        float len=SIM_MIN(v0.getLength(),v1.getLength());
                        float lengthByAngle=len/angle;
                        float rv=lengthByAngle/(2.0f*_avp_turningCircleRadiusForHalfVelocity);
                        if (rv<maxOk[i+0])
                            maxOk[i+0]=rv;
                    }
                }
            }
        }
    }

    // 5. Take care of the velocity limitations linked to "turning circle diameter" of rotations!:
    if (_attributes&sim_pathproperty_auto_velocity_profile_rotation_deprecated)
    {
        for (int i=0;i<int(_bezierPathPoints.size());i++)
        {
            if ( ((_attributes&sim_pathproperty_closed_path)==0)&&(i==int(_bezierPathPoints.size())-1) )
                break;
            if ( (_attributes&sim_pathproperty_closed_path)||(i!=0) )
            {
                CBezierPathPoint* bp_b=nullptr;
                if (i==0)
                    bp_b=_bezierPathPoints[_bezierPathPoints.size()-1];
                else
                    bp_b=_bezierPathPoints[i-1];
                CBezierPathPoint* bp_0=_bezierPathPoints[i+0];
                CBezierPathPoint* bp_a=nullptr;
                if (i==int(_bezierPathPoints.size())-1)
                    bp_a=_bezierPathPoints[0];
                else
                    bp_a=_bezierPathPoints[i+1];
                C7Vector tb(bp_b->getTransformation());
                C7Vector t0(bp_0->getTransformation());
                C7Vector ta(bp_a->getTransformation());
                if ((tb.Q.getAngleBetweenQuaternions(t0.Q)>0.0f)||(t0.Q.getAngleBetweenQuaternions(ta.Q)>0.0f))
                { // A point that is coincident with its previous AND next orientations is not handled
                    C4Vector v0(tb.Q.getInverse()*t0.Q);
                    C4Vector v1(t0.Q.getInverse()*ta.Q);
                    C4Vector tmpQ(v0.getAngleAndAxis());
                    C3Vector axisA(tmpQ(1),tmpQ(2),tmpQ(3)); // tmpQ(0) is the angle!
                    tmpQ=v1.getAngleAndAxis();
                    C3Vector axisB(tmpQ(1),tmpQ(2),tmpQ(3)); // tmpQ(0) is the angle!
                    float rv=1.0f;
                    if ((tb.Q.getAngleBetweenQuaternions(t0.Q)==0.0f)||(t0.Q.getAngleBetweenQuaternions(ta.Q)==0.0f))
                        rv=_avp_relativeVelocityAtRotationAxisChange;
                    else
                    {
                        if (axisA.getAngle(axisB)>60.0f*degToRad_f)
                            rv=_avp_relativeVelocityAtRotationAxisChange;
                    }
                    if (rv<maxOk[i+0])
                        maxOk[i+0]=rv;
                }
            }
        }
    }


    // 6. Now make sure all maximum velocities can be reached (some will shadow others!)
    for (int i=0;i<int(maxOk.size());i++)
    {
        // 1. Forward direction:
        float cumulPos=0.0f;
        float cumulAngle=0.0f;
        for (int j=1;j<int(maxOk.size());j++)
        {
            int k=i+j;
            if (k>=int(maxOk.size()))
            {
                if (_attributes&sim_pathproperty_closed_path)
                    k-=(int)maxOk.size();
                else
                    break; // path is open
            }
            cumulPos+=_bezierPathPoints[k]->virtualSegmentLength;
            cumulAngle+=_bezierPathPoints[k]->segmentAngleVariation*_angleVarToDistanceCoeff;
            float w=sqrtf(2.0f*cumulPos*_avp_relativeAcceleration+maxOk[i]*maxOk[i]);
            if (maxOk[k]>w)
                maxOk[k]=w; // We limit the vel. at this pos
            else
                break;
        }
        // 2. Backward direction:
        cumulPos=0.0f;
        cumulAngle=0.0f;
        int oldK=i;
        for (int j=1;j<int(maxOk.size());j++)
        {
            int k=i-j;
            if (k<0)
            {
                if (_attributes&sim_pathproperty_closed_path)
                    k+=(int)maxOk.size();
                else
                    break; // path is open
            }
            cumulPos+=_bezierPathPoints[oldK]->virtualSegmentLength; 
            cumulAngle+=_bezierPathPoints[oldK]->segmentAngleVariation*_angleVarToDistanceCoeff; 
            oldK=k;
            float w=sqrtf(2.0f*cumulPos*_avp_relativeAcceleration+maxOk[i]*maxOk[i]);
            if (maxOk[k]>w)
                maxOk[k]=w; // We limit the vel. at this pos
            else
                break;
        }
    }

    // 7. Done!
    for (int i=0;i<int(_bezierPathPoints.size());i++)
        _bezierPathPoints[i]->setMaxRelAbsVelocity(maxOk[i]);
}


bool CPathCont::_getNextBezierPathPointIndex(int currentIndex,int& nextIndex)
{
    if ( (currentIndex<0)||(currentIndex>=int(_bezierPathPoints.size())) )
        return(false);
    if (_bezierPathPoints.size()<2)
        return(false);
    int ni=currentIndex+1;
    if (ni>=int(_bezierPathPoints.size()))
    {
        if ( (_attributes&sim_pathproperty_closed_path)==0 )
            return(false);
        ni=0;
    }
    nextIndex=ni;
    return(true);
}

float CPathCont::getBezierVirtualPathLength()
{
    if (_bezierPathPoints.size()<2)
        return(0.0f);
    if (_attributes&sim_pathproperty_closed_path)
        return(_bezierPathPoints[0]->virtualCumulativeLength);
    return(_bezierPathPoints[_bezierPathPoints.size()-1]->virtualCumulativeLength);
}

float CPathCont::getBezierNormalPathLength()
{
    if (_bezierPathPoints.size()<2)
        return(0.0f);
    if (_attributes&sim_pathproperty_closed_path)
        return(_bezierPathPoints[0]->cumulativeLength);
    return(_bezierPathPoints[_bezierPathPoints.size()-1]->cumulativeLength);
}

float CPathCont::getBezierAngularPathLength()
{
    if (_bezierPathPoints.size()<2)
        return(0.0f);
    if (_attributes&sim_pathproperty_closed_path)
        return(_bezierPathPoints[0]->cumulativeAngle);
    return(_bezierPathPoints[_bezierPathPoints.size()-1]->cumulativeAngle);
}

void CPathCont::_getInterpolatedBezierCurveData(int index0,float t,int& auxFlags,float auxChannels[4])
{
    int index1=index0+1;
    if (index1>=int(_bezierPathPoints.size()))
        index1=0;
    if (t<0.5f)
        auxFlags=_bezierPathPoints[index0]->getAuxFlags();
    else
        auxFlags=_bezierPathPoints[index1]->getAuxFlags();
    float ac0[4];
    float ac1[4];
    _bezierPathPoints[index0]->getAuxChannels(ac0);
    _bezierPathPoints[index1]->getAuxChannels(ac1);
    for (int i=0;i<4;i++)
        auxChannels[i]=ac0[i]*(1.0f-t)+ac1[i]*t;
}


C7Vector CPathCont::_getInterpolatedBezierCurvePoint(int index0,float t)
{
    C7Vector retVal;
    int index1=index0+1;
    if (index1>=int(_bezierPathPoints.size()))
        index1=0;
    retVal.buildInterpolation(_bezierPathPoints[index0]->getTransformation(),_bezierPathPoints[index1]->getTransformation(),t);
    return(retVal);
}

bool CPathCont::_getPointOnBezierCurveAtVirtualDistance(float& l,int& index0,float& t)
{
    if (_bezierPathPoints.size()<2)
    {
        if (_bezierPathPoints.size()==0)
        { // Path is empty
            index0=-1;
            t=0.0f;
            l=0.0f;
            return(false);
        }
        // Path has just one point
        index0=0;
        t=0.0f;
        l=0.0f;
        return(true);
    }
    float totLength=getBezierVirtualPathLength();
    if (totLength==0.0f)
    { // Path length is 0
        index0=0;
        t=0.0f;
        l=0.0f;
        return(true);
    }
    if (_attributes&sim_pathproperty_closed_path)
    {
        l=CMath::robustFmod(l,totLength);
        if (l<0.0f)
            l+=totLength;
    }
    else
        tt::limitValue(0.0f,totLength,l);
    int cIndex=0;
    int nIndex=0;
    int passCnt=0;
    while (_getNextBezierPathPointIndex(cIndex,nIndex))
    {
        if (_bezierPathPoints[nIndex]->virtualCumulativeLength>=l)
        { // We found the position
            float dl=_bezierPathPoints[nIndex]->virtualSegmentLength;
            index0=cIndex;
            float ps=l-_bezierPathPoints[cIndex]->virtualCumulativeLength;
            if (passCnt==0)
                ps=l;
            if (dl==0.0f)
                t=0.0f;
            else
                t=ps/dl;
            return(true);
        }
        cIndex=nIndex;
        passCnt++;
    }
    return(false);
}

bool CPathCont::getAuxDataOnBezierCurveAtNormalizedVirtualDistance(float l,int& auxFlags,float auxChannels[4])
{ // l is between 0 and 1!
    float ll=l*getBezierVirtualPathLength();
    int index;
    float t;
    if (!_getPointOnBezierCurveAtVirtualDistance(ll,index,t))
        return(false);
    _getInterpolatedBezierCurveData(index,t,auxFlags,auxChannels);
    return(true);
}


bool CPathCont::getTransformationOnBezierCurveAtNormalizedVirtualDistance(float l,C7Vector& tr)
{ // l is between 0 and 1!
    float ll=l*getBezierVirtualPathLength();
    int index;
    float t;
    if (!_getPointOnBezierCurveAtVirtualDistance(ll,index,t)) 
        return(false);
    tr=_getInterpolatedBezierCurvePoint(index,t);
    return(true);
}

bool CPathCont::getPointOnBezierCurveAtNormalDistance(float& l,int& index0,float& t)
{
    if (_bezierPathPoints.size()<2)
    {
        if (_bezierPathPoints.size()==0)
        { // Path is empty
            index0=-1;
            t=0.0f;
            l=0.0f;
            return(false);
        }
        // Path has just one point
        index0=0;
        t=0.0f;
        l=0.0f;
        return(true);
    }
    float totLength=getBezierNormalPathLength();
    if (totLength==0.0f)
    { // Path length is 0
        index0=0;
        t=0.0f;
        l=0.0f;
        return(true);
    }
    if (_attributes&sim_pathproperty_closed_path)
    {
        tt::limitValue(0.0f,l,l);
        l=CMath::robustFmod(l,totLength);
    }
    else
        tt::limitValue(0.0f,totLength,l);
    int cIndex=0;
    int nIndex=0;
    int passCnt=0;
    while (_getNextBezierPathPointIndex(cIndex,nIndex))
    {
        if (_bezierPathPoints[nIndex]->cumulativeLength>=l)
        { // We found the position
            float dl=_bezierPathPoints[nIndex]->segmentLength;
            index0=cIndex;
            float ps=l-_bezierPathPoints[cIndex]->cumulativeLength;
            if (passCnt==0)
                ps=l;
            if (dl==0.0f)
                t=0.0f;
            else
                t=ps/dl;
            return(true);
        }
        cIndex=nIndex;
        passCnt++;
    }
    return(false);
}

void CPathCont::setPosition(double p)
{
    if (_attributes&sim_pathproperty_closed_path)
    {
        p=CMath::robustmod(p,getBezierVirtualPathLength());
        while (p<0.0)
            p+=getBezierVirtualPathLength();
    }
    else
        p=tt::getLimitedDouble(0.0,getBezierVirtualPathLength(),p);
    _position=p;

    int ind;
    float t;
    float l=float(_position);
    if (_getPointOnBezierCurveAtVirtualDistance(l,ind,t))
        _startPosition=_getInterpolatedBezierCurvePoint(ind,t).X;
    // Following is not elegant at all. Change later! (maybe simply merge the CPathCont and CPath)
    CPath* parentPathObject=nullptr;
    for (int i=0;i<int(App::ct->objCont->pathList.size());i++)
    {
        parentPathObject=App::ct->objCont->getPath(App::ct->objCont->pathList[i]);
        if (parentPathObject!=nullptr)
        { // Can happen during object destruction!
            if (parentPathObject->pathContainer==this)
                break;
        }
    }
    _handleAttachedDummies(parentPathObject);
    setNominalVelocity(getNominalVelocity()); // Is this still needed???// To make sure velocity is within allowed range
}
double CPathCont::getPosition()
{
    return(_position); 
}

double CPathCont::getPosition_linearComponentOnly()
{
    float t;
    int index0,index1;
    float posCop=float(_position);
    if (_getPointOnBezierCurveAtVirtualDistance(posCop,index0,t))
    {
        index1=index0;
        {
            index1=index0+1;
            if (index1>=getBezierPathPointCount())
                index1=0;
        }
        float cumul=0.0f;
        if (index0!=0)
            cumul=_bezierPathPoints[index0]->cumulativeLength;
        return(cumul+_bezierPathPoints[index1]->segmentLength*t);
    }
    else
        return(0.0f);
}

double CPathCont::getPosition_angularComponentOnly()
{
    float t;
    int index0,index1;
    float posCop=float(_position);
    if (_getPointOnBezierCurveAtVirtualDistance(posCop,index0,t))
    {
        index1=index0;
        {
            index1=index0+1;
            if (index1>=getBezierPathPointCount())
                index1=0;
        }
        float cumul=0.0f;
        if (index0!=0)
            cumul=_bezierPathPoints[index0]->cumulativeAngle;
        return(cumul+_bezierPathPoints[index1]->segmentAngleVariation*t);
    }
    else
    {
        return(0.0f);
    }
}

void CPathCont::setNominalVelocity(float v)
{ 
    tt::limitValue(-1000.0f,1000.0f,v);
    _nominalVelocity=v; 
}

float CPathCont::getNominalVelocity()
{
    return(_nominalVelocity);
}

void CPathCont::setTargetNominalVelocity(float v)
{
    tt::limitValue(-1000.0f,1000.0f,v);
    _targetNominalVelocity=v;
}

float CPathCont::getTargetNominalVelocity()
{
    return(_targetNominalVelocity);
}

void CPathCont::setPosition_ratio(double p)
{
    double l=getBezierVirtualPathLength();
    setPosition(l*p);
}

float CPathCont::getPosition_ratio()
{
    float l=getBezierVirtualPathLength();
    if (l==0.0f)
        return(0.0f);
    return(float(_position/l));
}

void CPathCont::_recomputeBezierPoints()
{
    _removeAllBezierPathPoints();
    //  check if at least 3 POINTS!!!
    if (_simplePathPoints.size()==0)
        return;
    float auxChannels[4];
    if (_simplePathPoints.size()==1)
    {
        CSimplePathPoint* itm=_simplePathPoints[0];
        itm->getAuxChannels(auxChannels);
        _addBezierPathPoint(itm->getTransformation(),itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),itm->getAuxFlags(),auxChannels);
        return;
    }
    if (_simplePathPoints.size()==2)
    {
        CSimplePathPoint* itm=_simplePathPoints[0];
        itm->getAuxChannels(auxChannels);
        _addBezierPathPoint(itm->getTransformation(),itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),itm->getAuxFlags(),auxChannels);

        itm=_simplePathPoints[1];
        itm->getAuxChannels(auxChannels);
        _addBezierPathPoint(itm->getTransformation(),itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),itm->getAuxFlags(),auxChannels);
        return;
    }
    // Here we have at least 3 points!
    for (int i=0;i<int(_simplePathPoints.size());i++)
    {
        CSimplePathPoint* itb=nullptr;
        if (i!=0)
            itb=_simplePathPoints[i-1];
        CSimplePathPoint* itm=_simplePathPoints[i+0];
        CSimplePathPoint* ita=nullptr;
        if (i!=int(_simplePathPoints.size())-1)
            ita=_simplePathPoints[i+1];
        if ((_attributes&sim_pathproperty_closed_path)!=0)
        { // We want a closed path!
            if (itb==nullptr)
                itb=_simplePathPoints[_simplePathPoints.size()-1];
            if (ita==nullptr)
                ita=_simplePathPoints[0];
        }
        if ( (ita==nullptr)||(itb==nullptr) )
        {
            itm->getAuxChannels(auxChannels);
            if (itb==nullptr)
                _addBezierPathPoint(itm->getTransformation(),itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),itm->getAuxFlags(),auxChannels); // first point, open path
            else
                _addBezierPathPoint(itm->getTransformation(),itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),itm->getAuxFlags(),auxChannels); // last point, open path
        }
        else
        {
            C7Vector tr0(itb->getTransformation());
            C7Vector tr1(itm->getTransformation());
            C7Vector tr2(ita->getTransformation());

            float auxChannels0[4];
            float auxChannels1[4];
            float auxChannels2[4];
            itb->getAuxChannels(auxChannels0);
            itm->getAuxChannels(auxChannels1);
            ita->getAuxChannels(auxChannels2);

            unsigned short auxFlags1=itm->getAuxFlags();

            bool linOk=false;
            bool angOk=false;
            bool onSpotDist=false;
            if (itm->getBezierPointCount()!=1)
            {
                if ( ((tr0.X-tr1.X).getLength()!=0.0f)&&((tr1.X-tr2.X).getLength()!=0.0f)&&((tr0.X-tr1.X).getNormalized()*(tr1.X-tr2.X).getNormalized()<0.99999f) )
                    linOk=true; // Here we have the Bezier interpolation thing for 3 non-coincident//, non-colinear points

                if ( (tr0.Q.getAngleBetweenQuaternions(tr1.Q)>1.0f*degToRad_f)&&(tr1.Q.getAngleBetweenQuaternions(tr2.Q)>1.0f*degToRad_f) )
                { // We have two pairs of non-identical orientations
                    C4Vector tmpQ((tr0.Q.getInverse()*tr1.Q).getAngleAndAxis());
                    C3Vector axisA(tmpQ(1),tmpQ(2),tmpQ(3)); // tmpQ(0) is the angle!
                    tmpQ=(tr1.Q.getInverse()*tr2.Q).getAngleAndAxis();
                    C3Vector axisB(tmpQ(1),tmpQ(2),tmpQ(3)); // tmpQ(0) is the angle!
                    axisA.normalize();
                    axisB.normalize();
                    if (axisA*axisB<0.99f)
                    { // The two rotations are not identical (e.g. not same axis and direction)
                        angOk=true; // Here we have the Bezier interpol thing for rotations
                    }
                }
                if ((itb->getOnSpotDistance()!=itm->getOnSpotDistance())&&(itm->getOnSpotDistance()!=ita->getOnSpotDistance()))
                { // the on-spot distances are different!
                    onSpotDist=true;
                }
                if (linOk||angOk||onSpotDist)
                {
                    C7Vector trm,trn;
                    trm.buildInterpolation(tr0,tr1,0.5f);
                    trn.buildInterpolation(tr1,tr2,0.5f);
                    
                    float mxRelVel_m=(itb->getMaxRelAbsVelocity()+itm->getMaxRelAbsVelocity())*0.5f;
                    float mxRelVel_n=(itm->getMaxRelAbsVelocity()+ita->getMaxRelAbsVelocity())*0.5f;
                    float onSpot_m=(itb->getOnSpotDistance()+itm->getOnSpotDistance())*0.5f;
                    float onSpot_n=(itm->getOnSpotDistance()+ita->getOnSpotDistance())*0.5f;
                    float bf[2];
                    itm->getBezierFactors(bf[0],bf[1]);
                    C7Vector tri,trj;
                    tri.buildInterpolation(trm,tr1,1.0f-bf[0]);
                    trj.buildInterpolation(trn,tr1,1.0f-bf[1]);
                    
                    float mxRelVel_i=mxRelVel_m*bf[0]+itm->getMaxRelAbsVelocity()*(1.0f-bf[0]);
                    float mxRelVel_j=mxRelVel_n*bf[1]+itm->getMaxRelAbsVelocity()*(1.0f-bf[1]);
                    float onSpot_i=onSpot_m*bf[0]+itm->getOnSpotDistance()*(1.0f-bf[0]);
                    float onSpot_j=onSpot_n*bf[1]+itm->getOnSpotDistance()*(1.0f-bf[1]);

                    float aatb=0.5f+(1.0f-bf[0])*0.5f;
                    float aata=bf[1]*0.5f;

                    float auxChannelsInterpol[4];
                    for (int o=0;o<4;o++)
                        auxChannelsInterpol[o]=auxChannels0[o]*(1.0f-aatb)+auxChannels1[o]*aatb;

                    _addBezierPathPoint(tri,mxRelVel_i,onSpot_i,auxFlags1,auxChannelsInterpol); // first point in the bezier curve section

                    int divCount=itm->getBezierPointCount()-1;
                    float ppp0=1.0f-aatb;
                    float pppt=ppp0+aata;
                    for (int k=1;k<divCount;k++)
                    {
                        float t=float(k)*(1.0f/float(divCount));
                        C7Vector x0,x1,trpt;
                        x0.buildInterpolation(tri,tr1,t);
                        x1.buildInterpolation(tr1,trj,t);

                        float pppt0=aatb+t*pppt;
                        float pppt1=pppt0-1.0f;
                        for (int o=0;o<4;o++)
                        {
                            if (pppt0<=1.0f)
                                auxChannelsInterpol[o]=auxChannels0[o]*(1.0f-pppt0)+auxChannels1[o]*pppt0;
                            else
                                auxChannelsInterpol[o]=auxChannels1[o]*(1.0f-pppt1)+auxChannels2[o]*pppt1;
                        }

                        float mxRelVel_x0=mxRelVel_i*(1.0f-t)+itm->getMaxRelAbsVelocity()*t;
                        float mxRelVel_x1=itm->getMaxRelAbsVelocity()*(1.0f-t)+mxRelVel_j*t;
                        float onSpot_x0=onSpot_i*(1.0f-t)+itm->getOnSpotDistance()*t;
                        float onSpot_x1=itm->getOnSpotDistance()*(1.0f-t)+onSpot_j*t;
                        trpt.buildInterpolation(x0,x1,t);

                        float mxRelVel_trpt=mxRelVel_x0*(1.0f-t)+mxRelVel_x1*(t);
                        float onSpot_trpt=onSpot_x0*(1.0f-t)+onSpot_x1*(t);
                        _addBezierPathPoint(trpt,mxRelVel_trpt,onSpot_trpt,auxFlags1,auxChannelsInterpol);
                    }

                    // We add the last point:
                    for (int o=0;o<4;o++)
                        auxChannelsInterpol[o]=auxChannels1[o]*(1.0f-aata)+auxChannels2[o]*aata;
                    _addBezierPathPoint(trj,mxRelVel_j,onSpot_j,auxFlags1,auxChannelsInterpol); // last point in the Bezier curve section
                }
            }
            if ( (!linOk)&&(!angOk)&&(!onSpotDist) )
            { // This means we either have Bezier interpol. disabled or ((coincident points or colinear points)and(two same orientations))! 
                _addBezierPathPoint(tr1,itm->getMaxRelAbsVelocity(),itm->getOnSpotDistance(),auxFlags1,auxChannels1);
            }
        }
    }
}

C3Vector CPathCont::_getPointOnBezierCubic(const C3Vector& ptBefore,const C3Vector& ptMiddle,const C3Vector& ptAfter,C3Vector& dir,float t)
{
    C3Vector x0,x1,retVal;
    x0.buildInterpolation(ptBefore,ptMiddle,t);
    x1.buildInterpolation(ptMiddle,ptAfter,t);
    retVal.buildInterpolation(x0,x1,t);
    dir=x1-x0;
    return(retVal);
}

C4Vector CPathCont::_getOrientationOnBezierCubic(const C4Vector& orBefore,const C4Vector& orMiddle,const C4Vector& orAfter,float t)
{
    C4Vector x0,x1,retVal;
    x0.buildInterpolation(orBefore,orMiddle,t);
    x1.buildInterpolation(orMiddle,orAfter,t);
    retVal.buildInterpolation(x0,x1,t);
    return(retVal);
}

void CPathCont::_removeAllBezierPathPoints()
{
    for (int i=0;i<int(_bezierPathPoints.size());i++)
        delete _bezierPathPoints[i];
    _bezierPathPoints.clear();
}

void CPathCont::setAttributes(int attrib)
{
    if ((attrib&sim_pathproperty_flat_path)&&((_attributes&sim_pathproperty_flat_path)==0))
    {
        _attributes=attrib;
        for (int i=0;i<getSimplePathPointCount();i++)
            _simplePathPoints[i]->setTransformation(_simplePathPoints[i]->getTransformation(),_attributes);
    }
    _attributes=attrib;
    if ( (_simplePathPoints.size()<3)&&(_attributes&sim_pathproperty_closed_path) )
        _attributes-=sim_pathproperty_closed_path;
    actualizePath();
}
int CPathCont::getAttributes()
{
    return(_attributes);
}
void CPathCont::setLineSize(int size)
{
    tt::limitValue(1,8,size);
    _lineSize=size;
}
int CPathCont::getLineSize()
{
    return(_lineSize);
}

void CPathCont::setSquareSize(float size)
{
    tt::limitValue(0.0001f,1.0f,size);
    _squareSize=size;   
}
float CPathCont::getSquareSize()
{
    return(_squareSize);
}

void CPathCont::initializeInitialValues(bool simulationIsRunning)
{
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {   
        _initialPosition=float(_position);
        _nominalVelocity=0.0f;
        _initialNominalVelocity=_nominalVelocity;
        _initialTargetNominalVelocity=_targetNominalVelocity;
    }
}

void CPathCont::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        _position=_initialPosition;
        _nominalVelocity=_initialNominalVelocity;
        _targetNominalVelocity=_initialTargetNominalVelocity; 
    }
    _initialValuesInitialized=false;
}

void CPathCont::setMaxAcceleration(float maxAccel)
{
    tt::limitValue(0.0001f,1000.0f,maxAccel);
    _maxAcceleration=maxAccel;
    actualizePath();
// folloiwng is done is actualizePath!  setVelocity(getVelocity()); // To make sure velocity is within allowed range
}

float CPathCont::getMaxAcceleration()
{
    return(_maxAcceleration);
}

bool CPathCont::invertSimplePathPointOrder(const std::vector<int>& selectedPoints)
{
    if (selectedPoints.size()<2)
        return(true);
    std::vector<int> pts(selectedPoints);
    std::vector<int> dummy(pts);
    tt::orderAscending(pts,dummy);
    int failed=0;
    int middleGap_up=0;
    for (int i=1;i<int(pts.size());i++)
    {
        if (pts[i]!=pts[i-1]+1)
        {
            failed++; // we have a gap
            middleGap_up=i;
        }
    }

    if (failed!=0)
    { // points do not "appear" as not consecutive. Check if they are consecutive anyway (e.g. 0,1,n-1,n):
        if (failed>1)
            return(false); // Really not consecutive!
        if ( (pts[0]!=0)||(pts[pts.size()-1]!=int(_simplePathPoints.size()-1)) )
            return(false); // Really not consecutive!
        // ok, we are consecutive.
        // We adjust the point list:
        std::vector<int> ptsTmp(pts);
        pts.clear();
        for (int i=0;i<int(ptsTmp.size());i++)
        {
            pts.push_back(ptsTmp[middleGap_up++]);
            if (middleGap_up>=int(ptsTmp.size()))
                middleGap_up=0;
        }
    }

    // ok, points are consecutive!
    if ( (pts[0]<0)||(pts[pts.size()-1]>=int(_simplePathPoints.size())) )
        return(false);
    for (int i=0;i<int(pts.size())/2;i++)
    {
        int frontIndex=pts[i];
        int backIndex=pts[pts.size()-1-i];
        CSimplePathPoint* frontIt=_simplePathPoints[frontIndex];
        _simplePathPoints[frontIndex]=_simplePathPoints[backIndex];
        _simplePathPoints[backIndex]=frontIt;
    }

    actualizePath();
    App::setLightDialogRefreshFlag();
    return(true);
}

CPathCont* CPathCont::copyYourself()
{ // Everything is copied. Don't call any functions or procedures, copy directly!
    CPathCont* newCont=new CPathCont();

    newCont->_attributes=_attributes;
    newCont->_pathModifID=_pathModifID;
    
    for (int i=0;i<int(_simplePathPoints.size());i++)
        newCont->_simplePathPoints.push_back(_simplePathPoints[i]->copyYourself());
    for (int i=0;i<int(_bezierPathPoints.size());i++)
        newCont->_bezierPathPoints.push_back(_bezierPathPoints[i]->copyYourself());

    newCont->_lineSize=_lineSize;
    newCont->_squareSize=_squareSize;
    newCont->_position=_position;
    newCont->_maxAcceleration=_maxAcceleration;
    newCont->_targetNominalVelocity=_targetNominalVelocity;
    newCont->_nominalVelocity=_nominalVelocity;
    _lineColor.copyYourselfInto(&newCont->_lineColor);
    newCont->_angleVarToDistanceCoeff=_angleVarToDistanceCoeff;
    newCont->_onSpotDistanceToDistanceCoeff=_onSpotDistanceToDistanceCoeff;
    newCont->_startPosition=_startPosition;

    newCont->_pathLengthCalculationMethod=_pathLengthCalculationMethod;


    newCont->_avp_turningCircleRadiusForHalfVelocity=_avp_turningCircleRadiusForHalfVelocity;
    newCont->_avp_relativeVelocityAtRotationAxisChange=_avp_relativeVelocityAtRotationAxisChange;
    newCont->_avp_relativeAcceleration=_avp_relativeAcceleration;

    return(newCont);
}

void CPathCont::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            for (int i=0;i<int(_simplePathPoints.size());i++)
            {
                ar.storeDataName("Pat");
                ar.setCountingMode();
                _simplePathPoints[i]->serialize(ar);
                if (ar.setWritingMode())
                    _simplePathPoints[i]->serialize(ar);
            }

            ar.storeDataName("Si2");
            ar << _lineSize << _squareSize;
            ar.flush();

            ar.storeDataName("At2");
            ar << _attributes;
            ar.flush();

            ar.storeDataName("Cl0");
            ar.setCountingMode();
            _lineColor.serialize(ar,1);
            if (ar.setWritingMode())
                _lineColor.serialize(ar,1);

            ar.storeDataName("Jm2");
            ar << float(_position) << _maxAcceleration << _nominalVelocity << _targetNominalVelocity;
            ar.flush();

            ar.storeDataName("Av3");
            ar << _angleVarToDistanceCoeff << _pathLengthCalculationMethod;
            ar.flush();

            ar.storeDataName("Av9");
            ar << _onSpotDistanceToDistanceCoeff;
            ar.flush();

            ar.storeDataName("Av2");
            ar << _avp_turningCircleRadiusForHalfVelocity << _avp_relativeVelocityAtRotationAxisChange;
            ar << _avp_relativeAcceleration;
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

                    if (theName.compare("Pat")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CSimplePathPoint* newPoint=new CSimplePathPoint();
                        newPoint->serialize(ar);
                        _simplePathPoints.push_back(newPoint);
                    }
                    if (theName.compare("Si2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _lineSize >> _squareSize;
                    }
                    if (theName.compare("At2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _attributes;
                    }
                    if (theName.compare("Cl0")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _lineColor.serialize(ar,1);
                    }
                    if (theName.compare("Jm2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        float dummyPos;
                        ar >> dummyPos >> _maxAcceleration >> _nominalVelocity >> _targetNominalVelocity;
                        _position=double(dummyPos);
                    }
                    if (theName.compare("Av3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _angleVarToDistanceCoeff >> _pathLengthCalculationMethod;
                    }
                    if (theName.compare("Av9")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _onSpotDistanceToDistanceCoeff;
                    }
                    if (theName.compare("Av2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _avp_turningCircleRadiusForHalfVelocity >> _avp_relativeVelocityAtRotationAxisChange;
                        ar >> _avp_relativeAcceleration;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            actualizePath();
        }
    }
}

#ifdef SIM_WITH_OPENGL
void CPathCont::render(bool pathEditMode,int displayAttrib,bool pathIsOnlySelectedObject,int objectID)
{
    if (pathEditMode)
        glLoadName(-1);
    if ( (displayAttrib&sim_displayattribute_selected)||pathEditMode )
    {
        if (displayAttrib&sim_displayattribute_selected)
        {
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            if (displayAttrib&sim_displayattribute_mainselection)
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorWhite);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorYellow);
        }
        else
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,0.1f,0.1f,1.0f);
        _draw(((std::vector<CPathPoint*>*)&_simplePathPoints)[0],pathEditMode,true,true,true,true,true,_lineSize,_squareSize,pathIsOnlySelectedObject,objectID);
    }
    if (pathEditMode)
        glLoadName(-1);
    _lineColor.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
    _draw(((std::vector<CPathPoint*>*)&_bezierPathPoints)[0],pathEditMode,false,((_attributes&sim_pathproperty_show_line)!=0)||pathEditMode,false,false,((_attributes&sim_pathproperty_show_orientation)!=0)||pathEditMode,_lineSize,_squareSize,pathIsOnlySelectedObject,objectID);
}

void CPathCont::_draw(std::vector<CPathPoint*>& ptCont,bool pathEditMode,bool isPath,bool showLine,bool stripplePts,bool showSquare,bool showOrientation,int lineSize,float squareSize,bool pathIsOnlySelectedObject,int objectID)
{
    if (showLine||(pathEditMode&&isPath))
    {
        if (!isPath)
            glLineWidth((float)lineSize);
        else
            glLineWidth(float(lineSize)+2); // added on 2010/09/03
        glLineStipple(1,0x00ff);
        if (stripplePts)
            glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        for (int i=0;i<int(ptCont.size());i++)
            glVertex3fv(ptCont[i]->getTransformation().X.ptr());
        if ( ((_attributes&sim_pathproperty_closed_path)!=0)&&(ptCont.size()>2) )
            glVertex3fv(ptCont[0]->getTransformation().X.ptr());
        glEnd();
        glLineWidth(1.0f);
        glDisable(GL_LINE_STIPPLE);
    }


    if (showSquare||(pathEditMode&&isPath))
    {
#ifdef SIM_WITH_GUI
        if (pathEditMode&&isPath)
        {
            std::vector<bool> selectedPts(ptCont.size(),false);
            int lastSel=-1;
            for (int i=0;i<App::mainWindow->editModeContainer->getEditModeBufferSize();i++)
            {
                int ind=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
                if ( (ind>=0)&&(ind<int(ptCont.size())) )
                    selectedPts[ind]=true;
            }
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)
                lastSel=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            for (int i=0;i<int(selectedPts.size());i++)
            { // display path points:
                if (!selectedPts[i])
                {
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,0.2f,0.2f,1.0f);
                    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlack);
                }
                else
                {
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorBlack);
                    if (i==lastSel)
                        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
                    else
                        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
                }
                glPushMatrix();
                C3Vector trx(ptCont[i]->getTransformation().X);
                glTranslatef(trx(0),trx(1),trx(2));
                C4Vector axis=ptCont[i]->getTransformation().Q.getAngleAndAxis();
                glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
                glLoadName(i);
                if (i==0)
                    ogl::drawSphere(squareSize,6,4,false);
                else
                    ogl::drawBox(squareSize,squareSize,squareSize,true,nullptr);
                glPopMatrix();
            }

            if (showOrientation)
            {
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                for (int i=0;i<int(selectedPts.size());i++)
                { // display path points:
                    glPushMatrix();
                    C3Vector trx(ptCont[i]->getTransformation().X);
                    glTranslatef(trx(0),trx(1),trx(2));
                    C4Vector axis=ptCont[i]->getTransformation().Q.getAngleAndAxis();
                    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
                    glLoadName(i);
                    ogl::drawReference(squareSize*2.0f,true,true,true,nullptr);
                    glPopMatrix();
                }
            }
        }
        else
#endif
        {
            std::vector<bool> selS(ptCont.size(),false);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                std::vector<int>* selP=App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode();
                for (int i=0;i<int(selP->size());i++)
                    selS[selP->at(i)]=true;
            }
#endif
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            for (int i=0;i<int(ptCont.size());i++)
            {
                if (!selS[i])
                {
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,0.2f,0.2f,1.0f);
                    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlack);
                }
                else
                { // point is selected (selected in the "non edit mode")
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorBlack);
                    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
                }
                if (pathIsOnlySelectedObject)
                    glLoadName(NON_OBJECT_PICKING_ID_PATH_PTS_START+i); // 2010/09/01 (individual path points)

                glPushMatrix();
                C3Vector trx(ptCont[i]->getTransformation().X);
                glTranslatef(trx(0),trx(1),trx(2));
                C4Vector axis=ptCont[i]->getTransformation().Q.getAngleAndAxis();
                glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
                if (i==0)
                    ogl::drawSphere(squareSize,6,4,false);
                else
                    ogl::drawBox(squareSize,squareSize,squareSize,true,nullptr);
                if (showOrientation)
                    ogl::drawReference(squareSize*2.0f,true,true,true,nullptr);
                glPopMatrix();
            }

            if (showOrientation)
            {
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                for (int i=0;i<int(ptCont.size());i++)
                {
                    if (pathIsOnlySelectedObject)
                        glLoadName(NON_OBJECT_PICKING_ID_PATH_PTS_START+i); // 2010/09/01 (individual path points)

                    glPushMatrix();
                    C3Vector trx(ptCont[i]->getTransformation().X);
                    glTranslatef(trx(0),trx(1),trx(2));
                    C4Vector axis=ptCont[i]->getTransformation().Q.getAngleAndAxis();
                    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
                    ogl::drawReference(squareSize*2.0f,true,true,true,nullptr);
                    glPopMatrix();
                }
            }

            glLoadName(-1); // 2010/09/01
        }
    }
    if (pathEditMode&&isPath)
        glLoadName(-1);
    if (showOrientation&&(!isPath))
    {
        for (int i=0;i<int(ptCont.size());i++)
        {
            glPushMatrix();
            C3Vector trx(ptCont[i]->getTransformation().X);
            glTranslatef(trx(0),trx(1),trx(2));
            C4Vector axis=ptCont[i]->getTransformation().Q.getAngleAndAxis();
            glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            ogl::drawReference(squareSize*0.5f,true,true,false,nullptr);
            glPopMatrix();
        }
    }
    if ( (!isPath)&&(ptCont.size()>1)&&(!pathEditMode)&&(_attributes&sim_pathproperty_show_position) )
    { // We have to display the start position:
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,1.0f,0.2f,0.2f);
        glPushMatrix();
        glTranslatef(_startPosition(0),_startPosition(1),_startPosition(2));
        ogl::drawSphere(squareSize,10,5,true);
        glPopMatrix();
    }
}
#endif
