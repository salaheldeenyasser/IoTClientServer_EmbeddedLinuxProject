#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QQmlEngine>
#include <QPainter>
#include <QtCharts/QChart>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ── Configuration tab: slider ──────────────────────────────────────────
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(100);
    ui->horizontalSlider->setValue(static_cast<int>(m_threshold));
    ui->lcdNumber->display(static_cast<int>(m_threshold));

    connect(ui->horizontalSlider, &QSlider::sliderMoved,
            this, &MainWindow::onSliderMoved);

    // ── Build the two dynamic tabs ─────────────────────────────────────────
    setupGaugeTab();
    setupChartTab();

    // ── Start TCP/UDP server ───────────────────────────────────────────────
    setupNetworking();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab 1 – Real Time Monitor  (QML gauge embedded in QQuickWidget)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupGaugeTab()
{
    QWidget *tab = ui->tab_2;               // "Real Time Monitor" tab
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignCenter);

    // Connection status label
    m_monitorStatus = new QLabel("Waiting for client connection…", tab);
    m_monitorStatus->setAlignment(Qt::AlignCenter);
    m_monitorStatus->setStyleSheet(
        "color:#aaaaaa; font-size:13px; padding:4px;");
    layout->addWidget(m_monitorStatus);

    // QQuickWidget hosting Gauge.qml
    m_gaugeWidget = new QQuickWidget(tab);
    m_gaugeWidget->setMinimumSize(650, 650);
    m_gaugeWidget->setMaximumSize(900, 900);
    m_gaugeWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_gaugeWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Make the QQuickWidget background fully transparent so the tab
    // background image shows through around the circular gauge.
    m_gaugeWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_gaugeWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_gaugeWidget->setClearColor(Qt::transparent);

    // Expose 'backend' → this; Gauge.qml accesses backend.currentTemperature
    // and backend.threshold via Q_PROPERTY bindings.
    m_gaugeWidget->rootContext()->setContextProperty("backend", this);

    // Gauge.qml is embedded via Photos.qrc under prefix "/new/prefix2"
    m_gaugeWidget->setSource(QUrl("qrc:/new/prefix2/Gauge.qml"));

    layout->addWidget(m_gaugeWidget, 0, Qt::AlignCenter);

    // Threshold / LED status label
    m_threshInfoLabel = new QLabel(
        QString("Threshold: %1 °C").arg(m_threshold), tab);
    m_threshInfoLabel->setAlignment(Qt::AlignCenter);
    m_threshInfoLabel->setStyleSheet(
        "color:#cccccc; font-size:13px; font-weight:bold; padding:4px;");
    layout->addWidget(m_threshInfoLabel);

    tab->setLayout(layout);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab 2 – Historical Analysis  (Qt Charts scrolling line graph)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupChartTab()
{
    QWidget *tab = ui->tab;                 // "Historical Analysis" tab
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(40, 30, 30, 60);
    layout->setSpacing(40);

    auto *title = new QLabel("Temperature History", tab);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "color:white; font-size:30px; font-weight:bold; padding:15px;");
    layout->addWidget(title);

    // ── Series ──────────────────────────────────────────────────────────────
    m_tempSeries = new QLineSeries();
    m_tempSeries->setName("Temperature (°C)");
    m_tempSeries->setColor(QColor("#2ecc71"));
    QPen tp = m_tempSeries->pen();
    tp.setWidth(2);
    m_tempSeries->setPen(tp);

    m_threshSeries = new QLineSeries();
    m_threshSeries->setName("Threshold");
    m_threshSeries->setColor(QColor("#e74c3c"));
    QPen thp;
    thp.setWidth(2);
    thp.setStyle(Qt::DashLine);
    m_threshSeries->setPen(thp);
    // Initialise threshold line across the visible window
    m_threshSeries->append(0,  m_threshold);
    m_threshSeries->append(60, m_threshold);

    // ── Chart ───────────────────────────────────────────────────────────────
    auto *chart = new QChart();
    chart->addSeries(m_tempSeries);
    chart->addSeries(m_threshSeries);
    chart->setBackgroundBrush(QBrush(QColor("#1a1a2e")));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor("#16213e")));
    chart->setPlotAreaBackgroundVisible(true);
    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setBackgroundVisible(false);

    // ── Axes ────────────────────────────────────────────────────────────────
    m_axisX = new QValueAxis();
    m_axisX->setTitleText("Sample");
    m_axisX->setRange(0, 60);
    m_axisX->setLabelFormat("%d");
    m_axisX->setLabelsColor(Qt::white);
    m_axisX->setTitleBrush(QBrush(Qt::white));
    m_axisX->setGridLineColor(QColor("#2d2d44"));
    m_axisX->setTickCount(7);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Temperature (°C)");
    m_axisY->setRange(0, 100);
    m_axisY->setLabelFormat("%.1f");
    m_axisY->setLabelsColor(Qt::white);
    m_axisY->setTitleBrush(QBrush(Qt::white));
    m_axisY->setGridLineColor(QColor("#2d2d44"));

    chart->addAxis(m_axisX, Qt::AlignBottom);
    chart->addAxis(m_axisY, Qt::AlignLeft);
    m_tempSeries->attachAxis(m_axisX);
    m_tempSeries->attachAxis(m_axisY);
    m_threshSeries->attachAxis(m_axisX);
    m_threshSeries->attachAxis(m_axisY);

    // ── ChartView ───────────────────────────────────────────────────────────
    m_chartView = new QChartView(chart, tab);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    layout->addWidget(m_chartView);
    tab->setLayout(layout);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Networking
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupNetworking()
{
    // TCP server – port 8080
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &MainWindow::onNewTcpConnection);

    if (!m_tcpServer->listen(QHostAddress::Any, 8080)) {
        qWarning() << "[Server] TCP listen failed:"
                   << m_tcpServer->errorString();
    } else {
        qDebug() << "[Server] TCP listening on port 8080";
    }

    // UDP socket – port 8081 (bound for future UDP use)
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::Any, 8081);

    // Protocol timer – fires every 1 second
    m_serverTimer = new QTimer(this);
    m_serverTimer->setInterval(1000);
    connect(m_serverTimer, &QTimer::timeout,
            this, &MainWindow::onServerTick);
    m_serverTimer->start();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Server protocol tick  (every 1 s)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onServerTick()
{
    if (!m_tcpClient ||
        m_tcpClient->state() != QAbstractSocket::ConnectedState)
        return;

    if (m_thresholdDirty) {
        // Threshold changed via slider → tell the client
        sendToClient("set threshold");
        sendToClient(QString::number(m_threshold, 'f', 1));
        m_thresholdDirty = false;
    } else {
        // Normal cycle → request current temperature
        sendToClient("get temp");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  TCP: new client connected
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onNewTcpConnection()
{
    QTcpSocket *incoming = m_tcpServer->nextPendingConnection();

    // Reject second client if one is already connected
    if (m_tcpClient &&
        m_tcpClient->state() == QAbstractSocket::ConnectedState) {
        incoming->disconnectFromHost();
        incoming->deleteLater();
        return;
    }

    m_tcpClient = incoming;
    connect(m_tcpClient, &QTcpSocket::readyRead,
            this, &MainWindow::onTcpDataReceived);
    connect(m_tcpClient, &QTcpSocket::disconnected,
            this, &MainWindow::onTcpClientDisconnected);

    const QString ip = m_tcpClient->peerAddress().toString();
    qDebug() << "[Server] Client connected:" << ip;
    m_monitorStatus->setText("Client connected: " + ip);

    // Send initial threshold immediately
    sendToClient("set threshold");
    sendToClient(QString::number(m_threshold, 'f', 1));
}

// ─────────────────────────────────────────────────────────────────────────────
//  TCP: data received from client (temperature reading as plain number string)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onTcpDataReceived()
{
    const QByteArray raw = m_tcpClient->readAll().trimmed();
    const QString    msg = QString::fromUtf8(raw);
    qDebug() << "[Server] Received:" << msg;

    bool ok = false;
    double temp = msg.toDouble(&ok);
    if (!ok) return;

    m_temperature = temp;
    emit temperatureChanged(m_temperature);

    addTemperatureSample(m_temperature);
    updateInfoLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
//  TCP: client disconnected
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onTcpClientDisconnected()
{
    qDebug() << "[Server] Client disconnected.";
    m_tcpClient->deleteLater();
    m_tcpClient = nullptr;
    m_monitorStatus->setText("Client disconnected — waiting for reconnect…");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::sendToClient(const QString &msg)
{
    if (!m_tcpClient) return;
    m_tcpClient->write((msg + "\n").toUtf8());
}

void MainWindow::addTemperatureSample(double temp)
{
    // ── Add point to temp series ─────────────────────────────────────────
    m_tempSeries->append(m_sampleIndex, temp);
    ++m_sampleIndex;

    // ── Scroll X window (keep last 60 samples) ───────────────────────────
    int xMin = qMax(0, m_sampleIndex - 60);
    int xMax = qMax(60, m_sampleIndex);
    m_axisX->setRange(xMin, xMax);

    // ── Update threshold line to match current X window ──────────────────
    m_threshSeries->clear();
    m_threshSeries->append(xMin, m_threshold);
    m_threshSeries->append(xMax, m_threshold);

    // ── Auto-scale Y to keep both temp + threshold in view ───────────────
    double yMin = qMin(temp, m_threshold) - 10.0;
    double yMax = qMax(temp, m_threshold) + 10.0;
    m_axisY->setRange(qMax(0.0, yMin), qMin(150.0, yMax));
}

void MainWindow::updateInfoLabel()
{
    if (!m_threshInfoLabel) return;
    const bool ledOn = (m_temperature >= m_threshold);
    m_threshInfoLabel->setText(
        QString("Temp: %1 °C  |  Threshold: %2 °C  |  LED: %3")
            .arg(m_temperature, 0, 'f', 1)
            .arg(m_threshold,   0, 'f', 1)
            .arg(ledOn ? "ON  🔴" : "OFF  🟢"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Configuration tab: slider moved
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSliderMoved(int value)
{
    m_threshold = static_cast<double>(value);
    ui->lcdNumber->display(value);
    emit thresholdChanged(m_threshold);

    if (m_threshold != m_prevThreshold) {
        m_thresholdDirty = true;
        m_prevThreshold  = m_threshold;
    }

    updateInfoLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Quick Access buttons
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_pushButton_clicked()
{
    QDesktopServices::openUrl(
        QUrl("https://www.facebook.com/edgesfortraining"));
}

void MainWindow::on_pushButton_2_clicked()
{
    QDesktopServices::openUrl(
        QUrl("https://www.linkedin.com/company/edges-for-training/"));
}

void MainWindow::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(
        QUrl("https://www.instagram.com/edgesfortraining/"));
}

void MainWindow::on_connectButton_clicked()
{

}

