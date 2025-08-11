#ifndef DATA_PLOTTER_H
#define DATA_PLOTTER_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

QT_CHARTS_USE_NAMESPACE

class DataPlotter :public QWidget
{
    Q_OBJECT

public:
    explicit DataPlotter(QGridLayout *chartLayout, QWidget *parent = nullptr);
    ~DataPlotter();
    void initializeChart();

private:
    QGridLayout *chartLayout;
    QChart *chart;
    QLineSeries *series;
    QChartView *chartView;
};

#endif