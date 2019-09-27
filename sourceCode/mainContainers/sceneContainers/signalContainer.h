
#pragma once

#include "vrepMainHeader.h"
#include "mainCont.h"

class CSignalContainer : public CMainCont 
{
public:
    CSignalContainer();
    virtual ~CSignalContainer();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void emptySceneProcedure();

    void removeAllSignals(bool onlyThoseFromEmbeddedScripts);

    void setIntegerSignal(const char* signalName,int value,bool fromEmbeddedScript);
    bool getIntegerSignal(const char* signalName,int& value);
    bool getIntegerSignalNameAtIndex(int index,std::string& signalName);
    int clearIntegerSignal(const char* signalName);
    int clearAllIntegerSignals(bool onlyThoseFromEmbeddedScripts);

    void setFloatSignal(const char* signalName,float value,bool fromEmbeddedScript);
    bool getFloatSignal(const char* signalName,float& value);
    bool getFloatSignalNameAtIndex(int index,std::string& signalName);
    int clearFloatSignal(const char* signalName);
    int clearAllFloatSignals(bool onlyThoseFromEmbeddedScripts);

    void setDoubleSignal(const char* signalName,double value,bool fromEmbeddedScript);
    bool getDoubleSignal(const char* signalName,double& value);
    bool getDoubleSignalNameAtIndex(int index,std::string& signalName);
    int clearDoubleSignal(const char* signalName);
    int clearAllDoubleSignals(bool onlyThoseFromEmbeddedScripts);

    void setStringSignal(const char* signalName,const std::string& value,bool fromEmbeddedScript);
    bool getStringSignal(const char* signalName,std::string& value);
    bool getStringSignalNameAtIndex(int index,std::string& signalName);
    int clearStringSignal(const char* signalName);
    int clearAllStringSignals(bool onlyThoseFromEmbeddedScripts);

protected:
    int _getIntegerSignalIndex(const char* signalName);
    int _getFloatSignalIndex(const char* signalName);
    int _getDoubleSignalIndex(const char* signalName);
    int _getStringSignalIndex(const char* signalName);

    std::vector<std::string> _intSignalNames;
    std::vector<int> _intSignalValues;
    std::vector<bool> _intSignalEmbScriptCreated;

    std::vector<std::string> _floatSignalNames;
    std::vector<float> _floatSignalValues;
    std::vector<bool> _floatSignalEmbScriptCreated;

    std::vector<std::string> _doubleSignalNames;
    std::vector<double> _doubleSignalValues;
    std::vector<bool> _doubleSignalEmbScriptCreated;

    std::vector<std::string> _stringSignalNames;
    std::vector<std::string> _stringSignalValues;
    std::vector<bool> _stringSignalEmbScriptCreated;
};
