#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <random>

#include "morse.h"

namespace {
QString fromText(const morse::Text& text) { return QString::fromStdU32String(text); }
QString fromStd(const std::string& s) { return QString::fromUtf8(s.data(), static_cast<int>(s.size())); }
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Таблица вариантов: колонки и режим выделения задаём здесь.
    ui->tblVariants->setColumnCount(3);
    ui->tblVariants->setHorizontalHeaderLabels({tr("№"), tr("Текст"), tr("Правок")});
    ui->tblVariants->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblVariants->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblVariants->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblVariants->verticalHeader()->setVisible(false);
    ui->tblVariants->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Пункт 1: кодирование
    connect(ui->btnEncLoad, &QPushButton::clicked, this, [this] { loadInto(ui->txtPlain, tr("Загрузить текст")); });
    connect(ui->btnEncRun, &QPushButton::clicked, this, &MainWindow::onEncode);
    connect(ui->btnEncSave, &QPushButton::clicked, this, [this] {
        saveText(ui->txtMorse->toPlainText(), tr("Сохранить код Морзе"), QStringLiteral("morse_code.txt"));
    });

    // Пункт 2: передача
    connect(ui->btnTxLoad, &QPushButton::clicked, this, [this] { loadInto(ui->txtTxIn, tr("Загрузить код Морзе")); });
    connect(ui->btnTxRun, &QPushButton::clicked, this, &MainWindow::onTransmit);
    connect(ui->btnTxSave, &QPushButton::clicked, this, [this] {
        saveText(ui->txtTxOut->toPlainText(), tr("Сохранить принятый код"), QStringLiteral("received_code.txt"));
    });

    // Пункт 3: декодирование
    connect(ui->btnDecLoad, &QPushButton::clicked, this, [this] { loadInto(ui->txtDecIn, tr("Загрузить код Морзе")); });
    connect(ui->btnDecRun, &QPushButton::clicked, this, &MainWindow::onDecode);
    connect(ui->btnDecRestore, &QPushButton::clicked, this, [this] { startRestoration(ui->txtDecIn->toPlainText()); });
    connect(ui->btnDecSave, &QPushButton::clicked, this, [this] {
        saveText(ui->txtDecOut->toPlainText(), tr("Сохранить текст"), QStringLiteral("decoded_text.txt"));
    });

    // Пункт 4: восстановление
    connect(ui->btnRestLoad, &QPushButton::clicked, this, [this] { loadInto(ui->txtRestIn, tr("Загрузить искажённый код")); });
    connect(ui->btnRestRun, &QPushButton::clicked, this, &MainWindow::onRestore);
    connect(ui->btnRestSave, &QPushButton::clicked, this, &MainWindow::onSaveVariant);
    connect(ui->tblVariants, &QTableWidget::itemSelectionChanged, this, &MainWindow::onVariantSelected);
}

MainWindow::~MainWindow() { delete ui; }

// ---------- Общие помощники ----------

morse::Pipeline MainWindow::makePipeline() const {
    return morse::Pipeline(static_cast<morse::ChannelMode>(ui->cmbChannel->currentIndex()),
                           ui->spnPanic->value(), ui->spnLoss->value());
}

bool MainWindow::readCode(QPlainTextEdit* edit, morse::Code& code) {
    try {
        code = morse::normalizeCode(edit->toPlainText().toUtf8().toStdString());
        return true;
    } catch (const std::invalid_argument& e) {
        QMessageBox::warning(this, tr("Ошибка формата"), fromStd(e.what()));
        return false;
    }
}

