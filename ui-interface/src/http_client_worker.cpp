#include "http_client_worker.h"

HttpClientWorker::HttpClientWorker(QObject *parent)
    : isStarted(false)
    , logger(&Logger::getInstance())
{

}

HttpClientWorker::~HttpClientWorker()
{

}

void HttpClientWorker::startClient()
{
    runClient();
}

void HttpClientWorker::runClient()
{
    while (!QThread::currentThread()->isInterruptionRequested()) 
    {
         logger->log(LogLevel::Info, "RUNNING!");

        QThread::msleep(1000);
    }
    std::cout << "HERE";
    emit finished();
}