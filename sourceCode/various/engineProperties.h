#pragma once

#include <annJson.h>

class CEngineProperties
{
  public:
    CEngineProperties();
    virtual ~CEngineProperties();

#ifdef SIM_WITH_GUI
    void editObjectProperties(int objectHandle) const;
#endif
    std::string getObjectProperties(int objectHandle, std::string* title = nullptr, bool stripComments = true) const;
    bool setObjectProperties(int objectHandle, const char* prop, std::string* errorString = nullptr, int* parseErrorLine = nullptr) const;

  private:
    double _getGlobalFloatParam(const char* item, std::string& comment, const char* additionalComment = nullptr) const;
    int _getGlobalIntParam(const char* item, std::string& comment, const char* additionalComment = nullptr) const;
    bool _getGlobalBoolParam(const char* item, std::string& comment, const char* additionalComment = nullptr) const;
    void _getGlobalFloatParams(const char* item, double* w, std::string& comment, const char* additionalComment = nullptr) const;

    void _writeGlobal(int engine, CAnnJson& annJson) const;
    void _readGlobal(int engine, CAnnJson& annJson, std::string* allErrors) const;
    void _writeJoint(int engine, int jointHandle, CAnnJson& annJson) const;
    void _readJoint(int engine, int jointHandle, CAnnJson& annJson, std::string* allErrors) const;
    void _writeShape(int engine, int shaoeHandle, CAnnJson& annJson) const;
    void _readShape(int engine, int shapeHandle, CAnnJson& annJson, std::string* allErrors) const;
    void _writeDummy(int engine, int dummyHandle, CAnnJson& annJson) const;
    void _readDummy(int engine, int dummyHandle, CAnnJson& annJson, std::string* allErrors) const;
};
