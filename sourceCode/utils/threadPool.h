
#pragma once

#include "vrepMainHeader.h"
#include "vMutex.h"
#include "vThread.h"

class CVThreadData
{
public:
    CVThreadData(void* _vthread,VTHREAD_ID_TYPE _vthreadID)
    {
        thread=_vthread;
        threadID=_vthreadID;
        threadDesiredTiming=2;  // this is the default thread switch timing (1-2 ms) (Put back to 2 ms on 2010/12/13)
        threadExecutionTime=-1; // -1 --> not yet executed
        usedMovementTime=0.0f;
        threadWantsResumeFromYield=false;
        threadShouldRunFreely=false;
        threadSwitchShouldTriggerNoOtherThread=false;
        threadResumeLocationAndOrder=1; // see below (but 1 is default)
        threadShouldNotSwitch=false; // can be used to avoid switching between two consecutive commands that need sequential execution
        allowToExecuteAgainInThisSimulationStep=false; // when scripts are forward-relocated for resume, we want to execute them a second time in the same simulation step
    }
    virtual ~CVThreadData()
    {
    }

    volatile void* thread;
    volatile VTHREAD_ID_TYPE threadID;
    volatile int threadDesiredTiming;
    volatile int threadExecutionTime;
    volatile float usedMovementTime;
    volatile bool threadWantsResumeFromYield;
    volatile bool threadShouldRunFreely;
    volatile bool threadSwitchShouldTriggerNoOtherThread;
    volatile int freeModeSavedThreadStartTime;
    volatile unsigned char threadResumeLocationAndOrder;
    volatile bool threadShouldNotSwitch;
    volatile bool allowToExecuteAgainInThisSimulationStep;
};

// FULLY STATIC CLASS
class CThreadPool  
{
public:
    static void init();
    static VTHREAD_ID_TYPE createNewThread(VTHREAD_START_ADDRESS threadStartAddress);
    static void switchToThread(VTHREAD_ID_TYPE threadID);
    static bool setThreadSwitchTiming(int timeInMs);
    static bool getThreadSwitchTiming(int& timeInMs);

    static int callRoutineViaSpecificThread(VTHREAD_ID_TYPE theThread,void* data);
    static bool setThreadAutomaticSwitch(bool switchIsAutomatic);
    static bool getThreadAutomaticSwitch();

    static bool setThreadResumeLocation(int location,int order);
    static bool switchBackToPreviousThread();
    static void switchBackToPreviousThreadIfNeeded();
    static bool isSwitchBackToPreviousThreadNeeded();

    static void prepareAllThreadsForResume_calledBeforeMainScript();
    static int handleAllThreads_withResumeLocation(int location);
    static int handleThread_ifHasResumeLocation(VTHREAD_ID_TYPE theThread,bool allThreadsWithResumeLocation,int location);

    static CVThreadData* getCurrentThreadData();
    static CVThreadData* getThreadData(VTHREAD_ID_TYPE threadId);
    static int getThreadPoolThreadCount();

    // Regular stop request (e.g. when the stop button is pressed, or when simStopSimulation is called.
    // Stop happens in a delayed fashion:
    static void setRequestSimulationStop(bool stop);
    static bool getSimulationStopRequested(); // returns true immediately after the request entered
    static bool getSimulationStopRequestedAndActivated(); // return true only after a delay after the request entered

    // Emergency stop request
    // (when a simulation script hangs and the Lua hook detected that, and the used pressend the emergency stop button):
    static void setSimulationEmergencyStop(bool stop);
    static bool getSimulationEmergencyStop();



    static void forceAutomaticThreadSwitch_simulationEnding();

    static int getProcessorCoreAffinity();
    static void setProcessorCoreAffinity(int affinity);

    static void cleanUp();

    static bool setThreadFreeMode(bool freeMode);
    static bool isThreadInFreeMode();

    static bool getShowThreadSwitches();
    static void setShowThreadSwitches(bool show);

private:
    static void _lock(unsigned char debugInfo);
    static void _unlock(unsigned char debugInfo);

    static void* _tmpData;
    static int _tmpRetData;
    static int _inInterceptRoutine;
    static VTHREAD_RETURN_TYPE _tmpCallback(VTHREAD_ARGUMENT_TYPE lpData);
    static bool _interceptThread(VTHREAD_ID_TYPE theThreadToIntercept,VTHREAD_START_ADDRESS theCallback);


    static VTHREAD_RETURN_TYPE _intermediateThreadStartPoint(VTHREAD_ARGUMENT_TYPE lpData);
    static void _cleanupTerminatedThreads();
    static void _terminateThread();
    static VTHREAD_START_ADDRESS _threadStartAdd;

    static std::vector<CVThreadData*> _allThreadData;

    static std::vector<VTHREAD_ID_TYPE> _threadQueue;
    static std::vector<int> _threadStartTime;

    static bool _simulationStopRequest;
    static int _simulationStopRequestTime;
    static bool _simulationEmergencyStopRequest;

    static VMutex _threadPoolMutex;

    static int _processorCoreAffinity; // -1: os default, 0: all on same core, but any core, >0: affinity mask
    static int _lockStage;
    static bool _threadShouldNotSwitch_override;

    static bool _showThreadSwitches;

    static VTHREAD_ID_TYPE _threadToIntercept;
    static VTHREAD_START_ADDRESS _threadInterceptCallback;
    static int _threadInterceptIndex;

};
