#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include "pbytearray.h"

class FileReader:public QObject{
    Q_OBJECT
public:
    FileReader(QString inputDirectory);
public:
    QString dir;
public:
    void loadFile(QString fileName);
    void loadBytesFromFile(QString fileName);
signals:
    readFile(QString filename,QVector<QString> qba);
    readBytesFromFile(QString fileName,PByteArray data);
};

#endif // FILEREADER_H
