#include "interpreter.h"
#include "bytecodeinstruction.h"
#include "heap.h"
#include <QCoreApplication>
#include <QDebug>
#include <QtGui>
#include <QWindow>
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTextBrowser>
#include <QElapsedTimer>
#include <cstdlib>
#include <chrono>
#include <ctime>


#include "architecture.h"

#include "nativestore.h"
#include "widgetmanager.h"
#include "filereader.h"
#include "filewriter.h"

#include "pipemanager.h"
#include "heap.h"
#include "compiler.h"

#include "errorregistry.h"


#include <iostream>
//using namespace std;
#include <cstdlib>
#include <sys/timeb.h>

int getMilliCount(){
    timeb tb;
    ftime(&tb);
    int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

int getMilliSpan(int nTimeStart){
    int nSpan = getMilliCount() - nTimeStart;
    if(nSpan < 0)
        nSpan += 0x100000 * 1000;
    return nSpan;
}

Interpreter::Interpreter(int parent,int id){
    this->parent=parent;
    this->id=id;
}

QVector<Interpreter*> Interpreter::runtimeInstances={};

int decodeRawInt(PByteArray* buf){
    return buf->toInt();
}

void Interpreter::yield(){
    PByteArray* rpnStore = new PByteArray();
    rpnStore->resize(Architecture::SIZE_OF_INITIAL_HEAP_STORE - Architecture::RPNAddress);
    int i=0;
    while(i<rpnStore->size()){
        rpnStore->setByte(i,this->heap->data->getByte(Architecture::RPNAddress+i));
        i++;
    }
    int argAddress = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
    int thisAddress = this->heap->decodeRawIntFrom(Architecture::ThisAddress);
    this->checkEventStack();
    i=0;
    while(i<rpnStore->size()){
        this->heap->data->setByte(Architecture::RPNAddress+i,rpnStore->getByte(i));
        i++;
    }
    this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argAddress);
    this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,thisAddress);
}

