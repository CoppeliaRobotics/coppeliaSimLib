
#include "vrepMainHeader.h"
#include "iKChainCont.h"

CIKChainCont::CIKChainCont(std::vector<CIKDummy*>& tipContainer,float interpolationFactor,int jointNbToExclude)
{
    // Here we prepare all element equations:
    for (int i=0;i<int(tipContainer.size());i++)
    {
        CIKChain* aChain=new CIKChain(tipContainer[i],interpolationFactor,jointNbToExclude);
        if (aChain->chainIsValid)
            allChains.push_back(aChain);
        else
            delete aChain;
    }
}

CIKChainCont::~CIKChainCont()
{
    for (int i=0;i<int(allChains.size());i++)
        delete allChains[i];
}
