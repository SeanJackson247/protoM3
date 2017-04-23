#include "heapstore.h"

#include <QDebug>
#include "architecture.h"

//int SIZE_OF_INT=4;

int HeapStore::lexicalID=0;
QVector<int> HeapStore::lexes={};
QVector<int> HeapStore::pars={};
//QVector<int> HeapStore::lambs={};
QVector<QString> HeapStore::primitives={};
QVector<PreObject*> HeapStore::literals={};

HeapStore::HeapStore(){

}

void HeapStore::addLexicalRule(int l,int p){

    qDebug()<<"Adding lexical rule:"<<l<<","<<p;

    HeapStore::lexes.append(l);
    HeapStore::pars.append(p);
}

void HeapStore::recieveLambda(int lexicalID){
    //HeapStore::lambs.append(lexicalID);
    HeapStore::literals.append(PreObject::newLambda(lexicalID));
}

void HeapStore::recieveLiteral(int lexicalId,QVector<Line*> lines){
    qDebug()<<"Recieving Literal!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1";
//    PreObject* preObject = new PreObject(lexicalId,lines,0);
    //we must also append a lambda for initialisation





    //    preObject->setProperty("init",HeapStore::lambs.size()-1,false,false,0);
//    HeapStore::literals.append(preObject);
//    qDebug().noquote()<<"\n\n\n::::::::::\nAppended Literal...\n\n\n\n\n\n\n";

    PreObject* pre = new PreObject(lexicalId,lines,"literal");

   // if(HeapStore::literals[HeapStore::literals.size()-1]->needsInit()==true){
        qDebug()<<"Lambda does need inits";
        HeapStore::recieveLambda(lexicalId);
        HeapStore::literals.append(pre);
        HeapStore::literals[HeapStore::literals.size()-1]->setInit(HeapStore::literals.size()-2);
    /*}
    else{
        qDebug()<<"Lambda does not need inits";
    }*/



}

void HeapStore::recieveClassDef(int lexicalId,QVector<Line*> lines){

    PreObject* staticClass = PreObject::newStaticClass(lexicalId,lines);
    PreObject* blueprint = PreObject::newInstanceBluePrint(lexicalId,lines);

    qDebug().noquote()<<"Static Class:"<<staticClass->toString(0);

    PreObject* prototype = PreObject::newInstancePrototype(lexicalId,lines);

    int firstLambdaIndex = HeapStore::literals.size();
    HeapStore::recieveLambda(lexicalId);
    HeapStore::literals.last()->setLexicalOffset(0);

    HeapStore::literals.append(prototype);
    int protoIndex = HeapStore::literals.size()-1;
    HeapStore::literals[protoIndex]->setInit(firstLambdaIndex);

    int secondLambdaIndex = HeapStore::literals.size();
    HeapStore::recieveLambda(lexicalId);
    HeapStore::literals.last()->setLexicalOffset(1);

    HeapStore::literals.append(blueprint);
    int blueprintIndex = HeapStore::literals.size()-1;
    HeapStore::literals[blueprintIndex]->setInit(secondLambdaIndex);

    int thirdLambdaIndex = HeapStore::literals.size();
    HeapStore::recieveLambda(lexicalId);
    HeapStore::literals.last()->setLexicalOffset(2);

    HeapStore::literals.append(staticClass);
    int staticIndex = HeapStore::literals.size()-1;
    HeapStore::literals[staticIndex]->setInit(thirdLambdaIndex);

    blueprint->setProto(protoIndex);
    //blueprint->setProperty("class",staticIndex,0,0,0);
    prototype->setProperty("class",staticIndex,0,0,0);
    staticClass->setProperty("blueprint",blueprintIndex,0,0,0);
    staticClass->setProperty("prototype",protoIndex,0,0,0);

    qDebug()<<"HeapStore recieved classedfintion...";
}

