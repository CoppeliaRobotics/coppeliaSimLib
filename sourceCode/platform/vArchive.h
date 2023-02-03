#pragma once

#include <simTypes.h>
#include <vFile.h>
#ifdef SIM_WITH_QT
#include <QDataStream>
#endif

class VArchive  
{
public:
    VArchive(VFile* file,unsigned short flag);
    virtual ~VArchive();

    void writeLine(const std::string& line); // Will add char(10) and char(13)
    void writeString(const std::string& str); // Will not add char(10) or char(13)
    bool readSingleLine(unsigned int& actualPosition,std::string& line,bool doNotReplaceTabsWithOneSpace);
    bool readMultiLine(unsigned int& actualPosition,std::string& line,bool doNotReplaceTabsWithOneSpace,const char* multilineSeparator);

    VFile* getFile();
    bool isStoring();
    bool isLoading();
    void close();

    inline VArchive& operator<< (const int& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << v;
#endif
        return(*this);
    }

    inline VArchive& operator<< (const unsigned int& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << v;
#endif
        return(*this);
    }

    inline VArchive& operator<< (const float& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << v;
#endif
        return(*this);
    }

    VArchive& operator<< (const double& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        // Very special here (because of the strange thing with _theArchive->setFloatingPointPrecision(QDataStream::SinglePrecision) )
        (*_theArchive) << ((unsigned char*)&v)[0];
        (*_theArchive) << ((unsigned char*)&v)[1];
        (*_theArchive) << ((unsigned char*)&v)[2];
        (*_theArchive) << ((unsigned char*)&v)[3];
        (*_theArchive) << ((unsigned char*)&v)[4];
        (*_theArchive) << ((unsigned char*)&v)[5];
        (*_theArchive) << ((unsigned char*)&v)[6];
        (*_theArchive) << ((unsigned char*)&v)[7];
#endif
        return(*this);
    }

    inline VArchive& operator<< (const unsigned short& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << v;
#endif
        return(*this);
    }

    inline VArchive& operator<< (const unsigned char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << quint8(v);
#endif
        return(*this);
    }

    inline VArchive& operator<< (const char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write(&v,sizeof(v));
#else
        (*_theArchive) << qint8(v);
#endif
        return(*this);
    }

    inline VArchive& operator<< (const signed char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->write((char*)&v,sizeof(v));
#else
        (*_theArchive) << qint8(v);
#endif
        return(*this);
    }

    inline VArchive& operator>> (int& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    inline VArchive& operator>> (unsigned int& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    inline VArchive& operator>> (float& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    VArchive& operator>> (double& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        // Very special here (because of the strange thing with _theArchive->setFloatingPointPrecision(QDataStream::SinglePrecision) )
        (*_theArchive) >> ((unsigned char*)&v)[0];
        (*_theArchive) >> ((unsigned char*)&v)[1];
        (*_theArchive) >> ((unsigned char*)&v)[2];
        (*_theArchive) >> ((unsigned char*)&v)[3];
        (*_theArchive) >> ((unsigned char*)&v)[4];
        (*_theArchive) >> ((unsigned char*)&v)[5];
        (*_theArchive) >> ((unsigned char*)&v)[6];
        (*_theArchive) >> ((unsigned char*)&v)[7];
#endif
        return(*this);
    }

    inline VArchive& operator>> (unsigned short& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    inline VArchive& operator>> (unsigned char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    inline VArchive& operator>> (signed char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read((char*)&v,sizeof(v));
#else
        (*_theArchive) >> v;
#endif
        return(*this);
    }

    inline VArchive& operator>> (char& v)
    {
#ifndef SIM_WITH_QT
        _theFile->getFile()->read(&v,sizeof(v));
#else
        (*_theArchive) >> ((qint8*)(&v))[0];
#endif
        return(*this);
    }

private:
#ifdef SIM_WITH_QT
    QDataStream* _theArchive;
#endif
    VFile* _theFile;
    bool _loading;

public:
    static unsigned short LOAD;
    static unsigned short STORE;
};
