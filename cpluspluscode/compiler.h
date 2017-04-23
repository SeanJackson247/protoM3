#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <line.h>

class Compiler:public QObject{
    Q_OBJECT
public:Compiler();
public:QVector<Line*> linesStore;
public:QVector<QString> includeURLs;
public:QString exportDirectory;
public:bool isRoot;
public slots:
    void doWork(QString fileName, QVector<QString> data);
    void recieveIncludes(QString fileName, QVector<Line*> data);
    void finalCompilation(QString fileName,QVector<Line*> lines);
signals:
    void workCompleted(QString,PByteArray);
    void debugScript(QString,QString);
    void exportToInclusion(QString name,QVector<Line*> lines);
};

#endif // COMPILER_H
