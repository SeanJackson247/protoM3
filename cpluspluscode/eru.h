#ifndef ERU_H
#define ERU_H


class ERU{
public: ERU(Interpreter *interpreter);
public: Interpreter* p;
public: Heap* heap;
public: void debuggerOut(QString out);
public: void eval(int A_Address, int B_Address, QString _operator, unsigned char buffer, int instructionPointer);
};

#endif // ERU_H
