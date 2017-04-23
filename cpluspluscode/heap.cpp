#include "heap.h"
#include "hasher.h"
#include <QString>
#include <QDebug>

#include "architecture.h"

#include "bytecodeinstruction.h"
#include "errorregistry.h"

Heap::Heap(Interpreter* interpreter){
    this->interpreter=interpreter;
    this->data = new PByteArray();
    this->data->reserve(Architecture::INITIAL_HEAP_RESERVATION);
}

void Heap::accept(unsigned char byte){
    this->data->append(byte);
}

void Heap::appendToArray(int A_Address,int B_Address){
    ////this->interpreter->debuggerOut("Appending address "+QString::number(B_Address)+" to array, "+this->arrayToString(A_Address)+" , 2 args...",true);

    A_Address = this->completeDeref(A_Address);
    B_Address = this->completeDeref(B_Address);
    unsigned char A_Type = this->data->getByte(A_Address);
    unsigned char B_Type = this->data->getByte(B_Address);

    if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
        int newBAddress = this->data->size();
        PByteArray* _data = new PByteArray();
        _data->encodeAsRunTimeInteger(0,this->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
        this->copyToNew(_data,Architecture::TypeRunTimeIntegerSize);
        B_Address = newBAddress;
        B_Type = this->data->getByte(B_Address);
    }
    else if(B_Type==ByteCodeInstruction::TypeConstFloatType){
/*        int newBAddress = this->data->size();
        PByteArray* _data = new PByteArray();
        _data->encodeAsRunTimeFloat(0,this->decodeRawFloatFrom(B_Address+Architecture::TypeConstFloatGutter));
        this->copyToNew(_data,Architecture::TypeRunTimeFloatSize);
        B_Address = newBAddress;
        B_Type = this->data->getByte(B_Address);*/
    }
    else if(B_Type==ByteCodeInstruction::TypeConstStringType){
        int newBAddress = this->data->size();
        PByteArray* _data = new PByteArray();
        _data->encodeAsString(this->decodeStringFrom(B_Address),false);
        this->copyToNew(_data,_data->size());
        B_Address = newBAddress;
        B_Type = this->data->getByte(B_Address);
    }


    if(A_Type==ByteCodeInstruction::TypeOpenArrayType || A_Type==ByteCodeInstruction::TypeArgsListType){

        if(B_Type==ByteCodeInstruction::TypeArgsListType){
            ////this->interpreter->debuggerOut("Appending to args list",true);

            int i=0;
            int bSize = this->decodeRawIntFrom(A_Address+Architecture::TypeArgumentsListGutterToSize);
            while(i<bSize){
                int entryAddress = (Architecture::TypeArgumentsListGutter)+(i*Architecture::TypeArrayEntrySize);
                entryAddress = this->decodeRawIntFrom(entryAddress);
                this->appendToArray(A_Address,entryAddress);
                i++;
            }
        }
        else{
            ////this->interpreter->debuggerOut("Appending to array",true);
            int GUTTER,GUTTER_TO_SIZE,GUTTER_TO_CAPACITY;
            if(A_Type==ByteCodeInstruction::TypeOpenArrayType){
                GUTTER = Architecture::TypeArrayGutter;
                GUTTER_TO_SIZE = Architecture::TypeArrayGutterToSize;
                GUTTER_TO_CAPACITY = Architecture::TypeArrayGutterToCapacity;
            }
            else{
                GUTTER = Architecture::TypeArgumentsListGutter;
                GUTTER_TO_SIZE = Architecture::TypeArgumentsListGutterToSize;
                GUTTER_TO_CAPACITY = Architecture::TypeArgumentsListGutterToCapacity;
            }
            int capacity = this->decodeRawIntFrom(A_Address+GUTTER_TO_CAPACITY);
            int size = this->decodeRawIntFrom(A_Address+GUTTER_TO_SIZE);
            if(size==capacity){
                ////this->interpreter->debuggerOut("Appending to array or args list, BAddress:"+QString::number(B_Address)+" size==capacity",true);
                int lastAddress = A_Address+(GUTTER)+(capacity*Architecture::TypeArrayEntrySize);
                if(lastAddress == (this->data->size()-1)){
                    ErrorRegistry::logError("Unwritten Code:End of array is the last entry in the heap.");
                }
                else{
                    int newAAddress = this->data->size();
                    int addr = A_Address;
                    PByteArray* _data = new PByteArray();
                    while(addr<lastAddress){
                        _data->resize(_data->size()+1);
                        _data->setByte(_data->size()-1,this->data->getByte(addr));
                        this->data->setByte(addr,NULL);
                        addr++;
                    }
                    int nCap = _data->decodeRawIntFrom(1);
                    int x=0;
                    while(x<(nCap*Architecture::TypeArrayEntrySize)){
                        _data->resize(_data->size()+1);
                        _data->setByte(_data->size()-1,NULL);
                        x++;
                    }
                    nCap*=2;
                    _data->encodeRawIntegerAt(1,nCap);
                    this->copyToNew(_data,_data->size());
                    this->appendToArray(newAAddress,B_Address);
                    PByteArray* refPointer = new PByteArray();
                    refPointer->encodeAsRunTimeInteger(0,newAAddress);
                    refPointer->setByte(0,ByteCodeInstruction::TypeHeapFragment);
                    this->copyInTo(A_Address,refPointer,refPointer->size());
                }
            }
            else if(size<capacity){
                ////this->interpreter->debuggerOut("Appending to array or args list, BAddress:"+QString::number(B_Address)+" size is less than capacity",true);
                int location = A_Address+GUTTER+(size*Architecture::TypeArrayEntrySize);
                size++;
                this->data->encodeRawIntegerAt(location,B_Address);
                this->data->encodeRawIntegerAt(A_Address+GUTTER_TO_SIZE,size);
            }
            else{
                ErrorRegistry::logError("Error! Malformed Array size > capacity ? on type:"+QString::number(A_Type));
            }
            this->incrementReferenceCount(B_Address);
        }
    }
    else if(A_Type==ByteCodeInstruction::TypeEventList){
        int size = this->decodeRawIntFrom(A_Address+Architecture::TypeEventListGutterToSize);//1
        int location = A_Address+Architecture::TypeEventListGutter+(size*Architecture::TypeEventListEntrySize);
        this->data->encodeRawIntegerAt(location,B_Address);
        this->data->encodeRawIntegerAt(A_Address+Architecture::TypeEventListGutterToSize,size+1);
    }
    else{
        ErrorRegistry::logError("Error - cannot append to object, object is not an array or argumentslist, type given:"+QString::number(A_Type)+", at address #"+QString::number(A_Address)+" on line:"+QString::number(this->interpreter->lineNumber));
    }
}

void Heap::clearTopStackFrame(){
    int stackFrameAddress = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    ////////this->debuggerOut("Clear stack frame at address:"+QString::number(stackFrameAddress),true);
    if(this->data->getByte(stackFrameAddress)==Architecture::TypeLettedScope){
        ////////this->debuggerOut("Letted stack frame at address:"+QString::number(stackFrameAddress),true);
        int parentStackFrameAddress = this->data->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToParent);
        this->data->encodeRawIntegerAt(Architecture::CurrentStackFrameAddress,parentStackFrameAddress);
        int size = this->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToSize);
        int stackFrameEnd = stackFrameAddress+(Architecture::StandardStackFrameSize*size*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter;
        if(stackFrameAddress>=this->initThreshold){
            if(this->data->getByte(stackFrameAddress)==Architecture::TypeLettedScope || this->data->getByte(stackFrameAddress)==Architecture::TypeFixedScope){
                while(stackFrameEnd>=stackFrameAddress){
                    this->clear(stackFrameEnd);
                    stackFrameEnd--;
                }
            }
            else{
                ErrorRegistry::logError("Internal Error - attempted removal of non-stack frame as though it were a stack frame, type:"+QString::number(this->interpreter->lineNumber)+" given, at address:"+QString::number(stackFrameAddress)+", on line: "+QString::number(this->interpreter->lineNumber));
            }
        }
        else if(stackFrameAddress!=Architecture::NULLLocation && stackFrameAddress!=Architecture::UnsetLocation){
            ErrorRegistry::logError("Internal Error - attempted removal of non-stack frame as though it were a stack frame, type:"+QString::number(this->interpreter->lineNumber)+" given, at address:"+QString::number(stackFrameAddress)+", on line: "+QString::number(this->interpreter->lineNumber));
        }
    }
    else if(this->data->getByte(stackFrameAddress)==Architecture::TypeFixedScope){
        ////////this->debuggerOut("Fixed stack frame at address:"+QString::number(stackFrameAddress),true);
        bool esc=false;
        int lastStackFrame = stackFrameAddress;
        int type = this->data->getByte(stackFrameAddress);
        while(type==Architecture::TypeFixedScope && esc==false){
            lastStackFrame = stackFrameAddress;
            stackFrameAddress = this->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToParent);
            if(stackFrameAddress<this->initThreshold){
                esc=true;
            }
            else{
                type = this->data->getByte(stackFrameAddress);
            }
        }
        if(esc==false){
            int parentStackFrameAddress = this->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToParent);
            PByteArray* _data = new PByteArray();
            _data->setNum(parentStackFrameAddress);
            this->copyInTo(lastStackFrame+Architecture::TypeStackFrameGutterToParent,_data,Architecture::RawIntegerSize);
            int size = this->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToSize);
            int stackFrameEnd = stackFrameAddress+(Architecture::StandardStackFrameSize*size*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter;
            if(stackFrameAddress>=this->initThreshold){
                if(this->data->getByte(stackFrameAddress)==Architecture::TypeLettedScope || this->data->getByte(stackFrameAddress)==Architecture::TypeFixedScope){
                    while(stackFrameEnd>=stackFrameAddress){
                        this->clear(stackFrameEnd);
                        stackFrameEnd--;
                    }
                }
                else{
                    ErrorRegistry::logError("Internal Error - attempted removal of non-stack frame as though it were a stack frame, type:"+QString::number(this->interpreter->lineNumber)+" given, at address:"+QString::number(stackFrameAddress)+", on line: "+QString::number(this->interpreter->lineNumber));
                }
            }
            else if(stackFrameAddress!=Architecture::NULLLocation && stackFrameAddress!=Architecture::UnsetLocation){
                ErrorRegistry::logError("Internal Error - attempted removal of non-stack frame as though it were a stack frame, type:"+QString::number(this->interpreter->lineNumber)+" given, at address:"+QString::number(stackFrameAddress)+", on line: "+QString::number(this->interpreter->lineNumber));
            }
        }
    }
    else{
        ErrorRegistry::logError("Stackframe not where expected during stack frame removal, no stack frame at:"+QString::number(stackFrameAddress));
    }
}

void Heap::clear(int index){
    if(index==this->data->size()-1){
        this->data->resize(this->data->size()-1);
    }
    else{
        this->data->setByte(index,NULL);
    }
}

QStringList Heap::getPropertyList(int oAddress){
    QStringList res={};
    int GUTTER;
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER=(Architecture::TypeConstObjectGutter);
    }
    else{
        GUTTER=(Architecture::TypeObjectGutter);
    }
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        int size = this->data->decodeRawIntFrom(oAddress+Architecture::TypeObjectGutterToSize);
        oAddress+=GUTTER;
        size = size*Architecture::StandardObjectSize;
        int i=0;
        while(i<size){
            if(this->data->getByte(oAddress)==NULL){
                int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                if(_address==Architecture::NULLLocation){
                }
                else if(_address==Architecture::UnsetLocation){
                }
                else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                    QString propKey = decodeStringFrom(_address);
                    res.append(propKey);
                    _address = decodeRawIntFrom(oAddress+Architecture::TypePropertyKeyGutter);
                }
            }
            i++;
            oAddress+=Architecture::TypeObjectPropertySize;
        }
    }
    else{
        //ErrorRegistry::logError("Internal:Attempted to harvest property list from non-object.");
        return res;
    }
    return res;
}

QVector<int> Heap::getPropertyKeyList(int oAddress){
    QVector<int> res={};
    int GUTTER;
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER=(Architecture::TypeConstObjectGutter);
    }
    else{
        GUTTER=(Architecture::TypeObjectGutter);
    }
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        int size = this->data->decodeRawIntFrom(oAddress+Architecture::TypeObjectGutterToSize);
        oAddress+=GUTTER;
        size = size*Architecture::StandardObjectSize;
        int i=0;
        while(i<size){
            if(this->data->getByte(oAddress)==NULL){
                int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                if(_address==Architecture::NULLLocation){
                }
                else if(_address==Architecture::UnsetLocation){
                }
                else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                    res.append(_address);
                }
            }
            i++;
            oAddress+=Architecture::TypeObjectPropertySize;
        }
    }
    else{
        ErrorRegistry::logError("Internal:Attempted to harvest property list from non-object.");
    }
    return res;
}

QVector<int> Heap::getPropertyValuesList(int oAddress){
    QVector<int> res={};
    int GUTTER;
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER=(Architecture::TypeConstObjectGutter);
    }
    else{
        GUTTER=(Architecture::TypeObjectGutter);
    }
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        int size = this->data->decodeRawIntFrom(oAddress+Architecture::TypeObjectGutterToSize);
        oAddress+=GUTTER;
        size = size*Architecture::StandardObjectSize;
        int i=0;
        while(i<size){
            if(this->data->getByte(oAddress)==NULL){
                int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                if(_address==Architecture::NULLLocation){
                }
                else if(_address==Architecture::UnsetLocation){
                }
                else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                    res.append(this->decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyValueIndent));
                }
            }
            i++;
            oAddress+=Architecture::TypeObjectPropertySize;
        }
    }
    else{
        ErrorRegistry::logError("Internal:Attempted to harvest property list from non-object.");
    }
    return res;
}

