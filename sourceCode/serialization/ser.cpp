#include "ser.h"
#include "huffman.h"
#include "simStrings.h"
#include "app.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "pluginContainer.h"
#include <boost/algorithm/string/predicate.hpp>
#include "imgLoaderSaver.h"
#include "pluginContainer.h"
#include "simFlavor.h"

int CSer::SER_SERIALIZATION_VERSION=23; // 9 since 2008/09/01,
                                        // 10 since 2009/02/14,
                                        // 11 since 2009/05/15,
                                        // 12 since 2009/07/03,
                                        // 13 since 2009/07/21,
                                        // 14 since 2010/02/26
                                        // 15 since 2010/04/17 (after removal of CGeomCont and cloning option for geom resources)
                                        // 16 since 2012/11/10 (after rewritten shape and geometric classes. We do not support serialization version 14 and earlier anymore!)
                                        // 17 since 2013/08/29 (after correcting for light intensity and object colors)
                                        // 18 since 2014/07/01 (after saving the vertices, indices, normals and edges in a separate section, in order to reduce file size with redundant content)
                                        // 19 since 2016/10/29 (Materials are now part of the shapes, and are not shared anymore. This version still supports older CoppeliaSim versions (material data is redundant for a while)
                                        // 20 since 2017/03/08 (small detail)
                                        // 21 since 2017/05/26 (New API notation)
                                        // 22 since 2019/04/29 (Striped away some backward compatibility features)
                                        // 23 since 2021/03/08 (not a big diff. in format, more in content (e.g. Lua5.3, main script using require, etc.))

int CSer::SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS=18; // means: files written with this can be read by older CoppeliaSim with serialization THE_NUMBER
int CSer::SER_MIN_SERIALIZATION_VERSION_THAT_THIS_CAN_READ=18; // means: this executable can read versions >=THE_NUMBER
int CSer::XML_XSERIALIZATION_VERSION=2;
int CSer::_serializationVersionThatWroteLastFile=-1;
const bool xmlDebug=false;
char CSer::getFileTypeFromName(const char* filename)
{
    return((char)CSimFlavor::getIntVal_str(0,filename));
}

CSer::CSer(const char* filename,char filetype)
{
    _commonInit();
    _filetype=filetype;
    _filename=filename;
}

CSer::CSer(std::vector<char>& bufferArchive,char filetype)
{ // if bufferArchive is empty, we are storing, otherwise we are restoring
    _commonInit();
    _filetype=filetype;
    _bufferArchive=&bufferArchive;
}


CSer::~CSer()
{
    if (theArchive!=nullptr)
    {
        theArchive->close();
        delete theArchive;
    }
    if (theFile!=nullptr)
    {
        theFile->close();
        delete theFile;
    }
}

void CSer::_commonInit()
{
    theFile=nullptr;
    theArchive=nullptr;
    _bufferArchive=nullptr;
    _xmlCurrentNode=nullptr;
    _multiPurposeCounter=0;
    _xmlUseImageAndMeshFileformats=false;
    _noHeader=false;
    countingMode=0;
    counter=0;
    _coutingModeDisabledExceptForExceptions=false;
    buffer.reserve(100000);
    _fileBuffer.reserve(1000000);
    _fileBuffer.clear();
    _fileBufferReadPointer=0;
    _foundUnknownCommands=false;
}

bool CSer::writeOpenBinary(bool compress)
{
    bool retVal=false;
    _storing=true;
    _compress=compress;
    if ( (_filetype==filetype_csim_bin_scene_file)||(_filetype==filetype_csim_bin_model_file)||
         (_filetype==filetype_csim_bin_thumbnails_file)||(_filetype==filetype_csim_bin_ui_file) )
    {
        theFile=new VFile(_filename.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE,true);
        if (theFile->getFile()!=nullptr)
        {
            theArchive=new VArchive(theFile,VArchive::STORE);
            retVal=true;
        }
        else
        {
            delete theFile;
            theFile=nullptr;
        }
    }
    if ( (_filetype==filetype_csim_bin_scene_buff)||(_filetype==filetype_csim_bin_model_buff) )
        retVal=true;
    return(retVal);
}

bool CSer::writeOpenXml(int maxInlineBufferSize,bool useImageAndMeshFileformats)
{
    _xmlUseImageAndMeshFileformats=useImageAndMeshFileformats;
    bool retVal=false;
    _xmlMaxInlineBufferSize=maxInlineBufferSize;
    _storing=true;
    if ( (_filetype==filetype_csim_xml_xscene_file)||(_filetype==filetype_csim_xml_xmodel_file)||(_filetype==filetype_csim_xml_simplescene_file)||(_filetype==filetype_csim_xml_simplemodel_file) )
    {
        _writeXmlHeader();
        retVal=true;
    }
    return(retVal);
}

bool CSer::writeOpenBinaryNoHeader(bool compress)
{
    bool retVal=false;
    _storing=true;
    _compress=compress;
    _noHeader=true;
    if (_filetype==filetype_bin_file)
    {
        theFile=new VFile(_filename.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE,true);
        if (theFile->getFile()!=nullptr)
        {
            theArchive=new VArchive(theFile,VArchive::STORE);
            retVal=true;
        }
        else
        {
            delete theFile;
            theFile=nullptr;
        }
    }
    if (_filetype==filetype_bin_buff)
        retVal=true;
    return(retVal);
}

void CSer::writeClose()
{
    if ( (_filetype==filetype_csim_xml_xscene_file)||(_filetype==filetype_csim_xml_xmodel_file)||(_filetype==filetype_csim_xml_simplescene_file)||(_filetype==filetype_csim_xml_simplemodel_file) )
        _writeXmlFooter();
    else
    { // we write the whole file from the fileBuffer:
        if (!_noHeader)
            _writeBinaryHeader();
        // Now we write all the data:
        if (_compress)
        { // compressed. When changing compression method, then serialization version has to be incremented and older version won't be able to read newer versions anymore!
            // Hufmann:
            unsigned char* writeBuff=new unsigned char[_fileBuffer.size()+400]; // actually 384
            int outSize=Huffman_Compress(&_fileBuffer[0],writeBuff,(int)_fileBuffer.size());
            if (theArchive!=nullptr)
            {
                for (int i=0;i<outSize;i++)
                    (*theArchive) << writeBuff[i];
            }
            else
            {
                for (int i=0;i<outSize;i++)
                    (*_bufferArchive).push_back(writeBuff[i]);
            }
            delete[] writeBuff;
        }
        else
        { // no compression
            int l=int(_fileBuffer.size());
            if (theArchive!=nullptr)
            {
                for (int i=0;i<l;i++)
                    (*theArchive) << _fileBuffer[i];
            }
            else
            {
                for (int i=0;i<l;i++)
                    (*_bufferArchive).push_back(_fileBuffer[i]);
            }
        }
        _fileBuffer.clear();
    }
}


