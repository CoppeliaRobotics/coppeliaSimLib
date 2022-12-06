#include "annJson.h"
#include "tt.h"
#include "ttUtil.h"
#include <cmath>

CAnnJson::CAnnJson(QJsonObject* mainObject)
{
    _mainObject=mainObject;
    _cnt=0;
}

CAnnJson::~CAnnJson()
{
}

std::string CAnnJson::_nbKey(const char* key)
{
    std::string retVal("__key");
    retVal+=tt::FNb(4,_cnt++);
    retVal+="__";
    retVal+=key;
    return(retVal);
}

void CAnnJson::_addAnnotation(const char* nbKey,const char* annotation)
{
    if (annotation!=nullptr)
        _keysAndAnnotations[nbKey]=annotation;
    else
        _keysAndAnnotations[nbKey]="";
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,const QJsonObject& value,const char* annotation/*=nullptr*/)
{
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,const QJsonArray& value,const char* annotation/*=nullptr*/)
{
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,bool value,const char* annotation/*=nullptr*/)
{
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,int value,const char* annotation/*=nullptr*/)
{
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

double CAnnJson::_round(double d) const
{
    if ( (fabs(d)>1e-35)&&(fabs(d)<1e35) )
    {
        double c=1.0;
        while (fabs(d)*c>=10.0)
            c/=10.0;
        while (fabs(d)*c<1.0)
            c*=10.0;
        c*=1000.00;
        d*=c;
        d+=0.5*d/fabs(d);
        d=double(int(d));
        d/=c;
    }
    return(d);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,double value,const char* annotation/*=nullptr*/)
{
    value=_round(value);
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,const double* v,size_t cnt,const char* annotation/*=nullptr*/)
{
    QJsonArray arr;
    for (size_t i=0;i<cnt;i++)
        arr.push_back(_round(v[i]));
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=arr;
    _addAnnotation(l.c_str(),annotation);
}

void CAnnJson::addJson(QJsonObject& jsonObj,const char* key,const char* value,const char* annotation/*=nullptr*/)
{
    std::string l(_nbKey(key));
    jsonObj[l.c_str()]=value;
    _addAnnotation(l.c_str(),annotation);
}

QJsonObject* CAnnJson::getMainObject()
{
    return(_mainObject);
}

void CAnnJson::setMainObject(QJsonObject* obj)
{
    _mainObject=obj;
}

std::string CAnnJson::stripComments(const char* jsonTxt)
{
    std::string input(jsonTxt);
    std::string retVal;
    std::string line;
    while (CTTUtil::extractLine(input,line))
    {
        CTTUtil::removeComments(line);
        retVal+=line+"\n";
    }
    return(retVal);
}

bool CAnnJson::getValue(QJsonObject& jsonObj,const char* key,QJsonValue::Type type,QJsonValue& value,std::string* errMsg/*=nullptr*/)
{
    bool retVal=false;
    std::string msg;
    if (jsonObj.contains(key))
    {
        if (jsonObj[key].type()==type)
        {
            value=jsonObj[key];
            retVal=true;
        }
        else
            msg=std::string("Key '")+key+"' has not correct type and will be ignored.";
    }
    else
        msg=std::string("Key '")+key+"' was not found and will be ignored.";
    if ( (!retVal)&&(errMsg!=nullptr) )
    {
        if (errMsg->size()>0)
            errMsg[0]+="\n";
        errMsg[0]+=msg;
    }
    return(retVal);
}

bool CAnnJson::getValue(QJsonObject& jsonObj,const char* key,double* vals,size_t cnt,std::string* errMsg/*=nullptr*/)
{
    bool retVal=false;
    std::string msg;
    if (jsonObj.contains(key))
    {
        if (jsonObj[key].type()==QJsonValue::Array)
        {
            QJsonArray arr=jsonObj[key].toArray();
            if (arr.size()>=cnt)
            {
                retVal=true;
                for (size_t i=0;i<cnt;i++)
                {
                    if (arr[int(i)].type()==QJsonValue::Double)
                        vals[i]=arr[int(i)].toDouble();
                    else
                    {
                        msg=std::string("Key '")+key+"' contains items with wrong type and will be ignored.";
                        break;
                    }
                }
            }
            else
                msg=std::string("Key '")+key+"' has not correct size and will be ignored.";
        }
        else
            msg=std::string("Key '")+key+"' has not correct type and will be ignored.";
    }
    else
        msg=std::string("Key '")+key+"' was not found and will be ignored.";
    if ( (!retVal)&&(errMsg!=nullptr) )
    {
        if (errMsg->size()>0)
            errMsg[0]+="\n";
        errMsg[0]+=msg;
    }
    return(retVal);
}

std::string CAnnJson::getAnnotatedString()
{
    QJsonDocument doc(_mainObject[0]);
    std::string json(doc.toJson(QJsonDocument::Indented).toStdString());
    std::string retVal;
    std::string line;
    while (CTTUtil::extractLine(json,line))
    {
        size_t p1=line.find("__key");
        if (p1!=std::string::npos)
        {
            size_t p2=line.find("__",p1+2);
            size_t p3=line.find("\"",p2);
            std::string nbKey(line.begin()+p1,line.begin()+p3);
            std::string key(line.begin()+p2+2,line.begin()+p3);
            line.replace(p1,p3-p1,key);
            if (_keysAndAnnotations[nbKey].size()>0)
            {
                line+="    //";
                line+=_keysAndAnnotations[nbKey];
            }
        }
        retVal+=line+"\n";
    }
    return(retVal);
}
