#ifndef HEAP_H
#define HEAP_H

#include <QByteArray>
#include "terminal.h"
#include "interpreter.h"
#include "pbytearray.h"
#include <QStringList>

class Interpreter;

class Heap{
public: Heap(Interpreter* interpreter);
private: Interpreter* interpreter;
public: PByteArray* data;
public: int initThreshold;
public: void setInitThreshold();
public: void accept(unsigned char byte);
public: void setUp(int rpnCount);
public: void addStackFrame(int parentAddress,bool fixed,int lexicalID);
public: bool LastRPNBufferIsTruthy();
public: void addToCurrentStackFrame(QString key, int stringAddress);
public: void addToCurrentStackFrameAsFixed(QString key, int stringAddress);
public: QString decodeStringFrom(int index);
public: int decodeRawIntFrom(int index);
public: float decodeRawFloatFrom(int index);
public: int valueAddressInStackFrames(QString key, int stringAddress);
public: void appendToArray(int A_Address,int B_Address);
public: int valueAddressInStackFrames(QString key, int stringAddress, int _topStackFrameAddress, int requestLexicalId);
public: int propertyValueAddressLookUp(int objectAddress, QString key, bool alongChain);
public: int propertyValueAddressLookUp(int objectAddress,QString key,int stringAddress, bool alongChain);
public: void setCurrentStackFramePointer(int StackFrameAddress);
public: void setRPNBuffer(int buffer, int address);
public: void setRETBuffer(int address);
public: void copyToNew(PByteArray* data, int length);
public: void copyInTo(int startIndex,PByteArray* data,int length);
public: void copyObjectToNew(int objectAddress);
public: void appendToString(int stringAddress,QString concat);
public: QString stackFrameToString(int oAddress);
//public: Terminal* Visualiser;
public: void draw();
public: QString prettyByte(char byte);
//public: int lastAddedAddress;
//public: int lastAddedValueAddress;
public: void clearTopStackFrame();
public: void clear(int index);
public: void constructNewArgsList(int aAddress,int bAddress);
public: void constructNewArgsList(int aAddress);
public: void removeFromArray(int arrayAddress,int index);
public: QString objectToString(int oAddress, int indent, QVector<int> passedAddresses);
public: QString objectToString(int oAddress,int indent);
public: void constructNewArray();
public: void constructNewArray(int aAddress);
public: int getValueAddressInArray(int arrayAddress,int index);
public: QString arrayToString(int address);
public: void prependToArray(int arrayAddress,int valueAddress);
public: void enlargeCurrentStackFrame();
public: int completeDeref(int address);
public: int partialDeref(int address);
public: int RPNAndThisDeref(int A_Address);
public: QStringList getPropertyList(int oAddress);
public: QVector<int> getPropertyKeyList(int oAddress);
public: QVector<int> getPropertyValuesList(int oAddress);
public: QVector<unsigned char> getPropertyModifiersList(int oAddress);
public: QString initThresholdString();
public: QString lexRuleToString(int address);
public: bool objectHasOwn(int objectAddress, int stringAddress, QString key);
public: bool objectHasOnChain(int objectAddress, int keyAddress, QString key, int startdepth, int enddepth);
public: void copyConstantObjectInitAndDefine(int objectAddress, int buffer, int prototypeReference, int classReference);
public: void incrementReferenceCount(int address);
public: void decrementReferenceCount(int address);
public: static int shiftIndex(int size, int index, int factor);
public: static int expectedIndex(int size, QString key);
public: bool isLexicalParentOf(int child,int parent);
};

#endif // HEAP_H
