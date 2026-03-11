#pragma once

#include <interfaceStackObject.h>

class CInterfaceStackHandleArray : public CInterfaceStackObject
{
  public:
    CInterfaceStackHandleArray(const long long int* vals, size_t cnt);
    virtual ~CInterfaceStackHandleArray();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const long long int* getValue(size_t* cnt) const;
    void setValue(const long long int* vals, size_t cnt);
    void setValue(const int* vals, size_t cnt);

  protected:
    std::vector<long long int> _handles;
};
