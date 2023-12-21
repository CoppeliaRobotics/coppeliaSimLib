#pragma once

#include <QEventLoop>
#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <vMutex.h>
#include <vThread.h>

struct SSimulationThreadCommand
{
    int cmdId;
    int sceneUniqueId;
    double execTime;
    std::vector<bool> boolParams;
    std::vector<int> intParams;
    std::vector<quint64> uint64Params;
    std::vector<unsigned char> uint8Params;
    std::vector<double> doubleParams;
    std::vector<void *> objectParams;
    std::vector<std::string> stringParams;
    std::vector<C3Vector> posParams;
    std::vector<C4Vector> quatParams;
    std::vector<C7Vector> transfParams;
    std::vector<std::vector<int>> intVectorParams;
    std::vector<std::vector<float>> floatVectorParams;
    std::vector<std::vector<double>> doubleVectorParams;
};

class CSimThread : public QObject
{
    Q_OBJECT

  public:
    CSimThread();
    virtual ~CSimThread();
    void handleExtCalls();
    void executeMessages();
    void appendSimulationThreadCommand(SSimulationThreadCommand cmd, double executionDelay = 0.0);

  private:
    QEventLoop _eventLoop;
    void _handleSimulationThreadCommands();
    void _executeSimulationThreadCommand(SSimulationThreadCommand cmd);

    std::vector<SSimulationThreadCommand> _simulationThreadCommands;
    void _handleAutoSaveSceneCommand(SSimulationThreadCommand cmd);
#ifdef SIM_WITH_GUI
    void _handleClickRayIntersection_old(SSimulationThreadCommand cmd);
    bool _renderRequired();

    VMutex _mutex;
#endif
};
