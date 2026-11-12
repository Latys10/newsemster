#pragma once
#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QElapsedTimer>
#include "graph.h"

class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setGraph(const Graph *graph);                                  // какой граф рисовать
    void setRoute(const QVector<int> &verts, const QVector<int> &edgs); // выделить маршрут
    void clearRoute();                                                  // убрать выделение
    void setReach(const QVector<bool> &reachable, int start);           // какие вершины достижимы из start

signals:
    void vertexDoubleClicked(int index);    // сигнал: двойной клик по вершине (выбор)
    void vertexTripleClicked(int index);    // сигнал: тройной клик за 0.5 с (удалить/вернуть)

protected:
    void paintEvent(QPaintEvent *event) override;                // рисование
    void mousePressEvent(QMouseEvent *event) override;           // одиночный клик
    void mouseDoubleClickEvent(QMouseEvent *event) override;     // второй клик Qt присылает сюда
    void mouseMoveEvent(QMouseEvent *event) override;            // подсказки при наведении

private:
    QPointF toWidget(const QPointF &p) const;   // координаты картинки -> координаты виджета
    int hitVertex(const QPointF &pos) const;    // номер вершины под курсором или -1
    bool isUnreachable(int v) const;            // вершина недостижима из выбранной?
    void registerClick(const QPointF &pos);     // подсчёт кликов (двойной / тройной)

    const Graph *g = nullptr;       // граф (хранится в MainWindow, здесь только ссылка)
    QPixmap map;                    // картинка карты
    QVector<int> routeVertices;     // вершины маршрута
    QVector<int> routeEdges;        // рёбра маршрута

    QVector<bool> reach;            // reach[v] = достижима ли вершина v из reachStart
    int reachStart = -1;            // выбранная точка А (-1 = не выбрана)

    QElapsedTimer clock;            // часы для измерения 0.5 секунды
    QVector<qint64> clickTimes;     // времена последних кликов по одной вершине
    int clickVertex = -1;           // по какой вершине кликали
};
