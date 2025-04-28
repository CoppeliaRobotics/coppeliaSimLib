#pragma once

#include <interfaceStackObject.h>

class CInterfaceStackBool : public CInterfaceStackObject
{
  public:
    CInterfaceStackBool(bool theValue);
    virtual ~CInterfaceStackBool();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    bool getValue() const;

  protected:
    bool _value;
};
