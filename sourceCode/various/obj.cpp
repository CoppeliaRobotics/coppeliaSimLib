#include <obj.h>
#include <utils.h>

std::string OBJECT_TYPE = "object";
std::string OBJECT_META_INFO = R"(
{
    "superclass": "",
    "namespaces": {
        "namedParam": {},
        "customData": {},
        "signal": {}
    }
}
)";

Obj::Obj()
{
}

Obj::~Obj()
{
}

int Obj::setBoolProperty(const char* ppName, bool pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getBoolProperty(const char* ppName, bool& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setIntProperty(const char* ppName, int pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getIntProperty(const char* ppName, int& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setLongProperty(const char* ppName, long long int pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getLongProperty(const char* ppName, long long int& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setHandleProperty(const char* ppName, long long int pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getHandleProperty(const char* ppName, long long int& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setFloatProperty(const char* ppName, double pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getFloatProperty(const char* ppName, double& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setStringProperty(const char* ppName, const char* pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getStringProperty(const char* ppName, std::string& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setBufferProperty(const char* ppName, const char* buffer, int bufferL)
{
    int retVal = -1;
    return retVal;
}

int Obj::getBufferProperty(const char* ppName, std::string& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setIntArray2Property(const char* ppName, const int* pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getIntArray2Property(const char* ppName, int* pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setVector2Property(const char* ppName, const double* pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getVector2Property(const char* ppName, double* pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setVector3Property(const char* ppName, const C3Vector& pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getVector3Property(const char* ppName, C3Vector& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setQuaternionProperty(const char* ppName, const C4Vector& pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getQuaternionProperty(const char* ppName, C4Vector& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setPoseProperty(const char* ppName, const C7Vector& pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getPoseProperty(const char* ppName, C7Vector& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setColorProperty(const char* ppName, const float* pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getColorProperty(const char* ppName, float* pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setFloatArrayProperty(const char* ppName, const double* v, int vL)
{
    int retVal = -1;
    return retVal;
}

int Obj::getFloatArrayProperty(const char* ppName, std::vector<double>& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setIntArrayProperty(const char* ppName, const int* v, int vL)
{
    int retVal = -1;
    return retVal;
}

int Obj::getIntArrayProperty(const char* ppName, std::vector<int>& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setHandleArrayProperty(const char* ppName, const long long int* v, int vL)
{
    int retVal = -1;
    return retVal;
}

int Obj::getHandleArrayProperty(const char* ppName, std::vector<long long int>& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::setStringArrayProperty(const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = -1;
    return retVal;
}

int Obj::getStringArrayProperty(const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = -1;
    return retVal;
}

int Obj::removeProperty(const char* ppName)
{
    int retVal = -1;
    return retVal;
}

int Obj::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = getPropertyName_static(index, pName, appartenance, excludeFlags);
    return retVal;
}

int Obj::getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_obj.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_obj[i].name, pName.c_str()))
        {
            if ((allProps_obj[i].flags & excludeFlags) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_obj[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    /*
    if (retVal == 1)
    { // Following needed to accomodate for Lua's object representation
        if (pName == "objectType")
            appartenance = "object";
        else if (pName == "objectMetaInfo")
            appartenance = "object";
        else if (pName == "handle")
            appartenance = "object";
    }
*/
    return retVal;
}

int Obj::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = getPropertyInfo_static(ppName, info, infoTxt);
    return retVal;
}

int Obj::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_obj.size(); i++)
    {
        if (strcmp(allProps_obj[i].name, ppName) == 0)
        {
            retVal = allProps_obj[i].type;
            info = allProps_obj[i].flags;
            if (infoTxt == "j")
                infoTxt = allProps_obj[i].shortInfoTxt;
            else
            {
                auto w = QJsonDocument::fromJson(allProps_obj[i].shortInfoTxt.c_str()).object();
                std::string descr = w["description"].toString().toStdString();
                std::string label = w["label"].toString().toStdString();
                if ( (infoTxt == "s") || (descr == "") )
                    infoTxt = label;
                else
                    infoTxt = descr;
            }
            break;
        }
    }
    return retVal;
}
