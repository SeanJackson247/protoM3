#ifndef COMPILER2_H
#define COMPILER2_H

#include <QObject>
#include <line.h>
#include <lexnode.h>

class Compiler2:public QObject{
    Q_OBJECT
public:Compiler2();
public:LexNode* linesStore;
public:QVector<QString> includeURLs;
public:QString exportDirectory;
public:bool isRoot;
public slots:
    void doWork(QString fileName, QVector<QString> data);
    void recieveIncludes(QString fileName, LexNode* data);
    void finalCompilation(QString fileName, LexNode* lexNode);
signals:
    void workCompleted(QString,PByteArray);
    void debugScript(QString,QString);
    void exportToInclusion(QString name,LexNode* lines);
};

#endif // COMPILER2_H