QString MainWindow::md5Hex(const std::string& data) {
    const QByteArray hash = QCryptographicHash::hash(QByteArray::fromStdString(data), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

void MainWindow::loadInto(QPlainTextEdit* edit, const QString& title) {
    const QString path = QFileDialog::getOpenFileName(this, title, QString(), tr("Текстовые файлы (*.txt);;Все файлы (*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, title, tr("Не удалось открыть файл:\n%1").arg(file.errorString()));
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);  // BOM
    edit->setPlainText(text);
    ui->statusbar->showMessage(tr("Загружено: %1").arg(path));
}

void MainWindow::saveText(const QString& content, const QString& title, const QString& defaultName) {
    if (content.trimmed().isEmpty()) {
        QMessageBox::information(this, title, tr("Нечего сохранять."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, title, defaultName, tr("Текстовые файлы (*.txt);;Все файлы (*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, title, tr("Не удалось записать файл:\n%1").arg(file.errorString()));
        return;
    }
    file.write(content.toUtf8());
    ui->statusbar->showMessage(tr("Сохранено: %1").arg(path));
}

// ---------- Пункт 1 ----------

void MainWindow::onEncode() {
    const morse::Text text = ui->txtPlain->toPlainText().toStdU32String();
    std::size_t skipped = 0;
    const morse::Code code = morse::encodeText(text, &skipped);
    const QString shown = fromStd(morse::formatCode(code));
    ui->txtMorse->setPlainText(shown);
    ui->txtTxIn->setPlainText(shown);  // сразу передаём на вкладку «Передача»

    QString message = tr("Закодировано. MD5 кода: %1").arg(md5Hex(code));
    if (skipped > 0)
        message += tr(" (пропущено символов вне таблицы: %1)").arg(static_cast<qulonglong>(skipped));
    ui->statusbar->showMessage(message);
}

// ---------- Пункт 2 ----------

void MainWindow::onTransmit() {
    morse::Code code;
    if (!readCode(ui->txtTxIn, code)) return;

    const int seed = ui->spnSeed->value();
    std::mt19937 rng(seed != 0 ? static_cast<unsigned>(seed) : std::random_device{}());

    const morse::Pipeline pipeline = makePipeline();
    const morse::Code received = pipeline.transmit(code, rng);
    const QString shown = fromStd(morse::formatCode(received));
    ui->txtTxOut->setPlainText(shown);
    ui->txtDecIn->setPlainText(shown);   // и на вкладки декодирования и восстановления
    ui->txtRestIn->setPlainText(shown);
    ui->statusbar->showMessage(tr("Передача выполнена, %1").arg(fromStd(pipeline.describe())));
}

// ---------- Пункт 3 ----------

void MainWindow::onDecode() {
    morse::Code code;
    if (!readCode(ui->txtDecIn, code)) return;

    const morse::DecodeResult result = morse::decodeCode(code);
    ui->txtDecOut->setPlainText(fromText(result.text));
    if (!result.hasErrors) {
        ui->statusbar->showMessage(tr("Ошибок не обнаружено (скрытые ошибки можно искать кнопкой «Восстановить»)"));
        return;
    }
    const auto answer = QMessageBox::question(
        this, tr("Обнаружена ошибка"),
        tr("Не удалось распознать букв: %1. Запустить восстановление?").arg(static_cast<qulonglong>(result.badTokens.size())));
    if (answer == QMessageBox::Yes) startRestoration(ui->txtDecIn->toPlainText());
}

// ---------- Пункт 4 ----------

void MainWindow::startRestoration(const QString& codeText) {
    ui->txtRestIn->setPlainText(codeText);
    ui->tabWidget->setCurrentWidget(ui->tabRestore);
    onRestore();
}

void MainWindow::onRestore() {
    morse::Code code;
    if (!readCode(ui->txtRestIn, code)) return;

    const morse::Pipeline pipeline = makePipeline();
    const morse::RestoreReport report = pipeline.restore(code);
    variants_ = report.variants;

    QStringList log;
    log << tr("Порядок: %1").arg(fromStd(pipeline.describe()));
    int iteration = 1;
    for (const morse::RestoreStepInfo& step : report.steps) {
        log << tr("Итерация %1. Отмена «%2»: кандидатов на входе %3, на выходе %4%5")
                   .arg(iteration++)
                   .arg(fromStd(step.name))
                   .arg(static_cast<qulonglong>(step.inputCount))
                   .arg(static_cast<qulonglong>(step.outputCount))
                   .arg(step.truncated ? tr(" (список урезан)") : QString());
    }
    log << tr("Итого вариантов: %1").arg(static_cast<qulonglong>(variants_.size()));
    ui->txtLog->setPlainText(log.join(QStringLiteral("\n")));

    fillVariantsTable();
    if (variants_.empty()) {
        QMessageBox::information(this, tr("Восстановление"),
                                 tr("Ни один вариант не согласуется с моделью нарушений.\n"
                                    "Проверьте, что выбран тот же режим канала, что и при передаче."));
    }
}

void MainWindow::fillVariantsTable() {
    QTableWidget* table = ui->tblVariants;
    table->clearContents();
    table->setRowCount(static_cast<int>(variants_.size()));
    for (int row = 0; row < static_cast<int>(variants_.size()); ++row) {
        const morse::Candidate& v = variants_[static_cast<std::size_t>(row)];
        table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        table->setItem(row, 1, new QTableWidgetItem(fromText(morse::decodeCode(v.code).text)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(v.cost)));
    }
    ui->txtVariantCode->clear();
    if (!variants_.empty()) table->selectRow(0);
}

void MainWindow::onVariantSelected() {
    const int row = ui->tblVariants->currentRow();
    if (row < 0 || row >= static_cast<int>(variants_.size())) return;
    const morse::Code& code = variants_[static_cast<std::size_t>(row)].code;
    ui->txtVariantCode->setPlainText(fromStd(morse::formatCode(code)));
    ui->statusbar->showMessage(tr("Вариант %1, MD5 кода: %2").arg(row + 1).arg(md5Hex(code)));
}

void MainWindow::onSaveVariant() {
    const int row = ui->tblVariants->currentRow();
    if (row < 0 || row >= static_cast<int>(variants_.size())) {
        QMessageBox::information(this, tr("Сохранение"), tr("Сначала выберите вариант в таблице."));
        return;
    }
    saveText(fromStd(morse::formatCode(variants_[static_cast<std::size_t>(row)].code)),
             tr("Сохранить код выбранного варианта"), QStringLiteral("restored_code.txt"));
}
