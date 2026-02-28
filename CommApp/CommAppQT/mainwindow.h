#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QSocketNotifier>
#include <QQuickWidget>
#include <QQmlContext>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QLabel>

#include "Socket.h"
#include "Channel.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

enum class ConnectionType
{
    TCP,
    UDP
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    Q_PROPERTY(double currentTemperature
                   READ currentTemperature
                       NOTIFY temperatureChanged)

    Q_PROPERTY(double threshold
                   READ threshold
                       NOTIFY thresholdChanged)

    double currentTemperature() const { return m_temperature; }
    double threshold() const { return m_threshold; }

signals:
    void temperatureChanged(double temp);
    void thresholdChanged(double threshold);

private slots:

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    void onSliderMoved(int value);

    void on_connectButton_clicked();

    void onServerTick();

    void onListenFdActivated(int fd);
    void onClientFdReadable(int fd);
    void onUdpFdReadable(int fd);

private:
    Ui::MainWindow *ui;

    QQuickWidget *m_gaugeWidget = nullptr;
    QLabel *m_monitorStatus = nullptr;
    QLabel *m_threshInfoLabel = nullptr;

    QChartView *m_chartView = nullptr;
    QLineSeries *m_tempSeries = nullptr;
    QLineSeries *m_threshSeries = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;
    int m_sampleIndex = 0;

    double m_temperature = 0.0;
    double m_threshold = 50.0;
    double m_prevThreshold = 50.0;
    bool m_thresholdDirty = false;
    ConnectionType m_connType = ConnectionType::TCP;

    TCPSocket m_tcpSock;
    UDPSocket m_udpSock;
    ServerChannel m_serverChannel;

    int m_clientFd = -1;

    QSocketNotifier *m_listenNotifier = nullptr;
    QSocketNotifier *m_clientNotifier = nullptr;
    QSocketNotifier *m_udpNotifier = nullptr;

    QTimer *m_serverTimer = nullptr;

    void setupGaugeTab();
    void setupChartTab();
    void startServer();
    void stopServer();
    void sendToClient(const std::string &msg);
    void handleIncomingData(const std::string &raw);
    void addTemperatureSample(double temp);
    void updateInfoLabel();
    void updateConnectButton();
};

#endif
