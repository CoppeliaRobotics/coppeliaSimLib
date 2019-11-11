
#include "simQApp.h"
#include "app.h"

CSimQApp::CSimQApp(int& argc ,char** argv) : QAPP(argc,argv)
{
}

CSimQApp::~CSimQApp()
{
    printf("*");
}

bool CSimQApp::notify(QObject* object,QEvent* event)
{
#ifdef SIM_WITH_GUI
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent* keyEvent=static_cast<QKeyEvent*>(event);
        int key=keyEvent->key();
        if (key==Qt::Key_Escape)
            App::setFullScreen(false);
    }
#endif
    return(QAPP::notify(object,event));
};
