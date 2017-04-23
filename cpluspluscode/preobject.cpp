#include "preobject.h"
#include "errorregistry.h"
#include <QDebug>
#include "architecture.h"
#include "hasher.h"
#include "heap.h"


PreObject* PreObject::newStaticClass(int lexicalId,QVector<Line*> lines){
    PreObject* starter = new PreObject(lexicalId,lines,"class");
  //  PreObject* starter = new PreObject(lexicalId,lines,1);
    return starter;
}
PreObject* PreObject::newInstancePrototype(int lexicalId,QVector<Line*> lines){
    PreObject* starter = new PreObject(lexicalId,lines,"prototype");
 //   PreObject* starter = new PreObject(lexicalId,lines,3);
    return starter;
}
PreObject* PreObject::newInstanceBluePrint(int lexicalId,QVector<Line*> lines){
    PreObject* starter = new PreObject(lexicalId,lines,"blueprint");
//    PreObject* starter = new PreObject(lexicalId,lines,3);
    return starter;
}
void PreObject::setProto(int id){
    this->proto=id;
}

void PreObject::setProperty(QString propertyName, int valueAddress, bool abstract, bool final, int privacy){
    PreProperty* prop = new PreProperty();
    prop->setName(propertyName);
    prop->heapPointer = valueAddress;
    prop->setAsAbstract(abstract);
    prop->setAsFinal(final);
    this->properties.append(prop);
}

PreObject::PreObject(int lexicalId,QVector<Line*> lines,QString mode){
    //mode - class, prototype, blueprint, literal...
    this->lexicalOffset=0;
    qDebug()<<"constructing pre object...";
    this->isLambda=false;
    this->lexicalId = lexicalId;
    this->proto=-1;
    this->init = -1;
    int i=0;
    PreProperty* property=new PreProperty();
    bool inArgs=false;
    bool inMethod=false;
    int methodCount=0;
    QString domain="literal";//0-instance,1-prototype,2-static
    if(QString::compare(mode,"class")==0){
        domain=mode;
    }
    while(i<lines.size()){
        int e=0;
        while(e<lines[i]->getData().size()){
            int o=0;
            while(o<lines[i]->getData()[e]->getData().size()){
                if(property->hasName()==false){
                    if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"*")==0){
                        if(QString::compare(mode,"literal")==0){
                            ErrorRegistry::logError("Unexpected class identifier * in literal.");
                        }
                        else{
                            domain = "class";
                        }
                    }

                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"**")==0){
                        if(QString::compare(mode,"literal")==0){
                            ErrorRegistry::logError("Unexpected class identifier ** in literal.");
                        }
                        else{
                            domain = "prototype";
                        }
                    }

                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),".")==0){
                        if(QString::compare(mode,"literal")==0){
                            ErrorRegistry::logError("Unexpected class identifier . in literal.");
                        }
                        else{
                            domain = "blueprint";
                        }
                    }

                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"-")==0){
                        property->setAs("-");
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"+")==0){
                        property->setAs("+");
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"&")==0){
                        property->setAs("&");
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"!")==0){
                        property->setAs("!");
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"#")==0){
                        property->setAs("#");
                    }
                    else{
                        property->setName(lines[i]->getData()[e]->getData()[o]->getString());
                    }
                }
                else{
                    if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),":")==0){
                        if(inMethod==true){
                            qDebug().noquote()<<"\n\n\n\n*****************************************\n\n\n\n\nAppending method\n\n\n\n\nn\n\n";
                            inMethod=false;
                            if(methodCount<lines[i]->getChilds().size()){
                                qDebug()<<"I got this lines for the lexical id sourcing..."+lines[i]->getChilds()[methodCount]->toString(0);
                                property->complexPointer = lines[i]->getChilds()[methodCount]->getLexicalId();
                            }
                            else{
                                qDebug().noquote()<<"\n\n\n\n*****************************************\n\n\n\n\nnot setting complex pointer?";
                            }
                            methodCount++;                            
                            if(QString::compare(mode,"literal")==0){
                                this->properties.append(property);
                            }
                            else if(QString::compare(domain,mode)==0){
                                this->properties.append(property);
                            }
                            qDebug()<<"\nMethod added:"<<property->toString(0)<<"\n";
                            property = new PreProperty();
                        }
                        else{
                            property->initScoop=true;
                        }
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"=")==0){
                        property->initScoop=true;
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),";")==0){
                        if(QString::compare(mode,"literal")==0){
                            this->properties.append(property);
                            domain = "blueprint";
                        }
                        else if(domain==mode){
                            this->properties.append(property);
                            domain = "blueprint";
                        }
                        qDebug().noquote()<<"\nProperty added by line terminator ; "<<property->toString(0)<<"\n";
                        property = new PreProperty();
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),",")==0){
                        if(inArgs==false){
                            if(QString::compare(mode,"literal")==0){
                                this->properties.append(property);
                                domain = "blueprint";
                            }
                            else if(QString::compare(domain,mode)==0){
                                this->properties.append(property);
                                domain = "blueprint";
                            }
    //                        qDebug().noquote()<<"\nProperty added by line terminator ; "<<property->toString(0)<<"\n";
                            property = new PreProperty();
                        }
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),"(")==0){
                        if(property->initScoop==false){
                            inArgs=true;
                            inMethod=true;
                        }
                        else{
                            property->appendInit(lines[i]->getData()[e]->getData()[o]);
                        }
                    }
                    else if(QString::compare(lines[i]->getData()[e]->getData()[o]->getString(),")")==0){
                        inArgs=false;
                        if(property->initScoop==true){
                            property->appendInit(lines[i]->getData()[e]->getData()[o]);
                        }
                    }
                    else{
                        if(inArgs==false){
                            if(property->initScoop==true){
                                property->appendInit(lines[i]->getData()[e]->getData()[o]);
                            }
                            else{
                            //    ErrorRegistry::logError("Unexpected token in property string");
                            }
                        }
                    }
                }
                o++;
            }
            e++;
        }


        if(property->hasName()==true){
            if(QString::compare(mode,"literal")==0){
                this->properties.append(property);
                domain="blueprint";
            }
            else if(QString::compare(domain,mode)==0){
                this->properties.append(property);
                domain="blueprint";
            }
        }


        i++;
    }
    //was here
    qDebug()<<"Created PreObject"<<this->toString(0);
}

