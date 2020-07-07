#include "vArchive.h"

unsigned short VArchive::LOAD=0;
unsigned short VArchive::STORE=1;

VArchive::VArchive(VFile* file,unsigned short flag)
{
#ifndef SIM_WITH_QT
    _theFile=file;
    _loading=((flag&1)==0);
#else
    _theFile=file;
    _loading=((flag&1)==0);
    _theArchive=new QDataStream(file->getFile());
    // Following 2 important to be compatible with the files written with first CoppeliaSim versions:
    _theArchive->setFloatingPointPrecision(QDataStream::SinglePrecision);
    _theArchive->setByteOrder(QDataStream::LittleEndian);
#endif
}

VArchive::~VArchive()
{
#ifdef SIM_WITH_QT
    delete _theArchive;
#endif
}

void VArchive::writeString(const std::string& str)
{
    for (int i=0;i<int(str.length());i++)
        (*this) << str[i];
}

void VArchive::writeLine(const std::string& line)
{
    writeString(line);
    (*this) << char(13);
    (*this) << char(10);
}

bool VArchive::readSingleLine(unsigned int& actualPosition,std::string& line,bool doNotReplaceTabsWithOneSpace)
{
    unsigned int archiveLength=(unsigned int)_theFile->getLength();
    unsigned char oneByte;
    line="";
    while (actualPosition<archiveLength)
    {
        (*this) >> oneByte;
        actualPosition++;
        if (oneByte!=(unsigned char)13)
        {
            if (oneByte==(unsigned char)10)
                return(true);
            if ( (oneByte!=(unsigned char)9)||doNotReplaceTabsWithOneSpace )
                line.insert(line.end(),(char)oneByte);
            else
                line.insert(line.end(),' ');
        }
    }
    return(line.length()!=0);
}

bool VArchive::readMultiLine(unsigned int& actualPosition,std::string& line,bool doNotReplaceTabsWithOneSpace,const char* multilineSeparator)
{
    line="";
    while (true)
    {
        std::string l;
        bool stillReadMatter=readSingleLine(actualPosition,l,doNotReplaceTabsWithOneSpace);
        while ((l.length()!=0)&&(l[l.length()-1]==' '))
            l.erase(l.begin()+l.length()-1);
        if ((l.length()!=0)&&(l[l.length()-1]=='\\'))
        {
            l.erase(l.begin()+l.length()-1);
            line+=l+multilineSeparator;
            if (!stillReadMatter)
                return(false);
        }
        else
        {
            line+=l;
            return(stillReadMatter);
        }
    }
}

VFile* VArchive::getFile()
{
    return(_theFile);
}

bool VArchive::isStoring()
{
    return(!_loading);
}

bool VArchive::isLoading()
{
    return(_loading);
}

void VArchive::close()
{
}
