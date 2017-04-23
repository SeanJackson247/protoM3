#include "pbytearray.h"
#include "errorregistry.h"
#include <QDebug>

#include "bytecodeinstruction.h"
#include "architecture.h"

PByteArray::PByteArray(){
    this->hasInnerReserve=false;
}

void PByteArray::reserve(int r){
    this->hasInnerReserve=true;
    this->innerReserve=r;
    this->data.reserve(r);
}

int PByteArray::size(){
    return this->data.size();
}

void PByteArray::encodeAsRawFloat(float x){
    this->resize(4);
    union{
        float f;
        unsigned char b[4];
    } u;
    u.f=x;
    int i=0;
    while(i<4){
        this->setByte(i,u.b[i]);
        i++;
    }
}

void PByteArray::encodeRawFloatAt(int dir,float x){
    union{
        float f;
        unsigned char b[4];
    } u;
    u.f=x;
    int i=dir;
    int e=0;
    while(e<4){
        this->setByte(i,u.b[e]);
        i++;
        e++;
    }
}

void PByteArray::resize(int size){
    if(this->hasInnerReserve==true){
        while(size>=this->innerReserve){
            this->innerReserve = 10*this->innerReserve;
            this->reserve(this->innerReserve);
        }
    }
    this->data.resize(size);
}

int PByteArray::decodeRawIntFrom(int index){
    if(index>this->data.size()-(Architecture::RawIntegerSize)){
       // ErrorRegistry::logError("Internal Error! Attempted reading decoding of integer at index > data.size() - raw int size(4) from within PByteArray class");
        return Architecture::NULLLocation;
    }
    else{
        PByteArray* res = new PByteArray();
        int i=0;
        while(i<Architecture::RawIntegerSize){
            res->resize(res->size()+1);
            res->setByte(res->size()-1,this->getByte(index+i));
            i++;
        }
        int x = res->toInt();
        return x;
    }
}

unsigned char PByteArray::getByte(int index){
    if(index>=0 && index<this->data.size()){
        return this->data.at(index);
    }
    else{
        ErrorRegistry::logError("Internal: index out of range on pbytearray");
        return NULL;
    }
}

QString PByteArray::decodeStringFrom(int index){
    QString res;
    int size = this->decodeRawIntFrom(index+Architecture::TypeStringGutterToSize)*Architecture::StandardStringSize;//16==stadardstringsize
    int GUTTER;
    if(this->getByte(index)==ByteCodeInstruction::TypeRunTimeStringType){
        qDebug()<<"Decoding runtime string:"<<res;
        GUTTER = Architecture::TypeRunTimeStringGutter;
    }
    else{
        qDebug()<<"Decoding constant string or key:"<<res;
        GUTTER = Architecture::TypeConstStringGutter;
    }
    index+=GUTTER;
    bool hitNull=false;
    int i=0;
    while(hitNull==false && i<size){
        if(this->getByte(index)==NULL){
            hitNull=true;
        }
        else{
            res+=this->getByte(index);
        }
        i++;
        index++;
    }
    qDebug()<<"Decoded string:"<<res;
    return res;
}

void PByteArray::encodeAsKey(QString string){
    this->resize(1);
    this->setByte(0,ByteCodeInstruction::TypeKeyType);
    int x = string.size()/Architecture::StandardStringSize;
    if(string.size()%Architecture::StandardStringSize>0){
        x++;
    }
    if(x==0){
        x=1;
    }
    PByteArray* lengthBytes = new PByteArray();
    lengthBytes->setNum(x);
    int i=0;
    while(i<lengthBytes->size()){
        this->resize(this->size()+1);
        this->setByte(this->size()-1,lengthBytes->getByte(i));
        i++;
    }
    int startIndex=this->size();
    this->resize(this->size()+(x*Architecture::StandardStringSize));
    i=0;
    int index=startIndex;
    while(index<this->size() && i<string.size()){
        this->setByte(index,string[i].toLatin1());
        i++;
        index++;
    }
    while(index<this->size()){
        this->setByte(index,NULL);
        index++;
    }
}


void PByteArray::encodeAsKeyAt(int position,QString string){
    PByteArray* newPB = new PByteArray();
    newPB->encodeAsKey(string);
    int i=position;
    int e=0;
    while(e<newPB->size()){
        this->setByte(i,newPB->getByte(e));
        i++;
        e++;
    }
}

void PByteArray::encodeAsStringAt(int position,QString string,bool isConstant){
    PByteArray* newPB = new PByteArray();
    newPB->encodeAsString(string,isConstant);
    int i=position;
    int e=0;
    while(e<newPB->size()){
        this->setByte(i,newPB->getByte(e));
        i++;
        e++;
    }
}

