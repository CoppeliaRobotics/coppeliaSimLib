#pragma once

#include <interfaceStackObject.h>

class CInterfaceStackNumber : public CInterfaceStackObject
{
  public:
    CInterfaceStackNumber(double theValue);
    virtual ~CInterfaceStackNumber();

    CInterfaceStackObject *copyYourself() const;
    void printContent(int spaces, std::string &buffer) const;
    std::string getObjectData() const;
    void addCborObjectData(CCbor *cborObj) const;
    unsigned int createFromData(const char *data, unsigned char version);
    static bool checkCreateFromData(const char *data, unsigned int &w, unsigned int l, unsigned char version);

    double getValue() const;
    void setValue(long long int v);
    void setValue(double v);

  protected:
    double _value;
};