int HeapStore::recievePrimitive(QString primitive){
    if(QString::compare(primitive,")")!=0 &&
       QString::compare(primitive,",")!=0 &&
       QString::compare(primitive,":")!=0 &&
       QString::compare(primitive,";")!=0 &&
       QString::compare(primitive,"**")!=0 &&
       QString::compare(primitive,"&")!=0 &&
       QString::compare(primitive,"!")!=0
     ){
        bool valid=true;
        if(QString::compare("if",primitive)==0 ||
           QString::compare("elseif",primitive)==0 ||
           QString::compare("else",primitive)==0 ||
           QString::compare("while",primitive)==0 ||
           QString::compare("for",primitive)==0 ||
           QString::compare("return",primitive)==0 ||
           QString::compare("lambda",primitive)==0 ||
           QString::compare("literal",primitive)==0 ||
           QString::compare("classdef",primitive)==0 ||
           QString::compare("let",primitive)==0 ||
           QString::compare("fix",primitive)==0 ||
                QString::compare("this",primitive)==0 ||
                QString::compare("null",primitive)==0 ||
                QString::compare("true",primitive)==0 ||
                QString::compare("false",primitive)==0 ||
                QString::compare("unset",primitive)==0 ||
                QString::compare("native",primitive)==0){
            valid=false;
        }
        if(valid==true){
            int i=0;
            while(i<HeapStore::primitives.size()){
                if(QString::compare(HeapStore::primitives[i],primitive)==0){
                    return i;
                }
                i++;
            }
            if(valid==true){
                qDebug()<<"Appending primitive..."<<primitive;
                HeapStore::primitives.append(primitive);
                return HeapStore::primitives.size()-1;
            }
        }
    }
    return -1;
}

int HeapStore::newGoToId(){
    qDebug()<<"lexical id:"<<QString::number(HeapStore::lexicalID);
    HeapStore::lexicalID++;
    return HeapStore::lexicalID;
}

QVector<Line*> HeapStore::constructLexTree(QVector<Line*> lines){
    QVector<Line*> res;
    //res.append(new Line());

    //should be root...
    Line* shouldBRoot = lines[0]->getParent();
    qDebug().noquote()<<"\nSHOULD BE ROOT:\n"<<shouldBRoot->toString(0);
    res.append(shouldBRoot);

    int i=0;
    while(i<lines.size()){
       // newLines.append(lines[i]);
        QVector<Line*> lexMarkers = lines[i]->insertLexicalMarkers();
        int e=0;
        while(e<lexMarkers.size()){
            res.append(lexMarkers[e]);
            e++;
        }
        i++;
    }
    i=0;
    while(i<lines.size()){
    //    res.append(lines[i]);
        i++;
    }
    return res;
}
QVector<Line*> HeapStore::extractConstants(QVector<Line*> lines){
    return lines;
}
int HeapStore::getFreshLexicalId(){
    qDebug()<<"lexical id:"<<QString::number(HeapStore::lexicalID);
    HeapStore::lexicalID++;
    return HeapStore::lexicalID;
}

PByteArray* encodeAsRawInteger(int n){
    PByteArray* ba = new PByteArray();
    ba->setNum(n);
    while(ba->size()<Architecture::RawIntegerSize){
        ba->resize(ba->size()+1);
        ba->setByte(ba->size()-1,0x00);
    }
    ba->resize(Architecture::RawIntegerSize);
    return ba;
}

PByteArray* encodeAsInteger(int x,bool isConstant){
    PByteArray* bytes = new PByteArray();
    bytes->resize(Architecture::RawIntegerSize+1);
    if(isConstant==true){
        bytes->setByte(0,ByteCodeInstruction::TypeConstIntegerType);
    }
    else{
        bytes->setByte(0,ByteCodeInstruction::TypeRunTimeIntegerType);
    }
    PByteArray* subBytes = encodeAsRawInteger(x);
    int e=0;
    while(e<subBytes->size()){
        bytes->setByte(e+1,subBytes->getByte(e));
        e++;
    }
    return bytes;
}


