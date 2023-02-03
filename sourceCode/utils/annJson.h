#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <vector>
#include <string>
#include <map>
#include <simTypes.h>

class CAnnJson
{
public:
    CAnnJson(QJsonObject* mainObject);
    virtual ~CAnnJson();

    void addJson(QJsonObject& jsonObj,const char* key,const QJsonObject& value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,const QJsonArray& value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,bool value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,int value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,double value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,const char* value,const char* annotation=nullptr);
    void addJson(QJsonObject& jsonObj,const char* key,const double* v,size_t cnt,const char* annotation=nullptr);

    std::string getAnnotatedString();

    QJsonObject* getMainObject();
    void setMainObject(QJsonObject* obj);

    static std::string stripComments(const char* jsonTxt);
    static bool getValue(QJsonObject& jsonObj,const char* key,QJsonValue::Type type,QJsonValue& value,std::string* errMsg=nullptr);
    static bool getValue(QJsonObject& jsonObj,const char* key,double* vals,size_t cnt,std::string* errMsg=nullptr);

protected:
    double _round(double d) const;
    std::string _nbKey(const char* key);
    void _addAnnotation(const char* nbKey,const char* annotation);

    int _cnt;
    QJsonObject* _mainObject;
    std::map<std::string,std::string> _keysAndAnnotations;
};
