#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QListWidget>
#include <QSignalBlocker>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);                       // создаём элементы, нарисованные в Designer
    buildGraph();                            // строим граф (все дороги сначала зелёные)

    ui->mapWidget->setGraph(&graph);         // отдаём граф виджету карты
    ui->txtRoute->setReadOnly(true);         // в поле маршрута нельзя печатать
    refreshLists();                          // заполняем листбоксы

    // соединяем сигналы (событие) со слотами (обработчик)
    connect(ui->btnTraffic, &QPushButton::clicked, this, &MainWindow::onTrafficClicked);
    connect(ui->btnGo,      &QPushButton::clicked, this, &MainWindow::onGoClicked);
    connect(ui->btnSave,    &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->listA, &QListWidget::currentItemChanged, this, &MainWindow::onListAChanged);
    connect(ui->listB, &QListWidget::currentItemChanged, this, &MainWindow::onListBChanged);
    connect(ui->mapWidget, &MapWidget::vertexDoubleClicked, this, &MainWindow::onVertexPicked);
    connect(ui->mapWidget, &MapWidget::vertexTripleClicked, this, &MainWindow::onVertexRemoved);
}

MainWindow::~MainWindow() { delete ui; }

// Ищем номер вершины по её имени
int MainWindow::vertexId(const QString &name) const
{
    for (int i = 0; i < graph.vertexCount(); ++i)
        if (graph.vertex(i).name == name) return i;
    return -1;
}

void MainWindow::buildGraph()
{
    // ---------- Вершины: имя и координаты на картинке (система 762x512) ----------
    graph.addVertex("А", {150,  58});  graph.addVertex("Б", {252,  84});
    graph.addVertex("В", {303,  97});  graph.addVertex("Г", {369, 110});
    graph.addVertex("Д", {462, 147});  graph.addVertex("Е", {537, 173});
    graph.addVertex("Ж", {133, 229});  graph.addVertex("З", {362, 233});
    graph.addVertex("К", {568, 232});  graph.addVertex("Л", { 58, 345});
    graph.addVertex("М", {200, 316});  graph.addVertex("Н", {247, 315});
    graph.addVertex("О", {332, 316});  graph.addVertex("П", {402, 297});
    graph.addVertex("Р", {530, 288});  graph.addVertex("С", {569, 284});
    graph.addVertex("Т", {720, 393});  graph.addVertex("У", {697, 478});
    graph.addVertex("Ф", { 66, 500});  graph.addVertex("Х", {402, 481});

    // ---------- Дополнительные вершины ----------
    graph.addVertex("Я", {367, 290});  // на ул. Багратиона между О и П
    graph.addVertex("Ч", {667, 316});  // у реки, между С и Т
    graph.addVertex("Ш", {587, 355});  // у парка, между Р и Т

    // ---------- Рёбра (дороги) ----------
    // oneWay = false: двустороннее движение, true: только от первой вершины ко второй
    struct Road { const char *a; const char *b; bool oneWay; };
    const Road roads[] = {
        {"А","Б",false},{"Б","В",false},{"В","Г",false},                       // верхняя улица
        {"Г","Д",true },{"Д","Е",true },                                       // Г -> Д -> Е (в одну сторону)
        {"Е","К",false},                                                       // набережная
        {"Г","З",false},{"Ж","З",false},{"З","К",false},                       // средние улицы
        {"Л","Ф",false},                                                       // проспект Ленинский
        {"Л","М",false},{"М","Н",false},{"Н","О",false},{"П","Р",false},       // ул. Багратиона
        {"О","Я",false},{"Я","П",true },                                       // Я -> П (в одну сторону)
        {"Р","С",false},
        {"П","Х",false},{"Ф","Х",false},{"Х","У",false},                       // нижние улицы
        {"С","Ч",false},{"Т","Ч",true },                                       // Т -> Ч (в одну сторону)
        {"Р","Ш",true },{"Ш","Т",true },                                       // Р -> Ш -> Т (в одну сторону)
        {"Т","У",false},{"З","Я",false},{"К","С",false},{"А","Л",false}
        // Рёбра З-Я, К-С и А-Л удалены: граф распался на две несвязные части
    };
    for (const Road &r : roads)
        graph.addEdge(vertexId(QString::fromUtf8(r.a)), vertexId(QString::fromUtf8(r.b)), r.oneWay);
}

// Заполняем один листбокс. Каждой строке прячем в UserRole НОМЕР вершины
void MainWindow::fillList(QListWidget *list, int selected, int hidden, const QVector<bool> *reach)
{
    list->clear();
    for (int i = 0; i < graph.vertexCount(); ++i) {
        if (graph.vertex(i).removed || i == hidden) continue;   // удалённых и выбранную в другом списке не показываем
        QListWidgetItem *item = new QListWidgetItem(graph.vertex(i).name);
        item->setData(Qt::UserRole, i);                          // запоминаем номер вершины
        if (reach && !(*reach)[i]) {                             // недостижимая из А: красный текст
            item->setForeground(QColor(220, 40, 40));
            item->setToolTip("Недостижимо");
        }
        list->addItem(item);
        if (i == selected) list->setCurrentItem(item);           // возвращаем выделение
    }
}

