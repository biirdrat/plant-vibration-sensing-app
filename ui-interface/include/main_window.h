#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "Logger.h"
#include"data_plotter.h"
#include "http_client_worker.h"
#include <QMainWindow>
#include <QThread>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    Logger* logger;
    DataPlotter* dataPlotter;
    QThread *httpClientThread;
    // QChart *chart;
    // QLineSeries *series;
    // QChartView *chartView;
    
private slots:
    void onLogMessage(LogLevel log_level, const QString& formatted_message);
};
#endif // MAINWINDOW_H