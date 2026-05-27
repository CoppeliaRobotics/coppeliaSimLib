#pragma once

#include <interfaceStackObject.h>
#include <simMath/mXnMatrix.h>

class CInterfaceStackMatrix : public CInterfaceStackObject
{
  public:
    CInterfaceStackMatrix(const double* matrix, size_t rows, size_t cols);
    virtual ~CInterfaceStackMatrix();

    CInterfaceStackObject* copyYourself() const override;
    CInterfaceStackObject* getTypeEquivalent() const override;
    void fetchContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const CMatrix* getValue() const;
    void setValue(const CMatrix* matrix);

  protected:
    CMatrix _matrix;
};
