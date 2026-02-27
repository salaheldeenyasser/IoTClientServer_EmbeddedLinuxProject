#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QQmlEngine>
#include <QPainter>
#include <QtCharts/QChart>

// POSIX socket API used directly through Socket.h / Channel.h
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ── Configuration tab: slider ─────────────────────────────────────────────
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(100);
    ui->horizontalSlider->setValue(static_cast<int>(m_threshold));
    ui->lcdNumber->display(static_cast<int>(m_threshold));
    connect(ui->horizontalSlider, &QSlider::sliderMoved,
            this, &MainWindow::onSliderMoved);

    // ── Configuration tab: checkboxes ─────────────────────────────────────────
    // The .ui already sets checkBox (TCP) = checked and wires the two
    // checkboxes so clicking one toggles the other (mutual exclusion).
    // No extra code needed here — we simply read isChecked() on Connect.

    // ── Build dynamic tabs ────────────────────────────────────────────────────
    setupGaugeTab();
    setupChartTab();

    // ── Protocol timer — starts only when a client is connected ──────────────
    m_serverTimer = new QTimer(this);
    m_serverTimer->setInterval(1000);
    connect(m_serverTimer, &QTimer::timeout, this, &MainWindow::onServerTick);

    // Set initial button appearance
    updateConnectButton();
}

MainWindow::~MainWindow()
{
    stopServer();
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab 1 – Real Time Monitor
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupGaugeTab()
{
    QWidget *tab = ui->tab_2;
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignCenter);

    m_monitorStatus = new QLabel(
        "Not connected — select protocol and press Connect.", tab);
    m_monitorStatus->setAlignment(Qt::AlignCenter);
    m_monitorStatus->setStyleSheet(
        "color:#aaaaaa; font-size:13px; padding:4px;");
    layout->addWidget(m_monitorStatus);

    m_gaugeWidget = new QQuickWidget(tab);
    m_gaugeWidget->setMinimumSize(650, 650);
    m_gaugeWidget->setMaximumSize(900, 900);
    m_gaugeWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_gaugeWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_gaugeWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_gaugeWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_gaugeWidget->setClearColor(Qt::transparent);
    m_gaugeWidget->rootContext()->setContextProperty("backend", this);
    m_gaugeWidget->setSource(QUrl("qrc:/new/prefix2/Gauge.qml"));
    layout->addWidget(m_gaugeWidget, 0, Qt::AlignCenter);

    m_threshInfoLabel = new QLabel(
        QString("Threshold: %1 °C").arg(m_threshold), tab);
    m_threshInfoLabel->setAlignment(Qt::AlignCenter);
    m_threshInfoLabel->setStyleSheet(
        "color:#cccccc; font-size:13px; font-weight:bold; padding:4px;");
    layout->addWidget(m_threshInfoLabel);

    tab->setLayout(layout);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab 2 – Historical Analysis
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupChartTab()
{
    QWidget *tab = ui->tab;
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(40, 30, 30, 60);
    layout->setSpacing(40);

    auto *title = new QLabel("Temperature History", tab);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "color:white; font-size:30px; font-weight:bold; padding:15px;");
    layout->addWidget(title);

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
    m_threshSeries->append(0,  m_threshold);
    m_threshSeries->append(60, m_threshold);

    auto *chart = new QChart();
    chart->addSeries(m_tempSeries);
    chart->addSeries(m_threshSeries);
    chart->setBackgroundBrush(QBrush(QColor("#1a1a2e")));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor("#16213e")));
    chart->setPlotAreaBackgroundVisible(true);
    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setBackgroundVisible(false);

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

    m_chartView = new QChartView(chart, tab);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_chartView);
    tab->setLayout(layout);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Configuration tab: Connect / Disconnect button
//  Slot name matches Qt's auto-connect convention: on_<objectName>_clicked
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_connectButton_clicked()
{
    // If already listening / connected → disconnect
    if (m_listenNotifier || m_udpNotifier || m_clientFd >= 0) {
        stopServer();
        return;
    }

    // Read protocol selection from the .ui checkboxes:
    //   checkBox   = TCP  (checked by default)
    //   checkBox_2 = UDP
    m_connType = ui->checkBox->isChecked()
                 ? ConnectionType::TCP
                 : ConnectionType::UDP;

    startServer();
}

