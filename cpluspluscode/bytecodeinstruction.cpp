#include "bytecodeinstruction.h"

#include <QVector>
#include "pbytearray.h"
#include <QDebug>
#include "line.h"
#include "heapstore.h"
#include "architecture.h"
#include "errorregistry.h"

ByteCodeInstruction::ByteCodeInstruction(int type, Token *A, Token *B, Token *C){
    this->type=type;
    this->A=A;
    this->B=B;
    this->C=C;
}

void ByteCodeInstruction::setRPNMarker(int rpn){
    this->idata = rpn;
}

QString ByteCodeInstruction::toString(int indent){
    QString res;
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
        res+=QString::number(this->A->idata);
    }
    else if(this->A!=NULL){
        res+=this->A->getString();
    }
    else{
        res+="NULL";
    }
    res+=",B:";
    if(this->B!=NULL){
        res+=this->B->getString();
    }
    else{
        res+="NULL";
    }
    res+=",C:";
    if(this->C!=NULL){
        res+=this->C->getString();
    }
    else{
        res+="NULL";
    }
    res+="]\n";
    return res;
}

QVector<ByteCodeInstruction*> ByteCodeInstruction::hoistVariables(QVector<ByteCodeInstruction*> lines){
    int i=0;
    QVector<ByteCodeInstruction*> newLines={};
    QVector<ByteCodeInstruction*> hoisted={};
    QVector<ByteCodeInstruction*> unhoisted={};
    while(i<lines.size()){
        if(lines[i]->type==ByteCodeInstruction::TypeLEXStartMarker || lines[i]->type==ByteCodeInstruction::TypeLEXEndMarker){
            int e=i+1;
            QVector<ByteCodeInstruction*> subs;
            bool done=false;
            unhoisted.append(lines[i]);
            while(e<lines.size() && done==false){
                if(lines[e]->type==ByteCodeInstruction::TypeLEXStartMarker || lines[e]->type==ByteCodeInstruction::TypeLEXEndMarker){
                    if(QString::compare(lines[e]->A->getString(),lines[i]->A->getString())==0){
                        done=true;
                        i=e;
                    }
                    else{
                        subs.append(lines[e]);
                    }
                }
                else{
                    subs.append(lines[e]);
                }
                e++;
            }
            e=0;
            subs = ByteCodeInstruction::hoistVariables(subs);
            while(e<subs.size()){
                unhoisted.append(subs[e]);
                e++;
            }
            unhoisted.append(lines[i]);
        }
        else{
            if(lines[i]->type==ByteCodeInstruction::TypeLET){
                hoisted.append(lines[i]);
            }
            else if(lines[i]->type==ByteCodeInstruction::TypeFIX){
                hoisted.append(lines[i]);
            }
            else{
                unhoisted.append(lines[i]);
            }
        }
        i++;
    }
    i=0;
    while(i<hoisted.size()){
        newLines.append(hoisted[i]);
        i++;
    }
    i=0;
    while(i<unhoisted.size()){
        newLines.append(unhoisted[i]);
        i++;
    }
    return newLines;
}

QVector<ByteCodeInstruction*> ByteCodeInstruction::ammendControlFlows(QVector<ByteCodeInstruction*> lines){
    QVector<ByteCodeInstruction*> res;
    qDebug()<<"amending control flows...";
    int i=0;
    bool cleanSweep=false;
    bool esc=false;
    while(cleanSweep==false){
        i=0;
        esc=false;
        while(i<lines.size() && esc==false){
            if(lines[i]->type==ByteCodeInstruction::TypeJMP){
                qDebug()<<"byte is JMP! at "<<i;
                int distance = (lines[i]->A->getString()).toInt();
                qDebug()<<"byte is JMP:"<<distance+i;
                if(lines.size()>distance+i){
                    qDebug()<<"in range...";
                    if(lines[i+distance]->type==ByteCodeInstruction::TypeLEXStartMarker){
                        qDebug()<<"points to start marker...";
                        if(lines[i+distance-1]->type==ByteCodeInstruction::TypeLEXEndMarker){
                            qDebug()<<"JMP to be ammended!";
                            lines.replace(i,new ByteCodeInstruction(ByteCodeInstruction::TypeJMP,new Token(QString::number(distance-1)),NULL,NULL));
                            esc=true;
                        }
                    }
                }
            }
            i++;
        }
        if(esc==false){
            cleanSweep=true;
        }
    }
    res=lines;
    return res;
}

