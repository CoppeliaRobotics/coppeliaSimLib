#pragma once

#include "annJson.h""

class CEngineProperties
{
public:
    CEngineProperties();
    virtual ~CEngineProperties();

    void editObjectProperties(int objectHandle);

private:
    void _writeGlobal(int engine,CAnnJson& annJson);
    void _readGlobal(int engine,CAnnJson& annJson,std::string* allErrors);
    void _writeJoint(int engine,int jointHandle,CAnnJson& annJson);
    void _readJoint(int engine,int jointHandle,CAnnJson& annJson,std::string* allErrors);
    void _writeShape(int engine,int shaoeHandle,CAnnJson& annJson);
    void _readShape(int engine,int shapeHandle,CAnnJson& annJson,std::string* allErrors);
};
