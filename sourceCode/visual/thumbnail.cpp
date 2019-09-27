#include "vrepMainHeader.h"
#include "thumbnail.h"
#include "global.h"
#include "v_repConst.h"
#include "thumbnailRendering.h"

CThumbnail::CThumbnail()
{
    _thumbnailRGBAImage=nullptr;
    _compressData=false;
}

CThumbnail::~CThumbnail()
{
    delete[] _thumbnailRGBAImage;
}

void CThumbnail::compressThumbnail(const char* uncompressedRGBAImage,char* compressedRGBImage)
{
    for (int i=0;i<128*64;i++)
    {
        unsigned char r0=uncompressedRGBAImage[8*i+0]/16;
        unsigned char g0=uncompressedRGBAImage[8*i+1]/16;
        unsigned char b0=uncompressedRGBAImage[8*i+2]/16;
        unsigned char a0=uncompressedRGBAImage[8*i+3];
        if (a0==0)
        {
            r0=0;
            g0=15;
            b0=0;
        }
        unsigned char r1=uncompressedRGBAImage[8*i+4]/16;
        unsigned char g1=uncompressedRGBAImage[8*i+5]/16;
        unsigned char b1=uncompressedRGBAImage[8*i+6]/16;
        unsigned char a1=uncompressedRGBAImage[8*i+7];
        if (a1==0)
        {
            r1=0;
            g1=15;
            b1=0;
        }
        unsigned char byte1=(r0&0x0f)|(16*(g0&0x0f));
        unsigned char byte2=(b0&0x0f)|(16*(r1&0x0f));
        unsigned char byte3=(g1&0x0f)|(16*(b1&0x0f));
        compressedRGBImage[3*i+0]=byte1;
        compressedRGBImage[3*i+1]=byte2;
        compressedRGBImage[3*i+2]=byte3;
    }
}

void CThumbnail::uncompressThumbnail(const char* compressedRGBImage,char* uncompressedRGBAImage)
{
    for (int i=0;i<128*64;i++)
    {
        unsigned char byte1=compressedRGBImage[3*i+0];
        unsigned char byte2=compressedRGBImage[3*i+1];
        unsigned char byte3=compressedRGBImage[3*i+2];
        unsigned char r0=((byte1&0x0f)*16);
        unsigned char g0=(byte1&0xf0);
        unsigned char b0=((byte2&0x0f)*16);
        unsigned char r1=(byte2&0xf0);
        unsigned char g1=((byte3&0x0f)*16);
        unsigned char b1=(byte3&0xf0);
        unsigned char a0=255;
        unsigned char a1=255;
        if ((r0==0)&&(g0==240)&&(b0==0))
            a0=0;
        if ((r1==0)&&(g1==240)&&(b1==0))
            a1=0;
        uncompressedRGBAImage[8*i+0]=r0;
        uncompressedRGBAImage[8*i+1]=g0;
        uncompressedRGBAImage[8*i+2]=b0;
        uncompressedRGBAImage[8*i+3]=a0;
        uncompressedRGBAImage[8*i+4]=r1;
        uncompressedRGBAImage[8*i+5]=g1;
        uncompressedRGBAImage[8*i+6]=b1;
        uncompressedRGBAImage[8*i+7]=a1;
    }
}

bool CThumbnail::getCompressedImage(char* compressedRGBImage)
{
    if (_thumbnailRGBAImage==nullptr)
        return(false);
    compressThumbnail(_thumbnailRGBAImage,compressedRGBImage);
    return(true);
}

char* CThumbnail::getPointerToUncompressedImage()
{
    return(_thumbnailRGBAImage);
}

void CThumbnail::setUncompressedThumbnailImage(const char* uncompressedRGBAImage,bool rgba,bool verticalFlip)
{
    setRandomImage();
    if (verticalFlip)
    {
        for (int i=0;i<128;i++)
        {
            for (int j=0;j<128;j++)
            {
                if (rgba)
                {
                    for (int k=0;k<4;k++)
                        _thumbnailRGBAImage[4*(i*128+j)+k]=uncompressedRGBAImage[4*((127-i)*128+j)+k];
                }
                else
                {
                    for (int k=0;k<3;k++)
                        _thumbnailRGBAImage[4*(i*128+j)+k]=uncompressedRGBAImage[3*((127-i)*128+j)+k];
                    _thumbnailRGBAImage[4*(i*128+j)+3]=(char)255;
                }
            }
        }
    }
    else
    {
        if (rgba)
        {
            for (int i=0;i<128*128*4;i++)
                _thumbnailRGBAImage[i]=uncompressedRGBAImage[i];
        }
        else
        {
            for (int i=0;i<128*128;i++)
            {
                _thumbnailRGBAImage[4*i+0]=uncompressedRGBAImage[3*i+0];
                _thumbnailRGBAImage[4*i+1]=uncompressedRGBAImage[3*i+1];
                _thumbnailRGBAImage[4*i+2]=uncompressedRGBAImage[3*i+2];
                _thumbnailRGBAImage[4*i+3]=(char)255;
            }
        }
    }
    _compressData=false;
}

void CThumbnail::setUncompressedThumbnailImageFromFloat(const float* uncompressedRGBAImage)
{
    setRandomImage();
    for (int i=0;i<128*128*4;i++)
        _thumbnailRGBAImage[i]=(unsigned char)(uncompressedRGBAImage[i]*255.1f);
    _compressData=false;
}