void PreObject::setInit(int index){
    this->init=index;
}

bool PreObject::needsInit(){
    if(this->isLambda==false){
        bool needs = false;
        int i=0;
        while(i<this->properties.size() && needs==false){
            if(this->properties[i]->initializers.size()>0){
                needs=true;
            }
            i++;
        }
        return needs;
    }
    return false;
}

QString PreObject::toString(int indent){
    int i=0;
    QString tabs="";
    while(i<indent){
        tabs+="\t";
        i++;
    }
    QString res=tabs+"Pre-Compilation Object[lexid:"+QString::number(this->lexicalId)+"]:\n";
    res+=tabs+"Proto reference to "+QString::number(this->proto)+" entry in the heap literal store,\n"+QString::number(this->init)+" init method n the literal store, and\nIs a lambda:"+QString::number(this->isLambda)+"\n";
    i=0;
    while(i<this->properties.size()){
        res+=this->properties[i]->toString(indent+1);
        i++;
    }
    return res;
}

PreObject* PreObject::newLambda(int lexicalId){
    PreObject* res = new PreObject(lexicalId,{},0);
    res->setAsLambda(true);
    return res;
}

void PreObject::setAsLambda(bool toggle){
    this->isLambda=toggle;
}

//int SIZE_OF_INT=3;

void PreObject::repointPropertyNames(QVector<int> oldAddresses,QVector<int> newAddresses){
    int i=0;
    while(i<this->properties.size()){
        qDebug()<<"Property Address Considering for replacement: "<<this->properties[i]->name;
        int o=0;
        bool found=false;
        while(o<oldAddresses.size() && found==false){
            if(this->properties[i]->name==oldAddresses[o]){
                qDebug()<<"Found Address of prop name";
                this->properties[i]->name=newAddresses[o];
                qDebug()<<"REset to:"<<newAddresses[o];
                found=true;
            }
            o++;
        }
        if(found==false){
            qDebug()<<"Property Name Address not found.";
        }
        i++;
    }
}

