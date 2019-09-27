
#pragma once

class CViewableBase;

class CMainCont  
{
public:
    CMainCont();
    virtual ~CMainCont();
    virtual void simulationAboutToStart();
    virtual void simulationEnded();
    virtual void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    virtual void simulationAboutToStart_guiDelayed();
    virtual void simulationEnded_guiDelayed();

};
