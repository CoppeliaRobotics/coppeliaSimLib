
#pragma once

#include "iKChain.h"
#include "iKDummy.h"

class CIKChainCont  
{
public:
    CIKChainCont(std::vector<CIKDummy*>& tipContainer,float interpolationFactor,int jointNbToExclude);
    virtual ~CIKChainCont();
    std::vector<CIKChain*> allChains;
};
