#ifndef HTTP_CLIENT_WORKER_H
#define HTTP_CLIENT_WORKER_H

#include <QObject>
#include <QThread>
#include <iostream>

class HttpClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit HttpClientWorker(QObject *parent = nullptr);
    ~HttpClientWorker();

public slots:
    void doWork();

signals:
    void finished();

};

#endif