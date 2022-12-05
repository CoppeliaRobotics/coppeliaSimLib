#pragma once

#include <string>
#include <vector>
#include "simTypes.h"

class CConfReaderAndWriter  
{
public:
    CConfReaderAndWriter();
    virtual ~CConfReaderAndWriter();

    bool readConfiguration(const char* filename);
    bool writeConfiguration(const char* filename);

    bool getString(const char* variableName,std::string& variable) const;
    bool getInteger(const char* variableName,int& variable) const;
    bool getFloat(const char* variableName,double& variable) const;
    bool getFloatVector3(const char* variableName,double variable[3]) const;
    bool getIntVector3(const char* variableName,int variable[3]) const;
    bool getIntVector2(const char* variableName,int variable[2]) const;
    bool getBoolean(const char* variableName,bool& variable) const;

    bool addString(const char* variableName,std::string variable,const char* comment="");
    bool addInteger(const char* variableName,int variable,const char* comment="");
    bool addFloat(const char* variableName,double variable,const char* comment="");
    bool addFloatVector3(const char* variableName,double variable[3],const char* comment="");
    bool addIntVector3(const char* variableName,int variable[3],const char* comment="");
    bool addIntVector2(const char* variableName,int variable[2],const char* comment="");
    bool addBoolean(const char* variableName,bool variable,const char* comment="");
    bool addRandomLine(const char* lineText);

private:
    static size_t _split(const char* value,char splitChar,std::vector<std::string>& vals);
    static bool _toFloatArray(const std::vector<std::string>& vals,size_t size,double* arr);
    static bool _toIntArray(const std::vector<std::string>& vals,size_t size,int* arr);
    int _getVariableIndex(const char* variableName) const;
    std::vector<std::string> _variables;
    std::vector<std::string> _values;
    std::vector<std::string> _comments;
};
