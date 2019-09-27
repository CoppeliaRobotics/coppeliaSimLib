
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "undoBufferCameras.h"
#include "app.h"


CUndoBufferCameras::CUndoBufferCameras()
{
}

CUndoBufferCameras::~CUndoBufferCameras()
{
}

void CUndoBufferCameras::preRestoreCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        std::vector<C3DObject*> cameraProxies;
        for (int cnt=0;cnt<int(App::ct->objCont->cameraList.size());cnt++)
        {
            CCamera* cam=App::ct->objCont->getCamera(App::ct->objCont->cameraList[cnt]);
            SCamBuff buff;
            buff.localTr=cam->getLocalTransformation();
            buff.orthoViewSize=cam->getOrthoViewSize();
            _preRestoreCameraBuffers[cam->getObjectName()]=buff;
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParentObject()!=nullptr))
            {
                bool present=false;
                for (int i=0;i<int(cameraProxies.size());i++)
                {
                    if (cameraProxies[i]==cam->getParentObject())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParentObject());
            }
        }

        // now camera proxies:
        for (int cnt=0;cnt<int(cameraProxies.size());cnt++)
        {
            SCamBuff buff;
            buff.localTr=cameraProxies[cnt]->getLocalTransformation();
            _preRestoreCameraProxyBuffers[cameraProxies[cnt]->getObjectName()]=buff;
        }
    }
}

void CUndoBufferCameras::restoreCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        // this is the saved cameras, might be a while back:
        std::vector<C3DObject*> cameraProxies;
        for (int cnt=0;cnt<int(App::ct->objCont->cameraList.size());cnt++)
        {
            CCamera* cam=App::ct->objCont->getCamera(App::ct->objCont->cameraList[cnt]);
            std::map<std::string,SCamBuff>::iterator it=_cameraBuffers.find(cam->getObjectName());
            if (it!=_cameraBuffers.end())
            {
                cam->setLocalTransformation(it->second.localTr);
                cam->setOrthoViewSize(it->second.orthoViewSize);
                cam->bufferMainDisplayStateVariables();
                cam->bufferedMainDisplayStateVariablesToDisplay();
            }
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParentObject()!=nullptr))
            {
                bool present=false;
                for (int i=0;i<int(cameraProxies.size());i++)
                {
                    if (cameraProxies[i]==cam->getParentObject())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParentObject());
            }
        }

        for (int cnt=0;cnt<int(App::ct->objCont->objectList.size());cnt++)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[cnt]);
            std::map<std::string,SCamBuff>::iterator it=_cameraProxyBuffers.find(obj->getObjectName());
            if (it!=_cameraProxyBuffers.end())
            {
                obj->setLocalTransformation(it->second.localTr);
                obj->bufferMainDisplayStateVariables();
                obj->bufferedMainDisplayStateVariablesToDisplay();
            }
        }


        // this is the saved cameras, just a moment ago:
        for (int cnt=0;cnt<int(App::ct->objCont->cameraList.size());cnt++)
        {
            CCamera* cam=App::ct->objCont->getCamera(App::ct->objCont->cameraList[cnt]);
            std::map<std::string,SCamBuff>::iterator it=_preRestoreCameraBuffers.find(cam->getObjectName());
            if (it!=_preRestoreCameraBuffers.end())
            {
                cam->setLocalTransformation(it->second.localTr);
                cam->setOrthoViewSize(it->second.orthoViewSize);
                cam->bufferMainDisplayStateVariables();
                cam->bufferedMainDisplayStateVariablesToDisplay();
            }
        }
        _preRestoreCameraBuffers.clear();

        for (int cnt=0;cnt<int(App::ct->objCont->objectList.size());cnt++)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[cnt]);
            std::map<std::string,SCamBuff>::iterator it=_preRestoreCameraProxyBuffers.find(obj->getObjectName());
            if (it!=_preRestoreCameraProxyBuffers.end())
            {
                obj->setLocalTransformation(it->second.localTr);
                obj->bufferMainDisplayStateVariables();
                obj->bufferedMainDisplayStateVariablesToDisplay();
            }
        }
        _preRestoreCameraProxyBuffers.clear();

    }
}

void CUndoBufferCameras::storeCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        std::vector<C3DObject*> cameraProxies;
        for (int cnt=0;cnt<int(App::ct->objCont->cameraList.size());cnt++)
        {
            CCamera* cam=App::ct->objCont->getCamera(App::ct->objCont->cameraList[cnt]);
            SCamBuff buff;
            buff.localTr=cam->getLocalTransformation();
            buff.orthoViewSize=cam->getOrthoViewSize();
            _cameraBuffers[cam->getObjectName()]=buff;
            C7Vector tr;
            tr.setIdentity();
            cam->setLocalTransformation(tr);
            cam->setOrthoViewSize(1.0f);
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParentObject()!=nullptr))
            {
                bool present=false;
                for (int i=0;i<int(cameraProxies.size());i++)
                {
                    if (cameraProxies[i]==cam->getParentObject())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParentObject());
            }
        }

        // now camera proxies:
        for (int cnt=0;cnt<int(cameraProxies.size());cnt++)
        {
            SCamBuff buff;
            buff.localTr=cameraProxies[cnt]->getLocalTransformation();
            _cameraProxyBuffers[cameraProxies[cnt]->getObjectName()]=buff;
            C7Vector tr;
            tr.setIdentity();
            cameraProxies[cnt]->setLocalTransformation(tr);
        }
    }
}