QVector<unsigned char> Heap::getPropertyModifiersList(int oAddress){
    QVector<unsigned char> res={};
    int GUTTER;
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER=(Architecture::TypeConstObjectGutter);
    }
    else{
        GUTTER=(Architecture::TypeObjectGutter);
    }
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        int size = this->data->decodeRawIntFrom(oAddress+Architecture::TypeObjectGutterToSize);
        oAddress+=GUTTER;
        size = size*Architecture::StandardObjectSize;
        int i=0;
        while(i<size){
            if(this->data->getByte(oAddress)==NULL){
                int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                if(_address==Architecture::NULLLocation){
                }
                else if(_address==Architecture::UnsetLocation){
                }
                else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                    res.append(this->data->getByte(oAddress));
                }
            }
            i++;
            oAddress+=Architecture::TypeObjectPropertySize;
        }
    }
    else{
        ErrorRegistry::logError("Internal:Attempted to harvest property list from non-object.");
    }
    return res;
}

int FACTOR=4;

int Heap::shiftIndex(int size,int index,int factor){
    factor*=FACTOR;
    int max = size*Architecture::StandardObjectSize;
    int shift = factor*size;
    index-=shift;
    index+=(2*shift);
    if(index<0){
        index=-index;
    }
    if(index/max>=1){
        index = index%max;
    }
    if(index>=max){
        index=max-1;
    }
    return index;
}

int Heap::expectedIndex(int size,QString key){
    int hashResult = Hasher::SuperFastHash(key);
    hashResult = hashResult % ((size*Architecture::StandardObjectSize)-1);
    return hashResult;
}

int Heap::propertyValueAddressLookUp(int objectAddress,QString key,int stringAddress,bool alongChain){
    int GUTTER;
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        GUTTER = Architecture::TypeObjectGutter;
    }
    else if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER = Architecture::TypeConstObjectGutter;
    }
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        int objectStartAddress = objectAddress;
        int size = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToSize);
        int index = this->expectedIndex(size,key);
        int keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
        if(keyAddress==stringAddress){
            if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
            }
            else{
                int privacy = this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize));
                if(privacy>=1){
                    int lexicalId = this->interpreter->getLexicalID();
                    //now check if the lexical id of the currently executing function
                    //is a child of the lexical id of the object in question.
                    int objectLexicalId = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToLexId);
                    if(lexicalId==objectLexicalId){
                        return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                    }
                    else if(lexicalId==0){
                        ErrorRegistry::logError("Could not accesss private member "+key+" on object, from within the context of line "+QString::number(this->interpreter->lineNumber));
                        return Architecture::UnsetLocation;
                    }
                    else{
                        bool pathFound=false;
                        bool esc=false;
                        while(pathFound==false && esc==false){
                            int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;
                            unsigned char lexType = this->data->getByte(lexRuleAddress);
                            if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                                ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                                return Architecture::UnsetLocation;
                            }
                            else{
                                while(lexType==ByteCodeInstruction::TypeLexRuleType){
                                    int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                                    int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                                    if(child==lexicalId){
                                        if(parent!=0){
                                            lexicalId = parent;
                                            if(lexicalId==objectLexicalId){
                                                pathFound=true;
                                            }
                                        }
                                        else{
                                            esc=true;
                                        }
                                    }
                                    lexRuleAddress+=Architecture::TypeLexRuleSize;
                                    lexType = this->data->getByte(lexRuleAddress);
                                }
                            }
                        }
                        if(pathFound==true){
                            return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                        }
                        else{
                            if(privacy!=1){
                                ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                            }
                            else{
                                if(alongChain==true){
                        //            ErrorRegistry::logError("Unwritten Code, attempted look up of protected member:"+key);
                                    return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                                }
                                else{
                                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                                }
                            }
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                }
                return Architecture::UnsetLocation;
            }
        }
        else{
            index = this->shiftIndex(size,index,1);
            keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
            if(keyAddress==stringAddress){
                if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                    return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                }
                else{
                    ErrorRegistry::logError("Unwritten Code:: Look up on object, is in second place for non-private. address,key address,string");
                }
            }
            else{
                index = this->shiftIndex(size,index,2);
                int keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
                if(keyAddress==stringAddress){
                    if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                        return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                    }
                    else{
                        ErrorRegistry::logError("Unwritten Code:: Look up on object, is in third place for non-private.");
                    }
                }
                else{
                    int protoAddress = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToProto);
                    if(protoAddress>1){
                        return this->propertyValueAddressLookUp(protoAddress,key,stringAddress);
                    }
                    else{
                        int objectProto = this->decodeRawIntFrom(Architecture::ObjectLocation);
                        if(objectProto<2){
                            return Architecture::UnsetLocation;
                        }
                        else{
                            if(objectStartAddress!=objectProto){
                                return this->propertyValueAddressLookUp(objectProto,key,stringAddress);
                            }
                            else{
                                return Architecture::UnsetLocation;
                            }
                        }
                    }
                }
            }
        }
        return Architecture::UnsetLocation;
    }
    else{
        ErrorRegistry::logError("Error! Look Up Non-Object, with key:"+key+", at address:"+QString::number(objectAddress)+", on line:"+QString::number(this->interpreter->lineNumber));
    }
    return Architecture::UnsetLocation;
}

bool Heap::isLexicalParentOf(int child,int parent){
    int lexicalId = child;
    int objectLexicalId = parent;
    if(lexicalId==objectLexicalId){
        return true;
    }
    else if(lexicalId==0){
 //       ErrorRegistry::logError("Could not accesss private member "+key+" on object, in first expected slot:"+QString::number(index)+" from within the context of line "+QString::number(this->interpreter->lineNumber));
        return false;
    }
    else{
        bool pathFound=false;
        bool esc=false;
        while(pathFound==false && esc==false){
            int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;
            unsigned char lexType = this->data->getByte(lexRuleAddress);
            if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                return Architecture::UnsetLocation;
            }
            else{
                while(lexType==ByteCodeInstruction::TypeLexRuleType){
                    int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                    int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                    if(child==lexicalId){
                        if(parent!=0){
                            lexicalId = parent;
                            if(lexicalId==objectLexicalId){
                                pathFound=true;
                            }
                        }
                        else{
                            esc=true;
                        }
                    }
                    lexRuleAddress+=Architecture::TypeLexRuleSize;
                    lexType = this->data->getByte(lexRuleAddress);
                }
            }
        }
        return pathFound;
    }
}

int Heap::propertyValueAddressLookUp(int objectAddress,QString key,bool alongChain){
    int GUTTER;
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        GUTTER = Architecture::TypeObjectGutter;
    }
    else if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER = Architecture::TypeConstObjectGutter;
    }
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        int objectStartAddress = objectAddress;
        int size = this->decodeRawIntFrom(objectAddress+Architecture::TypeObjectGutterToSize);
        int factor = 0;
        QString str;
        int index;
        int keyAddress;
        while(factor<3){
            if(factor==0){
                index = this->expectedIndex(size,key);
                //////this->debuggerOut("expected index:"+QString::number(index)+" for key:"+key,true);
                keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
                if(ErrorRegistry::errorCount>0){
                    ErrorRegistry::logError("Error During looking up key.");
                }
                str = this->decodeStringFrom(keyAddress);
            }
            else{
                index = this->shiftIndex(size,index,factor);
                keyAddress = this->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
                str = this->decodeStringFrom(keyAddress);
            }
            if(QString::compare(key,str)==0){
                if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                    return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                }
                else{
                    int privacy = this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize));
                    if(privacy>=1){
                        int lexicalId = this->interpreter->getLexicalID();
                        //now check if the lexical id of the currently executing function
                        //is a child of the lexical id of the object in question.
                        int objectLexicalId = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToLexId);
                        if(lexicalId==objectLexicalId){
                            return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                        }
                        else if(lexicalId==0){
                            ErrorRegistry::logError("Could not accesss private member "+key+" on object, in first expected slot:"+QString::number(index)+" from within the context of line "+QString::number(this->interpreter->lineNumber));
                            return Architecture::UnsetLocation;
                        }
                        else{
                            bool pathFound=false;
                            bool esc=false;
                            while(pathFound==false && esc==false){
                                int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;
                                unsigned char lexType = this->data->getByte(lexRuleAddress);
                                if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                                    ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                                    return Architecture::UnsetLocation;
                                }
                                else{
                                    while(lexType==ByteCodeInstruction::TypeLexRuleType){
                                        int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                                        int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                                        if(child==lexicalId){
                                            if(parent!=0){
                                                lexicalId = parent;
                                                if(lexicalId==objectLexicalId){
                                                    pathFound=true;
                                                }
                                            }
                                            else{
                                                esc=true;
                                            }
                                        }
                                        lexRuleAddress+=Architecture::TypeLexRuleSize;
                                        lexType = this->data->getByte(lexRuleAddress);
                                    }
                                }
                            }
                            if(pathFound==true){
                                return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                            }
                            else{
                                if(privacy!=1){
                                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                                }
                                else{
                                    if(alongChain==true){
    //                                    ErrorRegistry::logError("Unwritten Code, attempted look up of protected member:"+key);
                                        return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                                    }
                                    else{
                                        ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                                    }
                                }
                            }
                        }
                    }
                    else{
                        ErrorRegistry::logError("Could not accesss non-public member "+key+" on object, from the context of line:"+QString::number(this->interpreter->lineNumber));
                    }
                    return Architecture::UnsetLocation;
                }
            }
            else if(factor==2){
                int protoAddress = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToProto);
                if(protoAddress>1){
                    return this->propertyValueAddressLookUp(protoAddress,key,true);
                }
                else{
                    int objectProto = this->decodeRawIntFrom(Architecture::ObjectLocation);
                    if(objectProto<2){
                        return Architecture::UnsetLocation;
                    }
                    else{
                        if(objectStartAddress!=objectProto){
                            return this->propertyValueAddressLookUp(objectProto,key,true);
                        }
                        else{
                            return Architecture::UnsetLocation;
                        }
                    }
                }
            }
            factor++;
        }
        /*
        if(QString::compare(key,str)==0){
            if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
            }
            else{
                int privacy = this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize));
                if(privacy>=1){
                    int lexicalId = this->interpreter->getLexicalID();
                    //now check if the lexical id of the currently executing function
                    //is a child of the lexical id of the object in question.
                    int objectLexicalId = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToLexId);
                    if(lexicalId==objectLexicalId){
                        return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                    }
                    else if(lexicalId==0){
                        ErrorRegistry::logError("Could not accesss private member "+key+" on object, in first expected slot:"+QString::number(index)+" from within the context of line "+QString::number(this->interpreter->lineNumber));
                        return Architecture::UnsetLocation;
                    }
                    else{
                        bool pathFound=false;
                        bool esc=false;
                        while(pathFound==false && esc==false){
                            int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;
                            unsigned char lexType = this->data->getByte(lexRuleAddress);
                            if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                                ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                                return Architecture::UnsetLocation;
                            }
                            else{
                                while(lexType==ByteCodeInstruction::TypeLexRuleType){
                                    int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                                    int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                                    if(child==lexicalId){
                                        if(parent!=0){
                                            lexicalId = parent;
                                            if(lexicalId==objectLexicalId){
                                                pathFound=true;
                                            }
                                        }
                                        else{
                                            esc=true;
                                        }
                                    }
                                    lexRuleAddress+=Architecture::TypeLexRuleSize;
                                    lexType = this->data->getByte(lexRuleAddress);
                                }
                            }
                        }
                        if(pathFound==true){
                            return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                        }
                        else{
                            if(privacy!=1){
                                ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                            }
                            else{
                                if(alongChain==true){
//                                    ErrorRegistry::logError("Unwritten Code, attempted look up of protected member:"+key);
                                    return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                                }
                                else{
                                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                                }
                            }
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object, from the context of line:"+QString::number(this->interpreter->lineNumber));
                }
                return Architecture::UnsetLocation;
            }
        }
        else{
            index = this->shiftIndex(size,index,1);
            keyAddress = this->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
            QString str = this->decodeStringFrom(keyAddress);
            if(QString::compare(key,str)==0){
                if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                    return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                }
                else{
                    ErrorRegistry::logError("Unwritten Code:: Look up on object, is in second place for non-private. (address,key,chain)");
                }
            }
            else{
                index = this->shiftIndex(size,index,2);
                keyAddress = this->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
                QString str = this->decodeStringFrom(keyAddress);
                if(QString::compare(key,str)==0){
                    if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                        return objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent;
                    }
                    else{
                        ErrorRegistry::logError("Unwritten Code:: Look up on object, is in third place for non-public.");
                    }
                }
                else{
                    int protoAddress = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToProto);
                    if(protoAddress>1){
                        return this->propertyValueAddressLookUp(protoAddress,key,true);
                    }
                    else{
                        int objectProto = this->decodeRawIntFrom(Architecture::ObjectLocation);
                        if(objectProto<2){
                            return Architecture::UnsetLocation;
                        }
                        else{
                            if(objectStartAddress!=objectProto){
                                return this->propertyValueAddressLookUp(objectProto,key,true);
                            }
                            else{
                                return Architecture::UnsetLocation;
                            }
                        }
                    }
                }
            }
        }*/
        return Architecture::UnsetLocation;
    }
    else{
        ErrorRegistry::logError("Error! Look Up Non-Object, for key:"+key+", type:"+QString::number(this->data->getByte(objectAddress))+" given, at #:"+QString::number(objectAddress)+" on line:"+QString::number(this->interpreter->lineNumber));
    }
    return Architecture::UnsetLocation;
}