QVector<ByteCodeInstruction*> ByteCodeInstruction::flattenControlFlows(QVector<ByteCodeInstruction*> lines){
    QVector<ByteCodeInstruction*> res;
    int i=0;
    int lastID=-1;
    int lastEntryID=-1;
   // int lastEntryID=0;
    bool esc=false;
    while(i<lines.size() && esc==false){
        if(lines[i]->type==ByteCodeInstruction::TypeGOTOMarker ||
           lines[i]->type==ByteCodeInstruction::TypeGOTOFCompMarker ||
           lines[i]->type==ByteCodeInstruction::TypeGOTOLoopMarker){
            bool ok;
            int id = lines[i]->A->getString().toInt(&ok,10);
            if(id==lastID){
                esc=true;
                int distance=i-lastEntryID;

                lines.remove(i);
                if(lines[lastEntryID]->type==ByteCodeInstruction::TypeGOTOMarker || lines[lastEntryID]->type==ByteCodeInstruction::TypeGOTOLoopMarker){
                    lines[lastEntryID-1]->type=ByteCodeInstruction::TypeCOMP;
                }
                else{
                    lines[lastEntryID-1]->type=ByteCodeInstruction::TypeFCOMP;
                }
                if(lines[lastEntryID]->type==ByteCodeInstruction::TypeGOTOLoopMarker){
                    int d=distance+1;
                    int l=i-d;
                    if(l>=0){
                        bool foundLine = false;
                        while(foundLine==false && l>=0){
                            if(lines[l]->type==ByteCodeInstruction::TypeLineNumberSetType){
                                foundLine=true;
                            }
                            else{
                                d++;
                            }
                            l--;
                        }
                        if(foundLine==true){
                        //    lines.insert(i,new ByteCodeInstruction(ByteCodeInstruction::TypeBackJMP,new Token(QString::number(distance+1)),NULL,NULL));
                            lines.insert(i,new ByteCodeInstruction(ByteCodeInstruction::TypeBackJMP,new Token(QString::number(d)),NULL,NULL));
                            lines.replace(lastEntryID,new ByteCodeInstruction(ByteCodeInstruction::TypeJMP,new Token(QString::number(distance+1)),NULL,NULL));
                        }
                        else{
                            ErrorRegistry::logError("Compile-Error: Malformed While Statement on line:"+QString::number(l));
                        }
                    }
                    else{
                        ErrorRegistry::logError("Compile-Error: Malformed While Statement on line:"+QString::number(l));
                    }
                }
                else{
                    lines.replace(lastEntryID,new ByteCodeInstruction(ByteCodeInstruction::TypeJMP,new Token(QString::number(distance)),NULL,NULL));
                }
            }
            else{
                lastID=id;
                lastEntryID=i;
            }
        }
        i++;
    }
    res=lines;
    if(esc==true){
       res=ByteCodeInstruction::flattenControlFlows(res);
    }
    return res;
}

