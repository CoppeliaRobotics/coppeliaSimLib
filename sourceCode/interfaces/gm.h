#pragma once

#include <QObject>

class CGm: public QObject
{
public:
    CGm();
    ~CGm();

    bool fi();
    bool fo();
    bool fic(const char*);
};
