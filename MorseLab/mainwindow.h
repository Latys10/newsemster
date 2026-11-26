#pragma once

#include <QMainWindow>
#include <vector>

#include "pipeline.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPlainTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onEncode();           // пункт 1: текст -> код
    void onTransmit();         // пункт 2: наложение нарушений
    void onDecode();           // пункт 3: код -> текст, при ошибке предлагает восстановление
    void onRestore();          // пункт 4: все варианты восстановления
    void onVariantSelected();  // показать код выбранного варианта
    void onSaveVariant();      // сохранить код выбранного варианта

private:
    morse::Pipeline makePipeline() const;                    // собирает цепочку по настройкам формы
    bool readCode(QPlainTextEdit* edit, morse::Code& code);  // поле -> нормализованный поток
    void startRestoration(const QString& codeText);          // перейти на вкладку 4 и запустить
    void fillVariantsTable();
    void loadInto(QPlainTextEdit* edit, const QString& title);
    void saveText(const QString& content, const QString& title, const QString& defaultName);
    static QString md5Hex(const std::string& data);

    Ui::MainWindow* ui;
    std::vector<morse::Candidate> variants_;  // варианты, показанные в таблице
};
