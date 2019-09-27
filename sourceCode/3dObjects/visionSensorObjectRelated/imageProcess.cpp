
#include "vrepMainHeader.h"
#include "imageProcess.h"
#include "mathDefines.h"

CImageProcess::CImageProcess()
{
}

CImageProcess::~CImageProcess()
{
}

float* CImageProcess::createRGBImage(int resX,int resY)
{
    float* retVal=new float[resX*resY*3];
    return(retVal);
}
void CImageProcess::copyRGBImage(int resX,int resY,float* imageSource,float* imageDest)
{
    int v=resX*resY*3;
    for (int i=0;i<v;i++)
        imageDest[i]=imageSource[i];
}
void CImageProcess::clearRGBImage(int resX,int resY,float* image,float clearRed,float clearGreen,float clearBlue)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
    {
        image[3*i+0]=clearRed;
        image[3*i+1]=clearGreen;
        image[3*i+2]=clearBlue;
    }
}
float* CImageProcess::createIntensityImage(int resX,int resY)
{
    float* retVal=new float[resX*resY];
    return(retVal);
}
void CImageProcess::copyIntensityImage(int resX,int resY,float* intensImageSource,float* intensImageDest)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
        intensImageDest[i]=intensImageSource[i];
}
void CImageProcess::clearIntensityImage(int resX,int resY,float* intensImage,float clearIntens)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
        intensImage[i]=clearIntens;
}
void CImageProcess::deleteImage(float* image)
{
    delete[] image;
}
void CImageProcess::rgbImageToIntensityImage(int resX,int resY,float* rgbImage,float* intensImage)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
        intensImage[i]=(rgbImage[3*i+0]+rgbImage[3*i+1]+rgbImage[3*i+2])/3.0f;
}
void CImageProcess::intensityImageToRGBImage(int resX,int resY,float* rgbImage,float* intensImage)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
    {
        rgbImage[3*i+0]=intensImage[i];
        rgbImage[3*i+1]=intensImage[i];
        rgbImage[3*i+2]=intensImage[i];
    }
}

void CImageProcess::filter3x3RgbImage(int resX,int resY,float* rgbIn,float* rgbOut,float m[9])
{ // m is specified line after line
    float w,cnt;
    int x,y,tmp;
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY;j++)
        {
            float rgbCumul[3]={0.0f,0.0f,0.0f};
            cnt=0.0f;
            for (int k=-1;k<2;k++)
            {
                x=i+k;
                if ((x>=0)&&(x<resX))
                {
                    for (int l=-1;l<2;l++)
                    {
                        y=j+l;
                        if ((y>=0)&&(y<resY))
                        {
                            w=m[k+1+(l+1)*3];
                            tmp=3*(x+y*resX);
                            rgbCumul[0]+=rgbIn[tmp+0]*w;
                            rgbCumul[1]+=rgbIn[tmp+1]*w;
                            rgbCumul[2]+=rgbIn[tmp+2]*w;
                            cnt+=1.0f;
                        }
                    }
                }
            }
            tmp=3*(i+j*resX);
            cnt/=9.0f;
            rgbOut[tmp+0]=rgbCumul[0]/cnt;
            rgbOut[tmp+1]=rgbCumul[1]/cnt;
            rgbOut[tmp+2]=rgbCumul[2]/cnt;
        }
    }
}

void CImageProcess::filter5x5RgbImage(int resX,int resY,float* rgbIn,float* rgbOut,float m[25])
{ // m is specified line after line
    float w,cnt;
    int x,y,tmp;
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY;j++)
        {
            float rgbCumul[3]={0.0f,0.0f,0.0f};
            cnt=0.0f;
            for (int k=-2;k<3;k++)
            {
                x=i+k;
                if ((x>=0)&&(x<resX))
                {
                    for (int l=-2;l<3;l++)
                    {
                        y=j+l;
                        if ((y>=0)&&(y<resY))
                        {
                            w=m[k+2+(l+2)*5];
                            tmp=3*(x+y*resX);
                            rgbCumul[0]+=rgbIn[tmp+0]*w;
                            rgbCumul[1]+=rgbIn[tmp+1]*w;
                            rgbCumul[2]+=rgbIn[tmp+2]*w;
                            cnt+=1.0f;
                        }
                    }
                }
            }
            tmp=3*(i+j*resX);
            cnt/=25.0f;
            rgbOut[tmp+0]=rgbCumul[0]/cnt;
            rgbOut[tmp+1]=rgbCumul[1]/cnt;
            rgbOut[tmp+2]=rgbCumul[2]/cnt;
        }
    }
}

