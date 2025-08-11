#include "main_window.h"
#include "ui_main_window.h"
#include <iostream>

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(&Logger::getInstance())
{
    ui->setupUi(this);

    connect(logger, &Logger::logMessageSignal, this, &MainWindow::onLogMessage);

    initializeChart();

    logger->log(LogLevel::Info, "Main Application Loaded Successfully!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeChart()
{
    // Create the series and populate it
    series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);

    // Create the chart and configure it
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();

    // Create chart view
    chartView = new QChartView(chart, this);
    chartView->setMinimumSize(550, 400);
    chartView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Add chart view to layout
    ui->graph_layout->addWidget(chartView);
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
        case LogLevel::Warn:
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