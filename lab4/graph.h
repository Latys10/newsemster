#pragma once                    // защита от повторного включения заголовка
#include <QString>
#include <QPointF>
#include <QVector>

// Вершина: имя, позиция на картинке и признак "удалена"
struct Vertex {
    QString name;
    QPointF pos;
    bool removed = false;   // true = вершина "удалена" тройным кликом, в алгоритмах не участвует
};

// Ребро: между вершинами a и b, постоянная длина, уровень загруженности 0..3
struct Edge {
    int a;
    int b;
    double length;   // ПОСТОЯННАЯ длина (км), считается один раз
    int level;       // ПЕРЕМЕННЫЙ уровень: 0 зелёный, 1 оранжевый, 2 красный, 3 фиолетовый
    bool oneWay;     // true = ориентированное ребро, движение только a -> b
};

class Graph {
public:
    int addVertex(const QString &name, const QPointF &pos);  // добавить вершину, вернуть её номер
    void addEdge(int a, int b, bool oneWay = false);         // добавить ребро (длина считается сама)

    int vertexCount() const { return vertices.size(); }
    const Vertex &vertex(int i) const { return vertices[i]; }
    int edgeCount() const { return edges.size(); }
    const Edge &edge(int i) const { return edges[i]; }

    void setRemoved(int v, bool removed);   // "удалить" или вернуть вершину

    static double coefficient(int level);   // коэффициент по уровню (0.8 / 1.2 / 1.9 / 3.3)
    double weight(int edgeIndex) const;     // вес = длина * коэффициент

    void randomizeCongestion();             // имитация загруженности: новые случайные уровни

    // Какие вершины достижимы из start (с учётом направления рёбер и удалённых вершин)
    QVector<bool> reachableFrom(int start) const;

    // Дейкстра. Возвращает false, если пути нет.
    bool shortestPath(int from, int to,
                      QVector<int> &pathVertices,   // вершины маршрута по порядку
                      QVector<int> &pathEdges,      // рёбра маршрута по порядку
                      double &total) const;         // суммарный вес маршрута

private:
    QVector<Vertex> vertices;          // все вершины
    QVector<Edge> edges;               // все рёбра (каждое хранится ОДИН раз)
    QVector<QVector<int>> adj;         // СПИСОК СМЕЖНОСТИ: adj[v] = номера рёбер, ВЫХОДЯЩИХ из v
};
