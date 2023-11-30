#include <serialPortContainer.h>
#include <simInternal.h>
#include <tt.h>
#include <utils.h>
#include <app.h>

CSerialPortContainer::CSerialPortContainer()
{
}

CSerialPortContainer::~CSerialPortContainer()
{
    _closeAllPorts();
}

void CSerialPortContainer::_closeAllPorts()
{ // closes all ports of this instance
    while (_openedSerialPorts.size() != 0)
        serialPortClose(_openedSerialPorts[0]->getPortHandle());
}

int CSerialPortContainer::_addOpenedPort(CSerialPort *port)
{
    _openedSerialPorts.push_back(port);
    return (port->getPortHandle());
}

CSerialPort *CSerialPortContainer::_getPortFromPortNumber_backwardCompatibility(int portNb)
{ // for backward compatibility (10/04/2012)
    for (int i = 0; i < int(_openedSerialPorts.size()); i++)
    {
        if (_openedSerialPorts[i]->getPortNumber_backwardCompatibility() == portNb)
            return (_openedSerialPorts[i]);
    }
    return (nullptr);
}

CSerialPort *CSerialPortContainer::_getPortFromHandle(int portHandle)
{
    for (int i = 0; i < int(_openedSerialPorts.size()); i++)
    {
        if (_openedSerialPorts[i]->getPortHandle() == portHandle)
            return (_openedSerialPorts[i]);
    }
    return (nullptr);
}

void CSerialPortContainer::simulationEnded()
{ // Close only ports that were opened through a script:
    for (int i = 0; i < int(_openedSerialPorts.size()); i++)
    {
        if (_openedSerialPorts[i]->wasOpenedFromAScript())
        {
            serialPortClose(_openedSerialPorts[i]->getPortHandle());
            i = -1; // We have to restart the loop from the beginning!
        }
    }
}

int CSerialPortContainer::serialPortOpen(bool fromScript, const char *name, int baudrate)
{
    CSerialPort *port = new CSerialPort(fromScript);
    if (port->open(name, baudrate) == 0)
    {
        delete port;
        return (-1);
    }
    return (_addOpenedPort(port));
}

bool CSerialPortContainer::serialPortClose(int portHandle)
{
    bool retVal = false;
    for (int i = 0; i < int(_openedSerialPorts.size()); i++)
    {
        if (_openedSerialPorts[i]->getPortHandle() == portHandle)
        {
            delete _openedSerialPorts[i];
            _openedSerialPorts.erase(_openedSerialPorts.begin() + i);
            retVal = true;
            break;
        }
    }
    return (retVal);
}

int CSerialPortContainer::serialPortCheck(int portHandle)
{
    int retVal = -1;
    CSerialPort *port = _getPortFromHandle(portHandle);
    if (port != nullptr)
        retVal = port->readDataWaiting();
    return (retVal);
}

int CSerialPortContainer::serialPortSend(int portHandle, const std::string &data)
{
    CSerialPort *port = _getPortFromHandle(portHandle);
    int retVal = -1;
    if (port != nullptr)
        retVal = port->sendData(&data[0], (int)data.size());
    return (retVal);
}

int CSerialPortContainer::serialPortReceive(int portHandle, std::string &data, int dataLengthToRead)
{
    CSerialPort *port = _getPortFromHandle(portHandle);
    int retVal = 0;
    if (port != nullptr)
    {
        int availableData = port->readDataWaiting();
        if (availableData > 0)
            retVal = port->readData(&data[0], std::min<int>(availableData, dataLengthToRead));
    }
    return (retVal);
}

bool CSerialPortContainer::serialPortOpen_old(bool fromScript, int portNumber, int baudRate)
{ // old, for backward compatibility
    CSerialPort *port = new CSerialPort(fromScript);
    std::string theString("\\\\.\\COM");
    theString += utils::getIntString(false, portNumber);
    if (port->open(theString.c_str(), baudRate) == 0)
        return (false);
    port->setPortNumber_backwardCompatibility(portNumber);
    _addOpenedPort(port);
    return (true);
}

bool CSerialPortContainer::serialPortClose_old(int portNumber)
{ // old, for backward compatibility
    CSerialPort *port = _getPortFromPortNumber_backwardCompatibility(portNumber);
    if (port == nullptr)
        return (false);
    serialPortClose(port->getPortHandle());
    return (true);
}

int CSerialPortContainer::serialPortSend_old(int portNumber, const char *data, int dataLength)
{ // old, for backward compatibility
    CSerialPort *port = _getPortFromPortNumber_backwardCompatibility(portNumber);
    if (port == nullptr)
        return (-1);
    return (port->sendData(data, dataLength));
}

int CSerialPortContainer::serialPortReceive_old(int portNumber, char *buffer, int dataLengthToRead)
{ // old, for backward compatibility
    CSerialPort *port = _getPortFromPortNumber_backwardCompatibility(portNumber);
    if (port == nullptr)
        return (-1);
    int retVal = 0;
    int availableData = port->readDataWaiting();
    if (availableData != 0)
        retVal = port->readData(buffer, std::min<int>(availableData, dataLengthToRead));
    return (retVal);
}
