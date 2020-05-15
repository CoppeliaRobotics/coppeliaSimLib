
#include "simInternal.h"
#include "memorizedConfContainer.h"
#include "app.h"
#include "tt.h"

CMemorizedConfContainer::CMemorizedConfContainer()
{
    removeMemorized();  
}

CMemorizedConfContainer::~CMemorizedConfContainer()
{ // beware, the current world could be nullptr
    removeMemorized();
}

void CMemorizedConfContainer::memorize()
{
    removeMemorized();
    for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
        if (it!=nullptr)
            configurations.push_back(new CMemorizedConf(it));
    }
}

void CMemorizedConfContainer::restoreMemorized()
{
    if (isBufferEmpty()) // this also destroys unexisting configurations
        return;
    for (int i=0;i<int(configurations.size());i++)
    {
        CMemorizedConf* aConf=configurations[i];
        aConf->restore();
    }
}

bool CMemorizedConfContainer::isBufferEmpty()
{ // this also destroys all objects that don't exist anymore:
    for (int i=0;i<int(configurations.size());i++)
    {
        CMemorizedConf* aConf=configurations[i];
        if (!aConf->doesStillExist())
        {
            delete aConf;
            configurations.erase(configurations.begin()+i);
            i--; // one element was removed, we have to adjust the loop!
        }
    }
    return(configurations.size()==0);
}

void CMemorizedConfContainer::removeMemorized()
{
    for (size_t i=0;i<configurations.size();i++)
        delete configurations[i];
    configurations.clear();
}

char* CMemorizedConfContainer::getObjectConfiguration(int objID)
{
    std::vector<char> data;
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objID);
    if (it==nullptr)
        return(nullptr);
    CMemorizedConf temp(it);
    temp.serializeToMemory(data);
    char* retBuffer=new char[data.size()+4];
    ((int*)retBuffer)[0]=int(data.size());
    for (int i=0;i<int(data.size());i++)
        retBuffer[4+i]=data[i];
    return(retBuffer);
}

void CMemorizedConfContainer::setObjectConfiguration(const char* data)
{
    if (data==nullptr)
        return;
    std::vector<char> arr;
    int l=((int*)data)[0];
    for (int i=0;i<l;i++)
        arr.push_back(data[i]);
    CMemorizedConf temp;
    temp.serializeFromMemory(arr);
    temp.restore();
}

char* CMemorizedConfContainer::getConfigurationTree(int objID)
{
    std::vector<char> data;
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objID);
    std::vector<CSceneObject*> sel;
    if (it==nullptr)
    { // We memorize everything:
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectFromIndex(i));
    }
    else
    { // We memorize just the object and all its children:
        it->getAllObjectsRecursive(&sel,true,true);
    }
    for (int i=0;i<int(sel.size());i++)
    {
        CMemorizedConf temp(sel[i]);
        temp.serializeToMemory(data);
    }
    char* retBuffer=new char[data.size()+4];
    ((int*)retBuffer)[0]=int(data.size());
    for (int i=0;i<int(data.size());i++)
        retBuffer[4+i]=data[i];
    return(retBuffer);
}

void CMemorizedConfContainer::setConfigurationTree(const char* data)
{
    if (data==nullptr)
        return;
    int l=((int*)data)[0];
    std::vector<char> arr;
    for (int i=0;i<l;i++)
        arr.push_back(data[i+4]);
    std::vector<CMemorizedConf*> allConfs;
    std::vector<int> parentCount;
    std::vector<int> index;
    while (arr.size()!=0)
    {
        CMemorizedConf* temp=new CMemorizedConf();
        temp->serializeFromMemory(arr);
        parentCount.push_back(temp->getParentCount());
        index.push_back((int)index.size());
        allConfs.push_back(temp);
    }
    tt::orderAscending(parentCount,index);
    for (int i=0;i<int(index.size());i++)
    {
        allConfs[index[i]]->restore();
        delete allConfs[index[i]];
    }
}
