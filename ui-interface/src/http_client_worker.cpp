#include "http_client_worker.h"

namespace 
{
    constexpr const char* SERVER_ADDRESS = "http://192.168.4.1";
}

HttpClientWorker::HttpClientWorker(QObject *parent)
    : logger(&Logger::getInstance())
    , httpClient(std::make_unique<httplib::Client>(SERVER_ADDRESS))
    , sensorDataVecs(NUM_SENSORS)
{
    configureClient();

    // Initialize sensor data vectors
    for(int vecIdx = 0; vecIdx < NUM_SENSORS; vecIdx++)
    {
        sensorDataVecs[vecIdx].resize(MAX_DATA_VALUES);
    }

    // parseData("{\"sensordata\":[0,45,78,23,56,89,34,67,90,11]}");
    // logger->log(LogLevel::Warning,
    //         QString("%1").arg(sensorDataVecs.at(0).at(0)));

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
    httplib::Result response = httpClient->Get("/link");

    if(response && response->status == 200)
    {
        logger->log(LogLevel::Info, "Successfully got response from server.");
        
        if(response->body != "Link Successful")
        {
            logger->log(LogLevel::Warning, "Server is already linked with client, failed to connect.");
            emit finished();
        }
        else
        {
            logger->log(LogLevel::Info, "Client Linked with Server Successfully.");
            runClient();
        }
    }
    else if(response && !(response->status == 200))
    {
        logger->log(LogLevel::Warning, "Server could not find handle for link request.");
        emit finished();
    }
    else
    {
        logger->log(LogLevel::Warning, "Server was not found.");
        emit finished();
    }
}

void HttpClientWorker::runClient()
{
    while (!QThread::currentThread()->isInterruptionRequested()) 
    {
        httplib::Result response = httpClient->Get("/data");
        if(response && response->status == 200)
        {
            parseData(response->body);
        }
        else
        {
            logger->log(LogLevel::Warning, "Failed to get valid response from server.");
            break;
        }

        QThread::msleep(10);
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
        
        std::string datakey = "sensordata";

        if (!jsonDoc.contains(datakey)) 
        {
            logger->log(LogLevel::Warning, QString::fromStdString("Warning: Missing key: " + datakey));
            return;
        }

        if (!jsonDoc[datakey].is_array())
        {
            logger->log(LogLevel::Warning, QString::fromStdString("Data is not an array."));
            return;
        }

        if(!(jsonDoc[datakey][0].is_number_integer()))
        {   
            logger->log(LogLevel::Warning, QString::fromStdString("First value in data array is not a number."));
            return;
        }

        int sensorIdx = jsonDoc[datakey][0].get<int>();


        for(int valIdx = 1; valIdx < MAX_DATA_VALUES+1; valIdx++)
        {
            if(jsonDoc[datakey][valIdx].is_number_integer())
            {
                sensorDataVecs[sensorIdx].at(valIdx-1) = jsonDoc[datakey][valIdx].get<int>();
            }
            else
            {
                logger->log(LogLevel::Warning,
                    QString("Warning: sensor%1 value%2 is not an integer.")
                        .arg(sensorIdx)
                        .arg(valIdx - 1)
                );
                break;
            }
        }

    }
    catch(const std::exception& e)
    {
        logger->log(LogLevel::Warning, "Warning: JSON Payload received is in incorrect format.");
    }
    
}