#include "compiler.h"

#include <QDebug>
#include <QTextEncoder>

#include <QFile>
#include <QTextStream>
#include <QIODevice>

#include <QElapsedTimer>
#include <QMutex>

#include "filereader.h"
#include "line.h"
#include "linestatic.h"
#include "errorregistry.h"
#include "heapstore.h"

Compiler::Compiler(){}

void Compiler::recieveIncludes(QString fileName,QVector<Line*> lines){
    qDebug()<<"Recieving Includes...";
    int e=0;
    while(e<lines.size()){
        qDebug()<<lines[e]->toString(0);
        e++;
    }
    if(lines.size()==2 && QString::compare(lines.first()->getData().first()->getData().first()->getString(),"ERROR")==0){
        qDebug()<<"FILEREADER ERROR";
        QString str = "ERROR:"+(lines.last()->toString(0));
        ErrorRegistry::logError(str);
        this->includeURLs={};
    }
    else{
        qDebug()<<"Appending includes lines...";

        QMutex* qMutex = new QMutex();

        qMutex->lock();

        int i=lines.size()-1;
        while(i>=0){
            qDebug()<<"looping";
            this->linesStore.prepend(lines[i]);
            qDebug()<<"between loop";
            if(this->includeURLs.size()>0){
                this->includeURLs.removeLast();
            }
            qDebug()<<"finished loop";
            i--;
        }
        if(i<=0){
            this->includeURLs={};
        }
        qDebug()<<"Escaped loop!";
        qMutex->unlock();

        delete qMutex;
    }
    QMutex* qMutex = new QMutex();

    qMutex->lock();

    if(this->includeURLs.size()==0){
        if(this->isRoot==true){
            qDebug()<<"Final Compilation...";
            emit this->finalCompilation(this->exportDirectory,this->linesStore);
        }
        else{
            qDebug()<<"Exporting to higher level...";
            emit this->exportToInclusion(this->exportDirectory,this->linesStore);
        }
    }

    qMutex->unlock();

    delete qMutex;

}

void Compiler::finalCompilation(QString fileName, QVector<Line *> lines){
    qDebug()<<"Final Compilation...";
    if(ErrorRegistry::errorCount>0){
        QString finalString = ErrorRegistry::printLog();
        emit debugScript(fileName,finalString);
    }
    else{
        QString finalString="\nPost-File Inclusion Tree::\n";

        lines = LineStatic::stripIncludes(lines);

        finalString+="\nPre-Control Flow flattening::\n";

        int i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }

        lines = LineStatic::flattenControlFlows(lines);

        finalString+="\nPost-Control Flow flattening::\n";

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }
        /*Now a bunch of data needs to be transferred from the instruction stream to the heap*/

        lines = LineStatic::labelLexicalScopes(lines);

        lines = LineStatic::flushToHeapStore(lines);

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }

        i=0;
        while(i<HeapStore::lexes.size()){
            finalString+="\nheap-rule[lex:'"+QString::number(HeapStore::lexes[i])+
            "' maps to : '"+
            QString::number(HeapStore::pars[i])+
            "'];\n";
            i++;
        }

        i=0;
        while(i<HeapStore::primitives.size()){
            qDebug()<<"has primitives...";
            finalString+="\n"+QString::number(i)+"heap-primitive["+HeapStore::primitives[i]+"];\n";
            i++;
        }

        i=0;
        while(i<HeapStore::literals.size()){
            qDebug()<<"has literals...";
            finalString+="\n"+QString::number(i)+"heap-literal["+HeapStore::literals[i]->toString(0)+"];\n";
            i++;
        }

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }

        /*WE are finally here!! time to finally compile...*/

        QVector<ByteCodeInstruction*> byteCodeInstructions = LineStatic::compile(lines);

        finalString+="\nBYTECODE INSTRUCTIONS:\n";

        qDebug()<<"debugging...";

        i=0;
        while(i<byteCodeInstructions.size()){
            finalString+=QString::number(i)+":"+byteCodeInstructions[i]->toString(0);
            i++;
        }

        //final byte code clean-up

        byteCodeInstructions = ByteCodeInstruction::cleanup(byteCodeInstructions);

        byteCodeInstructions = ByteCodeInstruction::ammendControlFlows(byteCodeInstructions);

        finalString+="\nCLEANED-UP BYTECODE INSTRUCTIONS:\n";

        qDebug()<<"debugging...";

        i=0;
        while(i<byteCodeInstructions.size()){
            finalString+=QString::number(i)+":"+byteCodeInstructions[i]->toString(0);
            i++;
        }

        qDebug()<<"Passed bytecode instructions to final string...";

        //now the heapstore pastes it together for us:

        PByteArray* bytes = HeapStore::encode(byteCodeInstructions);
        qDebug()<<"Got bytes!";

        //we spit out the debug info...

        qDebug()<<"Emitting debug...";

        emit debugScript(fileName,finalString);
        //and the compiled version...
        qDebug()<<"Emitted debug...";

        QString finalFileName = "";

        i=0;
        bool esc=false;
        while(i<fileName.size() && esc==false){
            if(fileName[i]=='.'){
                esc=true;
            }
            else{
                finalFileName.append(fileName[i]);
            }
            i++;
        }
        finalFileName+=".prt";
        qDebug()<<"Emitting work complete...";

        emit workCompleted(finalFileName,*bytes);

        qDebug()<<"Emitted work complete...";
    }
}