void Interpreter::checkEventStack(){

    this->debuggerOut("Checking event Stack!",true);

    QMutex* mutex = new QMutex();
    mutex->lock();
    int eventStack = this->heap->decodeRawIntFrom(Architecture::EventStackAddress);
    if(eventStack<this->heap->data->size() && eventStack!=Architecture::UnsetLocation && eventStack!=Architecture::NULLLocation){
        unsigned char typeBit = this->heap->data->getByte(eventStack);
        if(typeBit==ByteCodeInstruction::TypeEventList){
            int size = this->heap->decodeRawIntFrom(eventStack+Architecture::TypeEventListGutterToSize);
            int i=0;
            while(i<size){
                int addr = this->heap->decodeRawIntFrom(eventStack+Architecture::TypeEventListGutter+(i*Architecture::TypeEventListEntrySize));
                unsigned char eType = this->heap->data->getByte(addr);
                if(eType==ByteCodeInstruction::TypeTimerEventType){
                    int timeEvent = this->heap->decodeRawIntFrom(addr+Architecture::TypeTimerEventGutterToTime);
                    int currentTime = (int)time(0);
                    int callbackAddress = this->heap->decodeRawIntFrom(addr+Architecture::TypeTimerEventGutterToCallBack);
                    if(currentTime>=timeEvent){
                        int argsAddress = this->heap->data->size();
                        this->heap->constructNewArgsList(Architecture::NULLLocation);
                        mutex->unlock();
                        this->executeLambda(callbackAddress,argsAddress,Architecture::NULLLocation);
                        mutex->lock();
                        this->heap->removeFromArray(eventStack,i);
                    }
                }
                else if(eType==ByteCodeInstruction::TypeModuleReadyEvent){
                    int processId = this->heap->decodeRawIntFrom(addr+Architecture::TypeModuleReadyEventGutterToId);
                    int callbackAddress = this->heap->decodeRawIntFrom(addr+Architecture::TypeModuleReadyEventGutterToCallBack);
                    int ready = this->heap->decodeRawIntFrom(addr+Architecture::TypeModuleReadyEventGutterToState);
                    if(ready==Architecture::FalseLocation){
                    }
                    else if(ready==Architecture::TrueLocation){
                        if(this->heap->data->getByte(callbackAddress)==ByteCodeInstruction::TypeLambdaType){
                            int idAddress = this->heap->data->size();
                            PByteArray* myInt = new PByteArray();
                            myInt->encodeAsRunTimeInteger(0,processId);
                            this->heap->copyToNew(myInt,Architecture::TypeRunTimeIntegerSize);
                            int argsAddress = this->heap->data->size();
                            this->heap->constructNewArgsList(idAddress);
                            mutex->unlock();
                            this->executeLambda(callbackAddress,argsAddress,Architecture::NULLLocation);
                            mutex->lock();
                            this->heap->removeFromArray(eventStack,i);
                        }
                        else{
                            ErrorRegistry::logError("Malformed Event Object.");
                        }
                    }
                    else{
                        ErrorRegistry::logError("Malformed Event Object.");
                    }
                }
                else if(eType==ByteCodeInstruction::TypeFileWrittenEvent){
                    int callbackAddress = this->heap->decodeRawIntFrom(addr+Architecture::TypeFileWrittenEventGutterToCallBack);
                    int ready = this->heap->decodeRawIntFrom(addr+Architecture::TypeFileWrittenEventGutterToState);
                    if(ready==Architecture::FalseLocation){
                    }
                    else if(ready==Architecture::TrueLocation){
                        if(this->heap->data->getByte(callbackAddress)==ByteCodeInstruction::TypeLambdaType){
                            mutex->unlock();
                            this->executeLambda(callbackAddress,Architecture::UnsetLocation,Architecture::NULLLocation);
                            mutex->lock();
                            this->heap->removeFromArray(eventStack,i);
                        }
                        else{
                            ErrorRegistry::logError("Malformed Event Object.");
                        }
                    }
                    else{
                        ErrorRegistry::logError("Malformed Event Object.");
                    }
                }
                else{
                    ErrorRegistry::logError("Unrecognised Event Type.");
                }
                i++;
            }
            if(i==0){
                this->heap->data->encodeRawIntegerAt(Architecture::EventStackAddress,Architecture::NULLLocation);
            }
        }
    }
    else{
    }
    mutex->unlock();
    delete mutex;
}

void Interpreter::connectRuntimeInstances(int parentid,int childid,QString propertyKey,int objectAddress){
    PWidget* pwid = new PWidget(300,300);
    pwid->setTitle("Connector....");
    pwid->show();
}

void Interpreter::executeLambda(int lambdaAddress,int argumentsAddress,int contextAddress){
    if(ErrorRegistry::errorCount==0){
        if((argumentsAddress==Architecture::UnsetLocation || this->heap->data->getByte(argumentsAddress)==ByteCodeInstruction::TypeArgsListType) &&
           this->heap->data->getByte(lambdaAddress)==ByteCodeInstruction::TypeLambdaType){
            this->heap->data->encodeRawIntegerAt(Architecture::ArgumentsAddress,argumentsAddress);
            this->heap->data->encodeRawIntegerAt(Architecture::ThisAddress,contextAddress);
            int instAddress = this->heap->decodeRawIntFrom(lambdaAddress+Architecture::TypeLambdaGutterToInstructionSet);
            //instAddress+=((this->heap->initThreshold-Architecture::SIZE_OF_INITIAL_HEAP_STORE)+Architecture::TypeHeapFlushSize);//edit!!!!!!!

            //added

            instAddress = (instAddress*13)+10+(this->heap->initThreshold-Architecture::SIZE_OF_INITIAL_HEAP_STORE);

            qDebug()<<"inst address:"<<instAddress;

            int lexId = this->heap->decodeRawIntFrom(lambdaAddress+Architecture::TypeLambdaGutterToLexId);
            int currentStackFrameAddress = this->heap->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
            this->heap->addStackFrame(currentStackFrameAddress,true,lexId);
            currentStackFrameAddress = this->heap->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
            this->heap->addStackFrame(currentStackFrameAddress,false,lexId);
            this->executeInstructionSetFrom(instAddress);
            if(ErrorRegistry::errorCount==0){
                this->heap->clearTopStackFrame();
            }
        }
        else{
            ErrorRegistry::logError("Error! - Function executed with an arguments address which is NOT an argslist on line:"+QString::number(this->lineNumber));
        }
    }
    else{
        this->debuggerOut(ErrorRegistry::printLog(),true);
    }
}