void CSer::_writeBinaryHeader()
{
    // We write the header:
    for (size_t i=0;i<strlen(SER_SIM_HEADER);i++)
    {
        if (theArchive!=nullptr)
            (*theArchive) << SER_SIM_HEADER[i];
        else
            (*_bufferArchive).push_back(SER_SIM_HEADER[i]);
    }
    // We write the serialization version:
    if (theArchive!=nullptr)
    {
        (*theArchive) << ((char*)&SER_SERIALIZATION_VERSION)[0];
        (*theArchive) << ((char*)&SER_SERIALIZATION_VERSION)[1];
        (*theArchive) << ((char*)&SER_SERIALIZATION_VERSION)[2];
        (*theArchive) << ((char*)&SER_SERIALIZATION_VERSION)[3];
    }
    else
    {
        (*_bufferArchive).push_back(((char*)&SER_SERIALIZATION_VERSION)[0]);
        (*_bufferArchive).push_back(((char*)&SER_SERIALIZATION_VERSION)[1]);
        (*_bufferArchive).push_back(((char*)&SER_SERIALIZATION_VERSION)[2]);
        (*_bufferArchive).push_back(((char*)&SER_SERIALIZATION_VERSION)[3]);
    }

    // We write the minimum sim. version that can read this:
    if (theArchive!=nullptr)
    {
        (*theArchive) << ((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[0];
        (*theArchive) << ((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[1];
        (*theArchive) << ((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[2];
        (*theArchive) << ((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[3];
    }
    else
    {
        (*_bufferArchive).push_back(((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[0]);
        (*_bufferArchive).push_back(((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[1]);
        (*_bufferArchive).push_back(((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[2]);
        (*_bufferArchive).push_back(((char*)&SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS)[3]);
    }
    // We write the compression method:
    char compressionMethod=0; 
    if (_compress)
        compressionMethod=1; // 1 for Huffman
    if (theArchive!=nullptr)
        (*theArchive) << char(compressionMethod); 
    else
        (*_bufferArchive).push_back(char(compressionMethod));

    // We write the uncompressed data length:
    int l=(int)_fileBuffer.size();
    if (theArchive!=nullptr)
    {
        (*theArchive) << ((char*)&l)[0];
        (*theArchive) << ((char*)&l)[1];
        (*theArchive) << ((char*)&l)[2];
        (*theArchive) << ((char*)&l)[3];
    }
    else
    {
        (*_bufferArchive).push_back(((char*)&l)[0]);
        (*_bufferArchive).push_back(((char*)&l)[1]);
        (*_bufferArchive).push_back(((char*)&l)[2]);
        (*_bufferArchive).push_back(((char*)&l)[3]);
    }

    // We write the compilation version: (since ser ver 13 (2009/07/21))
    int compilVer=-1;
    int v=CSimFlavor::getIntVal(2);
    if (v==-1)
        compilVer=6;
    if (v==0)
        compilVer=2;
    if (v==1)
        compilVer=0;
    if (v==2)
        compilVer=1;
    if (v==3)
        compilVer=3;

    if (theArchive!=nullptr)
    {
        (*theArchive) << ((char*)&compilVer)[0];
        (*theArchive) << ((char*)&compilVer)[1];
        (*theArchive) << ((char*)&compilVer)[2];
        (*theArchive) << ((char*)&compilVer)[3];
    }
    else
    {
        (*_bufferArchive).push_back(((char*)&compilVer)[0]);
        (*_bufferArchive).push_back(((char*)&compilVer)[1]);
        (*_bufferArchive).push_back(((char*)&compilVer)[2]);
        (*_bufferArchive).push_back(((char*)&compilVer)[3]);
    }

    // We write the license version that wrote this file and the CoppeliaSim version:
    unsigned int licenseType;
    int lt=CSimFlavor::getIntVal(2);
    if (lt==-1)
        licenseType=0x00006000;
    if (lt==0)
        licenseType=0x00005000;
    if (lt==1)
        licenseType=0x00001000;
    if (lt==2)
        licenseType=0x00002000;
    if (lt==3)
        licenseType=0x00007000;
    if (theArchive!=nullptr)
    {
        unsigned short v=SIM_PROGRAM_VERSION_NB;
        (*theArchive) << ((char*)&v)[0];
        (*theArchive) << ((char*)&v)[1];

        (*theArchive) << ((char*)&licenseType)[0];
        (*theArchive) << ((char*)&licenseType)[1];
        (*theArchive) << ((char*)&licenseType)[2];
        (*theArchive) << ((char*)&licenseType)[3];
    }
    else
    {
        unsigned short v=SIM_PROGRAM_VERSION_NB;
        (*_bufferArchive).push_back(((char*)&v)[0]);
        (*_bufferArchive).push_back(((char*)&v)[1]);

        (*_bufferArchive).push_back(((char*)&licenseType)[0]);
        (*_bufferArchive).push_back(((char*)&licenseType)[1]);
        (*_bufferArchive).push_back(((char*)&licenseType)[2]);
        (*_bufferArchive).push_back(((char*)&licenseType)[3]);
    }

    // We write the revision number:
    if (theArchive!=nullptr)
        (*theArchive) << (char)SIM_PROGRAM_REVISION_NB;
    else
        (*_bufferArchive).push_back(SIM_PROGRAM_REVISION_NB);


    // File type:
    if (theArchive!=nullptr)
        (*theArchive) << _filetype;
    else
        (*_bufferArchive).push_back(_filetype);

    // We write 1000-8 bytes for future use:
    for (int i=0;i<992;i++)
    {
        if (theArchive!=nullptr)
            (*theArchive) << (char)0;
        else
            (*_bufferArchive).push_back((char)0);
    }
}

std::string CSer::getFilenamePath() const
{
    std::string retVal;
    size_t pos=_filename.find_last_of("/");
    if (pos!=std::string::npos)
        retVal.assign(_filename.begin(),_filename.begin()+pos+1);
    return(retVal);
}

std::string CSer::getFilenameBase() const
{
    std::string retVal;
    size_t pos1=_filename.find_last_of("/");
    size_t pos2=_filename.find_last_of(".");
    if (pos1==std::string::npos)
        pos1=-1;
    if (_filename.size()>4)
        retVal.assign(_filename.begin()+pos1+1,_filename.begin()+pos2);
    return(retVal);
}

void CSer::_writeXmlHeader()
{
    bool exhaustiveXml=( (_filetype!=filetype_csim_xml_simplescene_file)&&(_filetype!=filetype_csim_xml_simplemodel_file) );
    xmlNode* mainNode=_xmlCreateNode("CoppeliaSim");
    _xmlPushNode(mainNode);
    std::string str("unknown");
    if (_filetype==filetype_csim_xml_xscene_file)
        str="exhaustiveScene";
    if (_filetype==filetype_csim_xml_xmodel_file)
        str="exhaustiveModel";
    if (_filetype==filetype_csim_xml_simplescene_file)
        str="simpleScene";
    if (_filetype==filetype_csim_xml_simplemodel_file)
        str="simpleModel";
    xmlAddNode_comment("",exhaustiveXml);
    xmlAddNode_comment(" All tags are optional, unless otherwise specified ",exhaustiveXml);
    xmlAddNode_comment(" Linear values are specified in meters, angular values in degrees, unless otherwise specified ",exhaustiveXml);
    xmlAddNode_comment("",exhaustiveXml);
    xmlAddNode_comment(" 'filetype' tag: required. Can be 'simpleScene' or 'simpleModel' ",exhaustiveXml);
    xmlAddNode_string("filetype",str.c_str());
    xmlAddNode_comment(" 'xmlSerializationNb' tag: required. Set to 1 ",exhaustiveXml);
    xmlAddNode_int("xmlSerializationNb",XML_XSERIALIZATION_VERSION);

    // Following not required for simple scenes/models:
    xmlAddNode_int("prgFlavor",CSimFlavor::ver());
    xmlAddNode_int("prgVer",SIM_PROGRAM_VERSION_NB);
    xmlAddNode_int("prgRev",SIM_PROGRAM_REVISION_NB);
}

void CSer::_writeXmlFooter()
{
    _xmlDocument.SaveFile(_filename.c_str(),false);
}

std::string CSer::_getNodeText(const xmlNode* node) const
{
    std::string retVal;
    const char* txt=node->GetText();

    if (txt!=nullptr)
        retVal=txt;
    return(retVal);
}

std::string CSer::_getNodeCdataText(const xmlNode* node) const
{
    std::string retVal;
    const sim::tinyxml2::XMLNode* _node=node->FirstChild();
    while(_node!=nullptr)
    {
        const sim::tinyxml2::XMLText* txt=_node->ToText();
        if (txt!=nullptr)
        {
            std::string tx(_node->Value());
            if (txt->CData())
            {
                if ( (tx.size()>1)&&(tx[0]=='\n')&&(tx[tx.size()-1]=='\n') )
                {
                    tx.erase(tx.begin(),tx.begin()+1);
                    tx.erase(tx.end()-1,tx.end());
                }
            }
            retVal+=tx;
        }
        _node=_node->NextSibling();
    }
    return(retVal);
}

std::string CSer::xmlGetStackString() const
{
    std::string retVal;
    for (size_t i=0;i<_xmlNodes.size();i++)
    {
        if (i!=0)
            retVal+=" --> ";
        retVal+=_xmlNodes[i]->Name();
    }
    return(retVal);
}

int CSer::_readXmlHeader(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,char& revNumber)
{
    if (_xmlDocument.LoadFile(_filename.c_str())==sim::tinyxml2::XML_NO_ERROR)
    {
        if (xmlPushChildNode("CoppeliaSim"))
        {
            std::string str;
            int prgVer=-1;
            int prgRev=-1;
            serializationVersion=-1;
            if (xmlGetNode_string("filetype",str))
            {
                int tp=-1;
                if (str.compare("simpleScene")==0)
                    tp=0;
                if (str.compare("simpleModel")==0)
                    tp=0;
                if (str.compare("exhaustiveScene")==0)
                {
                    _filetype=filetype_csim_xml_xscene_file;
                    tp=1;
                }
                if (str.compare("exhaustiveModel")==0)
                {
                    _filetype=filetype_csim_xml_xmodel_file;
                    tp=1;
                }
                if ( xmlGetNode_int("xmlSerializationNb",serializationVersion) && xmlGetNode_int("prgVer",prgVer,tp==1) && xmlGetNode_int("prgRev",prgRev,tp==1) )
                {
                    coppeliaSimVersionThatWroteThis=prgVer;
                    revNumber=prgRev;
                }
                if (tp>=0)
                    return(1);
            }
        }
    }
    return(-3);
}
int CSer::readOpenBinaryNoHeader()
{
    int retVal=-3;
    _storing=false;
    if (_filetype==filetype_bin_file)
    {
        theFile=new VFile(_filename.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
        if (theFile->getFile()!=nullptr)
        {
            theArchive=new VArchive(theFile,VArchive::LOAD);
            unsigned long l=(unsigned long)theArchive->getFile()->getLength();
            char dummy;
            for (unsigned long i=0;i<l;i++)
            {
                (*theArchive) >> dummy;
                _fileBuffer.push_back(dummy);
            }
            retVal=1;
        }
        else
        {
            delete theFile;
            theFile=nullptr;
        }
    }
    if (_filetype==filetype_bin_buff)
        retVal=1;
    return(retVal);
}

int CSer::readOpenXml(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,int& licenseTypeThatWroteThis,char& revNumber,bool ignoreTooOldSerializationVersion)
{ // return values: -4 file can't be opened, -3=wrong fileformat, -2=format too old, -1=format too new, 0=compressor unknown, 1=alright!
    _storing=false;
    coppeliaSimVersionThatWroteThis=0; // means: not yet supported
    licenseTypeThatWroteThis=-1; // means: not yet supported
    serializationVersion=-1; // error
    _serializationVersionThatWroteThisFile=serializationVersion;
    _serializationVersionThatWroteLastFile=serializationVersion;
    return(_readXmlHeader(serializationVersion,coppeliaSimVersionThatWroteThis,revNumber));
}
int CSer::readOpenBinary(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,int& licenseTypeThatWroteThis,char& revNumber,bool ignoreTooOldSerializationVersion)
{ // return values: -4 file can't be opened, -3=wrong fileformat, -2=format too old, -1=format too new, 0=compressor unknown, 1=alright!
    _storing=false;
    if ( (_filetype!=filetype_csim_bin_scene_buff)&&(_filetype!=filetype_csim_bin_model_buff) )
    {
        theFile=new VFile(_filename.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
        if (theFile->getFile()!=nullptr)
            theArchive=new VArchive(theFile,VArchive::LOAD);
        else
        {
            delete theFile;
            theFile=nullptr;
            return(-4);
        }
    }

    coppeliaSimVersionThatWroteThis=0; // means: not yet supported
    licenseTypeThatWroteThis=-1; // means: not yet supported
    serializationVersion=-1; // error
    _serializationVersionThatWroteThisFile=serializationVersion;
    _serializationVersionThatWroteLastFile=serializationVersion;
    int minSerializationVersionThatCanReadThis=-1; // error
    int compilationVersion=-1;
    int alreadyReadDataCount=0;
    char filetype=0;
    char compressMethod=0;
    int originalDataSize=0;
    int bufferArchivePointer=0;

    if (!_noHeader)
    {
        // We try to read the header:
        if (theArchive!=nullptr)
        {
            VFile* theFile=theArchive->getFile();
            if (theFile->getLength()<strlen(SER_SIM_HEADER))
                return(-3); // wrong fileformat
        }
        else
        {
            if ((*_bufferArchive).size()<strlen(SER_SIM_HEADER))
                return(-3); // wrong fileformat
        }
        std::string head;
        for (size_t i=0;i<strlen(SER_SIM_HEADER);i++)
        {
            char tmp;
            if (theArchive!=nullptr)
                (*theArchive) >> tmp;
            else
                tmp=(*_bufferArchive)[bufferArchivePointer++];
            head+=tmp;
        }
        if (head!=SER_SIM_HEADER)
            return(-3); // we don't have the appropriate header! (wrong fileformat)
        else
        { // We have the correct header!
            // We read the serialization version:
            if (theArchive!=nullptr)
            {
                (*theArchive) >> ((char*)&serializationVersion)[0];
                (*theArchive) >> ((char*)&serializationVersion)[1];
                (*theArchive) >> ((char*)&serializationVersion)[2];
                (*theArchive) >> ((char*)&serializationVersion)[3];
                (*theArchive) >> ((char*)&minSerializationVersionThatCanReadThis)[0];
                (*theArchive) >> ((char*)&minSerializationVersionThatCanReadThis)[1];
                (*theArchive) >> ((char*)&minSerializationVersionThatCanReadThis)[2];
                (*theArchive) >> ((char*)&minSerializationVersionThatCanReadThis)[3];
            }
            else
            {
                ((char*)&serializationVersion)[0]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&serializationVersion)[1]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&serializationVersion)[2]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&serializationVersion)[3]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&minSerializationVersionThatCanReadThis)[0]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&minSerializationVersionThatCanReadThis)[1]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&minSerializationVersionThatCanReadThis)[2]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&minSerializationVersionThatCanReadThis)[3]=(*_bufferArchive)[bufferArchivePointer++];
            }
            _serializationVersionThatWroteThisFile=serializationVersion;
            _serializationVersionThatWroteLastFile=serializationVersion;
            // We read the compression method:
            if (theArchive!=nullptr)
                (*theArchive) >> compressMethod;
            else
                compressMethod=(*_bufferArchive)[bufferArchivePointer++];
            // We read the uncompressed data size:
            if (theArchive!=nullptr)
            {
                (*theArchive) >> ((char*)&originalDataSize)[0];
                (*theArchive) >> ((char*)&originalDataSize)[1];
                (*theArchive) >> ((char*)&originalDataSize)[2];
                (*theArchive) >> ((char*)&originalDataSize)[3];
            }
            else
            {
                ((char*)&originalDataSize)[0]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&originalDataSize)[1]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&originalDataSize)[2]=(*_bufferArchive)[bufferArchivePointer++];
                ((char*)&originalDataSize)[3]=(*_bufferArchive)[bufferArchivePointer++];
            }

            alreadyReadDataCount=17; // this is for ser version 12, ser version 13 has additional 1004!! (added a bit further down)
            if (serializationVersion>12)
            { // for serialization version 13 and above! (2009/07/21)
                if (theArchive!=nullptr)
                {
                    (*theArchive) >> ((char*)&compilationVersion)[0];
                    (*theArchive) >> ((char*)&compilationVersion)[1];
                    (*theArchive) >> ((char*)&compilationVersion)[2];
                    (*theArchive) >> ((char*)&compilationVersion)[3];
                }
                else
                {
                    ((char*)&compilationVersion)[0]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&compilationVersion)[1]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&compilationVersion)[2]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&compilationVersion)[3]=(*_bufferArchive)[bufferArchivePointer++];
                }

                // We read the license version that wrote this file and the CoppeliaSim version:
                if (theArchive!=nullptr)
                {
                    (*theArchive) >> ((char*)&coppeliaSimVersionThatWroteThis)[0];
                    (*theArchive) >> ((char*)&coppeliaSimVersionThatWroteThis)[1];
                    unsigned int licenseTypeThatWroteThisTmp;
                    (*theArchive) >> ((char*)&licenseTypeThatWroteThisTmp)[0];
                    (*theArchive) >> ((char*)&licenseTypeThatWroteThisTmp)[1];
                    (*theArchive) >> ((char*)&licenseTypeThatWroteThisTmp)[2];
                    (*theArchive) >> ((char*)&licenseTypeThatWroteThisTmp)[3];
                    licenseTypeThatWroteThis=licenseTypeThatWroteThisTmp-1; // -1 because -1 means: no info about license type yet!
                }
                else
                {
                    ((char*)&coppeliaSimVersionThatWroteThis)[0]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&coppeliaSimVersionThatWroteThis)[1]=(*_bufferArchive)[bufferArchivePointer++];
                    unsigned int licenseTypeThatWroteThisTmp;
                    ((char*)&licenseTypeThatWroteThisTmp)[0]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&licenseTypeThatWroteThisTmp)[1]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&licenseTypeThatWroteThisTmp)[2]=(*_bufferArchive)[bufferArchivePointer++];
                    ((char*)&licenseTypeThatWroteThisTmp)[3]=(*_bufferArchive)[bufferArchivePointer++];
                    licenseTypeThatWroteThis=licenseTypeThatWroteThisTmp-1; // -1 because -1 means: no info about license type yet!
                }

                // We read the revision number:
                if (theArchive!=nullptr)
                    (*theArchive) >> revNumber;
                else
                    revNumber=(*_bufferArchive)[bufferArchivePointer++];

                if (theArchive!=nullptr)
                    (*theArchive) >> filetype;
                else
                    filetype=(*_bufferArchive)[bufferArchivePointer++];

                for (int i=0;i<992;i++)
                { // for future use!
                    char dummy;
                    if (theArchive!=nullptr)
                        (*theArchive) >> dummy; // not used for now
                    else
                        dummy=(*_bufferArchive)[bufferArchivePointer++];
                }
                alreadyReadDataCount+=1004;
            }
        }


        _coppeliaSimVersionThatWroteThis=coppeliaSimVersionThatWroteThis;
        _licenseTypeThatWroteThis=licenseTypeThatWroteThis;

        if (!ignoreTooOldSerializationVersion) // we can most of the time ignore a too old serialization number, if we only want to load the thumbnail
        {
            if (serializationVersion<SER_MIN_SERIALIZATION_VERSION_THAT_THIS_CAN_READ)
                return(-2); // This file is too old
        }
        if (minSerializationVersionThatCanReadThis>SER_SERIALIZATION_VERSION)
            return(-1); // This file is too new
        if (serializationVersion>SER_SERIALIZATION_VERSION)
        { // we might have problems reading this (even if it should be supported). Some functions might not be available.
#ifdef SIM_WITH_GUI
            App::uiThread->messageBox_warning(App::mainWindow,"Serialization",IDS_READING_NEWER_SERIALIZATION_FILE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#else
            App::logMsg(sim_verbosity_warnings,"%s.",IDS_READING_NEWER_SERIALIZATION_FILE_WARNING);
#endif
        }
    }

    // We read the whole file:
    if (theArchive!=nullptr)
    {
        unsigned long l=(unsigned long)theArchive->getFile()->getLength()-alreadyReadDataCount;
        char dummy;
        for (unsigned long i=0;i<l;i++)
        {
            (*theArchive) >> dummy;
            _fileBuffer.push_back(dummy);
        }
    }
    else
    {
        for (unsigned long i=bufferArchivePointer;i<(*_bufferArchive).size();i++)
            _fileBuffer.push_back((*_bufferArchive)[i]);
    }

    if (compressMethod!=0)
    { // compressed
        if (compressMethod==1) // for now, only Huffman is supported
        { // Huffman uncompression:
            unsigned char* uncompressedBuffer=new unsigned char[originalDataSize];
            Huffman_Uncompress(&_fileBuffer[0],uncompressedBuffer,(int)_fileBuffer.size(),originalDataSize);
            _fileBuffer.clear();
            for (int i=0;i<originalDataSize;i++)
                _fileBuffer.push_back(uncompressedBuffer[i]);
            delete[] uncompressedBuffer;

            return(CSimFlavor::handleReadOpenFile(_filetype,(char*)&_fileBuffer[0]));
        }
    }
    else
        return(1); // everything went ok!
    return(0); // error, unknown compressor
}

void CSer::readClose()
{
    _fileBuffer.clear();
}


char CSer::getFileType() const
{
    return(_filetype);
}

bool CSer::isStoring() const
{
    return(_storing);
}

bool CSer::isBinary() const
{
    return( (_filetype!=filetype_csim_xml_xscene_file)&&(_filetype!=filetype_csim_xml_xmodel_file)&&(_filetype!=filetype_csim_xml_simplescene_file)&&(_filetype!=filetype_csim_xml_simplemodel_file) );
}

void CSer::disableCountingModeExceptForExceptions()
{
    _coutingModeDisabledExceptForExceptions=true;
}

void CSer::setCountingMode(bool force)
{ // force is false by default
    if ((!_coutingModeDisabledExceptForExceptions)||force)
    {
        countingMode++;
        if (countingMode==1)
            counter=0;
    }
    else
    { // here we write a dummy value!!
        for (int i=0;i<int(sizeof(counter));i++)
            _fileBuffer.push_back(((char*)&counter)[i]);
    }
}

bool CSer::setWritingMode(bool force)
{ // force is false by default
    if ((!_coutingModeDisabledExceptForExceptions)||force)
    {
        countingMode--;
        if (countingMode==0)
        {
            for (int i=0;i<int(sizeof(counter));i++)
                _fileBuffer.push_back(((char*)&counter)[i]);
            counter=0;
            return(true);
        }
        else
        {
            counter+=sizeof(counter);
            return(false);
        }
    }
    else
        return(false);
}

int CSer::getCounter() const
{
    return(counter);
}

void CSer::loadUnknownData()
{ // If return value is true, a warning message should be displayed
    int quantity;
    for (size_t i=0;i<sizeof(quantity);i++)
        ((char*)&quantity)[i]=_fileBuffer[_fileBufferReadPointer++];
    _foundUnknownCommands=true;
    _fileBufferReadPointer+=quantity;
}

bool CSer::getFoundUnknownCommands()
{
    return(_foundUnknownCommands);
}

std::vector<unsigned char>* CSer::getBufferPointer()
{
    return(&buffer);
}

CSer& CSer::operator<< (const int& v)
{
    buffer.push_back(((unsigned char*)&v)[0]);
    buffer.push_back(((unsigned char*)&v)[1]);
    buffer.push_back(((unsigned char*)&v)[2]);
    buffer.push_back(((unsigned char*)&v)[3]);
    return(*this);
}

#ifdef NEWOPERATION
CSerTmp& CSer::flt()
#else
CSer& CSer::flt()
#endif
{
    return(*this);
}

#ifdef NEWOPERATION
CSerTmp& CSer::dbl()
#else
CSer& CSer::dbl()
#endif
{
    return(*this);
}

#ifndef NEWOPERATION
CSer& CSer::operator<< (const float& v)
{
    buffer.push_back(((unsigned char*)&v)[0]);
    buffer.push_back(((unsigned char*)&v)[1]);
    buffer.push_back(((unsigned char*)&v)[2]);
    buffer.push_back(((unsigned char*)&v)[3]);
    return(*this);
}

CSer& CSer::operator<< (const double& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        buffer.push_back(tmp[i]);
    return(*this);
}
#endif

CSer& CSer::operator<< (const unsigned short& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        buffer.push_back(tmp[i]);
    return(*this);
}

CSer& CSer::operator<< (const unsigned int& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        buffer.push_back(tmp[i]);
    return(*this);
}

CSer& CSer::operator<< (const quint64& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        buffer.push_back(tmp[i]);
    return(*this);
}

CSer& CSer::operator<< (const long& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        buffer.push_back(tmp[i]);
    return(*this);
}

CSer& CSer::operator<< (const unsigned char& v)
{
    buffer.push_back(v);
    return(*this);
}

CSer& CSer::operator<< (const char& v)
{
    buffer.push_back(v);
    return(*this);
}

CSer& CSer::operator<< (const std::string& v)
{
    (*this) << ((int)v.length());
    for (int i=0;i<int(v.length());i++)
        (*this) << ((unsigned char)v[i]);
    return(*this);
}

std::vector<unsigned char>* CSer::getFileBuffer()
{
    return(&_fileBuffer);
}

int CSer::getFileBufferReadPointer() const
{
    return(_fileBufferReadPointer);
}

void CSer::addOffsetToFileBufferReadPointer(int off)
{
    _fileBufferReadPointer+=off;
}

CSer& CSer::operator>> (int& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

#ifndef NEWOPERATION
CSer& CSer::operator>> (float& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (double& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}
#endif

CSer& CSer::operator>> (unsigned short& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (unsigned int& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (quint64& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (long& v)
{
    unsigned char* tmp=(unsigned char*)(&v);
    for (int i=0;i<int(sizeof(v));i++)
        tmp[i]=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (unsigned char& v)
{
    v=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (char& v)
{
    v=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

CSer& CSer::operator>> (std::string& v)
{
    v="";
    int l;
    for (int i=0;i<int(sizeof(l));i++)
        ((char*)&l)[i]=_fileBuffer[_fileBufferReadPointer++];
    for (int i=0;i<l;i++)
        v+=_fileBuffer[_fileBufferReadPointer++];
    return(*this);
}

void CSer::flush(bool writeNbOfBytes)
{ // writeNbOfBytes is true by default
    if (countingMode==0)
    {
        if (writeNbOfBytes)
        {
            int s=(int)buffer.size();
            for (int i=0;i<int(sizeof(s));i++)
                _fileBuffer.push_back(((char*)&s)[i]);
        }
        for (int i=0;i<int(buffer.size());i++)
            _fileBuffer.push_back(buffer[i]);
        buffer.clear();
    }
    else
    {
        if (writeNbOfBytes)
            counter+=sizeof(int)+(int)buffer.size();
        else
            counter+=(int)buffer.size();
        buffer.clear();
    }
}

void CSer::storeDataName(const char* name)
{
    if (isBinary())
    {
        while (strlen(name)!=3); // freezes
        if (countingMode!=0)
            counter+=3;
        else
        {
            for (int i=0;i<3;i++)
                _fileBuffer.push_back(name[i]);
        }
    }
}

std::string CSer::readDataName()
{
    std::string retS="";
    for (int i=0;i<3;i++)
        retS+=_fileBuffer[_fileBufferReadPointer++];
    return(retS);
}

int CSer::readBytesButKeepPointerUnchanged(unsigned char* buffer,int desiredCount)
{
    int left=int(_fileBuffer.size())-_fileBufferReadPointer;
    if (left<desiredCount)
        desiredCount=left; // we can't read as much as we want!
    for (int i=0;i<desiredCount;i++)
        buffer[i]=_fileBuffer[_fileBufferReadPointer+i];
    return(desiredCount); // we return the nb of bytes we could read
}

VArchive& CSer::getArchive()
{
    return(*theArchive);
}

unsigned short CSer::getCoppeliaSimVersionThatWroteThisFile()
{
    return(_coppeliaSimVersionThatWroteThis);
}

int CSer::getLicenseTypeThatWroteThisFile()
{
    return(_licenseTypeThatWroteThis);
}

int CSer::getSerializationVersionThatWroteThisFile()
{
    return(_serializationVersionThatWroteThisFile);
}

int CSer::getSerializationVersionThatWroteLastFile()
{
    return(_serializationVersionThatWroteLastFile);
}

int CSer::getIncrementCounter()
{
    return(_multiPurposeCounter++);
}

void CSer::clearIncrementCounter()
{
    _multiPurposeCounter=0;
}

bool CSer::xmlSaveDataInline(size_t bufferSize) const
{
    return( (_xmlMaxInlineBufferSize>=int(bufferSize))||(_xmlMaxInlineBufferSize<=0) );
}

void CSer::warnMissingNode(const char* name) const
{
    if ( (App::getStatusbarVerbosity()>=sim_verbosity_debug)||CSimFlavor::getBoolVal(18) )
        App::logMsg(sim_verbosity_warnings,"XML read: missing node '%s' (stack: %s).",name,xmlGetStackString().c_str());
}

xmlNode* CSer::_xmlCreateNode(const char* name)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    return(node);
}

xmlNode* CSer::_xmlCreateNode(const char* name,const char* nameAttribute)
{
    xmlNode* node=_xmlCreateNode(name);
    node->SetAttribute("name",nameAttribute);
    return(node);
}

xmlNode* CSer::_xmlCreateNode(const char* name,int idAttribute)
{
    xmlNode* node=_xmlCreateNode(name);
    node->SetAttribute("id",idAttribute);
    return(node);
}

xmlNode* CSer::xmlPushNewNode(const char* name)
{
    xmlNode* node=_xmlCreateNode(name);
    _xmlPushNode(node);
    return(node);
}

xmlNode* CSer::xmlPushNewNode(const char* name,const char* nameAttribute)
{
    xmlNode* node=_xmlCreateNode(name,nameAttribute);
    _xmlPushNode(node);
    return(node);
}

xmlNode* CSer::xmlPushNewNode(const char* name,int idAttribute)
{
    xmlNode* node=_xmlCreateNode(name,idAttribute);
    _xmlPushNode(node);
    return(node);
}

void CSer::_xmlPushNode(xmlNode* node)
{
    if (_xmlCurrentNode==nullptr)
        _xmlDocument.InsertFirstChild(node);
    else
        _xmlCurrentNode->InsertEndChild(node);
    _xmlCurrentNode=node;
    _xmlNodes.push_back(node);
}

void CSer::xmlPopNode()
{
    if (_xmlNodes.size()>0)
    {
        _xmlNodes.pop_back();
        if (_xmlNodes.size()>0)
            _xmlCurrentNode=_xmlNodes[_xmlNodes.size()-1];
        else
            _xmlCurrentNode=nullptr;
    }
}

xmlNode* CSer::xmlGetCurrentNode()
{
    return(_xmlCurrentNode);
}

void CSer::xmlAddNode_comment(const char* comment,bool doNotInsert)
{
    if (!doNotInsert)
        _xmlCurrentNode->InsertEndChild(_xmlDocument.NewComment(comment));
}

void CSer::xmlAddNode_bool(const char* name,bool val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt;
    if (val)
        txt=_xmlDocument.NewText("true");
    else
        txt=_xmlDocument.NewText("false");
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_bools(const char* name,const std::vector<bool>& vals)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (i>0)
            tmp+=" ";
        if (vals[i])
            tmp+="true";
        else
            tmp+="false";
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_binFile(const char* name,const char* localFilenameSuffix,const unsigned char* buff,size_t buffSize)
{
    std::string fn(getFilenameBase()+"_"+localFilenameSuffix+".bin");
    xmlNode* node=_xmlCreateNode(name,fn.c_str());
    _xmlPushNode(node);
    xmlPopNode();

    CSer serObj((getFilenamePath()+fn).c_str(),filetype_bin_file);
    serObj.writeOpenBinaryNoHeader(false);
    for (size_t i=0;i<buffSize;i++)
        serObj << buff[i];
    serObj.flush();
    serObj.writeClose();
}

CSer* CSer::xmlAddNode_binFile(const char* name,const char* localFilenameSuffix)
{
    std::string fn(getFilenameBase()+"_"+localFilenameSuffix+".bin");
    xmlNode* node=_xmlCreateNode(name,fn.c_str());
    _xmlPushNode(node);
    xmlPopNode();

    CSer* serObj=new CSer((getFilenamePath()+fn).c_str(),filetype_bin_file);
    serObj->writeOpenBinaryNoHeader(false);
    return(serObj);
}

void CSer::xmlAddNode_imageFile(const char* name,const char* localFilenameSuffix,const unsigned char* img,int resX,int resY,bool rgba)
{
    if (_xmlUseImageAndMeshFileformats)
    {
        std::string fn(getFilenameBase()+"_"+localFilenameSuffix+".png");
        xmlNode* node=_xmlCreateNode(name,fn.c_str());
        _xmlPushNode(node);
        xmlPopNode();

        int res[2]={resX,resY};
        int options=0;
        if (rgba)
            options=1;
        CImageLoaderSaver::save(img,res,options,(getFilenamePath()+fn).c_str(),100,nullptr);
    }
    else
    {
        std::vector<unsigned char> buff;
        for (size_t i=0;i<4;i++)
            buff.push_back(((unsigned char*)&resX)[i]);
        for (size_t i=0;i<4;i++)
            buff.push_back(((unsigned char*)&resY)[i]);
        int s=resX*resY;
        if (rgba)
        {
            s*=4;
            buff.push_back(1);
        }
        else
        {
            s*=3;
            buff.push_back(0);
        }
        for (size_t i=0;i<s;i++)
            buff.push_back(img[i]);
        xmlAddNode_binFile(name,localFilenameSuffix,&buff[0],s);
    }
}

void CSer::xmlAddNode_meshFile(const char* name,const char* localFilenameSuffix,const float* vertices,int vl,const int* indices,int il,const float* normals,int nl,const unsigned char* edges,int el)
{
    bool exhaustiveXml=( (getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
    if ( _xmlUseImageAndMeshFileformats&&CPluginContainer::isAssimpPluginAvailable()&&(!exhaustiveXml) )
    {
        std::string fn(getFilenameBase()+"_"+localFilenameSuffix+".ply");
        xmlNode* node=_xmlCreateNode(name,fn.c_str());
        _xmlPushNode(node);
        xmlPopNode();
        float** _vertices;
        int* _verticesSizes;
        int** _indices;
        int* _indicesSizes;
        _vertices=new float*[1];
        _verticesSizes=new int[1];
        _indices=new int*[1];
        _indicesSizes=new int[1];
        _vertices[0]=(float*)vertices;
        _verticesSizes[0]=vl;
        _indices[0]=(int*)indices;
        _indicesSizes[0]=il;
        CPluginContainer::assimp_exportMeshes(1,(const float**)_vertices,_verticesSizes,(const int**)_indices,_indicesSizes,(getFilenamePath()+fn).c_str(),"ply",1.0f,1,256);
        delete[] _vertices;
        delete[] _verticesSizes;
        delete[] _indices;
        delete[] _indicesSizes;
    }
    else
    {
        CSer* w=xmlAddNode_binFile(name,localFilenameSuffix);
        w[0] << vl;
        for (size_t i=0;i<vl;i++)
            w[0] << vertices[i];
        w[0] << il;
        for (size_t i=0;i<il;i++)
            w[0] << indices[i];
        w[0] << nl;
        for (size_t i=0;i<nl;i++)
            w[0] << normals[i];
        w[0] << el;
        for (size_t i=0;i<el;i++)
            w[0] << edges[i];
        w->flush();
        w->writeClose();
        delete w;
    }
}

void CSer::xmlAddNode_string(const char* name,const char* str)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(str);
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_strings(const char* name,const std::vector<std::string>& vals)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%s") % vals[i].c_str());
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_cdata(const char* name,const char* str)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);

    std::string s(str);
    size_t p0=0;
    size_t p1=s.find("]]>",p0);
    while (p1!=std::string::npos)
    {
        std::string _s(s.begin()+p0,s.begin()+p1);
        _s="\n"+_s+"\n";
        sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(_s.c_str());
        txt->SetCData(true);
        node->InsertEndChild(txt);
        sim::tinyxml2::XMLText* txt2=_xmlDocument.NewText("]]>");
        node->InsertEndChild(txt2);
        p0=p1+3;
        p1=s.find("]]>",p0);
    }
    std::string _s(s.begin()+p0,s.end());
    _s="\n"+_s+"\n";
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(_s.c_str());
    txt->SetCData(true);
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_enum(const char* name,int val,int v1,const char* str1,int v2,const char* str2,int v3/*=-1*/,const char* str3/*=nullptr*/,int v4/*=-1*/,const char* str4/*=nullptr*/,int v5/*=-1*/,const char* str5/*=nullptr*/,int v6/*=-1*/,const char* str6/*=nullptr*/,int v7/*=-1*/,const char* str7/*=nullptr*/,int v8/*=-1*/,const char* str8/*=nullptr*/,int v9/*=-1*/,const char* str9/*=nullptr*/)
{
    std::string tmp;
    if (val==v1)
        tmp=str1;
    if (val==v2)
        tmp=str2;
    if ( (str3!=nullptr)&&(val==v3) )
        tmp=str3;
    if ( (str4!=nullptr)&&(val==v4) )
        tmp=str4;
    if ( (str5!=nullptr)&&(val==v5) )
        tmp=str5;
    if ( (str6!=nullptr)&&(val==v6) )
        tmp=str6;
    if ( (str7!=nullptr)&&(val==v7) )
        tmp=str7;
    if ( (str8!=nullptr)&&(val==v8) )
        tmp=str8;
    if ( (str9!=nullptr)&&(val==v9) )
        tmp=str9;
    if (tmp.size()==0)
        xmlAddNode_int(name,val);
    else
        xmlAddNode_string(name,tmp.c_str());
}

void CSer::xmlAddNode_enum(const char* name,int val,const std::vector<int>& vals,const std::vector<std::string>& strings)
{
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (val==vals[i])
            tmp=strings[i];
    }
    if (tmp.size()==0)
        xmlAddNode_int(name,val);
    else
        xmlAddNode_string(name,tmp.c_str());
}

void CSer::xmlAddNode_int(const char* name,int val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%i") % val).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_2int(const char* name,int val1,int val2)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%i %i") % val1 % val2).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_3int(const char* name,int val1,int val2,int val3)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%i %i %i") % val1 % val2 %val3).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_ints(const char* name,const int* vals,size_t cnt)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<cnt;i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%i") % vals[i]);
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_ints(const char* name,const std::vector<int>& vals)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%i") % vals[i]);
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_uint(const char* name,unsigned int val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%u") % val).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_ulonglong(const char* name,unsigned long long val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%u") % val).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_uchars(const char* name,const std::vector<unsigned char>& vals)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%u") % (unsigned int)vals[i]);
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_float(const char* name,float val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%f") % val).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_2float(const char* name,float val1,float val2)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%f %f") % val1 % val2).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_3float(const char* name,float val1,float val2,float val3)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%f %f %f") % val1 % val2 %val3).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_4float(const char* name,float val1,float val2,float val3,float val4)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%f %f %f %f") % val1 % val2 %val3 %val4).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_floats(const char* name,const float* vals,size_t cnt)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<cnt;i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%f") % vals[i]);
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_floats(const char* name,const std::vector<float>& vals)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    std::string tmp;
    for (size_t i=0;i<vals.size();i++)
    {
        if (i>0)
            tmp+=" ";
        tmp+=boost::str(boost::format("%f") % vals[i]);
    }
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(tmp.c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlAddNode_double(const char* name,double val)
{
    xmlNode* node=_xmlDocument.NewElement(name);
    _xmlCurrentNode->InsertEndChild(node);
    sim::tinyxml2::XMLText* txt=_xmlDocument.NewText(boost::str(boost::format("%f") % val).c_str());
    node->InsertEndChild(txt);
}

void CSer::xmlGetAllChildNodeNames(std::vector<std::string>& allNames)
{
    xmlNode* node=nullptr;
    if (_xmlCurrentNode==nullptr)
        node=_xmlDocument.FirstChildElement();
    else
        node=_xmlCurrentNode->FirstChildElement();
    while (node!=nullptr)
    {
        allNames.push_back(node->Name());
        node=node->NextSiblingElement();
    }
}

bool CSer::xmlPushChildNode(const char* name,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlPushChildNode, name: %s",name);
    xmlNode* node=nullptr;
    if (_xmlCurrentNode==nullptr)
        node=_xmlDocument.FirstChildElement(name);
    else
        node=_xmlCurrentNode->FirstChildElement(name);
    if (node==nullptr)
    {
        if (required)
            warnMissingNode(name);
        return(false);
    }
    _xmlCurrentNode=node;
    _xmlNodes.push_back(node);
    return(true);
}

bool CSer::xmlPushSiblingNode(const char* name,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlPushSiblingNode, name: %s",name);
    if (_xmlCurrentNode!=nullptr)
    {
        xmlNode* node=_xmlCurrentNode->NextSiblingElement(name);
        if (node!=nullptr)
        {
            _xmlNodes.pop_back();
            _xmlCurrentNode=node;
            _xmlNodes.push_back(node);
            return(true);
        }
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_nameAttribute(std::string& val,bool required/*=true*/)
{
    const char* nm=_xmlCurrentNode->Attribute("name");
    if (nm!=nullptr)
        val=nm;
    if (nm==nullptr)
    {
        if (required)
            App::logMsg(sim_verbosity_warnings,"XML read: missing attribute 'name' (stack: %s).",xmlGetStackString().c_str());
    }
    return(nm!=nullptr);
}

bool CSer::xmlGetNode_idAttribute(int& val,bool required/*=true*/)
{
    val=_xmlCurrentNode->IntAttribute("id");
    return(true);
}

bool CSer::xmlGetNode_bool(const char* name,bool& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_bool, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        if (str.compare("true")==0)
            val=true;
        else
            val=false;
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_bools(const char* name,std::vector<bool>& vals,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_bools, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        while (true)
        {
            if (ss >> buff)
            {
                if (buff.compare("true")==0)
                    vals.push_back(true);
                else
                {
                    if (buff.compare("false")==0)
                        vals.push_back(false);
                    else
                    {
                        if (required)
                            App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                        vals.clear();
                        return(false);
                    }
                }
            }
            else
                break;
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_flags(const char* name,int& flags,int flagWhenTrue,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_flags, name: %s",name);
    bool v;
    if (xmlGetNode_bool(name,v,required))
    {
        if (v)
            flags|=flagWhenTrue;
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_binFile(const char* name,std::vector<unsigned char>& buffer,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_binFile, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        const char* str=node->Attribute("name");
        if (str!=nullptr)
        {
            std::string filename(getFilenamePath()+str);
            CSer serObj(filename.c_str(),filetype_bin_file);
            if (serObj.readOpenBinaryNoHeader()==1)
            {
                int s;
                serObj >> s;
                buffer.resize(s);
                for (int i=0;i<s;i++)
                    serObj >> buffer[i];
                serObj.readClose();
                return(true);
            }
            App::logMsg(sim_verbosity_errors,"XML read: file '%s' can't be opened.",filename.c_str());
            return(false);
        }
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_binFile(const char* name,std::string& buffer,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_binFile, name: %s",name);
    std::vector<unsigned char> buff;
    bool retVal=xmlGetNode_binFile(name,buff,required);
    if (retVal)
    {
        buffer.resize(buff.size());
        for (size_t i=0;i<buff.size();i++)
            buffer[i]=buff[i];
    }
    return(retVal);
}

bool CSer::xmlGetNode_imageFile(const char* name,std::vector<unsigned char>& image,int* resX/*=nullptr*/,int* resY/*=nullptr*/,bool* rgba/*=nullptr*/,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_imageFile, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        const char* str=node->Attribute("name");
        if (str!=nullptr)
        {
            std::string filename(getFilenamePath()+str);
            if (boost::algorithm::ends_with(str,".png"))
            {
                int _resX,_resY,_components;
                unsigned char* img=CImageLoaderSaver::load(filename.c_str(),&_resX,&_resY,&_components,4,0);
                int options=4;
                if (_components==4)
                    options|=1;
                CImageLoaderSaver::transformImage(img,_resX,_resY,options);
                if (img!=nullptr)
                {
                    image.assign(img,img+_resX*_resY*_components);
                    delete[] img;
                    if (resX!=nullptr)
                        resX[0]=_resX;
                    if (resY!=nullptr)
                        resY[0]=_resY;
                    if (rgba!=nullptr)
                    {
                        rgba[0]=false;
                        if (_components==4)
                            rgba[0]=true;
                    }
                    return(true);
                }
            }
            else
            {
                bool retVal=xmlGetNode_binFile(name,image,required);
                if (retVal)
                {
                    if (resX!=nullptr)
                        resX[0]=((int*)&image[0])[0];
                    if (resY!=nullptr)
                        resY[0]=((int*)&image[0])[1];
                    if (rgba!=nullptr)
                    {
                        rgba[0]=false;
                        if (image[8]>0)
                            rgba[0]=true;
                    }
                    image.erase(image.begin(),image.begin()+9);
                }
                return(retVal);
            }
            App::logMsg(sim_verbosity_errors,"XML read: file '%s' can't be opened.",filename.c_str());
            return(false);
        }
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_meshFile(const char* name,std::vector<float>& vertices,std::vector<int>& indices,std::vector<float>& normals,std::vector<unsigned char>& edges,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_meshFile, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        const char* str=node->Attribute("name");
        if (str!=nullptr)
        {
            std::string filename(getFilenamePath()+str);
            if (boost::algorithm::ends_with(str,".ply"))
            {
                bool retVal=false;
                float** _vertices;
                int* _verticesSizes;
                int** _indices;
                int* _indicesSizes;
                if (!CPluginContainer::isAssimpPluginAvailable())
                    App::logMsg(sim_verbosity_errors,"assimp plugin was not found. CoppeliaSim will now crash.");
                int cnt=CPluginContainer::assimp_importMeshes(filename.c_str(),1.0f,1,16+256,&_vertices,&_verticesSizes,&_indices,&_indicesSizes);
                if (cnt>0)
                {
                    if (cnt==1)
                    {
                        for (int i=0;i<_verticesSizes[0];i++)
                            vertices.push_back(_vertices[0][i]);
                        for (int i=0;i<_indicesSizes[0];i++)
                            indices.push_back(_indices[0][i]);
                        retVal=true;
                    }
                    for (int i=0;i<cnt;i++)
                    {
                        delete[] _indices[i];
                        delete[] _vertices[i];
                    }
                    delete[] _indicesSizes;
                    delete[] _indices;
                    delete[] _verticesSizes;
                    delete[] _vertices;
                }
                return(retVal);
            }
            else
            {
                CSer* w=xmlGetNode_binFile("file",required);
                int cnt;
                w[0] >> cnt;
                vertices.resize(cnt);
                for (int i=0;i<cnt;i++)
                    w[0] >> vertices[i];

                w[0] >> cnt;
                indices.resize(cnt);
                for (int i=0;i<cnt;i++)
                    w[0] >> indices[i];

                w[0] >> cnt;
                normals.resize(cnt);
                for (int i=0;i<cnt;i++)
                    w[0] >> normals[i];

                w[0] >> cnt;
                edges.resize(cnt);
                for (int i=0;i<cnt;i++)
                    w[0] >> edges[i];

                w->readClose();
                delete w;
                return(true);
            }
            App::logMsg(sim_verbosity_errors,"XML read: file '%s' can't be opened.",filename.c_str());
            return(false);
        }
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

CSer* CSer::xmlGetNode_binFile(const char* name,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_binFile, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        const char* str=node->Attribute("name");
        if (str!=nullptr)
        {
            std::string filename(getFilenamePath()+str);
            CSer* serObj=new CSer(filename.c_str(),filetype_bin_file);
            if (serObj->readOpenBinaryNoHeader()==1)
            {
                int s;
                serObj[0] >> s;
                return(serObj);
            }
            return(nullptr);
        }
    }
    if (required)
        warnMissingNode(name);
    return(nullptr);
}

bool CSer::xmlGetNode_string(const char* name,std::string& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_string, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        val=_getNodeText(node);
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_strings(const char* name,std::vector<std::string>& vals,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_strings, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        while (true)
        {
            if (ss >> buff)
                vals.push_back(buff);
            else
                break;
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_cdata(const char* name,std::string& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_cdata, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        val=_getNodeCdataText(node);
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_enum(const char* name,int& val,bool required,const char* str1,int v1,const char* str2,int v2,const char* str3/*=nullptr*/,int v3/*=-1*/,const char* str4/*=nullptr*/,int v4/*=-1*/,const char* str5/*=nullptr*/,int v5/*=-1*/,const char* str6/*=nullptr*/,int v6/*=-1*/,const char* str7/*=nullptr*/,int v7/*=-1*/,const char* str8/*=nullptr*/,int v8/*=-1*/,const char* str9/*=nullptr*/,int v9/*=-1*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_enum, name: %s",name);
    std::string tmp;
    bool f=false;
    bool retVal=false;
    if (xmlGetNode_string(name,tmp,required))
    {
        if (tmp.compare(str1)==0)
        {
            val=v1;
            f=true;
        }
        if (tmp.compare(str2)==0)
        {
            val=v2;
            f=true;
        }
        if ( (str3!=nullptr)&&(tmp.compare(str3)==0) )
        {
            val=v3;
            f=true;
        }
        if ( (str4!=nullptr)&&(tmp.compare(str4)==0) )
        {
            val=v4;
            f=true;
        }
        if ( (str5!=nullptr)&&(tmp.compare(str5)==0) )
        {
            val=v5;
            f=true;
        }
        if ( (str6!=nullptr)&&(tmp.compare(str6)==0) )
        {
            val=v6;
            f=true;
        }
        if ( (str7!=nullptr)&&(tmp.compare(str7)==0) )
        {
            val=v7;
            f=true;
        }
        if ( (str8!=nullptr)&&(tmp.compare(str8)==0) )
        {
            val=v8;
            f=true;
        }
        if ( (str9!=nullptr)&&(tmp.compare(str9)==0) )
        {
            val=v9;
            f=true;
        }
        if (!f)
            retVal=xmlGetNode_int(name,val,required);
        else
            retVal=true;
    }
    else
    {
        if (required)
            warnMissingNode(name);
    }
    return(retVal);
}

bool CSer::xmlGetNode_enum(const char* name,int& val,bool required,const std::vector<int>& vals,const std::vector<std::string>& strings)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_enum, name: %s",name);
    std::string tmp;
    bool f=false;
    bool retVal=false;
    if (xmlGetNode_string(name,tmp,required))
    {
        for (size_t i=0;i<vals.size();i++)
        {
            if (tmp.compare(strings[i])==0)
            {
                val=vals[i];
                f=true;
                break;
            }
        }
        if (!f)
            retVal=xmlGetNode_int(name,val,required);
        else
            retVal=true;
    }
    else
    {
        if (required)
            warnMissingNode(name);
    }
    return(retVal);
}

bool CSer::xmlGetNode_int(const char* name,int& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_int, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        if (ss >> buff)
        {
            try
            {
                val=boost::lexical_cast<int>(buff);
            }
            catch (boost::bad_lexical_cast &)
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                return(false);
            }
        }
        else
        {
            if (required)
                App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
            return(false);
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_2int(const char* name,int& val1,int& val2,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_2int, name: %s",name);
    int vals[2];
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        for (int i=0;i<2;i++)
        {
            if (ss >> buff)
            {
                try
                {
                    vals[i]=boost::lexical_cast<int>(buff);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    return(false);
                }
            }
            else
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
                return(false);
            }
        }
        val1=vals[0];
        val2=vals[1];
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_3int(const char* name,int& val1,int& val2,int& val3,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_3int, name: %s",name);
    int vals[3];
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        for (int i=0;i<3;i++)
        {
            if (ss >> buff)
            {
                try
                {
                    vals[i]=boost::lexical_cast<int>(buff);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    return(false);
                }
            }
            else
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
                return(false);
            }
        }
        val1=vals[0];
        val2=vals[1];
        val3=vals[2];
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_ints(const char* name,int* vals,size_t cnt,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_ints, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        for (size_t i=0;i<cnt;i++)
        {
            if (ss >> buff)
            {
                try
                {
                    vals[i]=boost::lexical_cast<int>(buff);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    return(false);
                }
            }
            else
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
                return(false);
            }
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_ints(const char* name,std::vector<int>& vals,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_ints, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        while (true)
        {
            if (ss >> buff)
            {
                try
                {
                    int v=boost::lexical_cast<int>(buff);
                    vals.push_back(v);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    vals.clear();
                    return(false);
                }
            }
            else
                break;
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_uint(const char* name,unsigned int& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_uint, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        if (ss >> buff)
        {
            try
            {
                val=boost::lexical_cast<unsigned int>(buff);
            }
            catch (boost::bad_lexical_cast &)
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                return(false);
            }
        }
        else
        {
            if (required)
                App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
            return(false);
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_ulonglong(const char* name,unsigned long long& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_ulonglong, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        if (ss >> buff)
        {
            try
            {
                val=boost::lexical_cast<unsigned long long>(buff);
            }
            catch (boost::bad_lexical_cast &)
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                return(false);
            }
        }
        else
        {
            if (required)
                App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
            return(false);
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_uchars(const char* name,std::vector<unsigned char>& vals,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_uchars, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        while (true)
        {
            if (ss >> buff)
            {
                try
                {
                    unsigned char v=(unsigned char)boost::lexical_cast<unsigned int>(buff);
                    vals.push_back(v);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    vals.clear();
                    return(false);
                }
            }
            else
                break;
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}


bool CSer::xmlGetNode_float(const char* name,float& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_float, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        if (ss >> buff)
        {
            try
            {
                val=boost::lexical_cast<float>(buff);
            }
            catch (boost::bad_lexical_cast &)
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                return(false);
            }
        }
        else
        {
            if (required)
                App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
            return(false);
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_2float(const char* name,float& val1,float& val2,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_2float, name: %s",name);
    float vals[2];
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        for (int i=0;i<2;i++)
        {
            if (ss >> buff)
            {
                try
                {
                    vals[i]=boost::lexical_cast<float>(buff);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    return(false);
                }
            }
            else
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
                return(false);
            }
        }
        val1=vals[0];
        val2=vals[1];
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_3float(const char* name,float& val1,float& val2,float& val3,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_3float, name: %s",name);
    float vals[3];
    bool retVal=xmlGetNode_floats(name,vals,3,required);
    if (retVal)
    {
        val1=vals[0];
        val2=vals[1];
        val3=vals[2];
    }
    else
    {
        if (required)
            warnMissingNode(name);
    }
    return(retVal);
}

bool CSer::xmlGetNode_4float(const char* name,float& val1,float& val2,float& val3,float& val4,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_4float, name: %s",name);
    float vals[4];
    bool retVal=xmlGetNode_floats(name,vals,4,required);
    if (retVal)
    {
        val1=vals[0];
        val2=vals[1];
        val3=vals[2];
        val4=vals[3];
    }
    else
    {
        if (required)
            warnMissingNode(name);
    }
    return(retVal);
}

bool CSer::xmlGetNode_floats(const char* name,float* vals,size_t cnt,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_floats, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        for (size_t i=0;i<cnt;i++)
        {
            if (ss >> buff)
            {
                try
                {
                    vals[i]=boost::lexical_cast<float>(buff);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    return(false);
                }
            }
            else
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
                return(false);
            }
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_floats(const char* name,std::vector<float>& vals,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_floats, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        while (true)
        {
            if (ss >> buff)
            {
                try
                {
                    float v=boost::lexical_cast<float>(buff);
                    vals.push_back(v);
                }
                catch (boost::bad_lexical_cast &)
                {
                    if (required)
                        App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                    vals.clear();
                    return(false);
                }
            }
            else
                break;
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

bool CSer::xmlGetNode_double(const char* name,double& val,bool required/*=true*/)
{
    if (xmlDebug)
        App::logMsg(sim_verbosity_debug,"XML read: xmlGetNode_double, name: %s",name);
    const xmlNode* node=_xmlCurrentNode->FirstChildElement(name);
    if (node!=nullptr)
    {
        std::string str(_getNodeText(node));
        std::string buff;
        std::stringstream ss(str);
        if (ss >> buff)
        {
            try
            {
                val=boost::lexical_cast<double>(buff);
            }
            catch (boost::bad_lexical_cast &)
            {
                if (required)
                    App::logMsg(sim_verbosity_warnings,"XML read: bad value(s) in node '%s'.",name);
                return(false);
            }
        }
        else
        {
            if (required)
                App::logMsg(sim_verbosity_warnings,"XML read: missing value(s) in node '%s'.",name);
            return(false);
        }
        return(true);
    }
    if (required)
        warnMissingNode(name);
    return(false);
}

