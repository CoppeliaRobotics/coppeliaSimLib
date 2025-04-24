#pragma once

#ifdef SIM_WITH_GUI
#include <QApplication>
#define QAPP QApplication
#else
#include <QCoreApplication>
#define QAPP QCoreApplication
#endif
#include <QKeyEvent>

class CSimQApp : public QAPP
{
  public:
    CSimQApp(int& argc, char** argv);
    virtual ~CSimQApp();

    bool notify(QObject* object, QEvent* event);
};

class QuitKeyInterceptor : public QObject
{
public:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
#ifdef Q_OS_MAC
            bool isCmdQ = (keyEvent->modifiers() & Qt::MetaModifier) && keyEvent->key() == Qt::Key_Q;
#else
            bool isCmdQ = (keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_Q;
#endif
            if (isCmdQ)
            {
                QCoreApplication::quit();
                return true; // prevent dialog from handling it
            }
        }
        return QObject::eventFilter(obj, event);
    }

    static void install()
    {
        static bool installed = false;
        if(installed) return;
        QuitKeyInterceptor* interceptor = new QuitKeyInterceptor();
        qApp->installEventFilter(interceptor);
        installed = true;
    }
};
