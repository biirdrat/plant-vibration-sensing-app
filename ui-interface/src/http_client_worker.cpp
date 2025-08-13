#include "http_client_worker.h"

HttpClientWorker::HttpClientWorker(QObject *parent)
{

}

HttpClientWorker::~HttpClientWorker()
{

}

void HttpClientWorker::doWork()
{
    while (!QThread::currentThread()->isInterruptionRequested()) 
    {
        // ... do work ...

        QThread::msleep(10);
    }
    std::cout << "HERE";
    emit finished();
}