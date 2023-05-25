#include <codeEditorFunctions.h>
#include <utils.h>

CCodeEditorFunctions::CCodeEditorFunctions()
{
}

CCodeEditorFunctions::~CCodeEditorFunctions()
{
    clear();
}

void CCodeEditorFunctions::clear()
{
    _allFuncs.clear();
}

bool CCodeEditorFunctions::set(const char* info)
{
    bool retVal=true;
    clear();

    std::string inf(info);
    std::string line;
    while (utils::extractLine(inf,line))
    {
        utils::removeSpacesAtBeginningAndEnd(line);
        utils::removeComments(line);
        size_t openP=line.find('(');
        size_t closeP=line.find(')');
        if ( (openP!=std::string::npos)&&(closeP!=std::string::npos)&&(openP<closeP) )
        {
            std::string funcName(line.begin(),line.begin()+openP);
            size_t equal=line.find('=');
            if (equal!=std::string::npos)
                funcName.assign(line.begin()+equal+1,line.begin()+openP);
            utils::removeSpacesAtBeginningAndEnd(funcName);
            auto it=_allFuncs.find(funcName);
            if (it==_allFuncs.end())
            {
                _allFuncs[funcName]="";
                it=_allFuncs.find(funcName);
            }
            if (it->second.size()!=0)
                it->second+="\n";
            it->second+=line;
        }
    }

    return(retVal);
}

void CCodeEditorFunctions::print() const
{
    printf("Funcs:\n");
    for (auto it=_allFuncs.begin();it!=_allFuncs.end();it++)
        printf("%s\n",it->second.c_str());
}

void CCodeEditorFunctions::insertWhatStartsSame(const char* txt,std::set<std::string>& v) const
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (auto it=_allFuncs.begin();it!=_allFuncs.end();it++)
    {
        std::string n(it->first);
        if (n.find(txt)==0)
        {
            if ( (!hasDot)&&(ttxt.size()>0) )
            {
                size_t dp=n.find('.');
                if (dp!=std::string::npos)
                    n.erase(n.begin()+dp,n.end()); // we only push the text up to the dot, if txt is not empty
            }
            v.insert(n);
        }
    }
}

std::string CCodeEditorFunctions::getFunctionCalltip(const char* txt) const
{
    std::string retVal;
    auto it=_allFuncs.find(txt);
    if (it!=_allFuncs.end())
        retVal=it->second;
    return(retVal);
}
