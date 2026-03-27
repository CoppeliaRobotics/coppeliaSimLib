#include <customObject.h>
#include <utils.h>

CustomObject::CustomObject(long long int handle, const char* objectTypeStr, const char* objectMetaInfo)
{
    _objectHandle = handle;
    _objectTypeStr = objectTypeStr;
    _objectMetaInfo = objectMetaInfo;
}

CustomObject::~CustomObject()
{
}

int CustomObject::getLongProperty(const char* ppName, long long int& pState) const
{
    int retVal = Obj::getLongProperty(ppName, pState);
    if (retVal == -1)
    {

    }
    return retVal;
}

int CustomObject::getStringProperty(const char* ppName, std::string& pState) const
{
    int retVal = Obj::getStringProperty(ppName, pState);
    if (retVal == -1)
    {

    }
    return retVal;
}

int CustomObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        /*
            for (size_t i = 0; i < allProps_customObj.size(); i++)
            {
                if ((pName.size() == 0) || utils::startsWith(allProps_customObj[i].name, pName.c_str()))
                {
                    if ((allProps_customObj[i].flags & excludeFlags) == 0)
                    {
                        index--;
                        if (index == -1)
                        {
                            pName = allProps_customObj[i].name;
                            appartenance = "object";
                            retVal = 1;
                            break;
                        }
                    }
                }
            }
            */
    }
    return retVal;
}

int CustomObject::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
    /*
    for (size_t i = 0; i < allProps_customObj.size(); i++)
    {
        if (strcmp(allProps_customObj[i].name, ppName) == 0)
        {
            retVal = allProps_customObj[i].type;
            info = allProps_customObj[i].flags;
            if (infoTxt == "j")
                infoTxt = allProps_customObj[i].shortInfoTxt;
            else
            {
                auto w = QJsonDocument::fromJson(allProps_customObj[i].shortInfoTxt.c_str()).object();
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
*/
    }
    return retVal;
}

