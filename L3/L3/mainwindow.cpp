#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeView->setTree(&m_tree);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnLoad_clicked() {
    QString name = QFileDialog::getOpenFileName(
        this, "Выберите файл с деревом", QString(),
        "Текстовые файлы (*.txt);;Все файлы (*)");
    if (name.isEmpty())
        return;

    if (!m_tree.loadFromFile(name))
        QMessageBox::warning(this, "Ошибка", "Не удалось прочитать дерево из файла.");

    m_lastResult.clear();
    ui->preorderLabel->setText("Префиксный обход: -");
    ui->heightLabel->setText("Высота дерева: -");
    ui->treeView->update();
}

void MainWindow::on_btnPreorder_clicked() {
    if (m_tree.isEmpty()) {
        QMessageBox::information(this, "Внимание", "Сначала загрузите дерево.");
        return;
    }
    QStringList parts;
    for (int v : m_tree.preorder())
        parts << QString::number(v);

    m_lastResult = parts.join(" ");
    ui->preorderLabel->setText("Префиксный обход (УЛП): " + m_lastResult);
}

void MainWindow::on_btnHeight_clicked() {
    ui->heightLabel->setText("Высота дерева: " + QString::number(m_tree.height()));
}

void MainWindow::on_btnSave_clicked() {
    if (m_lastResult.isEmpty()) {
        QMessageBox::information(this, "Внимание", "Сначала выполните обход.");
        return;
    }
    QString name = QFileDialog::getSaveFileName(
        this, "Сохранить результат", "preorder.txt", "Текстовые файлы (*.txt)");
    if (name.isEmpty())
        return;

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }
    QTextStream out(&file);
    out << m_lastResult << "\n";
}
