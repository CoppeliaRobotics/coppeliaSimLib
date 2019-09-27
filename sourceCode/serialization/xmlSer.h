
#pragma once

#include "vrepMainHeader.h"
#include "tinyxml2.h"

typedef tinyxml2::XMLElement xmlNode;

#define XML_VERSION "version"
#define XML_REVISION "revision"

class CXmlSer  
{
public:
    CXmlSer(const char* filename);
    virtual ~CXmlSer();

    xmlNode* readOpen();
    xmlNode* writeOpen();
    bool writeClose();

    xmlNode* createNode(const char* name);
    void insertFirstChildNode(xmlNode* parentNode,xmlNode* nodeToInsert);
    void insertNodeAfterSiblingNode(xmlNode* parentNode,xmlNode* siblingNode,xmlNode* nodeToInsert);

    xmlNode* getFirstChildNode(xmlNode* parentNode,const char* name);
    xmlNode* getNodeAfterSiblingNode(xmlNode* siblingNode,const char* name);

    void addWarningMessage(const char* msg);
    void addErrorMessage(const char* msg);
    void addMessage(const char* msg,unsigned int indent=0);
    std::string getMessages();

    void addBoolAttribute(xmlNode* node,const char* attribName,bool attrib);
    void addIntAttribute(xmlNode* node,const char* attribName,int attrib);
    void addInt2Attribute(xmlNode* node,const char* attribName,const int attrib[2]);
    void addInt2Attribute(xmlNode* node,const char* attribName,int attrib1,int attrib2);
    void addInt2Attribute(xmlNode* node,const char* attribName,const std::vector<int>& attrib);
    void addInt3Attribute(xmlNode* node,const char* attribName,const int attrib[3]);
    void addInt3Attribute(xmlNode* node,const char* attribName,int attrib1,int attrib2,int attrib3);
    void addInt3Attribute(xmlNode* node,const char* attribName,const std::vector<int>& attrib);
    void addIntArrayAttribute(xmlNode* node,const char* attribName,const int* attrib,unsigned int cnt);
    void addIntVectorAttribute(xmlNode* node,const char* attribName,const std::vector<int>& attrib);
    void addFloatAttribute(xmlNode* node,const char* attribName,double attrib);
    void addFloat2Attribute(xmlNode* node,const char* attribName,const float attrib[2]);
    void addFloat2Attribute(xmlNode* node,const char* attribName,const double attrib[2]);
    void addFloat2Attribute(xmlNode* node,const char* attribName,double attrib1,double attrib2);
    void addFloat2Attribute(xmlNode* node,const char* attribName,const std::vector<float>& attrib);
    void addFloat2Attribute(xmlNode* node,const char* attribName,const std::vector<double>& attrib);
    void addFloat3Attribute(xmlNode* node,const char* attribName,const float attrib[3]);
    void addFloat3Attribute(xmlNode* node,const char* attribName,const double attrib[3]);
    void addFloat3Attribute(xmlNode* node,const char* attribName,double attrib1,double attrib2,double attrib3);
    void addFloat3Attribute(xmlNode* node,const char* attribName,const std::vector<float>& attrib);
    void addFloat3Attribute(xmlNode* node,const char* attribName,const std::vector<double>& attrib);
    void addFloat4Attribute(xmlNode* node,const char* attribName,const float attrib[4]);
    void addFloat4Attribute(xmlNode* node,const char* attribName,const double attrib[4]);
    void addFloat4Attribute(xmlNode* node,const char* attribName,double attrib1,double attrib2,double attrib3,double attrib4);
    void addFloat4Attribute(xmlNode* node,const char* attribName,const std::vector<float>& attrib);
    void addFloat4Attribute(xmlNode* node,const char* attribName,const std::vector<double>& attrib);
    void addFloatVectorAttribute(xmlNode* node,const char* attribName,const std::vector<float>& attrib);
    void addFloatVectorAttribute(xmlNode* node,const char* attribName,const std::vector<double>& attrib);
    void addFloatArrayAttribute(xmlNode* node,const char* attribName,const float* attrib,unsigned int cnt);
    void addFloatArrayAttribute(xmlNode* node,const char* attribName,const double* attrib,unsigned int cnt);
    void addStringAttribute(xmlNode* node,const char* attribName,const char* attrib);
    void addStringAttribute(xmlNode* node,const char* attribName,const std::string& attrib);

    bool getBoolAttribute(const xmlNode* node,const char* attribName,bool& attrib);
    bool getBoolAttribute(const xmlNode* node,const char* attribName);
    bool getIntAttribute(const xmlNode* node,const char* attribName,int& attrib);
    int getIntAttribute(const xmlNode* node,const char* attribName);
    bool getInt2Attribute(const xmlNode* node,const char* attribName,int attrib[2]);
    bool getInt2Attribute(const xmlNode* node,const char* attribName,int& attrib1,int& attrib2);
    bool getInt2Attribute(const xmlNode* node,const char* attribName,std::vector<int>& attrib);
    bool getInt3Attribute(const xmlNode* node,const char* attribName,int attrib[3]);
    bool getInt3Attribute(const xmlNode* node,const char* attribName,int& attrib1,int& attrib2,int& attrib3);
    bool getInt3Attribute(const xmlNode* node,const char* attribName,std::vector<int>& attrib);
    bool getIntArrayAttribute(const xmlNode* node,const char* attribName,int* attrib,unsigned int cnt);
    int getIntVectorAttribute(const xmlNode* node,const char* attribName,std::vector<int>& attrib,unsigned int cnt);
    bool getFloatAttribute(const xmlNode* node,const char* attribName,float& attrib);
    bool getFloatAttribute(const xmlNode* node,const char* attribName,double& attrib);
    double getFloatAttribute(const xmlNode* node,const char* attribName);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,float attrib[2]);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,double attrib[2]);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,float& attrib1,float& attrib2);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,double& attrib1,double& attrib2);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,std::vector<float>& attrib);
    bool getFloat2Attribute(const xmlNode* node,const char* attribName,std::vector<double>& attrib);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,float attrib[3]);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,double attrib[3]);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,float& attrib1,float& attrib2,float& attrib3);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,double& attrib1,double& attrib2,double& attrib3);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,std::vector<float>& attrib);
    bool getFloat3Attribute(const xmlNode* node,const char* attribName,std::vector<double>& attrib);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,float attrib[4]);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,double attrib[4]);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,float& attrib1,float& attrib2,float& attrib3,float& attrib4);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,double& attrib1,double& attrib2,double& attrib3,double& attrib4);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,std::vector<float>& attrib);
    bool getFloat4Attribute(const xmlNode* node,const char* attribName,std::vector<double>& attrib);
    int getFloatVectorAttribute(const xmlNode* node,const char* attribName,std::vector<float>& attrib,unsigned int cnt);
    int getFloatVectorAttribute(const xmlNode* node,const char* attribName,std::vector<double>& attrib,unsigned int cnt);
    bool getFloatArrayAttribute(const xmlNode* node,const char* attribName,float* attrib,unsigned int cnt);
    bool getFloatArrayAttribute(const xmlNode* node,const char* attribName,double* attrib,unsigned int cnt);
    bool getStringAttribute(const xmlNode* node,const char* attribName,std::string& attrib);
    std::string getStringAttribute(const xmlNode* node,const char* attribName);



private:
    tinyxml2::XMLDocument _xmlDocument;
    std::string _filename;
    std::string _messages;
};
