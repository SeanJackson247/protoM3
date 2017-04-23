#include "alu.h"

#include "bytecodeinstruction.h"
#include "architecture.h"

#include "errorregistry.h"
#include "pbytearray.h"
#include "heapstore.h"

ERU::ERU(Interpreter* interpreter){
    this->p = interpreter;
    this->heap = this->p->heap;
}

void ERU::debuggerOut(QString out){
    ////////////this->p->debuggerOut(out);
}

void ERU::eval(int A_Address,int B_Address,QString _operator,unsigned char buffer,int instructionPointer){
    ////////////this->p->debuggerOut("B Address passed to ERU:"+QString::number(B_Address),true);
    unsigned char A_Type = this->heap->data->getByte(A_Address);
    unsigned char B_Type = this->heap->data->getByte(B_Address);
    PByteArray* Bbytes = new PByteArray();
    Bbytes->setNum(B_Address);
    if(_operator.length()==0){
        //////this->p->debuggerOut("Empty Operator! A Address prior to deref:"+QString::number(A_Address),true);

        A_Address = this->heap->completeDeref(A_Address);
        //////this->p->debuggerOut("Empty Operator! A Address after __ deref:"+QString::number(A_Address)+", containing type:"+QString::number(this->heap->data->getByte(A_Address)),true);
        A_Type = this->heap->data->getByte(A_Address);
        if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            A_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(A_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            A_Type = this->heap->data->getByte(A_Address);
        }
    }
    else if(QString::compare(_operator,"=")==0){
        B_Address = this->heap->completeDeref(B_Address);
        B_Type = this->heap->data->getByte(B_Address);
        if(B_Type==ByteCodeInstruction::TypeConstObjectType){
            int new_B_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(B_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            B_Address = new_B_Address;
            B_Type = this->heap->data->getByte(B_Address);
        }
    }
    else if(QString::compare(_operator,",")==0){
        //////this->p->debuggerOut("List Operator!",true);
        A_Address = this->heap->completeDeref(A_Address);
        A_Type = this->heap->data->getByte(A_Address);
        //////this->p->debuggerOut("B Address prior to deref:"+QString::number(B_Address)+" with type: "+QString::number(B_Type),true);
        B_Address = this->heap->completeDeref(B_Address);
        B_Type = this->heap->data->getByte(B_Address);
        //////this->p->debuggerOut("B Address post deref:"+QString::number(B_Address),true);
    }
    else if(QString::compare(_operator,"$")==0){
        ////////////this->p->debuggerOut("$ Operator!",true);
        ////////////this->p->debuggerOut("A Address prior to partial deref:"+QString::number(A_Address)+", with type:"+QString::number(A_Type),true);
        A_Address = this->heap->partialDeref(A_Address);
        ////////////this->p->debuggerOut("A Address after partial deref:"+QString::number(A_Address),true);
        A_Type = this->heap->data->getByte(A_Address);
        if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            int new_A_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(A_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            A_Address = new_A_Address;
            A_Type = this->heap->data->getByte(A_Address);
        }
        ////////////this->p->debuggerOut("B Address to prior deref:"+QString::number(B_Address),true);
        B_Address = this->heap->completeDeref(B_Address);
        B_Type = this->heap->data->getByte(B_Address);
        ////////////this->p->debuggerOut("B Address:"+QString::number(B_Address),true);
        if(B_Type==ByteCodeInstruction::TypeConstObjectType){
            int new_B_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(B_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            B_Address = new_B_Address;
            B_Type = this->heap->data->getByte(B_Address);
        }
    }
    else if(QString::compare(_operator,"^")==0){//array look up....
        if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
            A_Address = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
            A_Address = this->heap->decodeRawIntFrom(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
        }
        else if(A_Type==ByteCodeInstruction::TypeKeyType){
            QString aString = this->heap->decodeStringFrom(A_Address);
            A_Address = this->heap->valueAddressInStackFrames(aString,A_Address);
            A_Address = this->heap->decodeRawIntFrom(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
        }
        B_Address = this->heap->completeDeref(B_Address);
        B_Type = this->heap->data->getByte(B_Address);
        if(B_Type==ByteCodeInstruction::TypeConstObjectType){
            int new_B_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(B_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            B_Address = new_B_Address;
            B_Type = this->heap->data->getByte(B_Address);
        }
    }
    else if(QString::compare(_operator,".")==0){
        A_Address = this->heap->completeDeref(A_Address);
        A_Type = this->heap->data->getByte(A_Address);
        if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            int new_A_Address = this->heap->data->size();
            this->heap->copyConstantObjectInitAndDefine(A_Address,buffer,Architecture::NULLLocation,Architecture::NULLLocation);
            A_Address = new_A_Address;
            A_Type = this->heap->data->getByte(A_Address);
        }
    }
    if(_operator.length()==0){
        //////this->p->debuggerOut("EMPTY OPERATOR!!",true);
        if(buffer>=Architecture::RPNAddress && buffer<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
            this->heap->setRPNBuffer(buffer,A_Address);
        }
        else{
            ////////////////this->p->debuggerOut("Setting Return Buffer! to A Address:"+QString::number(A_Address));
            this->heap->setRETBuffer(A_Address);
        }
    }
    else if(QString::compare(_operator,"=")==0){
        if(A_Address==Architecture::NULLLocation){
            ErrorRegistry::logError("Error! Cannot assign NULL.");
        }
        else if(A_Address==Architecture::UnsetLocation){
            ErrorRegistry::logError("Error! Cannot assign unset.");
        }
        else if(A_Address==Architecture::TrueLocation){
            ErrorRegistry::logError("Error! Cannot assign true.");
        }
        else if(A_Address==Architecture::FalseLocation){
            ErrorRegistry::logError("Error! Cannot assign false.");
        }
        else if(A_Address==Architecture::NativeLocation){
            ErrorRegistry::logError("Error! Cannot assign native.");
        }
        else{
            if(A_Type==ByteCodeInstruction::TypeKeyType){

                ////////////this->p->debuggerOut("Assignment to type:"+QString::number(B_Type),true);

                int previous = this->heap->decodeRawIntFrom(this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(A_Address),A_Address));
                if(B_Address==Architecture::NULLLocation){
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->setNum(Architecture::NULLLocation);
                    this->heap->copyInTo(AValAddress,pbytes,Architecture::RawIntegerSize);
                }
                else if(B_Address==Architecture::UnsetLocation){
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->setNum(Architecture::UnsetLocation);
                    this->heap->copyInTo(AValAddress,pbytes,Architecture::RawIntegerSize);
                }
                else if(B_Address==Architecture::TrueLocation){
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->setNum(Architecture::TrueLocation);
                    this->heap->copyInTo(AValAddress,pbytes,Architecture::RawIntegerSize);
                }
                else if(B_Address==Architecture::FalseLocation){
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* pbytes = new PByteArray();
                    pbytes->setNum(Architecture::FalseLocation);
                    this->heap->copyInTo(AValAddress,pbytes,Architecture::RawIntegerSize);
                }
                else if(B_Address==Architecture::NativeLocation){
                    ErrorRegistry::logError("Error - cannot assign to native.");
                }
                else if(B_Address==Architecture::ArrayInitLocation){
                    B_Address = this->heap->data->size();
                    this->heap->constructNewArray();
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int valAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(valAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType || B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    int GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        GUTTER=Architecture::TypeRunTimeIntegerGutter;
                    }
                    else{
                        GUTTER=Architecture::TypeConstIntegerGutter;
                    }
                    int bAddress = this->heap->decodeRawIntFrom(B_Address+GUTTER);
                    int newBAddress = this->heap->data->size();
                    PByteArray* B_Data = new PByteArray();
                    B_Data->encodeAsRunTimeInteger(0,bAddress);
                    this->heap->copyToNew(B_Data,Architecture::TypeRunTimeIntegerSize);
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(newBAddress);
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        this->heap->setRPNBuffer(buffer,AValAddress);
                    }
                    else{
                        this->heap->setRETBuffer(AValAddress);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeFloatType || B_Type==ByteCodeInstruction::TypeConstFloatType){
                    int GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeConstFloatType){
                        GUTTER = Architecture::TypeConstFloatGutter;
                    }
                    else{
                        GUTTER = Architecture::TypeRunTimeFloatGutter;
                    }
                    float bAddress = this->heap->decodeRawFloatFrom(B_Address+GUTTER);

                    ////////////this->p->debuggerOut("Assignment to float:"+QString::number(bAddress,'G',5),true);

                    int newBAddress = this->heap->data->size();
                    PByteArray* B_Data = new PByteArray();
    //                B_Data->encodeAsRawFloat(bAddress);
                    B_Data->encodeAsRunTimeFloat(0,bAddress);
    //                B_Data->prepend(ByteCodeInstruction::TypeRunTimeFloatType);
                    this->heap->copyToNew(B_Data,Architecture::TypeRunTimeFloatSize);
                    QString AString = this->heap->decodeStringFrom(A_Address);
                    int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(newBAddress);
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawFloatSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        this->heap->setRPNBuffer(buffer,AValAddress);
                    }
                    else{
                        this->heap->setRETBuffer(AValAddress);
                    }
                    if(ErrorRegistry::errorCount>0){
                        ////////////this->p->debuggerOut("Completed Assignment with errors.",true);
                    }
                    else{
                        ////////////this->p->debuggerOut("Completed Assignment:",true);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeObjectType){
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int valAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(valAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int valAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(valAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddress = this->heap->valueAddressInStackFrames(bString,B_Address);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int aValAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
                    PByteArray* _bValAddress = new PByteArray();
                    _bValAddress->setNum(bValAddress);
                    this->heap->copyInTo(aValAddress,_bValAddress,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddress = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int aValAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
                    PByteArray* _bValAddress = new PByteArray();
                    _bValAddress->setNum(bValAddress);
                    this->heap->copyInTo(aValAddress,_bValAddress,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    if(B_Address<this->heap->initThreshold){
                        //QString bAddress = this->heap->decodeStringFrom(Bbytes->toInt());
                        QString bAddress = this->heap->decodeStringFrom(B_Address);

                        ////////////this->p->debuggerOut("Assignment to string:"+bAddress,true);

                        int newBAddress = this->heap->data->size();
                        PByteArray* B_Data = new PByteArray();
                        B_Data->encodeAsString(bAddress,false);
                        this->heap->copyToNew(B_Data,B_Data->size());
                        QString AString = this->heap->decodeStringFrom(A_Address);
                        int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
                        PByteArray* bytes = new PByteArray();
                        bytes->setNum(newBAddress);
                        this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->heap->setRPNBuffer(buffer,A_Address);
                        }
                        else{
                            this->heap->setRETBuffer(A_Address);
                        }

                    }
                    else{
                        PByteArray* data = new PByteArray();
                        data->setNum(B_Address);
                        QString aString = this->heap->decodeStringFrom(A_Address);
                        int startIndex = this->heap->valueAddressInStackFrames(aString,A_Address);
                        this->heap->copyInTo(startIndex,data,Architecture::RawIntegerSize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->heap->setRPNBuffer(buffer,B_Address);
                        }
                        else{
                            this->heap->setRETBuffer(B_Address);
                        }
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeOpenArrayType || B_Type==ByteCodeInstruction::TypeClosedArrayType){
                    PByteArray* data = new PByteArray();
                    data->setNum(B_Address);
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int startIndex = this->heap->valueAddressInStackFrames(aString,A_Address);
                    this->heap->copyInTo(startIndex,data,Architecture::RawIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        this->heap->setRPNBuffer(buffer,B_Address);
                    }
                    else{
                        this->heap->setRETBuffer(B_Address);
                    }
                }
                else{
                    qDebug()<<"Logging assignment error...";
                    ErrorRegistry::logError("Cannot perform assignment, lefthand is key, righthand operand is unknown type, type given:"+QString::number(B_Type)+" at address:#"+QString::number(B_Address)+", on line "+QString::number(this->p->lineNumber));
                }
                this->p->heap->incrementReferenceCount(B_Address);
                this->p->heap->decrementReferenceCount(previous);
            }
            else if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
                //////////////this->p->debuggerOut("Assignment of property key!",true);
                if(B_Address<Architecture::RPNAddress){
                    int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType || B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    int B_GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        B_GUTTER = Architecture::TypeRunTimeIntegerGutter;
                    }
                    else{
                        B_GUTTER=Architecture::TypeConstIntegerGutter;
                    }
                    int bVal = this->heap->decodeRawIntFrom(B_Address+B_GUTTER);
                    int newBAddress = this->heap->data->size();

                    //////////////this->p->debuggerOut("Assignment of property key to number:"+QString::number(bVal),true);
                    PByteArray* B_Data = new PByteArray();
                    B_Data->encodeAsRunTimeInteger(1,bVal);
                    this->heap->copyToNew(B_Data,Architecture::TypeRunTimeIntegerSize);
                    int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(newBAddress);
                    this->heap->decrementReferenceCount(this->heap->decodeRawIntFrom(AValAddress));
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,newBAddress);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeObjectType){
                    int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,AValAddress);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                    int valAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(valAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    PByteArray* pBytes = new PByteArray();
                    pBytes->setNum(this->heap->decodeRawIntFrom(this->heap->valueAddressInStackFrames(bString,B_Address)));
                    this->heap->copyInTo(this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter),pBytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    if(B_Address<this->heap->initThreshold){
                        QString bAddress = this->heap->decodeStringFrom(Bbytes->toInt());
                        int newBAddress = this->heap->data->size();
                        PByteArray* B_Data = new PByteArray();
                        B_Data->encodeAsString(bAddress,false);
                        this->heap->copyToNew(B_Data,B_Data->size());
                        int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                        PByteArray* bytes = new PByteArray();
                        bytes->setNum(newBAddress);
                        this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->heap->setRPNBuffer(buffer,AValAddress);
                        }
                        else{
                            this->heap->setRETBuffer(AValAddress);
                        }
                    }
                    else{
                        int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                        PByteArray* bytes = new PByteArray();
                        bytes->setNum(B_Address);
                        this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->heap->setRPNBuffer(buffer,AValAddress);
                        }
                        else{
                            this->heap->setRETBuffer(AValAddress);
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Cannot perform assignment on property key, right operand is unknown type("+QString::number(B_Type)+") on line:"+QString::number(this->p->lineNumber));
                }
            }
            else if(A_Type==ByteCodeInstruction::TypeArrayIndexType){
                if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    int GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                        GUTTER = Architecture::TypeConstIntegerGutter;
                    }
                    else{
                        GUTTER = Architecture::TypeRunTimeIntegerGutter;
                    }
                    int bAddress = this->heap->decodeRawIntFrom(B_Address+GUTTER);
                    int newBAddress = this->heap->data->size();
                    PByteArray* B_Data = new PByteArray();
                    B_Data->encodeAsRunTimeInteger(0,bAddress);
                    this->heap->copyToNew(B_Data,Architecture::TypeRunTimeIntegerSize);
                    int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypeArrayIndexGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(newBAddress);
                    this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,AValAddress);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeObjectType){
                    ErrorRegistry::logError("Unwritten code: Assignment on array index type = object-type.");
                }
                else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                    ErrorRegistry::logError("Unwritten code: Assignment on array index type = lambda-type.");
                    int valAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypeArrayIndexGutter);
                    PByteArray* bytes = new PByteArray();
                    bytes->setNum(B_Address);
                    this->heap->copyInTo(valAddress,bytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    ErrorRegistry::logError("Unwritten code: Assignment on array index type = key-type.");
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    PByteArray* pBytes = new PByteArray();
                    pBytes->setNum(this->heap->decodeRawIntFrom(this->heap->valueAddressInStackFrames(bString,B_Address)));
                    this->heap->copyInTo(this->heap->decodeRawIntFrom(A_Address+Architecture::TypeArrayIndexGutter),pBytes,Architecture::RawIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    if(B_Address<this->heap->initThreshold){
                        QString bAddress = this->heap->decodeStringFrom(Bbytes->toInt());
                        int newBAddress = this->heap->data->size();
                        PByteArray* B_Data = new PByteArray();
                        B_Data->encodeAsString(bAddress,false);
                        this->heap->copyToNew(B_Data,B_Data->size());
                        int AValAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypeArrayIndexGutter);
                        PByteArray* bytes = new PByteArray();
                        bytes->setNum(newBAddress);
                        this->heap->copyInTo(AValAddress,bytes,Architecture::RawIntegerSize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->heap->setRPNBuffer(buffer,AValAddress);
                        }
                        else{
                            this->heap->setRETBuffer(AValAddress);
                        }
                    }
                    else{
                        ErrorRegistry::logError("Unwritten Code: assignment of array-index type to non-constant string.");
                    }
                }
                else{
                    ErrorRegistry::logError("Cannot perform assignment, left operand is array-index-type, right operand is unknown type. Type given:"+QString::number(B_Type)+", at address: #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                }
            }
            else{
                qDebug()<<"Left hand assignment error";
                ErrorRegistry::logError("Cannot perform assignment, left operand is unknown type. Type given:"+QString::number(A_Type)+", at address: #"+QString::number(A_Address)+", on line:"+QString::number(this->p->lineNumber));
            }
        }
    }
    else if(QString::compare(_operator,",")==0){
        ////this->p->debuggerOut("List operator!...",true);
        if(A_Address==Architecture::ArrayInitLocation){
            ////this->p->debuggerOut("List operation on array initialisation",true);
            if(B_Address<this->heap->initThreshold){
                if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    int bVal = this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter);
                    B_Address = this->heap->data->size();
                    PByteArray* _bVal = new PByteArray();
                    _bVal->encodeAsRunTimeInteger(0,bVal);
                    this->heap->copyToNew(_bVal,Architecture::TypeRunTimeIntegerSize);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType){
                    ////this->p->debuggerOut("List operation on array initialisation and string",true);
                    QString bAddress = this->heap->decodeStringFrom(Bbytes->toInt());
                    B_Address = this->heap->data->size();
                    PByteArray* B_Data = new PByteArray();
                    B_Data->encodeAsString(bAddress,false);
                    this->heap->copyToNew(B_Data,B_Data->size());
                }
                else if(B_Type==ByteCodeInstruction::TypeOpenArrayType){
                    ErrorRegistry::logError("Unwritten Code: List operator on ArrayInit and const-array-type.");
                }
                else if(B_Type==ByteCodeInstruction::TypeConstObjectType){
                    ErrorRegistry::logError("Unwritten Code: List operator on ArrayInit and const-object-type.");
                }
                else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                    ErrorRegistry::logError("Unwritten Code: List operator on ArrayInit and const-lambda-type.");
                }
                else{
                    ErrorRegistry::logError("Unwritten Code: List operator on ArrayInit and uncaught type.");
                }
            }
            int newIndex;
            if(this->heap->data->getByte(B_Address)==ByteCodeInstruction::TypeArgsListType){
                newIndex = this->heap->data->size();
                this->heap->constructNewArray();
                int e=0;
                int argsListSize = this->heap->decodeRawIntFrom(B_Address+Architecture::TypeArgumentsListGutterToSize);
                while(e<argsListSize){
                    this->heap->appendToArray(newIndex,this->heap->decodeRawIntFrom(B_Address+Architecture::TypeArgumentsListGutter+(e*Architecture::RawIntegerSize)));
                    e++;
                }
            }
            else{
                newIndex = this->heap->data->size();
                this->heap->constructNewArray(B_Address);
            }
            int sameIndex = this->heap->completeDeref(newIndex);
            this->heap->data->setByte(sameIndex,ByteCodeInstruction::TypeClosedArrayType);
            if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                this->heap->setRPNBuffer(buffer,newIndex);
            }
            else{
                this->heap->setRETBuffer(newIndex);
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeOpenArrayType){
            ////this->p->debuggerOut("list operation on array.",true);
            if(B_Address==Architecture::NULLLocation){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is null.");
            }
            else if(B_Address==Architecture::UnsetLocation){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is unset.");
            }
            else if(B_Address==Architecture::TrueLocation){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is true.");
            }
            else if(B_Address==Architecture::FalseLocation){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is false.");
            }
            else if(B_Address==Architecture::NativeLocation){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is native.");
            }
            else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType || B_Type==ByteCodeInstruction::TypeConstIntegerType){
                if(B_Address<this->heap->initThreshold){
                    int new_B_Address = this->heap->data->size();
                    PByteArray* data = new PByteArray();
                    data->encodeAsRunTimeInteger(0,this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                    this->heap->copyToNew(data,Architecture::TypeRunTimeIntegerSize);
                    B_Address = new_B_Address;
                }
                this->heap->appendToArray(A_Address,B_Address);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,A_Address);
                }
                else{
                    this->heap->setRETBuffer(A_Address);
                }
            }
            else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                if(B_Address<this->heap->initThreshold){
                    QString bAddress = this->heap->decodeStringFrom(Bbytes->toInt());
                    B_Address = this->heap->data->size();
                    PByteArray* B_Data = new PByteArray();
                    B_Data->encodeAsString(bAddress,false);
                    this->heap->copyToNew(B_Data,B_Data->size());
                }

                this->heap->appendToArray(A_Address,B_Address);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,A_Address);
                }
                else{
                    this->heap->setRETBuffer(A_Address);
                }
            }
            else if(B_Type==ByteCodeInstruction::TypeConstObjectType){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is a const-object.");
            }
            else if(B_Type==ByteCodeInstruction::TypeObjectType){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is an object.");
            }
            else if(B_Type==ByteCodeInstruction::TypeOpenArrayType){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is an array.");
            }
            else if(B_Type==ByteCodeInstruction::TypeArgsListType){
                ErrorRegistry::logError("Unwritten Code: list operator, lefthand side is an array, righthand side is an argslist.");
            }
            else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                this->heap->appendToArray(A_Address,B_Address);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,A_Address);
                }
                else{
                    this->heap->setRETBuffer(A_Address);
                }
            }
            else{
                ErrorRegistry::logError("Invalid type: list operator, lefthand side is an array, righthand side is of type:"+QString::number(B_Type)+", at address:#"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            if(A_Type==ByteCodeInstruction::TypeArgsListType){
                ////this->p->debuggerOut("list operator on argslist!",true);
                ////this->p->debuggerOut("list on argslist",true);
                this->heap->appendToArray(A_Address,B_Address);
            }
            else if(A_Type==ByteCodeInstruction::TypeHeapFragment){
                ErrorRegistry::logError("Unwritten code, list operator on Heap Fragment.");
            }
            else{
                ////this->p->debuggerOut("not array init,array,arglist or heapfragment.",true);
                bool alreadyArgsList=false;
                int newIndex = this->heap->data->size();
                if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int aValAddr = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
                    int aAddr = this->heap->decodeRawIntFrom(aValAddr);
                    if(aValAddr<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        aAddr = aValAddr;
                    }
                    A_Address = aAddr;
                }
                else if(A_Type==ByteCodeInstruction::TypeKeyType){
                    QString aString = this->heap->decodeStringFrom(A_Address);
                    int aValAddr = this->heap->valueAddressInStackFrames(aString,A_Address);
                    int aAddr = this->heap->decodeRawIntFrom(aValAddr);
                    if(aValAddr<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        aAddr = aValAddr;
                    }
                    A_Address = aAddr;
                }
                else if(A_Type==ByteCodeInstruction::TypeOpenArrayType){
                    ////this->p->debuggerOut("OPEN-ARRAY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1",true);
                }
                else if(A_Type==ByteCodeInstruction::TypeArgsListType){
                    ////this->p->debuggerOut("ARGSLIST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1",true);
                }
                if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddr = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    int bAddr = this->heap->decodeRawIntFrom(bValAddr);
                    if(bValAddr<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        bAddr = bValAddr;
                    }
                    B_Address = bAddr;
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddr = this->heap->valueAddressInStackFrames(bString,B_Address);
                    int bAddr = this->heap->decodeRawIntFrom(bValAddr);
                    if(bValAddr<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        bAddr = bValAddr;
                    }
                    B_Address = bAddr;
                }
                else if(B_Type==ByteCodeInstruction::TypeOpenArrayType){
                    ErrorRegistry::logError("Unwritten Code:Append non-iterable to openarray.");
                }
                else if(B_Type==ByteCodeInstruction::TypeArgsListType){
                    //ErrorRegistry::logError("Unwritten Code:Append non-iterable to argslist.");
                    if(this->heap->data->getByte(A_Address)==ByteCodeInstruction::TypeConstStringType){
                        int newA = this->heap->data->size();
                        PByteArray* newString = new PByteArray();
                        QString str = this->heap->decodeStringFrom(A_Address);
                        newString->encodeAsString(str,false);
                        this->heap->copyToNew(newString,newString->size());
                        A_Address = newA;
                    }
                    this->heap->prependToArray(B_Address,A_Address);
                    newIndex = B_Address;
                    alreadyArgsList=true;
                }
                else if(B_Type==ByteCodeInstruction::TypeHeapFragment){
                    ErrorRegistry::logError("Unwritten Code:list operator on non-array non-arg and heap fragment.");
                }
                A_Type = this->heap->data->getByte(A_Address);
                B_Type = this->heap->data->getByte(B_Address);
                if(alreadyArgsList==false){
                    this->heap->constructNewArgsList(A_Address,B_Address);
                }
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newIndex);
                }
                else{
                    this->heap->setRETBuffer(newIndex);
                }
            }
        }
    }
    else if(QString::compare(_operator,"$")==0){
        B_Address = this->heap->completeDeref(B_Address);
        B_Type = this->heap->data->getByte(B_Address);
        if(A_Address==Architecture::NativeLocation){
            //////this->p->debuggerOut("invocation on native...",true);
            if(B_Address==Architecture::NULLLocation){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Address==Architecture::UnsetLocation){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Address==Architecture::TrueLocation){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Address==Architecture::FalseLocation){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Address==Architecture::NativeLocation){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Type==ByteCodeInstruction::TypeArgsListType){
                this->debuggerOut("Executing native with argslist:"+this->heap->objectToString(B_Address,0));
                this->p->nru->execute(B_Address);
            }
            else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Type==ByteCodeInstruction::TypeObjectType){
                int argAddress = this->heap->data->size();
                this->heap->constructNewArgsList(B_Address);
                this->p->nru->execute(argAddress);
            }
            else if(B_Type==ByteCodeInstruction::TypeConstObjectType){
            }

            else if(B_Type==ByteCodeInstruction::TypeHeapFragment){
            }
            else{
                ErrorRegistry::logError("Native Call - B is an unrecognised type:"+QString::number(B_Type)+", on line:"+QString::number(this->p->lineNumber)+" with operator:"+_operator);
            }
            if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                this->heap->setRPNBuffer(buffer,retPointer);
            }
            else{
            }
        }
        else if(A_Address==Architecture::NULLLocation){
            ErrorRegistry::logError("Invocation on null, on line "+QString::number(this->p->lineNumber));
        }
        else if(A_Address==Architecture::UnsetLocation){
            ErrorRegistry::logError("Invocation on unset.");
        }
        else if(A_Address==Architecture::TrueLocation){
            ErrorRegistry::logError("Invocation on true.");
        }
        else if(A_Address==Architecture::FalseLocation){
            ErrorRegistry::logError("Invocation on false.");
        }
        else if(A_Type==ByteCodeInstruction::TypeLambdaType){
            //////this->p->debuggerOut("Invocation on lambda.",true);
            int valAddress = A_Address;
            char type = this->heap->data->getByte(valAddress);
            if(type==ByteCodeInstruction::TypeLambdaType){
                B_Type = this->heap->data->getByte(B_Address);
                if(B_Type!=ByteCodeInstruction::TypeArgsListType){
                    int new_B_Address = this->heap->data->size();
                    this->heap->constructNewArgsList(B_Address);
                    B_Address = new_B_Address;
                }
                int _argsAddress = B_Address;
                PByteArray* rpnStore = new PByteArray();
                rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                int i=0;
                while(i<rpnStore->size()){
                    rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
                    i++;
                }
                int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
                this->p->executeLambda(valAddress,_argsAddress,Architecture::NULLLocation);
                i=0;
                while(i<rpnStore->size()){
                    this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                    i++;
                }
                this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
                this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
                int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                //////this->p->debuggerOut("Ret Pointer points to:"+QString::number(retPointer),true);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                   this->heap->setRPNBuffer(buffer,retPointer);
                }
                else{
                    ////////////////this->p->debuggerOut("Not Setting RPN - return on return, from within ERU....",true);
                }
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            ErrorRegistry::logError("Unwritten Code: Invocation on const-object");
        }
        else if(A_Type==ByteCodeInstruction::TypeObjectType){
            int contextAddress = A_Address;
            int valAddress = this->heap->propertyValueAddressLookUp(A_Address,"__invoke",false);
            ////////////////this->p->debuggerOut("which has a value address of:"+QString::number(valAddress),true);
            valAddress = this->heap->decodeRawIntFrom(valAddress);
            ////////////////this->p->debuggerOut("which in turn is just a pointer to:"+QString::number(valAddress),true);
            unsigned char type = this->heap->data->getByte(valAddress);
            if(type==ByteCodeInstruction::TypeLambdaType){
                ////////////////this->p->debuggerOut("Pointer to Lambda",true);
                B_Type = this->heap->data->getByte(B_Address);
                if(B_Type!=ByteCodeInstruction::TypeArgsListType){
                    if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                        B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeKeyType){
                        B_Address = this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(B_Address),B_Address);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    int new_B_Address = this->heap->data->size();
                    this->heap->constructNewArgsList(B_Address);
                    B_Address = new_B_Address;
                }
                int _argsAddress = B_Address;
                PByteArray* rpnStore = new PByteArray();
                rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                int i=0;
                while(i<rpnStore->size()){
                    rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
                    i++;
                }
                int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
                this->p->executeLambda(valAddress,_argsAddress,contextAddress);
                i=0;
                while(i<rpnStore->size()){
                    this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                    i++;
                }
                this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
                this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                   int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                   this->heap->setRPNBuffer(buffer,retPointer);
                }
            }
            else{
                ErrorRegistry::logError("Cannot invoke object, __invoke method not found...");
            }
        }
        else if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
            int contextAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutterToContext);
            int valAddress = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
            valAddress = this->heap->decodeRawIntFrom(valAddress);
            char type = this->heap->data->getByte(valAddress);
            if(type==ByteCodeInstruction::TypeLambdaType){
                B_Type = this->heap->data->getByte(B_Address);
                if(B_Type!=ByteCodeInstruction::TypeArgsListType){
                    if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                        B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeKeyType){
                        B_Address = this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(B_Address),B_Address);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    int new_B_Address = this->heap->data->size();
                    this->heap->constructNewArgsList(B_Address);
                    B_Address = new_B_Address;
                }
                int _argsAddress = B_Address;
                PByteArray* rpnStore = new PByteArray();
                rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                int i=0;
                while(i<rpnStore->size()){
                    rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
                    i++;
                }
                int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
                this->p->executeLambda(valAddress,_argsAddress,contextAddress);
                i=0;
                while(i<rpnStore->size()){
                    this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                    i++;
                }
                this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
                this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                   int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                   this->heap->setRPNBuffer(buffer,retPointer);
                }
            }
            else{
                ErrorRegistry::logError("Unwritten code: invocation on propertykey!=>lambda, type:"+QString::number(type)+" given, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeKeyType){
            QString aString = this->heap->decodeStringFrom(A_Address);
            int valAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
            valAddress = this->heap->decodeRawIntFrom(valAddress);
            char type = this->heap->data->getByte(valAddress);
            if(type==ByteCodeInstruction::TypeLambdaType){
                if(B_Type!=ByteCodeInstruction::TypeArgsListType){
                    if(B_Address>Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                            B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                            B_Address = this->heap->decodeRawIntFrom(B_Address);
                            B_Type = this->heap->data->getByte(B_Address);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeKeyType){
                            B_Address = this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(B_Address),B_Address);
                            B_Address = this->heap->decodeRawIntFrom(B_Address);
                            B_Type = this->heap->data->getByte(B_Address);
                        }
                    }
                    int new_B_Address = this->heap->data->size();
                    this->heap->constructNewArgsList(B_Address);
                    B_Address = new_B_Address;
                }
                int _argsAddress = B_Address;
                PByteArray* rpnStore = new PByteArray();
                rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                int i=0;
                while(i<rpnStore->size()){
                    rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
                    i++;
                }
                int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
                this->p->executeLambda(valAddress,_argsAddress,Architecture::NULLLocation);
                i=0;
                while(i<rpnStore->size()){
                    this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                    i++;
                }
                this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
                this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                    this->heap->setRPNBuffer(buffer,retPointer);
                }
                else{
                    ErrorRegistry::logError("Unwritten Code: Return invocation on key=>lambda");
                }

            }
            else if(type==ByteCodeInstruction::TypeObjectType){
                int contextAddress = A_Address;
                int valAddress = this->heap->propertyValueAddressLookUp(A_Address,"__invoke",false);
                valAddress = this->heap->decodeRawIntFrom(valAddress);
                char type = this->heap->data->getByte(valAddress);
                if(type==ByteCodeInstruction::TypeLambdaType){
                    B_Type = this->heap->data->getByte(B_Address);
                    if(B_Type!=ByteCodeInstruction::TypeArgsListType){
                        if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                            B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                            B_Address = this->heap->decodeRawIntFrom(B_Address);
                            B_Type = this->heap->data->getByte(B_Address);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeKeyType){
                            B_Address = this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(B_Address),B_Address);
                            B_Address = this->heap->decodeRawIntFrom(B_Address);
                            B_Type = this->heap->data->getByte(B_Address);
                        }
                        int new_B_Address = this->heap->data->size();
                        this->heap->constructNewArgsList(B_Address);
                        B_Address = new_B_Address;
                    }
                    int _argsAddress = B_Address;
                    PByteArray* rpnStore = new PByteArray();
                    rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                    int i=0;
                    while(i<rpnStore->size()){
                        rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
                        i++;
                    }
                    int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                    int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
                    this->p->executeLambda(valAddress,_argsAddress,contextAddress);
                    i=0;
                    while(i<rpnStore->size()){
                        this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                        i++;
                    }
                    this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
                    this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       int retPointer = this->heap->decodeRawIntFrom(Architecture::ReturnAddress);
                       this->heap->setRPNBuffer(buffer,retPointer);
                    }
                }
            }
            else{
                ErrorRegistry::logError("Error - code: invocation on key("+aString+")!=>lambda, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            if(A_Type==ByteCodeInstruction::TypeConstStringType || A_Type==ByteCodeInstruction::TypeRunTimeStringType){
                ErrorRegistry::logError("Attempted invocation on string \""+this->heap->decodeStringFrom(A_Address)+"\", on line "+QString::number(this->p->lineNumber)+", instruction code:"+QString::number((instructionPointer - (this->heap->initThreshold+1))/13)+" with righthand type:"+QString::number(B_Type)+" as string:"+this->heap->decodeStringFrom(B_Address));
            }
            else{
                ErrorRegistry::logError("Attempted invocation on invalid type:"+QString::number(A_Type)+", on line "+QString::number(this->p->lineNumber));
            }
        }
    }
    else if(QString::compare(_operator,".")==0){
        if(A_Address==Architecture::NULLLocation){
            ErrorRegistry::logError("Cannot perform property look up on null.");
        }
        else if(A_Address==Architecture::UnsetLocation){
            ErrorRegistry::logError("Cannot perform property look up on unset.");
        }
        else if(A_Address==Architecture::TrueLocation){
            ErrorRegistry::logError("Cannot perform property look up on true.");
        }
        else if(A_Address==Architecture::FalseLocation){
            ErrorRegistry::logError("Cannot perform property look up on false.");
        }
        else if(A_Address==Architecture::NativeLocation){
            ErrorRegistry::logError("Cannot perform property look up on native.");
        }
        else if(A_Type==ByteCodeInstruction::TypeKeyType){
            QString aString = this->heap->decodeStringFrom(A_Address);
            int aAddress = this->heap->valueAddressInStackFrames(aString,A_Address);
            aAddress = this->heap->decodeRawIntFrom(aAddress);
            char aType = this->heap->data->getByte(aAddress);
            while(aType==ByteCodeInstruction::TypeHeapFragment){
                aAddress = this->heap->decodeRawIntFrom(aAddress+Architecture::TypeHeapFragmentGutter);
                aType = this->heap->data->getByte(aAddress);
            }
            if(B_Type==ByteCodeInstruction::TypeKeyType){
                QString bString = this->heap->decodeStringFrom(B_Address);
                if(aType==ByteCodeInstruction::TypeObjectType){
                    int propAddress = this->heap->propertyValueAddressLookUp(aAddress,bString,B_Address);
                    PByteArray* propertyKey = new PByteArray();
                    propertyKey->resize(Architecture::TypePropertyKeySize);
                    propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,aAddress);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                    int newDataAddress = this->heap->data->size();
                    this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,newDataAddress);
                    }
                    else{
                       this->heap->setRETBuffer(newDataAddress);
                    }
                }
                else if(aType==ByteCodeInstruction::TypeConstObjectType){
                    ErrorRegistry::logError("Unwritten code: property look up on key=>const-object, on line:"+QString::number(this->p->lineNumber));
                }
                else if(aType==ByteCodeInstruction::TypeOpenArrayType){
                    int arrayProtoReference = this->heap->decodeRawIntFrom(Architecture::ArrayLocation);
                    if(arrayProtoReference==Architecture::UnsetLocation || arrayProtoReference==Architecture::NULLLocation){
                        ErrorRegistry::logError("Error! Attempted property look up on Array, array.prototype == null || unset. Did you mean to use native('Register.prototype','Array',object); ? Or maybe include 'std.proto';");
                    }
                    else{
                        int propAddress = this->heap->propertyValueAddressLookUp(arrayProtoReference,bString,B_Address);
                        PByteArray* propertyKey = new PByteArray();
                        propertyKey->resize(Architecture::TypePropertyKeySize);
                        propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                        propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,aAddress);
                        propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                        int newDataAddress = this->heap->data->size();
                        this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                        if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                           this->heap->setRPNBuffer(buffer,newDataAddress);
                        }
                        else{
                           this->heap->setRETBuffer(newDataAddress);
                        }
                    }
                }
                else if(aType==ByteCodeInstruction::TypeLambdaType){
                    int propAddress = this->heap->propertyValueAddressLookUp(this->heap->decodeRawIntFrom(Architecture::LambdaLocation),bString,B_Address);
                    PByteArray* propertyKey = new PByteArray();
                    propertyKey->resize(Architecture::TypePropertyKeySize);
                    propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,aAddress);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                    int newDataAddress = this->heap->data->size();
                    this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,newDataAddress);
                    }
                    else{
                       this->heap->setRETBuffer(newDataAddress);
                    }
                }
                else{
                    ErrorRegistry::logError("Unwritten code: property look up, on line:"+QString::number(this->p->lineNumber));
                }
            }
            else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                ErrorRegistry::logError("Unwritten code: property look up on String, on line:"+QString::number(this->p->lineNumber));
            }
            else{
                ErrorRegistry::logError("Unwritten code: property look up on type:"+QString::number(B_Type)+" at "+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeConstStringType || A_Type==ByteCodeInstruction::TypeRunTimeStringType){
        }
        else if(A_Type==ByteCodeInstruction::TypeConstObjectType){
            if(B_Type==ByteCodeInstruction::TypeKeyType){
                QString bString = this->heap->decodeStringFrom(B_Address);
                int propAddress = this->heap->propertyValueAddressLookUp(A_Address,bString,B_Address);
                PByteArray* propertyKey = new PByteArray();
                propertyKey->resize(Architecture::TypePropertyKeySize);
                propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,A_Address);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                int newDataAddress = this->heap->data->size();
                this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newDataAddress);
                }
                else{
                    this->heap->setRETBuffer(propAddress);
                }
            }
            else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                ErrorRegistry::logError("Unwritten code: property look up on string, on line:"+QString::number(this->p->lineNumber));
            }
            else{
                ErrorRegistry::logError("Unwritten code: property look up, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeObjectType){
            if(B_Type==ByteCodeInstruction::TypeKeyType){
                QString bString = this->heap->decodeStringFrom(B_Address);
                int propAddress = this->heap->propertyValueAddressLookUp(A_Address,bString,B_Address);
                //////////////this->p->debuggerOut("Creating property key for :"+bString+" with propaddress: "+QString::number(propAddress)+" on object:"+this->p->heap->objectToString(A_Address,0),true);
                PByteArray* propertyKey = new PByteArray();
                propertyKey->resize(Architecture::TypePropertyKeySize);
                propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,A_Address);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                int newDataAddress = this->heap->data->size();
                this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newDataAddress);
                }
                else{
                    this->heap->setRETBuffer(this->heap->decodeRawIntFrom(propAddress));
                }
            }
            else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                ErrorRegistry::logError("Unwritten code: property look up, on line:"+QString::number(this->p->lineNumber));
            }
            else{
                ErrorRegistry::logError("Unwritten code: property look up, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeOpenArrayType){
            if(B_Type==ByteCodeInstruction::TypeKeyType){
                QString bString = this->heap->decodeStringFrom(B_Address);
                int arrayProtoReference = this->heap->decodeRawIntFrom(Architecture::ArrayLocation);
                if(arrayProtoReference==Architecture::UnsetLocation || arrayProtoReference==Architecture::NULLLocation){
                    ErrorRegistry::logError("Error! Attempted property look up on Array, array.prototype == null || unset. Did you mean to use native('Register.prototype','Array',object); ? Or maybe include 'std.proto';");
                }
                else{
                    int propAddress = this->heap->propertyValueAddressLookUp(arrayProtoReference,bString,B_Address);
                    PByteArray* propertyKey = new PByteArray();
                    propertyKey->resize(Architecture::TypePropertyKeySize);
                    propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,A_Address);
                    propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                    int newDataAddress = this->heap->data->size();
                    this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                       this->heap->setRPNBuffer(buffer,newDataAddress);
                    }
                    else{
                       this->heap->setRETBuffer(newDataAddress);
                    }
                }
            }
            else{
                ErrorRegistry::logError("Property look up on array with non key, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType || A_Type==ByteCodeInstruction::TypeConstIntegerType){
            ErrorRegistry::logError("Unwritten code: property look up on integer, on line:"+QString::number(this->p->lineNumber));
        }
        else if(A_Type==ByteCodeInstruction::TypeConstFloatType || A_Type==ByteCodeInstruction::TypeRunTimeFloatType){
            ErrorRegistry::logError("Unwritten code: property look up on float, on line:"+QString::number(this->p->lineNumber));
        }
        else if(A_Type==ByteCodeInstruction::TypeLambdaType){
            if(B_Type==ByteCodeInstruction::TypeKeyType){
                QString bString = this->heap->decodeStringFrom(B_Address);
                int propAddress = this->heap->propertyValueAddressLookUp(this->heap->decodeRawIntFrom(Architecture::LambdaLocation),bString,B_Address);
                PByteArray* propertyKey = new PByteArray();
                propertyKey->resize(Architecture::TypePropertyKeySize);
                propertyKey->setByte(0,ByteCodeInstruction::TypePropertyKeyType);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutterToContext,A_Address);
                propertyKey->encodeRawIntegerAt(Architecture::TypePropertyKeyGutter,propAddress);
                int newDataAddress = this->heap->data->size();
                this->heap->copyInTo(newDataAddress,propertyKey,Architecture::TypePropertyKeySize);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                   this->heap->setRPNBuffer(buffer,newDataAddress);
                }
                else{
                   this->heap->setRETBuffer(newDataAddress);
                }
            }
            else{
                ErrorRegistry::logError("Unwritten code: property look up n lambda, with no key, on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            ErrorRegistry::logError("Unwritten code: property look up on type:"+QString::number(A_Type)+" at address:#"+QString::number(A_Address)+", on line:"+QString::number(this->p->lineNumber));
        }
    }
    else{//array look up operator
        if(A_Type!=ByteCodeInstruction::TypeArgsListType && A_Type!=ByteCodeInstruction::TypeOpenArrayType && A_Type!=ByteCodeInstruction::TypeClosedArrayType){
            if(A_Type==ByteCodeInstruction::TypeObjectType){
                if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int valueAddress = this->heap->propertyValueAddressLookUp(A_Address,bString,B_Address);
                    PByteArray* indexKey = new PByteArray();
                    indexKey->resize(Architecture::TypeArrayIndexSize);
                    indexKey->setByte(0,ByteCodeInstruction::TypeArrayIndexType);
                    indexKey->encodeRawIntegerAt(Architecture::TypeArrayIndexGutterToContext,A_Address);
                    indexKey->encodeRawIntegerAt(Architecture::TypeArrayIndexGutter,valueAddress);
                    valueAddress = this->heap->data->size();
                    this->heap->copyToNew(indexKey,Architecture::TypePropertyKeySize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        this->heap->setRPNBuffer(buffer,valueAddress);
                    }
                    else{
                        this->heap->setRETBuffer(valueAddress);
                    }
                }
                else{
                    ErrorRegistry::logError("Attempted look up on object with non-string, type:"+QString::number(B_Type)+", given on line:"+QString::number(this->p->lineNumber));
                }
            }
            else{
                ErrorRegistry::logError("Attempted look up on non-array, non-arguments list on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType || B_Type==ByteCodeInstruction::TypeConstIntegerType){
                int GUTTER;
                if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    GUTTER = Architecture::TypeRunTimeIntegerGutter;
                }
                else{
                    GUTTER = Architecture::TypeConstIntegerGutter;
                }
                int BVal = this->heap->decodeRawIntFrom(B_Address+GUTTER);
                int valueAddress = this->heap->getValueAddressInArray(A_Address,BVal);
                PByteArray* indexKey = new PByteArray();
                indexKey->resize(Architecture::TypeArrayIndexSize);
                indexKey->setByte(0,ByteCodeInstruction::TypeArrayIndexType);
                indexKey->encodeRawIntegerAt(Architecture::TypeArrayIndexGutterToContext,A_Address);
                indexKey->encodeRawIntegerAt(Architecture::TypeArrayIndexGutter,valueAddress);
                valueAddress = this->heap->data->size();
                this->heap->copyToNew(indexKey,Architecture::TypeArrayIndexSize);
                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,valueAddress);
                }
                else{
                    this->heap->setRETBuffer(valueAddress);
                }
            }
            else{
                ErrorRegistry::logError("Could not perform Array Look Up Operator on Array or ArgsList - righthand operand is not an integer, type:"+QString::number(B_Type)+", at address:#"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
            }
        }
    }
}
