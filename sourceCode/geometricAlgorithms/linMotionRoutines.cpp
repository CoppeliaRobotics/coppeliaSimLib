// Those routines should be replaced with the Reflexxes RML libraries!!!!

#include <linMotionRoutines.h>
#include <tt.h>
#include <simMath/mathDefines.h>
#include <algorithm>

const double LIN_MOTION_POSITION_TOLERANCE  =0.000001;
const double LIN_MOTION_TIME_TOLERANCE      =0.000001;
const double LIN_MOTION_VELOCITY_TOLERANCE  =0.000001;

CLinMotionRoutines::CLinMotionRoutines()
{
}

CLinMotionRoutines::~CLinMotionRoutines()
{
}

double CLinMotionRoutines::getTimeToVelocity(double speed,double accel,double nextSpeed)
{
    return((nextSpeed-speed)/accel);
}

double CLinMotionRoutines::getTimeToPosition_negOk(double pos,double speed,double accel,double nextPos)
{
    double delta=speed*speed-2.0*accel*(pos-nextPos);
    if (delta<0.0)
        return(-0.01*FLOAT_MAX);
    double t0=(-speed+sqrt(delta))/accel;
    double t1=(-speed-sqrt(delta))/accel;
    if ( (t0==0.0)||(t1==0.0) )
        return(0.0);
    if (t0<0.0)
    {
        if (t1>0.0)
            return(t1);
        return(std::max<double>(t0,t1));
    }
    if (t1<0.0)
    {
        if (t0>0.0)
            return(t0);
        return(std::max<double>(t0,t1));
    }
    return(std::min<double>(t0,t1));
}


double CLinMotionRoutines::getTimeToVelocity_lvp(double currentVelocity,double accel,double targetVelocity)
{
    // Return value can be negative if the target velocity cannot be reached
    if (accel==0.0)
    {
        if (currentVelocity==targetVelocity)
            return(0.0);
        return(-1.0); // cannot be reached in future nor past
    }
    return((targetVelocity-currentVelocity)/accel); // can be negative
}

double CLinMotionRoutines::getDistanceForTimeStep_lvp(double currentVelocity,double accel,double deltaTime)
{
    return(currentVelocity*deltaTime+0.5*accel*deltaTime*deltaTime);
}
double CLinMotionRoutines::getNextVelocityForTimeStep_lvp(double currentVelocity,double accel,double deltaTime)
{
    return(currentVelocity+accel*deltaTime);
}

double CLinMotionRoutines::getDistanceToDeceleration(double currentVelocity,double absAccel,double targetVelocity,double distanceToLimit)
{ // absAccel and currentVelocity have to be different from zero!
    // 1. Until when can we go on at this speed before we have to decelerate? (t1==same speed duration, t0==deceleration duration)
    // (d1=same speed distance, d0=deceleration distance)
    double signedAccel=absAccel;
    if (targetVelocity<currentVelocity)
        signedAccel*=-1.0;
    double t0=0.0;
    if (targetVelocity!=currentVelocity)
        t0=getTimeToVelocity_lvp(currentVelocity,signedAccel,targetVelocity);
    double d0=getDistanceForTimeStep_lvp(currentVelocity,signedAccel,t0);
    return(distanceToLimit-d0);
}