void Interpreter::executeRPN(PByteArray* contents, char buffer, int i){
    if(ErrorRegistry::errorCount==0){
        PByteArray* Abytes = new PByteArray();
        PByteArray* Bbytes = new PByteArray();
        int e=0;
        i++;
        while(e<Architecture::RawIntegerSize){
            Abytes->resize(Abytes->size()+1);
            Abytes->setByte(Abytes->size()-1,contents->getByte(i));
            Bbytes->resize(Bbytes->size()+1);
            Bbytes->setByte(Bbytes->size()-1,contents->getByte(i+Architecture::RawIntegerSize));
            i++;
            e++;
        }
        int A_Address = Abytes->toInt();
        int B_Address = Bbytes->toInt();
        //this->debuggerOut("B Address Initial:"+QString::number(B_Address),true);
        QString _operator;
        i+=Architecture::RawIntegerSize;
        if(contents->getByte(i)!=NULL){
            char x = contents->getByte(i);
            _operator = QString(x);
           if(contents->getByte(i+1)!=NULL){
               char x2 = contents->getByte(i+1);
               _operator+=x2;
               if(contents->getByte(i+2)!=NULL){
                   char x3 = contents->getByte(i+2);
                   _operator+=x3;
               }
           }
        }
        unsigned char A_Type = this->heap->data->getByte(A_Address);
        unsigned char B_Type = this->heap->data->getByte(B_Address);
        if(A_Address==B_Address && (A_Address>=Architecture::RPNAddress && A_Address<Architecture::SIZE_OF_INITIAL_HEAP_STORE)){
            ErrorRegistry::logError("Critical Error!!!! - A and B are both pointers to the same RPN buffer.");
        }
        else{
            A_Address = this->heap->RPNAndThisDeref(A_Address);
            A_Type = this->heap->data->getByte(A_Address);
            B_Address = this->heap->RPNAndThisDeref(B_Address);
            //this->debuggerOut("B Address after this and deref:"+QString::number(B_Address),true);
            B_Type = this->heap->data->getByte(B_Address);
        }
        if(_operator.length()==0 || QString::compare(_operator,"=")==0 || QString::compare(_operator,",")==0 || QString::compare(_operator,"$")==0 || QString::compare(_operator,".")==0 || QString::compare(_operator,"^")==0){
            //this->debuggerOut("Evaluative operation",true);
            this->eru->eval(A_Address,B_Address,_operator,buffer,i);
        }
        else{
            //this->debuggerOut("Arithmetic operation",true);
            this->alu->eval(A_Address,B_Address,_operator,buffer);
        }
    }
    else{
        //this->debuggerOut(ErrorRegistry::printLog(),true);
    }
}


void Interpreter::launchDebugger(){
    this->nativeStore->resolve(NativeStore::DebugAddress,{"launch"});
}

void Interpreter::debuggerOut(QString string,bool debug){
    if(debug==true){
        this->nativeStore->resolve(NativeStore::DebugAddress,{"out","<font color=#6666ff>Debug From Thread:</font><font color=#ff6666>&nbsp;&nbsp;"+this->threadName+"</font><br><font color=#6666ff>"+string+"</font><br><font color=#6666ff>************************</font>"});
    }
    else{
    }
}

void Interpreter::debuggerOut(QString string){
}

