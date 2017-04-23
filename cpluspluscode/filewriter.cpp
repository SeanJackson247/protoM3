#include "filewriter.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMutex>

#include "interpreter.h"

#include "errorregistry.h"

#include "bytecodeinstruction.h"
#include "architecture.h"

FileWriter::FileWriter(QString outputDirectory){
    this->dir = outputDirectory;
    this->id=FileWriter::instancesCount;
    FileWriter::instancesCount++;
}

int FileWriter::instancesCount=0;

void FileWriter::writeRequest(QString filename, PByteArray contents){
    qDebug()<<"FileWrite recieving final write request...:"+this->dir+filename;
    QFile file(this->dir+filename);

    QByteArray contentsQ;
    contentsQ.resize(contents.size());

    int i=0;
    while(i<contents.size()){
        contentsQ[i]=contents.getByte(i);
        i++;
    }

    if (file.open(QIODevice::WriteOnly)) {
       QTextStream stream(&file);
       stream << contentsQ;
    }

    qDebug()<<"FileWrite completed final write request...:"+this->dir+filename;
}

void FileWriter::nativeWriteRequest(QString filename, PByteArray contents,int runtimeInstanceId){
    qDebug()<<"FileWrite recieving final write request...:"+this->dir+filename;

    this->writeRequest(filename,contents);

    /*QFile file(this->dir+filename);

    QByteArray contentsQ;
    contentsQ.resize(contents.size());

    int i=0;
    while(i<contents.size()){
        contentsQ[i]=contents.getByte(i);
        i++;
    }

    if (file.open(QIODevice::ReadWrite)) {
       QTextStream stream(&file);
       stream << contentsQ;
    }*/

    qDebug()<<"FileWrite completed final write request...:"+this->dir+filename;

    QMutex* mtx = new QMutex();
    mtx->lock();

    Interpreter* runtimeInstance = Interpreter::runtimeInstances[runtimeInstanceId];
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
                if(type==ByteCodeInstruction::TypeFileWrittenEvent){
                    int id = runtimeInstance->heap->decodeRawIntFrom(addr+1);
                    if(id==this->id){
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
                    //addr =
                    //PByteArray* _data = new PByteArray();
                    //_data->setNum(Architecture::TrueLocation);
                }
                i++;
            }
            if(found==false){
                ErrorRegistry::logError("Error on native('File.write') - target module does not have a file.write event.");
            }
        }
        else{
            ErrorRegistry::logError("Error! Unexpected data at event queue location.");
        }
    }
    else{
        //ErrorRegistry::logError("Error on  native('File.write')! - Target Thread has no pre-existing event queue.");
    }


    mtx->unlock();
    delete mtx;
}

void FileWriter::debugRequest(QString filename,QString contents){
    qDebug()<<"FileWrite recieving debug request...:"+this->dir+filename;
    QFile file(this->dir+filename);
    if (file.open(QIODevice::WriteOnly)) {
       QTextStream stream(&file);
       stream << contents;
    }
    file.close();
}
