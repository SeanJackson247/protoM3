#ifndef NRU_H
#define NRU_H

#include "interpreter.h"

class NRU{
public: NRU(Interpreter *interpreter);
public: Interpreter* p;
public: Heap* heap;
public: void debuggerOut(QString out);
public: void execute(int argsAddress);
};

#endif // NRU_H