void CImageProcess::clampRgbImage(int resX,int resY,float* rgbImage,float lowClamp,float highClamp)
{
    int s=resX*resY*3;
    for (int i=0;i<s;i++)
    {
        if (rgbImage[i]<lowClamp)
            rgbImage[i]=lowClamp;
        else
        {
            if (rgbImage[i]>highClamp)
                rgbImage[i]=highClamp;
        }
    }
}

void CImageProcess::getEdges(int resX,int resY,float* imageSource,float* imageDest)
{
    float maxV=0.0f;
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY;j++)
        {
            float gx=0.0f;
            float gy=0.0f;
            int gxc=0;
            int gyc=0;

            if ( (i>0)&&(i<resX-1) )
            { // Here we do the x-direction
                if (j>0)
                {
                    gx+=3.0f*imageSource[(i-1)+(j-1)*resX];
                    gx-=3.0f*imageSource[(i+1)+(j-1)*resX];
                    gxc++;
                }
                gx+=10.0f*imageSource[(i-1)+(j+0)*resX];
                gx-=10.0f*imageSource[(i+1)+(j+0)*resX];
                gxc++;
                if (j<resY-1)
                {
                    gx+=3.0f*imageSource[(i-1)+(j+1)*resX];
                    gx-=3.0f*imageSource[(i+1)+(j+1)*resX];
                    gxc++;
                }
            }

            if ( (j>0)&&(j<resY-1) )
            { // Here we do the y-direction
                if (i>0)
                {
                    gy+=3.0f*imageSource[(i-1)+(j-1)*resX];
                    gy-=3.0f*imageSource[(i-1)+(j+1)*resX];
                    gyc++;
                }
                gy+=10.0f*imageSource[(i+0)+(j-1)*resX];
                gy-=10.0f*imageSource[(i+0)+(j+1)*resX];
                gyc++;
                if (i<resX-1)
                {
                    gy+=3.0f*imageSource[(i+1)+(j-1)*resX];
                    gy-=3.0f*imageSource[(i+1)+(j+1)*resX];
                    gyc++;
                }
            }

            if (gxc==0)
                gx=0.0f;
            if (gyc==0)
                gy=0.0f;
            float g=sqrtf(gx*gx+gy*gy);
            imageDest[i+j*resX]=g;
            if (g>maxV)
                maxV=g;
        }
    }
    if (maxV!=0.0f)
        scaleIntensity(resX,resY,imageDest,1.0f/maxV,false);
}

void CImageProcess::scaleIntensity(int resX,int resY,float* image,float scaleFactor,bool clampToMax)
{
    int v=resX*resY;
    if (clampToMax)
    {
        for (int i=0;i<v;i++)
        {
            image[i]*=scaleFactor;
            if (image[i]>1.0f)
                image[i]=1.0f;
        }
    }
    else
    {
        for (int i=0;i<v;i++)
            image[i]*=scaleFactor;
    }
}

void CImageProcess::scaleRGB(int resX,int resY,float* image,float redFactor,float greenFactor,float blueFactor,bool clampToMax)
{
    int v=resX*resY;
    if (clampToMax)
    {
        for (int i=0;i<v;i++)
        {
            image[3*i+0]*=redFactor;
            if (image[3*i+0]>1.0f)
                image[3*i+0]=1.0f;
            image[3*i+1]*=greenFactor;
            if (image[3*i+1]>1.0f)
                image[3*i+1]=1.0f;
            image[3*i+2]*=blueFactor;
            if (image[3*i+2]>1.0f)
                image[3*i+2]=1.0f;
        }
    }
    else
    {
        for (int i=0;i<v;i++)
        {
            image[3*i+0]*=redFactor;
            image[3*i+1]*=greenFactor;
            image[3*i+2]*=blueFactor;
        }
    }
}

