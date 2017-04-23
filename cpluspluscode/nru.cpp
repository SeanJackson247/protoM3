#include "nru.h"

#include "errorregistry.h"
#include "bytecodeinstruction.h"
#include "architecture.h"
#include "filereader.h"
#include "filewriter.h"
#include "compiler2.h"

#include <cstdlib>
#include <chrono>
#include <ctime>


NRU::NRU(Interpreter *interpreter){
    this->p=interpreter;
    this->heap=this->p->heap;
}

void NRU::execute(int argsAddress){
    //this->debuggerOut("Native Call:"+QString::number(argsAddress),true);
    bool customReturn = false;
    if(ErrorRegistry::errorCount==0){
        if(this->p->heap->data->getByte(argsAddress)==ByteCodeInstruction::TypeArgsListType){
            int argumentCount = this->p->heap->decodeRawIntFrom(argsAddress+(Architecture::TypeArgumentsListGutterToSize));
            int firstArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter);
            firstArgAddress = this->p->heap->decodeRawIntFrom(firstArgAddress);
            //this->p->debuggerOut("Executing Native, first Address:"+QString::number(firstArgAddress),true);
            if(this->p->heap->data->getByte(firstArgAddress)==ByteCodeInstruction::TypeRunTimeStringType){
                QString firstArgString = this->p->heap->decodeStringFrom(firstArgAddress);
                if(QString::compare(firstArgString,"sys.out")==0 || QString::compare(firstArgString,"app.debug")==0){
                    //this->p->debuggerOut("sys.out or app.debug call...",true);
                    bool isDebug=false;
                    if(QString::compare(firstArgString,"app.debug")==0){
                        isDebug=true;
                    }
                    //this->p->debuggerOut(QString::number(argumentCount)+" arguments",true);
                    int i=1;
                    while(i<argumentCount){
                        int argAddress = argsAddress+Architecture::TypeArgumentsListGutter+(i*Architecture::TypeArgumentsListEntrySize);
                        argAddress = this->p->heap->decodeRawIntFrom(argAddress);
                        while(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeHeapFragment){
                            argAddress = this->p->heap->decodeRawIntFrom(argAddress+Architecture::TypeHeapFragmentGutter);
                        }
                        //this->p->debuggerOut("Arguments Address:"+QString::number(argAddress),true);
                        if(argAddress==Architecture::NULLLocation){
                            if(isDebug){
                                this->p->debuggerOut("NULL",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"NULL"});
                            }
                        }
                        else if(argAddress==Architecture::UnsetLocation){
                            if(isDebug){
                                this->p->debuggerOut("unset",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"unset"});
                            }
                        }
                        else if(argAddress==Architecture::TrueLocation){
                            if(isDebug){
                                this->p->debuggerOut("true",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"true"});
                            }
                        }
                        else if(argAddress==Architecture::FalseLocation){
                            if(isDebug){
                                this->p->debuggerOut("false",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"false"});
                            }
                        }
                        else if(argAddress>=Architecture::RPNAddress && argAddress<Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            if(isDebug){
                                this->p->debuggerOut("[RPN]",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"[RPN]"});
                            }
                        }
                        else if(argAddress<Architecture::RPNAddress){
                            ErrorRegistry::logError("Malformed Argument.");
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeRunTimeIntegerType){
                            if(isDebug==true){
                                this->p->debuggerOut(QString::number(this->p->heap->decodeRawIntFrom(argAddress+Architecture::TypeRunTimeIntegerGutter)),true);
                            }
                            else{
                                int addr = this->p->heap->decodeRawIntFrom(argAddress+Architecture::TypeRunTimeIntegerGutter);
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{QString::number(addr)});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeRunTimeFloatType){
                            if(isDebug==true){
                                this->p->debuggerOut(QString::number(this->p->heap->decodeRawFloatFrom(argAddress+Architecture::TypeRunTimeFloatGutter),'G',5),true);
                            }
                            else{
                                float addr = this->p->heap->decodeRawFloatFrom(argAddress+Architecture::TypeRunTimeFloatGutter);
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{QString::number(addr,'G',5)});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeRunTimeStringType || this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeConstStringType){
                            if(isDebug){
                                this->p->debuggerOut(this->p->heap->decodeStringFrom(argAddress),true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{this->p->heap->decodeStringFrom(argAddress)});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeObjectType){
                            QString objectString = this->p->heap->objectToString(argAddress,0);
                            if(isDebug){
                                this->p->debuggerOut(objectString,true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{objectString});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeConstObjectType){
                            if(isDebug){
                                this->p->debuggerOut("[CONSTANT-OBJECT]",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"[CONSTANT-OBJECT]"});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypePropertyKeyType){
                            if(isDebug){
                                this->p->debuggerOut("[PROPERTY-KEY]",true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"[PROPERTY-KEY]"});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeKeyType){
                            int propAddress = this->p->heap->decodeRawIntFrom(
                                        this->p->heap->valueAddressInStackFrames(
                                            this->p->heap->decodeStringFrom(argAddress),
                                            argAddress)
                                      );
                            unsigned char typeBit = this->p->heap->data->getByte(propAddress);
                            if(typeBit==ByteCodeInstruction::TypeRunTimeIntegerType){
                               if(isDebug==true){
                                   this->p->debuggerOut(QString::number(this->p->heap->decodeRawIntFrom(propAddress+Architecture::TypeRunTimeIntegerGutter)),true);
                               }else{
                                   this->p->nativeStore->resolve(NativeStore::SysOutAddress,{QString::number(this->p->heap->decodeRawIntFrom(propAddress+1))});
                                }
                            }
                            else if(typeBit==ByteCodeInstruction::TypeRunTimeStringType){
                               this->p->nativeStore->resolve(NativeStore::SysOutAddress,{this->p->heap->decodeStringFrom(propAddress)});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeOpenArrayType || this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeClosedArrayType){
                            QString arrayString = this->p->heap->arrayToString(argAddress);
                            if(isDebug){
                                this->p->debuggerOut(arrayString,true);
                            }
                            else{
                                this->p->nativeStore->resolve(NativeStore::SysOutAddress,{arrayString});
                            }
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeArgsListType){
                            QString argList = "[ARGSLIST][\n";
                            int i=0;
                            int capacity = this->p->heap->decodeRawIntFrom(argAddress+1);
                            int size = this->p->heap->decodeRawIntFrom(argAddress+5);
                            argList+=QString::number(size)+":"+QString::number(capacity)+"][\n";
                            while(i<size){
                                int argaddr = this->p->heap->decodeRawIntFrom(argAddress+9+(i*Architecture::TypeArgumentsListEntrySize));
                                argList+="\t["+QString::number(i)+"]:[#"+QString::number(argaddr)+"][";
                                if(this->p->heap->data->getByte(argaddr)==ByteCodeInstruction::TypeRunTimeIntegerType){
                                    argList+=QString::number(this->p->heap->decodeRawIntFrom(argaddr+Architecture::TypeRunTimeIntegerGutter));
                                }
                                else if(this->p->heap->data->getByte(argaddr)==ByteCodeInstruction::TypeObjectType){
                                    argList+="OBJECT";
                                }
                                else if(this->p->heap->data->getByte(argaddr)==ByteCodeInstruction::TypeOpenArrayType){
                                    argList+="ARRAY";
                                }
                                else if(this->p->heap->data->getByte(argaddr)==ByteCodeInstruction::TypeArgsListType){
                                    argList+="ARGUMENTS LIST";
                                }
                                if(i!=(size-1)){
                                    argList+="],";
                                }
                                else{
                                    argList+="]\n";
                                }
                                i++;
                            }
                            argList+="]";
                            this->p->nativeStore->resolve(NativeStore::SysOutAddress,{argList});
                        }
                        else if(this->p->heap->data->getByte(argAddress)==ByteCodeInstruction::TypeLambdaType){
                            this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"[LAMBDA][#"+QString::number(argAddress)+"]"});
                        }
                        else{
                            this->p->nativeStore->resolve(NativeStore::SysOutAddress,{"[Error:("+QString::number(this->p->heap->data->getByte(argAddress))+")#"+QString::number(argAddress)+"]"});
                        }
                        i++;
                    }
                }
                else if(QString::compare(firstArgString,"Integer.random")==0){
                    if(argumentCount==1){
                        customReturn = true;
                        rand();
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsRunTimeInteger(0,rand());
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,Architecture::TypeRunTimeIntegerSize);
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else{
                        ErrorRegistry::logError("Unexpected Argument for native('Integer.random');");
                    }
                }
                else if(QString::compare(firstArgString,"Array.size")==0){
                    customReturn = true;
                    int arrayaddr = this->p->heap->decodeRawIntFrom(argsAddress+(13));
                    if(this->p->heap->data->getByte(arrayaddr)==ByteCodeInstruction::TypeOpenArrayType){
                        int size = this->p->heap->decodeRawIntFrom(arrayaddr+Architecture::TypeArrayGutterToSize);//5
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsRunTimeInteger(0,size);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,Architecture::TypeRunTimeIntegerSize);
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else{
                        ErrorRegistry::logError("native('Array.size') expects second argument to be an array, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Array.capacity")==0){
                    customReturn = true;
                    int arrayaddr = this->p->heap->decodeRawIntFrom(argsAddress+(13));
                    if(this->p->heap->data->getByte(arrayaddr)==ByteCodeInstruction::TypeOpenArrayType){
                        int size = this->p->heap->decodeRawIntFrom(arrayaddr+1);
                        PByteArray* randomInt = new PByteArray();
                        randomInt->setNum(size);
                        randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,5);
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else{
                        ErrorRegistry::logError("native('Array.size') expects second argument to be an array, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Array.remove")==0){
                    //////this->debuggerOutArray.remove",true);
                    //ErrorRegistry::logError("Array.remove");
                    int arrayaddr = this->p->heap->decodeRawIntFrom(argsAddress+(Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*1)));
                    if(this->p->heap->data->getByte(arrayaddr)==ByteCodeInstruction::TypeOpenArrayType){
                        ////this->debuggerOutArray.remove arg2 is array...",true);
                        int indexAddress = this->p->heap->decodeRawIntFrom(argsAddress+(Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*2)));
                        if(this->p->heap->data->getByte(indexAddress)==ByteCodeInstruction::TypeRunTimeIntegerType){
                            ////this->debuggerOutArray.remove arg3 is int...",true);
                            int index = this->p->heap->decodeRawIntFrom(indexAddress+Architecture::TypeRunTimeIntegerGutter);
                            ////this->debuggerOutCalling heap.removefromarray",true);
                            this->p->heap->removeFromArray(arrayaddr,index);
                        }
                        else{
                            ErrorRegistry::logError("native('Array.remove') expects third argument to be integer, type:"+QString::number(this->p->heap->data->getByte(indexAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Array.remove') expects second argument to be an array, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"app.yield")==0){
                    this->p->yield();
                }
                else if(QString::compare(firstArgString,"Clock.now")==0){
                    customReturn = true;
                    PByteArray* randomInt = new PByteArray();
                    long start = time(0);
                    randomInt->setNum((int)start);
                    randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                    int returnAddress = this->p->heap->data->size();
                    this->p->heap->copyToNew(randomInt,5);
                    this->p->heap->setRETBuffer(returnAddress);
                }
                else if(QString::compare(firstArgString,"app.setTimeout")==0){
                    QMutex* mutex = new QMutex();
                    mutex->lock();
                    int i=1;
                    int callbackAddress = argsAddress+9+(i*4);
                    callbackAddress = this->p->heap->decodeRawIntFrom(callbackAddress);
                    i++;
                    int timerAddress = argsAddress+9+(i*4);
                    timerAddress = this->p->heap->decodeRawIntFrom(timerAddress);
                    unsigned char cType = this->p->heap->data->getByte(callbackAddress);
                    unsigned char tType = this->p->heap->data->getByte(timerAddress);
                    if(cType==ByteCodeInstruction::TypeLambdaType && tType==ByteCodeInstruction::TypeRunTimeIntegerType){
                        int eventAddress = this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                        if(eventAddress!=Architecture::NULLLocation && eventAddress!=Architecture::UnsetLocation){
                            int eventAddress = this->p->heap->data->size();
                            PByteArray* timerEvent = new PByteArray();
                            int _currentTime = (int)time(0);
                            PByteArray* enquedTime = new PByteArray();
                            int queTime = _currentTime+this->p->heap->decodeRawIntFrom(timerAddress+1);
                            enquedTime->setNum(queTime);
                            PByteArray* callbackPointer = new PByteArray();
                            callbackPointer->setNum(callbackAddress);
                            timerEvent->resize(9);
                            timerEvent->setByte(0,ByteCodeInstruction::TypeTimerEventType);
                            timerEvent->setByte(1,enquedTime->getByte(0));
                            timerEvent->setByte(2,enquedTime->getByte(1));
                            timerEvent->setByte(3,enquedTime->getByte(2));
                            timerEvent->setByte(4,enquedTime->getByte(3));
                            timerEvent->setByte(5,callbackPointer->getByte(0));
                            timerEvent->setByte(6,callbackPointer->getByte(1));
                            timerEvent->setByte(7,callbackPointer->getByte(2));
                            timerEvent->setByte(8,callbackPointer->getByte(3));
                            this->p->heap->copyToNew(timerEvent,9);
                            this->p->heap->appendToArray(this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress),eventAddress);
                        }
                        else{
                            int newEventAddress = this->p->heap->data->size();
                            PByteArray* timerEvent = new PByteArray();
                            int _currentTime = (int)time(0);
                            PByteArray* enquedTime = new PByteArray();
                            int queTime = _currentTime+this->p->heap->decodeRawIntFrom(timerAddress+1);
                            enquedTime->setNum(queTime);
                            PByteArray* callbackPointer = new PByteArray();
                            callbackPointer->setNum(callbackAddress);
                            timerEvent->resize(9);
                            timerEvent->setByte(0,ByteCodeInstruction::TypeTimerEventType);
                            timerEvent->setByte(1,enquedTime->getByte(0));
                            timerEvent->setByte(2,enquedTime->getByte(1));
                            timerEvent->setByte(3,enquedTime->getByte(2));
                            timerEvent->setByte(4,enquedTime->getByte(3));
                            timerEvent->setByte(5,callbackPointer->getByte(0));
                            timerEvent->setByte(6,callbackPointer->getByte(1));
                            timerEvent->setByte(7,callbackPointer->getByte(2));
                            timerEvent->setByte(8,callbackPointer->getByte(3));
                            this->p->heap->copyToNew(timerEvent,9);
                            int newEventListAddress = this->p->heap->data->size();
                            PByteArray* eventList = new PByteArray();
                            PByteArray* listSize = new PByteArray();
                            listSize->setNum(1);
                            PByteArray* firstEntry = new PByteArray();
                            firstEntry->setNum(newEventAddress);
                            eventList->resize(9);
                            eventList->setByte(0,ByteCodeInstruction::TypeEventList);
                            eventList->setByte(1,listSize->getByte(0));
                            eventList->setByte(2,listSize->getByte(1));
                            eventList->setByte(3,listSize->getByte(2));
                            eventList->setByte(4,listSize->getByte(3));
                            eventList->setByte(5,firstEntry->getByte(0));
                            eventList->setByte(6,firstEntry->getByte(1));
                            eventList->setByte(7,firstEntry->getByte(2));
                            eventList->setByte(8,firstEntry->getByte(3));
                            this->p->heap->copyToNew(eventList,9);
                            PByteArray* _newEventListAddress = new PByteArray();
                            _newEventListAddress->setNum(newEventListAddress);
                            this->p->heap->data->encodeRawIntegerAt(Architecture::EventStackAddress,newEventListAddress);
                        }
                    }
                    else{
                        ErrorRegistry::logError("native(app.setTimeout) expects arguments lambda,int");
                    }
                    mutex->unlock();
                    delete mutex;
                }
                else if(QString::compare(firstArgString,"Array.append")==0){
                    int secondArgAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize));
                    int argAddress = argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*2);
 //                   argAddress = this->p->heap->decodeRawIntFrom(argAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeOpenArrayType){
  //                      int i=2;
    //                    while(i<argumentCount){
                            int _argAddress = this->p->heap->decodeRawIntFrom(argAddress);
                            this->p->heap->appendToArray(secondArgAddress,_argAddress);
                            if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeHeapFragment){
                                secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress+Architecture::TypeHeapFragmentGutter);
                                if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeOpenArrayType){
                                    ErrorRegistry::logError("Internal Error::native('Array.append'), array not where expected after derefencing heap fragment.");
                                }
                            }
                            else if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeOpenArrayType){
                                ErrorRegistry::logError("Internal Error::native('Array.append'), - argument address is not heap fragment or array.");
                            }
          //                  argAddress+=Architecture::TypeArrayEntrySize;
        //                    i++;
      //                  }
                    }
                    else{
                        ErrorRegistry::logError("native('Array.append') expects first argument to be array, type:"+QString::number(this->p->heap->data->getByte(secondArgAddress))+" given at address:#"+QString::number(secondArgAddress)+" on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Array.merge")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    int thirdArgAddress =argsAddress+(17);
                    thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                    if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeOpenArrayType){
                        ErrorRegistry::logError("Unwritten Code::native('Array.merge')");
                    }
                    else{
                        ErrorRegistry::logError("native('Array.merge') expects second argument to be array, non-array given on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Array.prepend")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeOpenArrayType){
                        int argAddress =argsAddress+(17);
                        int size = this->p->heap->decodeRawIntFrom(argsAddress+(5));
                        int i=2;
                        while(i<size){
                            int _argAddress = this->p->heap->decodeRawIntFrom(argAddress);
                            this->p->heap->prependToArray(secondArgAddress,_argAddress);
                            if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeHeapFragment){
                                secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress+1);
                                if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeOpenArrayType){
                                    ErrorRegistry::logError("Internal Error::native('Array.append'), array not where expected after derefencing heap fragment.");
                                }
                            }
                            else if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeOpenArrayType){
                                ErrorRegistry::logError("Internal Error::native('Array.append'), - argument address is not heap fragment or array.");
                            }
                            argAddress+=4;
                            i++;
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Array.prepend') expects first argument to be array, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Register.prototype")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeRunTimeStringType){
                        ErrorRegistry::logError("Error - native('Register.prototype') expects first argument to be a string, type ("+QString::number(this->p->heap->data->getByte(secondArgAddress))+") given on line:"+QString::number(this->p->lineNumber));
                    }
                    else{
                        QString secondArgString = this->p->heap->decodeStringFrom(secondArgAddress);
                        if(QString::compare(secondArgString,"Array")==0){
                            int thirdArgAddress =argsAddress+(17);
                            thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                            if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeObjectType){
                                PByteArray* _addr = new PByteArray();
                                _addr->setNum(thirdArgAddress);
                                this->p->heap->copyInTo(Architecture::ArrayLocation,_addr,4);
                            }
                            else{
                                ErrorRegistry::logError("Error - native('Register.prototype') expects second argument to be an object.");
                            }
                        }
                        else if(QString::compare(secondArgString,"Lambda")==0){
                           int thirdArgAddress =argsAddress+(17);
                           thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                           if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeObjectType){
                               PByteArray* _addr = new PByteArray();
                               _addr->setNum(thirdArgAddress);
                               this->p->heap->copyInTo(Architecture::LambdaLocation,_addr,4);
                           }
                           else{
                               ErrorRegistry::logError("Error - native('Register.prototype') expects second argument to be an object.");
                           }
                        }
                        else if(QString::compare(secondArgString,"Object")==0){
                           int thirdArgAddress =argsAddress+(17);
                           thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                           if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeObjectType){
                               PByteArray* _addr = new PByteArray();
                               _addr->setNum(thirdArgAddress);
                               this->p->heap->copyInTo(Architecture::ObjectLocation,_addr,4);
                           }
                           else{
                               ErrorRegistry::logError("Error - native('Register.prototype') expects second argument to be an object.");
                           }
                        }
                        else if(QString::compare(secondArgString,"String")==0){
                           int thirdArgAddress =argsAddress+(17);
                           thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                           if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeObjectType){
                               PByteArray* _addr = new PByteArray();
                               _addr->setNum(thirdArgAddress);
                               this->p->heap->copyInTo(Architecture::StringLocation,_addr,4);
                           }
                           else{
                               ErrorRegistry::logError("Error - native('Register.prototype') expects second argument to be an object.");
                           }
                        }
                        else{
                            ErrorRegistry::logError("native('Register.prototype') does not understand first argument:"+secondArgString+", on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                }
                else if(QString::compare(firstArgString,"Object.getInit")==0){
                    ErrorRegistry::logError("Unwritten Code:Object.getInit.");
                }
                else if(QString::compare(firstArgString,"Object.setInit")==0){
                    ErrorRegistry::logError("Unwritten Code:Object.setInit.");
                }
                else if(QString::compare(firstArgString,"type")==0){
                    customReturn = true;
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    secondArgAddress = this->p->heap->completeDeref(secondArgAddress);
                    unsigned char type = this->p->heap->data->getByte(secondArgAddress);
                    if(secondArgAddress==Architecture::NULLLocation){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("NULL",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(secondArgAddress==Architecture::UnsetLocation){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("unset",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(secondArgAddress==Architecture::TrueLocation || secondArgAddress==Architecture::FalseLocation){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("bool",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(type==ByteCodeInstruction::TypeRunTimeIntegerType){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("Integer",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(type==ByteCodeInstruction::TypeRunTimeStringType){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("String",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }/*
                    else if(type==ByteCodeInstruction::TypeFloatType){

                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("Float",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }*/
                    else if(type==ByteCodeInstruction::TypeOpenArrayType){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("Array",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(type==ByteCodeInstruction::TypeObjectType){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("Object",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else if(type==ByteCodeInstruction::TypeLambdaType){
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("Lambda",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                    else{
                        PByteArray* randomInt = new PByteArray();
                        randomInt->encodeAsString("null",false);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                }
                else if(QString::compare(firstArgString,"Heap.constant.toString")==0){
                    PByteArray* randomInt = new PByteArray();
                    qDebug()<<"getting threshold string...";
                    QString str = this->p->heap->initThresholdString();
                    qDebug()<<"got threshold string..."<<str;
                    randomInt->encodeAsString(str,false);
                    int returnAddress = this->p->heap->data->size();
                    this->p->heap->copyToNew(randomInt,randomInt->size());
                    this->p->heap->setRETBuffer(returnAddress);
                }
                else if(QString::compare(firstArgString,"Object.matches")==0){
                    if(argumentCount==3){
                        int secondArgAddress =argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*1);
                        secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                        int thirdArgAddress =argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*2);
                        thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                        unsigned char secondArgType = this->heap->data->getByte(secondArgAddress);
                        unsigned char thirdArgType = this->heap->data->getByte(thirdArgAddress);
                        if(secondArgType==ByteCodeInstruction::TypeObjectType && thirdArgType==ByteCodeInstruction::TypeObjectType){
                            customReturn = true;
                            ////this->debuggerOut("Getting property lists...:",true);
                            QStringList propList = this->p->heap->getPropertyList(secondArgAddress);
                            QStringList propList2 = this->p->heap->getPropertyList(thirdArgAddress);
                            ////this->debuggerOutGot property lists...:",true);
                            int i=0;
                            bool matches=true;
                            while(i<propList2.size() && matches==true){
                                int o=0;
                                bool found=false;
                                while(o<propList.size() && found==false){

                                    ////this->debuggerOutComparing Property:"+propList[o]+" with "+propList2[i],true);

                                    if(QString::compare(propList[o],propList2[i])==0){
                                        found=true;
                                    }
                                    o++;
                                }
                                if(found==false){
                                    matches=false;
                                }
                                i++;
                            }
                            int returnAddress;
                            if(matches){
                                ////this->debuggerOut("Object.matches!",true);
                                returnAddress = Architecture::TrueLocation;
                            }
                            else{
                                ////this->debuggerOut("Object.doesn'tMatch!",true);
                                returnAddress = Architecture::FalseLocation;
                            }
                            this->p->heap->setRETBuffer(returnAddress);
                        }
                        else{
                            ErrorRegistry::logError("native('Object.matches'); expects 2 arguments which are objects, types: "+QString::number(secondArgType)+" and "+QString::number(thirdArgType)+" given on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Object.matches'); expects 2 additional arguments - "+QString::number(argumentCount)+" given, on line"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Object.reInit")==0){
//                    customReturn=true;
                    int secondArgAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize));
                    if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeObjectType){
                        ErrorRegistry::logError("Error - native('Object.reInit') expects first argument to be an object.");
                    }
                    else{
                        int initAddress = this->p->heap->decodeRawIntFrom(secondArgAddress+Architecture::TypeObjectGutterToInit);
                        if(initAddress>Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            PByteArray* rpnStore = new PByteArray();
                            rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                            int i=0;
                            while(i<rpnStore->size()){
                                rpnStore->setByte(i,this->p->heap->data->getByte(Architecture::RPNAddress+i));
                                i++;
                            }
                            int thisStore = this->p->heap->decodeRawIntFrom(Architecture::ThisAddress);
                            this->p->executeLambda(initAddress,Architecture::UnsetLocation,secondArgAddress);//fix this later
                            if(ErrorRegistry::errorCount>0){
                                ErrorRegistry::logError("Error during object initialization, object:"+this->p->heap->objectToString(secondArgAddress,0)+", on line:"+QString::number(this->p->lineNumber));
                            }
                            else{
                                i=0;
                                while(i<rpnStore->size()){
                                    this->p->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                                    i++;
                                }
                                this->p->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisStore);
                            }
                        }
                    }
                }
                else if(QString::compare(firstArgString,"Object.getProto")==0){
                    customReturn = true;
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(secondArgAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                        int returnAddress = this->heap->decodeRawIntFrom(secondArgAddress+5);
                        if(returnAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                            this->p->heap->setRETBuffer(returnAddress);
                        }
                        else{
                            this->p->heap->setRETBuffer(this->heap->decodeRawIntFrom(Architecture::ObjectLocation));
                        }
                    }else{
                        this->p->heap->setRETBuffer(this->heap->decodeRawIntFrom(Architecture::ObjectLocation));
                    }
                }
                else if(QString::compare(firstArgString,"Object.setProto")==0){
                    //ErrorRegistry::logError("Unwritten Code:Object.setProto.");
                    int objectAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize));
                    int newProtoAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*2));
                    if(this->heap->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType){
                        this->heap->data->encodeRawIntegerAt(objectAddress+Architecture::TypeObjectGutterToProto,newProtoAddress);
                    }
                    else{
                        ErrorRegistry::logError("native('Object.setProto') expects first argument to be an object, type:"+QString::number(this->heap->data->getByte(objectAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Object.mixin")==0){
                    int objectAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize));
                    int mixinAddress = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*2));
                    if(this->heap->data->getByte(objectAddress)==ByteCodeInstruction::TypeObjectType && this->heap->data->getByte(mixinAddress)==ByteCodeInstruction::TypeObjectType){
                        int mode = this->heap->data->decodeRawIntFrom(argsAddress+Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*3));
                        if(mode==Architecture::TrueLocation){
                            QStringList keys1 = this->heap->getPropertyList(objectAddress);
                            QStringList keys2 = this->heap->getPropertyList(mixinAddress);
                            QVector<int> keyAddresses1 = this->heap->getPropertyKeyList(objectAddress);
                            QVector<int> keyAddresses2 = this->heap->getPropertyKeyList(mixinAddress);
                            QVector<int> vals1 = this->heap->getPropertyValuesList(objectAddress);
                            QVector<int> vals2 = this->heap->getPropertyValuesList(mixinAddress);
                            QVector<unsigned char> mods1 = this->heap->getPropertyModifiersList(objectAddress);
                            QVector<unsigned char> mods2 = this->heap->getPropertyModifiersList(mixinAddress);
                            int i=0;
                            while(i<keys2.size()){
                                keys1.append(keys2[i]);
                                i++;
                            }
                            i=0;
                            while(i<vals2.size()){
                               vals1.append(vals2[i]);
                               i++;
                            }
                            i=0;
                            while(i<mods2.size()){
                               mods1.append(mods2[i]);
                               i++;
                            }
                            i=0;
                            while(i<keyAddresses2.size()){
                               keyAddresses1.append(keyAddresses2[i]);
                               i++;
                            }

                            //construct a new object to replace the old

                            PByteArray* newObject = new PByteArray();
                            newObject->resize(Architecture::TypeObjectGutter);
                            i=0;
                            while(i<Architecture::TypeObjectGutter){
                                newObject->setByte(i,this->heap->data->getByte(objectAddress+i));
                                i++;
                            }

                            int factor = 1;

                            //now set size
                            int size = keys1.size()/Architecture::StandardObjectSize;
                            if(keys1.size()%Architecture::StandardObjectSize>0){
                                size++;
                            }
                            if(size==0){
                                size=1;
                            }
                            size = size*factor;

                            //reserve bytes

                            newObject->resize(newObject->size()+(size*Architecture::TypeObjectPropertySize));

                            QVector<int> usedIndexices={};

                            i=0;
                            while(i<keys1.size()){
                                int index = this->heap->expectedIndex(size,keys1[i]);
                                int e=0;
                                bool used=false;
                                while(e<usedIndexices.size()){
                                    if(usedIndexices[e]==index){
                                        used=true;
                                    }
                                    e++;
                                }
                                if(used){
//                                    ErrorRegistry::logError("Unwritten Code:Object.mixin requires used index.");
                                    index=this->heap->shiftIndex(size,index,1);
                                    e=0;
                                    bool used=false;
                                    while(e<usedIndexices.size()){
                                        if(usedIndexices[e]==index){
                                            used=true;
                                        }
                                        e++;
                                    }
                                    if(used){
                                        index=this->heap->shiftIndex(size,index,2);
                                        e=0;
                                        bool used=false;
                                        while(e<usedIndexices.size()){
                                            if(usedIndexices[e]==index){
                                                used=true;
                                            }
                                            e++;
                                        }
                                        if(used){
                                            ErrorRegistry::logError("Unwritten Code:Object.mixin requires enlargment.");
                                        }
                                        else{
                                            newObject->setByte(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize),mods1[i]);
                                            newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent,keyAddresses1[i]);
                                            newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent,vals1[i]);
                                        }
                                    }
                                    else{
                                        newObject->setByte(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize),mods1[i]);
                                        newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent,keyAddresses1[i]);
                                        newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent,vals1[i]);
                                    }
                                }
                                else{
                                    newObject->setByte(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize),mods1[i]);
                                    newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyKeyIndent,keyAddresses1[i]);
                                    newObject->encodeRawIntegerAt(Architecture::TypeObjectGutter+(index*Architecture::TypeObjectPropertySize)+Architecture::TypeObjectPropertyValueIndent,vals1[i]);
                                }
                                usedIndexices.append(index);
                                i++;
                            }
                            customReturn=true;
                            int returnAddress = this->heap->data->size();
                            this->heap->copyToNew(newObject,newObject->size());
                            this->heap->setRETBuffer(returnAddress);
                        }
                        else if(mode==Architecture::FalseLocation){
                            ErrorRegistry::logError("Unwritten Code:native('Object.mixin') public,partial,shallow,false");
                        }
                        else{
                            ErrorRegistry::logError("native('Object.mixin') expects 3rd argument to be boolean, type:"+QString::number(this->heap->data->getByte(mode))+" given, on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Object.mixin') expects first 2 argument to be objects, types:"+QString::number(this->heap->data->getByte(objectAddress))+" and "+QString::number(this->heap->data->getByte(mixinAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                    }
                }
                else if(QString::compare(firstArgString,"Object.create")==0){
                    ErrorRegistry::logError("Unwritten Code:Object.create.");
                }
                else if(QString::compare(firstArgString,"Object.forEach")==0){
                    customReturn = true;
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    int thirdArgAddress =argsAddress+(17);
                    thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                    bool hasOwn=false;
                    if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeObjectType){
                        if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeLambdaType){
                            int i=0;
                            int size = this->p->heap->decodeRawIntFrom(secondArgAddress+13)*Architecture::StandardObjectSize;
                            while(i<size){
                                int keyAddress = this->p->heap->data->decodeRawIntFrom(secondArgAddress+17+(i*9)+1);
                                if(keyAddress!=Architecture::NULLLocation && keyAddress!=Architecture::UnsetLocation){
                                    QString key = this->p->heap->decodeStringFrom(keyAddress);
                                    PByteArray* _key = new PByteArray();
                                    _key->encodeAsString(key,false);
                                    int keyStringAddress = this->p->heap->data->size();
                                    this->p->heap->copyToNew(_key,_key->size());
                                    int argsAddress = this->p->heap->data->size();
                                    this->p->heap->constructNewArgsList(keyStringAddress);
                                    this->p->executeLambda(thirdArgAddress,argsAddress,secondArgAddress);
                                }
                                i++;
                            }
                        }
                        else if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeObjectType){
                            int i=0;
                            int size = this->p->heap->decodeRawIntFrom(secondArgAddress+13)*Architecture::StandardObjectSize;
                            int invokeAddress = this->p->heap->propertyValueAddressLookUp(thirdArgAddress,"__invoke",false);
                            invokeAddress=this->p->heap->decodeRawIntFrom(invokeAddress);
                            if(invokeAddress>=Architecture::SIZE_OF_INITIAL_HEAP_STORE && this->p->heap->data->getByte(invokeAddress)==ByteCodeInstruction::TypeLambdaType){
                                while(i<size){
                                    int keyAddress = this->p->heap->data->decodeRawIntFrom(secondArgAddress+17+(i*9)+1);
                                    if(keyAddress!=Architecture::NULLLocation && keyAddress!=Architecture::UnsetLocation){
                                        QString key = this->p->heap->decodeStringFrom(keyAddress);
                                        PByteArray* _key = new PByteArray();
                                        _key->encodeAsString(key,false);
                                        int keyStringAddress = this->p->heap->data->size();
                                        this->p->heap->copyToNew(_key,_key->size());
                                        int argsAddress = this->p->heap->data->size();
                                        this->p->heap->constructNewArgsList(keyStringAddress);
                                        this->p->executeLambda(invokeAddress,argsAddress,thirdArgAddress);
                                    }
                                    i++;
                                }
                            }
                            else{
                                ErrorRegistry::logError("Object passed to Object.forEach does not contain an __invoke method.");
                            }
                        }
                        else{
                            ErrorRegistry::logError("native('Object.forEach') expects second argument to be a function, type:"+QString::number(this->heap->data->getByte(thirdArgAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Object.forEach') expects first argument to be an object, type:"+QString::number(this->heap->data->getByte(secondArgAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                    }
                    int returnAddress;
                    if(hasOwn){
                        returnAddress = Architecture::TrueLocation;
                    }
                    else{
                        returnAddress = Architecture::FalseLocation;
                    }
                    this->p->heap->setRETBuffer(returnAddress);
                }
                else if(QString::compare(firstArgString,"Object.hasOwn")==0){
                    customReturn = true;
                    int secondArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*2);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    int thirdArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*3);
                    thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                    bool hasOwn=false;
                    if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeObjectType){
                        if(this->p->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeRunTimeStringType){
                            QString key = this->p->heap->decodeStringFrom(thirdArgAddress);
                            hasOwn = this->p->heap->objectHasOwn(secondArgAddress,thirdArgAddress,key);
                        }
                    }
                    int returnAddress;
                    if(hasOwn){
                        //this->debuggerOut("Object has Own!",true);
                        returnAddress = Architecture::TrueLocation;
                    }
                    else{
                        //this->debuggerOut("Object doesnt have Own!",true);
                        returnAddress = Architecture::FalseLocation;
                    }
                    this->p->heap->setRETBuffer(returnAddress);
                }
                else if(QString::compare(firstArgString,"PWidget.create")==0){
//                    customReturn = true;
                    if(argumentCount==4){
                        int secondArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*2);
                        secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                        int thirdArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*3);
                        thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                        int fourthArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*4);
                        fourthArgAddress = this->p->heap->decodeRawIntFrom(fourthArgAddress);
                        if(this->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeRunTimeIntegerType && this->heap->data->getByte(thirdArgAddress)==ByteCodeInstruction::TypeRunTimeIntegerType && this->heap->data->getByte(fourthArgAddress)==ByteCodeInstruction::TypeRunTimeStringType){
                            customReturn = true;
                            int pwidgetid = WidgetManager::pwidgetIds;
                            int w = this->heap->decodeRawIntFrom(secondArgAddress+Architecture::TypeRunTimeIntegerGutter);
                            int h = this->heap->decodeRawIntFrom(thirdArgAddress+Architecture::TypeRunTimeIntegerGutter);
                            QString title = this->heap->decodeStringFrom(fourthArgAddress);
                            this->p->nativeStore->resolve(NativeStore::PWidgetManager,{"create",QString::number(w),QString::number(h),title});
                            int newAddress = this->heap->data->size();
                            PByteArray* pbytes = new PByteArray();
                            pbytes->encodeAsRunTimeInteger(0,pwidgetid);
                            this->heap->copyToNew(pbytes,Architecture::TypeRunTimeIntegerSize);
                            this->heap->setRETBuffer(newAddress);
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('PWidget.create') expects 4 arguments.");
                    }
                }
                else if(QString::compare(firstArgString,"PWidget.show")==0){
//                    customReturn = true;
                    if(argumentCount==2){
                        int secondArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize*2);
                        secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                        if(this->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeRunTimeIntegerType){
                            this->p->nativeStore->resolve(NativeStore::PWidgetManager,{"show",QString::number(this->heap->decodeRawIntFrom(secondArgAddress+Architecture::TypeRunTimeIntegerGutter))});
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('PWidget.show') expects 2 arguments.");
                    }
                }
                else if(QString::compare(firstArgString,"Object.copy")==0){
                    if(argumentCount==2){
                        int secondArgAddress =argsAddress+(Architecture::TypeArgumentsListGutter)+(Architecture::TypeArgumentsListEntrySize);
                        secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                        if(this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeObjectType ||
                           this->p->heap->data->getByte(secondArgAddress)==ByteCodeInstruction::TypeConstObjectType){
                            customReturn=true;
                            int returnAddress = this->p->heap->data->size();

                            ////this->debuggerOut("Object.copying...",true);

                            this->p->heap->copyObjectToNew(secondArgAddress);
                            ////this->debuggerOut("Object.copyied...",true);
                            this->p->heap->setRETBuffer(returnAddress);
                        }
                        else{
                            ErrorRegistry::logError("Non-object passed to Object.copy, type:"+QString::number(this->p->heap->data->getByte(secondArgAddress))+" given, on line:"+QString::number(this->p->lineNumber));
                        }
                    }
                    else if(argumentCount>2){
                        ErrorRegistry::logError("Unexpected Argument for native('Object.copy');");
                    }
                    else{
                        ErrorRegistry::logError("native('Object.copy') requires 1 additional argument;");
                    }
                }
                else if(QString::compare(firstArgString,"Error.throw")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    QString str = this->heap->decodeStringFrom(secondArgAddress);
                    ErrorRegistry::logError(str+", on line:"+QString::number(this->p->lineNumber));
                }
                else if(QString::compare(firstArgString,"Arguments.Array")==0){
                    customReturn = true;
                    int oldArgsAddress = this->p->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                    if(oldArgsAddress==Architecture::NULLLocation || oldArgsAddress==Architecture::UnsetLocation){
                        this->p->heap->setRETBuffer(Architecture::UnsetLocation);
                    }
                    else{
                        PByteArray* randomInt = new PByteArray();
                        int arrayCapacity = this->p->heap->decodeRawIntFrom(oldArgsAddress+Architecture::TypeArgumentsListGutterToCapacity);
                        arrayCapacity = (arrayCapacity*Architecture::TypeArrayEntrySize)+Architecture::TypeArgumentsListGutter;
                        int i=0;

                        while(i<Architecture::TypeArgumentsListGutter){
                            randomInt->append(this->p->heap->data->getByte(oldArgsAddress));
                            oldArgsAddress++;
                            i++;
                        }
                        randomInt->resize(randomInt->size()+Architecture::RawIntegerSize);
                        randomInt->encodeRawIntegerAt(randomInt->size()-Architecture::RawIntegerSize,0);
                        while(i<arrayCapacity){
                            randomInt->append(this->p->heap->data->getByte(oldArgsAddress));
                            oldArgsAddress++;
                            i++;
                        }
                        randomInt->setByte(0,ByteCodeInstruction::TypeOpenArrayType);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,randomInt->size());
                        this->p->heap->setRETBuffer(returnAddress);
                    }
                }
                else if(QString::compare(firstArgString,"Lambda.call")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeLambdaType){
                        ErrorRegistry::logError("Error - native('Lambda.call') expects first argument to be a lambda, type:"+QString::number(this->p->heap->data->getByte(secondArgAddress))+" given.");
                    }
                    else{
                        int thirdArgAddress =argsAddress+(17);
                        thirdArgAddress = this->p->heap->decodeRawIntFrom(thirdArgAddress);
                        int fourthArgAddress =argsAddress+(21);
                        fourthArgAddress = this->p->heap->decodeRawIntFrom(fourthArgAddress);
                        if(this->p->heap->data->getByte(fourthArgAddress)==ByteCodeInstruction::TypeOpenArrayType){
                            customReturn = true;
                            PByteArray* rpnStore = new PByteArray();
                            rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
                            int i=0;
                            while(i<rpnStore->size()){
                                rpnStore->setByte(i,this->p->heap->data->getByte(Architecture::RPNAddress+i));
                                i++;
                            }
                            int thisStore = this->p->heap->decodeRawIntFrom(Architecture::ThisAddress);
                            int argsStore = this->p->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                            PByteArray* _argsList = new PByteArray();
                            int argCapacity = this->p->heap->decodeRawIntFrom(fourthArgAddress+1);
                            int l = (argCapacity*4)+9+fourthArgAddress;
                            while(fourthArgAddress<l){
                                _argsList->append(this->p->heap->data->getByte(fourthArgAddress));
                                fourthArgAddress++;
                            }
                            _argsList->setByte(0,ByteCodeInstruction::TypeArgsListType);
                            int argumentsAddress = this->p->heap->data->size();
                            this->p->heap->copyToNew(_argsList,_argsList->size());
                            this->p->executeLambda(secondArgAddress,argumentsAddress,thirdArgAddress);
                            i=0;
                            while(i<rpnStore->size()){
                                this->p->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
                                i++;
                            }
                            this->p->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisStore);
                            this->p->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argsStore);
                        }
                        else{
                            ErrorRegistry::logError("Error - native('Lambda.call') expects third argument to be an array.");
                        }
                    }
                }
                else if(QString::compare(firstArgString,"Module.load")==0){
                    int secondArgAddress =argsAddress+(13);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeRunTimeStringType){
                        ErrorRegistry::logError("Error - native('Module.load') expects first argument to be a string, type:"+QString::number(this->p->heap->data->getByte(secondArgAddress))+"given, at address:"+QString::number(secondArgAddress));
                    }
                    else{
                        QString secondArgString = this->p->heap->decodeStringFrom(secondArgAddress);
                        auto parts = secondArgString.split(".");
                        QString extension = parts[parts.size()-1];
                        if(QString::compare(extension,"proto")==0){
                            FileReader* fileReader = new FileReader("C:/proto/src/");
                            FileWriter* fileWriter = new FileWriter("C:/proto/src/compiled/");
                            int i=Interpreter::runtimeInstances.size();
                            customReturn = true;
                            PByteArray* randomInt = new PByteArray();
                            randomInt->setNum(i);
                            randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                            int returnAddress = this->p->heap->data->size();
                            this->p->heap->copyToNew(randomInt,5);
                            this->p->heap->setRETBuffer(returnAddress);
                            QMutex* mutex = new QMutex();
                            mutex->lock();
                            int instanceId = Interpreter::runtimeInstances.size();
                            Interpreter* interpreter = new Interpreter(this->p->id,instanceId);
                            Interpreter::runtimeInstances.append(interpreter);
                            mutex->unlock();
                            delete mutex;
                            QThread* interpreterThread = new QThread();
                            QThread* fileIOThread = new QThread();
                            QThread* compilerThread = new QThread();
                            LexNode::encounteredIntegers={};
                            LexNode::encounteredKeys={};
                            LexNode::encounteredStrings={};
                            Compiler2* compiler = new Compiler2();
                            compiler->isRoot=true;
                            interpreter->moveToThread(interpreterThread);
                            fileReader->moveToThread(fileIOThread);
                            fileWriter->moveToThread(fileIOThread);
                            compiler->moveToThread(compilerThread);
                            interpreterThread->setObjectName("Proto Interpreter Thread");
                            interpreter->threadName="Proto Interpreter Thread";
                            QObject::connect(fileReader,SIGNAL(readFile(QString,QVector<QString>)),compiler,SLOT(doWork(QString,QVector<QString>)));
                            QObject::connect(compiler,SIGNAL(debugScript(QString,QString)),fileWriter,SLOT(debugRequest(QString,QString)));
                            QObject::connect(compiler,SIGNAL(exportToInclusion(QString,LexNode*)),compiler,SLOT(finalCompilation(QString,LexNode*)));
                            QObject::connect(compiler,SIGNAL(workCompleted(QString,PByteArray)),fileWriter,SLOT(writeRequest(QString,PByteArray)));
                            QObject::connect(compiler,SIGNAL(workCompleted(QString,PByteArray)),interpreter,SLOT(launch(QString,PByteArray)));
                            compilerThread->start();
                            interpreterThread->start();
                            fileIOThread->start();
                            int callbackAddress=argsAddress+(17);
                            callbackAddress = this->p->heap->decodeRawIntFrom(callbackAddress);
                            if(this->p->heap->data->getByte(callbackAddress)==ByteCodeInstruction::TypeLambdaType){
                                int eventListAddress = this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                                if(eventListAddress!=Architecture::NULLLocation && eventListAddress!=Architecture::UnsetLocation && this->p->heap->data->getByte(eventListAddress)==ByteCodeInstruction::TypeEventList){
                                    int eventAddress = this->p->heap->data->size();
                                    PByteArray* timerEvent = new PByteArray();
                                    PByteArray* enquedTime = new PByteArray();
                                    enquedTime->setNum(instanceId);
                                    PByteArray* callbackPointer = new PByteArray();
                                    callbackPointer->setNum(callbackAddress);
                                    timerEvent->resize(13);
                                    timerEvent->setByte(0,ByteCodeInstruction::TypeModuleReadyEvent);
                                    timerEvent->setByte(1,enquedTime->getByte(0));
                                    timerEvent->setByte(2,enquedTime->getByte(1));
                                    timerEvent->setByte(3,enquedTime->getByte(2));
                                    timerEvent->setByte(4,enquedTime->getByte(3));
                                    timerEvent->setByte(5,callbackPointer->getByte(0));
                                    timerEvent->setByte(6,callbackPointer->getByte(1));
                                    timerEvent->setByte(7,callbackPointer->getByte(2));
                                    timerEvent->setByte(8,callbackPointer->getByte(3));
                                    PByteArray* falsey = new PByteArray();
                                    falsey->setNum(Architecture::FalseLocation);
                                    timerEvent->setByte(9,falsey->getByte(0));
                                    timerEvent->setByte(10,falsey->getByte(1));
                                    timerEvent->setByte(11,falsey->getByte(2));
                                    timerEvent->setByte(12,falsey->getByte(3));
                                    this->p->heap->copyToNew(timerEvent,13);
                                    this->p->heap->appendToArray(this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress),eventAddress);
                                }
                                else{
                                    int newEventAddress = this->p->heap->data->size();
                                    PByteArray* timerEvent = new PByteArray();
                                    PByteArray* enquedTime = new PByteArray();
                                    enquedTime->setNum(instanceId);
                                    PByteArray* callbackPointer = new PByteArray();
                                    callbackPointer->setNum(callbackAddress);
                                    timerEvent->resize(13);
                                    timerEvent->setByte(0,ByteCodeInstruction::TypeModuleReadyEvent);
                                    timerEvent->setByte(1,enquedTime->getByte(0));
                                    timerEvent->setByte(2,enquedTime->getByte(1));
                                    timerEvent->setByte(3,enquedTime->getByte(2));
                                    timerEvent->setByte(4,enquedTime->getByte(3));
                                    timerEvent->setByte(5,callbackPointer->getByte(0));
                                    timerEvent->setByte(6,callbackPointer->getByte(1));
                                    timerEvent->setByte(7,callbackPointer->getByte(2));
                                    timerEvent->setByte(8,callbackPointer->getByte(3));
                                    PByteArray* falsey = new PByteArray();
                                    falsey->setNum(Architecture::FalseLocation);
                                    timerEvent->setByte(9,falsey->getByte(0));
                                    timerEvent->setByte(10,falsey->getByte(1));
                                    timerEvent->setByte(11,falsey->getByte(2));
                                    timerEvent->setByte(12,falsey->getByte(3));
                                    this->p->heap->copyToNew(timerEvent,13);
                                    int newEventListAddress = this->p->heap->data->size();
                                    PByteArray* eventList = new PByteArray();
                                    PByteArray* listSize = new PByteArray();
                                    listSize->setNum(1);
                                    PByteArray* firstEntry = new PByteArray();
                                    firstEntry->setNum(newEventAddress);
                                    eventList->resize(9);
                                    eventList->setByte(0,ByteCodeInstruction::TypeEventList);
                                    eventList->setByte(1,listSize->getByte(0));
                                    eventList->setByte(2,listSize->getByte(1));
                                    eventList->setByte(3,listSize->getByte(2));
                                    eventList->setByte(4,listSize->getByte(3));
                                    eventList->setByte(5,firstEntry->getByte(0));
                                    eventList->setByte(6,firstEntry->getByte(1));
                                    eventList->setByte(7,firstEntry->getByte(2));
                                    eventList->setByte(8,firstEntry->getByte(3));
                                    this->p->heap->copyToNew(eventList,9);
                                    PByteArray* _newEventListAddress = new PByteArray();
                                    _newEventListAddress->setNum(newEventListAddress);
                                    this->p->heap->data->encodeRawIntegerAt(Architecture::EventStackAddress,newEventListAddress);
                                }
                            }
                            fileReader->loadFile(secondArgString);
                        }
                        else if(QString::compare(extension,"prt")==0){
                            FileReader* fileReader = new FileReader("C:/proto/src/");
                            int i=Interpreter::runtimeInstances.size();
                            customReturn = true;
                            PByteArray* randomInt = new PByteArray();
                            randomInt->setNum(i);
                            randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                            int returnAddress = this->p->heap->data->size();
                            this->p->heap->copyToNew(randomInt,5);
                            this->p->heap->setRETBuffer(returnAddress);
                            QMutex* mutex = new QMutex();
                            mutex->lock();
                            Interpreter* interpreter = new Interpreter(this->p->id,Interpreter::runtimeInstances.size());
                            Interpreter::runtimeInstances.append(interpreter);
                            mutex->unlock();
                            delete mutex;
                            QThread* interpreterThread = new QThread();
                            QThread* fileIOThread = new QThread();
                            interpreter->moveToThread(interpreterThread);
                            fileReader->moveToThread(fileIOThread);
                            interpreterThread->setObjectName("Proto Main Interpreter Thread");
                            interpreter->threadName="Proto Main Interpreter Thread";
                            QObject::connect(fileReader,SIGNAL(readFile(QString,QVector<QString>)),interpreter,SLOT(launch(QString,PByteArray)));
                            interpreterThread->start();
                            fileIOThread->start();
                            fileReader->loadFile(secondArgString);
                        }
                        else{
                            ErrorRegistry::logError("Error - native('Module.load') path: '"+secondArgString+"' does not end with '.proto' or '.prt'.");
                        }
                    }
                }
                else if(QString::compare(firstArgString,"Module.id")==0){
                    customReturn = true;
                    PByteArray* randomInt = new PByteArray();
                    randomInt->setNum(this->p->id);
                    randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                    int returnAddress = this->p->heap->data->size();
                    this->p->heap->copyToNew(randomInt,5);
                    this->p->heap->setRETBuffer(returnAddress);
                }
                else if(QString::compare(firstArgString,"Module.connect")==0){
                    int argumentCount = this->p->heap->decodeRawIntFrom(argsAddress+(5));
                    if(argumentCount==5){
                        int processidaddr = this->p->heap->decodeRawIntFrom(argsAddress+9+(0*4));
                        int clientidaddr = this->p->heap->decodeRawIntFrom(argsAddress+9+(1*4));
                        int pipekeyaddr = this->p->heap->decodeRawIntFrom(argsAddress+9+(2*4));
                        int pipeaddr = this->p->heap->decodeRawIntFrom(argsAddress+9+(3*4));
                        if(this->p->heap->data->getByte(processidaddr)==ByteCodeInstruction::TypeRunTimeIntegerType &&
                           this->p->heap->data->getByte(clientidaddr)==ByteCodeInstruction::TypeRunTimeIntegerType &&
                           this->p->heap->data->getByte(pipekeyaddr)==ByteCodeInstruction::TypeRunTimeStringType &&
                           this->p->heap->data->getByte(pipeaddr)==ByteCodeInstruction::TypeObjectType
                                ){
                            int processid = this->p->heap->decodeRawIntFrom(processidaddr+1);
                            int clientid = this->p->heap->decodeRawIntFrom(clientidaddr+1);
                            QString pipekey = this->p->heap->decodeStringFrom(pipekeyaddr+1);
                            this->p->connectRuntimeInstances(processid,clientid,pipekey,pipeaddr);
                        }
                        else{
                            if(this->p->heap->data->getByte(processidaddr)!=ByteCodeInstruction::TypeRunTimeIntegerType){
                                ErrorRegistry::logError("native('Module.connect') expects first argument to be an integer, type :"+QString::number(this->p->heap->data->getByte(processidaddr))+" given at address: #"+QString::number(processidaddr)+", on line:"+QString::number(this->p->lineNumber));
                            }
                            else if(this->p->heap->data->getByte(clientidaddr)!=ByteCodeInstruction::TypeRunTimeIntegerType){
                                ErrorRegistry::logError("native('Module.connect') expects second argument to be an integer, type :"+QString::number(this->p->heap->data->getByte(processidaddr))+" given at address: #"+QString::number(processidaddr)+", on line:"+QString::number(this->p->lineNumber));
                            }
                            else if(this->p->heap->data->getByte(pipekeyaddr)!=ByteCodeInstruction::TypeRunTimeStringType){
                                ErrorRegistry::logError("native('Module.connect') expects third argument to be a string, type :"+QString::number(this->p->heap->data->getByte(processidaddr))+" given at address: #"+QString::number(processidaddr)+", on line:"+QString::number(this->p->lineNumber));
                            }
                            else if(this->p->heap->data->getByte(pipeaddr)!=ByteCodeInstruction::TypeObjectType){
                                ErrorRegistry::logError("native('Module.connect') expects fourth argument to be an object, type :"+QString::number(this->p->heap->data->getByte(processidaddr))+" given at address: #"+QString::number(processidaddr)+", on line:"+QString::number(this->p->lineNumber));
                            }
                            else{
                                ErrorRegistry::logError("native('Module.connect') arguments are not of the type expected.");
                            }
                        }
                    }
                    else{
                        ErrorRegistry::logError("native('Module.connect') expects 4 ("+QString::number(argumentCount-1)+" given) arguments: parent id, child id, pipeName and pipe.");
                    }
                }
                else if(QString::compare(firstArgString,"Module.ready")==0){
                    if(this->p->parent>=0){
                        QMutex* mutex = new QMutex();
                        mutex->lock();
                        if(this->p->parent<Interpreter::runtimeInstances.size()){
                            Interpreter* runtimeInstance = Interpreter::runtimeInstances[this->p->parent];
                            int eventStackAddress = runtimeInstance->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                            if(eventStackAddress!=Architecture::UnsetLocation && eventStackAddress!=Architecture::NULLLocation){
                                if(runtimeInstance->heap->data->getByte(eventStackAddress)==ByteCodeInstruction::TypeEventList){
                                    int eventStackSize = runtimeInstance->heap->decodeRawIntFrom(eventStackAddress+1);
                                    int i=0;
                                    bool found=false;
                                    while(i<eventStackSize && found==false){
                                        int addr = eventStackAddress+5+(4*i);
                                        addr = runtimeInstance->heap->decodeRawIntFrom(addr);
                                        unsigned char type = runtimeInstance->heap->data->getByte(addr);
                                        if(type==ByteCodeInstruction::TypeModuleReadyEvent){
                                            int id = runtimeInstance->heap->decodeRawIntFrom(addr+1);
                                            if(id==this->p->id){
                                                int _data = runtimeInstance->heap->decodeRawIntFrom(addr+9);
                                                if(_data==Architecture::FalseLocation){
                                                    PByteArray* truthy = new PByteArray();
                                                    truthy->setNum(Architecture::TrueLocation);
                                                    runtimeInstance->heap->data->setByte(addr+9,truthy->getByte(0));
                                                    runtimeInstance->heap->data->setByte(addr+10,truthy->getByte(1));
                                                    runtimeInstance->heap->data->setByte(addr+11,truthy->getByte(2));
                                                    runtimeInstance->heap->data->setByte(addr+12,truthy->getByte(3));
                                                }
                                                found=true;
                                            }
                                        }
                                        i++;
                                    }
                                    if(found==false){
                                        ErrorRegistry::logError("Error on native('Module.ready') - parent module does not have a module ready event.");
                                    }
                                }
                                else{
                                    ErrorRegistry::logError("Error! Unwritten Code:EventList does not exist at event queue location.");
                                }
                            }
                            else{
                                ErrorRegistry::logError("Error on  native('Module.ready')! - Parent Thread has no pre-existing event queue, on line:"+QString::number(this->p->lineNumber));
                            }
                        }
                        mutex->unlock();
                        delete mutex;
                    }
                    else{
                        ErrorRegistry::logError("Cannot call native('Module.ready') - module is root instance.");
                    }
                }
                else if(QString::compare(firstArgString,"File.write")==0){
                    this->p->debuggerOut("File.write Event....",true);
                    int secondArgAddress =argsAddress+(13);
                    int dataAddress=argsAddress+(17);
                    dataAddress = this->p->heap->decodeRawIntFrom(dataAddress);
                    secondArgAddress = this->p->heap->decodeRawIntFrom(secondArgAddress);
                    if(this->p->heap->data->getByte(secondArgAddress)!=ByteCodeInstruction::TypeRunTimeStringType){
                        ErrorRegistry::logError("Error - native('Module.load') expects second argument to be a string.");
                    }
                    else if(this->p->heap->data->getByte(dataAddress)!=ByteCodeInstruction::TypeRunTimeStringType && this->p->heap->data->getByte(dataAddress)!=ByteCodeInstruction::TypeConstStringType){
                        ErrorRegistry::logError("Error - native('File.write') expects third argument to be a string,type:"+QString::number(this->p->heap->data->getByte(dataAddress))+", at address:"+QString::number(dataAddress));
                    }
                    else{
                        this->p->debuggerOut("File.write Event is valid....",true);
                        QString secondArgString = this->p->heap->decodeStringFrom(secondArgAddress);
                        auto parts = secondArgString.split(".");
                        QString extension = parts[parts.size()-1];
                        int i=FileWriter::instancesCount;
                        FileWriter* fileWriter = new FileWriter("C:/proto/");
                        customReturn = true;
                        PByteArray* randomInt = new PByteArray();
                        randomInt->setNum(i);
                        randomInt->prepend(ByteCodeInstruction::TypeRunTimeIntegerType);
                        int returnAddress = this->p->heap->data->size();
                        this->p->heap->copyToNew(randomInt,5);
                        this->p->heap->setRETBuffer(returnAddress);
                        int instanceId=i;
                        QMutex* mutex = new QMutex();
                        mutex->lock();
                        mutex->unlock();
                        delete mutex;
                        QThread* fileIOThread = new QThread();
                        fileWriter->moveToThread(fileIOThread);
                        fileIOThread->start();
                        QString __data = this->p->heap->decodeStringFrom(dataAddress);
                        __data = __data.split("\\n").join("\n").split("\\t").join("\t");
                        PByteArray* _data = new PByteArray();
                        i=0;
                        while(i<__data.length()){
                            _data->append(__data.at(i).toLatin1());
                            i++;
                        }
                        int callbackAddress=argsAddress+(21);
                        if(argumentCount>=4){
                            callbackAddress = this->p->heap->decodeRawIntFrom(callbackAddress);
                            if(this->p->heap->data->getByte(callbackAddress)==ByteCodeInstruction::TypeLambdaType){
                                this->p->debuggerOut("File.write Event, setting callback....",true);
                                int eventListAddress = this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                                if(eventListAddress!=Architecture::NULLLocation && eventListAddress!=Architecture::UnsetLocation && this->p->heap->data->getByte(eventListAddress)==ByteCodeInstruction::TypeEventList){
                                    this->p->debuggerOut("Pre-existing eventqueue...",true);
                                    int eventAddress = this->p->heap->data->size();
                                    PByteArray* timerEvent = new PByteArray();
                                    PByteArray* enquedTime = new PByteArray();
                                    enquedTime->setNum(instanceId);
                                    PByteArray* callbackPointer = new PByteArray();
                                    callbackPointer->setNum(callbackAddress);
                                    timerEvent->resize(13);
                                    timerEvent->setByte(0,ByteCodeInstruction::TypeFileWrittenEvent);
                                    timerEvent->setByte(1,enquedTime->getByte(0));
                                    timerEvent->setByte(2,enquedTime->getByte(1));
                                    timerEvent->setByte(3,enquedTime->getByte(2));
                                    timerEvent->setByte(4,enquedTime->getByte(3));
                                    timerEvent->setByte(5,callbackPointer->getByte(0));
                                    timerEvent->setByte(6,callbackPointer->getByte(1));
                                    timerEvent->setByte(7,callbackPointer->getByte(2));
                                    timerEvent->setByte(8,callbackPointer->getByte(3));
                                    PByteArray* falsey = new PByteArray();
                                    falsey->setNum(Architecture::FalseLocation);
                                    timerEvent->setByte(9,falsey->getByte(0));
                                    timerEvent->setByte(10,falsey->getByte(1));
                                    timerEvent->setByte(11,falsey->getByte(2));
                                    timerEvent->setByte(12,falsey->getByte(3));
                                    this->p->heap->copyToNew(timerEvent,13);
                                    this->p->heap->appendToArray(this->p->heap->decodeRawIntFrom(Architecture::EventStackAddress),eventAddress);
                                }
                                else{
                                    this->p->debuggerOut("Making an eventqueue...",true);
                                    int newEventAddress = this->p->heap->data->size();
                                    PByteArray* timerEvent = new PByteArray();
                                    PByteArray* enquedTime = new PByteArray();
                                    enquedTime->setNum(instanceId);
                                    PByteArray* callbackPointer = new PByteArray();
                                    callbackPointer->setNum(callbackAddress);
                                    timerEvent->resize(13);
                                    timerEvent->setByte(0,ByteCodeInstruction::TypeFileWrittenEvent);
                                    timerEvent->setByte(1,enquedTime->getByte(0));
                                    timerEvent->setByte(2,enquedTime->getByte(1));
                                    timerEvent->setByte(3,enquedTime->getByte(2));
                                    timerEvent->setByte(4,enquedTime->getByte(3));
                                    timerEvent->setByte(5,callbackPointer->getByte(0));
                                    timerEvent->setByte(6,callbackPointer->getByte(1));
                                    timerEvent->setByte(7,callbackPointer->getByte(2));
                                    timerEvent->setByte(8,callbackPointer->getByte(3));
                                    PByteArray* falsey = new PByteArray();
                                    falsey->setNum(Architecture::FalseLocation);
                                    timerEvent->setByte(9,falsey->getByte(0));
                                    timerEvent->setByte(10,falsey->getByte(1));
                                    timerEvent->setByte(11,falsey->getByte(2));
                                    timerEvent->setByte(12,falsey->getByte(3));
                                    this->p->heap->copyToNew(timerEvent,13);
                                    int newEventListAddress = this->p->heap->data->size();
                                    PByteArray* eventList = new PByteArray();
                                    PByteArray* listSize = new PByteArray();
                                    listSize->setNum(1);
                                    PByteArray* firstEntry = new PByteArray();
                                    firstEntry->setNum(newEventAddress);
                                    eventList->resize(9);
                                    eventList->setByte(0,ByteCodeInstruction::TypeEventList);
                                    eventList->setByte(1,listSize->getByte(0));
                                    eventList->setByte(2,listSize->getByte(1));
                                    eventList->setByte(3,listSize->getByte(2));
                                    eventList->setByte(4,listSize->getByte(3));
                                    eventList->setByte(5,firstEntry->getByte(0));
                                    eventList->setByte(6,firstEntry->getByte(1));
                                    eventList->setByte(7,firstEntry->getByte(2));
                                    eventList->setByte(8,firstEntry->getByte(3));
                                    this->p->heap->copyToNew(eventList,9);
                                    PByteArray* _newEventListAddress = new PByteArray();
                                    _newEventListAddress->setNum(newEventListAddress);
                                    this->p->heap->data->encodeRawIntegerAt(Architecture::EventStackAddress,newEventListAddress);
                                    this->p->debuggerOut("Event Stack Address:"+QString::number(newEventListAddress),true);
                                }
                            }
                        }
                        fileWriter->nativeWriteRequest(secondArgString,*_data,this->p->id);
                    }
                }
                else{
                    ErrorRegistry::logError("Error - Native call does not understand first argument '"+firstArgString+"'.");
                }
            }
            else{
                ErrorRegistry::logError("Error - Native call expects first argument to be string, type given:"+QString::number(this->p->heap->data->getByte(firstArgAddress))+", at address:"+QString::number(firstArgAddress)+" on line:"+QString::number(this->p->lineNumber));
            }
        }
        else{
            ErrorRegistry::logError("Error - Native call with non-arguments list.");
        }
    }
    else{
        this->p->debuggerOut(ErrorRegistry::printLog(),true);
    }
    if(customReturn==false){
        this->p->heap->setRETBuffer(Architecture::NULLLocation);
    }
    qDebug()<<"Finished native execution";
}
