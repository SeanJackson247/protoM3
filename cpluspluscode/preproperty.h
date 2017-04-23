#ifndef PREPROPERTY_H
#define PREPROPERTY_H

#include <QString>
#include <QVector>

#include "token.h"

class PreProperty
{
public:
    PreProperty();
public:bool hasName();
public:void setName(QString name);
public:void setAs(QString name);
public:void appendInit(Token* token);
//public:void appendInit(int address);
public:int name;
private:int lexicalId;
public:int heapPointer;
private:bool isAbstract;
private:bool isFinal;
public:unsigned char privacy;
public:int complexPointer;
public:bool initScoop;
public:QVector<Token*> initializers;
public:QString toString(int indent);
public:void setAsAbstract(bool val);
public:void setAsFinal(bool val);
};

#endif // PREPROPERTY_H