void CImageProcess::scaleRgbImageWithIntensityImage(int resX,int resY,float* rgbImage,float* intensImage)
{
    int s=resX*resY;
    for (int i=0;i<s;i++)
    {
        float d=intensImage[i]*3.0f;
        rgbImage[3*i+0]*=d;
        rgbImage[3*i+1]*=d;
        rgbImage[3*i+2]*=d;
    }
}

void CImageProcess::boxBlurIntensity(int resX,int resY,float* imageSource,float* imageDest)
{
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY;j++)
        {
            float gx=0.0f;
            int gxc=0;
            if (i>0)
            {
                if (j>0)
                {
                    gx+=imageSource[(i-1)+(j-1)*resX];
                    gxc++;
                }
                gx+=imageSource[(i-1)+(j+0)*resX];
                gxc++;
                if (j<resY-1)
                {
                    gx+=imageSource[(i-1)+(j+1)*resX];
                    gxc++;
                }
            }
            if (j>0)
            {
                gx+=imageSource[(i+0)+(j-1)*resX];
                gxc++;
            }
            gx+=imageSource[(i+0)+(j+0)*resX];
            gxc++;
            if (j<resY-1)
            {
                gx+=imageSource[(i+0)+(j+1)*resX];
                gxc++;
            }

            if (i<resX-1)
            {
                if (j>0)
                {
                    gx+=imageSource[(i+1)+(j-1)*resX];
                    gxc++;
                }
                gx+=imageSource[(i+1)+(j+0)*resX];
                gxc++;
                if (j<resY-1)
                {
                    gx+=imageSource[(i+1)+(j+1)*resX];
                    gxc++;
                }
            }
            gx/=float(gxc);
            imageDest[i+j*resX]=gx;
        }
    }
}

void CImageProcess::boxBlurRGB(int resX,int resY,float* imageSource,float* imageDest)
{
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY;j++)
        {
            float gRed=0.0f;
            float gGreen=0.0f;
            float gBlue=0.0f;
            int gxc=0;
            if (i>0)
            {
                if (j>0)
                {
                    gRed+=imageSource[3*((i-1)+(j-1)*resX)+0];
                    gGreen+=imageSource[3*((i-1)+(j-1)*resX)+1];
                    gBlue+=imageSource[3*((i-1)+(j-1)*resX)+2];
                    gxc++;
                }
                gRed+=imageSource[3*((i-1)+(j+0)*resX)+0];
                gGreen+=imageSource[3*((i-1)+(j+0)*resX)+1];
                gBlue+=imageSource[3*((i-1)+(j+0)*resX)+2];
                gxc++;
                if (j<resY-1)
                {
                    gRed+=imageSource[3*((i-1)+(j+1)*resX)+0];
                    gGreen+=imageSource[3*((i-1)+(j+1)*resX)+1];
                    gBlue+=imageSource[3*((i-1)+(j+1)*resX)+2];
                    gxc++;
                }
            }
            if (j>0)
            {
                gRed+=imageSource[3*((i+0)+(j-1)*resX)+0];
                gGreen+=imageSource[3*((i+0)+(j-1)*resX)+1];
                gBlue+=imageSource[3*((i+0)+(j-1)*resX)+2];
                gxc++;
            }
            gRed+=imageSource[3*((i+0)+(j+0)*resX)+0];
            gGreen+=imageSource[3*((i+0)+(j+0)*resX)+1];
            gBlue+=imageSource[3*((i+0)+(j+0)*resX)+2];
            gxc++;
            if (j<resY-1)
            {
                gRed+=imageSource[3*((i+0)+(j+1)*resX)+0];
                gGreen+=imageSource[3*((i+0)+(j+1)*resX)+1];
                gBlue+=imageSource[3*((i+0)+(j+1)*resX)+2];
                gxc++;
            }

            if (i<resX-1)
            {
                if (j>0)
                {
                    gRed+=imageSource[3*((i+1)+(j-1)*resX)+0];
                    gGreen+=imageSource[3*((i+1)+(j-1)*resX)+1];
                    gBlue+=imageSource[3*((i+1)+(j-1)*resX)+2];
                    gxc++;
                }
                gRed+=imageSource[3*((i+1)+(j+0)*resX)+0];
                gGreen+=imageSource[3*((i+1)+(j+0)*resX)+1];
                gBlue+=imageSource[3*((i+1)+(j+0)*resX)+2];
                gxc++;
                if (j<resY-1)
                {
                    gRed+=imageSource[3*((i+1)+(j+1)*resX)+0];
                    gGreen+=imageSource[3*((i+1)+(j+1)*resX)+1];
                    gBlue+=imageSource[3*((i+1)+(j+1)*resX)+2];
                    gxc++;
                }
            }
            gRed/=float(gxc);
            gGreen/=float(gxc);
            gBlue/=float(gxc);
            imageDest[3*(i+j*resX)+0]=gRed;
            imageDest[3*(i+j*resX)+1]=gGreen;
            imageDest[3*(i+j*resX)+2]=gBlue;
        }
    }
}