PByteArray* encodeAsFloat(float x,bool isConstant){
    PByteArray* bytes = new PByteArray();
    //bytes->resize(5);
    bytes->encodeAsRawFloat(x);
    if(isConstant==true){
        bytes->prepend(ByteCodeInstruction::TypeConstFloatType);
    }
    else{
        bytes->prepend(ByteCodeInstruction::TypeRunTimeFloatType);
    }
/*    PByteArray sub_bytes = PByteArray(reinterpret_cast<const char*>(&x), sizeof(x));
    int i=0;
    int e=1;
    while(i<sub_bytes->size()){
        bytes->setByte(e,sub_bytes->getByte(i));
        e++;
        i++;
    }*/
    return bytes;
}

PByteArray* HeapStore::encodeAsKey(QString string){
    PByteArray* res = new PByteArray();
    res->resize(1);
    res->setByte(0,ByteCodeInstruction::TypeKeyType);
    int x = string.size()/Architecture::StandardStringSize;
    if(string.size()%Architecture::StandardStringSize>0){
        x++;
    }
    if(x==0){
        x=1;
    }
    PByteArray* lengthBytes = encodeAsRawInteger(x);
    int i=0;
    while(i<lengthBytes->size()){
        res->resize(res->size()+1);
        res->setByte(res->size()-1,lengthBytes->getByte(i));
        i++;
    }
    int startIndex=res->size();
    res->resize(res->size()+(x*Architecture::StandardStringSize));
    i=0;
    int index=startIndex;
    while(index<res->size() && i<string.size()){
        res->setByte(index,string[i].toLatin1());
        i++;
        index++;
    }
    while(index<res->size()){
        res->setByte(index,NULL);
        index++;
    }
    return res;
}

PByteArray* encodeAsString(QString string,bool isConstant){
    qDebug()<<"Encoding string...";
    string.remove(0,1);
    string.resize(string.size()-1);
    PByteArray* res = HeapStore::encodeAsKey(string);
    if(isConstant==true){
        res->setByte(0,ByteCodeInstruction::TypeConstStringType);
    }
    else{
        res->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
    }
    qDebug()<<"Encoded string...";
    return res;
}

PByteArray* encodeAsLexRule(int l,int p){
    PByteArray* res = new PByteArray();
    res->resize(1);
    res->setByte(0,ByteCodeInstruction::TypeLexRuleType);
    PByteArray* lbytes = encodeAsRawInteger(l);
    PByteArray* pbytes= encodeAsRawInteger(p);
    int i=0;
    while(i<lbytes->size()){
        res->resize(res->size()+1);
        res->setByte(res->size()-1,lbytes->getByte(i));
        i++;
    }
    i=0;
    while(i<pbytes->size()){
        res->resize(res->size()+1);
        res->setByte(res->size()-1,pbytes->getByte(i));
        i++;
    }
    return res;
}

//int SIZE_OF_HEAP_PRESETS=30;

