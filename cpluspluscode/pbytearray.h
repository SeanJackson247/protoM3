#ifndef PBYTEARRAY_H
#define PBYTEARRAY_H

#include <QByteArray>

class PByteArray{
public: PByteArray();
private: QByteArray data;
public: int size();
public: void resize(int size);
public: unsigned char getByte(int index);
public: void setByte(int index,unsigned char byte);
public: void prepend(unsigned char byte);
public: void append(unsigned char byte);
public: void setNum(int index);
public: void setNum(float f);
public: int toInt();
public: float toFloat();
public: void encodeAsKey(QString string);
public: void encodeAsKeyAt(int position,QString string);
public: void encodeAsString(QString string, bool isConstant);
public: void encodeAsStringAt(int position,QString string, bool isConstant);
public: void encodeAsRawFloat(float x);
public: void reserve(int r);
public: QString decodeStringFrom(int index);
public: int decodeRawIntFrom(int index);
public: void encodeAsRunTimeInteger(int refCount,int value);
public: void encodeRawIntegerAt(int position,int data);
public: void encodeRawFloatAt(int dir,float x);
public: void encodeAsLambda();
public: void encodeLambdaAt(int position, int lexNode, int instructionPointer);
public: void encodeAsLiteral(int lexNode,int instructionSetPointer);
public: void encodeAsClass();
public: void encodeLiteralAt(int lexNode,int instructionSetPointer);
public: void encodeClassAt(int position,PByteArray* pbytes);
public: bool hasInnerReserve;
public: int innerReserve;
public: void copyInto(int position,PByteArray* sub,int length);
public: void encodeAsRunTimeFloat(int refCount, float value);
public: void encodeLexRuleAt(int position,int parent, int child);
};

#endif // PBYTEARRAY_H