bool CLinMotionRoutines::getNextValues(double& position,double& velocity,
                                       double targetVelocity,double accelAbs,
                                       double lowerLimitPosition,double upperLimitPosition,
                                       double lowerMaxAbsVel,double upperMaxAbsVel,
                                       double& deltaTime)
{ // accelAbs cannot be zero! position and velocity will be updated to the next position and velocity values
    // deltaTime will contain the remaining of the time after a limit has been reached and invertTargetVelocityAtExtremities is false
    // Return value true means deltaTime is not zero (--> we need to call getNextValues again with different limits)
    // Handle trivial cases:
    if (deltaTime<LIN_MOTION_TIME_TOLERANCE)
    {
        deltaTime=0.0;
        return(false);
    }
    if ( (velocity==targetVelocity)&&(velocity==0.0) )
    {
        deltaTime=0.0;
        return(false);
    }
    if (position==lowerLimitPosition)
    {
        // Make sure the allowed velocity is respected:
        if (velocity!=0.0)
        {
            if (fabs(velocity)>lowerMaxAbsVel)
                velocity=lowerMaxAbsVel*(fabs(velocity)/velocity);
        }
        if (velocity<0.0)
            return(true);
        if (velocity==0.0)
        {
            if (targetVelocity<0.0)
                return(true);
        }
    }
    if (position==upperLimitPosition)
    {
        // Make sure the allowed velocity is respected:
        if (velocity!=0.0)
        {
            if (fabs(velocity)>upperMaxAbsVel)
                velocity=upperMaxAbsVel*(fabs(velocity)/velocity);
        }
        if (velocity>0.0)
            return(true);
        if (velocity==0.0)
        {
            if (targetVelocity>0.0)
                return(true);
        }
    }
    // We divide the task into two sub-tasks if targetVelocity*velocity is negative:
    if (targetVelocity*velocity<0.0)
    { // 1.0.0 We first try to go to zero. The only thing that limits us here is the deltaTime that might not be enough to reach zero or a limit.
        // signedAccel means speeding down!
        double nextLimitMaxSpeed=lowerMaxAbsVel;
        double nextLimitPos=lowerLimitPosition;
        double signedAccel=accelAbs;
        if (velocity>0.0)
        {
            nextLimitMaxSpeed=upperMaxAbsVel;
            nextLimitPos=upperLimitPosition;
            signedAccel*=-1.0;
        }
        // How long to reach zero vel.?
        double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,0.0); // t=(v1-v0)/a
        // Will we reach a limit before zero vel.? How long to next limit?
        double t2=getTimeToPosition_negOk(position,velocity,signedAccel,nextLimitPos);
        if ( (t2>=0.0)&&(t2<t) )
        { // 1.A.0 We will reach a limit before we reach zero velocity!
            t=t2;
            if (t>deltaTime)
                t=deltaTime; // We cannot or can just reach the limit in the given time step:
            double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
            velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,t); // v=v0+at
            position+=dist;
            deltaTime-=t;
            return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here
        }
        else
        { // 1.B.0 We will reach zero velocity before we reach a limit!
            if (t>=deltaTime)
            { // 1.B.A We cannot or can just reach zero velocity in the given time step:
                double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,deltaTime); // d=v0t+0.5*at^2
                velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,deltaTime); // v=v0+at
                position+=dist;
                deltaTime=0.0;
                return(false); // We end here
            }
            else
            { // 1.B.B We can reach zero velocity and even go further. We do this in 2 steps:
                // 1. We go to zero velocity:
                double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
                velocity=0.0;
                position+=dist;
                deltaTime-=t;
                // 2. We continue and accelerate from zero:
                return(getNextValues(position,velocity,targetVelocity,accelAbs,lowerLimitPosition,upperLimitPosition,lowerMaxAbsVel,upperMaxAbsVel,deltaTime));
            }
        }
    }
    else
    { // 2.0.0 Here velocity always stays with the same sign (not going from pos to neg or opposite):
        // We have 3 cases that have to be handled:
        // B. We want to keep the speed:
        if (fabs(velocity-targetVelocity)<LIN_MOTION_POSITION_TOLERANCE)
        { // 2.B.0  signedAccel means speeding down!
            double nextLimitMaxSpeed=lowerMaxAbsVel;
            double nextLimitPos=lowerLimitPosition;
            double signedAccel=accelAbs;
            if (velocity>0.0)
            {
                nextLimitMaxSpeed=upperMaxAbsVel;
                nextLimitPos=upperLimitPosition;
                signedAccel*=-1.0;
            }
            // 1. Can we keep the speed until we reach next limit?
            if (fabs(velocity)<=nextLimitMaxSpeed)
            { // 2.B.A Yes we can:
                // a. Will we reach the next limit within timeStep?
                double t=fabs((nextLimitPos-position)/velocity);// constant speed! no divide by zero problem here (velocity cannot be zero!)
                if (t>=deltaTime)
                    t=deltaTime; // we will not reach a limit during deltaTime!
                double dist=velocity*t; // We are at constant velocity here
                position+=dist;
                deltaTime-=t;
                return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here
            }
            else
            { // 2.B.B No, we cannot keep the speed until we reach next limit:
                // What is the signed distance from the next limit we can keep this velocity?
                double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                double absD=getDistanceForTimeStep_lvp(fabs(velocity),-accelAbs,t); // d=v0t+0.5*at^2
                if (absD>=fabs(position-nextLimitPos))
                { // 2.B.B.1 We cannot keep the speed at all! We have to decelerate:
                    if (t>deltaTime)
                        t=deltaTime; // we will not reach a limit during deltaTime!
                    double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
                    velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,t); // v=v0+at
                    position+=dist;
                    deltaTime-=t;
                    return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here (we either reached a limit or we finished deltaTime)
                }
                else
                { // 1.B.B.2 We can keep the speed for a while, but then we have to decelerate:
                    double nextVirtLimPos=nextLimitPos-absD*(velocity/fabs(velocity)); // velocity is not zero!
                    t=fabs(position-nextVirtLimPos)/fabs(velocity);
                    // Can we perform this within the deltaTime?
                    if (t>deltaTime)
                        t=deltaTime; // no, we can't
                    double dist=velocity*t; // constant speed here
                    position+=dist;
                    deltaTime-=t;
                    if (deltaTime<LIN_MOTION_TIME_TOLERANCE)
                        return(false); // We leave, we finished deltaTime
                    // Now we have to decelerate:
                    double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                    if (t>deltaTime)
                        t=deltaTime; // we will not reach a limit during deltaTime!
                    dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
                    velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,t); // v=v0+at
                    position+=dist;
                    deltaTime-=t;
                    return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here (we either reached a limit, or we finished deltaTime)
                }
            }
        }
        // A. We want to slow down:
        if (fabs(velocity)>fabs(targetVelocity))
        { // 2.A.0   signedAccel means speeding down!
            // Will we reach targetVelocity before we reach a limit?
            double nextLimitMaxSpeed=lowerMaxAbsVel;
            double nextLimitPos=lowerLimitPosition;
            double signedAccel=accelAbs;
            if (velocity+targetVelocity>0.0)
            {
                nextLimitMaxSpeed=upperMaxAbsVel;
                nextLimitPos=upperLimitPosition;
                signedAccel*=-1.0;
            }
            // What is the time to target velocity?
            double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,fabs(targetVelocity)); // t=(v1-v0)/a
            // What is the time to the next limit?
            double t2=getTimeToPosition_negOk(position,velocity,signedAccel,nextLimitPos);
            if ( (t2>=0.0)&&(t2<t) )
            { // 2.A.A We will reach a limit before we reach target velocity!
                t=t2;
                if (t>deltaTime)
                { // We cannot reach the limit in the given time step:
                    t=deltaTime; 
                    double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
                    velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,t); // v=v0+at
                    position+=dist;
                }
                else
                { // We reach the limit:
                    velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,t); // v=v0+at
                    position=nextLimitPos; // important because of some rounding errors!
                }
                deltaTime-=t;
                return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here
            }
            else
            { // 2.A.B We will reach target velocity before we reach a limit!
                if (t>=deltaTime)
                { // 2.A.B.0 We cannot or can just reach target velocity in the given time step:
                    double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,deltaTime); // d=v0t+0.5*at^2
                    velocity=getNextVelocityForTimeStep_lvp(velocity,signedAccel,deltaTime); // v=v0+at
                    position+=dist;
                    deltaTime=0.0;
                    return(false); // We end here
                }
                else
                { // 2.A.B.1 We can reach target velocity and even go further. We do this in 2 steps:
                    // 1. We go to target velocity:
                    double dist=getDistanceForTimeStep_lvp(velocity,signedAccel,t); // d=v0t+0.5*at^2
                    velocity=targetVelocity;
                    position+=dist;
                    deltaTime-=t;
                    // 2. We continue: (and keep at constant target velocity for a while)
                    return(getNextValues(position,velocity,targetVelocity,accelAbs,lowerLimitPosition,upperLimitPosition,lowerMaxAbsVel,upperMaxAbsVel,deltaTime));
                }
            }
        }
        // C. We want to speed up:
        if (fabs(velocity)<fabs(targetVelocity))
        { // 2.C.0   signedAccel means speeding down!
            double nextLimitMaxSpeed=lowerMaxAbsVel;
            double nextLimitPos=lowerLimitPosition;
            double signedAccel=accelAbs;
            if (targetVelocity>0.0)
            {
                nextLimitMaxSpeed=upperMaxAbsVel;
                nextLimitPos=upperLimitPosition;
                signedAccel*=-1.0;
            }
            // Can we speed-up?
            if (fabs(velocity)>=nextLimitMaxSpeed)
            {
                double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                double absD=getDistanceForTimeStep_lvp(fabs(velocity),-accelAbs,t); // d=v0t+0.5*at^2
                if (absD>=fabs(position-nextLimitPos))
                { // 2.C.A We cannot speed-up! we have to decelerate
                    // We decelerate to nextLimitMaxSpeed:
                    return(getNextValues(position,velocity,nextLimitMaxSpeed*(targetVelocity/fabs(targetVelocity)),accelAbs,lowerLimitPosition,upperLimitPosition,lowerMaxAbsVel,upperMaxAbsVel,deltaTime));
                }
            }
            // We reached this point which means that we can speed-up!
            double maxSpeed=velocity;
            if (fabs(velocity)>=nextLimitMaxSpeed)
            { // 2.C.B.1
                double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                double absD=getDistanceForTimeStep_lvp(fabs(velocity),-accelAbs,t); // d=v0t+0.5*at^2
                double halfDist=(fabs(nextLimitPos-position)-absD)*0.5;
                if (halfDist>0.0)
                { 
                    double t2=getTimeToPosition_negOk(0.0,fabs(velocity),accelAbs,halfDist);
                    maxSpeed=getNextVelocityForTimeStep_lvp(velocity,-signedAccel,t2);
                }
            }
            else
            { // 2.C.B.2
                double t=getTimeToVelocity_lvp(fabs(velocity),accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                // Compute the time to reach the next limit position:
                double t2=getTimeToPosition_negOk(position,velocity,-signedAccel,nextLimitPos);
                if (t2<=t)
                {
                    t=t2; // 2.C.B.3 We will reach a limit!
                    maxSpeed=getNextVelocityForTimeStep_lvp(velocity,-signedAccel,t);
                }
                else
                {
                    double absD=getDistanceForTimeStep_lvp(fabs(velocity),accelAbs,t); // d=v0t+0.5*at^2
                    double halfDist=(fabs(nextLimitPos-position)-absD)*0.5;
                    if (halfDist>0.0)
                    { 
                        t2=getTimeToPosition_negOk(0.0,nextLimitMaxSpeed,accelAbs,halfDist);
                        maxSpeed=getNextVelocityForTimeStep_lvp(nextLimitMaxSpeed,accelAbs,t2)*(targetVelocity/fabs(targetVelocity));
                    }
                }
            }
            if (fabs(maxSpeed)>fabs(targetVelocity))
                maxSpeed=targetVelocity;
            // Compute the time to reach max allowed speed (limited by targetVelocity or by one limit's max vel.):
            double t=getTimeToVelocity_lvp(fabs(velocity),accelAbs,fabs(maxSpeed));
            if (t>deltaTime)
                t=deltaTime; // We will finish deltaTime!
            double dist=getDistanceForTimeStep_lvp(velocity,-signedAccel,t); // d=v0t+0.5*at^2
            velocity=getNextVelocityForTimeStep_lvp(velocity,-signedAccel,t); // v=v0+at
            position+=dist;
            deltaTime-=t;
            if (deltaTime<LIN_MOTION_TIME_TOLERANCE)
                return(false); // We end here (we either reached a limit, or we finished deltaTime)
            // Here we reached a point where we have to keep the speed for a while, or where we need to decelerate:

            // Do we have to slow-down?
            if (fabs(velocity)>=nextLimitMaxSpeed)
            {
                double t=getTimeToVelocity_lvp(fabs(velocity),-accelAbs,nextLimitMaxSpeed); // t=(v1-v0)/a
                double absD=getDistanceForTimeStep_lvp(fabs(velocity),-accelAbs,t); // d=v0t+0.5*at^2
                if (absD>=fabs(position-nextLimitPos))
                { // 2.C.B.4 We cannot speed-up! we have to decelerate
                    // We decelerate to nextLimitMaxSpeed:
                    return(getNextValues(position,velocity,nextLimitMaxSpeed*(targetVelocity/fabs(targetVelocity)),accelAbs,lowerLimitPosition,upperLimitPosition,lowerMaxAbsVel,upperMaxAbsVel,deltaTime));
                }
            }
            // 2.C.B.5 No, we need to keep the same velocity!
            return(getNextValues(position,velocity,velocity,accelAbs,lowerLimitPosition,upperLimitPosition,lowerMaxAbsVel,upperMaxAbsVel,deltaTime));
        }
        return(true); // should never happen!
    }
}