// ─────────────────────────────────────────────────────────────────────────────
//  startServer
//  Assigns the chosen Socket subclass to ServerChannel and starts listening.
//  All networking goes through the Socket / Channel OOP interface.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::startServer()
{
    // Polymorphic assignment: ServerChannel holds a Socket* at runtime
    if (m_connType == ConnectionType::TCP)
        m_serverChannel.channelSocket = &m_tcpSock;   // TCPSocket → Socket*
    else
        m_serverChannel.channelSocket = &m_udpSock;   // UDPSocket → Socket*

    // Call waitForConnect() through the Channel interface.
    // Returns the fd to watch with QSocketNotifier, or -1 on failure.
    int listenFd = m_serverChannel.startListening();

    if (listenFd < 0) {
        m_monitorStatus->setText(
            "❌  Bind failed — port may be in use. Try again.");
        m_monitorStatus->setStyleSheet(
            "color:#e74c3c; font-size:13px; padding:4px;");
        return;
    }

    if (m_connType == ConnectionType::TCP) {
        // Watch the listen fd: becomes readable when a client tries to connect
        m_listenNotifier = new QSocketNotifier(
            listenFd, QSocketNotifier::Read, this);
        connect(m_listenNotifier, &QSocketNotifier::activated,
                this, &MainWindow::onListenFdActivated);

        m_monitorStatus->setText(
            "🔶  Listening on TCP :8080 — waiting for client…");
        m_monitorStatus->setStyleSheet(
            "color:#f39c12; font-size:13px; padding:4px;");

    } else {
        // UDP: the bound fd is immediately ready to receive datagrams
        m_udpNotifier = new QSocketNotifier(
            listenFd, QSocketNotifier::Read, this);
        connect(m_udpNotifier, &QSocketNotifier::activated,
                this, &MainWindow::onUdpFdReadable);

        // UDP is connectionless → start protocol timer immediately
        m_serverTimer->start();

        m_monitorStatus->setText(
            "🔶  Listening on UDP :8081 — waiting for client…");
        m_monitorStatus->setStyleSheet(
            "color:#f39c12; font-size:13px; padding:4px;");
    }

    // Lock checkboxes while server is running (prevent mid-session switching)
    ui->checkBox->setEnabled(false);
    ui->checkBox_2->setEnabled(false);

    updateConnectButton();
}

// ─────────────────────────────────────────────────────────────────────────────
//  stopServer — tears down notifiers and sockets via the OOP interface
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::stopServer()
{
    m_serverTimer->stop();
    m_clientFd = -1;

    // Always delete notifiers BEFORE closing file descriptors
    delete m_listenNotifier; m_listenNotifier = nullptr;
    delete m_clientNotifier; m_clientNotifier = nullptr;
    delete m_udpNotifier;    m_udpNotifier    = nullptr;

    // Shut down via Channel → Socket interface (OOP shutdown / polymorphism)
    m_serverChannel.stop();

    // Re-enable protocol selection
    ui->checkBox->setEnabled(true);
    ui->checkBox_2->setEnabled(true);

    m_monitorStatus->setText(
        "Disconnected — select protocol and press Connect.");
    m_monitorStatus->setStyleSheet(
        "color:#aaaaaa; font-size:13px; padding:4px;");

    updateConnectButton();
}

// ─────────────────────────────────────────────────────────────────────────────
//  QSocketNotifier: TCP listen fd readable → new client is waiting to connect
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onListenFdActivated(int /*fd*/)
{
    if (m_clientFd >= 0) return;   // already have a client — reject second

    // Accept via the TCPSocket object (safe downcast — we know the runtime type)
    TCPSocket *tcp = static_cast<TCPSocket *>(m_serverChannel.channelSocket);
    m_clientFd = tcp->acceptConnection();

    if (m_clientFd < 0) {
        m_monitorStatus->setText("❌  accept() failed.");
        return;
    }

    // Watch the new client fd for incoming temperature readings
    m_clientNotifier = new QSocketNotifier(
        m_clientFd, QSocketNotifier::Read, this);
    connect(m_clientNotifier, &QSocketNotifier::activated,
            this, &MainWindow::onClientFdReadable);

    // Push the current threshold to the client right away
    sendToClient("set threshold");
    sendToClient(QString::number(m_threshold, 'f', 1).toStdString());

    m_monitorStatus->setText(
        QString("✅  TCP client connected (fd %1)").arg(m_clientFd));
    m_monitorStatus->setStyleSheet(
        "color:#2ecc71; font-size:13px; padding:4px;");

    m_serverTimer->start();
    updateConnectButton();
}

