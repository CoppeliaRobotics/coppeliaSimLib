#include <codeEditorVariables.h>
#include <utils.h>

CCodeEditorVariables::CCodeEditorVariables()
{
}

CCodeEditorVariables::~CCodeEditorVariables()
{
    clear();
}

void CCodeEditorVariables::clear()
{
    _allVars.clear();
}

bool CCodeEditorVariables::set(const char* info)
{
    bool retVal=true;
    clear();

    std::string inf(info);
    std::string line;
    while (utils::extractLine(inf,line))
    {
        utils::removeSpacesAtBeginningAndEnd(line);
        utils::removeComments(line);
        utils::replaceSubstring(line,"\\n","\n");
        size_t equalP=line.find('=');
        size_t openP=line.find('(');
        size_t closeP=line.find(')');
        if ( (equalP==std::string::npos)&&(openP==std::string::npos)&&(closeP==std::string::npos) )
        {
            if (line.find(' ')!=std::string::npos)
            {
                retVal=false;
                break;
            }
            if (line.size()>0) // empty lines are ok
                _allVars.push_back(line);
        }
    }

    return(retVal);
}

void CCodeEditorVariables::print() const
{
    printf("Vars:\n");
    for (size_t i=0;i<_allVars.size();i++)
        printf("%s\n",_allVars[i].c_str());
}

void CCodeEditorVariables::insertWhatStartsSame(const char* txt,std::set<std::string>& v) const
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (size_t i=0;i<_allVars.size();i++)
    {
        std::string n(_allVars[i]);
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
