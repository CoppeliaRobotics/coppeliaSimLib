// Linux/Mac part Written by Maciej Szymanski on behalf of Coppelia Robotics GmbH

#include "serial_win_mac_linux.h"
#include "serialPortWin.h"

#ifdef WIN_VREP
#include <vector>

static std::vector<CSerialPortWin*> allOpenedSerialPorts;

CSerialPortWin * _getOpenedPortFromHandle(int portHandle)
{
    for (size_t i=0;i<allOpenedSerialPorts.size();i++)
    {
        if (allOpenedSerialPorts[i]->getPortHandle()==portHandle)
            return(allOpenedSerialPorts[i]);
    }
    return(nullptr);
}

int serialOpen(const char *portName, int baudrate)
{
    CSerialPortWin* p=new CSerialPortWin();
    if (p->Open(portName,baudrate)!=0)
    {
        allOpenedSerialPorts.push_back(p);
        return(p->getPortHandle());
    }
    delete p;
    return(-1);
}

char serialClose(int portHandle)
{
    for (size_t i=0;i<allOpenedSerialPorts.size();i++)
    {
        if (allOpenedSerialPorts[i]->getPortHandle()==portHandle)
        {
            CSerialPortWin* p=allOpenedSerialPorts[i];
            p->Close();
            delete p;
            allOpenedSerialPorts.erase(allOpenedSerialPorts.begin()+i);
            return(0); // ok
        }
    }
    return(1);
}

int serialWrite(int portHandle, const char* buffer, int size)
{
    CSerialPortWin* p=_getOpenedPortFromHandle(portHandle);
    if (p==nullptr)
        return(0);
    return(p->SendData(buffer,size));
}

int serialCheck(int portHandle)
{
    CSerialPortWin* p=_getOpenedPortFromHandle(portHandle);
    if (p==nullptr)
        return(0);
    return(p->ReadDataWaiting());
}

int serialRead(int portHandle, char *buffer, int maxSize)
{
    CSerialPortWin* p=_getOpenedPortFromHandle(portHandle);
    if (p==nullptr)
        return(0);
    return(p->ReadData(buffer,maxSize));
}

void serialCleanUp(void)
{
    for (size_t i=0;i<allOpenedSerialPorts.size();i++)
        delete allOpenedSerialPorts[i];
}

#else
/* ------------------------------ Includes -------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <map>
#include <string>

/* -------------------------------- Types --------------------------------- */

typedef enum serial_result_e
{
  SERIAL_RESULT_OK = 0,
  SERIAL_RESULT_GENERAL_ERROR = -1,
  SERIAL_RESULT_ALREADY_OPENED = -2,
  SERIAL_RESULT_CANNOT_OPEN = -3,
  SERIAL_RESULT_INVALID_HANDLE = -4,
  SERIAL_RESULT_INVALID_PARAMETER = -5,
  SERIAL_RESULT_READ_TIMEOUT = -6
} serial_result_t;

/* ----------------------------- Structures ------------------------------- */

typedef struct serial_struct_s
{
  int             fd;
  std::string     name;
} serial_struct_t;

/* --------------------------- Local routines ----------------------------- */

static std::map<int, serial_struct_t *> serial_map;
static pthread_mutex_t                  serial_map_mutex;
static int                              serial_counter = 1;

/* --------------------------- Implementation ----------------------------- */

/**
 * Converts serial_result_t to given type
 *
 * @param T type to convert to
 * @param res serial_result_t value
 *
 * @return T type value
 */
template<typename T> T convert(serial_result_t res)
{
  return (T) res;
}

/**
 * Converts baudrate given in kilobits to speed_t
 *
 * @param baudrate a baudrate to convert
 * @param speed an out speed_t variable
 *
 * @result true if conversion succeed, false otherwise
 */
