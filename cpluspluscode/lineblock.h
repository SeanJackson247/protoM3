#ifndef LINEBLOCK_H
#define LINEBLOCK_H

#include <QVector>
#include "token.h"

class LineBlock{
    //line blocks have only one constructor and are made from a string
public: LineBlock(int lineNumber,QString line);
    //except that they dont they have two for initialisers...
public: LineBlock(QVector<Token*> data);
    //line blocks are made up from tokens:
private: QVector<Token*> data;
public: QVector<Token*> getData();
    //but also retain a line number and indentation
private: int lineNumber;
private: int indent;
    //public getters...
public: int getLineNumber();
public: int getIndent();
public: bool isMethodSignature();
    //they must be able to return some info about their tokens:
public:int size();
    //and a polite toString method...
public: QString toString();
};

#endif // LINEBLOCK_H
