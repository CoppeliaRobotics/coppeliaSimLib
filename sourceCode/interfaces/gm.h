#pragma once

#include <QObject>

class CGm: public QObject
{
public:
    CGm();
    ~CGm();

    void ll(const char*);
};
