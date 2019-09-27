
#pragma once

#include "vrepMainHeader.h"

class CConfReaderAndWriter  
{
public:
    CConfReaderAndWriter();
    virtual ~CConfReaderAndWriter();

    bool readConfiguration(const char* filename);
    bool writeConfiguration(const char* filename);

    bool getString(const char* variableName,std::string& variable);
    bool getInteger(const char* variableName,int& variable);
    bool getFloat(const char* variableName,float& variable);
    bool getFloatVector3(const char* variableName,float variable[3]);
    bool getIntVector3(const char* variableName,int variable[3]);
    bool getBoolean(const char* variableName,bool& variable);

    bool addString(const char* variableName,std::string variable,const char* comment="");
    bool addInteger(const char* variableName,int variable,const char* comment="");
    bool addFloat(const char* variableName,float variable,const char* comment="");
    bool addFloatVector3(const char* variableName,float variable[3],const char* comment="");
    bool addIntVector3(const char* variableName,int variable[3],const char* comment="");
    bool addBoolean(const char* variableName,bool variable,const char* comment="");
    bool addRandomLine(const char* lineText);

private:
    int _getVariableIndex(const char* variableName);
    std::vector<std::string> _variables;
    std::vector<std::string> _values;
    std::vector<std::string> _comments;
};
