#ifndef HEAPSTORE_H
#define HEAPSTORE_H

#include <QVector>

#include <line.h>
#include <preobject.h>

class Line;

class HeapStore{
public: HeapStore();
public: QVector<Line*> constructLexTree(QVector<Line*> lines);
public: static getFreshLexicalId();
public: static int lexicalID;
public: static int gotoID;
public: static int newGoToId();
public: static QVector<Line*> extractConstants(QVector<Line*> lines);
public: static void addLexicalRule(int l,int p);
public: static QVector<int> lexes;
public: static QVector<int> pars;
//public: static QVector<int> lambs;
public: static QVector<PreObject*> literals;
public: static QVector<QString> primitives;
public: static int recievePrimitive(QString primitive);
public: static void recieveLambda(int lexicalID);
public: static void recieveLiteral(int lexicalID,QVector<Line*> lines);
public: static void recieveClassDef(int lexicalID,QVector<Line*> lines);
public: static PByteArray* encode(QVector<ByteCodeInstruction*> lines);
public: static PByteArray* encodeAsKey(QString string);
};

#endif // HEAPSTORE_H
