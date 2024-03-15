#pragma once

#include <simLib/simConst.h>
#include <string>
#include <cbor.h>

class CInterfaceStackObject
{
  public:
    CInterfaceStackObject();
    virtual ~CInterfaceStackObject();

    virtual CInterfaceStackObject *copyYourself() const;
    virtual void printContent(int spaces, std::string &buffer) const;
    virtual std::string getObjectData(std::string &auxInfos) const;
    virtual void addCborObjectData(CCbor *cborObj) const;
    virtual unsigned int createFromData(const char *data, unsigned char version, std::vector<CInterfaceStackObject*> &allCreatedObjects);
    static CInterfaceStackObject *createFromDataStatic(const char *data, unsigned int &retOffset, unsigned char version, std::vector<CInterfaceStackObject*> &allCreatedObjects);

    int getObjectType() const;

  protected:
    int _objectType;
};
