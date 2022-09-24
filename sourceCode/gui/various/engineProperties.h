#pragma once

#include "annJson.h"

class CEngineProperties
{
public:
    CEngineProperties();
    virtual ~CEngineProperties();

    void editObjectProperties(int objectHandle) const;

private:
    float _getGlobalFloatParam(int item,std::string& comment,const char* additionalComment=nullptr) const;
    int _getGlobalIntParam(int item,std::string& comment,const char* additionalComment=nullptr) const;
    bool _getGlobalBoolParam(int item,std::string& comment,const char* additionalComment=nullptr) const;
    void _getGlobalFloatParams(int item,double* w,size_t cnt,std::string& comment,const char* additionalComment=nullptr) const;

    void _writeGlobal(int engine,CAnnJson& annJson) const;
    void _readGlobal(int engine,CAnnJson& annJson,std::string* allErrors) const;
    void _writeJoint(int engine,int jointHandle,CAnnJson& annJson) const;
    void _readJoint(int engine,int jointHandle,CAnnJson& annJson,std::string* allErrors) const;
    void _writeShape(int engine,int shaoeHandle,CAnnJson& annJson) const;
    void _readShape(int engine,int shapeHandle,CAnnJson& annJson,std::string* allErrors) const;
    void _writeDummy(int engine,int dummyHandle,CAnnJson& annJson) const;
    void _readDummy(int engine,int dummyHandle,CAnnJson& annJson,std::string* allErrors) const;
};
