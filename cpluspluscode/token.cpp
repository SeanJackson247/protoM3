#include "token.h"

#include <QString>
#include <QStringList>
#include <QDebug>

#include "architecture.h"

Token::Token(int type){
    this->matched=false;
    this->type=type;
    if(this->type==Token::TypeArrayInitMarker){
        this->data="[ARRAY_INIT_MARKER]";
    }
    else if(this->type==Token::TypeDUDRight){
        this->data="[DUD_RIGHT]";
    }
    else if(this->type==Token::TypeDUDLeft){
        this->data="[DUD_LEFT]";
    }
    else if(this->type==Token::TypeInvocationMarker){
        this->data="[INVOCATION_MARKER]";
    }
    else if(this->type==Token::TypeNOARGSMarker){
        this->data="[NOARGS_MARKER]";
    }
    else if(this->type==Token::TypeArrayLookUpMarker){
        this->data="[ARRAy_LOOKUP__MARKER]";
    }
    else if(this->type==Token::TypeRPNMarker){
        this->data="[RPN"+QString::number(this->idata)+"MARKER]";
    }
/*    else if(this->type==Token::TypeRPNMarker2){
        this->data="[RPN2MARKER]";
    }*/
    else if(this->type==Token::TypePrimitiveAddress){
        this->data="[PRIMITIVEADDRESS]";
    }
    else if(this->type==Token::TypeComplexAddress){
        this->data="[COMPLEXADDRESS]";
    }
}
Token::Token(int type,int data){
    this->matched=false;
    this->type=type;
    if(this->type==Token::TypeArrayInitMarker){
        this->data="[ARRAY_INIT_MARKER]";
    }
    else if(this->type==Token::TypeDUDRight){
        this->data="[DUD_RIGHT]";
    }
    else if(this->type==Token::TypeDUDLeft){
        this->data="[DUD_LEFT]";
    }
    else if(this->type==Token::TypeInvocationMarker){
        this->data="[INVOCATION_MARKER]";
    }
    else if(this->type==Token::TypeNOARGSMarker){
        this->data="[NOARGS_MARKER]";
    }
    else if(this->type==Token::TypeArrayLookUpMarker){
        this->data="[ARRAy_LOOKUP__MARKER]";
    }
    else if(this->type==Token::TypeRPNMarker){
        this->data="[RPN["+QString::number(data)+"]MARKER]";
    }
    else if(this->type==Token::TypePrimitiveAddress){
        this->data="[PRIMITIVEADDRESS]";
    }
    else if(this->type==Token::TypeComplexAddress){
        this->data="[COMPLEXADDRESS]";
    }
    else if(this->type==Token::TypeLineNumberSetType){
        matched=true;
    }
    this->idata=data;
}
Token::Token(QString data){
    this->matched=false;
    this->data=data;
    this->type=Token::TypeRegular;
}
QString Token::getString(){
/*    if(this->type==Token::TypePrimitiveAddress){
        return "#"+QString::number(this->idata);
    }*/
    /*
    if(this->type==Token::TypeComplexAddress){
        return "[COMPLEX:"+this->data+"];";
    }*/
    if(this->type==Token::TypePrimitiveAddress){
        return "[primitve:["+QString::number(this->idata)+"]]";
    }
    return this->data;
}
int Token::getType(){
    return this->type;
}
bool Token::isOperator(){
    if(QString::compare(this->data,"[")==0){
        return true;
    }
    else if(QString::compare(this->data,"<")==0){
        return true;
    }
    else if(QString::compare(this->data,">")==0){
        return true;
    }
    else if(QString::compare(this->data,"<=")==0){
        return true;
    }
    else if(QString::compare(this->data,">=")==0){
        return true;
    }
    else if(QString::compare(this->data,"&&")==0){
        return true;
    }
    else if(QString::compare(this->data,"||")==0){
        return true;
    }
    else if(QString::compare(this->data,",")==0){
        return true;
    }
    else if(QString::compare(this->data,".")==0){
        return true;
    }
    else if(QString::compare(this->data,"]")==0){
        return true;
    }
    else if(QString::compare(this->data,"(")==0){
        return true;
    }
    else if(QString::compare(this->data,")")==0){
        return true;
    }
    else if(QString::compare(this->data,"+")==0){
        return true;
    }
    else if(QString::compare(this->data,"-")==0){
        return true;
    }
    else if(QString::compare(this->data,"*")==0){
        return true;
    }
    else if(QString::compare(this->data,"/")==0){
        return true;
    }
    else if(QString::compare(this->data,"+")==0){
        return true;
    }
    else if(QString::compare(this->data,"=")==0){
        return true;
    }
    else if(QString::compare(this->data,"%")==0){
        return true;
    }
    else if(QString::compare(this->data,"!")==0){
        return true;
    }
    else if(QString::compare(this->data,"++")==0){
        return true;
    }
    else if(QString::compare(this->data,"+=")==0){
        return true;
    }
    else if(QString::compare(this->data,"--")==0){
        return true;
    }
    else if(QString::compare(this->data,"-=")==0){
        return true;
    }
    else if(QString::compare(this->data,"/=")==0){
        return true;
    }
    else if(QString::compare(this->data,"%=")==0){
        return true;
    }
    else if(QString::compare(this->data,"*=")==0){
        return true;
    }
    else if(QString::compare(this->data,"==")==0){
        return true;
    }
    else if(QString::compare(this->data,"!=")==0){
        return true;
    }
    else if(QString::compare(this->data,"!==")==0){
        return true;
    }
    else if(QString::compare(this->data,"===")==0){
        return true;
    }
    else if(this->type==Token::TypeArrayInitMarker){
        return false;
    }
    else if(this->type==Token::TypeDUDRight){
        return false;
    }
    else if(this->type==Token::TypeInvocationMarker){
        return true;
    }
    else if(this->type==Token::TypeNOARGSMarker){
        return false;
    }
    else if(this->type==Token::TypeArrayLookUpMarker){
        return true;
    }
    else
    return false;
}
bool Token::isRightUnaryOperator(){
    if(QString::compare(this->data,"++")==0){
        return true;
    }
    else if(QString::compare(this->data,"--")==0){
        return true;
    }
    return false;
}
bool Token::isLeftUnaryOperator(){
    if(QString::compare(this->data,"!")==0){
        return true;
    }
/*    else if(QString::compare(this->data,"--")==0){
        return true;
    }*/
    return false;
}
void Token::setAsHeapAddress(int address){
    this->type=Token::TypePrimitiveAddress;
    this->idata=address;
    this->data="[PRIMITIVEADDRESS]";
}

