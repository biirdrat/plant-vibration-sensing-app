#include "main_window.h"
#include "ui_main_window.h"

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(&Logger::getInstance())
    , mainLoopTimer(new QTimer())
{
    qRegisterMetaType<LogLevel>("LogLevel");

    ui->setupUi(this);

    connect(logger, &Logger::logMessageSignal, this, &MainWindow::onLogMessage);

    dataPlotter = new DataPlotter(ui->graph_layout, this);

    initializeHttpClientThread();

    startHttpClientThread();

    connect(mainLoopTimer, &QTimer::timeout, this, &MainWindow::onMainLoopTick);
    
    mainLoopTimer->start(1000);

    logger->log(LogLevel::Info, "Main Application Loaded Successfully!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLogMessage(LogLevel log_level, const QString& formatted_message)
{
    int rowCount = ui->log_table->rowCount();
    ui->log_table->insertRow(rowCount);

    QTableWidgetItem* item = new QTableWidgetItem(formatted_message);

    // Set font size
    QFont font;
    font.setPointSize(14);
    item->setFont(font);

    // Set RGB text color based on log level
    QColor textColor;
    switch (log_level) {
        case LogLevel::Debug:
            textColor = QColor(15, 157, 189);
            break;
        case LogLevel::Info:
            textColor = QColor(70, 153, 47);
            break;
        case LogLevel::Warning:
            textColor = QColor(255, 163, 5);
            break;
        case LogLevel::Error:
            textColor = QColor(199, 0, 0);
            break;
        default:
            textColor = QColor(0, 0, 0);
            break;
    }

    item->setForeground(QBrush(textColor));
    ui->log_table->setItem(rowCount, 0, item);

    // Auto-scroll to bottom
    ui->log_table->scrollToBottom();
}

void MainWindow::initializeHttpClientThread()
{
    httpClientThread = new QThread(this);
    
    httpClientWorker = new HttpClientWorker();

    httpClientWorker->moveToThread(httpClientThread);

    connect(httpClientThread, &QThread::started, httpClientWorker, &HttpClientWorker::startClient);

    connect(httpClientWorker, &HttpClientWorker::finished, httpClientThread, &QThread::quit);
}

void MainWindow::startHttpClientThread()
{
    if (!httpClientThread->isRunning())
    {
        httpClientThread->start();
    }
}

void MainWindow::stopHttpClientThread()
{
    httpClientThread->requestInterruption();
}

void MainWindow::onMainLoopTick()
{
    std::cout << httpClientThread->isRunning() << std::endl;
}