double CLinMotionRoutines::getMaxVelocityAtPosition(double position,double accelAbs,
                                                   double lowerLimitPosition,double upperLimitPosition,double lowerMaxAbsVel,double upperMaxAbsVel)
{ // Returns an absolute velocity
    double d0=position-lowerLimitPosition;
    double d1=upperLimitPosition-position;
    double dt0=getTimeToPosition_negOk(0.0,lowerMaxAbsVel,accelAbs,d0);
    double dt1=getTimeToPosition_negOk(0.0,upperMaxAbsVel,accelAbs,d1);
    double v0=lowerMaxAbsVel;
    if (dt0>0.0)
        v0=getNextVelocityForTimeStep_lvp(lowerMaxAbsVel,accelAbs,dt0);
    double v1=upperMaxAbsVel;
    if (dt1>0.0)
        v1=getNextVelocityForTimeStep_lvp(upperMaxAbsVel,accelAbs,dt1);
    return(std::min<double>(v0,v1));
}

double CLinMotionRoutines::getCurrentVelocityOnPath(double position,double nominalVelocity,double lowerLimitPosition,double upperLimitPosition,
                                      double lowerMaxRelVel,double upperMaxRelVel)
{
    // Handle trivial cases:
    if (nominalVelocity==0.0)
        return(0.0); // Nominal velocity is zero --> velocity is zero
    if (lowerMaxRelVel+upperMaxRelVel==0.0)
        return(0.0); // relative velocities are both zero --> velocity is zero
    if (lowerMaxRelVel==upperMaxRelVel)
        return(lowerMaxRelVel*nominalVelocity); // relative velocities are same --> velocity is rel*nominal
    // Here we have the cases where we accelerate or decelerate:
    double d=upperLimitPosition-lowerLimitPosition;
    if (d==0.0)
    { // Two coincident points->jump in velocity
        if (nominalVelocity>0.0)
            return(upperMaxRelVel*nominalVelocity);
        else
            return(lowerMaxRelVel*nominalVelocity);
    }
    double t=2.0*d/((lowerMaxRelVel+upperMaxRelVel)*fabs(nominalVelocity));
    double a=(upperMaxRelVel-lowerMaxRelVel)*fabs(nominalVelocity)/t;

    double t0=getTimeToPosition_negOk(0.0,lowerMaxRelVel*fabs(nominalVelocity),a,position-lowerLimitPosition);
    double t1=getTimeToPosition_negOk(0.0,upperMaxRelVel*fabs(nominalVelocity),-a,upperLimitPosition-position);
    if ( (t0>=0.0)&&(t1>=0.0) )
    {
        if (t0<t1)
            t1=-1.0;
        else
            t0=-1.0;
    }
    double absVel;
    if (t0>=0.0)
        absVel=getNextVelocityForTimeStep_lvp(lowerMaxRelVel*fabs(nominalVelocity),a,t0);
    else
        absVel=getNextVelocityForTimeStep_lvp(upperMaxRelVel*fabs(nominalVelocity),-a,t1);
    if (nominalVelocity>0.0)
        return(absVel);
    return(-absVel);
}


