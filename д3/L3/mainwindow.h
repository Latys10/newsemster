#pragma once
#include <QMainWindow>
#include "BinaryTree.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnLoad_clicked();
    void on_btnPreorder_clicked();
    void on_btnHeight_clicked();
    void on_btnSave_clicked();

private:
    Ui::MainWindow* ui;     // доступ ко всем элементам формы
    BinaryTree m_tree;
    QString    m_lastResult;
};
