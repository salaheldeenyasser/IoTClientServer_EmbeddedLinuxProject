#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
 * mainwindow.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Qt6 Server GUI – Main Window
 *
 * Tabs:
 *   1. Real Time Monitor  – QML circular gauge (Gauge.qml via QQuickWidget)
 *   2. Historical Analysis – Qt Charts line graph (scrolling, last 60 samples)
 *   3. Configuration       – QSlider sets the temperature threshold
 *   4. Quick Access        – Social-media shortcut buttons
 *
 * Networking:
 *   • TCP server on port 8080 – one client at a time
 *   • UDP socket  on port 8081 – bound and ready for UDP extension
 *   • 1-second QTimer drives the communication protocol loop
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QQuickWidget>
#include <QQmlContext>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // ── Q_PROPERTYs exposed to QML (Gauge.qml binds to 'backend') ─────────
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
    // Quick-Access tab
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    // Configuration tab
    void onSliderMoved(int value);

    // Server protocol timer
    void onServerTick();

    // TCP server
    void onNewTcpConnection();
    void onTcpDataReceived();
    void onTcpClientDisconnected();

private:
    // ── UI ─────────────────────────────────────────────────────────────────
    Ui::MainWindow *ui;

    // Tab 1 – Real Time Monitor
    QQuickWidget *m_gaugeWidget     = nullptr;
    QLabel       *m_monitorStatus   = nullptr;
    QLabel       *m_threshInfoLabel = nullptr;

    // Tab 2 – Historical Analysis
    QChartView   *m_chartView       = nullptr;
    QLineSeries  *m_tempSeries      = nullptr;
    QLineSeries  *m_threshSeries    = nullptr;
    QValueAxis   *m_axisX           = nullptr;
    QValueAxis   *m_axisY           = nullptr;
    int           m_sampleIndex     = 0;

    // ── State ──────────────────────────────────────────────────────────────
    double m_temperature     = 0.0;
    double m_threshold       = 50.0;
    double m_prevThreshold   = 50.0;
    bool   m_thresholdDirty  = false;   ///< true when slider moved → send "set threshold"

    // ── Networking ─────────────────────────────────────────────────────────
    QTcpServer *m_tcpServer   = nullptr;
    QTcpSocket *m_tcpClient   = nullptr;
    QUdpSocket *m_udpSocket   = nullptr;
    QTimer     *m_serverTimer = nullptr;

    // ── Private helpers ─────────────────────────────────────────────────────
    void setupGaugeTab();
    void setupChartTab();
    void setupNetworking();
    void sendToClient(const QString &msg);
    void addTemperatureSample(double temp);
    void updateInfoLabel();
};

#endif // MAINWINDOW_H
