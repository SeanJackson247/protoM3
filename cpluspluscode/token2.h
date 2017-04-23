#ifndef TOKEN2_H
#define TOKEN2_H

#include<QString>
#include<QVector>
#include "pbytearray.h"

class Token2{
public: Token2(int type,QString data);
public: Token2(int type,QString data,int _data);
public: Token2(int type,int data);
public: int type;
public: int data;
public: int lexId;
public: bool isKeyWord();
public: bool isOperator();
public: bool isInteger();
public: bool isFloat();
//public: bool isEndOfIf;
public: int toInteger();
public: float toFloat();
public: static constexpr int TypeCode = 2;
public: static constexpr int TypeOperator = 5;
public: static constexpr int TypeString = 3;
public: static constexpr int TypeChildMarker = 4;
public: int operatorPrecedence();
public: QString _string;
public: void setAsAddress(int i);
public: int address;
public: bool isLeftUnaryOperator();
public: bool isRightUnaryOperator();
public: bool isRightUnaryOperator2();
public: int asInt();
public: PByteArray* compile();
public: QVector<QString> expectedArguments;
};

#endif // TOKEN2_H
