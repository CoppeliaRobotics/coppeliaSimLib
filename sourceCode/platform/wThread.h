#pragma once

#include <QThread>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread();
    virtual ~Thread();
    static unsigned int(*startAddress)(void*);
    static volatile bool startAddressIsFree;

private slots:
    void threadFinished();

protected:
    void run();
};