void CImageProcess::blurIntensity(int resX,int resY,float* image)
{
    float* im2=createIntensityImage(resX,resY);
    copyIntensityImage(resX,resY,image,im2);
    boxBlurIntensity(resX,resY,im2,image);
    boxBlurIntensity(resX,resY,image,im2);
    boxBlurIntensity(resX,resY,im2,image);
    deleteImage(im2);
}

void CImageProcess::blurRGB(int resX,int resY,float* image)
{
    float* im2=createRGBImage(resX,resY);
    copyRGBImage(resX,resY,image,im2);
    boxBlurRGB(resX,resY,im2,image);
    boxBlurRGB(resX,resY,image,im2);
    boxBlurRGB(resX,resY,im2,image);
    deleteImage(im2);
}

void CImageProcess::horizontalFlipRGB(int resX,int resY,float* image)
{
    float tmp;
    for (int i=0;i<resX/2;i++)
    {
        for (int j=0;j<resY;j++)
        {
            for (int k=0;k<3;k++)
            {
                tmp=image[3*(i+j*resX)+k];
                image[3*(i+j*resX)+k]=image[3*((resX-1-i)+j*resX)+k];
                image[3*((resX-1-i)+j*resX)+k]=tmp;
            }
        }
    }
}

void CImageProcess::verticalFlipRGB(int resX,int resY,float* image)
{
    float tmp;
    for (int i=0;i<resX;i++)
    {
        for (int j=0;j<resY/2;j++)
        {
            for (int k=0;k<3;k++)
            {
                tmp=image[3*(i+j*resX)+k];
                image[3*(i+j*resX)+k]=image[3*(i+(resY-1-j)*resX)+k];
                image[3*(i+(resY-1-j)*resX)+k]=tmp;
            }
        }
    }
}

void CImageProcess::keepThresholdIntensity(int resX,int resY,float* image,float thresholdVal,bool keepAbove)
{
    int v=resX*resY;
    if (keepAbove)
    {
        for (int i=0;i<v;i++)
        {
            if (image[i]<thresholdVal)
                image[i]=0.0f;
        }
    }
    else
    {
        for (int i=0;i<v;i++)
        {
            if (image[i]>thresholdVal)
                image[i]=0.0f;
        }
    }
}

void CImageProcess::keepThresholdRGB(int resX,int resY,float* image,float thresholdVal,bool keepAbove)
{
    int v=resX*resY;
    thresholdVal*=3.0f;
    if (keepAbove)
    {
        for (int i=0;i<v;i++)
        {
            float w=image[3*i+0]+image[3*i+1]+image[3*i+2];
            if (w<thresholdVal)
            {
                image[3*i+0]=0.0f;
                image[3*i+1]=0.0f;
                image[3*i+2]=0.0f;
            }
        }
    }
    else
    {
        for (int i=0;i<v;i++)
        {
            float w=image[3*i+0]+image[3*i+1]+image[3*i+2];
            if (w>thresholdVal)
            {
                image[3*i+0]=0.0f;
                image[3*i+1]=0.0f;
                image[3*i+2]=0.0f;
            }
        }
    }
}

