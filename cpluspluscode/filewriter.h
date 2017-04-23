#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QObject>
#include "pbytearray.h"

class FileWriter:public QObject{
    Q_OBJECT
public: QString dir;
public: FileWriter(QString outputDirectory);
public: static int instancesCount;
public: int id;
public slots:
    void writeRequest(QString filename, PByteArray contents);
    void nativeWriteRequest(QString filename, PByteArray contents, int runtimeInstanceId);
    void debugRequest(QString filename, QString contents);
};

#endif // FILEWRITER_H
