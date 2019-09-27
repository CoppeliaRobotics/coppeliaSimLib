
#pragma once

class COutsideCommandQueue  
{ // Only used for LUA scripts
public:
    COutsideCommandQueue();
    virtual ~COutsideCommandQueue();

    bool addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);
    void newSceneProcedure();
};
