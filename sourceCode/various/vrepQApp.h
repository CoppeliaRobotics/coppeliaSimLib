
#pragma once

#ifdef SIM_WITH_GUI
#include <QApplication>
#define QAPP QApplication
#else
#include <QCoreApplication>
#define QAPP QCoreApplication
#endif

class CVrepQApp : public QAPP
{
public:
    CVrepQApp(int& argc ,char** argv);
    virtual ~CVrepQApp();

    bool notify(QObject* object,QEvent* event);
};