// Пересобираем оба листбокса (сигналы на это время блокируем)
void MainWindow::refreshLists()
{
    QSignalBlocker blockA(ui->listA);
    QSignalBlocker blockB(ui->listB);
    QVector<bool> reach = graph.reachableFrom(selA);
    fillList(ui->listA, selA, selB, nullptr);                              // в А нет вершины Б
    fillList(ui->listB, selB, selA, selA >= 0 ? &reach : nullptr);         // в Б нет вершины А
}

// Говорим карте, из какой вершины считать недостижимость
void MainWindow::updateReach()
{
    ui->mapWidget->setReach(graph.reachableFrom(selA), selA);
}

// Выбор точек изменился: старый маршрут неактуален
void MainWindow::afterSelectionChanged()
{
    routeActive = false;
    ui->mapWidget->clearRoute();
    ui->txtRoute->clear();
    refreshLists();
    updateReach();
}

// Выбор в листбоксе. Список пересобираем чуть позже (из обработчика его сигнала это небезопасно)
void MainWindow::onListAChanged(QListWidgetItem *current, QListWidgetItem *)
{
    selA = current ? current->data(Qt::UserRole).toInt() : -1;
    pickA = false;                                       // следующий двойной клик пойдёт в Б
    QTimer::singleShot(0, this, [this] { afterSelectionChanged(); });
}

void MainWindow::onListBChanged(QListWidgetItem *current, QListWidgetItem *)
{
    selB = current ? current->data(Qt::UserRole).toInt() : -1;
    pickA = true;                                        // следующий двойной клик пойдёт в А
    QTimer::singleShot(0, this, [this] { afterSelectionChanged(); });
}

// Строим маршрут между selA и selB. interactive = true: показываем окна с сообщениями
bool MainWindow::buildRoute(bool interactive)
{
    if (selA < 0 || selB < 0) {
        if (interactive) {
            QMessageBox::warning(this, "Маршрут", "Выберите обе точки.");
        } else {                                         // автоперестроение: просто убираем маршрут
            routeActive = false;
            ui->mapWidget->clearRoute();
            ui->txtRoute->clear();
        }
        return false;
    }

    QVector<int> pv, pe;
    double total = 0;
    if (!graph.shortestPath(selA, selB, pv, pe, total)) {   // пути нет
        routeActive = false;
        ui->mapWidget->clearRoute();
        ui->txtRoute->setText("Недостижимо: маршрута нет");
        if (interactive)
            QMessageBox::information(this, "Маршрут", "Недостижимо: маршрут между выбранными точками не найден.");
        return false;
    }

    QStringList names;
    for (int v : pv) names << graph.vertex(v).name;      // имена вершин маршрута по порядку
    ui->txtRoute->setText(names.join("-") + QString("\nДлина с учётом загруженности: %1")
                                                .arg(total, 0, 'f', 2));
    ui->mapWidget->setRoute(pv, pe);                     // подсвечиваем маршрут на карте
    routeActive = true;                                  // теперь маршрут "живой": будет перестраиваться
    return true;
}

// Кнопка «Загруженность»
void MainWindow::onTrafficClicked()
{
    graph.randomizeCongestion();             // новые уровни загруженности
    if (routeActive)
        buildRoute(false);                   // путь сам перестраивается под новые веса
    ui->mapWidget->update();                 // перерисовать карту с новыми цветами
}

// Кнопка «Вперёд!»
void MainWindow::onGoClicked()
{
    buildRoute(true);
}

// Кнопка «Сохранить»
void MainWindow::onSaveClicked()
{
    QString text = ui->txtRoute->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(this, "Сохранение", "Сначала постройте маршрут.");
        return;
    }
    QString fn = QFileDialog::getSaveFileName(this, "Сохранить маршрут", "route.txt", "Текст (*.txt)");
    if (fn.isEmpty()) return;
    QFile f(fn);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(text.toUtf8());              // пишем в UTF-8
}

// Двойной клик по вершине на карте: выбираем её в А или Б по очереди
void MainWindow::onVertexPicked(int index)
{
    if (graph.vertex(index).removed) return;             // удалённую выбрать нельзя
    if (pickA) { if (index == selB) return; selA = index; }   // одну вершину в обе точки нельзя
    else       { if (index == selA) return; selB = index; }
    pickA = !pickA;
    afterSelectionChanged();
}

// Тройной клик: вершина удаляется (чернеет), повторный тройной клик возвращает её
void MainWindow::onVertexRemoved(int index)
{
    bool nowRemoved = !graph.vertex(index).removed;
    graph.setRemoved(index, nowRemoved);
    if (nowRemoved) {
        if (selA == index) { selA = -1; pickA = true;  }  // освободилась точка А
        if (selB == index) { selB = -1; pickA = false; }  // освободилась точка Б
    }
    refreshLists();                          // удалённая пропадает из списков
    updateReach();                           // недостижимые пересчитываются
    if (routeActive)
        buildRoute(false);                   // маршрут обходит удалённую вершину
    ui->mapWidget->update();
}
