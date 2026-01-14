#pragma once

#include <interfaceStackObject.h>
#include <simMath/4Vector.h>

class CInterfaceStackQuaternion : public CInterfaceStackObject
{
  public:
    CInterfaceStackQuaternion(const double* data, bool xyzwLayout);
    virtual ~CInterfaceStackQuaternion();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const C4Vector* getValue() const;
    void setValue(const C4Vector* q);

  protected:
    C4Vector _quaternion;
};
