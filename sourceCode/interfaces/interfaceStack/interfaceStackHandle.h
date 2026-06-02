#pragma once

#include <interfaceStackObject.h>

class CInterfaceStackHandle : public CInterfaceStackObject
{
  public:
    CInterfaceStackHandle(int64_t theValue);
    virtual ~CInterfaceStackHandle();

    CInterfaceStackObject* copyYourself() const override;
    void fetchContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    int64_t getValue() const;
    void setValue(int64_t v);

  protected:
    int64_t _value;
};
