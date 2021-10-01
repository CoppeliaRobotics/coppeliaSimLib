#include "moduleMenuItem.h"

CModuleMenuItem::CModuleMenuItem(const char* path,int scriptHandle)
{
    std::string s(path);
    _label=path;
    size_t pos=0;
    while ((pos=s.find("\n"))!=std::string::npos)
    {
        _path+=s.substr(0,pos+1);
        s.erase(0,pos+1);
    }
    if (_path.size()>0)
        _label=s;
    _state=1;
    _scriptHandle=scriptHandle;
}

CModuleMenuItem::~CModuleMenuItem()
{
}

void CModuleMenuItem::setHandle(int h)
{
    _handle=h;
}

int CModuleMenuItem::getHandle() const
{
    return(_handle);
}

int CModuleMenuItem::getScriptHandle() const
{
    return(_scriptHandle);
}

void CModuleMenuItem::setState(int s)
{
    _state=s;
}

int CModuleMenuItem::getState() const
{
    return(_state);
}

std::string CModuleMenuItem::getLabel() const
{
    return(_label);
}

void CModuleMenuItem::setLabel(const char* label)
{
    _label=label;
}

std::string CModuleMenuItem::getPath() const
{
    return(_path);
}

#ifdef SIM_WITH_GUI
void CModuleMenuItem::addMenu(std::vector<VMenu*>& menus,std::vector<std::string>& labels)
{
    size_t pos=0;
    std::string token;
    std::string s(_path);
    if (s.size()>0)
        s+=_label;
    size_t p=0;
    while ((pos=s.find("\n"))!=std::string::npos)
    {
        token=s.substr(0,pos);
        if (p<labels.size()-1)
        {
            if (token.compare(labels[p+1])!=0)
            {
                labels.erase(labels.begin()+p+1,labels.end());
                menus.erase(menus.begin()+p+1,menus.end());
            }
        }
        if (p+1>=labels.size())
        {
            labels.push_back(token);
            VMenu* prim=new VMenu();
            menus[menus.size()-1]->appendMenuAndDetach(prim,true,token.c_str());
            menus.push_back(prim);
        }
        p++;
        s.erase(0,pos+1);
    }
    if (p==0)
    {
        labels.erase(labels.begin()+1,labels.end());
        menus.erase(menus.begin()+1,menus.end());
    }
    if (_label.size()>0)
        menus[menus.size()-1]->appendMenuItem((_state&1)!=0,(_state&2)!=0,_handle,_label.c_str(),(_state&6)!=0);
}
#endif
