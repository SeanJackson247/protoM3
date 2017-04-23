#include "token2.h"

#include <QString>
#include <QStringList>
#include <QDebug>

#include "architecture.h"

Token2::Token2(int type, QString data){
    //this->isEndOfIf=false;
    this->type=type;
    this->_string=data;
    this->data = 0;
    this->address = 0;
}

Token2::Token2(int type, QString data,int _data){
    //this->isEndOfIf=false;
    this->type=type;
    this->_string=data;
    this->data = _data;
    this->address = 0;
}

Token2::Token2(int type, int data){
 //   this->isEndOfIf=false;
    this->type=type;
    this->data=data;
    this->address = 0;
}

int Token2::operatorPrecedence(){
    if(this->type==Token2::TypeCode){
        if(QString::compare(this->_string,"=")==0){
            //return 2;//changed
            //return 6;
            return 0;
        }
        else if(QString::compare(this->_string,"(")==0 ||
                QString::compare(this->_string,"[")==0 ||
                QString::compare(this->_string,"!")==0){
            return 1;
        }
        else if(QString::compare(this->_string,",")==0 ||
                QString::compare(this->_string,"::")==0){
            return 2;
//            return 0;
 //             return 7;
        }
        else if(QString::compare(this->_string,"+")==0 ||
                QString::compare(this->_string,"-")==0){
            return 3;
//            return 4;
        }
        else if(QString::compare(this->_string,"*")==0){
            return 4;
//            return 3;
        }
        else if(QString::compare(this->_string,"/")==0 ||
                QString::compare(this->_string,"%")==0){
            return 5;
        }/*
        else if(QString::compare(this->data,"(")==0 ||
                QString::compare(this->data,"[")==0){
            return 8;
        }*/
        else if(QString::compare(this->_string,".")==0){
//            return 10;
            return 7;
        }
        //comparative must trump logical
        else if(QString::compare(this->_string,"<")==0 ||
                QString::compare(this->_string,"<=")==0 ||
                QString::compare(this->_string,">")==0 ||
                QString::compare(this->_string,">=")==0 ||
                QString::compare(this->_string,"!=")==0 ||
                QString::compare(this->_string,"==")==0 ||
                QString::compare(this->_string,"!==")==0 ||
                QString::compare(this->_string,"!==")==0){
            return 1;
//            return 7;
        }
        return 0;
    }
    else if(this->type==Token2::TypeOperator &&
            QString::compare(this->_string,"$")==0){
       // return 6;
        return 1;
        //return 0;
    }


    else if(this->type==Token2::TypeCode &&
            QString::compare(this->_string,"ArrayInit")==0){
        return 1;
    }
    else if(this->type==Token2::TypeCode &&
            QString::compare(this->_string,"ArrayLookUp")==0){
        return 10;
    }
    else{
        return 0;
    }
}

bool Token2::isLeftUnaryOperator(){
    if(QString::compare(this->_string,"!")==0){
        return true;
    }
/*    else if(QString::compare(this->data,"--")==0){
        return true;
    }*/
    return false;
}

bool Token2::isRightUnaryOperator(){
    if(QString::compare(this->_string,"++")==0){
        return true;
    }
    else if(QString::compare(this->_string,"--")==0){
        return true;
    }
    return false;
}

bool Token2::isKeyWord(){
    bool res=false;
    if(this->type==Token2::TypeCode){
        if(QString::compare(this->_string,"lambda")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"class")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"literal")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"if")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"else")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"elseif")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"while")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"do")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"for")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"in")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"include")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"return")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"let")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"fix")==0){
            res=true;
        }
    }
    return res;
}

bool Token2::isInteger(){
    bool ok;
    int id = this->_string.toInt(&ok,10);
    return ok;
}
bool Token2::isFloat(){
    QString str1;
    QString str2;
    QStringList stringlist = this->_string.split('.');
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
int Token2::toInteger(){
    bool ok;
    int id = this->_string.toInt(&ok,10);
    return id;
}
float Token2::toFloat(){
    return this->_string.toFloat();
}

void Token2::setAsAddress(int i){
    this->address = i;
}

bool Token2::isOperator(){
    bool res=false;
    if(this->type==Token2::TypeCode){
        if(QString::compare(this->_string,"+")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"-")==0){
            res=true;
        }
        else if(QString::compare(this->_string,",")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"*")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"/")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"%")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"$")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"&&")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"||")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"!")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"=")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"!=")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"==")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"!==")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"===")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"^")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"<")==0){
            res=true;
        }
        else if(QString::compare(this->_string,">")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"<=")==0){
            res=true;
        }
        else if(QString::compare(this->_string,">=")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"++")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"--")==0){
            res=true;
        }
