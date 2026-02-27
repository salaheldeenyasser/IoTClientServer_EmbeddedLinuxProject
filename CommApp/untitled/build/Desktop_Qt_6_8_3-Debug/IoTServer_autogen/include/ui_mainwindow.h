/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QTabWidget *tabWidget;
    QWidget *tab_2;
    QWidget *tab;
    QWidget *tab_3;
    QHBoxLayout *horizontalLayout_4;
    QGridLayout *gridLayout;
    QLCDNumber *lcdNumber;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *verticalSpacer;
    QCheckBox *checkBox;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label;
    QLabel *label_3;
    QPushButton *connectButton;
    QSlider *horizontalSlider;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QSpacerItem *verticalSpacer_3;
    QCheckBox *checkBox_2;
    QSpacerItem *verticalSpacer_4;
    QWidget *tab_4;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(920, 920);
        MainWindow->setMinimumSize(QSize(720, 720));
        MainWindow->setMaximumSize(QSize(1200, 1200));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout_3 = new QHBoxLayout(centralwidget);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        tab_2->setStyleSheet(QString::fromUtf8("#tab_2 {\n"
"    border-image: url(:/new/prefix1/Imagesandicons/Background.jpg) 0 0 0 0 stretch stretch;\n"
"}"));
        tabWidget->addTab(tab_2, QString());
        tab = new QWidget();
        tab->setObjectName("tab");
        tab->setStyleSheet(QString::fromUtf8("#tab {\n"
"    border-image: url(:/new/prefix1/Imagesandicons/Background.jpg) 0 0 0 0 stretch stretch;\n"
"}"));
        tabWidget->addTab(tab, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        tab_3->setStyleSheet(QString::fromUtf8("#tab_3 {\n"
"    border-image: url(:/new/prefix1/Imagesandicons/Background.jpg) 0 0 0 0 stretch stretch;\n"
"}\n"
"QLabel {\n"
"    color: white;\n"
"    font-size: 14px;\n"
"}\n"
"QSlider::groove:horizontal {\n"
"    height: 8px;\n"
"    background: #2d2d44;\n"
"    border-radius: 4px;\n"
"}\n"
"QSlider::handle:horizontal {\n"
"    background: #2ecc71;\n"
"    width: 22px;\n"
"    height: 22px;\n"
"    margin: -7px 0;\n"
"    border-radius: 11px;\n"
"}\n"
"QSlider::sub-page:horizontal {\n"
"    background: #2ecc71;\n"
"    border-radius: 4px;\n"
"}\n"
"QLCDNumber {\n"
"    background: #16213e;\n"
"    color: #2ecc71;\n"
"    border: 2px solid #0f3460;\n"
"    border-radius: 6px;\n"
"    font-size: 32px;\n"
"}"));
        horizontalLayout_4 = new QHBoxLayout(tab_3);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        lcdNumber = new QLCDNumber(tab_3);
        lcdNumber->setObjectName("lcdNumber");
        lcdNumber->setMinimumSize(QSize(160, 60));
        lcdNumber->setFrameShape(QFrame::Shape::StyledPanel);
        lcdNumber->setFrameShadow(QFrame::Shadow::Sunken);
        lcdNumber->setDigitCount(3);

        gridLayout->addWidget(lcdNumber, 13, 2, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 200, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 0, 2, 1, 1);

        verticalSpacer = new QSpacerItem(5, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(verticalSpacer, 6, 2, 1, 1);

        checkBox = new QCheckBox(tab_3);
        checkBox->setObjectName("checkBox");
        checkBox->setChecked(true);
        checkBox->setTristate(false);

        gridLayout->addWidget(checkBox, 2, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(220, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 8, 3, 1, 1);

        label = new QLabel(tab_3);
        label->setObjectName("label");
        label->setStyleSheet(QString::fromUtf8("QLabel { font-size:15px; font-weight:bold; padding:8px; }"));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(label, 12, 2, 1, 1);

        label_3 = new QLabel(tab_3);
        label_3->setObjectName("label_3");
        label_3->setStyleSheet(QString::fromUtf8("QLabel { font-size:15px; font-weight:bold; padding:8px; }"));

        gridLayout->addWidget(label_3, 8, 2, 1, 1);

        connectButton = new QPushButton(tab_3);
        connectButton->setObjectName("connectButton");
        connectButton->setCheckable(false);

        gridLayout->addWidget(connectButton, 4, 2, 1, 1);

        horizontalSlider = new QSlider(tab_3);
        horizontalSlider->setObjectName("horizontalSlider");
        horizontalSlider->setMinimum(0);
        horizontalSlider->setMaximum(100);
        horizontalSlider->setValue(50);
        horizontalSlider->setOrientation(Qt::Orientation::Horizontal);
        horizontalSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
        horizontalSlider->setTickInterval(10);

        gridLayout->addWidget(horizontalSlider, 10, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(220, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 8, 1, 1, 1);

        label_2 = new QLabel(tab_3);
        label_2->setObjectName("label_2");
        label_2->setStyleSheet(QString::fromUtf8("QLabel { font-size:15px; font-weight:bold; padding:8px; }"));
        label_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(label_2, 1, 2, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 120, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 14, 2, 1, 1);

        checkBox_2 = new QCheckBox(tab_3);
        checkBox_2->setObjectName("checkBox_2");

        gridLayout->addWidget(checkBox_2, 3, 2, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(verticalSpacer_4, 11, 2, 1, 1);


        horizontalLayout_4->addLayout(gridLayout);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName("tab_4");
        tab_4->setStyleSheet(QString::fromUtf8("#tab_4 {\n"
"    border-image: url(:/new/prefix1/Imagesandicons/Background.jpg) 0 0 0 0 stretch stretch;\n"
"}\n"
"QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,30);\n"
"    border-radius: 12px;\n"
"}"));
        horizontalLayout_2 = new QHBoxLayout(tab_4);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pushButton = new QPushButton(tab_4);
        pushButton->setObjectName("pushButton");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/Imagesandicons/facebook-app-symbol.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButton->setIcon(icon);
        pushButton->setIconSize(QSize(90, 90));

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(tab_4);
        pushButton_2->setObjectName("pushButton_2");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/Imagesandicons/linkedin.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButton_2->setIcon(icon1);
        pushButton_2->setIconSize(QSize(90, 90));

        horizontalLayout->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(tab_4);
        pushButton_3->setObjectName("pushButton_3");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix1/Imagesandicons/instagram.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButton_3->setIcon(icon2);
        pushButton_3->setIconSize(QSize(90, 90));

        horizontalLayout->addWidget(pushButton_3);


        horizontalLayout_2->addLayout(horizontalLayout);

        tabWidget->addTab(tab_4, QString());

        horizontalLayout_3->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 920, 23));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);
        QObject::connect(checkBox_2, &QCheckBox::clicked, checkBox, qOverload<>(&QCheckBox::toggle));
        QObject::connect(checkBox, &QCheckBox::clicked, checkBox_2, qOverload<>(&QCheckBox::toggle));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "IoT Server \342\200\224 Edges For Training", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow", "Real Time Monitor", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "Historical Analysis", nullptr));
        checkBox->setText(QCoreApplication::translate("MainWindow", "TCP", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Current threshold (\302\260C):", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Slide to configure the temperature threshold", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Select Connection Type: ", nullptr));
        checkBox_2->setText(QCoreApplication::translate("MainWindow", "UDP", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("MainWindow", "Configuration", nullptr));
#if QT_CONFIG(tooltip)
        pushButton->setToolTip(QCoreApplication::translate("MainWindow", "Open Facebook \342\200\223 Edges For Training", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton->setText(QString());
#if QT_CONFIG(tooltip)
        pushButton_2->setToolTip(QCoreApplication::translate("MainWindow", "Open LinkedIn \342\200\223 Edges For Training", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_2->setText(QString());
#if QT_CONFIG(tooltip)
        pushButton_3->setToolTip(QCoreApplication::translate("MainWindow", "Open Instagram \342\200\223 Edges For Training", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_3->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("MainWindow", "Quick Access", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
