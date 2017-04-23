#include "alu.h"

#include "bytecodeinstruction.h"
#include "architecture.h"

#include "errorregistry.h"
#include "pbytearray.h"
#include "heapstore.h"

ALU::ALU(Interpreter* interpreter){
    this->p = interpreter;
    this->heap = this->p->heap;
}

void ALU::debuggerOut(QString out){
    //////////////this->p->debuggerOut(out);
}

void ALU::logicalEval(int A_Address,int B_Address,QString _operator,unsigned char buffer){
    unsigned char A_Type = this->heap->data->getByte(A_Address);
    unsigned char B_Type = this->heap->data->getByte(B_Address);
    bool aVal = false;
    bool bVal = true;
    while(A_Address!=Architecture::TrueLocation && A_Address!=Architecture::FalseLocation){
        if(A_Address==Architecture::NULLLocation){
            A_Address = Architecture::FalseLocation;
        }
        else if(A_Address==Architecture::UnsetLocation){
            A_Address = Architecture::FalseLocation;
        }
        else{
            A_Address = this->heap->completeDeref(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
            if(A_Type==ByteCodeInstruction::TypeConstIntegerType || A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                int GUTTER;
                if(A_Type==ByteCodeInstruction::TypeConstIntegerType){
                    GUTTER = Architecture::TypeConstIntegerGutter;
                }
                else{
                    GUTTER = Architecture::TypeRunTimeIntegerGutter;
                }
                int x = this->heap->decodeRawIntFrom(A_Address+GUTTER);
                if(x>0){
                    A_Address = Architecture::TrueLocation;
                }
                else{
                    A_Address = Architecture::FalseLocation;
                }
            }
            else if(A_Type==ByteCodeInstruction::TypeConstStringType || A_Type==ByteCodeInstruction::TypeRunTimeStringType){
                QString aString = this->heap->decodeStringFrom(A_Address);
                if(QString::compare(aString,"unset")==0){
                    A_Address = Architecture::FalseLocation;
                }
                else if(QString::compare(aString,"NULL")==0){
                    A_Address = Architecture::FalseLocation;
                }
                else if(QString::compare(aString,"0")==0){
                    A_Address = Architecture::FalseLocation;
                }
                else{
                    if(aString.length()>0){
                        A_Address = Architecture::TrueLocation;
                    }
                    else{
                        A_Address = Architecture::FalseLocation;
                    }
                }
            }
            else{
                A_Address = Architecture::FalseLocation;
            }
        }
    }
    if(A_Address==Architecture::TrueLocation){
        aVal=true;
    }
    A_Address = B_Address;
    A_Type = B_Type;
    if(A_Address==Architecture::NULLLocation){
        A_Address = Architecture::FalseLocation;
    }
    else if(A_Address==Architecture::UnsetLocation){
        A_Address = Architecture::FalseLocation;
    }
    else{
        A_Address = this->heap->completeDeref(A_Address);
        A_Type = this->heap->data->getByte(A_Address);
        if(A_Type==ByteCodeInstruction::TypeConstIntegerType || A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
           int GUTTER;
           if(A_Type==ByteCodeInstruction::TypeConstIntegerType){
               GUTTER = Architecture::TypeConstIntegerGutter;
           }
           else{
               GUTTER = Architecture::TypeRunTimeIntegerGutter;
           }
            int x = this->heap->decodeRawIntFrom(A_Address+GUTTER);
            if(x>0){
                A_Address = Architecture::TrueLocation;
            }
            else{
                A_Address = Architecture::FalseLocation;
            }
        }
        else if(A_Type==ByteCodeInstruction::TypeConstStringType || A_Type==ByteCodeInstruction::TypeRunTimeStringType){
            QString aString = this->heap->decodeStringFrom(A_Address);
            if(QString::compare(aString,"unset")==0){
                A_Address = Architecture::FalseLocation;
            }
            else if(QString::compare(aString,"NULL")==0){
                A_Address = Architecture::FalseLocation;
            }
            else if(QString::compare(aString,"0")==0){
                A_Address = Architecture::FalseLocation;
            }
            else{
                if(aString.length()>0){
                    A_Address = Architecture::TrueLocation;
                }
                else{
                    A_Address = Architecture::FalseLocation;
                }
            }
        }
        else{
            A_Address = Architecture::FalseLocation;
        }
    }
    if(A_Address==Architecture::TrueLocation){
        bVal=true;
    }
    if(QString::compare(_operator,"&&")==0){
        ////////////////this->p->debuggerOut("Evaluating "+QString::number(aVal)+" && "+QString::number(bVal)+", for line:"+QString::number(this->p->lineNumber),true);
        if(aVal && bVal){
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
        }
        else{
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
        }
    }
    else if(QString::compare(_operator,"||")==0){
        ////////////this->p->debuggerOutEvaluating "+QString::number(aVal)+" || "+QString::number(bVal),true);
        if(aVal || bVal){
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
        }
        else{
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
        }
    }
    else if(QString::compare(_operator,"!")==0){
        ////////////this->p->debuggerOutEvaluating !"+QString::number(bVal),true);
        if(!bVal){
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
        }
        else{
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
        }
    }
    else{
        ErrorRegistry::logError("Unwritten Code, logical operator:aVal:"+QString::number(aVal)+_operator+"bVal:"+QString::number(bVal)+", on line:"+QString::number(this->p->lineNumber));
    }
}

void ALU::eval(int A_Address,int B_Address,QString _operator,unsigned char buffer){

    //////////////this->p->debuggerOut("Operator for ALU:"+_operator,true);

    unsigned char A_Type = this->heap->data->getByte(A_Address);
    unsigned char B_Type = this->heap->data->getByte(B_Address);
    if(QString::compare(_operator,"&&")==0 || QString::compare(_operator,"||")==0 || QString::compare(_operator,"!")==0){
        this->logicalEval(A_Address,B_Address,_operator,buffer);
    }
    else if(QString::compare(_operator,"-")==0 ||
            QString::compare(_operator,"/")==0 ||
            QString::compare(_operator,"*")==0 ||
            QString::compare(_operator,"%")==0 ||
            QString::compare(_operator,"+")==0){
        if(B_Address==Architecture::NativeLocation){
            ErrorRegistry::logError("Cannot perform operation: "+_operator+", righthand is native on line "+QString::number(this->p->lineNumber));
        }
        else {
            A_Address = this->heap->completeDeref(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
            B_Address = this->heap->completeDeref(B_Address);
            B_Type = this->heap->data->getByte(B_Address);
        }
        if(QString::compare(_operator,"+")==0){
            if(A_Address==Architecture::NULLLocation){

                QString aString = "NULL";
                //////////////this->p->debuggerOut("A is:"+aString,true);
                if(B_Address==Architecture::NULLLocation){
                    ////this->debuggerOut("B is null.");
                    aString+="NULL";
                }
                else if(B_Address==Architecture::UnsetLocation){
                    ////this->debuggerOut("B is unset.");
                    aString+="unset";
                }
                else if(B_Address==Architecture::TrueLocation){
                    ////this->debuggerOut("B is true.");
                    aString+="true";
                }
                else if(B_Address==Architecture::FalseLocation){
                    ////this->debuggerOut("B is false.");
                    aString+="false";
                }
                else if(B_Address==Architecture::NativeLocation){
  //                  ////this->debuggerOut("B is native.");
    //                aString+="false";
                      ErrorRegistry::logError("Cannot perform addition on native.");
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    ////this->debuggerOut("B is a string.");
                    aString+=this->heap->decodeStringFrom(B_Address);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    ////this->debuggerOut("B is an integer.");
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    //////////////this->p->debuggerOut("B is an integer.",true);
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    ////this->debuggerOut("B is a key.");
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddress = this->heap->valueAddressInStackFrames(bString,B_Address);
                    B_Address = this->heap->decodeRawIntFrom(bValAddress);
                    if(B_Address==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(B_Address==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(B_Address==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        ErrorRegistry::logError("Error - cannot perform addition on native.");
                    }
                    else{
                        B_Type = this->heap->data->getByte(B_Address);
                        if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                            aString+=this->heap->decodeStringFrom(B_Address);
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else{
                            ErrorRegistry::logError("Error on addition - righthand is a property key pointing to an unknown type.");
                        }
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddress = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    if(bValAddress==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(bValAddress==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(bValAddress==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(bValAddress==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(bValAddress==Architecture::NativeLocation){
                        ErrorRegistry::logError("Cannot perform addition on Native.");
                    }
                    else{
                        unsigned char bType = this->heap->data->getByte(bValAddress);
                        if(bType==ByteCodeInstruction::TypeConstStringType || bType==ByteCodeInstruction::TypeRunTimeStringType){
                            QString bString = this->heap->decodeStringFrom(bValAddress);
                            aString+=bString;
                        }
                        else if(bType==ByteCodeInstruction::TypeConstIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeConstIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else if(bType==ByteCodeInstruction::TypeRunTimeIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeRunTimeIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else{
                            ErrorRegistry::logError("Error during addition: invalid types. A is a string, B is a property key pointing to uknown type:"+QString::number(bType));
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Error during addition: invalid righthand side on string, type:"+QString::number(B_Type)+" given at #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                }
                PByteArray* newString = HeapStore::encodeAsKey(aString);
                newString->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
                int newIndex = this->heap->data->size();
                this->heap->copyToNew(newString,newString->size());

                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newIndex);
                }
                else if(buffer==Architecture::ReturnBufferCode){
                    this->heap->setRETBuffer(newIndex);
                }

            }
            else if(A_Address==Architecture::UnsetLocation){
                QString aString = "unset";
                ////this->debuggerOut("A is:"+aString);
                if(B_Address==Architecture::NULLLocation){
                    ////this->debuggerOut("B is null.");
                    aString+="NULL";
                }
                else if(B_Address==Architecture::UnsetLocation){
                    ////this->debuggerOut("B is unset.");
                    aString+="unset";
                }
                else if(B_Address==Architecture::TrueLocation){
                    ////this->debuggerOut("B is true.");
                    aString+="true";
                }
                else if(B_Address==Architecture::FalseLocation){
                    ////this->debuggerOut("B is false.");
                    aString+="false";
                }
                else if(B_Address==Architecture::NativeLocation){
  //                  ////this->debuggerOut("B is native.");
    //                aString+="false";
                      ErrorRegistry::logError("Cannot perform addition on native.");
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    ////this->debuggerOut("B is a string.");
                    aString+=this->heap->decodeStringFrom(B_Address);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    ////this->debuggerOut("B is an integer.");
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    ////this->debuggerOut("B is an integer.");
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    ////this->debuggerOut("B is a key.");
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddress = this->heap->valueAddressInStackFrames(bString,B_Address);
                    B_Address = this->heap->decodeRawIntFrom(bValAddress);
                    if(B_Address==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(B_Address==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(B_Address==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        ErrorRegistry::logError("Error - cannot perform addition on native.");
                    }
                    else{
                        B_Type = this->heap->data->getByte(B_Address);
                        if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                            aString+=this->heap->decodeStringFrom(B_Address);
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else{
                            ErrorRegistry::logError("Error on addition - righthand is a property key pointing to an unknown type.");
                        }
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddress = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    if(bValAddress==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(bValAddress==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(bValAddress==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(bValAddress==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(bValAddress==Architecture::NativeLocation){
                        ErrorRegistry::logError("Cannot perform addition on Native.");
                    }
                    else{
                        unsigned char bType = this->heap->data->getByte(bValAddress);
                        if(bType==ByteCodeInstruction::TypeConstStringType || bType==ByteCodeInstruction::TypeRunTimeStringType){
                            QString bString = this->heap->decodeStringFrom(bValAddress);
                            aString+=bString;
                        }
                        else if(bType==ByteCodeInstruction::TypeRunTimeIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeRunTimeIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else if(bType==ByteCodeInstruction::TypeConstIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeConstIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else{
                            ErrorRegistry::logError("Error during addition: invalid types. A is a string, B is a property key pointing to uknown type:"+QString::number(bType));
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Error during addition: invalid righthand side on string, type:"+QString::number(B_Type)+" given at #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                }
                PByteArray* newString = HeapStore::encodeAsKey(aString);
                newString->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
                int newIndex = this->heap->data->size();
                this->heap->copyToNew(newString,newString->size());

                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newIndex);
                }
                else if(buffer==Architecture::ReturnBufferCode){
                    this->heap->setRETBuffer(newIndex);
                }

            }
            else if(A_Address==Architecture::TrueLocation){

                QString aString = "true";
                ////this->debuggerOut("A is:"+aString);
                if(B_Address==Architecture::NULLLocation){
                    ////this->debuggerOut("B is null.");
                    aString+="NULL";
                }
                else if(B_Address==Architecture::UnsetLocation){
                    ////this->debuggerOut("B is unset.");
                    aString+="unset";
                }
                else if(B_Address==Architecture::TrueLocation){
                    ////this->debuggerOut("B is true.");
                    aString+="true";
                }
                else if(B_Address==Architecture::FalseLocation){
                    ////this->debuggerOut("B is false.");
                    aString+="false";
                }
                else if(B_Address==Architecture::NativeLocation){
  //                  ////this->debuggerOut("B is native.");
    //                aString+="false";
                      ErrorRegistry::logError("Cannot perform addition on native.");
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    ////this->debuggerOut("B is a string.");
                    aString+=this->heap->decodeStringFrom(B_Address);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    ////this->debuggerOut("B is an integer.");
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    ////this->debuggerOut("B is an integer.");
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                    ////this->debuggerOut("string is now:"+aString);
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    ////this->debuggerOut("B is a key.");
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddress = this->heap->valueAddressInStackFrames(bString,B_Address);
                    B_Address = this->heap->decodeRawIntFrom(bValAddress);
                    if(B_Address==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(B_Address==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(B_Address==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        ErrorRegistry::logError("Error - cannot perform addition on native.");
                    }
                    else{
                        B_Type = this->heap->data->getByte(B_Address);
                        if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else if(B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                            aString+=this->heap->decodeStringFrom(B_Address);
                            ////this->debuggerOut("string is now:"+aString);
                        }
                        else{
                            ErrorRegistry::logError("Error on addition - righthand is a property key pointing to an unknown type.");
                        }
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddress = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    if(bValAddress==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(bValAddress==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(bValAddress==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(bValAddress==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(bValAddress==Architecture::NativeLocation){
                        ErrorRegistry::logError("Cannot perform addition on Native.");
                    }
                    else{
                        unsigned char bType = this->heap->data->getByte(bValAddress);
                        if(bType==ByteCodeInstruction::TypeRunTimeStringType){
                            QString bString = this->heap->decodeStringFrom(bValAddress);
                            aString+=bString;
                        }
                        else if(bType==ByteCodeInstruction::TypeRunTimeIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeRunTimeIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else{
                            ErrorRegistry::logError("Error during addition: invalid types. A is a string, B is a property key pointing to uknown type:"+QString::number(bType));
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Error during addition: invalid righthand side on string, type:"+QString::number(B_Type)+" given at #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                }
                PByteArray* newString = HeapStore::encodeAsKey(aString);
                newString->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
                int newIndex = this->heap->data->size();
                this->heap->copyToNew(newString,newString->size());

                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newIndex);
                }
                else if(buffer==Architecture::ReturnBufferCode){
                    this->heap->setRETBuffer(newIndex);
                }


            }
            else if(A_Address==Architecture::FalseLocation){
                QString aString = "false";
                if(B_Address==Architecture::NULLLocation){
                    aString+="NULL";
                }
                else if(B_Address==Architecture::UnsetLocation){
                    aString+="unset";
                }
                else if(B_Address==Architecture::TrueLocation){
                    aString+="true";
                }
                else if(B_Address==Architecture::FalseLocation){
                    aString+="false";
                }
                else if(B_Address==Architecture::NativeLocation){
                      ErrorRegistry::logError("Cannot perform addition on native.");
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    aString+=this->heap->decodeStringFrom(B_Address);
                }
                else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                }
                else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                }
                else if(B_Type==ByteCodeInstruction::TypeKeyType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    int bValAddress = this->heap->valueAddressInStackFrames(bString,B_Address);
                    B_Address = this->heap->decodeRawIntFrom(bValAddress);
                    if(B_Address==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(B_Address==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(B_Address==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(B_Address==Architecture::FalseLocation){
                        ErrorRegistry::logError("Error - cannot perform addition on native.");
                    }
                    else{
                        B_Type = this->heap->data->getByte(B_Address);
                        if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                            aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                        }
                        else if(B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                            aString+=this->heap->decodeStringFrom(B_Address);
                        }
                        else{
                            ErrorRegistry::logError("Error on addition - righthand is a property key pointing to an unknown type.");
                        }
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                    int bValAddress = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                    bValAddress = this->heap->decodeRawIntFrom(bValAddress);
                    if(bValAddress==Architecture::NULLLocation){
                        aString+="NULL";
                    }
                    else if(bValAddress==Architecture::UnsetLocation){
                        aString+="unset";
                    }
                    else if(bValAddress==Architecture::TrueLocation){
                        aString+="true";
                    }
                    else if(bValAddress==Architecture::FalseLocation){
                        aString+="false";
                    }
                    else if(bValAddress==Architecture::NativeLocation){
                        ErrorRegistry::logError("Cannot perform addition on Native.");
                    }
                    else{
                        unsigned char bType = this->heap->data->getByte(bValAddress);
                        if(bType==ByteCodeInstruction::TypeRunTimeStringType){
                            QString bString = this->heap->decodeStringFrom(bValAddress);
                            aString+=bString;
                        }
                        else if(bType==ByteCodeInstruction::TypeRunTimeIntegerType){
                            int bString = this->heap->decodeRawIntFrom(bValAddress+Architecture::TypeRunTimeIntegerGutter);
                            aString+=QString::number(bString);
                        }
                        else{
                            ErrorRegistry::logError("Error during addition: invalid types. A is a string, B is a property key pointing to uknown type:"+QString::number(bType));
                        }
                    }
                }
                else{
                    ErrorRegistry::logError("Error during addition: invalid righthand side on string, type:"+QString::number(B_Type)+" given at #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                }
                PByteArray* newString = HeapStore::encodeAsKey(aString);
                newString->setByte(0,ByteCodeInstruction::TypeRunTimeStringType);
                int newIndex = this->heap->data->size();
                this->heap->copyToNew(newString,newString->size());

                if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                    this->heap->setRPNBuffer(buffer,newIndex);
                }
                else if(buffer==Architecture::ReturnBufferCode){
                    this->heap->setRETBuffer(newIndex);
                }

            }
            else if(A_Address<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                ErrorRegistry::logError("Error! Unexpected A Address.");
            }
            else if(A_Type==ByteCodeInstruction::TypeConstIntegerType || A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                int GUTTER;
                if(A_Type==ByteCodeInstruction::TypeConstIntegerType){
                    GUTTER =Architecture::TypeConstIntegerGutter;
                }
                else{
                    GUTTER =Architecture::TypeRunTimeIntegerGutter;
                }
                  int AVal = this->heap->decodeRawIntFrom(A_Address+GUTTER);
                  if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                      int B_GUTTER;
                      if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                          B_GUTTER =Architecture::TypeConstIntegerGutter;
                      }
                      else{
                          B_GUTTER =Architecture::TypeRunTimeIntegerGutter;
                      }
                      int BVal = this->heap->decodeRawIntFrom(B_Address+B_GUTTER);
                      int res = AVal+BVal;
                      PByteArray* resBytes = new PByteArray();
                      resBytes->encodeAsRunTimeInteger(0,res);
                      int resultAddress = this->heap->data->size();
                      this->heap->copyToNew(resBytes,Architecture::TypeRunTimeIntegerSize);
                      if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                          this->heap->setRPNBuffer(buffer,resultAddress);
                      }
                      else{
                          this->heap->setRETBuffer(resultAddress);
                      }
                  }
                  else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                      QString BVal = this->heap->decodeStringFrom(B_Address);
                      QString res = QString::number(AVal)+BVal;
                      PByteArray* resBytes = new PByteArray();
                      resBytes->encodeAsString(res,false);
                      int resultAddress = this->heap->data->size();
                      this->heap->copyToNew(resBytes,resBytes->size());
                      if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                          this->heap->setRPNBuffer(buffer,resultAddress);
                      }
                      else{
                          this->heap->setRETBuffer(resultAddress);
                      }
                  }
                  else{
                      ErrorRegistry::logError("Cannot perform addition, lefthand is integer, righthand is unhandled type"+QString::number(B_Type)+", at #"+QString::number(B_Address)+". on line "+QString::number(this->p->lineNumber));
                  }
            }
            else if(A_Type==ByteCodeInstruction::TypeRunTimeStringType || A_Type==ByteCodeInstruction::TypeConstStringType){
                  QString aString = this->heap->decodeStringFrom(A_Address);

                  //////////////this->p->debuggerOut("Addition on string:"+aString,true);

                  if(B_Address==Architecture::NULLLocation){
                      aString+="NULL";
                  }
                  else if(B_Address==Architecture::UnsetLocation){
                      aString+="unset";
                  }
                  else if(B_Address==Architecture::TrueLocation){
                      aString+="true";
                  }
                  else if(B_Address==Architecture::FalseLocation){
                      aString+="false";
                  }
                  else if(B_Address==Architecture::NativeLocation){
                        ErrorRegistry::logError("Cannot perform addition on native.");
                  }
                  else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                      aString+=this->heap->decodeStringFrom(B_Address);
                  }
                  else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                      aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                  }
                  else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                      aString+=QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                  }
                  else if(B_Type==ByteCodeInstruction::TypeOpenArrayType){
                      aString+=this->heap->arrayToString(B_Address);
                  }
                  else if(B_Type==ByteCodeInstruction::TypeObjectType){
                      aString+=this->heap->objectToString(B_Address,0,{});
                  }
                  else if(B_Type==ByteCodeInstruction::TypeConstObjectType){
                      aString+=this->heap->objectToString(B_Address,0,{});
                  }
                  else if(B_Type==ByteCodeInstruction::TypeLambdaType){
                      aString+="[lambda]";
                  }
                  else{
                      ErrorRegistry::logError("Error during addition: invalid righthand side on string, type:"+QString::number(B_Type)+" given at #"+QString::number(B_Address)+", on line:"+QString::number(this->p->lineNumber));
                  }
                  PByteArray* newString = new PByteArray();
                  newString->encodeAsString(aString,false);
                  int newIndex = this->heap->data->size();
                  this->heap->copyToNew(newString,newString->size());
                  if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                      this->heap->setRPNBuffer(buffer,newIndex);
                  }
                  else if(buffer==Architecture::ReturnBufferCode){
                      this->heap->setRETBuffer(newIndex);
                  }
            }
            else{
                ErrorRegistry::logError("Cannot perform addition, right hand is non-integer, non-string and non-key on line "+QString::number(this->p->lineNumber));
            }
        }
        else{
            if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType || A_Type==ByteCodeInstruction::TypeConstIntegerType){
                int GUTTER;
                if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    GUTTER = Architecture::TypeRunTimeIntegerGutter;
                }
                else{
                    GUTTER = Architecture::TypeConstIntegerGutter;
                }
                int AVal = this->heap->decodeRawIntFrom(A_Address+GUTTER);
                if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    int B_GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        B_GUTTER = Architecture::TypeRunTimeIntegerGutter;
                    }
                    else{
                        B_GUTTER = Architecture::TypeConstIntegerGutter;
                    }
                    int BVal = this->heap->decodeRawIntFrom(B_Address+B_GUTTER);
                    int res;
                    if(QString::compare(_operator,"-")==0){
                        res = AVal-BVal;
                    }
                    else if(QString::compare(_operator,"*")==0){
                        res = AVal*BVal;
                    }
                    else if(QString::compare(_operator,"/")==0){
                        res = AVal/BVal;
                    }
                    else{
                        res = AVal%BVal;
                    }
                    PByteArray* resBytes = new PByteArray();
                    resBytes->encodeAsRunTimeInteger(0,res);
                    int resultAddress = this->heap->data->size();
                    this->heap->copyToNew(resBytes,Architecture::TypeRunTimeIntegerSize);
                    if(buffer>=Architecture::RPNAddress && buffer<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        this->heap->setRPNBuffer(buffer,resultAddress);
                    }
                    else{
                        this->heap->setRETBuffer(resultAddress);
                    }
                }
                else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                    QString bString = this->heap->decodeStringFrom(B_Address);
                    ErrorRegistry::logError("Cannot perform operator '"+_operator+"' on string "+bString+" on line:"+QString::number(this->p->lineNumber));
                }
                else{
                    ErrorRegistry::logError("Cannot perform operator '"+_operator+"' on non-key non-integer, type:"+QString::number(B_Type)+", on line:"+QString::number(this->p->lineNumber));
                }
            }
            else{
                ErrorRegistry::logError("Cannot perform operator '"+_operator+"' on non-key non-integer, type:"+QString::number(A_Type)+", at address:#"+QString::number(A_Address)+" on line:"+QString::number(this->p->lineNumber));
            }
        }
    }
    else if(QString::compare(_operator,"<")==0 ||
              QString::compare(_operator,">")==0 ||
              QString::compare(_operator,">=")==0 ||
              QString::compare(_operator,"<=")==0 ||
              QString::compare(_operator,"==")==0 ||
            QString::compare(_operator,"!=")==0){
            A_Address = this->heap->completeDeref(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
            B_Address = this->heap->completeDeref(B_Address);
            B_Type = this->heap->data->getByte(B_Address);
            if(A_Address==Architecture::NULLLocation){
                ErrorRegistry::logError("Unwritten Code: A is NULL for operator"+_operator+", on line:"+QString::number(this->p->lineNumber));
            }
            else if(A_Address==Architecture::UnsetLocation){
                ErrorRegistry::logError("Unwritten Code: A is unset for operator"+_operator+", on line:"+QString::number(this->p->lineNumber));
            }
            else if(A_Address==Architecture::TrueLocation){
                ErrorRegistry::logError("Unwritten Code: A is true for operator"+_operator+", on line:"+QString::number(this->p->lineNumber));
            }
            else if(A_Address==Architecture::FalseLocation){
                ErrorRegistry::logError("Unwritten Code: A is false for operator"+_operator+", on line:"+QString::number(this->p->lineNumber));
            }
            else if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType || A_Type==ByteCodeInstruction::TypeConstIntegerType){
                int GUTTER;
                if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    GUTTER = Architecture::TypeRunTimeIntegerGutter;
                }
                else{
                    GUTTER = Architecture::TypeConstIntegerGutter;
                }
              int AVal = this->heap->decodeRawIntFrom(A_Address+GUTTER);
              if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int B_GUTTER;
                  if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                      B_GUTTER = Architecture::TypeRunTimeIntegerGutter;
                  }
                  else{
                      B_GUTTER = Architecture::TypeConstIntegerGutter;
                  }
                  int BVal = this->heap->decodeRawIntFrom(B_Address+B_GUTTER);
                  if(QString::compare(_operator,"==")==0){
                      if(AVal==BVal){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else if(QString::compare(_operator,"<")==0){
                      if(AVal<BVal){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else if(QString::compare(_operator,">")==0){
                      if(AVal>BVal){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else if(QString::compare(_operator,">=")==0){
                      if(AVal>=BVal){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else if(QString::compare(_operator,"<=")==0){
                      if(AVal<=BVal){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
              }
              else{
                  ErrorRegistry::logError("Cannot compare integer to non-integer on line:"+QString::number(this->p->lineNumber));
              }
          }
          else if(A_Type==ByteCodeInstruction::TypeConstStringType || A_Type==ByteCodeInstruction::TypeRunTimeStringType){
              QString AVal = this->heap->decodeStringFrom(A_Address);
              if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                  int _BVal = this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter);
                   QString BVal = QString::number(_BVal);
                  if(QString::compare(_operator,"==")==0){
                      if(QString::compare(AVal,BVal)==0){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else{
                      this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                  }
              }
              else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int _BVal = this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter);
                   QString BVal = QString::number(_BVal);
                  if(QString::compare(_operator,"==")==0){
                      if(QString::compare(AVal,BVal)==0){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else{
                      this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                  }
              }
              else if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                  QString BVal = this->heap->decodeStringFrom(B_Address);
                  if(QString::compare(_operator,"==")==0){
                      if(QString::compare(AVal,BVal)==0){
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
                      }
                      else{
                          this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                      }
                  }
                  else{
                      this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
                  }
              }
              else{
                  ErrorRegistry::logError("Cannot compare integer to non-integer on line:"+QString::number(this->p->lineNumber));
              }
          }
          else{
              ErrorRegistry::logError("Unwritten Code: Unhandled A Type:"+QString::number(A_Type)+" at #"+QString::number(A_Address)+" for operator"+_operator+", on line:"+QString::number(this->p->lineNumber));
          }
      }
    else if(QString::compare(_operator,"++")==0){
          ////////////this->p->debuggerOutIncrement");
          if(A_Address==Architecture::NULLLocation){
              ErrorRegistry::logError("Unwritten Code: Cannot perform increment on non-integer type:"+QString::number(A_Type)+" given at:#"+QString::number(A_Address)+" , on line:"+QString::number(this->p->lineNumber));
          }
          else if(A_Address==Architecture::UnsetLocation){
              ErrorRegistry::logError("Unwritten Code: Cannot perform increment on non-integer type:"+QString::number(A_Type)+" given at:#"+QString::number(A_Address)+" , on line:"+QString::number(this->p->lineNumber));
          }
          else if(A_Address==Architecture::TrueLocation){
              ErrorRegistry::logError("Unwritten Code: Cannot perform increment on non-integer type:"+QString::number(A_Type)+" given at:#"+QString::number(A_Address)+" , on line:"+QString::number(this->p->lineNumber));
          }
          else if(A_Address==Architecture::FalseLocation){
              ErrorRegistry::logError("Unwritten Code: Cannot perform increment on non-integer type:"+QString::number(A_Type)+" given at:#"+QString::number(A_Address)+" , on line:"+QString::number(this->p->lineNumber));
          }
          else if(A_Address==Architecture::NativeLocation){
              ErrorRegistry::logError("Unwritten Code: Cannot perform increment on non-integer type:"+QString::number(A_Type)+" given at:#"+QString::number(A_Address)+" , on line:"+QString::number(this->p->lineNumber));
          }
          if(A_Type==ByteCodeInstruction::TypeKeyType){
              QString AString = this->heap->decodeStringFrom(A_Address);
            //  //////////////this->p->debuggerOut("increment on key..."+AString,true);
              int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
           //   //////////////this->p->debuggerOut("At value address:"+QString::number(AValAddress),true);
              int AVal = this->heap->decodeRawIntFrom(AValAddress);
              char typeBit = this->heap->data->getByte(AVal);
              if(typeBit==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int variableVal = this->heap->decodeRawIntFrom(AVal+Architecture::TypeRunTimeIntegerGutter);
                  variableVal++;
                  PByteArray* newVal = new PByteArray();
                  newVal->setNum(variableVal);
                  this->heap->copyInTo(AVal+Architecture::TypeRunTimeIntegerGutter,newVal,Architecture::RawIntegerSize);
              }
              else{
                  ErrorRegistry::logError("Cannot perform increment on non-integer type:"+QString::number(typeBit)+" given at:#"+QString::number(AVal)+" , on line:"+QString::number(this->p->lineNumber));
              }
          }
          else if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
              //////////////this->p->debuggerOut("increment on property key...",true);
              A_Address = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
              A_Address = this->heap->decodeRawIntFrom(A_Address);
              A_Type = this->heap->data->getByte(A_Address);
              if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int val = this->heap->decodeRawIntFrom(A_Address+Architecture::TypeRunTimeIntegerGutter);
                  val++;
                  PByteArray* temp = new PByteArray();
                  temp->setNum(val);
                  temp->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                  this->heap->copyInTo(A_Address,temp,Architecture::TypeRunTimeIntegerSize);
              }
              else{
                  ErrorRegistry::logError("Cannot increment non integer.");
              }
          }
          else{
              ErrorRegistry::logError("Cannot perform increment on type:"+QString::number(A_Type)+", on line:"+QString::number(this->p->lineNumber));
          }
      }
    else if(QString::compare(_operator,"--")==0){
          if(A_Type==ByteCodeInstruction::TypeKeyType){
              QString AString = this->heap->decodeStringFrom(A_Address);
              int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
              int AVal = this->heap->decodeRawIntFrom(AValAddress);
              char typeBit = this->heap->data->getByte(AVal);
              if(typeBit==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int variableVal = this->heap->decodeRawIntFrom(AVal+Architecture::TypeRunTimeIntegerGutter);
                  variableVal--;
                  PByteArray* newVal = new PByteArray();
                  newVal->setNum(variableVal);
                  this->heap->copyInTo(AVal+Architecture::TypeConstIntegerGutter,newVal,Architecture::RawIntegerSize);
              }
              else{
                  ErrorRegistry::logError("Cannot decrement non integer.");
              }
          }
          else if(A_Type==ByteCodeInstruction::TypePropertyKeyType){
              A_Address = this->heap->decodeRawIntFrom(A_Address+Architecture::TypePropertyKeyGutter);
              A_Address = this->heap->decodeRawIntFrom(A_Address);
              A_Type = this->heap->data->getByte(A_Address);
              if(A_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                  int val = this->heap->decodeRawIntFrom(A_Address+Architecture::TypeRunTimeIntegerGutter);
                  val--;
                  PByteArray* temp = new PByteArray();
                  temp->setNum(val);
                  temp->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                  this->heap->copyInTo(A_Address,temp,Architecture::TypeRunTimeIntegerSize);
              }
              else{
                  ErrorRegistry::logError("Cannot decrement non integer.");
              }
          }
          else{
              ErrorRegistry::logError("Cannot decrement non key.");
          }
    }
    else if(QString::compare(_operator,"^")==0){
        ErrorRegistry::logError("Unwritten code for operator '"+_operator+"'.");
    }
    else if(QString::compare(_operator,"===")==0){
        A_Address = this->heap->completeDeref(A_Address);
        B_Address = this->heap->completeDeref(B_Address);
        if(A_Address==B_Address){
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
        }
        else{
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
        }
    }
    else if(QString::compare(_operator,"!==")==0){
        A_Address = this->heap->completeDeref(A_Address);
        B_Address = this->heap->completeDeref(B_Address);
        if(A_Address!=B_Address){
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::TrueLocation);
        }
        else{
            this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
        }
    }
    else if(QString::compare(_operator,"+=")==0){
        if(A_Type==ByteCodeInstruction::TypeKeyType){
            QString aString = this->heap->decodeStringFrom(A_Address);
            A_Address = this->heap->valueAddressInStackFrames(aString,A_Address);
            int AValAddr = this->heap->decodeRawIntFrom(A_Address);
            unsigned char type = this->heap->data->getByte(AValAddr);
            if(this->heap->data->getByte(AValAddr)==ByteCodeInstruction::TypeHeapFragment){
                //repoint
                PByteArray* _valAddress = new PByteArray();
                _valAddress->setNum(this->heap->decodeRawIntFrom(AValAddr+Architecture::TypeHeapFragmentGutter));
                this->heap->copyInTo(A_Address,_valAddress,Architecture::RawIntegerSize);
                AValAddr = this->heap->decodeRawIntFrom(A_Address);
                type = this->heap->data->getByte(AValAddr);
            }
            if(type==ByteCodeInstruction::TypeRunTimeIntegerType){
                if(B_Type==ByteCodeInstruction::TypeConstIntegerType || B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                    int GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                        GUTTER = Architecture::TypeConstIntegerGutter;
                    }
                    else{
                        GUTTER = Architecture::TypeRunTimeIntegerGutter;
                    }
                    int bVal  = this->heap->decodeRawIntFrom(B_Address+GUTTER);
                    int variableVal = this->heap->decodeRawIntFrom(AValAddr+Architecture::TypeRunTimeIntegerGutter);
                    variableVal+=bVal;
                    PByteArray* newVal = new PByteArray();
                    newVal->setNum(variableVal);
                    this->heap->copyInTo(AValAddr+Architecture::TypeRunTimeIntegerGutter,newVal,Architecture::RawIntegerSize);
                }
                else{
                    ErrorRegistry::logError("Cannot perform *= , righthand is non-integer.");
                }
            }
            else if(type==ByteCodeInstruction::TypeConstStringType || type==ByteCodeInstruction::TypeRunTimeStringType){
                //deref B
                while(B_Type==ByteCodeInstruction::TypePropertyKeyType || B_Type==ByteCodeInstruction::TypeHeapFragment || B_Type==ByteCodeInstruction::TypeKeyType){
                    if(B_Type==ByteCodeInstruction::TypePropertyKeyType){
                        B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypePropertyKeyGutter);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeHeapFragment){
                        B_Address = this->heap->decodeRawIntFrom(B_Address+Architecture::TypeHeapFragmentGutter);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeKeyType){
                        QString aString = this->heap->decodeStringFrom(B_Address);
                        B_Address = this->heap->valueAddressInStackFrames(aString,B_Address);
                        B_Address = this->heap->decodeRawIntFrom(B_Address);
                        B_Type = this->heap->data->getByte(B_Address);
                    }
                }
                if(B_Address==Architecture::NULLLocation){
                    QString bString = "NULL";
                    this->heap->appendToString(AValAddr,bString);
                }
                else if(B_Address==Architecture::UnsetLocation){
                    QString bString = "unset";
                    this->heap->appendToString(AValAddr,bString);
                }
                else if(B_Address==Architecture::TrueLocation){
                    QString bString = "true";
                    this->heap->appendToString(AValAddr,bString);
                }
                else if(B_Address==Architecture::FalseLocation){
                    QString bString = "false";
                    this->heap->appendToString(AValAddr,bString);
                }
                else{
                    if(B_Type==ByteCodeInstruction::TypeConstStringType || B_Type==ByteCodeInstruction::TypeRunTimeStringType){
                        QString bString = this->heap->decodeStringFrom(B_Address);
                        this->heap->appendToString(AValAddr,bString);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        QString bString = QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeRunTimeIntegerGutter));
                        this->heap->appendToString(AValAddr,bString);
                    }
                    else if(B_Type==ByteCodeInstruction::TypeConstIntegerType){
                        QString bString = QString::number(this->heap->decodeRawIntFrom(B_Address+Architecture::TypeConstIntegerGutter));
                        this->heap->appendToString(AValAddr,bString);
                    }
                    else{
                        ErrorRegistry::logError("Cannot perform operator '"+_operator+"' on type:"+QString::number(B_Type)+" at: #"+QString::number(B_Address)+" on line:"+QString::number(this->p->lineNumber));
                    }
                }
            }
            else{
                ErrorRegistry::logError("Invalid Type:Cannot perform operator '"+_operator+"' on type:"+QString::number(type)+" at: #"+QString::number(AValAddr)+". on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            ErrorRegistry::logError("Cannot perform operator '"+_operator+"'. on line:"+QString::number(this->p->lineNumber));
        }
    }
    else if(QString::compare(_operator,"*=")==0 || QString::compare(_operator,"-=")==0 || QString::compare(_operator,"/=")==0 || QString::compare(_operator,"%=")==0){
        if(A_Type==ByteCodeInstruction::TypeKeyType){
            QString AString = this->heap->decodeStringFrom(A_Address);
            int AValAddress = this->heap->valueAddressInStackFrames(AString,A_Address);
            int AVal = this->heap->decodeRawIntFrom(AValAddress);
            char typeBit = this->heap->data->getByte(AVal);
            if(typeBit==ByteCodeInstruction::TypeRunTimeIntegerType){
                if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType || B_Type==ByteCodeInstruction::TypeConstIntegerType){
                    int GUTTER;
                    if(B_Type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        GUTTER=ByteCodeInstruction::TypeRunTimeIntegerType;
                    }
                    else{
                        GUTTER=ByteCodeInstruction::TypeConstIntegerType;
                    }
                    int bVal  = this->heap->decodeRawIntFrom(B_Address+GUTTER);
                    int variableVal = this->heap->decodeRawIntFrom(AVal+Architecture::TypeRunTimeIntegerGutter);
                    if(QString::compare(_operator,"*=")==0){
                        variableVal*=bVal;
                    }
                    else if(QString::compare(_operator,"-=")==0){
                        variableVal-=bVal;
                    }
                    else if(QString::compare(_operator,"/=")==0){
                        variableVal/=bVal;
                    }
                    else if(QString::compare(_operator,"%=")==0){
                        variableVal%=bVal;
                    }
                    PByteArray* newVal = new PByteArray();
                    newVal->setNum(variableVal);
                    this->heap->copyInTo(AVal+Architecture::TypeRunTimeIntegerGutter,newVal,Architecture::RawIntegerSize);
                }
                else{
                    ErrorRegistry::logError("Cannot perform *= , righthand is non-integer.");
                }
            }
            else{
                ErrorRegistry::logError("Cannot perform multiplication on non-integer");
            }
        }
        else{
            ErrorRegistry::logError("Cannot perform multiplaction on non-key type:"+QString::number(A_Type)+", on line:"+QString::number(this->p->lineNumber));
        }
    }
    else{
        ErrorRegistry::logError("Malformed ByteCode: Interpreter does not understand :"+_operator);
    }
}