PByteArray* HeapStore::encode(QVector<ByteCodeInstruction*> lines){
    PByteArray* res = new PByteArray();

    PByteArray* _rpnCountTest = new PByteArray();
    _rpnCountTest->resize(4);

    //copy over rpn reserve statement
    if(lines.first()->type==ByteCodeInstruction::TypeRPNReserve){
        PByteArray* rpnReserve = lines.first()->compile(0);
        int e=0;
        while(e<rpnReserve->size()){
            res->resize(res->size()+1);
            res->setByte(res->size()-1,rpnReserve->getByte(e));
            e++;
        }
    }
    else{
        qDebug()<<"Error! rpn reserve not where expected";
   //     ErrorRegistry::logError("RPN Reserve Marker not found.");
    }

    _rpnCountTest->setByte(0,res->getByte(1));
    _rpnCountTest->setByte(1,res->getByte(2));
    _rpnCountTest->setByte(2,res->getByte(3));
    _rpnCountTest->setByte(3,res->getByte(4));

    qDebug()<<"RPN Count set as:"<<_rpnCountTest->toInt();

    int rpnCount = _rpnCountTest->toInt();
    int i=res->size();

    res->resize(Architecture::RawIntegerSize+1+res->size());
    res->setByte(i,ByteCodeInstruction::TypeHeapFlush);

    //now we must resolve initializers of properties
    i=0;
    while(i<HeapStore::literals.size()){
       // int e=0;
        //while(e<HeapStore::literals[i]->properties.size()){

            //first check property is not a method

           // if(!((!(HeapStore::literals[i]->properties[e]->complexPointer==-1 && HeapStore::literals[i]->properties[e]->heapPointer==-1)) && (HeapStore::literals[i]->properties[e]->complexPointer!=-1))){
            if(HeapStore::literals[i]->isLambda==false){
                QVector<ByteCodeInstruction*> temp_inits = ByteCodeInstruction::fromInitializers(HeapStore::literals[i]->properties);
                int x=0;
                lines.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXStartMarker,new Token(QString::number(literals[i]->lexicalId)),NULL,NULL));
                while(x<temp_inits.size()){
                    lines.append(temp_inits[x]);
                    x++;
                }
                lines.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXEndMarker,new Token(QString::number(literals[i]->lexicalId)),NULL,NULL));
            }
            //}
         //   e++;
        //}
        i++;
    }
    i=0;
    qDebug()<<"initialisers...";
    while(i<lines.size()){
        qDebug()<<"("+QString::number(i)+"):"+lines[i]->toString(0);
        i++;
    }

    //set lex rules

    qDebug()<<"Encoding lex rules...";
    i=0;
    while(i<HeapStore::lexes.size()){
        qDebug()<<"encoding....";
        PByteArray* lexRule = encodeAsLexRule(HeapStore::lexes[i],HeapStore::pars[i]);
        qDebug()<<"encoded..."<<QString::number(lexRule->size());
        int e=0;
        while(e<lexRule->size()){
            qDebug()<<"Adding lexRule byte...";
            res->resize(res->size()+1);
            res->setByte(res->size()-1,lexRule->getByte(e));
            qDebug()<<"Added lexRule byte...";
            e++;
        }
        i++;
    }
    qDebug()<<"Encoded lex rules...";

    //right time to compile primitives...
    QVector<int> oldAddresses;
    QVector<int> newAddresses;
    i=0;
    qDebug()<<"Primtive rendering...";
    while(i<HeapStore::primitives.size()){
        Token* TestToken = new Token(HeapStore::primitives[i]);
        if(TestToken->isOperator()==false){
            qDebug()<<"not operator:"<<TestToken->getString();
            PByteArray* bytes = new PByteArray();
            if(TestToken->isInteger()==true){
                qDebug()<<"Encoding integer...."<<QString::number(TestToken->asInt());
                bytes = encodeAsInteger(TestToken->asInt(),true);
                qDebug()<<"The result of which is "<<QString::number(bytes->size())<<" bytes long.";
            }
            else if(TestToken->isFloat()==true){
                qDebug()<<"Encoding as float!!!";
                bytes = encodeAsFloat(TestToken->asFloat(),true);
                qDebug()<<"The result of which is "<<QString::number(bytes->size())<<" bytes long.";
            }
            else{
                qDebug()<<"Encoding string...."<<HeapStore::primitives[i];
                if(HeapStore::primitives[i][0].toLatin1()=='\'' || HeapStore::primitives[i][0].toLatin1()=='\"'){
                    bytes = encodeAsString(HeapStore::primitives[i],true);
                }
                else{
                    bytes = encodeAsKey(HeapStore::primitives[i]);
                }
                qDebug()<<"The result of which is "<<QString::number(bytes->size())<<" bytes long.";
            }
            oldAddresses.append(i);
            //newAddresses.append((res->size()-((Architecture::RawIntegerSize)+1))+Architecture::RPNAddress+(rpnCount*4));
            newAddresses.append((res->size()-((Architecture::RawIntegerSize)+14))+Architecture::RPNAddress+(rpnCount*4));
            qDebug()<<"Replaced old address "<<QString::number(oldAddresses.last())<<" with "<<QString::number(newAddresses.last());
            int lastIndex = res->size();
            res->resize(res->size()+bytes->size());
            int o=0;
            while(o<bytes->size()){
                res->setByte(lastIndex,bytes->getByte(o));
                o++;
                lastIndex++;
            }
        }
        else{
            HeapStore::primitives.remove(i);
        }
        i++;
    }
    qDebug()<<"end of Primtive rendering...";
    i=0;
    while(i<lines.size()){
        qDebug()<<"Lines(i="<<QString::number(i)<<")";
        if(lines[i]->type!=ByteCodeInstruction::TypeLineNumberSetType){
            if(lines[i]->A!=NULL){
                if(lines[i]->A->matched==false){
                    if(lines[i]->A->getType()==Token::TypePrimitiveAddress){
                        int e=0;
                        bool matched=false;
                        while(e<oldAddresses.size() && matched==false){
                            if(oldAddresses[e]==lines[i]->A->idata){
                                qDebug()<<"on A, Replaced old address "<<QString::number(oldAddresses[e])<<" with "<<QString::number(newAddresses[e]);
                                lines[i]->A->idata=newAddresses[e];
                                matched=true;
                                lines[i]->A->matched=true;
                            }
                            e++;
                        }
                    }
                }
            }
            if(lines[i]->B!=NULL){
                if(lines[i]->B->matched==false){
                    if(lines[i]->B->getType()==Token::TypePrimitiveAddress){
                        int e=0;
                        bool matched=false;
                        while(e<oldAddresses.size() && matched==false){
                            if(oldAddresses[e]==lines[i]->B->idata){
                                qDebug()<<"on B, Replaced old address "<<QString::number(oldAddresses[e])<<" with "<<QString::number(newAddresses[e]);
                                lines[i]->B->idata=newAddresses[e];
                                matched=true;
                                lines[i]->B->matched=true;
                            }
                            e++;
                        }
                    }
                }
            }
            if(lines[i]->C!=NULL){
                if(lines[i]->C->matched==false){
                    if(lines[i]->C->getType()==Token::TypePrimitiveAddress){
                        int e=0;
                        bool matched=false;
                        while(e<oldAddresses.size() && matched==false){
                            if(oldAddresses[e]==lines[i]->C->idata){
                                qDebug()<<"on C, Replaced old address "<<QString::number(oldAddresses[e])<<" with "<<QString::number(newAddresses[e]);
                                lines[i]->C->idata=newAddresses[e];
                                matched=true;
                                lines[i]->C->matched=true;
                            }
                            e++;
                        }
                    }
                }
            }
        }
        i++;
    }

    i=0;
    while(i<HeapStore::literals.size()){
        HeapStore::literals[i]->repointPropertyNames(oldAddresses,newAddresses);
        i++;
    }

    QVector<int> primitiveAddresses = newAddresses;

    //now we encode complex objects...

    oldAddresses={};
    newAddresses={};

    QVector<int> lexCodes = {};

    i=0;
    int currentIndex = res->size();
    while(i<HeapStore::literals.size()){
        PreObject* obj = HeapStore::literals[i];
        int index = res->size();
        int s = obj->compileSize(HeapStore::primitives,primitiveAddresses,rpnCount);
        qDebug()<<"Compiled Size of object expected:"<<s;
        res->resize(index+s);
        //newAddresses.append(index-(((Architecture::RawIntegerSize)+14)+Architecture::RPNAddress+(4*rpnCount)));
        newAddresses.append((index-((Architecture::RawIntegerSize)+14))+Architecture::RPNAddress+(rpnCount*4));
        oldAddresses.append(i);
        lexCodes.append(obj->lexicalId);
        i++;
    }

    //repoint all init and proto pointers

    i=0;
    while(i<HeapStore::literals.size()){
        HeapStore::literals[i]->repointProtoAndInitPointers(oldAddresses,newAddresses);
        HeapStore::literals[i]->repointMethodPointers(lexCodes,newAddresses);
        i++;
    }

    //was here


    //now to repoint all complex pointers...
    i=0;
    while(i<lines.size()){
        if(lines[i]->type!=ByteCodeInstruction::TypeLineNumberSetType){

            if(lines[i]->A!=NULL){
                if(lines[i]->A->getType()==Token::TypeComplexAddress){
                    int e=0;
                    while(e<oldAddresses.size()){
                        if(oldAddresses[e]==lines[i]->A->idata){
                            lines[i]->A->idata=newAddresses[e];
                        }
                        e++;
                    }
                }
            }
            if(lines[i]->B!=NULL){
                if(lines[i]->B->getType()==Token::TypeComplexAddress){
                    int e=0;
                    while(e<oldAddresses.size()){
                        if(oldAddresses[e]==lines[i]->B->idata){
                            lines[i]->B->idata=newAddresses[e];
                        }
                        e++;
                    }
                }
            }
            if(lines[i]->C!=NULL){
                if(lines[i]->C->getType()==Token::TypeComplexAddress){
                    int e=0;
                    while(e<oldAddresses.size()){
                        if(oldAddresses[e]==lines[i]->C->idata){
                            lines[i]->C->idata=newAddresses[e];
                        }
                        e++;
                    }
                }
            }
        }
        i++;
    }





 //   int heap_size = res->size()-(Architecture::RawIntegerSize+1);
    int heap_size = res->size()-(Architecture::RawIntegerSize+1+13);
    qDebug()<<"Set Heap Size:..."<<QString::number(heap_size);
    PByteArray* heap_size_bytes = encodeAsRawInteger(heap_size);