bool CLinMotionRoutines::getNextValuesForPath(double& position,
                               double nominalVelocity,
                               double lowerLimitPosition,double upperLimitPosition,
                               double lowerMaxRelVel,double upperMaxRelVel,
                               double& deltaTime)
{   // position and velocity will be updated to the next position and velocity values. accelAbs cannot be zero.
    // deltaTime will contain the remaining of the time after a limit has been reached
    // Return value true means deltaTime is not zero (--> we need to call getNextValues again with different limits)
    
    // 1. What is current velocity?
    double velocity=getCurrentVelocityOnPath(position,nominalVelocity,lowerLimitPosition,upperLimitPosition,lowerMaxRelVel,upperMaxRelVel);

    // 2. Handle trivial cases:
    if (deltaTime<LIN_MOTION_TIME_TOLERANCE)
    {
        deltaTime=0.0;
        return(false);
    }
    if (position-lowerLimitPosition<LIN_MOTION_POSITION_TOLERANCE)
    {
        if (nominalVelocity<0.0)
        {
            position=lowerLimitPosition;
            velocity=lowerMaxRelVel*nominalVelocity;
            return(deltaTime>LIN_MOTION_TIME_TOLERANCE);
        }
    }
    if (upperLimitPosition-position<LIN_MOTION_POSITION_TOLERANCE)
    {
        if (nominalVelocity>0.0)
        {
            position=upperLimitPosition;
            velocity=upperMaxRelVel*nominalVelocity;
            return(deltaTime>LIN_MOTION_TIME_TOLERANCE);
        }
    }
    if (fabs(nominalVelocity)<LIN_MOTION_VELOCITY_TOLERANCE)
    {
        deltaTime=0.0;
        velocity=0.0;
        return(false);
    }
    if ( (lowerMaxRelVel*fabs(nominalVelocity)<LIN_MOTION_VELOCITY_TOLERANCE)&&(upperMaxRelVel*fabs(nominalVelocity)<LIN_MOTION_VELOCITY_TOLERANCE) )
    {
        deltaTime=0.0;
        velocity=0.0;
        return(false);
    }
    // 3. We search the next value (regular cases):
    double v2,d;
    double nextLimitPos,oppositeLimitPos;
    if (nominalVelocity>=0.0)
    {
        nextLimitPos=upperLimitPosition;
        oppositeLimitPos=lowerLimitPosition;
        v2=upperMaxRelVel*nominalVelocity;
        d=upperLimitPosition-position;
    }
    else
    {
        nextLimitPos=lowerLimitPosition;
        oppositeLimitPos=upperLimitPosition;
        v2=lowerMaxRelVel*nominalVelocity;
        d=lowerLimitPosition-position;
    }
    if ( (fabs(velocity)<LIN_MOTION_VELOCITY_TOLERANCE)&&(fabs(v2)<LIN_MOTION_VELOCITY_TOLERANCE) )
    {
        deltaTime=0.0;
        return(false);
    }
    double t=2.0*d/(velocity+v2);
    double a=(v2-velocity)/t;
    if (t>deltaTime)
        t=deltaTime;
    double dist=getDistanceForTimeStep_lvp(velocity,a,t); // d=v0t+0.5*at^2
    velocity=getNextVelocityForTimeStep_lvp(velocity,a,t); // v=v0+at
    double oldPos=position;
    position+=dist;
    if (oldPos==position)
    { // We reached the double/double resolution
        if (dist>0.0)
            position*=(1.0+DBL_EPSILON);
        if (dist<0.0)
            position*=(1.0-DBL_EPSILON);
    }
    deltaTime-=t;
    return(deltaTime>LIN_MOTION_TIME_TOLERANCE); // We end here
}

