#ifndef HTTP_CLIENT_WORKER_H
#define HTTP_CLIENT_WORKER_H

#include "httplib.h"
#include "logger.h"
#include <QObject>
#include <QThread>
#include <iostream>

class HttpClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit HttpClientWorker(QObject *parent = nullptr);
    ~HttpClientWorker();
    
private:
    void configureClient();
    void runClient();
    Logger* logger;
    std::unique_ptr<httplib::Client> httpClient;

public slots:
    void startClient();
    
signals:
    void finished();
};

#endif