void PreObject::repointProtoAndInitPointers(QVector<int> oldAddresses,QVector<int> newAddresses){
    if(this->isLambda==false){
        if(this->proto!=-1){
            int o=0;
            bool found=false;
            while(o<oldAddresses.size() && found==false){
                if(this->proto==oldAddresses[o]){
                    qDebug()<<"Found Address of prop name";
                    this->proto=newAddresses[o];
                    found=true;
                }
                o++;
            }
            if(found==false){
                qDebug()<<"Proto Address not found.";
            }
        }
        if(this->init!=-1){
            int o=0;
            bool found=false;
            while(o<oldAddresses.size() && found==false){
                if(this->init==oldAddresses[o]){
                    qDebug()<<"replacing init address "<<this->init<<" with "<<newAddresses[o];
                    this->init=newAddresses[o];
                    found=true;
                }
                o++;
            }
            if(found==false){
                qDebug()<<"Init Address not found.";
            }
        }
        int i=0;
        while(i<this->properties.size()){
            //complex member checking
            if((!(this->properties[i]->complexPointer==-1 && this->properties[i]->heapPointer==-1)) && (this->properties[i]->heapPointer!=-1)){
                int o=0;
                bool found=false;
                while(o<oldAddresses.size() && found==false){
                    if(this->properties[i]->heapPointer==oldAddresses[o]){
                        qDebug()<<"Found Address of complex member, replacing"<<oldAddresses[o]<<" with "<<newAddresses[o];
                        this->properties[i]->heapPointer=newAddresses[o];
                        found=true;
                    }
                    o++;
                }
                if(found==false){
                    qDebug()<<"method Address not found.";
                }
            }
            i++;
        }
    }
}

void PreObject::repointMethodPointers(QVector<int> oldAddresses,QVector<int> newAddresses){
    if(this->isLambda==false){
        int i=0;
        while(i<this->properties.size()){

            //method checking
            if((!(this->properties[i]->complexPointer==-1 && this->properties[i]->heapPointer==-1)) && (this->properties[i]->complexPointer!=-1)){
                int o=0;
                bool found=false;
                while(o<oldAddresses.size() && found==false){
                    if(this->properties[i]->complexPointer==oldAddresses[o]){
                        qDebug()<<"Found Address of method, replacing"<<oldAddresses[o]<<" with "<<newAddresses[o];
                        this->properties[i]->complexPointer=newAddresses[o];
                        found=true;
                    }
                    o++;
                }
                if(found==false){
//                    qDebug()<<"method Address not found.";
                    ErrorRegistry::logError("Critical Compile error, method not found for object:"+this->toString(0));
                }
            }
            i++;
        }
    }
}