void CLinMotionRoutines::adjustNominalVelocity(double& nominalVelocity,double targetNominalVelocity,double acceleration,double deltaTime)
{
    if (deltaTime<LIN_MOTION_TIME_TOLERANCE*0.1)
        return;
    if (acceleration==0.0)
        return;
    if (fabs(nominalVelocity-targetNominalVelocity)<LIN_MOTION_VELOCITY_TOLERANCE)
    {
        nominalVelocity=targetNominalVelocity;
        return;
    }
    if (nominalVelocity*targetNominalVelocity<0.0)
    { // We are changing direction! We perform this in 2 steps:
        // 1. Slow down:
        double t=getTimeToVelocity_lvp(fabs(nominalVelocity),-acceleration,0.0);
        if (t<=deltaTime)
        { // We go to zero and then do the rest:
            nominalVelocity=0.0;
            deltaTime-=t;
            adjustNominalVelocity(nominalVelocity,targetNominalVelocity,acceleration,deltaTime); // Here we continue
        }
        else
        { // We don't even reach zero:
            if (nominalVelocity>=0.0)
                nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,-acceleration,deltaTime);
            else
                nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,+acceleration,deltaTime);
        }
    }
    else
    { // Regular cases (acceleration or deceleration (constant vel case was already handled previously)):
        if (fabs(nominalVelocity)<fabs(targetNominalVelocity))
        { // We have to accelerate
            double t=getTimeToVelocity_lvp(fabs(nominalVelocity),+acceleration,fabs(targetNominalVelocity));
            if (t<=deltaTime)
            { // We accelerate and then reach the target velocity:
                nominalVelocity=targetNominalVelocity;
                deltaTime-=t;
                adjustNominalVelocity(nominalVelocity,targetNominalVelocity,acceleration,deltaTime); // Here we continue at constant velocity
            }
            else
            { // We don't reach the target velocity:
                double v=nominalVelocity;
                if (v==0.0)
                    v=targetNominalVelocity;
                if (v>=0.0)
                    nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,+acceleration,deltaTime);
                else
                    nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,-acceleration,deltaTime);
            }
        }
        else
        { // We have to decelerate
            double t=getTimeToVelocity_lvp(fabs(nominalVelocity),-acceleration,fabs(targetNominalVelocity));
            if (t<=deltaTime)
            { // We decelerate and then reach the target velocity:
                nominalVelocity=targetNominalVelocity;
                deltaTime-=t;
                adjustNominalVelocity(nominalVelocity,targetNominalVelocity,acceleration,deltaTime); // Here we continue at constant velocity
            }
            else
            { // We don't reach the target velocity:
                if (nominalVelocity>=0.0)
                    nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,-acceleration,deltaTime);
                else
                    nominalVelocity=getNextVelocityForTimeStep_lvp(nominalVelocity,+acceleration,deltaTime);
            }
        }
    }
}


bool CLinMotionRoutines::getNextValuesForPath(double& position,
                                 double& nominalVelocity,double targetNominalVelocity,double acceleration,
                                 double lowerLimitPosition,double upperLimitPosition,
                                 double lowerMaxRelVel,double upperMaxRelVel,
                                 double& deltaTime)
{
    double originalDeltaTime=deltaTime;
    bool retVal=getNextValuesForPath(position,nominalVelocity,lowerLimitPosition,upperLimitPosition,lowerMaxRelVel,upperMaxRelVel,deltaTime);
    adjustNominalVelocity(nominalVelocity,targetNominalVelocity,acceleration,originalDeltaTime-deltaTime);
    return(retVal);
}
