#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QObject>

class EventHandler:public QObject{
    Q_OBJECT
public: EventHandler();
public slots:
  //  void sendGUIEvent();
signals:
//    void guiRequest();
    void sendGUIEvent();
};

#endif // EVENTHANDLER_H
