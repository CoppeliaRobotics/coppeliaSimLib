#pragma once

class CSerialPort
{

  public:
    CSerialPort(bool wasOpenedFromAScript);
    ~CSerialPort();

    bool open(const char* portString, int nBaud);
    bool close();

    int readData(void* buffer, int limit);
    int sendData(const char* buffer, int size);
    int readDataWaiting();

    int getPortHandle();
    bool wasOpenedFromAScript();

    void setPortNumber_backwardCompatibility(int nb);
    int getPortNumber_backwardCompatibility();

  protected:
    int _portHandle;
    bool _wasOpenedFromAScript;
    int _portNumber_backwardCompatibility;
};
