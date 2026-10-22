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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "treewidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QPushButton *btnLoad;
    TreeWidget *treeView;
    QPushButton *btnPreorder;
    QLabel *preorderLabel;
    QPushButton *btnHeight;
    QLabel *heightLabel;
    QPushButton *btnSave;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(394, 508);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        btnLoad = new QPushButton(centralwidget);
        btnLoad->setObjectName("btnLoad");

        verticalLayout->addWidget(btnLoad);

        treeView = new TreeWidget(centralwidget);
        treeView->setObjectName("treeView");
        treeView->setMinimumSize(QSize(300, 250));
        treeView->setMaximumSize(QSize(420, 250));

        verticalLayout->addWidget(treeView);

        btnPreorder = new QPushButton(centralwidget);
        btnPreorder->setObjectName("btnPreorder");

        verticalLayout->addWidget(btnPreorder);

        preorderLabel = new QLabel(centralwidget);
        preorderLabel->setObjectName("preorderLabel");

        verticalLayout->addWidget(preorderLabel);

        btnHeight = new QPushButton(centralwidget);
        btnHeight->setObjectName("btnHeight");

        verticalLayout->addWidget(btnHeight);

        heightLabel = new QLabel(centralwidget);
        heightLabel->setObjectName("heightLabel");

        verticalLayout->addWidget(heightLabel);

        btnSave = new QPushButton(centralwidget);
        btnSave->setObjectName("btnSave");

        verticalLayout->addWidget(btnSave);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 394, 21));
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
        btnLoad->setText(QCoreApplication::translate("MainWindow", "\320\267\320\260\320\263\321\200\321\203\320\267\320\272\320\260", nullptr));
        btnPreorder->setText(QCoreApplication::translate("MainWindow", "\320\276\320\261\321\205\320\276\320\264", nullptr));
        preorderLabel->setText(QString());
        btnHeight->setText(QCoreApplication::translate("MainWindow", "\320\262\321\213\321\201\320\276\321\202\320\260", nullptr));
        heightLabel->setText(QString());
        btnSave->setText(QCoreApplication::translate("MainWindow", "\321\201\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
