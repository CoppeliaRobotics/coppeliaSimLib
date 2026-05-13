#include <obj.h>
#include <utils.h>

std::string OBJECT_TYPE = "object";

Obj::Obj()
{
    _isClass = false;
    _isSceneObject = false;
}

Obj::Obj(long long int objectHandle, const char* objectTypeStr, const char* metaInfo)
{
    _objectHandle = objectHandle;
    _objectTypeStr = objectTypeStr;
    setMetaInfo(metaInfo);
    _originalObjectTypeStr = objectTypeStr;
    _isClass = false;
    _isSceneObject = false;
}

Obj::~Obj()
{
}

void Obj::copyYourselfInto(Obj* it) const
{
    it->_objectHandle = _objectHandle;
    it->_objectTypeStr = _objectTypeStr;
    it->_superClass = _superClass;
    it->_nameSpaces = _nameSpaces;
    it->_originalObjectTypeStr = _originalObjectTypeStr;
    it->_isClass = _isClass;
    it->_isSceneObject = _isSceneObject;
}

void Obj::addObjectEventData(CCbor* ev)
{
    ev->appendKeyText(propObject_objectType.name, _objectTypeStr.c_str());
}

long long int Obj::getObjectHandle() const
{
    return _objectHandle;
}

void Obj::setOriginalObjectTypeStr(const char* originalObjectTypeStr)
{
    _originalObjectTypeStr = originalObjectTypeStr;
}

std::string Obj::getOriginalObjectTypeStr() const
{
    return _originalObjectTypeStr;
}

void Obj::setObjectTypeStr(const char* objectTypeStr)
{
    _objectTypeStr = objectTypeStr;
    _originalObjectTypeStr = _objectTypeStr;
}

std::string Obj::getObjectTypeStr() const
{
    return _objectTypeStr;
}

void Obj::setMetaInfo(const char* info)
{
    _superClass.clear();
    _nameSpaces.clear();

    const QStringList parts = QString(info).split(';', Qt::SkipEmptyParts);
    for (const auto& part : parts)
    {
        const int colonIdx = part.indexOf(':');
        if (colonIdx < 0) continue;

        const QString key = part.left(colonIdx).trimmed().toLower();
        const QStringList values = part.mid(colonIdx + 1).split(',', Qt::SkipEmptyParts);

        std::vector<std::string>* target = nullptr;
        if (key == "superclass")
            target = &_superClass;
        else if (key == "namespaces")
            target = &_nameSpaces;

        if (target)
            for (const auto& v : values)
                target->push_back(v.trimmed().toStdString());
    }
    if ((_superClass.size() == 0) || (_superClass[_superClass.size() - 1] != "object"))
        _superClass.push_back("object");
}

void Obj::setMetaInfo(const std::vector<std::string>& superClass, const std::vector<std::string>& nameSpaces)
{
    _superClass = superClass;
    _nameSpaces = nameSpaces;
    if ((_superClass.size() == 0) || (_superClass[_superClass.size() - 1] != "object"))
        _superClass.push_back("object");
}

std::string Obj::getMetaInfo() const
{
    std::string retVal;
    for (size_t i = 0; i < _superClass.size(); i++)
    {
        if (i == 0)
            retVal += "superClass: ";
        else
            retVal += ",";
        retVal += _superClass[i];
    }
    if (!retVal.empty())
        retVal +=";";
    for (size_t i = 0; i < _nameSpaces.size(); i++)
    {
        if (i == 0)
            retVal += "nameSpaces: ";
        else
            retVal += ",";
        retVal += _nameSpaces[i];
    }
    return retVal;
}

void Obj::setIsClass(bool isClass)
{
    _isClass = isClass;
}

bool Obj::isClass() const
{
    return _isClass;
}

bool Obj::isSceneObject() const
{
    return _isSceneObject;
}

int Obj::getBoolProperty(const char* ppName, bool& pState) const
{
    int retVal = sim_propertyret_unknownproperty;

    if (strcmp(ppName, propObject_metaInfoIsClass.name) == 0)
    {
        pState = _isClass;
        retVal = sim_propertyret_ok;
    }
    else if (strcmp(ppName, propObject_metaInfoIsSceneObject.name) == 0)
    {
        pState = _isSceneObject;
        retVal = sim_propertyret_ok;
    }

    return retVal;
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

    return retVal;
}

int Obj::getStringArrayProperty(const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknownproperty;

    if (strcmp(ppName, propObject_metaInfoSuperClass.name) == 0)
    {
        pState = _superClass;
        retVal = sim_propertyret_ok;
    }
    else if (strcmp(ppName, propObject_metaInfoNameSpaces.name) == 0)
    {
        pState = _nameSpaces;
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