bool Heap::objectHasOwn(int objectAddress, int stringAddress,QString key){
    int GUTTER;
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        GUTTER = Architecture::TypeObjectGutter;
    }
    else if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER = Architecture::TypeConstObjectGutter;
    }
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        int objectStartAddress = objectAddress;
        int size = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToSize);
        int index = this->expectedIndex(size,key);
        int keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
        if(keyAddress==stringAddress){
            if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                return true;
            }
            else{
                int privacy = this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize));
                if(privacy>=1){
                    int lexicalId = this->interpreter->getLexicalID();
                    //now check if the lexical id of the currently executing function
                    //is a child of the lexical id of the object in question.
                    int objectLexicalId = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToLexId);
                    if(lexicalId==objectLexicalId){
                        return true;
                    }
                    else if(lexicalId==0){
                        ErrorRegistry::logError("Could not accesss private member "+key+" on object, from within the context of line "+QString::number(this->interpreter->lineNumber));
                        return false;
                    }
                    else{
                        bool pathFound=false;
                        bool esc=false;
                        while(pathFound==false && esc==false){
                            int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;
                            unsigned char lexType = this->data->getByte(lexRuleAddress);
                            if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                                ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                                return false;
                            }
                            else{
                                while(lexType==ByteCodeInstruction::TypeLexRuleType){
                                    int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                                    int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                                    if(child==lexicalId){
                                        if(parent!=0){
                                            lexicalId = parent;
                                            if(lexicalId==objectLexicalId){
                                                pathFound=true;
                                            }
                                        }
                                        else{
                                            esc=true;
                                        }
                                    }
                                    lexRuleAddress+=Architecture::TypeLexRuleSize;
                                    lexType = this->data->getByte(lexRuleAddress);
                                }
                            }
                        }
                        if(pathFound==true){
                            return true;
                        }
                        else{
                            if(privacy!=1){
                                ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                            }
                            else{
                                return true;
                            }
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Could not accesss non-public member "+key+" on object("+this->objectToString(objectAddress,0)+"), from the context of line:"+QString::number(this->interpreter->lineNumber));
                }
                return false;
            }
        }
        else{
            index = this->shiftIndex(size,index,1);
            keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
            if(keyAddress==stringAddress){
                if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                    return true;
                }
                else{
                    ErrorRegistry::logError("Unwritten Code:: Look up on object, is in second place for non-private. address,key address,string");
                }
            }
            else{
                index = this->shiftIndex(size,index,2);
                int keyAddress = this->data->decodeRawIntFrom(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent);
                if(keyAddress==stringAddress){
                    if(this->data->getByte(objectStartAddress+GUTTER+(index*Architecture::TypeObjectPropertySize))==NULL){
                        return true;
                    }
                    else{
                        ErrorRegistry::logError("Unwritten Code:: Look up on object, is in third place for non-private.");
                    }
                }
                else{
                    int protoAddress = this->decodeRawIntFrom(objectStartAddress+Architecture::TypeObjectGutterToProto);
                    if(protoAddress>1){
                        return false;
                    }
                    else{
                        int objectProto = this->decodeRawIntFrom(Architecture::ObjectLocation);
                        if(objectProto<2){
                            return false;
                        }
                        else{
                            if(objectStartAddress!=objectProto){
                                return false;
                            }
                            else{
                                return false;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
    else{
        ErrorRegistry::logError("Error! Look Up Non-Object, with key:"+key+", at address:"+QString::number(objectAddress)+", on line:"+QString::number(this->interpreter->lineNumber));
    }
    return false;
}

bool Heap::objectHasOnChain(int objectAddress, int keyAddress,QString key,int startdepth,int enddepth){
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
        if(startdepth>=0 && enddepth>=0){
            int i=0;
            int protoAddress = objectAddress;
            while(i<startdepth && protoAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                protoAddress = this->decodeRawIntFrom(objectAddress+Architecture::TypeObjectGutterToProto);
                i++;
            }
            if(protoAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE && i==startdepth){
                while(i<startdepth+enddepth && protoAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    if(this->objectHasOwn(protoAddress,keyAddress,key)){
                        return true;
                    }
                    protoAddress=this->decodeRawIntFrom(protoAddress+Architecture::TypeObjectGutterToProto);
                    i++;
                }
                return false;
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

int Heap::RPNAndThisDeref(int A_Address){
    while(A_Address==Architecture::ThisAddress || (A_Address>=Architecture::RPNAddress && A_Address<Architecture::SIZE_OF_INITIAL_HEAP_STORE)){
        if(A_Address==Architecture::ThisAddress){
            A_Address = this->decodeRawIntFrom(Architecture::ThisAddress);
        }
        else{
            A_Address = this->decodeRawIntFrom(A_Address);
        }
    }
    return A_Address;
}

void Heap::copyObjectToNew(int objectAddress){
    if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(objectAddress)==ByteCodeInstruction::TypeConstObjectType){
        int isize = this->decodeRawIntFrom(objectAddress+Architecture::TypeObjectGutterToSize);
        isize = isize*Architecture::TypeObjectPropertySize*Architecture::StandardObjectSize;
        if(isize<1){
            ErrorRegistry::logError("Internal: CopyObjectToNew on object of size less than 1? At Address:"+QString::number(objectAddress));
        }
        else{
            //////this->debuggerOut("The size of object in bytes being copied:"+QString::number(isize),true);
            int start = objectAddress+1;
            int end;
            if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeConstObjectType){
                end = isize+Architecture::TypeConstObjectGutter+objectAddress;
            }
            else{
                end = isize+Architecture::TypeObjectGutter+objectAddress;
            }
            PByteArray* res=new PByteArray();
            res->resize(1);
            res->setByte(0,ByteCodeInstruction::TypeObjectType);
            while(start<end && ErrorRegistry::errorCount==0){
                res->resize(res->size()+1);
                res->setByte(res->size()-1,this->data->getByte(start));
                if(start==objectAddress+Architecture::TypeConstObjectGutter){
                    PByteArray* numb = new PByteArray();
                    numb->setNum(0);
                    int i=0;
                    while(i<numb->size()){
                        res->resize(res->size()+1);
                        res->setByte(res->size()-1,numb->getByte(i));
                        if(this->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
                            start++;
                        }
                        i++;
                    }
                }
                start++;
            }
            start=0;
            int newObjectAddress = this->data->size();
            while(start<res->size()){
                this->accept(res->getByte(start));
                start++;
            }
            start = newObjectAddress+Architecture::TypeObjectGutter;
            end = isize+start;
            ////this->debuggerOut("Copying Object To New, start:"+QString::number(start)+", end:"+QString::number(end),true);
            while(start<end){
                int _address = this->decodeRawIntFrom(start+Architecture::TypeObjectPropertyValueIndent);
                unsigned char type = this->data->getByte(_address);
                ////this->debuggerOut("Type:"+QString::number(type),true);
                if(type==ByteCodeInstruction::TypeConstFloatType){
/*                    int newAddress = this->data->size();
                    float f = this->decodeRawFloatFrom(_address+Architecture::TypeConstFloatGutter);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsRunTimeFloat(1,f);
                    this->data->copyToNew(pbytes);
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);*/
                }
                else if(type==ByteCodeInstruction::TypeConstIntegerType){
                    int newAddress = this->data->size();
                    int i = this->decodeRawFloatFrom(_address+Architecture::TypeConstIntegerGutter);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsRunTimeInteger(1,i);
                    this->copyToNew(pbytes,Architecture::TypeRunTimeIntegerSize);
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);
                }
                else if(type==ByteCodeInstruction::TypeConstStringType){
                    int newAddress = this->data->size();
                    QString str = this->decodeStringFrom(_address);
                    ////this->debuggerOut("Copying Const string:"+str+", during Heap::copyObjectToNew()",true);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsString(str,false);
                    this->copyToNew(pbytes,pbytes->size());
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);
                }
                else if(type==ByteCodeInstruction::TypeRunTimeFloatType){
/*                    int newAddress = this->data->size();
                    float f = this->decodeRawFloatFrom(_address+Architecture::TypeConstFloatGutter);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsRunTimeFloat(1,f);
                    this->data->copyToNew(pbytes);
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);*/
                }
                else if(type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    int newAddress = this->data->size();
                    int i = this->decodeRawIntFrom(_address+Architecture::TypeRunTimeIntegerGutter);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsRunTimeInteger(1,i);
                    this->copyToNew(pbytes,Architecture::TypeRunTimeIntegerSize);
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);
                }
                else if(type==ByteCodeInstruction::TypeRunTimeStringType){
                    int newAddress = this->data->size();
                    QString str = this->decodeStringFrom(_address);

                    ////this->debuggerOut("Copying RunTime string:"+str+", during Heap::copyObjectToNew()",true);

                    PByteArray* pbytes = new PByteArray();
                    pbytes->encodeAsString(str,false);
                    this->copyToNew(pbytes,pbytes->size());
                    this->data->encodeRawIntegerAt(start+Architecture::TypeObjectPropertyValueIndent,newAddress);
                }
                start+=Architecture::TypeObjectPropertySize;
            }


        }
    }
    else{
        ErrorRegistry::logError("Internal: Attempted CopyObjectToNew on non-object.");
    }
}

void Heap::copyConstantObjectInitAndDefine(int objectAddress,int buffer,int prototypeReference,int classReference){
    if(ErrorRegistry::errorCount>0){
        ErrorRegistry::logError("Error Prior to Copying Constant Object, Prior to copying object to new...");
    }
    else{
        int A_Address = objectAddress;
        unsigned char A_Type = this->data->getByte(A_Address);
        ////////this->debuggerOut("Copying Constant Object!, at address:"+QString::number(A_Address),true);
        if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            if(A_Address<this->initThreshold){
                //////this->debuggerOut("Copying Constant Object"+this->objectToString(A_Address,0)+", at address:"+QString::number(A_Address),true);
                if(ErrorRegistry::errorCount==0){
                    ////////this->debuggerOut("Copying Constant Object!",true);
                    int newAAddress = this->data->size();
                    ////////this->debuggerOut("new object address:"+QString::number(newAAddress),true);
                    //////this->debuggerOut("Copying Object to New....",true);
                    this->copyObjectToNew(A_Address);
                    //////this->debuggerOut("Copied Object to New....",true);
                    //////this->debuggerOut("Type of Object Copied:"+QString::number(this->data->getByte(newAAddress)),true);
                    if(ErrorRegistry::errorCount==0){
                        if(prototypeReference>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            //////this->debuggerOut("Object which may be in need of proto repointing...",true);
                            //////this->debuggerOut("Object which may be in need of proto repointing..."+this->objectToString(newAAddress,0),true);
                            int protoReference = this->decodeRawIntFrom(newAAddress+Architecture::TypeObjectGutterToProto);
                            if(protoReference>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                if(this->data->getByte(protoReference)==ByteCodeInstruction::TypeConstObjectType){
                                    //////this->debuggerOut("Proto repointing..."+QString::number(protoReference)+", to "+QString::number(prototypeReference)+", holding type:"+QString::number(this->data->getByte(prototypeReference))+", toString:"+this->objectToString(prototypeReference,0),true);
                                    PByteArray* temp = new PByteArray();
                                    temp->setNum(prototypeReference);
                                    this->copyInTo(newAAddress+Architecture::TypeObjectGutterToProto,temp,Architecture::RawIntegerSize);
                                }
                            }
                        }
                        else{
                            //////this->debuggerOut("Object is not in need of proto repointing...",true);
                        }
                        //check for class
                        if(ErrorRegistry::errorCount==0){
                            if(classReference>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                //////this->debuggerOut("Object which may have a .class in need of repointing..."+this->objectToString(newAAddress,0),true);
                                int classPropertyAddress = this->propertyValueAddressLookUp(newAAddress,"class",false);
                                if(classPropertyAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                    int classValueAddress = this->decodeRawIntFrom(classPropertyAddress);
                                    if(classValueAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                        if(this->data->getByte(classValueAddress)==ByteCodeInstruction::TypeConstObjectType){
                                            PByteArray* temp = new PByteArray();
                                            temp->setNum(classReference);
                                            this->copyInTo(classPropertyAddress,temp,Architecture::RawIntegerSize);
                                        }
                                    }
                                }
                            }
                            else{
                                //////this->debuggerOut("Object does not have a .class in need of repointing...",true);
                            }
                            //check for prototype and then blueprint
                            if(ErrorRegistry::errorCount==0){
                                //////this->debuggerOut("prototype look up on object:"+this->objectToString(newAAddress,0)+", at address:"+QString::number(newAAddress),true);
                                int prototypeStartAddress = this->propertyValueAddressLookUp(newAAddress,"prototype",false);
                                //////this->debuggerOut("prototype start address:"+QString::number(prototypeStartAddress)+" on object:"+this->objectToString(newAAddress,0)+", at address:"+QString::number(newAAddress),true);
                                if(prototypeStartAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                    int prototypeAddress = prototypeStartAddress;
                                    prototypeAddress=this->decodeRawIntFrom(prototypeAddress);
                                    if(prototypeAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                        if(this->data->getByte(prototypeAddress)==ByteCodeInstruction::TypeConstObjectType){
                                            int newPrototypeAddress = this->data->size();
                                            //////this->debuggerOut("Copying prototype...",true);
                                            this->copyConstantObjectInitAndDefine(prototypeAddress,buffer,Architecture::NULLLocation,newAAddress);
                                            PByteArray* temp = new PByteArray();
                                            temp->setNum(newPrototypeAddress);
                                            this->copyInTo(prototypeStartAddress,temp,Architecture::RawIntegerSize);
                                            this->incrementReferenceCount(newPrototypeAddress);
                                            if(ErrorRegistry::errorCount==0){
                                                //////this->debuggerOut("Checking for blueprint on:"+this->objectToString(newAAddress,0),true);

                                                //blueprint....
                                                int blueprintStartAddress = this->propertyValueAddressLookUp(newAAddress,"blueprint",false);
                                                if(blueprintStartAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                                    int blueprintAddress = blueprintStartAddress;
                                                    if(blueprintAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                                        blueprintAddress=this->decodeRawIntFrom(blueprintAddress);

                                                        if(this->data->getByte(blueprintAddress)==ByteCodeInstruction::TypeConstObjectType){
                                                            int newBlueprintAddress = this->data->size();
                                                            ////////this->debuggerOut("Copying blueprint at address:"+QString::number(blueprintAddress),true);
                                                            this->copyConstantObjectInitAndDefine(blueprintAddress,buffer,this->decodeRawIntFrom(prototypeStartAddress),Architecture::NULLLocation);
                                                            if(ErrorRegistry::errorCount>0){
                                                                ////////this->debuggerOut("Finished Copying blueprint, with errors...",true);
                                                            }
                                                            else{
                                                                ////////this->debuggerOut("Finished Copying blueprint, with NO errors...",true);
                                                            }
                                                            PByteArray* temp = new PByteArray();
                                                            temp->setNum(newBlueprintAddress);
                                                            this->copyInTo(blueprintStartAddress,temp,Architecture::RawIntegerSize);
                                                            this->incrementReferenceCount(newBlueprintAddress);
                                                        }
                                                    }
                                                }
                                                else{
                                                    //////this->debuggerOut("Object does not have a .blueprint in need of initialization...",true);
                                                }
                                            }












                                        }
                                        else{
                                            //////this->debuggerOut("Prototype is not const-object...",true);
                                        }
                                    }
                                    else{
                                        //////this->debuggerOut("Prototype is not runtime type...",true);
                                    }
                                }
                                else{
                                    //////this->debuggerOut("Object does not have a .prototype in need of initialization...",true);
                                }
                            }
                        }
                        //inits...
                        if(ErrorRegistry::errorCount==0){
                            int initAddress = this->decodeRawIntFrom(newAAddress+Architecture::TypeObjectGutterToInit);
                            if(initAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE && initAddress<this->data->size()){
                                //////this->debuggerOut("Object does have an init method...",true);
                                if(this->data->getByte(initAddress)==ByteCodeInstruction::TypeLambdaType){
                                    if(ErrorRegistry::errorCount>0){
                                        //////this->debuggerOut("Error prior to init method on object 2:"+this->objectToString(newAAddress,0),true);
                                    }
                                    PByteArray* rpnStore = new PByteArray();
                                    rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                                    int i=0;
                                    while(i<rpnStore->size()){
                                        rpnStore->setByte(i,this->data->getByte(Architecture::RPNAddress+i));
                                        i++;
                                    }
                                    int _thisStore = this->decodeRawIntFrom(Architecture::ThisAddress);
                                    if(ErrorRegistry::errorCount>0){
                                        //////this->debuggerOut("Error prior to init method on object:"+this->objectToString(newAAddress,0),true);
                                    }
                                    this->interpreter->executeLambda(initAddress,Architecture::UnsetLocation,newAAddress);//fix this later
                                    i=0;
                                    while(i<rpnStore->size()){
                                        this->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                                        i++;
                                    }
                                    this->data->encodeRawIntegerAt(Architecture::ThisAddress,_thisStore);
                                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                        this->data->encodeRawIntegerAt(buffer,this->decodeRawIntFrom(Architecture::ReturnAddress));
                                    }
                                    else{
                                        ErrorRegistry::logError("Unwritten code: Assigning ket to constant object and putting in return buffer.");
                                    }
                                }
                            }
                            else{
                                //////this->debuggerOut("Object does not have a init method...",true);
                            }
                            if(prototypeReference==Architecture::NULLLocation && classReference==Architecture::NULLLocation){
                                int lambdaAddress = this->propertyValueAddressLookUp(newAAddress,"__define",false);
                                ////////this->debuggerOut("Address of __define method:"+QString::number(lambdaAddress),true);
                                if(lambdaAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                    lambdaAddress=this->decodeRawIntFrom(lambdaAddress);
                                    if(this->data->getByte(lambdaAddress)==ByteCodeInstruction::TypeLambdaType){
                                        ////////this->debuggerOut("__define function found!",true);
                                        PByteArray* rpnStore = new PByteArray();
                                        rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                                        int i=0;
                                        while(i<rpnStore->size()){
                                            rpnStore->setByte(i,this->data->getByte(Architecture::RPNAddress+i));
                                            i++;
                                        }
                                        int thisStore = this->data->decodeRawIntFrom(Architecture::ThisAddress);
                                        this->interpreter->executeLambda(lambdaAddress,Architecture::UnsetLocation,newAAddress);//fix this later
                                        i=0;
                                        while(i<rpnStore->size()){
                                            this->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                                            i++;
                                        }
                                        this->data->encodeRawIntegerAt(Architecture::ThisAddress,thisStore);
                                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                                            this->data->encodeRawIntegerAt(buffer,this->decodeRawIntFrom(Architecture::ReturnAddress));
                                        }
                                        else{
                                            ErrorRegistry::logError("Unwritten code: Assigning ket to constant object and putting in return buffer.");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else{
                ErrorRegistry::logError("Malformed ByteCode - Const-Object outside init threshold.");
            }
        }
        else{
            ErrorRegistry::logError("Copy Constant Object on non Const-Object");
        }
        if(ErrorRegistry::errorCount>0){
            ErrorRegistry::logError("Error During Copy Constant Object, Init and Define...");
        }
    }
}

void Heap::setInitThreshold(){
    this->initThreshold=this->data->size();
}

void Heap::setRPNBuffer(int buffer, int _address){
    if(buffer>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
        ErrorRegistry::logError("Internal: Attempted invalid buffer address");
    }else{
        int previous = this->decodeRawIntFrom(buffer);
        this->data->encodeRawIntegerAt(buffer,_address);
        if(previous!=_address){
            this->incrementReferenceCount(_address);
            this->decrementReferenceCount(previous);
        }
    }
}

void Heap::incrementReferenceCount(int address){
    unsigned char type = this->data->getByte(address);
    if(type==ByteCodeInstruction::TypeRunTimeFloatType){
        int GUTTER =Architecture::TypeRunTimeFloatGutterToReferenceCount;
        this->data->encodeRawIntegerAt(address+GUTTER,this->data->decodeRawIntFrom(address+GUTTER)+1);
    }
    else if(type==ByteCodeInstruction::TypeRunTimeStringType){
        int GUTTER =Architecture::TypeStringGutterToReferenceCount;
        this->data->encodeRawIntegerAt(address+GUTTER,this->data->decodeRawIntFrom(address+GUTTER)+1);
    }
    else if(type==ByteCodeInstruction::TypeObjectType){
        int GUTTER =Architecture::TypeObjectGutterToReferenceCount;
        this->data->encodeRawIntegerAt(address+GUTTER,this->data->decodeRawIntFrom(address+GUTTER)+1);
    }
    else if(type==ByteCodeInstruction::TypeRunTimeIntegerType){
        int GUTTER =Architecture::TypeRunTimeIntegerGutterToReferenceCount;
        this->data->encodeRawIntegerAt(address+GUTTER,this->data->decodeRawIntFrom(address+GUTTER)+1);
    }
    else if(type==ByteCodeInstruction::TypeOpenArrayType || type==ByteCodeInstruction::TypeClosedArrayType){
        int GUTTER =Architecture::TypeArrayGutterToReferenceCount;
        this->data->encodeRawIntegerAt(address+GUTTER,this->data->decodeRawIntFrom(address+GUTTER)+1);
    }
}

void Heap::decrementReferenceCount(int address){
    unsigned char type = this->data->getByte(address);
    if(type==ByteCodeInstruction::TypeRunTimeFloatType){
        int GUTTER =Architecture::TypeRunTimeFloatGutterToReferenceCount;
        int RC = this->data->decodeRawIntFrom(address+GUTTER)-1;
        int SIZE = Architecture::TypeRunTimeFloatSize;
        if(RC>0){
            this->data->encodeRawIntegerAt(address+GUTTER,RC);
        }
        else{
            int i = address+SIZE-1;
            while(i>=address){
                this->clear(i);
                i--;
            }
        }
    }
    else if(type==ByteCodeInstruction::TypeRunTimeStringType){
        int GUTTER =Architecture::TypeStringGutterToReferenceCount;
        int RC = this->data->decodeRawIntFrom(address+GUTTER)-1;
        int SIZE = this->decodeRawIntFrom(address+Architecture::TypeStringGutterToSize)*Architecture::StandardStringSize;
        if(RC>0){
            this->data->encodeRawIntegerAt(address+GUTTER,RC);
        }
        else{
            int i = address+SIZE-1;
            while(i>=address){
                this->clear(i);
                i--;
            }
        }
    }
    else if(type==ByteCodeInstruction::TypeObjectType){
        int GUTTER =Architecture::TypeObjectGutterToReferenceCount;
        int RC = this->data->decodeRawIntFrom(address+GUTTER)-1;
        int SIZE = this->decodeRawIntFrom(address+Architecture::TypeObjectGutterToSize)*Architecture::StandardObjectSize;
        if(RC>0){
            this->data->encodeRawIntegerAt(address+GUTTER,RC);
        }
        else{
            int i = address+SIZE-1;
            while(i>=address){
                this->clear(i);
                i--;
            }
        }
    }
    else if(type==ByteCodeInstruction::TypeRunTimeIntegerType){
        int GUTTER =Architecture::TypeRunTimeIntegerGutterToReferenceCount;
        int RC = this->data->decodeRawIntFrom(address+GUTTER)-1;
        int SIZE = Architecture::TypeRunTimeIntegerSize;
        if(RC>0){
            this->data->encodeRawIntegerAt(address+GUTTER,RC);
        }
        else{
            int i = address+SIZE-1;
            while(i>=address){
                this->clear(i);
                i--;
            }
        }
    }
    else if(type==ByteCodeInstruction::TypeOpenArrayType || type==ByteCodeInstruction::TypeClosedArrayType){
        int GUTTER =Architecture::TypeArrayGutterToReferenceCount;
        int RC = this->data->decodeRawIntFrom(address+GUTTER)-1;
        int SIZE = this->decodeRawIntFrom(address+Architecture::TypeArrayGutterToSize);
        if(RC>0){
            this->data->encodeRawIntegerAt(address+GUTTER,RC);
        }
        else{

            //////this->interpreter->debuggerOut("Clearing Array....",true);

            int i = address+SIZE-1;
            while(i>=address){
                this->clear(i);
                i--;
            }
        }
    }
    //auto-delete-types array index and propertykey
    else if(type==ByteCodeInstruction::TypePropertyKeyType){
        int SIZE = Architecture::TypePropertyKeySize;
        int i = address+SIZE-1;
        while(i>=address){
            this->clear(i);
            i--;
        }
    }
    else if(type==ByteCodeInstruction::TypeArrayIndexType){
        int SIZE = Architecture::TypePropertyKeySize;
        int i = address+SIZE-1;
        while(i>=address){
            this->clear(i);
            i--;
        }
    }
}

QString Heap::objectToString(int oAddress,int indent){
    return this->objectToString(oAddress,indent,{});
}

QString Heap::lexRuleToString(int address){
    QString res="";
    int parent = this->decodeRawIntFrom(address+Architecture::TypeLexRuleGutterToParent);
    int child = this->decodeRawIntFrom(address+Architecture::TypeLexRuleGutterToChild);
    res+="LexRule<br>Parent:<br>"+QString::number(parent)+"<br>Child:"+QString::number(child);
    return res;
}

QString Heap::initThresholdString(){
    int i=Architecture::SIZE_OF_INITIAL_HEAP_STORE;
    QString res="";
    while(i<this->initThreshold){
        if(this->data->getByte(i)==ByteCodeInstruction::TypeLexRuleType){
            res+=this->lexRuleToString(i)+"<br>";
            i+=Architecture::TypeLexRuleSize;
        }
        else if(this->data->getByte(i)==ByteCodeInstruction::TypeObjectType){
            i++;
        }
        else if(this->data->getByte(i)==ByteCodeInstruction::TypeKeyType){
//            res+="key:"+this->decodeStringFrom(i)+"<br>";
            QString str = this->decodeStringFrom(i);
            res+="("+QString::number(i)+")key:"+str+"<br>";
            int index = i+Architecture::TypeStringGutterToSize;
            //int size = this->decodeRawIntFrom(index)*Architecture::StandardStringSize;
            int size = Architecture::TypeConstStringGutter+str.size();
            i+=size;
        }
        else if(this->data->getByte(i)==ByteCodeInstruction::TypeConstStringType){
            QString str = this->decodeStringFrom(i);
            res+="("+QString::number(i)+")Str:"+str+"<br>";
            int size = str.length()+Architecture::TypeRunTimeStringGutter;
            i+=size;
        }
        else if(this->data->getByte(i)==ByteCodeInstruction::TypeConstIntegerType){
            int str = this->decodeRawIntFrom(i+1);
            res+="("+QString::number(i)+")Int:"+QString::number(str)+"<br>";
            int size = Architecture::TypeConstIntegerSize;
            i+=size;
        }
        else if(this->data->getByte(i)==ByteCodeInstruction::TypeLambdaType){
            int str = this->decodeRawIntFrom(i+Architecture::TypeLambdaGutterToLexId);
            int str2 = this->decodeRawIntFrom(i+Architecture::TypeLambdaGutterToInstructionSet);
            res+="("+QString::number(i)+")Lamb: lexical-id:"+QString::number(str);
            res+=",inst-pointer:"+QString::number(str2)+"<br>";
            int size = Architecture::TypeConstIntegerSize;
            i+=size;
        }
        else{
            res+="[unknown]<br>";
            i++;
        }
    }

    qDebug()<<"Init threshold string:"<<res;

    return res;
}

QString Heap::objectToString(int oAddress,int indent,QVector<int> passedAddresses){
    int GUTTER;
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType){
        GUTTER = Architecture::TypeObjectGutter;
    }
    else if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        GUTTER = Architecture::TypeConstObjectGutter;
    }
    QString res = "Object:["+QString::number(oAddress)+"]\\n";//"[OBJECT][<br>";
    if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
        res = "Const-Object:["+QString::number(oAddress)+"]\\n";//"[OBJECT][<br>";
    }
    int i=0;
    bool circular=false;
    while(i<passedAddresses.size()){
        if(oAddress==passedAddresses[i]){
            circular=true;
        }
        i++;
    }
    if(circular){
        return "[circular]";
    }
    else{
        passedAddresses.append(oAddress);
        if(this->data->getByte(oAddress)==ByteCodeInstruction::TypeObjectType || this->data->getByte(oAddress)==ByteCodeInstruction::TypeConstObjectType){
            int osAddress = oAddress;
            int size= this->decodeRawIntFrom(oAddress+Architecture::TypeObjectGutterToSize);
            //lex,proto,init...
            int lexicalId = this->decodeRawIntFrom(osAddress+Architecture::TypeObjectGutterToLexId);
            int protoAddress = this->decodeRawIntFrom(osAddress+Architecture::TypeObjectGutterToProto);
            int initAddress = this->decodeRawIntFrom(osAddress+Architecture::TypeObjectGutterToInit);
            int o=0;
            while(o<indent+1){
                res+="\\t";
                if(o==indent){
                    res+="+";
                }
                o++;
            }
            res+=" Lexical Id:"+QString::number(lexicalId)+"<br>";
            o=0;
            while(o<indent+1){
                res+="\\t";
                if(o==indent){
                    res+="+";
                }
                o++;
            }
            res+=" Proto Addr:"+QString::number(protoAddress)+"<br>";
            o=0;
            while(o<indent+1){
                res+="\\t";
                if(o==indent){
                    res+="+";
                }
                o++;
            }
            res+=" Init Addr:"+QString::number(initAddress)+"<br>";
            oAddress+=(GUTTER);
            size = size*Architecture::StandardObjectSize;
            o=0;
            while(o<indent+1){
                res+="\\t";
                if(o==indent){
                    res+="+";
                }
                o++;
            }
            res+="      Size:"+QString::number(size)+"<br>";
           if(GUTTER==Architecture::TypeObjectGutter){
                int refCount = this->decodeRawIntFrom(osAddress+Architecture::TypeObjectGutterToReferenceCount);
                o=0;
                while(o<indent+1){
                    res+="\\t";
                    if(o==indent){
                        res+="+";
                    }
                    o++;
                }
                res+="     Ref.C.:"+QString::number(refCount)+"<br>";
            }
            int i=0;
            while(i<size){
                if(this->data->getByte(oAddress)==NULL){
                    int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                    if(_address==Architecture::NULLLocation){
                    //    res+="NULL";
                    }
                    else if(_address==Architecture::UnsetLocation){
                    //    res+="unset";
                    }
                    else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                        int o=0;
                        while(o<indent+1){
                            res+="\\t";
                            if(o==indent){
                                res+="+";
                            }
                            o++;
                        }
                        res+="["+QString::number(i)+"]";
                        QString propKey = decodeStringFrom(_address);
                        res+=" "+propKey+":";
                        _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyValueIndent);
                        res+="[#"+QString::number(oAddress+Architecture::TypeObjectPropertyValueIndent)+"]";
                        if(_address==Architecture::NULLLocation){
                            res+="[NULL]";
                        }
                        else if(_address==Architecture::UnsetLocation){
                            res+="[unset]";
                        }
                        else if(_address==Architecture::TrueLocation){
                            res+="[true]";
                        }
                        else if(_address==Architecture::FalseLocation){
                            res+="[false]";
                        }
                        else{
                            if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeStringType){
                                res+=this->decodeStringFrom(_address)+"{"+QString::number(this->decodeRawIntFrom(_address+Architecture::TypeStringGutterToReferenceCount))+"}";
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeIntegerType){
                                res+=QString::number(this->decodeRawIntFrom(_address+Architecture::TypeRunTimeIntegerGutter));
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeObjectType || this->data->getByte(_address)==ByteCodeInstruction::TypeConstObjectType){
                                QString objString = this->objectToString(_address,indent+1,passedAddresses);
                                res+=objString;
                                if(QString::compare(objString,"[circular]")!=0){
                                    res.resize(res.size()-2);
                                }
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeLambdaType){
                                res+="[lambda]";
                            }
                            else{
                                res+="[unknown:"+QString::number(this->data->getByte(_address))+" at #"+QString::number(_address)+"]";
                            }
                        }
                        res+="\\n";
                    }
                    else{
                        res+="[unknown]<br>";
                    }
                }
                else if(this->data->getByte(oAddress)<10){
                    int _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyKeyIndent);
                    if(_address==Architecture::NULLLocation){
                    //    res+="NULL";
                    }
                    else if(_address==Architecture::UnsetLocation){
                    //    res+="unset";
                    }
                    else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                        int o=0;
                        while(o<indent+1){
                            res+="\\t";
                            if(o==indent){
                                res+="#";
                            }
                            o++;
                        }
                        res+="["+QString::number(i)+"]";
                        QString propKey = decodeStringFrom(_address);
                        res+=" "+propKey+":";
                        _address = decodeRawIntFrom(oAddress+Architecture::TypeObjectPropertyValueIndent);
                        res+="[#"+QString::number(oAddress+Architecture::TypeObjectPropertyValueIndent)+"]";
                        if(_address==Architecture::NULLLocation){
                            res+="[NULL]";
                        }
                        else if(_address==Architecture::UnsetLocation){
                            res+="[unset]";
                        }
                        else if(_address==Architecture::TrueLocation){
                            res+="[true]";
                        }
                        else if(_address==Architecture::FalseLocation){
                            res+="[false]";
                        }
                        else{
                            if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeStringType){
                                res+=this->decodeStringFrom(_address);
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeIntegerType){
                                res+=QString::number(this->decodeRawIntFrom(_address+Architecture::TypeRunTimeIntegerGutter));
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeObjectType || this->data->getByte(_address)==ByteCodeInstruction::TypeConstObjectType){
                                QString objString = this->objectToString(_address,indent+1,passedAddresses);
                                res+=objString;
                                if(QString::compare(objString,"[circular]")!=0){
                                    res.resize(res.size()-2);
                                }
                            }
                            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeLambdaType){
                                res+="[lambda]";
                            }
                            else{
                                res+="[unknown:"+QString::number(this->data->getByte(_address))+" at #"+QString::number(_address)+"]";
                            }
                        }
                        res+="\\n";
                    }
                    else{
                        res+="[unknown]<br>";
                    }
                }
                else{
                    res+="\\t-["+QString::number(i)+"][PRIVATE]<br>";
                }
                i++;
                oAddress+=Architecture::TypeObjectPropertySize;
            }
        }
        else{
            res = "[NON-OBJECT]<br>";
        }
        return res;
    }
}

