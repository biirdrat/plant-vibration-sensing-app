#include "http_client_worker.h"

namespace 
{
    constexpr const char* SERVER_ADDRESS = "http://httpbin.org";
}

HttpClientWorker::HttpClientWorker(QObject *parent)
    : logger(&Logger::getInstance())
    , httpClient(std::make_unique<httplib::Client>(SERVER_ADDRESS))
    , sensorDataVecs(NUM_SENSORS)
{
    configureClient();

    for(int vecIdx = 0; vecIdx < NUM_SENSORS; vecIdx++)
    {
        sensorDataVecs[vecIdx].reserve(MAX_DATA_VALUES);
    }
}

HttpClientWorker::~HttpClientWorker()
{
}

void HttpClientWorker::configureClient()
{
    httpClient->set_connection_timeout(3, 0);
    httpClient->set_read_timeout(3, 0);
    httpClient->set_write_timeout(3, 0);
}

void HttpClientWorker::startClient()
{   
    auto response = httpClient->Get("/hello");

    if(response)
    {
        logger->log(LogLevel::Info, "Successfully got response from server.");
        logger->log(LogLevel::Info, "Response body: " + QString::fromStdString(response->body));
        
        if(response->body != "Client successfully linked with server.")
        {
            logger->log(LogLevel::Warning, "Server is already linked with client, failed to connect.");
            emit finished();
        }
        else
        {
            runClient();
        }
    }
    else
    {
        logger->log(LogLevel::Warning, "Failed to get a response from the server.");
        emit finished();
    }
}

void HttpClientWorker::runClient()
{
    while (!QThread::currentThread()->isInterruptionRequested()) 
    {
         logger->log(LogLevel::Info, "RUNNING!");

        QThread::msleep(1000);
    }
    emit finished();
}

void HttpClientWorker::parseData(const std::string& payload)
{
    try
    {
        nlohmann::json jsonDoc = nlohmann::json::parse(payload);

        // Validate top-level type
        if (!jsonDoc.is_object()) 
        {
            logger->log(LogLevel::Warning, "Error: Top-level JSON must be an object.");
            return;
        }

        if (jsonDoc.empty())
        {
            logger->log(LogLevel::Warning, "Error: JSON object is empty.");
            return;
        }

        for(int sensorIdx = 0; sensorIdx < NUM_SENSORS; sensorIdx++)
        {
            
        }
    }
    catch(const std::exception& e)
    {
        logger->log(LogLevel::Warning, "JSON Payload received is in incorrect format.");
    }
    
}