int PreObject::compileSize(QVector<QString> primitives,QVector<int> primitiveAddresses,int rpnCount){
    return this->compileSize(primitives,primitiveAddresses,rpnCount,1);
}
void PreObject::setLexicalOffset(int i){
    this->lexicalOffset=i;
}
int PreObject::compileSize(QVector<QString> primitives,QVector<int> primitiveAddresses,int rpnCount,int factor){
    PByteArray* res = new PByteArray();
    res->resize(1);
    if(this->isLambda==false){
        res->setByte(0,ByteCodeInstruction::TypeConstObjectType);
        //first lexical id.
        PByteArray* lexical_id = new PByteArray();
        lexical_id->setNum(this->lexicalId);
        lexical_id->resize(4);
        int e=0;
        while(e<lexical_id->size()){
            res->resize(res->size()+1);
            res->setByte(e+1,lexical_id->getByte(e));
            e++;
        }
        //now set protorefernce
        if(this->proto>=0){
            PByteArray* _proto = new PByteArray();
            _proto->setNum(this->proto);
            _proto->resize(4);
            e=0;
            while(e<_proto->size()){
                res->resize(res->size()+1);
                res->setByte(e+5,_proto->getByte(e));
                e++;
            }
        }
        else{
            PByteArray* _proto = new PByteArray();
            _proto->setNum(Architecture::UnsetLocation);
            _proto->resize(4);
            e=0;
            while(e<_proto->size()){
                res->resize(res->size()+1);
                res->setByte(e+5,_proto->getByte(e));
                e++;
            }
        }
        //and initialisers...

        if(this->init>=0){
            //init==-1 is fine for compilation
            //but negative integers will become garbage during interpretation
            //due to casting
            PByteArray* _init = new PByteArray();
            _init->setNum(this->init);
            _init->resize(4);
            e=0;
            while(e<_init->size()){
                res->resize(res->size()+1);
                res->setByte(e+9,_init->getByte(e));
                e++;
            }
        }
        else{
            PByteArray* _init = new PByteArray();
            _init->setNum(Architecture::UnsetLocation);
            _init->resize(4);
            e=0;
            while(e<_init->size()){
                res->resize(res->size()+1);
                res->setByte(e+9,_init->getByte(e));
                e++;
            }
        }

        //now set size
        int size = this->properties.size()/Architecture::StandardObjectSize;
        if(this->properties.size()%Architecture::StandardObjectSize>0){
            size++;
        }
        if(size==0){
            size=1;
        }
        size = size*factor;
        PByteArray* object_size = new PByteArray();
        object_size->setNum(size);
        qDebug()<<"Setting size of object:"<<QString::number(size);
        object_size->resize(4);
        e=0;
        while(e<object_size->size()){
           res->resize(res->size()+1);
           res->setByte(e+13,object_size->getByte(e));
           e++;
        }

        //reserving bytes...

        e=0;
        while(e<size){
            int o=0;
            while(o<9){
                int x=0;
                while(x<Architecture::StandardObjectSize){
                    res->resize(res->size()+1);
                    res->setByte(res->size()-1,NULL);
                    x++;
                }
                o++;
            }
            e++;
        }

        int actualSize = res->size();

        //now set properties as if a hash map...

        QVector<int> usedIndexices={};

        //if(preExistingHeap->size()>0){
        if(primitiveAddresses.size()>0){

            int i=0;
            int index = (1+4+4+4+4);
            while(i<this->properties.size()){

                index = (1+4+4+4+4);

                //int e=(this->properties[i]->name)+(13+5)-(Architecture::RPNAddress + (rpnCount*4));
                int e=this->properties[i]->name;
                qDebug()<<"Property name addres::"<<e;

                int o=0;
                bool keyFound=false;
                while(o<primitiveAddresses.size() && keyFound==false){
                    if(primitiveAddresses[o]==e){
                        keyFound=true;
                    }
                    else{
                        o++;
                    }
                }
                if(keyFound==true){


                    QString str = primitives[o];
                    qDebug()<<"Property Name harvested during compilation from pre-existing heap"<<str;
                    int hashResult = Hasher::SuperFastHash(str);
                    qDebug()<<"Resulting in index:"<<hashResult<<" % "<<size;
                    hashResult = hashResult % ((size*Architecture::StandardObjectSize)-1);

                    qDebug()<<"Resulting in index:"<<hashResult;

                    int baseindex = index;

                    index+= hashResult*9;
                    qDebug()<<"Resulting in index:"<<index;

                    if(index>=(actualSize-5)){
                        ErrorRegistry::logError("Key out of range for hashmap during compilation. Key:"+str);
                    }

                    int q=0;
                    qDebug()<<"index:"<<index;
                    bool checked=false;
                    while(q<usedIndexices.length() && checked==false){
                        if(usedIndexices[q]==index){
                            index = hashResult;
                           // int max = size*Architecture::StandardObjectSize;
                            int max = size*Architecture::StandardObjectSize-1;
                            int third = 4*size;
                            int less = 8*size;
                            index+=third;
                            index-=less;
                            if(index>=Architecture::StandardObjectSize*size){
                                index = ((Architecture::StandardObjectSize*size)-1)-index;
                            }
                            else if(index<0){
                                index = index+(size*Architecture::StandardObjectSize);
                            }
                            qDebug()<<"index adjusted:"<<index;
                            hashResult = index;
                            index = baseindex+(index*9);
                            int q1=0;
                            qDebug()<<"index adjusted actual:"<<index;

                            int compindex = index;
                            index = hashResult;

                            while(q1<usedIndexices.length() && checked==false){

                                qDebug()<<"comparing:"<<compindex<<"and"<<usedIndexices[q1];
                                if(usedIndexices[q1]==compindex){
                                    third = 8*size;
                                    int less = 12*size;
                                    index+=third;
                                    index-=less;
                                    if(index>=16){
                                        index = 15-index;
                                    }
                                    else if(index<0){
                                        index = index+(size*Architecture::StandardObjectSize);
                                    }
                                    qDebug()<<"index adjusted again:"<<index;
                                    index = baseindex+(index*9);
                                    int q2=0;
                                    qDebug()<<"index adjusted again:"<<index;
                                    while(q2<usedIndexices.length() && checked==false){
                                        if(usedIndexices[q2]==index){
                                            checked=true;
                                            //we must deal with this recursively

                                            //ErrorRegistry::logError("Error during compilation - object hashmap full, duplicate index:"+QString::number(index)+" on object:"+this->toString(0));
                                            return this->compileSize(primitives,primitiveAddresses,rpnCount,factor*2);
                                        }
                                        q2++;
                                    }
                                    checked=true;
                                }
                                q1++;
                            }
                            if(checked==false){
                                index=compindex;
                            }
                            checked=true;
                        }
                        q++;
                    }

                    usedIndexices.append(index);

                    PByteArray* propNamePointer = new PByteArray();
                    propNamePointer->setNum(this->properties[i]->name);
                    propNamePointer->resize(4);
                    char modifierBit = 0x00;

                    modifierBit = this->properties[i]->privacy;

                    res->setByte(index,modifierBit);
                    res->setByte(index+1,propNamePointer->getByte(0));
                    res->setByte(index+2,propNamePointer->getByte(1));
                    res->setByte(index+3,propNamePointer->getByte(2));
                    res->setByte(index+4,propNamePointer->getByte(3));
                    PByteArray* unsetLocate = new PByteArray();
                    unsetLocate->resize(4);

                    if((!(this->properties[i]->complexPointer==-1 && this->properties[i]->heapPointer==-1)) && (this->properties[i]->complexPointer!=-1)){
                        unsetLocate->setNum(this->properties[i]->complexPointer);
                    }
                    else{
                        unsetLocate->setNum(Architecture::UnsetLocation);
                    }

                    res->setByte(index+5,unsetLocate->getByte(0));
                    res->setByte(index+6,unsetLocate->getByte(1));
                    res->setByte(index+7,unsetLocate->getByte(2));
                    res->setByte(index+8,unsetLocate->getByte(3));

                }
                else{
                    QString errString = "Compile Time Error: Could not extrapolate object size, non existant property with addres "+QString::number(e)+":<br>";
                    int q=0;
                    while(q<primitiveAddresses.size()){
                        errString+=QString::number(q)+": "+QString::number(primitiveAddresses[q])+"  "+primitives[q]+"<br>";
                        q++;
                    }
                    ErrorRegistry::logError(errString);
                }
                i++;
            }
        }
        if(res->size()!=actualSize){
            qDebug()<<"SIZE ERROR ON OBJECT!!!!!!!!!!!";
        }
    }
    else{
        res->setByte(0,ByteCodeInstruction::TypeLambdaType);
        res->resize(9);
        PByteArray* _lexCode = new PByteArray();
        _lexCode->setNum(this->lexicalId);
        _lexCode->resize(4);
        res->setByte(1,_lexCode->getByte(0));
        res->setByte(2,_lexCode->getByte(1));
        res->setByte(3,_lexCode->getByte(2));
        res->setByte(4,_lexCode->getByte(3));

        _lexCode->resize(0);
        _lexCode->setNum(this->instructionSetPointer);
        _lexCode->resize(4);
        res->setByte(5,_lexCode->getByte(0));
        res->setByte(6,_lexCode->getByte(1));
        res->setByte(7,_lexCode->getByte(2));
        res->setByte(8,_lexCode->getByte(3));
    }
    return res->size();
}