void Compiler::doWork(QString fileName,QVector<QString> data){
    qDebug()<<"Compiling...";
    this->exportDirectory = fileName;
    if(data.size()==2 && QString::compare(data.first(),"ERROR")==0){
        qDebug()<<"Error Encountered...";
        emit exportToInclusion(fileName,{new Line(0,"ERROR"),new Line(0,data.last())});
    }
    else{
        qDebug()<<"File read..";
        QElapsedTimer myTimer;
        myTimer.start();

        QString finalString="TEST\n";

        int i=0;
        while(i<data.size()){
            finalString+=data[i]+"\n";
            i++;
        }

        i=0;
        int lineNumber=1;
        QVector<Line*> lines;
        Line::root = new Line(Line::TypeRoot);

        while(i<data.size()){
            lines.append(new Line(lineNumber,data[i]));
            lineNumber++;
            i++;
        }

        finalString+="\nRaw Lines:\n";

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }
        lines = LineStatic::removeEmptyLines(lines);

        finalString+="\nRemoved empty Lines:\n";

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }

        lines = LineStatic::validateAsFirstOrderLines(0,lines);

        if(ErrorRegistry::errorCount==0){
            finalString+="\nAfter first order validation::\n";
            qDebug().noquote()<<"\nAfter first order validation::\n";

            i=0;
            while(i<lines.size()){
                finalString+=lines[i]->toString(0);
                qDebug().noquote()<<lines[i]->toString(0);
                i++;
            }

            //compile time inclusion

            qDebug()<<"Compile time inclusion...";

            //bit more complicated than static method. we'll be multithreading.

            QMutex* qMutex = new QMutex();

            qMutex->lock();

            this->linesStore = lines;

            qMutex->unlock();

            QVector<QString> urls = LineStatic::getIncludesURLS(lines);

            qMutex->lock();

            this->includeURLs = urls;

            qMutex->unlock();

            delete qMutex;

            int i=0;
            while(i<this->includeURLs.size()){
                Compiler* compiler = new Compiler();

                compiler->isRoot=false;

                FileReader* fileReader = new FileReader("C:/proto/src/");

                QObject::connect(fileReader,SIGNAL(readFile(QString,QVector<QString>)),compiler,SLOT(doWork(QString,QVector<QString>)));

                QObject::connect(compiler,SIGNAL(exportToInclusion(QString,QVector<Line*>)),this,SLOT(recieveIncludes(QString,QVector<Line*>)));

//                QObject::connect(compiler,SIGNAL(finalCompilation(QString,QVector<Line*>)),this,SLOT(recieveIncludes(QString,QVector<Line*>)));

                fileReader->loadFile(this->includeURLs[i]);
                i++;
            }
            if(i==0){
                qDebug()<<"No inclusions found...";
                this->includeURLs={};
                this->linesStore=lines;
                emit this->exportToInclusion(fileName,lines);
             //   finalCompilation(fileName,lines);
            }

           // lines = Line::pullIncludes(lines);

            /*
            lines = HeapStore::constructLexTree(lines);
            if(ErrorRegistry::errorCount==0){
                lines = Line::compile(lines);
                i=0;
                while(i<lines.size()){
                    finalString+=lines[i]->toString(0);
                    i++;
                }
                lines = HeapStore::extractConstants(lines);
                i=0;
                while(i<lines.size()){
                    finalString+=lines[i]->toString(0);
                    i++;
                }
            }
            else{
                finalString+=ErrorRegistry::printLog();
            }*/
        }
        else{
            finalString+=ErrorRegistry::printLog();
        }
        int ms = myTimer.elapsed();

        float secs = float(ms)/1000.00;

        qDebug() << "Compilation finished, took "<<ms<<" milliseconds.("<<secs<<") seconds.";
    //    emit debugScript(fileName,finalString);
    //    emit workCompleted(fileName,bytes);
    }
}
