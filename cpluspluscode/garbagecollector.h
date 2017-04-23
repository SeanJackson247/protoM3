#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

#include "interpreter.h"

class GarbageCollector:public QObject{
    Q_OBJECT
public: Interpreter* p;
public slots: void doWork();
public:
    GarbageCollector(Interpreter* i);
};

#endif // GARBAGECOLLECTOR_H