void Token::setAsComplexPointer(int address){

    qDebug()<<"Setting:"<<this->getString()<<"to complex pointer...";

    this->type=Token::TypeComplexAddress;
    this->idata=address;
 //   this->data="[COMPLEXADDRESS]";
}



/*
void Token::setAsLambdaPointer(int address){
    this->type=Token::TypeLambdaAddress;
    this->idata=address;
 //   this->data="[LAMBDAADDRESS]";
}*/
int Token::operatorPrecedence(){
    if(this->type==Token::TypeRegular){
        if(QString::compare(this->data,"=")==0){
            return 2;//changed
        }
        else if(QString::compare(this->data,"(")==0 ||
                QString::compare(this->data,"[")==0 ||
                QString::compare(this->data,"!")==0){
            return 1;
        }
        else if(QString::compare(this->data,",")==0 ||
                QString::compare(this->data,"::")==0){
            return 2;
//            return 0;
        }
        else if(QString::compare(this->data,"+")==0 ||
                QString::compare(this->data,"-")==0){
            return 3;
//            return 4;
        }
        else if(QString::compare(this->data,"*")==0){
            return 4;
//            return 3;
        }
        else if(QString::compare(this->data,"/")==0 ||
                QString::compare(this->data,"%")==0){
            return 5;
        }/*
        else if(QString::compare(this->data,"(")==0 ||
                QString::compare(this->data,"[")==0){
            return 8;
        }*/
        else if(QString::compare(this->data,".")==0){
//            return 10;
            return 7;
        }
        //comparative must trump logical
        else if(QString::compare(this->data,"<")==0 ||
                QString::compare(this->data,"<=")==0 ||
                QString::compare(this->data,">")==0 ||
                QString::compare(this->data,">=")==0 ||
                QString::compare(this->data,"!=")==0 ||
                QString::compare(this->data,"==")==0 ||
                QString::compare(this->data,"!==")==0 ||
                QString::compare(this->data,"!==")==0){
            return 1;
//            return 7;
        }
        return 0;
    }
    else if(this->type==Token::TypeInvocationMarker){
        return 6;
    }
    else if(this->type==Token::TypeArrayInitMarker){
        return 1;
    }
    else if(this->type==Token::TypeArrayLookUpMarker){
//        return 6;
        return 10;
    }
    else{
        return 0;
    }
}

bool Token::isInteger(){
    bool ok;
    int id = this->data.toInt(&ok,10);
    return ok;
}
bool Token::isFloat(){
    QString str1;
    QString str2;
    QStringList stringlist = this->data.split('.');
    if(stringlist.size()==2){
        str1=stringlist[0];
        str2=stringlist[1];
        bool ok;
        int id = str1.toInt(&ok,10);
        if(ok==true){
            int id = str2.toInt(&ok,10);
            return ok;
        }
        return false;
    }
    return false;
}

