#include "preproperty.h"
#include "heapstore.h"

PreProperty::PreProperty(){
    this->heapPointer=-1;
    this->name=-1;
    this->isAbstract=false;
    this->isFinal=false;
    this->privacy=0;
    this->initScoop=false;
    this->initializers={};
    this->complexPointer=-1;
}

QString PreProperty::toString(int indent){
    int i=0;
    QString tabs="";
    while(i<indent){
        tabs+="\t";
        i++;
    }
    QString res=tabs;
    res+="Name:#"+
            QString::number(this->name)+
            ",isAbstract:"+
            QString::number(this->isAbstract)+
            ",isFinal:"+
            QString::number(this->isFinal)+
            ",privacy:"+
            QString::number(this->privacy)+
            "\n"+tabs;
    res+="inits:[";
    if(this->complexPointer==-1 && this->heapPointer==-1){
        i=0;
        while(i<this->initializers.size()){
            res+="[#"+this->initializers[i]->getString()+"]";
            i++;
        }
    }
    else if(this->complexPointer!=-1){
        res+="Pointer to lexID:"+QString::number(this->complexPointer);
    }
    else{
        res+="Pointer to literal on the heap:"+QString::number(this->heapPointer);
    }
    res+="];\n";
    return res;
}

bool PreProperty::hasName(){
    if(this->name==-1){
        return false;
    }
    else{
        return true;
    }
}
void PreProperty::setName(QString name){
    int n = HeapStore::recievePrimitive(name);
    this->name=n;
}
void PreProperty::setAs(QString name){
    if(QString::compare(name,"-")==0){
        this->privacy=2;
    }
    else if(QString::compare(name,"#")==0){
        this->privacy=1;
    }
    else if(QString::compare(name,"+")==0){
        this->privacy=0;
    }
    else if(QString::compare(name,"&")==0){
        this->isAbstract=true;
    }
    else if(QString::compare(name,"!")==0){
        this->isFinal=true;
    }
}
void PreProperty::appendInit(Token*token){
//    Token* tester = new Token(init);
  //  if(tester->isOperator()==false){
//    if(token->){
  //  int n = HeapStore::recievePrimitive(init);
        this->initializers.append(token);
   /* }
    else{
        this->initializers.append(n);
    }*/
}
/*
void PreProperty::appendInit(int n){
    this->initializers.append(n);
}*/
void PreProperty::setAsAbstract(bool val){
    this->isAbstract=val;
}

void PreProperty::setAsFinal(bool val){
    this->isFinal=val;
}
