#ifndef NATIVESTORE_H
#define NATIVESTORE_H

#include <QString>
#include <QTextBrowser>

#include "terminal.h"

#include "pipeconnection.h"
#include "interpreter.h"

class Interpreter;

class NativeStore{
    public: NativeStore(Interpreter* interpreter);
    public: static lookUpProperty(QString object,QString key);
    //public: static Terminal* SysTerminal;
    public: static bool SysLaunched;
    public: void resolve(int address,QVector<QString> argument);
    public: void resolveString(int address,QString argument);
    public: void resolve(int address,int argument);
    public: static void SysLaunch();
    public: static void SysOut(QString str);
    public: QVector<PipeConnection*> pipes;
    public: void wirePipe(int pipe,int NativeAddress);
    public: static int constexpr SysOutAddress=0;
    public: Interpreter* interpreter;
    public: static int constexpr HeapVisualizerAddress=14;
    public: static int constexpr DebugAddress=30;
    public: static int constexpr PWidgetManager=60;
};

#endif // NATIVESTORE_H
