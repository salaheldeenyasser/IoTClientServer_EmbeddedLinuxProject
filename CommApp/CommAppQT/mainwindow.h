#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
 * mainwindow.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Qt6 Server GUI – Main Window
 *
 * Tabs:
 *   1. Real Time Monitor   – QML circular gauge (Gauge.qml via QQuickWidget)
 *   2. Historical Analysis – Qt Charts scrolling line graph (last 60 samples)
 *   3. Configuration       – TCP/UDP checkboxes + Connect button + threshold slider
 *   4. Quick Access        – Social-media shortcut buttons
 *
 * Networking:
 *   Uses ONLY Socket.h / Channel.h (project OOP classes).
 *   QTcpServer, QTcpSocket, QUdpSocket are NOT included anywhere.
 *   QSocketNotifier (part of Qt::Core) bridges POSIX file descriptors into
 *   the Qt event loop so the GUI never blocks.
 *
 * Configuration tab widget names (from mainwindow.ui):
 *   checkBox       – TCP  (checked by default; mutual-exclusion wired in .ui)
 *   checkBox_2     – UDP
 *   connectButton  – Connect / Disconnect push button
 *   horizontalSlider / lcdNumber – threshold control
 * ─────────────────────────────────────────────────────────────────────────────
 */

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

// ── Project OOP networking classes (replaces all Qt networking headers) ───────
#include "Socket.h"
#include "Channel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Active connection protocol
enum class ConnectionType { TCP, UDP };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // ── Q_PROPERTYs exposed to QML (Gauge.qml reads these via 'backend') ─────
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
    void onListenFdActivated(int fd);   // TCP: new client pending on listen fd
    void onClientFdReadable(int fd);    // TCP: data arrived from connected client
    void onUdpFdReadable(int fd);       // UDP: datagram arrived

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

    // ── OOP networking (Socket.h / Channel.h) — no Qt networking classes ──────
    TCPSocket      m_tcpSock;           // concrete TCPSocket  (value member)
    UDPSocket      m_udpSock;           // concrete UDPSocket  (value member)
    ServerChannel  m_serverChannel;    // holds Socket* polymorphically

    int            m_clientFd = -1;    // fd of the accepted TCP client

    // QSocketNotifiers watch POSIX fds inside the Qt event loop
    QSocketNotifier *m_listenNotifier = nullptr;  // TCP listen socket
    QSocketNotifier *m_clientNotifier = nullptr;  // TCP connected client
    QSocketNotifier *m_udpNotifier    = nullptr;  // UDP bound socket

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