bool convert(int baudrate, speed_t & speed)
{
  bool result = true;
  switch(baudrate)
  {
    case 50:      speed = B50; break;
    case 75:      speed = B75; break;
    case 110:     speed = B110; break;
    case 134:     speed = B134; break;
    case 150:     speed = B150; break;
    case 200:     speed = B200; break;
    case 300:     speed = B300; break;
    case 600:     speed = B600; break;
    case 1200:    speed = B1200; break;
    case 1800:    speed = B1800; break;
    case 2400:    speed = B2400; break;
    case 4800:    speed = B4800; break;
    case 9600:    speed = B9600; break;
    case 19200:   speed = B19200; break;
    case 38400:   speed = B38400; break;
    case 57600:   speed = B57600; break;
    case 115200:  speed = B115200; break;
    case 230400:  speed = B230400; break;
    default:    result = false;
  }

  return result;
}

/**
 * Closes all opened handles
 */
static void cleanup(void)
{
  while (serial_map.size() > 0)
  {
    std::map<int, serial_struct_t *>::iterator it = serial_map.begin();
    close(it->second->fd);
    delete it->second;
    serial_map.erase(it);
  }
}

/*
 * Library contructor, initializes library mutex
 */
void __attribute__ ((constructor)) serial_constructor(void)
{
  pthread_mutex_init(&serial_map_mutex, nullptr);
}

/*
 * Library destructor, releases all opened handles
 * and deinitializes library mutex
 */
void __attribute__ ((destructor)) serial_destructor(void)
{
  cleanup();
  pthread_mutex_destroy(&serial_map_mutex);
}

/*
 * Checks if given serial port path exists in main map
 *
 * @param name a serial port name with path
 *
 * @return true if given name exists in main map, false otherwise
 */
static bool in_map(const char *name)
{
  bool result = false;

  for (std::map<int, serial_struct_t *>::const_iterator it = serial_map.begin(); it != serial_map.end(); it++)
  {
    if (strcmp(it->second->name.c_str(), name) == 0)
    {
      result = true;
      break;
    }
  }

  return result;
}

/**
 * Gets serial_struct_t pointer for given Handle
 *
 * @param a serial port handle
 *
 * @result a pointer to serial_port structure or nullptr when invalid handle
 */
static serial_struct_t *get_handle(int portHandle)
{
  std::map<int, serial_struct_t *>::const_iterator it = serial_map.find(portHandle);
  return (it != serial_map.end() ? (serial_struct_t *) it->second : nullptr);
}

/* --------------------------------- API ---------------------------------- */

/**
 * Serial port open function
 *
 * @param portName a serial port node, f.e. /dev/ttyS0
 * @param baudrate a serial port baudrate (in kilobits)
 *
 * @return serial port handle (posistive value) or error code (negative value)
 */
int serialOpen(const char *portName, int baudrate)
{
  int result = convert<int>(SERIAL_RESULT_GENERAL_ERROR);
  pthread_mutex_lock(&serial_map_mutex);
  speed_t speed = B0;

  if (convert(baudrate, speed) == false)
  {
    result = convert<int>(SERIAL_RESULT_INVALID_PARAMETER);
  }
  else if (in_map(portName) == false)
  {
    serial_struct_t *s_ptr = new serial_struct_t;

    if (s_ptr != nullptr)
    {
      s_ptr->name = std::string(portName);

      if ((s_ptr->fd = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK)) != -1)
      {
        struct termios options;
        fcntl(s_ptr->fd, F_SETFL, 0);
        memset(&options, 0, sizeof(struct termios));

        if (tcgetattr(s_ptr->fd, &options) == 0)
        {
          //Set serial port baudrate
          cfsetispeed(&options, speed);
          cfsetospeed(&options, speed);

          //Set serial port settings
          options.c_cflag &= ~PARENB; /* Clear parity enable */
          options.c_cflag &= ~CSTOPB; /* Clear stop bit */
          options.c_cflag &= ~CSIZE;  /* Mask the character size bits */
          options.c_cflag |= CS8;     /* Select 8 data bits */

          /* Enable the receiver and set local mode */
          options.c_cflag |= (CLOCAL | CREAD);

          //Set serial port timeout to 1 second
          options.c_cc[VMIN]  = 0;
          options.c_cc[VTIME] = 10;

          tcflush(s_ptr->fd, TCIFLUSH);

          if (tcsetattr(s_ptr->fd, TCSANOW, &options) == 0)
          {
            serial_map[serial_counter] = s_ptr;
            result = serial_counter;
            serial_counter++;
          }
          else
          {
            close(s_ptr->fd);
            delete s_ptr;
          }
        }
        else
        {
          close(s_ptr->fd);
          delete s_ptr;
        }
      }
      else
      {
        delete s_ptr;
        result = convert<int>(SERIAL_RESULT_CANNOT_OPEN);
      }
    }
  }
  else
  {
    result = convert<int>(SERIAL_RESULT_ALREADY_OPENED);
  }

  pthread_mutex_unlock(&serial_map_mutex);

  return result;
}