void CThumbnail::setRandomImage()
{
    if (_thumbnailRGBAImage==nullptr)
        _thumbnailRGBAImage=new char[128*128*4];
}

void CThumbnail::setCompressedThumbnailImage(const char* compressedRGBImage)
{
    setRandomImage();
    uncompressThumbnail(compressedRGBImage,_thumbnailRGBAImage);
    _compressData=true;
}

void CThumbnail::clearThumbnailImage()
{
    delete[] _thumbnailRGBAImage;
    _thumbnailRGBAImage=nullptr;
    _compressData=false;
}

bool CThumbnail::copyUncompressedImageToBuffer(char* buffer)
{
    if (_thumbnailRGBAImage==nullptr)
        return(false);
    for (int i=0;i<128*128*4;i++)
        buffer[i]=_thumbnailRGBAImage[i];
    return(true);
}

bool CThumbnail::hasImage()
{
    return(_thumbnailRGBAImage!=nullptr);
}

CThumbnail* CThumbnail::copyYourself()
{
    CThumbnail* newT=new CThumbnail();
    if (_thumbnailRGBAImage!=nullptr)
        newT->setUncompressedThumbnailImage(_thumbnailRGBAImage,true,false);
    newT->_compressData=_compressData;
    return(newT);
}

void CThumbnail::copyFrom(CThumbnail* it)
{
    if (it->_thumbnailRGBAImage==nullptr)
    {
        delete[] _thumbnailRGBAImage;
        _thumbnailRGBAImage=nullptr;
    }
    else
    {
        if (_thumbnailRGBAImage==nullptr)
            _thumbnailRGBAImage=new char[128*128*4];
        setUncompressedThumbnailImage(it->_thumbnailRGBAImage,true,false);
    }
    _compressData=it->_compressData;
}

void  CThumbnail::serializeAdditionalModelInfos(CSer& ar,C7Vector& modelTr,C3Vector& modelBBSize,float& modelNonDefaultTranslationStepSize)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Mo2");
            for (int i=0;i<7;i++)
                ar << modelTr(i);
            for (int i=0;i<3;i++)
                ar << modelBBSize(i);
            ar << modelNonDefaultTranslationStepSize;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Mo2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<7;i++)
                            ar >> modelTr(i);
                        for (int i=0;i<3;i++)
                            ar >> modelBBSize(i);
                        ar >> modelNonDefaultTranslationStepSize;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CThumbnail::serialize(CSer& ar,bool forceCompressedSaving/*=false*/)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Tm2");
            if (_thumbnailRGBAImage==nullptr)
                ar << (unsigned char)(0);
            else
            {
                ar << (unsigned char)(1);
                if (forceCompressedSaving||_compressData)
                {
                    ar << (unsigned char)(1);
                    char* compressedImage=new char[128*64*3];
                    getCompressedImage(compressedImage);
                    for (int j=0;j<128*64*3;j++)
                        ar << compressedImage[j];
                    delete[] compressedImage;
                }
                else
                {
                    ar << (unsigned char)(0);
                    for (int j=0;j<128*128*4;j++)
                        ar << _thumbnailRGBAImage[j];
                }
            }
            ar.flush();
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Tmb")==0)
                    { // for backward compatibility (7/3/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char imagePresent;
                        ar >> imagePresent;
                        char dum;
                        if (imagePresent!=0)
                        {
                            _thumbnailRGBAImage=new char[128*128*4];
                            unsigned char compressed;
                            ar >> compressed;
                            _compressData=(compressed!=0);
                            if (_compressData)
                            {
                                char* compressedImage=new char[128*64*3];
                                for (int j=0;j<128*64*3;j++)
                                {
                                    ar >> dum;
                                    compressedImage[j]=dum;
                                }
                                uncompressThumbnail(compressedImage,_thumbnailRGBAImage);
                                delete[] compressedImage;
                            }
                            else
                            {
                                for (int j=0;j<128*128*4;j++)
                                {
                                    ar >> dum;
                                    _thumbnailRGBAImage[j]=dum;
                                }
                            }
                            for (int j=0;j<128*128;j++)
                                _thumbnailRGBAImage[4*j+3]=char(float((unsigned char)(_thumbnailRGBAImage[4*j+3]))/0.22508f);
                        }
                    }
                    if (theName.compare("Tm2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char imagePresent;
                        ar >> imagePresent;
                        char dum;
                        if (imagePresent!=0)
                        {
                            if (_thumbnailRGBAImage==nullptr)
                                _thumbnailRGBAImage=new char[128*128*4];
                            unsigned char compressed;
                            ar >> compressed;
                            _compressData=(compressed!=0);
                            if (_compressData)
                            {
                                char* compressedImage=new char[128*64*3];
                                for (int j=0;j<128*64*3;j++)
                                {
                                    ar >> dum;
                                    compressedImage[j]=dum;
                                }
                                uncompressThumbnail(compressedImage,_thumbnailRGBAImage);
                                delete[] compressedImage;
                            }
                            else
                            {
                                for (int j=0;j<128*128*4;j++)
                                {
                                    ar >> dum;
                                    _thumbnailRGBAImage[j]=dum;
                                }
                            }
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CThumbnail::display(int posX,int posY,const char* name,int sc)
{
    displayThumbnail(this,posX,posY,name,sc);
}
