
#include "simQApp.h"
#include "app.h"

CSimQApp::CSimQApp(int& argc ,char** argv) : QAPP(argc,argv)
{
}

CSimQApp::~CSimQApp()
{
    App::logMsg(sim_verbosity_loadinfos,"0");
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