void PByteArray::encodeAsString(QString string,bool isConstant){
   // qDebug()<<"Encoding string...";
    this->encodeAsKey(string);
    if(isConstant){
        this->setByte(0,ByteCodeInstruction::TypeConstStringType);
    }
    else{
        this->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
        PByteArray* runtimestr = new PByteArray();
        int i=0;
        while(i<Architecture::TypeConstStringGutter){
            runtimestr->append(this->getByte(i));
            i++;
        }
        runtimestr->resize(runtimestr->size()+Architecture::RawIntegerSize);
        runtimestr->encodeRawIntegerAt(runtimestr->size()-Architecture::RawIntegerSize,0);
        i=Architecture::TypeConstStringGutter;
        while(i<this->size()){
            runtimestr->append(this->getByte(i));
            i++;
        }
        this->resize(0);
        i=0;
        while(i<runtimestr->size()){
            this->append(runtimestr->getByte(i));
            i++;
        }
    }
}

void PByteArray::encodeRawIntegerAt(int position,int data){
    PByteArray* _data = new PByteArray();
    _data->setNum(data);
    int i=0;
    while(i<_data->size()){
        this->setByte(position+i,_data->getByte(i));
        i++;
    }
}

void PByteArray::encodeAsRunTimeInteger(int refCount,int value){
    PByteArray* refC = new PByteArray();
    refC->setNum(refCount);
    PByteArray* val = new PByteArray();
    val->setNum(value);
    this->resize(Architecture::TypeRunTimeIntegerSize);
    this->setByte(0,ByteCodeInstruction::TypeRunTimeIntegerType);
    int i=0;
    int e=1;
    while(i<refC->size()){
        this->setByte(e,refC->getByte(i));
        e++;
        i++;
    }
    i=0;
    while(i<val->size()){
        this->setByte(e,val->getByte(i));
        e++;
        i++;
    }
}

void PByteArray::encodeAsRunTimeFloat(int refCount,float value){
    PByteArray* refC = new PByteArray();
    refC->setNum(refCount);
    PByteArray* val = new PByteArray();
    val->setNum(value);
    this->resize(Architecture::TypeRunTimeFloatSize);
    this->setByte(0,ByteCodeInstruction::TypeRunTimeFloatType);
    int i=0;
    int e=1;
    while(i<refC->size()){
        this->setByte(e,refC->getByte(i));
        e++;
        i++;
    }
    i=0;
    while(i<val->size()){
        this->setByte(e,val->getByte(i));
        e++;
        i++;
    }
}

void PByteArray::setByte(int index,unsigned char byte){
    this->data[index]=byte;
}

void PByteArray::prepend(unsigned char byte){
    this->data.prepend(byte);
}

void PByteArray::setNum(int index){
    std::vector<unsigned char> arrayOfByte(4);
    for (unsigned int i = 0; i < 4; i++){
         arrayOfByte[i] = (index >> (i * 8));
    }
    this->resize(4);
    this->data[0]=arrayOfByte[0];
    this->data[1]=arrayOfByte[1];
    this->data[2]=arrayOfByte[2];
    this->data[3]=arrayOfByte[3];
}

void PByteArray::setNum(float f){
    union{
        float f;
        unsigned char a[4];
    } u;
    u.f=f;
    /*for (unsigned int i = 0; i < 4; i++){
         arrayOfByte[i] = (index >> (i * 8));
    }*/
    this->resize(4);
    this->data[0]=u.a[0];
    this->data[1]=u.a[1];
    this->data[2]=u.a[2];
    this->data[3]=u.a[3];
}

int PByteArray::toInt(){

//    qDebug()<<"To int on pbytearray!";

    if(this->size()==4){
        unsigned int x = (((unsigned char)this->data[3]<<24)|((unsigned char)this->data[2]<<16)|((unsigned char)this->data[1]<<8)|((unsigned char)this->data[0]));
  //      qDebug()<<"Returning:"<<x;
        return x;
    }
    else{
        qDebug()<<"Error - Failed toInt() on PByteArray ; PByteArray.size() !=4 bytes.";
        ErrorRegistry::logError("Failed toInt() on PByteArray ; PByteArray.size() !=4 bytes.");
        return 0;
    }
}
void PByteArray::encodeLambdaAt(int position,int lexNode,int instructionPointer){
    this->setByte(position,ByteCodeInstruction::TypeLambdaType);
    this->encodeRawIntegerAt(position+Architecture::TypeLambdaGutterToLexId,lexNode);
    this->encodeRawIntegerAt(position+Architecture::TypeLambdaGutterToInstructionSet,instructionPointer);
}

void PByteArray::encodeLexRuleAt(int position,int parent, int child){
    this->setByte(position,ByteCodeInstruction::TypeLexRuleType);
    this->encodeRawIntegerAt(position+Architecture::TypeLexRuleGutterToParent,parent);
    this->encodeRawIntegerAt(position+Architecture::TypeLexRuleGutterToChild,child);
}

void PByteArray::copyInto(int position,PByteArray* sub,int length){
    int i=position;
    int l=position+length;
    int e=0;
    while(i<l){
        this->setByte(i,sub->getByte(e));
        e++;
        i++;
    }
}

float PByteArray::toFloat(){
    union{
        float f;
        unsigned char b[4];
    } u;
    u.b[0] = this->getByte(0);
    u.b[1] = this->getByte(1);
    u.b[2] = this->getByte(2);
    u.b[3] = this->getByte(3);
    return u.f;
}
void PByteArray::append(unsigned char byte){
    this->data.append(byte);
}