QVector<Pipe*> Interpreter::Pipes={};

int Interpreter::getLexicalID(){
    int _stackAddress = this->heap->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
    int lexId = this->heap->decodeRawIntFrom(_stackAddress+Architecture::TypeStackFrameGutterToLexId);
    while(_stackAddress>this->heap->initThreshold &&
          this->heap->data->getByte(_stackAddress)==Architecture::TypeFixedScope){
        _stackAddress = this->heap->decodeRawIntFrom(_stackAddress+Architecture::TypeStackFrameGutterToParent);
        lexId = this->heap->decodeRawIntFrom(_stackAddress+Architecture::TypeStackFrameGutterToLexId);
    }
    return lexId;
}

void Interpreter::executeInstructionSetFrom(int i){
    //this->debuggerOut("Executing instrucitn set from:"+QString::number(i),true);
    while(i<contents->size() && halt==false){
        if(ErrorRegistry::errorCount>0){
            ErrorRegistry::logError("Error on line:"+QString::number(this->lineNumber));
            //this->debuggerOut(ErrorRegistry::printLog(),true);
            halt=true;
        }
        else{
            ////this->debuggerOut("Interpreter::executeInstructionSetFrom("+QString::number(i)+");",true);
            if(contents->getByte(i)==ByteCodeInstruction::TypeLET){
                int address = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
                char start = this->heap->data->getByte(address);
                //this->debuggerOut("let statement, letting:#"+QString::number(address),true);
                if(start==ByteCodeInstruction::TypeKeyType){
                    QString name = this->heap->decodeStringFrom(address);
                    this->heap->addToCurrentStackFrame(name,address);
                }
                else{
                    halt=true;
                }
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeFIX){
                int address = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
                char start = this->heap->data->getByte(address);
                if(start==ByteCodeInstruction::TypeKeyType){
                    QString name = this->heap->decodeStringFrom(address);
                    this->heap->addToCurrentStackFrameAsFixed(name,address);
                }
                else{
                    halt=true;
                }
            }
            else if(contents->getByte(i)>=Architecture::RPNAddress && contents->getByte(i)<=Architecture::SIZE_OF_INITIAL_HEAP_STORE){
                //this->debuggerOut("Executing RPN:"+QString::number(contents->getByte(i)),true);
                executeRPN(contents,contents->getByte(i),i);
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeCOMP){
                //this->debuggerOut("Executing CMP:"+QString::number(contents->getByte(i)),true);
                executeRPN(contents,Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,i);
                //more comp logic
                if(this->heap->LastRPNBufferIsTruthy()==true){
                    //this->debuggerOut("Truthy!",true);
                    i+=Architecture::TypeInstructionSize*2;
                    jumped=true;
                    failedLastComp=false;
                }
                else{
                    //this->debuggerOut("Falsey!",true);
                    failedLastComp=true;
                }
                this->heap->setRPNBuffer(Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,Architecture::FalseLocation);
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeFCOMP){
                if(failedLastComp==true){
                    executeRPN(contents,Architecture::SIZE_OF_INITIAL_HEAP_STORE-Architecture::RawIntegerSize,i);
                    if(this->heap->LastRPNBufferIsTruthy()==true){
                        i+=Architecture::TypeInstructionSize*2;
                        jumped=true;
                        failedLastComp=false;
                    }
                    else{
                        failedLastComp=true;
                    }
                }
                else{
                }
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeJMP){


                int d = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
                int dx=0;
                //this->debuggerOut("Executing JUMP, JUMPing "+QString::number(d)+" instructions.",true);
                if(d>0){
                    dx = d*Architecture::TypeInstructionSize;
                }
                i+=dx;
                jumped=true;
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeBackJMP){
                int d = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
                //this->debuggerOut("Jump back "+QString::number(d)+" steps.",true);
                int dx=0;
                if(d>0){
                    dx = d*Architecture::TypeInstructionSize;
                }
                i-=dx;
                jumped=true;
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeLEXStartMarker){

                //this->debuggerOut("Lex-start marker encountered at byte:"+QString::number(i),true);
                qDebug()<<"Lex-start marker encountered at byte:"<<QString::number(i);

                int lexID = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
                if(lexID!=this->getLexicalID()){
                    //this->debuggerOut("Skipping to Lex-End Marker:"+QString::number(i),true);
                    bool matched=false;
                    while(i<contents->size() && matched==false){
                        if(contents->getByte(i)==ByteCodeInstruction::TypeLEXEndMarker){
                            int lexID2 = contents->decodeRawIntFrom(i+1);
                            if(lexID==lexID2){
                                matched=true;
                            }
                        }
                        i+=Architecture::TypeInstructionSize;
                        jumped=true;
                    }
                }
                else{
                }
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeLEXEndMarker){
                this->halt=true;
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeReturnMarker){

                //this->debuggerOut("Return Marker Encountered...",true);

                executeRPN(contents,Architecture::ReturnAddress,i);
                this->halt=true;
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeEARG){
                jumped=true;
                int count=0;
                while(i<contents->size() && contents->getByte(i)==ByteCodeInstruction::TypeEARG){
                    int address = contents->decodeRawIntFrom(i+1);
                    char start = this->heap->data->getByte(address);
                    if(start==ByteCodeInstruction::TypeKeyType){
                        QString name = this->heap->decodeStringFrom(address);
                        this->heap->addToCurrentStackFrame(name,address);
                    }
                    else{
                        halt=true;
                    }
                    //followed by the assignment of that letted var to the corresponding entry in the arguments array
                    int argumentAddr = this->heap->decodeRawIntFrom(Architecture::ArgumentsAddress);
                    //size check this array
                    int argCount = this->heap->decodeRawIntFrom(argumentAddr+(Architecture::TypeArgumentsListGutterToSize));
                    if(count<argCount){
                        argumentAddr+=Architecture::TypeArgumentsListGutter+(Architecture::TypeArgumentsListEntrySize*count);
                        argumentAddr = this->heap->data->decodeRawIntFrom(argumentAddr);
                        this->heap->data->encodeRawIntegerAt(this->heap->valueAddressInStackFrames(this->heap->decodeStringFrom(address),address),argumentAddr);
                        count++;
                    }
                    i+=Architecture::TypeInstructionSize;
                }
            }
            else if(contents->getByte(i)==ByteCodeInstruction::TypeLineNumberSetType){
                this->lineNumber = contents->decodeRawIntFrom(i+Architecture::TypeInstructionGutterToA);
            }
            else{
                ErrorRegistry::logError("Malformed ByteCode:Unrecognised Instruction Code:"+QString::number(contents->getByte(i))+", at address:"+QString::number(i)+" on line:"+QString::number(this->lineNumber));
            }
            if(jumped==true){
                jumped=false;
            }
            else{
                i+=Architecture::TypeInstructionSize;
            }
        }
    }
    if(halt==true){
        halt=false;
    }
}