void Heap::appendToString(int stringAddress, QString concat){
    if(this->data->getByte(stringAddress)==ByteCodeInstruction::TypeRunTimeStringType){
        QString str = this->decodeStringFrom(stringAddress);
        str+=concat;
        int size = this->data->decodeRawIntFrom(stringAddress+Architecture::TypeStringGutterToSize)*Architecture::StandardStringSize;
        int index = stringAddress+Architecture::TypeRunTimeStringGutter;
        int lastAddress = index+(size)-1;
        int newStringLength = str.length()+stringAddress+Architecture::TypeRunTimeStringGutter;
        if(lastAddress == (this->data->size()-1)){
            PByteArray* _newStr = new PByteArray();
            _newStr->encodeAsString(str,false);
            this->copyInTo(stringAddress,_newStr,_newStr->size());
        }
        else if(lastAddress <= newStringLength){
            PByteArray* _newStr = new PByteArray();
            _newStr->encodeAsString(str,false);
            int newStringAddress = this->data->size();
            PByteArray* heapFragment = new PByteArray();
            heapFragment->setNum(newStringAddress);
            heapFragment->prepend(ByteCodeInstruction::TypeHeapFragment);
            this->copyInTo(stringAddress,heapFragment,Architecture::TypeHeapFragmentSize);
            while(lastAddress>stringAddress+Architecture::TypeRunTimeStringGutter){
                this->clear(lastAddress);
                lastAddress--;
            }
            this->copyToNew(_newStr,_newStr->size());
        }
        else{
            if((lastAddress-index)>concat.size()){
                int o=0;
                while(index<lastAddress && o<str.size()){
                    this->data->setByte(index,str[o].toLatin1());
                    o++;
                    index++;
                }
            }
            else{
                ErrorRegistry::logError("Error! - Runnout of space for string.");
            }
        }
    }
    else{
        ErrorRegistry::logError("Error! - Attempted concatenation onto non-string type.");
    }
}

