#pragma once

#include <simLib/simTypes.h>
#include <vFile.h>
#include <QDataStream>

class VArchive
{
  public:
    VArchive(VFile *file, unsigned short flag);
    virtual ~VArchive();

    void writeLine(const std::string &line);  // Will add char(10) and char(13)
    void writeString(const std::string &str); // Will not add char(10) or char(13)
    bool readSingleLine(unsigned int &actualPosition, std::string &line, bool doNotReplaceTabsWithOneSpace);
    bool readMultiLine(unsigned int &actualPosition, std::string &line, bool doNotReplaceTabsWithOneSpace,
                       const char *multilineSeparator);

    VFile *getFile();
    bool isStoring();
    bool isLoading();
    void close();

    inline VArchive &operator<<(const std::string &v)
    {
        (*_theArchive) << int(v.length());
        _theArchive->writeRawData(v.data(), int(v.length()));
        return (*this);
    }

    inline VArchive &operator<<(const int &v)
    {
        (*_theArchive) << v;
        return (*this);
    }

    inline VArchive &operator<<(const unsigned int &v)
    {
        (*_theArchive) << v;
        return (*this);
    }

    inline VArchive &operator<<(const float &v)
    {
        (*_theArchive) << v;
        return (*this);
    }

    VArchive &operator<<(const double &v)
    {
        // Very special here (because of the strange thing with
        // _theArchive->setFloatingPointPrecision(QDataStream::SinglePrecision) )
        (*_theArchive) << ((unsigned char *)&v)[0];
        (*_theArchive) << ((unsigned char *)&v)[1];
        (*_theArchive) << ((unsigned char *)&v)[2];
        (*_theArchive) << ((unsigned char *)&v)[3];
        (*_theArchive) << ((unsigned char *)&v)[4];
        (*_theArchive) << ((unsigned char *)&v)[5];
        (*_theArchive) << ((unsigned char *)&v)[6];
        (*_theArchive) << ((unsigned char *)&v)[7];
        return (*this);
    }

    inline VArchive &operator<<(const unsigned short &v)
    {
        (*_theArchive) << v;
        return (*this);
    }

    inline VArchive &operator<<(const unsigned char &v)
    {
        (*_theArchive) << quint8(v);
        return (*this);
    }

    inline VArchive &operator<<(const char &v)
    {
        (*_theArchive) << qint8(v);
        return (*this);
    }

    inline VArchive &operator<<(const signed char &v)
    {
        (*_theArchive) << qint8(v);
        return (*this);
    }

    inline VArchive &operator>>(std::string &v)
    {
        int l;
        (*_theArchive) >> l;
        v.resize(size_t(l));
        _theArchive->readRawData(&v[0], l);
        return (*this);
    }

    inline VArchive &operator>>(int &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    inline VArchive &operator>>(unsigned int &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    inline VArchive &operator>>(float &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    VArchive &operator>>(double &v)
    {
        // Very special here (because of the strange thing with
        // _theArchive->setFloatingPointPrecision(QDataStream::SinglePrecision) )
        (*_theArchive) >> ((unsigned char *)&v)[0];
        (*_theArchive) >> ((unsigned char *)&v)[1];
        (*_theArchive) >> ((unsigned char *)&v)[2];
        (*_theArchive) >> ((unsigned char *)&v)[3];
        (*_theArchive) >> ((unsigned char *)&v)[4];
        (*_theArchive) >> ((unsigned char *)&v)[5];
        (*_theArchive) >> ((unsigned char *)&v)[6];
        (*_theArchive) >> ((unsigned char *)&v)[7];
        return (*this);
    }

    inline VArchive &operator>>(unsigned short &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    inline VArchive &operator>>(unsigned char &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    inline VArchive &operator>>(signed char &v)
    {
        (*_theArchive) >> v;
        return (*this);
    }

    inline VArchive &operator>>(char &v)
    {
        (*_theArchive) >> ((qint8 *)(&v))[0];
        return (*this);
    }

  private:
    QDataStream *_theArchive;
    VFile *_theFile;
    bool _loading;

  public:
    static unsigned short LOAD;
    static unsigned short STORE;
};
