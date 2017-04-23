#ifndef TOKEN_H
#define TOKEN_H

#include <QString>
#include "pbytearray.h"

class Token{
public: Token(QString data);
public: Token(int type);
public: Token(int type,int data);
private: int type;
public: QString data;
public: int idata;
public: bool matched;
public: int getType();
public: QString getString();
public: bool isOperator();
public: bool isInteger();
public: bool isFloat();
public: int asInt();
public: float asFloat();
public: bool isRightUnaryOperator();
public: bool isLeftUnaryOperator();
public: PByteArray* compile();
public: void setAsHeapAddress(int index);
public: void setAsComplexPointer(int index);



//public: void setAsLambdaPointer(int index);
public: int operatorPrecedence();
public: static constexpr int TypeRegular=0;
public: static constexpr int TypeInvocationMarker=1;
public: static constexpr int TypeNOARGSMarker=2;
public: static constexpr int TypeDUDRight=3;
public: static constexpr int TypeDUDLeft=4;
public: static constexpr int TypeArrayInitMarker=5;
public: static constexpr int TypeArrayLookUpMarker=6;
public: static constexpr int TypePrimitiveAddress=7;
public: static constexpr int TypeComplexAddress=8;
//public: static constexpr int TypeLambdaAddress=10;
//public: static constexpr int TypeRPNMarker1=7;
public: static constexpr int TypeRPNMarker=9;
//public: static constexpr int TypeRPNMarker2=8;
public: static constexpr int TypeLineNumberSetType=10;
//public: static constexpr int TypeRPNMarker1Pointer=10;
//public: static constexpr int TypeRPNMarker2Pointer=14;
//public: static constexpr int TypeSysAddress=8;

//public: static constexpr int TypeRPN=7;
};

#endif // TOKEN_H
