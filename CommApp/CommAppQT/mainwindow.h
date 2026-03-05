#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QSocketNotifier>          // Qt::Core — no Qt networking module needed
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
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class ConnectionType { TCP, UDP };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    Q_PROPERTY(double currentTemperature
               READ   currentTemperature
               NOTIFY temperatureChanged)

    Q_PROPERTY(double threshold
               READ   threshold
               NOTIFY thresholdChanged)

    double currentTemperature() const { return m_temperature; }
    double threshold()          const { return m_threshold;   }

signals:
    void temperatureChanged(double temp);
    void thresholdChanged(double threshold);

private slots:
    // ── Quick Access tab ──────────────────────────────────────────────────────
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    // ── Configuration tab ─────────────────────────────────────────────────────
    void onSliderMoved(int value);

    // Auto-connected by Qt name convention (on_<objectName>_clicked)
    void on_connectButton_clicked();

    // ── Protocol timer (1 s) ─────────────────────────────────────────────────
    void onServerTick();

    // ── QSocketNotifier callbacks (replaces Qt socket signals) ───────────────
    void onListenFdActivated(int fd);
    void onClientFdReadable(int fd);
    void onUdpFdReadable(int fd);

private:
    // ── UI ────────────────────────────────────────────────────────────────────
    Ui::MainWindow *ui;

    // Tab 1
    QQuickWidget *m_gaugeWidget     = nullptr;
    QLabel       *m_monitorStatus   = nullptr;
    QLabel       *m_threshInfoLabel = nullptr;

    // Tab 2
    QChartView   *m_chartView       = nullptr;
    QLineSeries  *m_tempSeries      = nullptr;
    QLineSeries  *m_threshSeries    = nullptr;
    QValueAxis   *m_axisX           = nullptr;
    QValueAxis   *m_axisY           = nullptr;
    int           m_sampleIndex     = 0;

    // ── Application state ─────────────────────────────────────────────────────
    double         m_temperature    = 0.0;
    double         m_threshold      = 50.0;
    double         m_prevThreshold  = 50.0;
    bool           m_thresholdDirty = false;
    ConnectionType m_connType       = ConnectionType::TCP;

    TCPSocket      m_tcpSock;
    UDPSocket      m_udpSock;
    ServerChannel  m_serverChannel;

    int            m_clientFd = -1;
    bool           m_udpClientReady = false;

    std::string    m_recvBuffer;


    QSocketNotifier *m_listenNotifier = nullptr;
    QSocketNotifier *m_clientNotifier = nullptr;
    QSocketNotifier *m_udpNotifier    = nullptr;

    QTimer *m_serverTimer = nullptr;

    // ── Helpers ───────────────────────────────────────────────────────────────
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

#endif // MAINWINDOW_H