/*        else if(QString::compare(this->_string,",")==0){
            res=true;
        }*/
    }
    else if(this->type==Token2::TypeOperator){
        if(QString::compare(this->_string,"$")==0){
            res=true;
        }
        else if(QString::compare(this->_string,"ARRAY_LOOK_UP")==0){
            res=true;
        }
    }
    return res;
}

int Token2::asInt(){
    bool ok;
    int id = this->_string.toInt(&ok,10);
    return id;
}

PByteArray* Token2::compile(){
    PByteArray* res = new PByteArray();
    res->resize(4);
    qDebug()<<"Compiling token..."<<this->_string;
    if(this->type==TypeOperator || this->isOperator()==true){
        qDebug()<<"is operator";
        if(QString::compare(this->_string,"$")==0){
            res->setByte(0,'$');
            res->setByte(1,NULL);
            res->setByte(2,NULL);
            res->setByte(3,NULL);
        }
        else if(QString::compare(this->_string,"ARRAY_LOOK_UP")==0){
            res->setByte(0,'^');
            res->setByte(1,NULL);
            res->setByte(2,NULL);
            res->setByte(3,NULL);
        }
        else if(QString::compare(this->_string,"ARRAY_INIT")==0){
            res->setNum(Architecture::ArrayInitLocation);
            res->resize(4);
        }
        else if(QString::compare(this->_string,"RPN")==0){
            res->setNum(Architecture::RPNAddress+(4*(this->data)));
            res->resize(4);
        }
        else if(QString::compare(this->_string,"DUDRIGHT")==0){
            res->setNum(0);
            res->resize(4);
        }
        else{
            int i=0;
            while(i<this->_string.size()){
                qDebug()<<"setting as operator..";
                res->setByte(i,this->_string[i].toLatin1());
                i++;
            }
            while(i<4){
                qDebug()<<"flling with null..";
                res->setByte(i,NULL);
                i++;
            }
        }
    }
    else if(this->type==Token2::TypeString){
        qDebug()<<"Compiling string("+this->_string+") to address:"<<this->address;
        res->setNum(this->address);
    }
    else if(this->type==Token2::TypeCode){
        qDebug()<<"code type:"<<this->address;
        if(QString::compare(this->_string,"native")==0){
            res->setNum(Architecture::NativeLocation);
        }
        else if(QString::compare(this->_string,"this")==0){
            qDebug()<<"Compiled a this pointer!";
            res->setNum(Architecture::ThisAddress);
        }
        else if(QString::compare(this->_string,"true")==0){
            qDebug()<<"Compiled a this pointer!";
            res->setNum(Architecture::TrueLocation);
        }
        else if(QString::compare(this->_string,"false")==0){
            qDebug()<<"Compiled a this pointer!";
            res->setNum(Architecture::FalseLocation);
        }
        else if(QString::compare(this->_string,"unset")==0){
            qDebug()<<"Compiled a this pointer!";
            res->setNum(Architecture::UnsetLocation);
        }
        else if(QString::compare(this->_string,"null")==0){
            qDebug()<<"Compiled a this pointer!";
            res->setNum(Architecture::NULLLocation);
        }
        else{
            qDebug()<<"checking for integer to address:"<<this->address;
            if(this->isInteger()==true || this->isFloat()==true){
                qDebug()<<"Compiling integer or float to address:"<<this->address;
                res->setNum(this->address);
            }
            else{
                qDebug()<<"is not an integer:"<<this->_string;
                qDebug()<<"Compiling key("+this->_string+") to address:"<<this->address;
                res->setNum(this->address);
            }
//            res->setNum(this->data);
        }
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
