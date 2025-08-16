#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>

enum class LogLevel 
{
    Debug,
    Info,
    Warning,
    Error
};
Q_DECLARE_METATYPE(LogLevel)

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger& getInstance();  // Singleton accessor

    void log(LogLevel log_level, const QString& message);  // Log method

signals:
    void logMessageSignal(LogLevel log_level, const QString& message);

private:
    explicit Logger(QObject *parent = nullptr);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H