PByteArray* PreObject::compile(int rpnCount,PByteArray* preExistingHeap){
    return this->compile(rpnCount,preExistingHeap,1);
}

PByteArray* PreObject::compile(int rpnCount,PByteArray* preExistingHeap,int factor){
    PByteArray* res = new PByteArray();
    res->resize(1);
    if(this->isLambda==false){
        res->setByte(0,ByteCodeInstruction::TypeConstObjectType);

        //reserve gutter
        res->resize(Architecture::TypeConstObjectGutter);
        res->encodeRawIntegerAt(Architecture::TypeObjectGutterToLexId,this->lexicalId);
        if(this->proto>=0){
            res->encodeRawIntegerAt(Architecture::TypeObjectGutterToProto,this->proto);
        }
        else{
            res->encodeRawIntegerAt(Architecture::TypeObjectGutterToProto,Architecture::UnsetLocation);
        }
        //and initialisers...

        if(this->init>=0){
            //init==-1 is fine for compilation
            //but negative integers will become garbage during interpretation
            //due to casting
            res->encodeRawIntegerAt(Architecture::TypeObjectGutterToInit,this->init);
        }
        else{
            res->encodeRawIntegerAt(Architecture::TypeObjectGutterToInit,Architecture::UnsetLocation);
        }

        //now set size
        int size = this->properties.size()/Architecture::StandardObjectSize;
        if(this->properties.size()%Architecture::StandardObjectSize>0){
            size++;
        }
        if(size==0){
            size=1;
        }
        size = size*factor;
        res->encodeRawIntegerAt(Architecture::TypeObjectGutterToSize,size);
        //reserving bytes...

        int e=0;
        while(e<size){
            int o=0;
            while(o<Architecture::TypeObjectPropertySize){
                int x=0;
                while(x<Architecture::StandardObjectSize){
                    res->resize(res->size()+1);
                    res->setByte(res->size()-1,NULL);
                    x++;
                }
                o++;
            }
            e++;
        }

        int actualSize = res->size();

        //now set properties as if a hash map...

        QVector<int> usedIndexices={};

        if(preExistingHeap->size()>0){

            int i=0;
            int index = (Architecture::TypeConstObjectGutter);
            while(i<this->properties.size()){
                bool isSet=false;
                int slot = 0;
                int e=(this->properties[i]->name)+(Architecture::TypeConstObjectGutter+1)-(Architecture::RPNAddress + (rpnCount*4));
                qDebug()<<"Property name addres::"<<e;
                QString str = preExistingHeap->decodeStringFrom(e);

                qDebug().noquote()<<"Compilation of property:"<<str;

                while(isSet==false && slot<3){
                    index = (Architecture::TypeConstObjectGutter);

                    int hashResult;

                    if(slot==0){
                        hashResult=Heap::expectedIndex(size,str);
                    }
                    else{
                        hashResult = Heap::shiftIndex(size,hashResult,slot);
                    }

                    int q=0;
                    bool checked=false;
                    while(q<usedIndexices.length() && checked==false){
                        if(usedIndexices[q]==hashResult){
                       /*     index = hashResult;
                            index = Heap::shiftIndex(size,hashResult,1);
                            hashResult = index;
                            index = baseindex+(index*Architecture::TypeObjectPropertySize);
                            int q1=0;
                            qDebug()<<"index adjusted actual:"<<index;

                            int compindex = index;
                            index = hashResult;

                            while(q1<usedIndexices.length() && checked==false){

                                qDebug()<<"comparing:"<<compindex<<"and"<<usedIndexices[q1];
                                if(usedIndexices[q1]==compindex){
                                    index = Heap::shiftIndex(size,index,2);
                                    qDebug()<<"index adjusted again:"<<index;
                                    index = baseindex+(index*9);
                                    int q2=0;
                                    qDebug()<<"index adjusted again:"<<index;
                                    while(q2<usedIndexices.length() && checked==false){
                                        if(usedIndexices[q2]==index){
                                            checked=true;
                                            //we must deal with this recursively

                                            //ErrorRegistry::logError("Error during compilation - object hashmap full, duplicate index:"+QString::number(index)+" on object:"+this->toString(0));
                                            return this->compile(rpnCount,preExistingHeap,factor*2);
                                        }
                                        q2++;
                                    }
                                    checked=true;
                                }
                                q1++;
                            }
                            if(checked==false){
                                index=compindex;
                            }*/
                            checked=true;
                        }
                        q++;
                    }
                    if(checked==true && slot==2){
                        qDebug().noquote()<<"Compilation of property:"<<str<<" require resize";
                        return this->compile(rpnCount,preExistingHeap,factor*2);
                    }
                    else if(!checked){

                        isSet = true;

                        usedIndexices.append(hashResult);

                        qDebug().noquote()<<"Compilation of property:"<<str<<" at index:"<<hashResult<<", on object with property count:"<<this->properties.size()<<", of size:("<<size<<"*16(standard object size)), being expected slot:"<<slot;

                        PByteArray* propNamePointer = new PByteArray();
                        propNamePointer->setNum(this->properties[i]->name);

                        qDebug()<<"prop name:"<<this->properties[i]->name;

                        propNamePointer->resize(4);
                        char modifierBit = 0x00;

                        modifierBit = this->properties[i]->privacy;

                        index = index+(hashResult*Architecture::TypeObjectPropertySize);

                        res->setByte(index,modifierBit);
                        res->encodeRawIntegerAt(index+Architecture::TypeObjectPropertyKeyIndent,this->properties[i]->name);
                        if((!(this->properties[i]->complexPointer==-1 && this->properties[i]->heapPointer==-1)) && (this->properties[i]->complexPointer!=-1)){
                            qDebug()<<"setting "<<str<<" as complex pointer...";
                            res->encodeRawIntegerAt(index+Architecture::TypeObjectPropertyValueIndent,this->properties[i]->complexPointer);
                        }
                        else if((!(this->properties[i]->complexPointer==-1 && this->properties[i]->heapPointer==-1)) && (this->properties[i]->heapPointer!=-1)){
                            qDebug()<<"setting "<<str<<" as heap pointer...";
                            res->encodeRawIntegerAt(index+Architecture::TypeObjectPropertyValueIndent,this->properties[i]->heapPointer);
                        }
                        else{
                            qDebug()<<"setting "<<str<<" as unset...";
                            res->encodeRawIntegerAt(index+Architecture::TypeObjectPropertyValueIndent,Architecture::UnsetLocation);
                        }

                    }
                    slot++;
                }
                i++;
            }
        }
    }
    else{
        res->setByte(0,ByteCodeInstruction::TypeLambdaType);
        res->resize(9);
        PByteArray* _lexCode = new PByteArray();
        _lexCode->setNum(this->lexicalId);
        _lexCode->resize(4);
        res->setByte(1,_lexCode->getByte(0));
        res->setByte(2,_lexCode->getByte(1));
        res->setByte(3,_lexCode->getByte(2));
        res->setByte(4,_lexCode->getByte(3));

        _lexCode->resize(0);
        _lexCode->setNum(this->instructionSetPointer);
        _lexCode->resize(4);
        res->setByte(5,_lexCode->getByte(0));
        res->setByte(6,_lexCode->getByte(1));
        res->setByte(7,_lexCode->getByte(2));
        res->setByte(8,_lexCode->getByte(3));
    }
    return res;
}

