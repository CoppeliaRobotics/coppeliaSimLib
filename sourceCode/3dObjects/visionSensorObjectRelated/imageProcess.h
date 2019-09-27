
#pragma once

class CImageProcess  
{
public:
    CImageProcess();
    virtual ~CImageProcess();
    static float* createRGBImage(int resX,int resY);
    static void copyRGBImage(int resX,int resY,float* imageSource,float* imageDest);
    static void clearRGBImage(int resX,int resY,float* image,float clearRed,float clearGreen,float clearBlue);

    static float* createIntensityImage(int resX,int resY);
    static void copyIntensityImage(int resX,int resY,float* intensImageSource,float* intensImageDest);
    static void clearIntensityImage(int resX,int resY,float* intensImage,float clearIntens);

    static void filter3x3RgbImage(int resX,int resY,float* rgbIn,float* rgbOut,float m[9]);
    static void filter5x5RgbImage(int resX,int resY,float* rgbIn,float* rgbOut,float m[25]);
    static void clampRgbImage(int resX,int resY,float* rgbImage,float lowClamp,float highClamp);

    static void deleteImage(float* image);

    static void rgbImageToIntensityImage(int resX,int resY,float* rgbImage,float* intensImage);
    static void intensityImageToRGBImage(int resX,int resY,float* rgbImage,float* intensImage);

    static void getEdges(int resX,int resY,float* imageSource,float* imageDest);
    static void scaleIntensity(int resX,int resY,float* image,float scaleFactor,bool clampToMax);
    static void scaleRGB(int resX,int resY,float* image,float redFactor,float greenFactor,float blueFactor,bool clampToMax);
    static void boxBlurIntensity(int resX,int resY,float* imageSource,float* imageDest);
    static void boxBlurRGB(int resX,int resY,float* imageSource,float* imageDest);
    static void blurIntensity(int resX,int resY,float* image);
    static void blurRGB(int resX,int resY,float* image);
    static void horizontalFlipRGB(int resX,int resY,float* image);
    static void verticalFlipRGB(int resX,int resY,float* image);

    static void scaleRgbImageWithIntensityImage(int resX,int resY,float* rgbImage,float* intensImage);

    static void keepThresholdIntensity(int resX,int resY,float* image,float thresholdVal,bool keepAbove);
    static void keepThresholdRGB(int resX,int resY,float* image,float thresholdVal,bool keepAbove);
    static void nonZeroToOneIntensity(int resX,int resY,float* image);
    static void nonZeroToOneRGB(int resX,int resY,float* image);
    static void addImagesRGB(int resX,int resY,float* imageInOut,float* imageOverlay);
    static void clampToOneRGB(int resX,int resY,float* image);

    static void predef_lightBlurRGB(int resX,int resY,float* image);
    static void predef_heavyBlurRGB(int resX,int resY,float* image);
    static void predef_getThinEdgesRGB(int resX,int resY,float* image);
    static void predef_getThickEdgesRGB(int resX,int resY,float* image);
    static void predef_getThinEdgeOverlayRGB(int resX,int resY,float* image);
    static void predef_invertRGB(int resX,int resY,float* image);
};