// ─────────────────────────────────────────────────────────────────────────────
//  QSocketNotifier: TCP client fd readable → temperature data has arrived
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onClientFdReadable(int fd)
{
    char buf[256] = {};
    int n = ::recv(fd, buf, sizeof(buf) - 1, 0);

    if (n <= 0) {
        // Client closed the connection
        delete m_clientNotifier;
        m_clientNotifier = nullptr;
        ::close(m_clientFd);
        m_clientFd = -1;
        m_serverTimer->stop();

        m_monitorStatus->setText(
            "🔶  TCP client disconnected — waiting for reconnect…");
        m_monitorStatus->setStyleSheet(
            "color:#f39c12; font-size:13px; padding:4px;");
        updateConnectButton();
        return;
    }

    buf[n] = '\0';
    std::string raw(buf);
    while (!raw.empty() && (raw.back() == '\n' || raw.back() == '\r'))
        raw.pop_back();

    handleIncomingData(raw);
}

// ─────────────────────────────────────────────────────────────────────────────
//  QSocketNotifier: UDP fd readable → datagram has arrived
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onUdpFdReadable(int /*fd*/)
{
    // receiveFrom() stores the sender address so sendReply() can reach it
    UDPSocket *udp = static_cast<UDPSocket *>(m_serverChannel.channelSocket);
    std::string raw = udp->receiveFrom();

    while (!raw.empty() && (raw.back() == '\n' || raw.back() == '\r'))
        raw.pop_back();

    if (!raw.empty())
        handleIncomingData(raw);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Protocol timer tick (every 1 s)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onServerTick()
{
    // TCP: guard against timer firing before a client is accepted
    if (m_connType == ConnectionType::TCP && m_clientFd < 0) return;

    if (m_thresholdDirty) {
        sendToClient("set threshold");
        sendToClient(QString::number(m_threshold, 'f', 1).toStdString());
        m_thresholdDirty = false;
    } else {
        sendToClient("get temp");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  sendToClient — routes through the Socket OOP interface (polymorphism)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::sendToClient(const std::string &msg)
{
    const std::string line = msg + "\n";   // protocol lines are newline-terminated

    if (m_connType == ConnectionType::TCP) {
        if (m_clientFd < 0) return;
        // Calls TCPSocket::send() via the Socket* pointer — polymorphism
        m_serverChannel.channelSocket->send(line);
    } else {
        // UDP: sendReply() addresses the last sender captured by receiveFrom()
        static_cast<UDPSocket *>(m_serverChannel.channelSocket)->sendReply(line);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  handleIncomingData — parse temperature reading and update GUI
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::handleIncomingData(const std::string &raw)
{
    bool ok = false;
    double temp = QString::fromStdString(raw).toDouble(&ok);
    if (!ok) return;

    m_temperature = temp;
    emit temperatureChanged(m_temperature);
    addTemperatureSample(m_temperature);
    updateInfoLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Chart helper
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::addTemperatureSample(double temp)
{
    m_tempSeries->append(m_sampleIndex, temp);
    ++m_sampleIndex;

    int xMin = qMax(0, m_sampleIndex - 60);
    int xMax = qMax(60, m_sampleIndex);
    m_axisX->setRange(xMin, xMax);

    m_threshSeries->clear();
    m_threshSeries->append(xMin, m_threshold);
    m_threshSeries->append(xMax, m_threshold);

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
//  updateConnectButton — reflects current server state in the UI
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::updateConnectButton()
{
    const bool active = (m_listenNotifier || m_udpNotifier || m_clientFd >= 0);

    if (active) {
        ui->connectButton->setText("Disconnect");
        ui->connectButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #e74c3c;"
            "  color: white;"
            "  font-weight: bold;"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "}"
            "QPushButton:hover { background-color: #c0392b; }");
    } else {
        ui->connectButton->setText("Connect");
        ui->connectButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #2ecc71;"
            "  color: #1a1a2e;"
            "  font-weight: bold;"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "}"
            "QPushButton:hover { background-color: #27ae60; }");
    }
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