//    qDebug()<<"Encoded raw integer..."<<QString::number(heap_size_bytes.size());
    i=0;
    while(i<heap_size_bytes->size()){
        res->setByte(14+i,heap_size_bytes->getByte(i));
        i++;
    }

    //compile instructions

    //remove rpn reserver...

    lines.remove(0);

    i=0;
    PByteArray* compiledInstructions = new PByteArray();
    while(i<lines.size()){
        qDebug()<<"Compiling...";
        PByteArray* compiledInstruction = lines[i]->compile(rpnCount);
        int e=0;
        while(e<compiledInstruction->size()){
            compiledInstructions->resize(compiledInstructions->size()+1);
            compiledInstructions->setByte(compiledInstructions->size()-1,compiledInstruction->getByte(e));
            e++;
        }
        qDebug()<<"Compiled an instruction line.";
        i++;
    }

    //copy intstructions onto res

    qDebug()<<"Encoding instructions at:"<<res->size()<<" of type "<<compiledInstructions->getByte(0);

    i=0;
    while(i<compiledInstructions->size()){
        res->resize(res->size()+1);
        res->setByte(res->size()-1,compiledInstructions->getByte(i));
        i++;
    }

    //now to reset lambda pointers...
    //these are the pointers to the instruction which begins the function
    //and thus have to repointed after the instructions are compiled

    i=0;
    QVector<int> matches = {};
    while(i<HeapStore::literals.size()){
        matches = HeapStore::literals[i]->setPointerToInstructionSet(compiledInstructions,matches);
        i++;
    }

    //final compilation of objects
    i=0;
    while(i<HeapStore::literals.size()){
        PreObject* obj = HeapStore::literals[i];
        PByteArray* compiledObject = obj->compile(rpnCount,res);
        qDebug()<<"Compiled Size of object:..."<<compiledObject->size();
        int e=0;
        int n=currentIndex;
        while(e<compiledObject->size()){
            res->setByte(n,compiledObject->getByte(e));
            n++;
            e++;
        }
        currentIndex=n;
        i++;
    }


    _rpnCountTest->setByte(0,res->getByte(1));
    _rpnCountTest->setByte(1,res->getByte(2));
    _rpnCountTest->setByte(2,res->getByte(3));
    _rpnCountTest->setByte(3,res->getByte(4));

    qDebug()<<"RPN Count set as:"<<_rpnCountTest->toInt() << " at end of compilation.";


    return res;
}
