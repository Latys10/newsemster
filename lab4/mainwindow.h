#pragma once
#include <QMainWindow>
#include "graph.h"

class QListWidget;
class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTrafficClicked();          // кнопка «Загруженность»
    void onGoClicked();               // кнопка «Вперёд!»
    void onSaveClicked();             // кнопка «Сохранить»
    void onVertexPicked(int index);   // двойной клик по вершине на карте (выбор)
    void onVertexRemoved(int index);  // тройной клик по вершине (удалить / вернуть)
    void onListAChanged(QListWidgetItem *current, QListWidgetItem *previous);   // выбор в listA
    void onListBChanged(QListWidgetItem *current, QListWidgetItem *previous);   // выбор в listB

private:
    void buildGraph();                // заполняем граф вершинами и рёбрами
    int vertexId(const QString &name) const;   // номер вершины по имени

    void refreshLists();              // пересобрать оба листбокса
    void fillList(QListWidget *list, int selected, int hidden, const QVector<bool> *reach);
    void updateReach();               // передать карте, какие вершины недостижимы
    void afterSelectionChanged();     // действия после смены выбранных точек
    bool buildRoute(bool interactive);// построить маршрут между выбранными точками

    Ui::MainWindow *ui;
    Graph graph;                      // сам граф
    int selA = -1;                    // номер выбранной вершины А (-1 = нет)
    int selB = -1;                    // номер выбранной вершины Б (-1 = нет)
    bool pickA = true;                // какой точке достанется следующий двойной клик
    bool routeActive = false;         // показан ли сейчас маршрут (нужно ли его перестраивать)
};
