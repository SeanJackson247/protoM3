#include "bytecodeinstruction2.h"
#include "architecture.h"
#include <QDebug>

ByteCodeInstruction2::ByteCodeInstruction2(int type, Token2 *A, Token2 *B, Token2 *C){
    this->isEndOfIf=false;
    this->conditionalId=0;
    this->type=type;
    this->A=A;
    this->B=B;
    this->C=C;
    this->idata=0;
}
void ByteCodeInstruction2::setRPNMarker(int rpn){
    this->idata = rpn;
}

QString ByteCodeInstruction2::toString(int indent){
    QString res="";
    int i=0;
    QString tabs="";
    while(i<indent){
        tabs+="\t";
        i++;
    }
    res+=tabs+"[[TYPE:";
    if(this->type==TypeRPN){
        res+="[RPN]["+QString::number(this->idata)+"]";
    }
    else if(this->type==TypeEARG){
        res+="[EARG]";
    }
    else if(this->type==TypeLineNumberSetType){
        res+="[LINE]";
    }
    else if(this->type==TypeLET){
        res+="[LET]";
    }
    else if(this->type==TypeFIX){
        res+="[FIX]";
    }
    else if(this->type==TypeGOTOMarker){
        res+="[GOTOMARKER]";
    }
    else if(this->type==TypeLEXStartMarker){
        res+="[LEX-start-MARKER]";
    }
    else if(this->type==TypeLEXEndMarker){
        res+="[LEX-end-MARKER]";
    }
    else if(this->type==TypeGOTOLoopMarker){
        res+="[GOTO-LOOP-MARKER]";
    }
    else if(this->type==TypeGOTOFCompMarker){
        res+="[GOTO-FCOMP-MARKER]";
    }
    else if(this->type==TypeCOMP){
        res+="[CMP]";
    }
    else if(this->type==TypeFCOMP){
        res+="[FCMP]";
    }
    else if(this->type==TypeJMP){
        res+="[JMP]";
    }
    else if(this->type==TypeBackJMP){
        res+="[BJMP]";
    }
    else if(this->type==TypeReturnMarker){
        res+="[RTN]";
    }
    res+=",A:";
    if(this->type==TypeLineNumberSetType){
        res+=QString::number(this->A->data);
    }
    else if(this->A!=NULL){
        res+=this->A->_string+":d("+QString::number(this->A->data)+")"+":a("+QString::number(this->A->address)+")\t";
    }
    else{
        res+="NULL";
    }
    res+=",B:";
    if(this->B!=NULL){
        res+=this->B->_string+":d("+QString::number(this->B->data)+")"+":a("+QString::number(this->B->address)+")\t";
    }
    else{
        res+="NULL";
    }
    res+=",C:";
    if(this->C!=NULL){
        res+=this->C->_string+":d("+QString::number(this->C->data)+")"+":a("+QString::number(this->C->address)+")\t";
    }
    else{
        res+="NULL";
    }
    if(this->isEndOfIf){
        res+=" is EndOfIf , conditionalid:"+QString::number(this->conditionalId);
    }
    res+="]\n";
    return res;
}

PByteArray* ByteCodeInstruction2::compile(LexNode* ln){

    qDebug()<<"Compiling bytecode instruction....";

        PByteArray* res = new PByteArray();
        res->resize(1+(ByteCodeInstruction2::ArrayIntSize*3));
        res->setByte(0,this->type);
        if(this->type==ByteCodeInstruction2::TypeRPN){
            qDebug()<<"RPN Address Set:"<<Architecture::RPNAddress+(4*(this->idata));
            res->setByte(0,((unsigned char)(Architecture::RPNAddress+(4*(this->idata)))));
            qDebug()<<"is type rpn...";
            if(ln->HighestRPN<this->idata){
                qDebug()<<"upping rpn...";
                ln->HighestRPN=this->idata;
                qDebug()<<"highest rpn..."<<ln->HighestRPN;
            }
        }
        int i=1;
        int e=0;
        while(i<((ByteCodeInstruction2::ArrayIntSize*3))){
            qDebug()<<"loop!";
            PByteArray* code = new PByteArray();
            if(e==0){
                if(this->A!=NULL){
                    if(this->type==ByteCodeInstruction2::TypeLEXStartMarker || this->type==ByteCodeInstruction2::TypeBackJMP || this->type==ByteCodeInstruction2::TypeJMP || this->type==ByteCodeInstruction2::TypeLEXEndMarker){
                        code->setNum(this->A->data);
                        code->resize(4);
                    }
                    else if(this->type==ByteCodeInstruction2::TypeRPNReserve){
                        code->setNum(this->A->asInt());
                        code->resize(4);
                    }
                    else{
                        code = this->A->compile();
                    }
                }
                else{
                    code->resize(4);
                    code->setByte(0,NULL);
                    code->setByte(1,NULL);
                    code->setByte(2,NULL);
                    code->setByte(3,NULL);
                }
            }
            else if(e==1){
                if(this->B!=NULL){
                    code = this->B->compile();
                }
                else{
                    code->resize(4);
                    code->setByte(0,NULL);
                    code->setByte(1,NULL);
                    code->setByte(2,NULL);
                    code->setByte(3,NULL);
                }
            }
            else{
                if(this->C!=NULL){
                    code = this->C->compile();
                }
                else{
                    code->resize(4);
                    code->setByte(0,NULL);
                    code->setByte(1,NULL);
                    code->setByte(2,NULL);
                    code->setByte(3,NULL);
                }
            }
            qDebug()<<"performed sub-compilation";
            res->setByte(i,code->getByte(0));
            res->setByte(i+1,code->getByte(1));
            res->setByte(i+2,code->getByte(2));
            res->setByte(i+3,code->getByte(3));
            i+=ByteCodeInstruction2::ArrayIntSize;
            e++;
        }
        qDebug()<<"escaped loop!Instruction code size:"<<QString::number(res->size());
        return res;
}
