#ifndef PREOBJECT_H
#define PREOBJECT_H


#include "line.h"
#include "preproperty.h"

class PreObject
{
public:QVector<PreProperty*> properties;
public:QString toString(int indent);
public: int lexicalId;
public: PreObject(int lexicalId, QVector<Line*> lines, QString mode);
public: static PreObject* newStaticClass(int lexicalId,QVector<Line*> lines);
public: static PreObject* newInstancePrototype(int lexicalId,QVector<Line*> lines);
public: static PreObject* newInstanceBluePrint(int lexicalId,QVector<Line*> lines);
public: static PreObject* newLambda(int lexicalId);
private: int proto;
private: int init;
private: int instructionSetPointer;
public: QVector<int> setPointerToInstructionSet(PByteArray* instructionSet, QVector<int> matches);
public: bool isLambda;
public:void setProto(int id);
public:void setInit(int lambdaIndex);
public:bool needsInit();
public:void setAsLambda(bool toggle);
public: int lexicalOffset;
public:PByteArray* compile(int rpnCount, PByteArray* preExistingHeap, int factor);
public:PByteArray* compile(int rpnCount,PByteArray* preExistingHeap);
public:void setProperty(QString propertyName, int valueAddress, bool abstract, bool final, int privacy);
public: void repointPropertyNames(QVector<int> oldAddresses,QVector<int> newAddresses);
public: int compileSize(QVector<QString> primitives, QVector<int> primitiveAddresses, int rpnCount, int factor);
public: int compileSize(QVector<QString> primitives,QVector<int> primitiveAddresses,int rpnCount);
public: void setLexicalOffset(int i);
public: void repointProtoAndInitPointers(QVector<int> oldAddresses,QVector<int> newAddresses);
public: void repointMethodPointers(QVector<int> oldAddresses,QVector<int> newAddresses);
};

#endif // PREOBJECT_H
