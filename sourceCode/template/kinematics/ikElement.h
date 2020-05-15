#pragma once

#include "_ikElement_.h"

class CIkElement : public _CIkElement_
{
public:
    CIkElement(int tooltip);
    virtual ~CIkElement();

private:
    static int cnt;
};
