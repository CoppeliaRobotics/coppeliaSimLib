#pragma once

#include <interfaceStackObject.h>

class CInterfaceStackMatrix : public CInterfaceStackObject
{
  public:
    CInterfaceStackMatrix(const double* matrix, size_t rows, size_t cols);
    virtual ~CInterfaceStackMatrix();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const double* getValue(size_t& rows, size_t& cols) const;
    void setValue(const double* matrix, size_t rows, size_t cols);

  protected:
    std::vector<double> _data;
    size_t _rows;
    size_t _cols;
};