/**
 * Closes a given serial port
 *
 * @param portHandle a handle to serial port to be closed
 *
 * @return 0 on success, error code (negative value) otherwise
 */
char serialClose(int portHandle)
{
  char result = convert<char>(SERIAL_RESULT_INVALID_HANDLE);
  pthread_mutex_lock(&serial_map_mutex);

  std::map<int, serial_struct_t *>::iterator it = serial_map.find(portHandle);

  if (it != serial_map.end())
  {
    close(it->second->fd);
    delete it->second;
    serial_map.erase(it);
    result = convert<char>(SERIAL_RESULT_OK);
  }

  pthread_mutex_unlock(&serial_map_mutex);
  return result;
}

/*
 * writes data to an opened port. Returns the number of written bytes, or a negative number in case of an error
 */
/**
 * Writes data to a given serial port
 *
 * @param portHandle a handle to serial port
 * @param buffer a pointer to data buffer to write
 * @param size a size of the data buffer to write
 *
 * @return number of successfully written bytes or error code (negative value)
 */
int serialWrite(int portHandle, const char* buffer, int size)
{
  int result = convert<int>(SERIAL_RESULT_INVALID_HANDLE);
  serial_struct_t *s_ptr = nullptr;
  pthread_mutex_lock(&serial_map_mutex);

  if ((buffer == nullptr) || (size == 0))
  {
    result = convert<int>(SERIAL_RESULT_INVALID_PARAMETER);
  }
  else if ((s_ptr = get_handle(portHandle)) != nullptr)
  {
    result = (int) write(s_ptr->fd, buffer, size);
  }

  pthread_mutex_unlock(&serial_map_mutex);
  return result;
}

/**
 * Checks if serial port buffer contains available data to read
 *
 * @param portHandle a handle to serial port
 *
 * @return a number of available bytes to read or error code (negative value)
 */
int serialCheck(int portHandle)
{
  int result = convert<int>(SERIAL_RESULT_INVALID_HANDLE);
  serial_struct_t *s_ptr = nullptr;
  pthread_mutex_lock(&serial_map_mutex);

  if ((s_ptr = get_handle(portHandle)) != nullptr)
  {
    int ncharin = 0;
    if (ioctl(s_ptr->fd, FIONREAD, &ncharin) == -1)
    {
      result = convert<int>(SERIAL_RESULT_GENERAL_ERROR);
    }
    else
    {
      result = ncharin;
    }
  }

  pthread_mutex_unlock(&serial_map_mutex);
  return result;
}

/**
 * Reads a data from serial port (if available).
 * Function is blocking for max 1 second.
 *
 * @param portHandle a handle to serial port
 * @param buffer a pointer to read buffer (to which data will be written)
 * @param maxSize a maximal number of bytes to read
 *
 * @return a number of successfully read bytes or error code (negative value)
 */
int serialRead(int portHandle, char *buffer, int maxSize)
{
  int result = convert<int>(SERIAL_RESULT_INVALID_HANDLE);
  serial_struct_t *s_ptr = nullptr;
  pthread_mutex_lock(&serial_map_mutex);

  if ((buffer == nullptr) || (maxSize == 0))
  {
    result = convert<int>(SERIAL_RESULT_INVALID_PARAMETER);
  }
  else if ((s_ptr = get_handle(portHandle)) != nullptr)
  {
    result = (int) read(s_ptr->fd, buffer, maxSize);
  }

  pthread_mutex_unlock(&serial_map_mutex);

  return result;
}

/**
 * Closes all opened serial ports
 */
void serialCleanUp(void)
{
  pthread_mutex_lock(&serial_map_mutex);
  cleanup();
  pthread_mutex_unlock(&serial_map_mutex);
}

#endif
