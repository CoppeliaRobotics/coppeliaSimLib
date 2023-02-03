#include <qdlgmodelthumbnail.h>
#include <ui_qdlgmodelthumbnail.h>
#include <sceneObjectOperations.h>
#include <imgLoaderSaver.h>
#include <app.h>
#include <simStrings.h>
#include <vFileDialog.h>
#include <vMessageBox.h>

#define THUMBNAIL_THING_VIEW_ANGLE 45.0

CQDlgModelThumbnail::CQDlgModelThumbnail(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgModelThumbnail)
{
    ui->setupUi(this);
    modelBaseDummyID=-1;
    thumbnailIsFromFile=false;
}

CQDlgModelThumbnail::~CQDlgModelThumbnail()
{
    App::currentWorld->sceneObjects->eraseObject(rs,false); // oops, we are in the wrong thread here. Very dangerous
    delete ui;
}

void CQDlgModelThumbnail::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgModelThumbnail::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgModelThumbnail::initialize()
{
    defaultDialogInitializationRoutine();
    rotX=(180.0-45.0)*degToRad;   // With that angle we have the best visualization (depends on lights in scene)
    rotY=30.0*degToRad;
    zoom=1.0;
    shiftX=0.0;
    shiftY=0.0;
    hideEdges=false;
    antialiasing=true;
    ui->qqAntialiasing->setChecked(antialiasing);
    thumbnailIsFromFile=false;

    thumbnail.setRandomImage();

    rs=new CVisionSensor();
    App::currentWorld->sceneObjects->addObjectToScene(rs,false,false); // oops, we are in the wrong thread here. Very dangerous
    int res[2]={256,256};
    rs->setResolution(res);
    rs->setPerspective(true);
    rs->setUseEnvironmentBackgroundColor(false);
    rs->setViewAngle(THUMBNAIL_THING_VIEW_ANGLE*degToRad);
    rs->setNearClippingPlane(0.01);
    rs->setFarClippingPlane(20.0);
    rs->setVisibilityLayer(0);
    sel.clear();
    sel.push_back(modelBaseDummyID);
    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
    actualizeBitmap();
}