QVector<ByteCodeInstruction*> ByteCodeInstruction::cleanup(QVector<ByteCodeInstruction*> lines){
    lines = ByteCodeInstruction::hoistVariables(lines);
    lines = ByteCodeInstruction::flattenControlFlows(lines);
    //lines = ByteCodeInstruction::ammendControlFlows(lines);
    int i=0;
    while(i<lines.size()){
        if(lines[i]->type==ByteCodeInstruction::TypeRPN){
            if(lines[i]->C==NULL){
                lines.remove(i);
                i--;
            }
        }
        i++;
    }

    //find highest rpn

    int highestRPN=0;
    i=0;
    while(i<lines.size()){
        if(lines[i]->type==ByteCodeInstruction::TypeRPN){
            if(lines[i]->idata>highestRPN){
                highestRPN = lines[i]->idata;
            }
        }
        i++;
    }

    lines.prepend(new ByteCodeInstruction(ByteCodeInstruction::TypeRPNReserve,new Token(QString::number(highestRPN)),NULL,NULL));

    return lines;
}

PByteArray* ByteCodeInstruction::compile(int rpnCount){

        PByteArray* res = new PByteArray();
        res->resize(1+(ByteCodeInstruction::ArrayIntSize*3));
        res->setByte(0,this->type);
        if(this->type==ByteCodeInstruction::TypeRPN){
            res->setByte(0,((unsigned char)(Architecture::RPNAddress+(4*(this->idata-1)))));
        }
        int i=1;
        int e=0;
        while(i<((ByteCodeInstruction::ArrayIntSize*3))){
            qDebug()<<"loop!";
            PByteArray* code = new PByteArray();
            if(e==0){
                if(this->A!=NULL){
                    if(this->type==ByteCodeInstruction::TypeRPNReserve || this->type==ByteCodeInstruction::TypeJMP || this->type==ByteCodeInstruction::TypeBackJMP || this->type==ByteCodeInstruction::TypeLEXStartMarker || this->type==ByteCodeInstruction::TypeLEXEndMarker){
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
            i+=ByteCodeInstruction::ArrayIntSize;
            e++;
        }
        qDebug()<<"escaped loop!Instruction code size:"<<QString::number(res->size());
        return res;
}

QVector<ByteCodeInstruction*> ByteCodeInstruction::fromInitializers(QVector<PreProperty*> properties){
    int i=0;
    QVector<ByteCodeInstruction*> res;
    while(i<properties.size()){

        if((!(properties[i]->complexPointer==-1 && properties[i]->heapPointer==-1)) && (properties[i]->complexPointer!=-1)){
           // do nothing cos its a method
            qDebug()<<"NOT Creating initializer for method:"<<properties[i]->toString(0);
        }
        else if((!(properties[i]->complexPointer==-1 && properties[i]->heapPointer==-1)) && (properties[i]->heapPointer!=-1)){
           // do nothing cos its a method
            qDebug()<<"NOT Creating initializer for complex member:"<<properties[i]->toString(0);
        }
        else{
            qDebug()<<"Creating initializers for "<<properties[i]->toString(0);
            QVector<Token*> tokens = {
                new Token("this"),
                new Token("."),
                new Token(Token::TypePrimitiveAddress,properties[i]->name),
                new Token("=")
            };
            qDebug()<<("this.["+QString::number(Token::TypePrimitiveAddress)+"::"+QString::number(properties[i]->name)+"]=");
            int e=0;
            while(e<properties[i]->initializers.size()){
                qDebug()<<"Appending property...";
         //       Token* tokenTester = new Token(HeapStore::primitives[properties[i]->initializers[e]]);
           //     if(tokenTester->isOperator()==false){
                    tokens.append(properties[i]->initializers[e]);
             //   }
               // else{
                 //   tokens.append(new Token(HeapStore::primitives[properties[i]->initializers[e]]));
                //}
                e++;
            }
            tokens.append(new Token(";"));
            Line* workingLine = new Line(tokens);
            QVector<ByteCodeInstruction*> temp = workingLine->_compile();

            qDebug()<<"Number of instrucitons acquired for property initiaization:"<<temp.size();

            int o=0;
            while(o<temp.size()){
                res.append(temp[o]);
                o++;
            }
            tokens={};
        }
        i++;
    }
    return res;
}
