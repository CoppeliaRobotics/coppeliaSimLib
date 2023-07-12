
// Those routines should be replaced with the Reflexxes RML libraries!!!!

#pragma once

class CLinMotionRoutines
{
public:
    CLinMotionRoutines();
    virtual ~CLinMotionRoutines();

    static double getTimeToVelocity(double speed,double accel,double nextSpeed);

    static double getTimeToVelocity_lvp(double currentVelocity,double accel,double targetVelocity);
    static double getDistanceToDeceleration(double currentVelocity,double absAccel,double targetVelocity,double distanceToLimit);

    static bool getNextValues(double& position,double& velocity,
        double targetVelocity,double accelAbs,
        double lowerLimitPosition,double upperLimitPosition,
        double lowerMaxAbsVel,double upperMaxAbsVel,
        double& deltaTime);
    static double getMaxVelocityAtPosition(double position,double accelAbs,double lowerLimitPosition,double upperLimitPosition,double lowerMaxAbsVel,double upperMaxAbsVel);

    static double getDistanceForTimeStep_lvp(double currentVelocity,double accel,double deltaTime);
    static double getTimeToPosition_negOk(double pos,double speed,double accel,double nextPos);
    static double getNextVelocityForTimeStep_lvp(double currentVelocity,double accel,double deltaTime);

    static bool getNextValuesForPath(double& position,
        double& nominalVelocity,double targetNominalVelocity,double acceleration,
        double lowerLimitPosition,double upperLimitPosition,
        double lowerMaxRelVel,double upperMaxRelVel,
        double& deltaTime);

    static bool getNextValuesForPath(double& position,
        double nominalVelocity,
        double lowerLimitPosition,double upperLimitPosition,
        double lowerMaxRelVel,double upperMaxRelVel,
        double& deltaTime);
    static void adjustNominalVelocity(double& nominalVelocity,double targetNominalVelocity,double acceleration,double deltaTime);


    static double getCurrentVelocityOnPath(double position,double nominalVelocity,double lowerLimitPosition,double upperLimitPosition,
        double lowerMaxRelVel,double upperMaxRelVel);

};
