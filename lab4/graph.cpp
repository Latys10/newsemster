#include "graph.h"
#include <QLineF>
#include <QRandomGenerator>
#include <limits>

int Graph::addVertex(const QString &name, const QPointF &pos)
{
    vertices.append(Vertex{name, pos, false});   // кладём вершину в массив
    adj.append(QVector<int>());                  // и создаём ей пустой список соседей
    return vertices.size() - 1;                  // номер новой вершины
}

void Graph::addEdge(int a, int b, bool oneWay)
{
    // длина = расстояние на картинке * 0.005 (условно 400 px ≈ 2 км)
    double len = QLineF(vertices[a].pos, vertices[b].pos).length() * 0.005;
    edges.append({a, b, len, 0, oneWay});   // по умолчанию дорога свободна (зелёная)
    int id = edges.size() - 1;              // номер этого ребра
    adj[a].append(id);                      // ребро выходит из a
    if (!oneWay)
        adj[b].append(id);                  // обычная дорога: выходит и из b (в обе стороны)
}

void Graph::setRemoved(int v, bool removed)
{
    vertices[v].removed = removed;
}

double Graph::coefficient(int level)
{
    switch (level) {
    case 0: return 0.8;    // зелёный
    case 1: return 1.2;    // оранжевый
    case 2: return 1.9;    // красный
    default: return 3.3;   // фиолетовый
    }
}

double Graph::weight(int edgeIndex) const
{
    const Edge &e = edges[edgeIndex];
    return e.length * coefficient(e.level);   // X * (X*k) из задания
}

void Graph::randomizeCongestion()
{
    for (Edge &e : edges) {
        int r = QRandomGenerator::global()->bounded(100);  // число 0..99
        if (r < 20)      e.level = 0;   // 20% свободно (зелёный)
        else if (r < 50) e.level = 1;   // 30% оранжевый
        else if (r < 80) e.level = 2;   // 30% красный
        else             e.level = 3;   // 20% фиолетовый (пробка)
    }
}

// Обход в глубину по списку смежности: какие вершины можно достичь из start
QVector<bool> Graph::reachableFrom(int start) const
{
    QVector<bool> seen(vertices.size(), false);
    if (start < 0 || start >= vertices.size() || vertices[start].removed)
        return seen;                                  // старта нет, достижимых нет

    QVector<int> stack;                               // стек вершин, которые ещё надо обойти
    stack.append(start);
    seen[start] = true;
    while (!stack.isEmpty()) {
        int v = stack.takeLast();
        for (int id : adj[v]) {                       // только рёбра, ВЫХОДЯЩИЕ из v
            int u = (edges[id].a == v) ? edges[id].b : edges[id].a;
            if (!seen[u] && !vertices[u].removed) {   // удалённые вершины пропускаем
                seen[u] = true;
                stack.append(u);
            }
        }
    }
    return seen;
}

bool Graph::shortestPath(int from, int to, QVector<int> &pathVertices,
                         QVector<int> &pathEdges, double &total) const
{
    if (vertices[from].removed || vertices[to].removed) return false;   // удалённые не участвуют

    const double INF = std::numeric_limits<double>::infinity();
    int n = vertices.size();
    QVector<double> dist(n, INF);     // лучшее известное расстояние до каждой вершины
    QVector<int> prevV(n, -1);        // из какой вершины пришли
    QVector<int> prevE(n, -1);        // по какому ребру пришли
    QVector<bool> done(n, false);     // обработана ли вершина
    dist[from] = 0;                   // до старта расстояние 0

    for (int iter = 0; iter < n; ++iter) {
        int v = -1;                                   // ищем необработанную вершину с минимальным dist
        for (int i = 0; i < n; ++i)
            if (!done[i] && (v == -1 || dist[i] < dist[v]))
                v = i;
        if (v == -1 || dist[v] == INF) break;         // дальше все недостижимы
        done[v] = true;
        if (v == to) break;                           // до цели дошли, можно остановиться

        for (int id : adj[v]) {                       // рёбра, выходящие из v (с учётом направления)
            int u = (edges[id].a == v) ? edges[id].b : edges[id].a;   // вершина на другом конце
            if (vertices[u].removed) continue;        // в удалённую вершину не едем
            double nd = dist[v] + weight(id);         // расстояние до u через v
            if (nd < dist[u]) {                       // нашли путь короче
                dist[u] = nd;
                prevV[u] = v;
                prevE[u] = id;
            }
        }
    }

    if (dist[to] == INF) return false;                // пути нет

    pathVertices.clear();
    pathEdges.clear();
    for (int v = to; v != -1; v = prevV[v]) {         // идём от цели назад к старту
        pathVertices.prepend(v);
        if (prevE[v] != -1) pathEdges.prepend(prevE[v]);
    }
    total = dist[to];
    return true;
}
