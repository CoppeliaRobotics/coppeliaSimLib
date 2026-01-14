#pragma once

#include <interfaceStackObject.h>
#include <simMath/7Vector.h>

class CInterfaceStackPose : public CInterfaceStackObject
{
  public:
    CInterfaceStackPose(const double* data, bool xyzqxqyqzqwLayout);
    virtual ~CInterfaceStackPose();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const C7Vector* getValue() const;
    void setValue(const C7Vector* p);

  protected:
    C7Vector _pose;
};
