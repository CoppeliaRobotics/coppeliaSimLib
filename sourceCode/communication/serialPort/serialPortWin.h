// SerialPort.h (original name: CSerial.h)
// Original by Tom Archer and Rick Leinecker as found on CODEGURU:
// http://www.codeguru.com/cpp/i-n/network/serialcommunications/article.php/c2503/CSerial--A-C-Class-for-Serial-Communications.htm
// Slightly modified/adapted by Coppelia Robotics GmbH

#pragma once

#ifdef WIN_VREP
#include <windows.h>
#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

class CSerialPortWin
{

public:
    CSerialPortWin();
    ~CSerialPortWin();

    int Open(const char* portString,int nBaud);
    int Close();

    int ReadData(char *buffer,int limit);
    int SendData(const char *buffer,int size);
    int ReadDataWaiting();

    int IsOpened(){return(m_bOpened);}
    int getPortHandle();

protected:
    int WriteCommByte(unsigned char ucByte);

    void* m_hIDComDev;
    OVERLAPPED m_OverlappedRead,m_OverlappedWrite;

    int m_bOpened;
    int _portHandle;
    static int _nextPortHandle;
};
#endif
