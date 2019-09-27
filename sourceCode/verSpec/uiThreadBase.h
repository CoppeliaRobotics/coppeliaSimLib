
#pragma once

struct SUIThreadCommand;

class CUiThreadBase
{
public:
    CUiThreadBase(){}
    virtual ~CUiThreadBase(){}

    void handleVerSpecExecuteCommandViaUiThread1(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut){}
};
