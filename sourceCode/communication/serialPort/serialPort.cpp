#include "vrepMainHeader.h"
#include "serialPort.h"
#include "serial_win_mac_linux.h"

int CSerialPort::_qserialPort_nextHandle=0;

CSerialPort::CSerialPort(bool wasOpenedFromAScript,bool useQSerialPort)
{
    _useQSerialPort=useQSerialPort;
    _qserialPort=nullptr;
    _portHandle=-1;
    _portNumber_backwardCompatibility=-1;
    _wasOpenedFromAScript=wasOpenedFromAScript;
}

CSerialPort::~CSerialPort()
{
    if (_portHandle>=0)
        close();
}

void CSerialPort::setPortNumber_backwardCompatibility(int nb)
{
    _portNumber_backwardCompatibility=nb;
}

int CSerialPort::getPortNumber_backwardCompatibility()
{
    return(_portNumber_backwardCompatibility);
}

int CSerialPort::getPortHandle()
{
    return(_portHandle);
}

bool CSerialPort::wasOpenedFromAScript()
{
    return(_wasOpenedFromAScript);
}

bool CSerialPort::open(const char* portString,int nBaud)
{
    if (!_useQSerialPort)
        _portHandle=serialOpen(portString,nBaud);
    else
    {
        _portHandle=-1; // means error
        _qserialPort=new QSerialPort();

        if (_qserialPort->setBaudRate(nBaud))
        {
//          _qserialPort->setFlowControl(QSerialPort::HardwareControl);
            _qserialPort->setPortName(portString);
            if (_qserialPort->open(QIODevice::ReadWrite))
                _portHandle=_qserialPort_nextHandle++;
        }
        if (_portHandle<0)
        {
            delete _qserialPort;
            _qserialPort=nullptr;
        }
    }
    return (_portHandle>=0);
}

bool CSerialPort::close()
{
    if (_portHandle>=0)
    {
        if (!_useQSerialPort)
            return(serialClose(_portHandle)==0);
        else
        {
            _qserialPort->close();
            delete _qserialPort;
            _qserialPort=nullptr;
            return(true);
        }
    }
    return(false);
}

int CSerialPort::sendData(const char *buffer,int size)
{
    if (_portHandle>=0)
    {
        if (!_useQSerialPort)
            return(serialWrite(_portHandle,buffer,size));
        else
        {
            int ret=_qserialPort->write(buffer,size);
            _qserialPort->flush();
            return(ret);
        }
    }
    return(0);
}


int CSerialPort::readDataWaiting( void )
{
    if (_portHandle>=0)
    {
        if (!_useQSerialPort)
            return(serialCheck(_portHandle));
        else
            return(_qserialPort->bytesAvailable());
    }
    return(0);
}

int CSerialPort::readData(void *buffer,int limit)
{
    if (_portHandle>=0)
    {
        if (!_useQSerialPort)
            return(serialRead(_portHandle,(char*)buffer,limit));
        else
            return(_qserialPort->read((char*)buffer,limit));
    }
    return(0);
}