void Heap::addStackFrame(int parentAddress, bool fixed, int lexicalID){
    if(parentAddress==Architecture::NULLLocation || parentAddress==Architecture::NULLLocation ||
            (parentAddress>=this->initThreshold &&
             (this->data->getByte(parentAddress)==Architecture::TypeLettedScope || this->data->getByte(parentAddress)==Architecture::TypeFixedScope))){
        int i=0;
        int l = (Architecture::StandardStackFrameSize*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter;
        int startIndex = this->data->size();
        while(i<l){
            Heap::accept(NULL);//unset
            i++;
        }
        PByteArray* _parentAddress = new PByteArray();
        _parentAddress->setNum(parentAddress);
        if(fixed==true){
            this->data->setByte(startIndex,Architecture::TypeFixedScope);
        }
        else{
            this->data->setByte(startIndex,Architecture::TypeLettedScope);
        }
        i=1;
        while(i<_parentAddress->size()){
            this->data->setByte(startIndex+i,_parentAddress->getByte(i-1));
            i++;
        }
        this->data->encodeRawIntegerAt(startIndex+Architecture::TypeStackFrameGutterToLexId,lexicalID);
        this->data->encodeRawIntegerAt(startIndex+Architecture::TypeStackFrameGutterToSize,1);
        this->data->encodeRawIntegerAt(Architecture::CurrentStackFrameAddress,startIndex);
        int unsetLocation = Architecture::UnsetLocation;//supposed to be NULL
        PByteArray* unsetAddress = new PByteArray();
        unsetAddress->setNum(unsetLocation);
        i=startIndex+Architecture::TypeStackFrameGutter;
        while(i<((Architecture::StandardStackFrameSize*Architecture::TypeStackFrameEntrySize)+startIndex+Architecture::TypeStackFrameGutter)){//?
            this->data->encodeRawIntegerAt(i,Architecture::UnsetLocation);
            i+=Architecture::TypeStackFrameEntryGutter;
            this->data->encodeRawIntegerAt(i,Architecture::UnsetLocation);
            i+=Architecture::TypeStackFrameEntryGutter;
        }
    }
    else{
        ErrorRegistry::logError("Critical Error: Attempted creation of stackframe with invalid parent address:"+QString::number(parentAddress)+", on line:"+QString::number(this->interpreter->lineNumber));
    }
}

void Heap::enlargeCurrentStackFrame(){
    int currentStackFrame = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    if(this->data->getByte(currentStackFrame)==Architecture::TypeLettedScope || this->data->getByte(currentStackFrame)==Architecture::TypeFixedScope){
        int currentSize = this->decodeRawIntFrom(currentStackFrame+Architecture::TypeStackFrameGutterToSize);
        int lastIndex = currentStackFrame+Architecture::TypeStackFrameGutter+(currentSize*Architecture::StandardStackFrameSize*Architecture::TypeStackFrameEntrySize)-1;
        if(lastIndex==this->data->size()-1){
            int newSize = currentSize*2;
            PByteArray* _newSize = new PByteArray();
            _newSize->setNum(newSize);
            this->copyInTo(currentStackFrame+Architecture::TypeStackFrameGutterToSize,_newSize,Architecture::RawIntegerSize);
            int l = currentSize*Architecture::StandardStackFrameSize*Architecture::TypeStackFrameEntrySize;
            int i=0;
            while(i<l){
                this->data->append(NULL);
                i++;
            }
        }
        else{
            PByteArray* stackFrameCopy = new PByteArray();
            int i = currentStackFrame;
            while(i<=lastIndex){
                stackFrameCopy->resize(stackFrameCopy->size()+1);
                stackFrameCopy->setByte(stackFrameCopy->size()-1,this->data->getByte(i));
                i++;
            }
            int o=0;
            int l = (Architecture::StandardStackFrameSize*Architecture::TypeStackFrameEntrySize*currentSize);
            while(o<l){
                stackFrameCopy->append(NULL);
                o++;
            }
            stackFrameCopy->encodeRawIntegerAt(Architecture::TypeStackFrameGutterToSize,currentSize*2);
            int newAddress = this->data->size();
            this->copyToNew(stackFrameCopy,stackFrameCopy->size());
            this->data->encodeRawIntegerAt(Architecture::CurrentStackFrameAddress,newAddress);
        }
        int startAddress = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress)+Architecture::TypeStackFrameGutter;
        int i=startAddress;
        PByteArray* keys = new PByteArray();
        PByteArray* vals = new PByteArray();
        while(i<lastIndex){
            int x = this->decodeRawIntFrom(i);
            if(x!=Architecture::NULLLocation && x!=Architecture::UnsetLocation){
                keys->resize(keys->size()+Architecture::RawIntegerSize);
                keys->encodeRawIntegerAt(keys->size()-Architecture::RawIntegerSize,x);
                vals->resize(vals->size()+Architecture::RawIntegerSize);
                vals->encodeRawIntegerAt(vals->size()-Architecture::RawIntegerSize,this->decodeRawIntFrom(i+Architecture::TypeStackFrameEntryGutter));
            }
            i+=(Architecture::TypeStackFrameEntrySize);
        }
        i=startAddress;
        while(i<=lastIndex){
            this->data->setByte(i,NULL);
            i++;
        }
        i=0;
        while(i<keys->size()){
            int key = keys->decodeRawIntFrom(i);
            int val = vals->decodeRawIntFrom(i);
            QString keyString = this->decodeStringFrom(key);
            this->addToCurrentStackFrame(keyString,key);
            int valAddress = this->valueAddressInStackFrames(keyString,key);
            PByteArray* valAddr = new PByteArray();
            valAddr->setNum(val);
            this->copyInTo(valAddress,valAddr,Architecture::RawIntegerSize);
            i+=Architecture::RawIntegerSize;
        }
    }
    else{
        ErrorRegistry::logError("Internal:Attempted Enlarging of non-stack frame!, at address:#"+QString::number(currentStackFrame));
    }
}

