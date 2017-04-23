#include "filereader.h"

#include <QFile>
#include <QDebug>
#include <QString>

FileReader::FileReader(QString inputDirectory){
    this->dir = inputDirectory;
}

void FileReader::loadFile(QString fileName){
    if(fileName.size()>=1 && (fileName.at(0).toLatin1()=='"' || fileName.at(0).toLatin1()=='\'')
            && fileName.at(0).toLatin1()==fileName.at(fileName.size()-1).toLatin1()){
        fileName = fileName.split(fileName.at(0).toLatin1())[1];
    }
    QFile file(this->dir+fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QVector<QString> err = {"ERROR", "ERROR opening file '"+this->dir+fileName+"' : " + file.error()};
        emit readFile(fileName,err);
    }
    else{
        QTextStream instream(&file);
        QVector<QString> data;
        int i=0;
        while(!instream.atEnd()){
            QString line = instream.readLine();
            i++;
            data.resize(i);
            data[i-1]=line;
        }
        file.close();
        emit readFile(fileName,data);
    }
}

void FileReader::loadBytesFromFile(QString fileName){
    QFile file(this->dir+fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "error opening file: " << file.error();
    }
    QByteArray data = file.readAll();

    PByteArray* res = new PByteArray();

    int i=0;
    while(i<res->size()){
        res->resize(res->size()+1);
        res->setByte(res->size()-1,(unsigned char)data[i]);
        i++;
    }

    file.close();
    emit readBytesFromFile(fileName,*res);
}