float Token::asFloat(){
    //return 0.0;
    return this->getString().toFloat();
}
int Token::asInt(){
    bool ok;
    int id = this->data.toInt(&ok,10);
    return id;
}

PByteArray* Token::compile(){
    PByteArray* res = new PByteArray();
    res->resize(4);
    qDebug()<<"Compiling token...";
    if(this->isOperator()==true){
        qDebug()<<"is operator";
        if(this->type==Token::TypeInvocationMarker){
            res->setByte(0,'$');
            res->setByte(1,NULL);
            res->setByte(2,NULL);
            res->setByte(3,NULL);
        }
        else if(this->type==Token::TypeArrayLookUpMarker){
            res->setByte(0,'^');
            res->setByte(1,NULL);
            res->setByte(2,NULL);
            res->setByte(3,NULL);
        }
        else if(this->type==Token::TypeRPNMarker){
            res->setNum(Architecture::RPNAddress+(4*this->idata));
            res->resize(4);
        }
        else{
            int i=0;
            while(i<this->data.size()){
                qDebug()<<"setting as operator..";
                res->setByte(i,this->data[i].toLatin1());
                i++;
            }
            while(i<4){
                qDebug()<<"flling with null..";
                res->setByte(i,NULL);
                i++;
            }
        }
    }
    else if(this->type==Token::TypeLineNumberSetType || this->type==Token::TypePrimitiveAddress || this->type==Token::TypeComplexAddress){
        qDebug()<<this->getString()<<"is heap pointer..."<<QString::number(this->idata);
        res->setNum(this->idata);
        res->resize(4);
    }
    /*
    Heap::accept(NULL);//null
    Heap::accept(NULL);//unset
    Heap::accept(NULL);//true
    Heap::accept(NULL);//false
    Heap::accept(NULL);//isNaN
    Heap::accept(NULL);//DUDRIGHT
    Heap::accept(NULL);//infinity

    Heap::accept(NULL);//rpn1 int
    Heap::accept(NULL);//rpn1 int
    Heap::accept(NULL);//rpn1 int
    Heap::accept(NULL);//rpn1 int

    Heap::accept(NULL);//rpn2 int
    Heap::accept(NULL);//rpn2 int
    Heap::accept(NULL);//rpn2 int
    Heap::accept(NULL);//rpn2 int
    */
    /*else if(this->type==Token::TypeRPNMarker1){
        res->setNum(Architecture::RPN1Address);
        res->resize(4);
    }
    else if(this->type==Token::TypeRPNMarker2){
        res->setNum(Architecture::RPN2Address);
        res->resize(4);
    }
    else if(this->type==Token::TypeRPNMarker2){
        res->setNum(Architecture::RPN2Address);
        res->resize(4);
    }*/
    else if(this->type==Token::TypeRPNMarker){
        //res->setNum(Architecture::RPNAddress+(4*this->idata));
        qDebug()<<"Setting RPN Marker:"<<this->idata;
        qDebug()<<"Setting RPN Marker:"<<this->idata*4;
        qDebug()<<"Setting RPN Marker:"<<Architecture::RPNAddress+(4*(this->idata-1));
        res->setNum(Architecture::RPNAddress+(4*(this->idata-1)));
        res->resize(4);
    }
    else if(this->type==Token::TypeArrayInitMarker){
        //res->setNum(Architecture::RPNAddress+(4*this->idata));
        res->setNum(Architecture::ArrayInitLocation);
        res->resize(4);
    }
    /*
    else if(this->type==Token::TypeDUDRight){
        res.setNum(Token::TypeDUDRightPointer);
        res.resize(4);
    }*/
    else if(QString::compare(this->data,"native")==0){
        res->setNum(Architecture::NativeLocation);
    }
    else if(QString::compare(this->data,"this")==0){
        qDebug()<<"Compiled a this pointer!";
        res->setNum(Architecture::ThisAddress);
    }
    else if(QString::compare(this->data,"true")==0){
        qDebug()<<"Compiled a this pointer!";
        res->setNum(Architecture::TrueLocation);
    }
    else if(QString::compare(this->data,"false")==0){
        qDebug()<<"Compiled a this pointer!";
        res->setNum(Architecture::FalseLocation);
    }
    else if(QString::compare(this->data,"unset")==0){
        qDebug()<<"Compiled a this pointer!";
        res->setNum(Architecture::UnsetLocation);
    }
    //insert this................
    else{
        qDebug()<<"unknown token";
        res->setByte(0,NULL);
        res->setByte(1,NULL);
        res->setByte(2,NULL);
        res->setByte(3,NULL);
    }
    return res;
}