int QUARTER=4;

void Heap::addToCurrentStackFrame(QString key,int stringAddress){
    int stackFrameAddress = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    int StackFrameSize = this->decodeRawIntFrom(stackFrameAddress+Architecture::TypeStackFrameGutterToSize);
    long hashResult = Hasher::SuperFastHash(key);
    //this->debuggerOut(QString::number(hashResult));
    int index = hashResult % (Architecture::StandardStackFrameSize*StackFrameSize);
    //this->debuggerOut(QString::number(index));
    int baseindex = index;
    index = (index*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+stackFrameAddress;
    int preKey = this->decodeRawIntFrom(index);
    if(preKey==Architecture::NULLLocation || preKey==Architecture::UnsetLocation){
        this->data->encodeRawIntegerAt(index,stringAddress);
    }
    else{
        if(preKey==stringAddress){
            ErrorRegistry::logError("Cannot redeclare variable : "+key+", on line:"+QString::number(this->interpreter->lineNumber));
        }
        else{
            int quarter = (StackFrameSize*Architecture::StandardStackFrameSize)/QUARTER;
            baseindex = baseindex-quarter;
            if(baseindex<0){
                baseindex = (StackFrameSize*Architecture::StandardStackFrameSize) + baseindex;
            }
            index = (baseindex*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+stackFrameAddress;
            preKey = this->decodeRawIntFrom(index);
            if(preKey==Architecture::NULLLocation || preKey==Architecture::UnsetLocation){
                this->data->encodeRawIntegerAt(index,stringAddress);
            }
            else if(preKey==stringAddress){
                ErrorRegistry::logError("Cannot redeclare variable : "+key);
            }
            else{
                baseindex = baseindex-quarter;
                if(baseindex<0){
                    baseindex = baseindex + (StackFrameSize*Architecture::StandardStackFrameSize);
                }
                index = (baseindex*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+stackFrameAddress;
                preKey = this->decodeRawIntFrom(index);
                if(preKey==Architecture::NULLLocation || preKey==Architecture::UnsetLocation){
                    this->data->encodeRawIntegerAt(index,stringAddress);
                }
                else if(preKey==stringAddress){
                    ErrorRegistry::logError("Cannot redeclare variable : "+key);
                }
                else{
                    this->enlargeCurrentStackFrame();
                    if(ErrorRegistry::errorCount==0){
                        this->addToCurrentStackFrame(key,stringAddress);
                    }
                    else{
                        ErrorRegistry::logError("Error During Stack Frame Resizing!");
                    }
                }
            }
        }
    }
}

QString Heap::stackFrameToString(int oAddress){
    QString res = "<br>[STACKFRAME at #:"+QString::number(oAddress)+"][<br>";
    if(this->data->getByte(oAddress)==Architecture::TypeLettedScope || this->data->getByte(oAddress)==Architecture::TypeFixedScope){
        if(this->data->getByte(oAddress)==Architecture::TypeLettedScope){
            res += "\t[LETTED]<br>";
        }
        else if(this->data->getByte(oAddress)==Architecture::TypeFixedScope){
            res += "\t[FIXED]<br>";
        }
        int osAddress = oAddress;
        //lex,proto,init...
        int lexicalId = this->decodeRawIntFrom(osAddress+Architecture::TypeStackFrameGutterToLexId);
        int parentAddress = this->decodeRawIntFrom(osAddress+Architecture::TypeStackFrameGutterToParent);
        res+="Lexical Id:"+QString::number(lexicalId)+"<br>";
        res+="Parent Addr:"+QString::number(parentAddress)+"<br>";
        int size = this->decodeRawIntFrom(osAddress+Architecture::TypeStackFrameGutterToSize);
        oAddress+=(Architecture::TypeStackFrameGutter);
        size = size*Architecture::StandardStackFrameSize;
        res+="      Size:"+QString::number(size)+"<br>";
        int i=0;
        while(i<size){
            int _address = decodeRawIntFrom(oAddress);
            if(_address==Architecture::NULLLocation){
              //  res+="[NULL]<br>";
            }
            else if(_address==Architecture::UnsetLocation){
              //  res+="[UNSET]<br>";
            }
            else if(this->data->getByte(_address)==ByteCodeInstruction::TypeKeyType){
                QString propKey = decodeStringFrom(_address);
                res+="["+propKey+"]";
                _address = decodeRawIntFrom(oAddress+Architecture::TypeStackFrameEntryGutter);
                if(_address==Architecture::NULLLocation){
                    res+="[NULL]";
                }
                else if(_address==Architecture::UnsetLocation){
                    res+="[unset]";
                }
                else if(_address==Architecture::TrueLocation){
                    res+="[true]";
                }
                else if(_address==Architecture::FalseLocation){
                    res+="[false]";
                }
                else{
                    if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeStringType){
                        res+="[\""+this->decodeStringFrom(_address)+"\"]";
                    }
                    else if(this->data->getByte(_address)==ByteCodeInstruction::TypeRunTimeIntegerType){
                        res+="["+QString::number(this->decodeRawIntFrom(_address+Architecture::TypeRunTimeIntegerGutter))+"]";
                    }
                    else if(this->data->getByte(_address)==ByteCodeInstruction::TypeObjectType){
                        res+="[OBJECT]";
                    }
                    else if(this->data->getByte(_address)==ByteCodeInstruction::TypeLambdaType){
                        res+="[LAMBDA][#"+QString::number(_address)+"]";
                    }
                    else{
                        res+="[unknown:"+QString::number(this->data->getByte(_address))+" at #"+QString::number(_address)+"]";
                    }
                }
                res+="<br>";
            }
            else{
                res+="[unknown]<br>";
            }
            i++;
            oAddress+=Architecture::TypeStackFrameEntrySize;
        }

        res+="]";
    }
    else{
        res = "[NON-STACKFRAME]<br>";
    }
    return res;
}

int Heap::completeDeref(int address){
    //////this->interpreter->debuggerOut("Derefing address:"+QString::number(address),true);
    unsigned char type = this->data->getByte(address);
    while(address>Architecture::SIZE_OF_INITIAL_HEAP_STORE &&  type==ByteCodeInstruction::TypePropertyKeyType || type==ByteCodeInstruction::TypeHeapFragment || type==ByteCodeInstruction::TypeKeyType || type==ByteCodeInstruction::TypeArrayIndexType){
        if(type==ByteCodeInstruction::TypePropertyKeyType){
            //////this->interpreter->debuggerOut("Dereferencing property key to value slot from:"+QString::number(address),true);
            address = this->decodeRawIntFrom(address+Architecture::TypePropertyKeyGutter);
            //////this->interpreter->debuggerOut("Dereferenced property key to value slot to:"+QString::number(address),true);
            if(address<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                return address;
            }
            address = this->decodeRawIntFrom(address);
            type = this->data->getByte(address);
            //////this->interpreter->debuggerOut("Dereferenced property key to type:"+QString::number(type)+" at address:"+QString::number(address),true);
            if(address<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                return address;
            }
        }
        else if(type==ByteCodeInstruction::TypeArrayIndexType){
            //////this->interpreter->debuggerOut("Array Index deref...",true);
            address = this->decodeRawIntFrom(address+Architecture::TypeArrayIndexGutter);
            address = this->decodeRawIntFrom(address);
            type = this->data->getByte(address);
            if(address<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                return address;
            }
        }
        else if(type==ByteCodeInstruction::TypeHeapFragment){
            //////this->interpreter->debuggerOut("Heap Fragment deref...",true);
            address = this->decodeRawIntFrom(address+Architecture::TypeHeapFragmentGutter);
            type = this->data->getByte(address);
            if(address<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                return address;
            }
        }
        else if(type==ByteCodeInstruction::TypeKeyType){
            //////this->interpreter->debuggerOut("Type Key Type deref...",true);
            QString aString = this->decodeStringFrom(address);
            address = this->valueAddressInStackFrames(aString,address);
            //////this->interpreter->debuggerOut("Derefed key:"+aString+", to addess:"+QString::number(address),true);
            if(address>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                address = this->decodeRawIntFrom(address);
                type = this->data->getByte(address);
            }
            else{
                return address;
            }
        }
    }
    if(ErrorRegistry::errorCount>0){
        ErrorRegistry::logError("Error after Complete Dereference...");
    }
    return address;
}

int Heap::partialDeref(int address){
    unsigned char type = this->data->getByte(address);
    while(address>Architecture::SIZE_OF_INITIAL_HEAP_STORE && type==ByteCodeInstruction::TypeHeapFragment || type==ByteCodeInstruction::TypeKeyType){
        if(type==ByteCodeInstruction::TypeHeapFragment){
            address = this->decodeRawIntFrom(address+Architecture::TypeHeapFragmentGutter);
            type = this->data->getByte(address);
            if(address<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                return address;
            }
        }
        else if(type==ByteCodeInstruction::TypeKeyType){
           QString aString = this->decodeStringFrom(address);
            address = this->valueAddressInStackFrames(aString,address);
            if(address>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                address = this->decodeRawIntFrom(address);
                type = this->data->getByte(address);
            }
            else{
                return address;
            }
        }
    }
    return address;
}

int Heap::valueAddressInStackFrames(QString key,int stringAddress,int _topStackFrameAddress,int requestLexicalId){
    int storedRequestedLexicalId = requestLexicalId;
    int stackFrameAddress = _topStackFrameAddress;
    char type = this->data->getByte(_topStackFrameAddress);
    if(QString::compare(this->decodeStringFrom(stringAddress),key)==0){
        if(type==Architecture::TypeLettedScope || type==Architecture::TypeFixedScope){
            //////this->debuggerOut("Checking for:"+key+", on stack frame:"+this->stackFrameToString(_topStackFrameAddress));
            int lexicalId = this->data->getByte(_topStackFrameAddress+Architecture::TypeStackFrameGutterToLexId);
            bool canCheck=false;
            if(lexicalId==requestLexicalId){
                canCheck=true;
            }
            else if(lexicalId==0){
                canCheck=true;
            }
            else{
                bool pathFound=false;
                bool esc=false;
                    int lexRuleAddress = Architecture::SIZE_OF_INITIAL_HEAP_STORE;

                    unsigned char lexType = this->data->getByte(lexRuleAddress);

                    if(lexType!=ByteCodeInstruction::TypeLexRuleType){
                        ErrorRegistry::logError("Malformed ByteCode - lexrule not where epected.");
                        return Architecture::UnsetLocation;
                    }
                    else{
                        while(pathFound==false && esc==false && lexType==ByteCodeInstruction::TypeLexRuleType && lexRuleAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            int child = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToChild);
                            int parent = this->decodeRawIntFrom(lexRuleAddress+Architecture::TypeLexRuleGutterToParent);
                            if(child==requestLexicalId){
                                if(parent!=0){
                                    requestLexicalId = parent;
                                    if(lexicalId==requestLexicalId){
                                        pathFound=true;
                                    }
                                }
                                else{
                                    esc=true;
                                }
                            }
                            lexRuleAddress+=Architecture::TypeLexRuleSize;
                            lexType = this->data->getByte(lexRuleAddress);
                        }
                    }
                if(pathFound==true){
                    canCheck=true;
                }
                else{
                    //////this->debuggerOut("Could not find path from :"+QString::number(requestLexicalId)+", to "+QString::number(lexicalId),true);
                }
            }
            if(canCheck==false){
                int parentAddress = this->decodeRawIntFrom(_topStackFrameAddress+Architecture::TypeStackFrameGutterToParent);
                if(parentAddress>=this->initThreshold && (this->data->getByte(parentAddress)==Architecture::TypeLettedScope || this->data->getByte(parentAddress)==Architecture::TypeFixedScope)){
                    return this->valueAddressInStackFrames(key,stringAddress,parentAddress,storedRequestedLexicalId);
                }
                else{
                    return Architecture::UnsetLocation;
                }
            }
            else{
                int StackFrameSize = this->decodeRawIntFrom(_topStackFrameAddress+Architecture::TypeStackFrameGutterToSize);
                long hashResult = Hasher::SuperFastHash(key);
                int index = hashResult % (Architecture::StandardStackFrameSize*StackFrameSize);
                int baseindex = index;
                index = (index*StackFrameSize*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+_topStackFrameAddress;
                int addressToCheck = this->decodeRawIntFrom(index);
                if(addressToCheck==stringAddress){
                    return index+Architecture::TypeStackFrameEntryGutter;
                }
                else{
                    int quarter = (Architecture::StandardStackFrameSize*StackFrameSize)/QUARTER;
                    baseindex = baseindex-quarter;
                    if(baseindex<0){
                        baseindex = baseindex + (StackFrameSize*Architecture::StandardStackFrameSize);
                    }
                    index = (baseindex*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+stackFrameAddress;
                    int preKey = this->decodeRawIntFrom(index);
                    if(preKey==stringAddress){
                        return index+Architecture::TypeStackFrameEntryGutter;
                    }
                    else{
                        baseindex = baseindex-quarter;
                        if(baseindex<0){
                            baseindex = baseindex + (StackFrameSize*Architecture::StandardStackFrameSize);
                        }
                        index = (baseindex*Architecture::TypeStackFrameEntrySize)+Architecture::TypeStackFrameGutter+stackFrameAddress;
                        preKey = this->decodeRawIntFrom(index);
                        if(preKey==stringAddress){
                            return index+Architecture::TypeStackFrameEntryGutter;
                        }
                        else{
                            int parentAddress = this->decodeRawIntFrom(_topStackFrameAddress+Architecture::TypeStackFrameGutterToParent);
                            if(parentAddress!=Architecture::NULLLocation && parentAddress!=Architecture::UnsetLocation){
                                return this->valueAddressInStackFrames(key,stringAddress,parentAddress,storedRequestedLexicalId);
                            }
                            else{
                                return Architecture::UnsetLocation;
                            }
                        }
                    }
                }
            }
        }
        else{
            ErrorRegistry::logError("Internal Runtime Error - attempted look up on non-stack frame with key:'"+key+"', which does not match address given of: "+QString::number(stringAddress));
        }
    }
    else{
        ErrorRegistry::logError("Internal Runtime Error - attempted look up on non-stack frame, with key:'"+key+"', object is of type: "+QString::number(type));
    }
    return Architecture::UnsetLocation;
}


void Heap::addToCurrentStackFrameAsFixed(QString key,int stringAddress){
    int oldStackFrameAddress = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    int stackFrameAddress = this->decodeRawIntFrom(oldStackFrameAddress+Architecture::TypeStackFrameGutterToParent);
    PByteArray* _data = new PByteArray();
    _data->setNum(stackFrameAddress);
    this->copyInTo(Architecture::CurrentStackFrameAddress,_data,Architecture::RawIntegerSize);
    if(ErrorRegistry::errorCount==0){
        this->addToCurrentStackFrame(key,stringAddress);
    }
    _data->setNum(oldStackFrameAddress);
    this->copyInTo(Architecture::CurrentStackFrameAddress,_data,Architecture::RawIntegerSize);
    delete _data;
}

void Heap::setCurrentStackFramePointer(int StackFrameAddress){

}

void Heap::copyInTo(int startIndex, PByteArray *data, int length){
    int i=0;
    while(i<length){
        this->data->setByte(startIndex,data->getByte(i));
        startIndex++;
        i++;
    }
}

void Heap::prependToArray(int arrayAddress,int valueAddress){
    if(this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeOpenArrayType || this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeClosedArrayType){
        int currentCapacity = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArrayGutterToCapacity);
        int arraySize = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArrayGutterToSize);
        int newArraySize = arraySize+1;
        if(newArraySize>currentCapacity){
            int A_Address = arrayAddress;
            int capacity = currentCapacity;
            int lastAddress = A_Address+Architecture::TypeArrayGutter+(capacity*Architecture::TypeArrayEntrySize);
            if(lastAddress == (this->data->size()-1)){
                ErrorRegistry::logError("Unwritten Code:End of array is the last entry in the heap.");
            }
            else{
                int newAAddress = this->data->size();
                int addr = A_Address;
                PByteArray* _data = new PByteArray();
                while(addr<lastAddress){
                    _data->resize(_data->size()+1);
                    _data->setByte(_data->size()-1,this->data->getByte(addr));
                    this->data->setByte(addr,NULL);
                    addr++;
                }
                int nCap = _data->decodeRawIntFrom(1);
                int x=0;
                while(x<(nCap*Architecture::TypeArrayEntrySize)){
                    _data->resize(_data->size()+1);
                    _data->setByte(_data->size()-1,NULL);
                    x++;
                }
                nCap*=2;
                _data->encodeRawIntegerAt(1,nCap);
                this->copyToNew(_data,_data->size());
                this->prependToArray(newAAddress,valueAddress);
                PByteArray* refPointer = new PByteArray();
                refPointer->setNum(newAAddress);
                refPointer->prepend(ByteCodeInstruction::TypeHeapFragment);
                this->copyInTo(A_Address,refPointer,refPointer->size());
            }
        }
        else{
            PByteArray* _nsize = new PByteArray();
            _nsize->setNum(newArraySize);
            this->copyInTo(arrayAddress+Architecture::TypeArrayGutterToSize,_nsize,Architecture::RawIntegerSize);
            int i=arraySize-1;
            while(i>=0){
                int e=0;
                while(e<Architecture::TypeArrayEntrySize){
                    this->data->setByte(arrayAddress+Architecture::TypeArrayGutter+(Architecture::TypeArrayEntrySize*(i+1)+e),this->data->getByte(arrayAddress+Architecture::TypeArrayGutter+(Architecture::TypeArrayEntrySize*i)+e));
                    e++;
                }
                i--;
            }
            _nsize->setNum(valueAddress);
            this->copyInTo(arrayAddress+Architecture::TypeArrayGutter,_nsize,Architecture::RawIntegerSize);
        }
    }
    else if(this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeArgsListType){

        this->interpreter->debuggerOut("Prepending #"+QString::number(valueAddress)+" of type:"+QString::number(this->data->getByte(valueAddress))+" to ArgsList..."+arrayToString(arrayAddress),true);

        int currentCapacity = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArgumentsListGutterToCapacity);
        int arraySize = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArgumentsListGutterToSize);
        int newArraySize = arraySize+1;
        if(newArraySize>currentCapacity){
            int A_Address = arrayAddress;
            int capacity = currentCapacity;
            int lastAddress = A_Address+Architecture::TypeArgumentsListGutter+(capacity*Architecture::TypeArrayEntrySize);
            if(lastAddress == (this->data->size()-1)){
                ErrorRegistry::logError("Unwritten Code:End of array is the last entry in the heap.");
            }
            else{
                int newAAddress = this->data->size();
                int addr = A_Address;
                PByteArray* _data = new PByteArray();
                int o=0;
                while(addr<lastAddress){
                    _data->resize(_data->size()+1);
                    _data->setByte(_data->size()-1,this->data->getByte(addr));

                    //////this->interpreter->debuggerOut("o:"+QString::number(o),true);

                    if(o==Architecture::TypeArgumentsListGutter){
                        int e=0;
                        PByteArray* _sub = new PByteArray();
                        _sub->setNum(valueAddress);
                        _sub->resize(4);
                        while(e<4){
                            _data->resize(_data->size()+1);
                            _data->setByte(_data->size()-1,_sub->getByte(e));
                            e++;
                            o++;
                        }
                    }
                    this->data->setByte(addr,NULL);
                    o++;
                    addr++;
                }
                int nCap = _data->decodeRawIntFrom(Architecture::TypeArgumentsListGutterToCapacity);
                int x=0;
                while(x<(nCap*Architecture::TypeArrayEntrySize)){
                    _data->resize(_data->size()+1);
                    _data->setByte(_data->size()-1,NULL);
                    x++;
                }
                nCap*=2;
                _data->encodeRawIntegerAt(Architecture::TypeArgumentsListGutterToCapacity,nCap);
                this->copyToNew(_data,_data->size());
                //this->prependToArray(newAAddress,valueAddress);
                PByteArray* refPointer = new PByteArray();
                refPointer->setNum(newAAddress);
                refPointer->prepend(ByteCodeInstruction::TypeHeapFragment);
                this->copyInTo(A_Address,refPointer,refPointer->size());
            }
        }
        else{
            PByteArray* _nsize = new PByteArray();
            _nsize->setNum(newArraySize);
            this->copyInTo(arrayAddress+Architecture::TypeArgumentsListGutterToSize,_nsize,Architecture::RawIntegerSize);
            int i=arraySize-1;
            while(i>=0){
                int e=0;
                while(e<Architecture::TypeArrayEntrySize){
                    this->data->setByte(arrayAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArrayEntrySize*(i+1)+e),this->data->getByte(arrayAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArrayEntrySize*i)+e));
                    e++;
                }
                i--;
            }
            _nsize->setNum(valueAddress);
            this->copyInTo(arrayAddress+Architecture::TypeArgumentsListGutter,_nsize,Architecture::RawIntegerSize);
        }

        //////this->interpreter->debuggerOut("Prepended to argslist:"+arrayToString(arrayAddress),true);
    }
    else{
        ErrorRegistry::logError("Error:: Cannot append to non-array of type on line:");
    }
}

