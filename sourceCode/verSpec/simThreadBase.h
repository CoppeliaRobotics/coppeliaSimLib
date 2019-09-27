
#pragma once

struct SSimulationThreadCommand;

class CSimThreadBase
{
public:
    CSimThreadBase(){}
    virtual ~CSimThreadBase(){}

    void handleVerSpecExecuteSimulationThreadCommand1(SSimulationThreadCommand* cmd){}
    bool handleVerSpecExecuteSimulationThreadCommand2(){return(true);}
    bool handleVerSpecHandleAutoSaveSceneCommand1(){return(true);}
    bool handleVerSpecHandleAutoSaveSceneCommand2(){return(true);}
};
