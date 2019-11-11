
#pragma once

#ifdef SIM_WITH_GUI
#include <QApplication>
#define QAPP QApplication
#else
#include <QCoreApplication>
#define QAPP QCoreApplication
#endif

class CSimQApp : public QAPP
{
public:
    CSimQApp(int& argc ,char** argv);
    virtual ~CSimQApp();

    bool notify(QObject* object,QEvent* event);
};
