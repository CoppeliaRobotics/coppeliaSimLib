
#include "vrepMainHeader.h"
#include "imgLoaderSaver.h"
#ifdef SIM_WITH_GUI
    #include "tGAFormat.h"
    #include "stb_image.h"
    #include "ttUtil.h"
    #include "vVarious.h"
    #include <QImage>
    #include <QImageWriter>
    #include <QColor>
    #include <QtCore/QBuffer>
#endif

unsigned char* CImageLoaderSaver::load(const char* filename,int* resX,int* resY,int* colorComponents,int desiredColorComponents,int scaleTo)
{ // scaleTo is 0 by default (no scaling). ScaleTo should be a power of 2!
#ifdef SIM_WITH_GUI
    std::string ext(CTTUtil::getLowerCaseString(VVarious::splitPath_fileExtension(filename).c_str()));
    unsigned char* data=nullptr;
    if ((ext.compare("tga")==0)||(ext.compare("gif")==0))
    {
        data=stbi_load(filename,resX,resY,colorComponents,desiredColorComponents);
    }
    else
    {
        QImage image;
        if (image.load(filename))
        {
            if (image.hasAlphaChannel())
                colorComponents[0]=4;
            else
                colorComponents[0]=3;
            resX[0]=image.width();
            resY[0]=image.height();
            data=new unsigned char[resX[0]*resY[0]*colorComponents[0]];
            for (int j=0;j<resY[0];j++)
            {
                for (int i=0;i<resX[0];i++)
                {
                    QRgb pixel=image.pixel(i,j);
                    if (colorComponents[0]==3)
                    {
                        data[3*(j*resX[0]+i)+0]=(unsigned char)qRed(pixel);
                        data[3*(j*resX[0]+i)+1]=(unsigned char)qGreen(pixel);
                        data[3*(j*resX[0]+i)+2]=(unsigned char)qBlue(pixel);
                    }
                    else
                    {
                        data[4*(j*resX[0]+i)+0]=(unsigned char)qRed(pixel);
                        data[4*(j*resX[0]+i)+1]=(unsigned char)qGreen(pixel);
                        data[4*(j*resX[0]+i)+2]=(unsigned char)qBlue(pixel);
                        data[4*(j*resX[0]+i)+3]=(unsigned char)qAlpha(pixel);
                    }
                }
            }
        }
    }
    if ((scaleTo!=0)&&(data!=nullptr))
    {
        unsigned char* img=data;
        int s[2];
        s[0]=resX[0];
        s[1]=resY[0];
        int ns[2];
        ns[0]=resX[0];
        ns[1]=resY[0];
        for (int i=0;i<2;i++)
        { // set the side size to a power of 2 and smaller or equal to 'scaleTo':
            int v=s[i];
            v&=(32768-1);
            unsigned short tmp=32768;
            while (tmp!=1)
            {
                if (v&tmp)
                {
                    v=tmp;
                    break;
                }
                tmp/=2;
            }
            if (v!=s[i])
                v=v*2;
            while (v>scaleTo)
                v/=2;
            ns[i]=v;
        }
        if ((ns[0]!=s[0])||(ns[1]!=s[1]))
        {
            data=getScaledImage(img,colorComponents[0],s[0],s[1],ns[0],ns[1]);
            delete[] img;
            resX[0]=ns[0];
            resY[0]=ns[1];
        }
    }
    return(data);
#else
    return(nullptr);
#endif
}

unsigned char* CImageLoaderSaver::loadQTgaImageData(const char* fileAndPath,int& resX,int& resY,bool& rgba,unsigned char invisibleColor[3],int bitsPerPixel[1])
{
#ifdef SIM_WITH_GUI
    unsigned char* data=CTGAFormat::getQ_ImageData(fileAndPath,resX,resY,rgba,invisibleColor,bitsPerPixel);
    return(data);
#else
    return(nullptr);
#endif
}

