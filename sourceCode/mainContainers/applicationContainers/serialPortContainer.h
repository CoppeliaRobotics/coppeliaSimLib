
#pragma once

#include "serialPort.h"
#include "uiThread.h"

class CSerialPortContainer
{
public:
    CSerialPortContainer();
    virtual ~CSerialPortContainer();

    void simulationEnded();

    // From UI thread:
    void executeCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    // General routines, from UI or SIM thread:
    int serialPortOpen(bool fromScript,const char* name,int baudrate);
    bool serialPortClose(int portHandle);
    int serialPortCheck(int portHandle);
    int serialPortSend(int portHandle,const std::string& data);
    int serialPortReceive(int portHandle,std::string& data,int dataLengthToRead);

    // Alternative routines from any thread:
    int serialPortOpen_alternativeRoutine(bool fromScript,const char* name,int baudrate);
    bool serialPortClose_alternativeRoutine(int portHandle);
    int serialPortCheck_alternativeRoutine(int portHandle);
    int serialPortSend_alternativeRoutine(int portHandle,const std::string& data);
    int serialPortReceive_alternativeRoutine(int portHandle,std::string& data,int dataLengthToRead);

    // Old routines, for backward compatibility:
    bool serialPortOpen_old(bool fromScript,int portNumber,int baudRate);
    bool serialPortClose_old(int portNumber);
    int serialPortSend_old(int portNumber,const char* data,int dataLength);
    int serialPortReceive_old(int portNumber,char* buffer,int dataLengthToRead);

protected:
    void _closeAllPorts();
    CSerialPort* _getPortFromHandle(int portHandle);
    int _addOpenedPort(CSerialPort* port);
    CSerialPort* _getPortFromPortNumber_backwardCompatibility(int portNb);

    std::vector<CSerialPort*> _openedSerialPorts;
};
