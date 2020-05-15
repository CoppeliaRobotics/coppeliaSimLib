#include "collectionElement.h"
#include "app.h"

int CCollectionElement::cnt=0;

CCollectionElement::CCollectionElement(int mainObjectHandle,int collectionType,bool add)
{
    _mainObjectHandle=mainObjectHandle;
    _elementType=collectionType;
    _additive=add;
    cnt++;
    std::string tmp("collection element created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CCollectionElement::~CCollectionElement()
{
    cnt--;
    std::string tmp("collection element destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}
