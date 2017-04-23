#ifndef ALU_H
#define ALU_H

#include <QString>
#include "interpreter.h"
#include "heap.h"

class Heap;

class ALU{
public: ALU(Interpreter *interpreter);
public: Interpreter* p;
public: Heap* heap;
public: void debuggerOut(QString out);
public: void eval(int A_Address, int B_Address, QString _operator, unsigned char buffer);
public: void logicalEval(int A_Address, int B_Address, QString _operator, unsigned char buffer);
};

#endif // ALU_H
