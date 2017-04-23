#ifndef PIPEMANAGER_H
#define PIPEMANAGER_H

#include <QObject>
#include <pipe.h>


class PipeManager:public QObject{
    Q_OBJECT
public:
    PipeManager();
    public: static QVector<Pipe*> Pipes;
public slots:
    void receiveGUIRequest();
    void receiveGUIEvent();
    void doWork();
    void receivePipeMessage();
signals:
    void guiRequest();
    void toPipeManager();
    void toPipeSignal();
};

#endif // PIPEMANAGER_H
