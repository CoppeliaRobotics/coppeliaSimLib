// SerialPort.cpp (original name: CSerial.cpp)
// Original by Tom Archer and Rick Leinecker as found on CODEGURU:
// http://www.codeguru.com/cpp/i-n/network/serialcommunications/article.php/c2503/CSerial--A-C-Class-for-Serial-Communications.htm
// Slightly modified/adapted by Coppelia Robotics GmbH

#include "serialPortWin.h"

#ifdef WIN_VREP

int CSerialPortWin::_nextPortHandle=0;

CSerialPortWin::CSerialPortWin()
{
    memset(&m_OverlappedRead,0,sizeof(OVERLAPPED));
    memset(&m_OverlappedWrite,0,sizeof(OVERLAPPED));
    m_hIDComDev=nullptr;
    m_bOpened=FALSE;
    _portHandle=_nextPortHandle++;
}

CSerialPortWin::~CSerialPortWin()
{
    Close();
}

int CSerialPortWin::getPortHandle()
{
    return(_portHandle);
}

int CSerialPortWin::Open(const char* portString,int nBaud)
{
    if (m_bOpened)
        return(TRUE);
    DCB dcb;
    m_hIDComDev=CreateFileA(portString,GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,nullptr);

    if (m_hIDComDev==nullptr)
        return(FALSE);
    memset(&m_OverlappedRead,0,sizeof(OVERLAPPED));
    memset(&m_OverlappedWrite,0,sizeof(OVERLAPPED));

    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout=0xFFFFFFFF;
    CommTimeOuts.ReadTotalTimeoutMultiplier=0;
    CommTimeOuts.ReadTotalTimeoutConstant=0;
    CommTimeOuts.WriteTotalTimeoutMultiplier=0;
    CommTimeOuts.WriteTotalTimeoutConstant=5000;
    SetCommTimeouts(m_hIDComDev,&CommTimeOuts);

    m_OverlappedRead.hEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);
    m_OverlappedWrite.hEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);

    dcb.DCBlength=sizeof(DCB);
    GetCommState(m_hIDComDev,&dcb);
    dcb.BaudRate=nBaud;
    dcb.ByteSize=8;
    unsigned char ucSet;
    ucSet=(unsigned char)((FC_RTSCTS&FC_DTRDSR)!=0);
    ucSet=(unsigned char)((FC_RTSCTS&FC_RTSCTS)!=0);
    ucSet=(unsigned char)((FC_RTSCTS&FC_XONXOFF)!=0);
    if( !SetCommState( m_hIDComDev, &dcb ) ||
        !SetupComm( m_hIDComDev, 10000, 10000 ) ||
        m_OverlappedRead.hEvent == nullptr ||
        m_OverlappedWrite.hEvent == nullptr ){
        //DWORD dwError = GetLastError();
        if( m_OverlappedRead.hEvent != nullptr ) CloseHandle( m_OverlappedRead.hEvent );
        if( m_OverlappedWrite.hEvent != nullptr ) CloseHandle( m_OverlappedWrite.hEvent );
        CloseHandle( m_hIDComDev );
        return( FALSE );
        }

    m_bOpened = TRUE;

    return( m_bOpened );
}

int CSerialPortWin::Close( void )
{
    if( !m_bOpened || m_hIDComDev == nullptr ) return( TRUE );

    if( m_OverlappedRead.hEvent != nullptr ) CloseHandle( m_OverlappedRead.hEvent );
    if( m_OverlappedWrite.hEvent != nullptr ) CloseHandle( m_OverlappedWrite.hEvent );
    CloseHandle( m_hIDComDev );
    m_bOpened = FALSE;
    m_hIDComDev = nullptr;

    return( TRUE );
}

int CSerialPortWin::WriteCommByte(unsigned char ucByte)
{
    int bWriteStat;
    DWORD dwBytesWritten;

    bWriteStat = WriteFile( m_hIDComDev, (LPSTR) &ucByte, 1, &dwBytesWritten, &m_OverlappedWrite );
    if( !bWriteStat && ( GetLastError() == ERROR_IO_PENDING ) ){
        if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) dwBytesWritten = 0;
        else{
            GetOverlappedResult( m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE );
            m_OverlappedWrite.Offset += dwBytesWritten;
            }
        }

    return( TRUE );
}

int CSerialPortWin::SendData(const char *buffer,int size)
{
  if( !m_bOpened || m_hIDComDev == nullptr ) return( 0 );

  int bWriteStat;
  DWORD dwBytesWritten;

  bWriteStat = WriteFile( m_hIDComDev, buffer, size, &dwBytesWritten, &m_OverlappedWrite );
  if( !bWriteStat && ( GetLastError() == ERROR_IO_PENDING ) ){
    if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) dwBytesWritten = 0;
    else{
      GetOverlappedResult( m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE );
      m_OverlappedWrite.Offset += dwBytesWritten;
      }
    }

  return( dwBytesWritten );
}

int CSerialPortWin::ReadDataWaiting( void )
{
    if( !m_bOpened || m_hIDComDev == nullptr ) return( 0 );

    DWORD dwErrorFlags;
    COMSTAT ComStat;

    ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );

    return( (int) ComStat.cbInQue );
}

int CSerialPortWin::ReadData(char *buffer,int limit)
{
    if( !m_bOpened || m_hIDComDev == nullptr ) return( 0 );

    int bReadStatus;
    DWORD dwBytesRead, dwErrorFlags;
    COMSTAT ComStat;

    ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );
    if( !ComStat.cbInQue ) return( 0 );

    dwBytesRead = (DWORD) ComStat.cbInQue;
    if( limit < (int) dwBytesRead ) dwBytesRead = (DWORD) limit;

    bReadStatus = ReadFile( m_hIDComDev, buffer, dwBytesRead, &dwBytesRead, &m_OverlappedRead );
    if( !bReadStatus ){
        if( GetLastError() == ERROR_IO_PENDING ){
            WaitForSingleObject( m_OverlappedRead.hEvent, 2000 );
            return( (int) dwBytesRead );
            }
        return( 0 );
        }

    return( (int) dwBytesRead );
}

#endif