void CQDlgModelThumbnail::actualizeBitmap()
{
    C3Vector minC(999.0,999.0,999.0);
    C3Vector maxC(-999.0,-999.0,-999.0);
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        bool display=true;
        if (it->getObjectType()==sim_object_proximitysensor_type)
        {
            if (!((CProxSensor*)it)->getShowVolume())
                display=false;
        }
        if (it->getObjectType()==sim_object_visionsensor_type)
        {
            if (!((CVisionSensor*)it)->getShowVolume())
                display=false;
        }
        if ( (!it->isObjectPartOfInvisibleModel())&&(it->getVisibilityLayer()&App::currentWorld->environment->getActiveLayers())&&display )
        {
            C7Vector tr(it->getCumulativeTransformation());
            C3Vector minV,maxV;
            it->getBoundingBox(minV,maxV);
            tr.X*=(maxV+minV)*0.5;
            C3Vector hs((maxV-minV)*0.5);
            for (double x=-1.0;x<2.0;x+=2.0)
            {
                for (double y=-1.0;y<2.0;y+=2.0)
                {
                    for (double z=-1.0;z<2.0;z+=2.0)
                    {
                        C3Vector w(hs(0)*x,hs(1)*y,hs(2)*z);
                        w*=tr;
                        minC.keepMin(w);
                        maxC.keepMax(w);
                    }
                }
            }
        }
    }
    C3Vector hs((maxC-minC)*0.5);
    C3Vector center((maxC+minC)*0.5);
    C4X4Matrix cameraTr;
    cameraTr.setIdentity();
    cameraTr.M.buildXRotation(piValD2);
    C3X3Matrix rot;
    rot.buildZRotation(rotX);
    cameraTr.M=rot*cameraTr.M;
    rot.buildXRotation(rotY);
    cameraTr.M*=rot;
    cameraTr.X=center;
    double d=-9999.0;
    double maxDist=0.0;
    double minDist=9999.0;
    double maxPerpX=0.0;
    double maxPerpY=0.0;
    for (double x=-1.0;x<2.0;x+=2.0)
    {
        for (double y=-1.0;y<2.0;y+=2.0)
        {
            for (double z=-1.0;z<2.0;z+=2.0)
            {
                C3Vector w(hs(0)*x,hs(1)*y,hs(2)*z);
                w+=center;
                w*=cameraTr.getInverse();
                if (fabs(w(0))>maxPerpX)
                    maxPerpX=fabs(w(0));
                if (fabs(w(1))>maxPerpY)
                    maxPerpY=fabs(w(1));
                double a=fabs(w(0))/tan(THUMBNAIL_THING_VIEW_ANGLE*degToRad*0.5);
                double b=fabs(w(1))/tan(THUMBNAIL_THING_VIEW_ANGLE*degToRad*0.5);
                double c=std::max<double>(a,b)-w(2);
                if (c>d)
                    d=c;
                if (w(2)>maxDist)
                    maxDist=w(2);
                if (w(2)<minDist)
                    minDist=w(2);
            }
        }
    }

    cameraTr.X+=cameraTr.M.axis[2]*-d*zoom;
    cameraTr.X+=cameraTr.M.axis[0]*shiftX*maxPerpX;
    cameraTr.X+=cameraTr.M.axis[1]*shiftY*maxPerpY;

    double dddd=minDist+d*zoom*0.5;
    if (dddd<0.0001)
        dddd=0.0001;
    rs->setNearClippingPlane(dddd);
    rs->setFarClippingPlane(maxDist+d*zoom*2.0);

    rs->setLocalTransformation(cameraTr.getTransformation());
    bool antialiasingSaved=App::userSettings->antiAliasing;
    App::userSettings->antiAliasing=antialiasing;

    // We render the image 2 times, so we can figure out the transparent areas:
    float backgroundCol[3]={1.0,1.0,1.0};
    rs->setDefaultBufferValues(backgroundCol);
    float* buff=rs->checkSensorEx(modelBaseDummyID,true,true,hideEdges,false);
    float backgroundCol2[3]={0.0,0.0,0.0};
    rs->setDefaultBufferValues(backgroundCol2);
    float* buff2=rs->checkSensorEx(modelBaseDummyID,true,true,hideEdges,false);
    App::userSettings->antiAliasing=antialiasingSaved;
    for (int i=0;i<256;i++)
    {
        for (int j=0;j<256;j++)
        {
            C3Vector rgb(buff[3*(i*256+j)+0],buff[3*(i*256+j)+1],buff[3*(i*256+j)+2]);
            C3Vector rgb2(buff2[3*(i*256+j)+0],buff2[3*(i*256+j)+1],buff2[3*(i*256+j)+2]);
            if ((rgb-rgb2).getLength()>1.70) // sqrt(3) is the max difference!
            {
                buff2[3*(i*256+j)+0]=0.0; // means transparent
                buff[3*(i*256+j)+0]=1.0;
                buff[3*(i*256+j)+1]=1.0;
                buff[3*(i*256+j)+2]=1.0;
            }
            else
                buff2[3*(i*256+j)+0]=1.0; // means opaque
        }
    }
    // Prepare the thumbnail:
    float* buff3=new float[128*128*4];

    const float diagWeight=0.05472f;//1.0f;
    const float horizWeight=0.11098f;//2.0f;
    const float centerWeight=0.22508f;//4.0f;
    for (int i=0;i<128;i++)
    {
        for (int j=0;j<128;j++)
        {
            float d=centerWeight;
            float red=buff[3*((i*2*256)+(j*2))+0]*centerWeight;
            float green=buff[3*((i*2*256)+(j*2))+1]*centerWeight;
            float blue=buff[3*((i*2*256)+(j*2))+2]*centerWeight;
            float alpha=buff2[3*((i*2*256)+(j*2))+0]*centerWeight;
            if (j>0)
            {
                d+=horizWeight;
                red+=buff[3*((i*2*256)+(j*2-1))+0]*horizWeight;
                green+=buff[3*((i*2*256)+(j*2-1))+1]*horizWeight;
                blue+=buff[3*((i*2*256)+(j*2-1))+2]*horizWeight;
                alpha+=buff2[3*((i*2*256)+(j*2-1))+0]*horizWeight;

                if (i>0)
                {
                    d+=diagWeight;
                    red+=buff[3*(((i*2-1)*256)+(j*2-1))+0]*diagWeight;
                    green+=buff[3*(((i*2-1)*256)+(j*2-1))+1]*diagWeight;
                    blue+=buff[3*(((i*2-1)*256)+(j*2-1))+2]*diagWeight;
                    alpha+=buff2[3*(((i*2-1)*256)+(j*2-1))+0]*diagWeight;
                }
                if (i<127)
                {
                    d+=diagWeight;
                    red+=buff[3*(((i*2+1)*256)+(j*2-1))+0]*diagWeight;
                    green+=buff[3*(((i*2+1)*256)+(j*2-1))+1]*diagWeight;
                    blue+=buff[3*(((i*2+1)*256)+(j*2-1))+2]*diagWeight;
                    alpha+=buff2[3*(((i*2+1)*256)+(j*2-1))+0]*diagWeight;
                }
            }
            if (j<127)
            {
                d+=horizWeight;
                red+=buff[3*((i*2*256)+(j*2+1))+0]*horizWeight;
                green+=buff[3*((i*2*256)+(j*2+1))+1]*horizWeight;
                blue+=buff[3*((i*2*256)+(j*2+1))+2]*horizWeight;
                alpha+=buff2[3*((i*2*256)+(j*2+1))+0]*horizWeight;

                if (i>0)
                {
                    d+=diagWeight;
                    red+=buff[3*(((i*2-1)*256)+(j*2+1))+0]*diagWeight;
                    green+=buff[3*(((i*2-1)*256)+(j*2+1))+1]*diagWeight;
                    blue+=buff[3*(((i*2-1)*256)+(j*2+1))+2]*diagWeight;
                    alpha+=buff2[3*(((i*2-1)*256)+(j*2+1))+0]*diagWeight;
                }
                if (i<127)
                {
                    d+=diagWeight;
                    red+=buff[3*(((i*2+1)*256)+(j*2+1))+0]*diagWeight;
                    green+=buff[3*(((i*2+1)*256)+(j*2+1))+1]*diagWeight;
                    blue+=buff[3*(((i*2+1)*256)+(j*2+1))+2]*diagWeight;
                    alpha+=buff2[3*(((i*2+1)*256)+(j*2+1))+0]*diagWeight;
                }
            }
            if (i>0)
            {
                d+=horizWeight;
                red+=buff[3*(((i*2-1)*256)+(j*2))+0]*horizWeight;
                green+=buff[3*(((i*2-1)*256)+(j*2))+1]*horizWeight;
                blue+=buff[3*(((i*2-1)*256)+(j*2))+2]*horizWeight;
                alpha+=buff2[3*(((i*2-1)*256)+(j*2))+0]*horizWeight;
            }
            if (i<127)
            {
                d+=horizWeight;
                red+=buff[3*(((i*2+1)*256)+(j*2))+0]*horizWeight;
                green+=buff[3*(((i*2+1)*256)+(j*2))+1]*horizWeight;
                blue+=buff[3*(((i*2+1)*256)+(j*2))+2]*horizWeight;
                alpha+=buff2[3*(((i*2+1)*256)+(j*2))+0]*horizWeight;
            }
            buff3[4*((i*128)+j)+0]=red/d;
            buff3[4*((i*128)+j)+1]=green/d;
            buff3[4*((i*128)+j)+2]=blue/d;
            buff3[4*((i*128)+j)+3]=alpha/d;
        }
    }

    QImage img(128,128,QImage::Format_RGB32);
    QRgb value;

    for (int i=0;i<128;i++)
    {
        for (int j=0;j<128;j++)
        {
            unsigned char r=(unsigned char)(buff3[4*((127-i)*128+j)+0]*255.1);
            unsigned char g=(unsigned char)(buff3[4*((127-i)*128+j)+1]*255.1);
            unsigned char b=(unsigned char)(buff3[4*((127-i)*128+j)+2]*255.1);
            value=qRgb(r,g,b);
            img.setPixel(j,i,value);
        }
    }

    QPixmap pimg;
    pimg.convertFromImage(img);
    ui->qqImage->setPixmap(pimg);

    thumbnail.setUncompressedThumbnailImageFromFloat(buff3);
    delete[] buff;
    delete[] buff2;
    delete[] buff3;
}

