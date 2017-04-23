#ifndef PIPE_H
#define PIPE_H

#include <QString>
#include <QVector>

class Pipe{
public: Pipe(QString processName, QString pipeName);
public: QString name;
public: void out(QString str);
public: bool flagged;
//public: QString dataOut;
public: QVector<QString> data;

};

#endif // PIPE_H