void Interpreter::launch(QString filename, PByteArray contents2){
    //we will run in debug mode for starters, so we will launch a window...
    this->parent=parent;
    PByteArray* contents = &contents2;
    this->lineNumber=0;
    QMutex* qMutex = new QMutex();
    qMutex->lock();
    if(PipeManager::Pipes.size()==0){
        PipeManager::Pipes.append(new Pipe(filename,"sys"));
        this->pipes.append(PipeManager::Pipes.size()-1);
        PipeManager::Pipes.append(new Pipe(filename,"app.debug"));
        this->pipes.append(PipeManager::Pipes.size()-1);
        PipeManager::Pipes.append(new Pipe(filename,"pwidget.manager"));
        this->pipes.append(PipeManager::Pipes.size()-1);
    }
    else{
        PipeManager::Pipes.append(new Pipe(filename,"sys"));
        this->pipes.append(PipeManager::Pipes.size()-1);
        this->pipes.append(1);
        this->pipes.append(2);
    }
    qMutex->unlock();
    delete qMutex;
    this->nativeStore = new NativeStore(this);
    this->nativeStore->wirePipe(this->pipes[0],NativeStore::SysOutAddress);
    this->nativeStore->wirePipe(this->pipes[1],NativeStore::DebugAddress);
    this->nativeStore->wirePipe(this->pipes[2],NativeStore::PWidgetManager);
    this->launchDebugger();
    this->heap = new Heap(this);
    this->alu= new ALU(this);
    this->eru= new ERU(this);
    this->nru= new NRU(this);
    this->failedLastComp=false;
    int start = getMilliCount();
    int o=0;
    while(o<this->args.size()){
        //this->debuggerOut("argv["+QString::number(o)+"]:"+this->args[o],true);
        o++;
    }
    if(contents->getByte(0)==ByteCodeInstruction::TypeRPNReserve){
        int rpnCount = contents->decodeRawIntFrom(1);
        //this->debuggerOut(QString::number(rpnCount),true);
//        if(contents->getByte(Architecture::TypeInstructionSize)==ByteCodeInstruction::TypeHeapFlush){
        if(contents->getByte(Architecture::TypeHeapGutterToFlush)==ByteCodeInstruction::TypeHeapFlush){
            //set up initial heap values
            this->heap->setUp(rpnCount);//null
            int i=1+Architecture::TypeHeapGutterToFlush;//to skip over rpn
            int hpCount = contents->decodeRawIntFrom(i);
            //this->debuggerOut("Flushing "+QString::number(hpCount)+" bytes to the heap.",true);
            //this->debuggerOut("Flushing "+QString::number(hpCount)+" bytes to the heap.",true);
//            i=i+hpCount;
            i+=4;
            int l = i+hpCount;
//            while(i<(hpCount+(Architecture::TypeInstructionSize+Architecture::TypeHeapFlushSize)) && i<contents->size()){
            while(i<l && i<contents->size()){
                this->heap->accept(contents->getByte(i));
                i++;
            }
            this->heap->setInitThreshold();
            //this->debuggerOut("Pretty Print Heap:\n"+this->heap->initThresholdString(),true);
            this->heap->addStackFrame(Architecture::NULLLocation,true,0);
            int lastStackFrameAddress = this->heap->decodeRawIntFrom(Architecture::CurrentStackFrameAddress);
            this->heap->addStackFrame(lastStackFrameAddress,false,0);
            if(i<contents->size()){
                this->halt=false;
                this->jumped=false;
                this->contents=contents;
                this->executeInstructionSetFrom(i);
                if(ErrorRegistry::errorCount>0){
                    this->debuggerOut(ErrorRegistry::printLog(),true);
                }
                else{
                    this->debuggerOut("Interpretation finished, checing event stack...",true);
                    this->heap->clearTopStackFrame();
                    int eventStackAddress = this->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                    while(eventStackAddress<this->heap->data->size() && eventStackAddress!=Architecture::NULLLocation && eventStackAddress!=Architecture::UnsetLocation && ErrorRegistry::errorCount==0){
                        this->checkEventStack();
                        eventStackAddress = this->heap->decodeRawIntFrom(Architecture::EventStackAddress);
                    }
                    this->debuggerOut("Finished checking event stack...",true);
                }
            }
            else{
                ErrorRegistry::logError("? No Instruction codes to execute...");
            }
        }
        else{
            ErrorRegistry::logError("Heap marker NOT where expected.");
        }
    }
    else{
        ErrorRegistry::logError("RPN Reserve marker NOT where expected.");
    }

    int ms = getMilliSpan(start);


    if(ErrorRegistry::errorCount>0){
        this->debuggerOut(ErrorRegistry::printLog(),true);
    }

    int secs = (float)ms/1000.00;

    this->debuggerOut("Interpretation finished, took "+QString::number(ms)+" milliseconds.("+QString::number(secs)+") seconds.",true);
}