void CImageProcess::nonZeroToOneIntensity(int resX,int resY,float* image)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
    {
        if (image[i]!=0.0f)
            image[i]=1.0f;
    }
}

void CImageProcess::nonZeroToOneRGB(int resX,int resY,float* image)
{
    int v=resX*resY;
    for (int i=0;i<v;i++)
    {
        float w=image[3*i+0]+image[3*i+1]+image[3*i+2];
        if (w!=0.0f)
        {
            image[3*i+0]=1.0f;
            image[3*i+1]=1.0f;
            image[3*i+2]=1.0f;
        }
    }
}

void CImageProcess::addImagesRGB(int resX,int resY,float* imageInOut,float* imageOverlay)
{
    int v=resX*resY*3;
    for (int i=0;i<v;i++)
        imageInOut[i]+=imageOverlay[i];
}

void CImageProcess::clampToOneRGB(int resX,int resY,float* image)
{
    int v=resX*resY*3;
    for (int i=0;i<v;i++)
    {
        if (image[i]>1.0f)
            image[i]=1.0f;
    }
}

void CImageProcess::predef_lightBlurRGB(int resX,int resY,float* image)
{
    blurRGB(resX,resY,image);
}

void CImageProcess::predef_heavyBlurRGB(int resX,int resY,float* image)
{
    blurRGB(resX,resY,image);
    blurRGB(resX,resY,image);
    blurRGB(resX,resY,image);
}

void CImageProcess::predef_getThinEdgesRGB(int resX,int resY,float* image)
{
    float* im0=createIntensityImage(resX,resY);
    rgbImageToIntensityImage(resX,resY,image,im0);
    float* im1=createIntensityImage(resX,resY);
    getEdges(resX,resY,im0,im1);
    keepThresholdIntensity(resX,resY,im1,0.1f,true);
    nonZeroToOneIntensity(resX,resY,im1);
    intensityImageToRGBImage(resX,resY,image,im1);
    deleteImage(im1);
    deleteImage(im0);
}

void CImageProcess::predef_getThinEdgeOverlayRGB(int resX,int resY,float* image)
{
    float* im0=createIntensityImage(resX,resY);
    rgbImageToIntensityImage(resX,resY,image,im0);
    float* im1=createIntensityImage(resX,resY);
    getEdges(resX,resY,im0,im1);
    keepThresholdIntensity(resX,resY,im1,0.1f,true);
    nonZeroToOneIntensity(resX,resY,im1);
    float* im2=createRGBImage(resX,resY);
    copyRGBImage(resX,resY,image,im2);
    intensityImageToRGBImage(resX,resY,im2,im1);
    addImagesRGB(resX,resY,image,im2);
    clampToOneRGB(resX,resY,image);
    deleteImage(im2);
    deleteImage(im1);
    deleteImage(im0);
}

void CImageProcess::predef_getThickEdgesRGB(int resX,int resY,float* image)
{
    float* im0=createIntensityImage(resX,resY);
    rgbImageToIntensityImage(resX,resY,image,im0);
    blurIntensity(resX,resY,im0);
    float* im1=createIntensityImage(resX,resY);
    getEdges(resX,resY,im0,im1);
    keepThresholdIntensity(resX,resY,im1,0.1f,true);
    nonZeroToOneIntensity(resX,resY,im1);
    intensityImageToRGBImage(resX,resY,image,im1);
    deleteImage(im1);
    deleteImage(im0);
}

void CImageProcess::predef_invertRGB(int resX,int resY,float* image)
{
    int v=resX*resY*3;
    for (int i=0;i<v;i++)
        image[i]=1.0f-image[i];
}
