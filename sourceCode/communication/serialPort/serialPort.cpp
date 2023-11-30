#include <serialPort.h>
#include <serial_win_mac_linux.h>

CSerialPort::CSerialPort(bool wasOpenedFromAScript)
{
    _portHandle = -1;
    _portNumber_backwardCompatibility = -1;
    _wasOpenedFromAScript = wasOpenedFromAScript;
}

CSerialPort::~CSerialPort()
{
    if (_portHandle >= 0)
        close();
}

void CSerialPort::setPortNumber_backwardCompatibility(int nb)
{
    _portNumber_backwardCompatibility = nb;
}

int CSerialPort::getPortNumber_backwardCompatibility()
{
    return (_portNumber_backwardCompatibility);
}

int CSerialPort::getPortHandle()
{
    return (_portHandle);
}

bool CSerialPort::wasOpenedFromAScript()
{
    return (_wasOpenedFromAScript);
}

bool CSerialPort::open(const char *portString, int nBaud)
{
    _portHandle = serialOpen(portString, nBaud);
    return (_portHandle >= 0);
}

bool CSerialPort::close()
{
    if (_portHandle >= 0)
        return (serialClose(_portHandle) == 0);
    return (false);
}

int CSerialPort::sendData(const char *buffer, int size)
{
    if (_portHandle >= 0)
        return (serialWrite(_portHandle, buffer, size));
    return (0);
}

int CSerialPort::readDataWaiting(void)
{
    if (_portHandle >= 0)
        return (serialCheck(_portHandle));
    return (0);
}

int CSerialPort::readData(void *buffer, int limit)
{
    if (_portHandle >= 0)
        return (serialRead(_portHandle, (char *)buffer, limit));
    return (0);
}
