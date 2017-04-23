#ifndef BYTECODEINSTRUCTION2_H
#define BYTECODEINSTRUCTION2_H

#include <QString>
#include "token2.h"
#include "preproperty.h"
#include "pbytearray.h"
#include "lexnode.h"

class LexNode;

class ByteCodeInstruction2{
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
public: static constexpr char TypeArrayType=0x0d;
public: static constexpr char TypeArgsListType=0x0e;
public: static constexpr char TypePropertyKeyType=0x0f;
public: static constexpr char TypeHeapFragment=0x10;
public: static constexpr char TypeEventList=0x11;
public: static constexpr char TypeTimerEventType=0x12;
public: static constexpr char TypeArrayIndexType=0x13;
public: static constexpr char TypeModuleReadyEvent=0x14;
public: static constexpr char TypeFileWrittenEvent=0x15;



public: static constexpr char TypeLineNumberSetType=16;
public: static char constexpr TypeRPNReserve = 17;
public: static char constexpr TypeRPN=20;
//public: static char constexpr TypeEndIfMarker = 21;
public: bool isEndOfIf;

public: static constexpr int ArrayIntSize=4;
public: ByteCodeInstruction2(int type,Token2* A,Token2* B,Token2* C);
public: char type;
public: Token2* A;
public: Token2* B;
public: Token2* C;
public: void setRPNMarker(int rpn);
public: int idata;
public: int conditionalId;
public:QString toString(int indent);
public: PByteArray* compile(LexNode* ln);
public: static QVector<ByteCodeInstruction2*> hoistVariables(QVector<ByteCodeInstruction2*> lines);
public: static QVector<ByteCodeInstruction2*> flattenControlFlows(QVector<ByteCodeInstruction2*> lines);
public: static QVector<ByteCodeInstruction2*> cleanup(QVector<ByteCodeInstruction2*> lines);
public:static QVector<ByteCodeInstruction2*> fromInitializers(QVector<PreProperty*> properties);
public: static QVector<ByteCodeInstruction2*> ammendControlFlows(QVector<ByteCodeInstruction2*> lines);
};

#endif // BYTECODEINSTRUCTION2_H
