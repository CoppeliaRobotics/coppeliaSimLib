#pragma once

#include "_dummy_.h"

class CDummy : public CSceneObject, public _CDummy_
{
public:
    CDummy();
    virtual ~CDummy();

private:
    static int cnt;
};
