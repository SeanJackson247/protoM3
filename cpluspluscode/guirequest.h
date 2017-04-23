#ifndef GUIREQUEST_H
#define GUIREQUEST_H

#include <QString>
#include <QVector>

#include "terminal.h"

class Terminal;

class GUIRequest{
public: QString object;
public: QString action;
public: QVector<QString> values;
public: static bool hasRequest;
public: QVector<Terminal*> Terminals;
public: static GUIRequest* CurrentRequest;
public:
    GUIRequest();
};

#endif // GUIREQUEST_H
