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

Obj::Obj(long long int objectHandle, const char* objectTypeStr, const char* objectMetaInfo)
{
    _objectHandle = objectHandle;
    _objectTypeStr = objectTypeStr;
    _objectMetaInfo =objectMetaInfo;
}

Obj::~Obj()
{
}

void Obj::addObjectEventData(CCbor* ev)
{
    ev->appendKeyText(propObject_objectType.name, _objectTypeStr.c_str());
}

long long int Obj::getObjectHandle() const
{
    return _objectHandle;
}

std::string Obj::getObjectTypeStr() const
{
    return _objectTypeStr;
}

std::string Obj::getObjectMetaInfo() const
{
    return _objectMetaInfo;
}

std::string Obj::getClass() const
{
    return _className;
}

void Obj::setClass(const char* className)
{
    _className = className;
}

int Obj::setLongProperty(const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknownproperty;

    if (strcmp(ppName, propObject_handle.name) == 0)
    {
        _objectHandle = pState;
        retVal = sim_propertyret_ok;
    }

    return retVal;
}

int Obj::getLongProperty(const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknownproperty;

    if (strcmp(ppName, propObject_handle.name) == 0)
    {
        pState = _objectHandle;
        retVal = sim_propertyret_ok;
    }

    return retVal;
}

int Obj::getStringProperty(const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknownproperty;

    if (strcmp(ppName, propObject_objectType.name) == 0)
    {
        pState = _objectTypeStr;
        retVal = sim_propertyret_ok;
    }
    else if (strcmp(ppName, propObject_objectMetaInfo.name) == 0)
    {
        pState = _objectMetaInfo;
        retVal = sim_propertyret_ok;
    }

    return retVal;
}

int Obj::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknownproperty;
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
                    appartenance = "object";
                    retVal = sim_propertyret_ok;
                    break;
                }
            }
        }
    }
    return retVal;
}

int Obj::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknownproperty;
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