void Heap::removeFromArray(int arrayAddress,int index){
    if(this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeEventList){
        int arrayIndex = arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize);
        int arraySize = this->decodeRawIntFrom(arrayAddress+Architecture::TypeEventListGutterToSize);
        PByteArray* restOfArray = new PByteArray();
        index++;
        while(index<arraySize){
            restOfArray->resize(restOfArray->size()+1);
            restOfArray->setByte(restOfArray->size()-1,this->data->getByte(arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize)+0));
            restOfArray->resize(restOfArray->size()+1);
            restOfArray->setByte(restOfArray->size()-1,this->data->getByte(arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize)+1));
            restOfArray->resize(restOfArray->size()+1);
            restOfArray->setByte(restOfArray->size()-1,this->data->getByte(arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize)+2));
            restOfArray->resize(restOfArray->size()+1);
            restOfArray->setByte(restOfArray->size()-1,this->data->getByte(arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize)+3));
            this->data->encodeRawIntegerAt(arrayAddress+Architecture::TypeEventListGutter+(index*Architecture::TypeEventListEntrySize),NULL);
            index++;
        }
        arraySize--;
        int i=0;
        while(i<restOfArray->size()){
            this->data->setByte(arrayIndex+i,restOfArray->getByte(i));
            i++;
        }
        restOfArray->resize(0);
        restOfArray->resize(Architecture::RawIntegerSize);
        restOfArray->setNum(arraySize);
        this->copyInTo(arrayAddress+Architecture::TypeEventListGutterToSize,restOfArray,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeOpenArrayType){
        int nsize = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArrayGutterToSize);
        if(index<0 || index>=nsize){
            ErrorRegistry::logError("Index out of range error: index "+QString::number(index)+" out of range on array of length "+QString::number(nsize)+" on line:"+QString::number(this->interpreter->lineNumber));
        }
        else{
            nsize--;
            int ccap = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArrayGutterToCapacity);
            if(nsize<(ccap/2)){
                PByteArray* _nsize = new PByteArray();
                _nsize->setNum(nsize);
                this->copyInTo(arrayAddress+Architecture::TypeArrayGutterToSize,_nsize,Architecture::RawIntegerSize);
                int i=index+1;
                while(i<(nsize+1)){
                    int e=0;
                    while(e<Architecture::TypeArrayEntrySize){
                        this->data->setByte(arrayAddress+Architecture::TypeArrayGutterToSize+(Architecture::TypeArrayEntrySize*(i-1)+e),this->data->getByte(arrayAddress+Architecture::TypeArrayGutter+(Architecture::TypeArrayEntrySize*i)+e));
                        e++;
                    }
                    i++;
                }
                _nsize->setNum(ccap/2);
                this->copyInTo(arrayAddress+Architecture::TypeArrayGutterToSize,_nsize,Architecture::RawIntegerSize);
            }
            else{
                PByteArray* _nsize = new PByteArray();
                _nsize->setNum(nsize);
                this->copyInTo(arrayAddress+Architecture::TypeArrayGutterToSize,_nsize,Architecture::RawIntegerSize);
                int i=index+1;
                while(i<(nsize+1)){
                    int e=0;
                    while(e<Architecture::TypeArrayEntrySize){
                        this->data->setByte(arrayAddress+Architecture::TypeArrayGutterToSize+(Architecture::TypeArrayEntrySize*(i-1)+e),this->data->getByte(arrayAddress+Architecture::TypeArrayGutter+(Architecture::TypeArrayEntrySize*i)+e));
                        e++;
                    }
                    i++;
                }
            }
        }
    }
    else{
        ErrorRegistry::logError("Internal Error:Cannot perform removal on non-array, non-event list, on line:"+QString::number(this->interpreter->lineNumber));
    }
}

void Heap::constructNewArgsList(int aAddress){
    int newArgsListAddress = this->data->size();
    aAddress = this->completeDeref(aAddress);
    int capacity = Architecture::TypeArgumentsListDefaultCapacity;
    int size = Architecture::TypeArgumentsListDefaultSize;
    this->data->resize(this->data->size()+Architecture::TypeArgumentsListGutter);
    this->data->setByte(newArgsListAddress,ByteCodeInstruction::TypeArgsListType);
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutterToSize,size);
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutterToCapacity,capacity);
    this->data->resize(this->data->size()+Architecture::TypeArgumentsListEntrySize);
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutter,aAddress);
    int i=0;
    while(i<(size*Architecture::TypeArgumentsListEntrySize)){
        this->accept(NULL);
        i++;
    }
    int newAddress = aAddress;
    if(this->data->getByte(aAddress)==ByteCodeInstruction::TypeConstIntegerType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsRunTimeInteger(0,this->decodeRawIntFrom(aAddress+Architecture::TypeConstIntegerGutter));
        newAddress = this->data->size();
        this->copyToNew(_aInt,Architecture::TypeRunTimeIntegerSize);
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter,_newAddress,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(aAddress)==ByteCodeInstruction::TypeConstStringType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsString(this->decodeStringFrom(aAddress),false);
        newAddress = this->data->size();
        this->copyToNew(_aInt,_aInt->size());
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter,_newAddress,Architecture::RawIntegerSize);
    }
    //args list are responsible pointers
    this->incrementReferenceCount(newAddress);
}

