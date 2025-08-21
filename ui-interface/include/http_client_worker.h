#ifndef HTTP_CLIENT_WORKER_H
#define HTTP_CLIENT_WORKER_H

#include "httplib.h"
#include "logger.h"
#include "json.hpp"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <iostream>

class HttpClientWorker : public QObject
{
    Q_OBJECT

public:
    explicit HttpClientWorker(QObject *parent = nullptr);
    ~HttpClientWorker();
    
private:
    static constexpr int NUM_SENSORS = 4;
    static constexpr int MAX_DATA_VALUES = 100;
    Logger* logger;
    std::unique_ptr<httplib::Client> httpClient;
    std::vector<std::vector<int>> sensorDataVecs;
    void configureClient();
    void runClient();
    void parseData(const std::string& payload);
    QMutex dataMutex;

public slots:
    void startClient();
    
signals:
    void finished();
};

#endif