QVector<int> PreObject::setPointerToInstructionSet(PByteArray *instructionSet, QVector<int> matches){
    if(this->isLambda==true){
        qDebug()<<"Setting pointer to instruction set of lexical id:"<<this->lexicalId;
        qDebug()<<"With lexical offset:"<<this->lexicalOffset;
        bool repointed=false;
        int i=0;
        int encountered=-1;
        while(i<instructionSet->size() && repointed==false){
            if(instructionSet->getByte(i)==ByteCodeInstruction::TypeLEXStartMarker){
                qDebug()<<"Lex start marker found.";
                PByteArray* _lexId = new PByteArray();
                _lexId->resize(4);
                _lexId->setByte(0,instructionSet->getByte(i+1));
                _lexId->setByte(1,instructionSet->getByte(i+2));
                _lexId->setByte(2,instructionSet->getByte(i+3));
                _lexId->setByte(3,instructionSet->getByte(i+4));
                int lexId = _lexId->toInt();
                qDebug()<<"Lexical id="<<QString::number(lexId);
                if(lexId==this->lexicalId){
                    encountered++;
                    if(encountered==this->lexicalOffset){
                        qDebug()<<"Repointing inst set pointer to:"<<QString::number(i);
                        this->instructionSetPointer=(i+13);
                        repointed=true;
                        matches.append(lexId);
                    }
                }
            }
            i+=13;
        }
        if(repointed==false){
            qDebug()<<"Lambda instruction set pointer not repointed, lexical id:"+QString::number(this->lexicalId);
        }
    }
    return matches;
}
