#ifndef BYTECODEINSTRUCTION_H
#define BYTECODEINSTRUCTION_H

#include <QString>
#include "token.h"
#include "preproperty.h"
#include "pbytearray.h"

class ByteCodeInstruction{

public: static constexpr char TypeLET=3;
public: static constexpr char TypeFIX=4;
public: static constexpr char TypeGOTOMarker=5;
public: static constexpr char TypeLEXEndMarker=6;
public: static constexpr char TypeLEXStartMarker=7;
public: static constexpr char TypeGOTOFCompMarker=8;
public: static constexpr char TypeGOTOLoopMarker=9;
public: static constexpr char TypeCOMP=10;
public: static constexpr char TypeFCOMP=11;
public: static constexpr char TypeJMP=12;
public: static constexpr char TypeBackJMP=14;
public: static constexpr char TypeReturnMarker=13;
public: static constexpr char TypeEARG=15;
public: static constexpr char TypeHeapFlush=0x01;
public: static constexpr char TypeConstStringType=0x02;
public: static constexpr char TypeConstIntegerType=0x03;
public: static constexpr char TypeConstFloatType=0x04;

public: static constexpr char TypeRunTimeStringType=0x05;
public: static constexpr char TypeRunTimeIntegerType=0x06;
public: static constexpr char TypeRunTimeFloatType=0x07;

public: static constexpr char TypeLexRuleType=0x08;
public: static constexpr char TypeObjectType=0x09;
public: static constexpr char TypeConstObjectType=0x0a;
public: static constexpr char TypeLambdaType=0x0b;
public: static constexpr char TypeKeyType=0x0c;
//public: static constexpr char TypeArrayType=0x0d;
public: static constexpr char TypeArgsListType=0x0e;
public: static constexpr char TypePropertyKeyType=0x0f;
public: static constexpr char TypeHeapFragment=0x10;
public: static constexpr char TypeEventList=0x11;
public: static constexpr char TypeTimerEventType=0x12;
public: static constexpr char TypeArrayIndexType=0x13;
public: static constexpr char TypeModuleReadyEvent=0x14;
public: static constexpr char TypeFileWrittenEvent=0x15;

public: static constexpr char TypeClosedArrayType=0x16;
public: static constexpr char TypeOpenArrayType=0x17;

public: static constexpr char TypeLineNumberSetType=16;
public: static char constexpr TypeRPNReserve = 17;
public: static char constexpr TypeRPN=20;


public: static constexpr int ArrayIntSize=4;
public: ByteCodeInstruction(int type,Token* A,Token* B,Token* C);
public: char type;
public: Token* A;
public: Token* B;
public: Token* C;
public: void setRPNMarker(int rpn);
public: int idata;
public:QString toString(int indent);
public:PByteArray* compile(int rpnCount);
public: static QVector<ByteCodeInstruction*> hoistVariables(QVector<ByteCodeInstruction*> lines);
public: static QVector<ByteCodeInstruction*> flattenControlFlows(QVector<ByteCodeInstruction*> lines);
public: static QVector<ByteCodeInstruction*> cleanup(QVector<ByteCodeInstruction*> lines);
public:static QVector<ByteCodeInstruction*> fromInitializers(QVector<PreProperty*> properties);
public: static QVector<ByteCodeInstruction*> ammendControlFlows(QVector<ByteCodeInstruction*> lines);
};

#endif // BYTECODEINSTRUCTION_H
