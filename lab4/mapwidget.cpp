#include "mapwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLineF>
#include <QToolTip>
#include <QtMath>
#include <QPolygonF>
#include <QFontMetrics>
#include <QDebug>

// Размер картинки, в пикселях которой заданы координаты вершин
static const double BASE_W = 762.0;
static const double BASE_H = 512.0;

MapWidget::MapWidget(QWidget *parent) : QWidget(parent), map(":/map.png")
{
    if (map.isNull())
        qDebug() << "Карта НЕ загрузилась: проверьте map.qrc, префикс / и имя файла";
    setMouseTracking(true);     // mouseMoveEvent приходит и без зажатой кнопки (для подсказок)
    clock.start();              // запускаем часы для подсчёта кликов
}

void MapWidget::setGraph(const Graph *graph) { g = graph; update(); }

void MapWidget::setRoute(const QVector<int> &verts, const QVector<int> &edgs)
{
    routeVertices = verts;
    routeEdges = edgs;
    update();                       // просим перерисовать виджет
}

void MapWidget::clearRoute() { routeVertices.clear(); routeEdges.clear(); update(); }

void MapWidget::setReach(const QVector<bool> &reachable, int start)
{
    reach = reachable;
    reachStart = start;
    update();
}

QPointF MapWidget::toWidget(const QPointF &p) const
{
    // масштабируем, чтобы граф совпадал с картой при любом размере виджета
    return QPointF(p.x() * width() / BASE_W, p.y() * height() / BASE_H);
}

int MapWidget::hitVertex(const QPointF &pos) const
{
    if (!g) return -1;
    for (int i = 0; i < g->vertexCount(); ++i)
        if (QLineF(toWidget(g->vertex(i).pos), pos).length() <= 12)   // попали в кружок (радиус 12 px)
            return i;
    return -1;
}

bool MapWidget::isUnreachable(int v) const
{
    // красной считается живая вершина, до которой нельзя добраться из выбранной А
    return g && reachStart >= 0 && v < reach.size()
           && !g->vertex(v).removed && !reach[v];
}

void MapWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);     // сглаживание линий
    p.drawPixmap(rect(), map);                   // 1) фон: карта на весь виджет
    if (!g) return;

    // цвета уровней загруженности: зелёный, оранжевый, красный, фиолетовый
    static const QColor colors[4] = { QColor(0, 200, 0), QColor(255, 150, 0),
                                     QColor(230, 0, 0), QColor(160, 0, 255) };

    // 2) рёбра своим цветом (цвет зависит от level)
    for (int i = 0; i < g->edgeCount(); ++i) {
        const Edge &e = g->edge(i);
        bool dead = g->vertex(e.a).removed || g->vertex(e.b).removed;   // ребро к удалённой вершине
        if (dead) p.setPen(QPen(QColor(120, 120, 120, 170), 2, Qt::DashLine));
        else      p.setPen(QPen(colors[e.level], 4));
        p.drawLine(toWidget(g->vertex(e.a).pos), toWidget(g->vertex(e.b).pos));
    }

    // 3) маршрут поверх: синяя толстая линия
    p.setPen(QPen(QColor(40, 60, 220), 7));
    for (int id : routeEdges) {
        const Edge &e = g->edge(id);
        p.drawLine(toWidget(g->vertex(e.a).pos), toWidget(g->vertex(e.b).pos));
    }

    // 4) стрелки у ориентированных рёбер и подписи длины над рёбрами
    QFont lf = font();
    lf.setPointSize(8);
    lf.setBold(true);
    QFontMetrics fm(lf);
    for (int i = 0; i < g->edgeCount(); ++i) {
        const Edge &e = g->edge(i);
        if (g->vertex(e.a).removed || g->vertex(e.b).removed) continue;   // у "мёртвых" рёбер подписей нет
        QPointF A = toWidget(g->vertex(e.a).pos);
        QPointF B = toWidget(g->vertex(e.b).pos);
        double len = QLineF(A, B).length();
        if (len < 1) continue;

        if (e.oneWay) {                                    // стрелка a -> b возле вершины b
            QPointF dir = (B - A) / len;                   // единичный вектор вдоль ребра
            QPointF n(-dir.y(), dir.x());                  // перпендикуляр к нему
            QPointF tip = B - dir * 12;                    // остриё стрелки (чуть дальше кружка)
            QPolygonF arrow;
            arrow << tip << (tip - dir * 12 + n * 6) << (tip - dir * 12 - n * 6);
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            p.drawPolygon(arrow);
        }

        // подпись в виде X(X*k): длина и вес. Рисуем повёрнутой вдоль линии, над ней
        QString txt = QString("%1(%2)").arg(e.length, 0, 'f', 1).arg(g->weight(i), 0, 'f', 1);
        txt.replace('.', ',');
        double ang = qRadiansToDegrees(qAtan2(B.y() - A.y(), B.x() - A.x()));
        if (ang > 90) ang -= 180;                          // чтобы текст не был вверх ногами
        else if (ang < -90) ang += 180;

        p.save();
        p.translate((A + B) / 2);                          // начало координат в середину ребра
        p.rotate(ang);                                     // поворачиваем вдоль ребра
        p.setFont(lf);
        int tw = fm.horizontalAdvance(txt) + 6;
        QRectF r(-tw / 2.0, -fm.height() - 3, tw, fm.height());   // прямоугольник НАД линией
        p.setPen(Qt::NoPen);
        p.fillRect(r, QColor(255, 255, 255, 215));         // светлая подложка для читаемости
        p.setPen(Qt::black);
        p.drawText(r, Qt::AlignCenter, txt);
        p.restore();
    }

    // 5) вершины: кружки с именами
    QFont f = font();
    f.setBold(true);
    f.setPointSize(11);
    p.setFont(f);
    for (int i = 0; i < g->vertexCount(); ++i) {
        const Vertex &v = g->vertex(i);
        QPointF c = toWidget(v.pos);

        QColor fill(200, 200, 200);                        // обычная вершина: серая
        if (v.removed)                      fill = Qt::black;               // удалённая: чёрная
        else if (routeVertices.contains(i)) fill = QColor(120, 140, 255);   // на маршруте: голубая
        else if (i == reachStart)           fill = QColor(255, 235, 120);   // выбранная А: жёлтая
        else if (isUnreachable(i))          fill = QColor(255, 90, 90);     // недостижимая: красная

        p.setPen(QPen(Qt::black, 2));
        p.setBrush(fill);
        p.drawEllipse(c, 9, 9);
        p.setPen(v.removed ? Qt::white : Qt::black);
        p.drawText(QRectF(c.x() - 9, c.y() - 9, 18, 18), Qt::AlignCenter, v.name);

        if (v.removed) {                                   // зачёркиваем удалённую вершину
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(c + QPointF(-6, -6), c + QPointF(6, 6));
            p.drawLine(c + QPointF(-6, 6), c + QPointF(6, -6));
        }
    }
}

// Общий подсчёт кликов. Вызывается и из mousePressEvent, и из mouseDoubleClickEvent
void MapWidget::registerClick(const QPointF &pos)
{
    int v = hitVertex(pos);
    if (v < 0) {                                  // кликнули мимо вершин: сбрасываем счётчик
        clickTimes.clear();
        clickVertex = -1;
        return;
    }
    qint64 now = clock.elapsed();                 // сколько мс прошло с запуска часов
    if (v != clickVertex) {                       // другая вершина: считаем заново
        clickTimes.clear();
        clickVertex = v;
    }
    clickTimes.append(now);
    while (!clickTimes.isEmpty() && now - clickTimes.first() > 500)
        clickTimes.removeFirst();                 // забываем клики старше 0.5 секунды

    if (clickTimes.size() == 2) {
        emit vertexDoubleClicked(v);              // 2 клика: выбрать вершину
    } else if (clickTimes.size() >= 3) {
        clickTimes.clear();
        clickVertex = -1;
        emit vertexTripleClicked(v);              // 3 клика за 0.5 с: удалить / вернуть вершину
    }
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) registerClick(event->position());
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Qt присылает второй клик серии как "double click" вместо обычного press
    if (event->button() == Qt::LeftButton) registerClick(event->position());
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int v = hitVertex(event->position());
    if (v >= 0 && g->vertex(v).removed)
        QToolTip::showText(event->globalPosition().toPoint(), "Удалена (3 клика - вернуть)", this);
    else if (v >= 0 && isUnreachable(v))
        QToolTip::showText(event->globalPosition().toPoint(), "Недостижимо", this);
    else
        QToolTip::hideText();
}
