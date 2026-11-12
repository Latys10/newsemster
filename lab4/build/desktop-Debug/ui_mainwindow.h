/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include "mapwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    MapWidget *mapWidget;
    QLabel *label;
    QListWidget *listA;
    QListWidget *listB;
    QPushButton *btnGo;
    QPushButton *btnTraffic;
    QTextEdit *txtRoute;
    QPushButton *btnSave;
    QLabel *label_2;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(796, 725);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mapWidget = new MapWidget(centralwidget);
        mapWidget->setObjectName("mapWidget");
        mapWidget->setGeometry(QRect(9, 9, 762, 512));
        mapWidget->setMinimumSize(QSize(762, 512));
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(20, 570, 71, 16));
        listA = new QListWidget(centralwidget);
        listA->setObjectName("listA");
        listA->setGeometry(QRect(110, 530, 141, 101));
        listB = new QListWidget(centralwidget);
        listB->setObjectName("listB");
        listB->setGeometry(QRect(280, 530, 141, 101));
        btnGo = new QPushButton(centralwidget);
        btnGo->setObjectName("btnGo");
        btnGo->setGeometry(QRect(570, 540, 91, 41));
        btnTraffic = new QPushButton(centralwidget);
        btnTraffic->setObjectName("btnTraffic");
        btnTraffic->setGeometry(QRect(570, 590, 91, 41));
        txtRoute = new QTextEdit(centralwidget);
        txtRoute->setObjectName("txtRoute");
        txtRoute->setGeometry(QRect(450, 540, 104, 70));
        btnSave = new QPushButton(centralwidget);
        btnSave->setObjectName("btnSave");
        btnSave->setGeometry(QRect(680, 560, 101, 41));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(140, 640, 71, 21));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(320, 640, 71, 21));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 796, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\320\234\320\260\321\200\321\210\321\200\321\203\321\202", nullptr));
        btnGo->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\321\201\321\202\321\200\320\276\320\270\321\202\321\214", nullptr));
        btnTraffic->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\263\321\200\321\203\320\266\320\265\320\275\320\275\320\276\321\201\321\202\321\214", nullptr));
        btnSave->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\320\242\320\276\321\207\320\272\320\260 \320\220", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\320\242\320\276\321\207\320\272\320\260 \320\221", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