void CQDlgModelThumbnail::on_qqFromFile_clicked()
{
    std::string tst(App::folders->getTexturesPath());
    std::string filenameAndPath=App::uiThread->getOpenFileName(this,0,"Loading texture...",tst.c_str(),"",true,"Image files","tga","jpg","jpeg","png","gif","bmp","tiff");
    if (filenameAndPath.length()!=0)
    {
        if (VFile::doesFileExist(filenameAndPath.c_str()))
        {
            App::appendSimulationThreadCommand(SET_CURRENTDIRECTORY_GUITRIGGEREDCMD,DIRECTORY_ID_TEXTURE,-1,0.0,0.0,App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
            int resX,resY,n;
            unsigned char* data=CImageLoaderSaver::load(filenameAndPath.c_str(),&resX,&resY,&n,0);
            if (n<3)
            {
                delete[] data;
                data=nullptr;
            }

            if (data==nullptr)
                App::uiThread->messageBox_critical(App::mainWindow,"Thumbnail",IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            else
            {
                if ( (resX!=128)||(resY!=128) )
                    App::uiThread->messageBox_critical(App::mainWindow,"Thumbnail",IDS_THUMBNAILS_REQUIRE_128X128_RESOLUTION,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                else
                {
                    thumbnail.setUncompressedThumbnailImage((char*)data,n>3,true);

                    delete[] data;
                    thumbnailIsFromFile=true;
                    defaultModalDialogEndRoutine(true);
                    return;
                }
                delete[] data;
            }
        }
    }
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqZoomP_clicked()
{
    zoom/=1.03;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqZoomM_clicked()
{
    zoom*=1.03;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqVShiftP_clicked()
{
    shiftY-=0.025;
    if (shiftY>1.5)
        shiftY=1.5;
    if (shiftY<-1.5)
        shiftY=-1.5;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqVShiftM_clicked()
{
    shiftY+=0.025;
    if (shiftY>1.5)
        shiftY=1.5;
    if (shiftY<-1.5)
        shiftY=-1.5;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqVRotP_clicked()
{
    rotY-=7.5*degToRad;
    if (rotY>90.0*degToRad)
        rotY=90.0*degToRad;
    if (rotY<-90.0*degToRad)
        rotY=-90.0*degToRad;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqVRotM_clicked()
{
    rotY+=7.5*degToRad;
    if (rotY>90.0*degToRad)
        rotY=90.0*degToRad;
    if (rotY<-90.0*degToRad)
        rotY=-90.0*degToRad;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqHShiftM_clicked()
{
    shiftX-=0.025;
    if (shiftX>1.5)
        shiftX=1.5;
    if (shiftX<-1.5)
        shiftX=-1.5;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqHShiftP_clicked()
{
    shiftX+=0.025;
    if (shiftX>1.5)
        shiftX=1.5;
    if (shiftX<-1.5)
        shiftX=-1.5;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqHRotM_clicked()
{
    rotX+=7.5*degToRad;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqHRotP_clicked()
{
    rotX-=7.5*degToRad;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgModelThumbnail::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgModelThumbnail::on_qqHideEdges_clicked(bool checked)
{
    hideEdges=checked;
    actualizeBitmap();
}

void CQDlgModelThumbnail::on_qqAntialiasing_clicked(bool checked)
{
    antialiasing=checked;
    actualizeBitmap();
}
