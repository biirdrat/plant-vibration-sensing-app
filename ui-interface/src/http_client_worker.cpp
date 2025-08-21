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
        sensorDataVecs[vecIdx].resize(MAX_DATA_VALUES);
    }

    // parseData("{\"sensor0\":[12,45,78,23,56,89,34,67,90,11]}");
    // logger->log(LogLevel::Warning,
    //         QString("%1").arg(sensorDataVecs.at(0).at(1)));

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
            logger->log(LogLevel::Warning, "Warning: Top-level JSON must be an object.");
            return;
        }

        if (jsonDoc.empty())
        {
            logger->log(LogLevel::Warning, "Warning: JSON object is empty.");
            return;
        }

        for(int sensorIdx = 0; sensorIdx < NUM_SENSORS; sensorIdx++)
        {
            std::string key = "sensor" + std::to_string(sensorIdx);

            if (!jsonDoc.contains(key)) 
            {
                logger->log(LogLevel::Warning, QString::fromStdString("Warning: Missing key: " + key));
                break;
            }

            if (!jsonDoc[key].is_array())
            {
                logger->log(LogLevel::Warning, QString::fromStdString(key + " data is not an array."));
                break;
            }

            if(jsonDoc[key].size() < MAX_DATA_VALUES)
            {
                logger->log(LogLevel::Warning, QString::fromStdString(key + " is missing data values."));
                break;
            }
            
            for(int valIdx = 0; valIdx < MAX_DATA_VALUES; valIdx++)
            {
                if(jsonDoc[key][valIdx].is_number_integer())
                {
                    sensorDataVecs[sensorIdx].at(valIdx) = jsonDoc[key][valIdx].get<int>();
                }
                else
                {
                    logger->log(LogLevel::Warning,
                                QString("Warning: sensor%1 value%2 is not an integer.").arg(sensorIdx, valIdx));
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        logger->log(LogLevel::Warning, "Warning: JSON Payload received is in incorrect format.");
    }
    
}