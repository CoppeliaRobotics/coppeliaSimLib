#ifndef __SERIAL_WIN_MAC_LINUX_H__
#define __SERIAL_WIN_MAC_LINUX_H__

int serialOpen(const char *,int baudrate);
char serialClose(int portHandle);
int serialWrite(int portHandle,const char* buffer,int size);
int serialCheck(int portHandle);
int serialRead(int portHandle,char* buffer,int maxSize);
void serialcleanUp(void);

#endif /* __SERIAL_WIN_MAC_LINUX_H__ */