unsigned char* CImageLoaderSaver::getScaledImage(const unsigned char* originalImg,int colorComponents,int originalX,int originalY,int newX,int newY)
{
#ifdef SIM_WITH_GUI
    QImage::Format f=QImage::Format_RGB888;
    unsigned char* im=new unsigned char[originalX*originalY*colorComponents];
    if (colorComponents==4)
    {
        f=QImage::Format_ARGB32;
        for (int i=0;i<originalX*originalY;i++)
        { // from rgba to bgra (corrected on 9/9/2914)
            im[4*i+0]=originalImg[4*i+2];
            im[4*i+1]=originalImg[4*i+1];
            im[4*i+2]=originalImg[4*i+0];
            im[4*i+3]=originalImg[4*i+3];
        }
    }
    else
    {
        for (int i=0;i<originalX*originalY;i++)
        {
            im[3*i+0]=originalImg[3*i+0];
            im[3*i+1]=originalImg[3*i+1];
            im[3*i+2]=originalImg[3*i+2];
        }
    }
    QImage image(im,originalX,originalY,originalX*colorComponents,f);
    unsigned char* nim=new unsigned char[newX*newY*colorComponents];
    QImage nimage(image.scaled(newX,newY,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

    for (int j=0;j<newY;j++)
    {
        for (int i=0;i<newX;i++)
        {
            QRgb pixel=nimage.pixel(i,j);
            if (colorComponents==3)
            {
                nim[3*(j*newX+i)+0]=(unsigned char)qRed(pixel);
                nim[3*(j*newX+i)+1]=(unsigned char)qGreen(pixel);
                nim[3*(j*newX+i)+2]=(unsigned char)qBlue(pixel);
            }
            else
            {
                nim[4*(j*newX+i)+0]=(unsigned char)qRed(pixel);
                nim[4*(j*newX+i)+1]=(unsigned char)qGreen(pixel);
                nim[4*(j*newX+i)+2]=(unsigned char)qBlue(pixel);
                nim[4*(j*newX+i)+3]=(unsigned char)qAlpha(pixel);
            }
        }
    }
    delete[] im;
    return(nim);
#else
    return(nullptr);
#endif
}

bool CImageLoaderSaver::transformImage(unsigned char* img,int resX,int resY,int options)
{
    int comp=3;
    if (options&1)
        comp=4;
    unsigned char* img2=new unsigned char[resX*resY*comp];
    for (int i=0;i<resX*resY*comp;i++)
        img2[i]=img[i];
    for (int x=0;x<resX;x++)
    {
        int x2=x;
        if (options&2)
            x2=resX-1-x;
        for (int y=0;y<resY;y++)
        {
            int y2=y;
            if (options&4)
                y2=resY-1-y;
            img[comp*(x+y*resX)+0]=img2[comp*(x2+y2*resX)+0];
            img[comp*(x+y*resX)+1]=img2[comp*(x2+y2*resX)+1];
            img[comp*(x+y*resX)+2]=img2[comp*(x2+y2*resX)+2];
            if (comp==4)
                img[comp*(x+y*resX)+3]=img2[comp*(x2+y2*resX)+3];
        }
    }

    delete[] img2;
    return(true);
}

unsigned char* CImageLoaderSaver::getScaledImage(const unsigned char* originalImg,const int resolIn[2],int resolOut[2],int options)
{
#ifdef SIM_WITH_GUI
    int compIn=3;
    int compOut=3;
    if (options&1)
        compIn=4;
    if (options&2)
        compOut=4;
    QImage::Format f=QImage::Format_RGB888;
    unsigned char* im=new unsigned char[resolIn[0]*resolIn[1]*compIn];
    if (compIn==4)
    {
        f=QImage::Format_ARGB32;
        for (int i=0;i<resolIn[0]*resolIn[1];i++)
        { // from rgba to bgra (corrected on 9/9/2914)
            im[4*i+0]=originalImg[4*i+2];
            im[4*i+1]=originalImg[4*i+1];
            im[4*i+2]=originalImg[4*i+0];
            im[4*i+3]=originalImg[4*i+3];
        }
    }
    else
    {
        for (int i=0;i<resolIn[0]*resolIn[1];i++)
        {
            im[3*i+0]=originalImg[3*i+0];
            im[3*i+1]=originalImg[3*i+1];
            im[3*i+2]=originalImg[3*i+2];
        }
    }
    QImage image(im,resolIn[0],resolIn[1],resolIn[0]*compIn,f);
    Qt::AspectRatioMode aspectRatio=Qt::IgnoreAspectRatio;
    if ((options&12)==4)
        aspectRatio=Qt::KeepAspectRatio;
    if ((options&12)==8)
        aspectRatio=Qt::KeepAspectRatioByExpanding;
    Qt::TransformationMode transform=Qt::SmoothTransformation;
    if (options&16)
        transform=Qt::FastTransformation;
    QImage nimage(image.scaled(resolOut[0],resolOut[1],aspectRatio,transform));
    resolOut[0]=nimage.width();
    resolOut[1]=nimage.height();
    unsigned char* nim=new unsigned char[resolOut[0]*resolOut[1]*compOut];
    for (int j=0;j<resolOut[1];j++)
    {
        for (int i=0;i<resolOut[0];i++)
        {
            QRgb pixel=nimage.pixel(i,j);
            if (compOut==3)
            {
                nim[3*(j*resolOut[0]+i)+0]=(unsigned char)qRed(pixel);
                nim[3*(j*resolOut[0]+i)+1]=(unsigned char)qGreen(pixel);
                nim[3*(j*resolOut[0]+i)+2]=(unsigned char)qBlue(pixel);
            }
            else
            {
                nim[4*(j*resolOut[0]+i)+0]=(unsigned char)qRed(pixel);
                nim[4*(j*resolOut[0]+i)+1]=(unsigned char)qGreen(pixel);
                nim[4*(j*resolOut[0]+i)+2]=(unsigned char)qBlue(pixel);
                nim[4*(j*resolOut[0]+i)+3]=(unsigned char)qAlpha(pixel);
            }
        }
    }
    delete[] im;
    return(nim);
#else
    return(nullptr);
#endif
}

bool CImageLoaderSaver::save(const unsigned char* data,const int resolution[2],int options,const char* filename,int quality,std::string* buffer)
{
    bool retVal=false;
#ifdef SIM_WITH_GUI
    unsigned char *buff=nullptr;
    QImage::Format format=QImage::Format_ARGB32;
    buff=new unsigned char[4*resolution[0]*resolution[1]];
    int bytesPerPixel=4;
    for (int i=0;i<resolution[0];i++)
    {
        for (int j=0;j<resolution[1];j++)
        {
            if ((options&1)==0)
            { // input img provided as rgb
                bytesPerPixel=3;
                format=QImage::Format_RGB888;
                buff[3*(resolution[0]*j+i)+0]=data[3*(resolution[0]*(resolution[1]-j-1)+i)+0];
                buff[3*(resolution[0]*j+i)+1]=data[3*(resolution[0]*(resolution[1]-j-1)+i)+1];
                buff[3*(resolution[0]*j+i)+2]=data[3*(resolution[0]*(resolution[1]-j-1)+i)+2];
            }
            else
            { // input img provided as rgba
                buff[4*(resolution[0]*j+i)+0]=data[4*(resolution[0]*(resolution[1]-j-1)+i)+2];
                buff[4*(resolution[0]*j+i)+1]=data[4*(resolution[0]*(resolution[1]-j-1)+i)+1];
                buff[4*(resolution[0]*j+i)+2]=data[4*(resolution[0]*(resolution[1]-j-1)+i)+0];
                buff[4*(resolution[0]*j+i)+3]=data[4*(resolution[0]*(resolution[1]-j-1)+i)+3];
            }
        }
    }
    {
        QImage image(buff,resolution[0],resolution[1],resolution[0]*bytesPerPixel,format);
        if (buffer==nullptr)
            retVal=image.save(filename,0,quality);
        else
        {
            if (filename[0]=='.')
            {
                QByteArray ba;
                QBuffer qbuffer(&ba);
                qbuffer.open(QIODevice::WriteOnly);
                retVal=image.save(&qbuffer,filename+1);
                if (retVal)
                    buffer->assign(ba.data(),ba.data()+ba.size());
            }
        }
    }
    delete[] buff;
#endif
    return(retVal);
}

unsigned char* CImageLoaderSaver::load(int resolution[2],int options,const char* filename,void* reserved)
{
    unsigned char* retVal=nullptr;
#ifdef SIM_WITH_GUI
    QImage image;
    bool loadRes=false;
    if (reserved!=nullptr)
        loadRes=image.loadFromData((const unsigned char*)filename,((int*)reserved)[0]);
    else
        loadRes=image.load(filename,0);

    if (loadRes)
    {
        if (options&1)
            retVal=new unsigned char[image.height()*image.width()*4];
        else
            retVal=new unsigned char[image.height()*image.width()*3];
        resolution[0]=image.width();
        resolution[1]=image.height();
        for (int x=0;x<image.width();++x)
        {
            for (int y=0;y<image.height();++y)
            {
                QColor col(image.pixel(x,y));
                if (options&1)
                {
                    retVal[4*(resolution[0]*(resolution[1]-y-1)+x)+0]=col.red();
                    retVal[4*(resolution[0]*(resolution[1]-y-1)+x)+1]=col.green();
                    retVal[4*(resolution[0]*(resolution[1]-y-1)+x)+2]=col.blue();
                    retVal[4*(resolution[0]*(resolution[1]-y-1)+x)+3]=col.alpha();
                }
                else
                {
                    retVal[3*(resolution[0]*(resolution[1]-y-1)+x)+0]=col.red();
                    retVal[3*(resolution[0]*(resolution[1]-y-1)+x)+1]=col.green();
                    retVal[3*(resolution[0]*(resolution[1]-y-1)+x)+2]=col.blue();
                }
            }
        }
    }
#endif
    return(retVal);
}