void Heap::constructNewArgsList(int aAddress,int bAddress){

    aAddress = this->completeDeref(aAddress);
    bAddress = this->completeDeref(bAddress);

    ////this->interpreter->debuggerOut("Constructing arguments list from :"+QString::number(this->data->getByte(aAddress))+", at "+QString::number(aAddress)+" and "+QString::number(this->data->getByte(bAddress))+", at "+QString::number(bAddress)+" on line:"+QString::number(this->interpreter->lineNumber),true);

    if(this->data->getByte(bAddress)==ByteCodeInstruction::TypeObjectType){
        //////this->debuggerOut("Constructing arguments list, B is object:"+this->objectToString(bAddress,0)+" on line:"+QString::number(this->interpreter->lineNumber),true);
    }

    int newArgsListAddress = this->data->size();
    int capacity = Architecture::TypeArgumentsListDefaultCapacity*2;
    int size = Architecture::TypeArgumentsListDefaultSize*2;
    this->data->resize(this->data->size()+Architecture::TypeArgumentsListGutter);
    this->data->setByte(newArgsListAddress,ByteCodeInstruction::TypeArgsListType);

    //this->debuggerOut("Encoding argslist size at:"+QString::number(newArgsListAddress+Architecture::TypeArgumentsListGutterToSize),true);

    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutterToSize,size);
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutterToCapacity,capacity);
    this->data->resize(this->data->size()+(Architecture::TypeArgumentsListEntrySize*2));
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutter,aAddress);
    this->data->encodeRawIntegerAt(newArgsListAddress+Architecture::TypeArgumentsListGutter+Architecture::TypeArgumentsListEntrySize,bAddress);
    int i=0;
    while(i<(size*Architecture::TypeArgumentsListEntrySize)){
        this->accept(NULL);
        i++;
    }
    int newAddress = aAddress;
    if(this->data->getByte(aAddress)==ByteCodeInstruction::TypeConstIntegerType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsRunTimeInteger(0,this->decodeRawIntFrom(aAddress+Architecture::TypeConstIntegerGutter));
        newAddress = this->data->size();
        this->copyToNew(_aInt,Architecture::TypeRunTimeIntegerSize);
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter,_newAddress,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(aAddress)==ByteCodeInstruction::TypeConstFloatType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsRunTimeFloat(0,this->decodeRawIntFrom(aAddress+Architecture::TypeConstFloatGutter));
        newAddress = this->data->size();
        this->copyToNew(_aInt,Architecture::TypeRunTimeFloatSize);
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter,_newAddress,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(aAddress)==ByteCodeInstruction::TypeConstStringType){
        ////this->interpreter->debuggerOut("A is constant string",true);
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsString(this->decodeStringFrom(aAddress),false);
        newAddress = this->data->size();
        this->copyToNew(_aInt,_aInt->size());
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter,_newAddress,Architecture::RawIntegerSize);
    }
    this->incrementReferenceCount(newAddress);
    newAddress = bAddress;
    if(this->data->getByte(bAddress)==ByteCodeInstruction::TypeConstIntegerType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsRunTimeInteger(0,this->decodeRawIntFrom(bAddress+Architecture::TypeConstIntegerGutter));
        newAddress = this->data->size();
        this->copyToNew(_aInt,Architecture::TypeRunTimeIntegerSize);
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter+Architecture::TypeArgumentsListEntrySize,_newAddress,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(bAddress)==ByteCodeInstruction::TypeConstFloatType){
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsRunTimeFloat(0,this->decodeRawIntFrom(bAddress+Architecture::TypeConstFloatGutter));
        newAddress = this->data->size();
        this->copyToNew(_aInt,Architecture::TypeRunTimeFloatSize);
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter+Architecture::TypeArgumentsListEntrySize,_newAddress,Architecture::RawIntegerSize);
    }
    else if(this->data->getByte(bAddress)==ByteCodeInstruction::TypeConstStringType){
        ////this->interpreter->debuggerOut("B is constant string",true);
        PByteArray* _aInt = new PByteArray();
        _aInt->encodeAsString(this->decodeStringFrom(bAddress),false);
        newAddress = this->data->size();
        this->copyToNew(_aInt,_aInt->size());
        PByteArray* _newAddress = new PByteArray();
        _newAddress->setNum(newAddress);
        this->copyInTo(newArgsListAddress+Architecture::TypeArgumentsListGutter+Architecture::TypeArgumentsListEntrySize,_newAddress,Architecture::RawIntegerSize);
    }

    ////this->interpreter->debuggerOut("Created args list:"+this->arrayToString(newArgsListAddress),true);

    this->incrementReferenceCount(newAddress);
}

int Heap::getValueAddressInArray(int arrayAddress,int index){
    if(this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeOpenArrayType || this->data->getByte(arrayAddress)==ByteCodeInstruction::TypeClosedArrayType){
        int size = this->decodeRawIntFrom(arrayAddress+Architecture::TypeArrayGutterToSize);
        if(index<size && index>=0){
            int r = arrayAddress+Architecture::TypeArrayGutter+(index*Architecture::TypeArrayEntrySize);
            return r;
        }
        else{
            ErrorRegistry::logError("Error! Index "+QString::number(index)+" out of range on array of length:"+QString::number(size)+", on line:"+QString::number(this->interpreter->lineNumber));
            return Architecture::NULLLocation;
        }
    }
    else{
        ErrorRegistry::logError("Error!:Attempted array look up on non-array, type "+QString::number(this->data->getByte(arrayAddress))+", at address:#"+QString::number(arrayAddress)+" on line:"+QString::number(this->interpreter->lineNumber));
    }
}

void Heap::constructNewArray(int aAddress){

    ////////this->interpreter->debuggerOut(("Array constructed... at address:"+QString::number(this->data->size())+", to store address:"+QString::number(aAddress),true);

    unsigned char typeBit = ByteCodeInstruction::TypeOpenArrayType;
    int capacity = Architecture::TypeArgumentsListDefaultCapacity;
    PByteArray* _capacity = new PByteArray();
    _capacity->setNum(capacity);
    int size = Architecture::TypeArgumentsListDefaultSize;
    PByteArray* _size = new PByteArray();
    _size->setNum(size);
    PByteArray* _refCount = new PByteArray();
    _refCount->setNum(1);
    PByteArray* _encodedA = new PByteArray();
    _encodedA->setNum(aAddress);
    this->accept(typeBit);
    int i=0;
    while(i<_capacity->size()){
        this->accept(_capacity->getByte(i));
        i++;
    }
    i=0;
    while(i<_size->size()){
        this->accept(_size->getByte(i));
        i++;
    }
    i=0;
    while(i<_refCount->size()){
        this->accept(_refCount->getByte(i));
        i++;
    }
    i=0;
    while(i<_encodedA->size()){
        this->accept(_encodedA->getByte(i));
        i++;
    }
    i=0;
    while(i<(size*Architecture::TypeArrayDefaultCapacity)){
        this->accept(NULL);
        i++;
    }
}

void Heap::constructNewArray(){
    unsigned char typeBit = ByteCodeInstruction::TypeOpenArrayType;
    int capacity = Architecture::TypeArrayDefaultCapacity;
    PByteArray* _capacity = new PByteArray();
    _capacity->setNum(capacity);
    int size = Architecture::TypeArrayDefaultSize;
    PByteArray* _size = new PByteArray();
    _size->setNum(size);
    PByteArray* _refCount = new PByteArray();
    _refCount->setNum(1);
    this->accept(typeBit);
    int i=0;
    while(i<_capacity->size()){
        this->accept(_capacity->getByte(i));
        i++;
    }
    i=0;
    while(i<_size->size()){
        this->accept(_size->getByte(i));
        i++;
    }
    i=0;
    while(i<_refCount->size()){
        this->accept(_size->getByte(i));
        i++;
    }
    i=0;
    while(i<(size*Architecture::TypeArrayEntrySize)){
        this->accept(NULL);
        i++;
    }
}
void Heap::setRETBuffer(int address){
//    this->data->encodeRawIntegerAt(Architecture::ReturnAddress,address);
    int previous = this->decodeRawIntFrom(Architecture::ReturnAddress);
    this->data->encodeRawIntegerAt(Architecture::ReturnAddress,address);
    if(previous!=address){
        this->incrementReferenceCount(address);
        this->decrementReferenceCount(previous);
    }
}

int Heap::valueAddressInStackFrames(QString key,int stringAddress){
    int _topStackFrameAddress = this->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    int requestLexicalId = this->interpreter->getLexicalID();
    return this->valueAddressInStackFrames(key,stringAddress,_topStackFrameAddress,requestLexicalId);
}

void Heap::setUp(int rpnCount){
    this->interpreter->nativeStore->resolve(NativeStore::HeapVisualizerAddress,{"launch"});
    Architecture::SIZE_OF_INITIAL_HEAP_STORE = Architecture::RPNAddress+(rpnCount*Architecture::RawIntegerSize);
    int i=0;
    while(i<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
        this->accept(NULL);
        i++;
    }
}

void Heap::draw(){
}


QString Heap::prettyByte(char byteVal){
    QString res="";
    if(byteVal==ByteCodeInstruction::TypeConstIntegerType){
        res+="Type:Integer:Type";
    }
    else if(byteVal==ByteCodeInstruction::TypeConstStringType){
        res+="Type:String:Type";
    }
    else if(byteVal==ByteCodeInstruction::TypeKeyType){
        res+="Type:Key:Type";
    }
    else if(byteVal==Architecture::TypeLettedScope){
        res+="Type:LettedScope:Type";
    }
    else if(byteVal==Architecture::TypeFixedScope){
        res+="Type:FixedScope:Type";
    }
    else{
        res+=byteVal;
    }
    return res;
}

bool Heap::LastRPNBufferIsTruthy(){
    int _address = Heap::decodeRawIntFrom(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize);
    if(_address==Architecture::NULLLocation){
        return false;
    }
    else if(_address==Architecture::UnsetLocation){
        return false;
    }
    else if(_address==Architecture::FalseLocation){
        return false;
    }
    else{
        return true;
    }
    return false;
}

int Heap::decodeRawIntFrom(int index){
    if(index>this->data->size()-(Architecture::RawIntegerSize)){
        return Architecture::NULLLocation;
    }
    else{
        PByteArray* res = new PByteArray();
        int i=0;
        while(i<Architecture::RawIntegerSize){
            res->resize(res->size()+1);
            res->setByte(res->size()-1,this->data->getByte(index+i));
            i++;
        }
        int x = res->toInt();
        return x;
    }
}

float Heap::decodeRawFloatFrom(int index){
    if(index>this->data->size()-(Architecture::RawIntegerSize)){
        ErrorRegistry::logError("Internal Error! Attempted reading decoding of integer at index > data.size() - raw int size(4), on line:"+QString::number(this->interpreter->lineNumber));
        return 0.0;
    }
    else{
        PByteArray* res = new PByteArray();
        int i=0;
        while(i<Architecture::RawIntegerSize){
            res->resize(res->size()+1);
            res->setByte(res->size()-1,this->data->getByte(index+i));
            i++;
        }
        float x = res->toFloat();
        return x;
    }
}

void Heap::copyToNew(PByteArray* data, int length){
    int i=0;
    while(i<length){
        Heap::accept(data->getByte(i));
        i++;
    }
}

QString Heap::arrayToString(int address){

    ////////this->interpreter->debuggerOut(("Printing array at address:"+QString::number(address),true);

    int GUTTER;
    int SIZE_GUTTER;
    int CAP_GUTTER;
    if(this->data->getByte(address)==ByteCodeInstruction::TypeOpenArrayType || this->data->getByte(address)==ByteCodeInstruction::TypeClosedArrayType){
        GUTTER = Architecture::TypeArrayGutter;
        SIZE_GUTTER = Architecture::TypeArrayGutterToSize;
        CAP_GUTTER = Architecture::TypeArrayGutterToCapacity;
    }
    else if(this->data->getByte(address)==ByteCodeInstruction::TypeArgsListType){
        GUTTER = Architecture::TypeArgumentsListGutter;
        SIZE_GUTTER = Architecture::TypeArgumentsListGutterToSize;
        CAP_GUTTER = Architecture::TypeArgumentsListGutterToCapacity;
    }
    else{

    }

    QString res="[";// = "[ARRAY][";
    int size = decodeRawIntFrom(address+SIZE_GUTTER);
    int cap = decodeRawIntFrom(address+CAP_GUTTER);
    int i=0;

    res+="[size:"+QString::number(size)+"],[_cap:"+QString::number(cap)+"]";

    address+=GUTTER;
    while(i<size){
        int valAddress = this->decodeRawIntFrom(address);

        ////////this->interpreter->debuggerOut(("Address of value in array:"+QString::number(valAddress),true);

        if(valAddress==Architecture::NULLLocation){
            res+="null";
        }
        else if(valAddress==Architecture::UnsetLocation){
            res+="unset";
        }
        else if(valAddress==Architecture::TrueLocation){
            res+="true";
        }
        else if(valAddress==Architecture::FalseLocation){
            res+="false";
        }
        else if(valAddress==Architecture::NativeLocation){
            res+="native";
        }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeRunTimeIntegerType){
            res+=QString::number(this->decodeRawIntFrom(valAddress+Architecture::TypeRunTimeIntegerGutter));
        }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeRunTimeStringType){
            res+=(this->decodeStringFrom(valAddress));
       }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeConstObjectType){
            res+="constant-object";
        }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeObjectType){
            res+="object";
        }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeOpenArrayType){
            res+="array";
        }
        else if(this->data->getByte(valAddress)==ByteCodeInstruction::TypeLambdaType){
            res+="lambda";
        }
        else{
            res+="unknown";
        }
        if(i!=(size-1)){
            res+=",";
        }
        address+=Architecture::TypeArrayEntrySize;
        i++;
    }
    res+="]";
    return res;
}

QString Heap::decodeStringFrom(int index){
    return this->data->decodeStringFrom(index);
}
