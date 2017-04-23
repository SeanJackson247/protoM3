#ifndef ERRORREGISTRY_H
#define ERRORREGISTRY_H

#include <QString>
#include <QVector>
class ErrorRegistry{
public: ErrorRegistry();
//The error registry is essentially static:
public:static void logError(QString error);
public:static int errorCount;
public:static QString printLog();
public:static QVector<QString> log;
};

#endif // ERRORREGISTRY_H
