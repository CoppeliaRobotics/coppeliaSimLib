
#include "simInternal.h"
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
        std::vector<CSceneObject*> cameraProxies;
        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getCameraCount();cnt++)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromIndex(cnt);
            SCamBuff buff;
            buff.localTr=cam->getFullLocalTransformation();
            buff.orthoViewSize=cam->getOrthoViewSize();
            _preRestoreCameraBuffers[cam->getObjectName()]=buff;
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParent()!=nullptr))
            {
                bool present=false;
                for (size_t i=0;i<cameraProxies.size();i++)
                {
                    if (cameraProxies[i]==cam->getParent())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParent());
            }
        }

        // now camera proxies:
        for (int cnt=0;cnt<int(cameraProxies.size());cnt++)
        {
            SCamBuff buff;
            buff.localTr=cameraProxies[cnt]->getFullLocalTransformation();
            _preRestoreCameraProxyBuffers[cameraProxies[cnt]->getObjectName()]=buff;
        }
    }
}

void CUndoBufferCameras::restoreCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        // this is the saved cameras, might be a while back:
        std::vector<CSceneObject*> cameraProxies;
        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getCameraCount();cnt++)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromIndex(cnt);
            std::map<std::string,SCamBuff>::iterator it=_cameraBuffers.find(cam->getObjectName());
            if (it!=_cameraBuffers.end())
            {
                cam->setLocalTransformation(it->second.localTr);
                cam->setOrthoViewSize(it->second.orthoViewSize);
            }
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParent()!=nullptr))
            {
                bool present=false;
                for (size_t i=0;i<cameraProxies.size();i++)
                {
                    if (cameraProxies[i]==cam->getParent())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParent());
            }
        }

        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getObjectCount();cnt++)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromIndex(cnt);
            std::map<std::string,SCamBuff>::iterator it=_cameraProxyBuffers.find(obj->getObjectName());
            if (it!=_cameraProxyBuffers.end())
                obj->setLocalTransformation(it->second.localTr);
        }


        // this is the saved cameras, just a moment ago:
        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getCameraCount();cnt++)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromIndex(cnt);
            std::map<std::string,SCamBuff>::iterator it=_preRestoreCameraBuffers.find(cam->getObjectName());
            if (it!=_preRestoreCameraBuffers.end())
            {
                cam->setLocalTransformation(it->second.localTr);
                cam->setOrthoViewSize(it->second.orthoViewSize);
            }
        }
        _preRestoreCameraBuffers.clear();

        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getObjectCount();cnt++)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromIndex(cnt);
            std::map<std::string,SCamBuff>::iterator it=_preRestoreCameraProxyBuffers.find(obj->getObjectName());
            if (it!=_preRestoreCameraProxyBuffers.end())
                obj->setLocalTransformation(it->second.localTr);
        }
        _preRestoreCameraProxyBuffers.clear();

    }
}

void CUndoBufferCameras::storeCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        std::vector<CSceneObject*> cameraProxies;
        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getCameraCount();cnt++)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromIndex(cnt);
            /*
            SCamBuff buff;
            buff.localTr=cam->getFullLocalTransformation();
            buff.orthoViewSize=cam->getOrthoViewSize();
            _cameraBuffers[cam->getObjectName()]=buff;
            C7Vector tr;
            tr.setIdentity();
            cam->setLocalTransformation(tr);
            cam->setOrthoViewSize(1.0f);
            */
            cam->setIgnorePosAndCameraOrthoviewSize_forUndoRedo(true);
            if ((cam->getUseParentObjectAsManipulationProxy())&&(cam->getParent()!=nullptr))
            {
                bool present=false;
                for (size_t i=0;i<cameraProxies.size();i++)
                {
                    if (cameraProxies[i]==cam->getParent())
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    cameraProxies.push_back(cam->getParent());
            }
        }

        // now camera proxies:
        for (size_t cnt=0;cnt<cameraProxies.size();cnt++)
        {
            /*
            SCamBuff buff;
            buff.localTr=cameraProxies[cnt]->getFullLocalTransformation();
            _cameraProxyBuffers[cameraProxies[cnt]->getObjectName()]=buff;
            C7Vector tr;
            tr.setIdentity();
            cameraProxies[cnt]->setLocalTransformation(tr);
            */
            cameraProxies[cnt]->setIgnorePosAndCameraOrthoviewSize_forUndoRedo(true);
        }
    }
}

void CUndoBufferCameras::releaseCameras()
{
    if (App::userSettings->getUndoRedoOnlyPartialWithCameras())
    {
        for (size_t cnt=0;cnt<App::currentWorld->sceneObjects->getObjectCount();cnt++)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromIndex(cnt);
            obj->setIgnorePosAndCameraOrthoviewSize_forUndoRedo(false);
        }
    }
}

