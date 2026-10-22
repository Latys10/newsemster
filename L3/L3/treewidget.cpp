#include "treewidget.h"

#include <QPainter>

TreeWidget::TreeWidget(QWidget* parent)
    : QWidget(parent), m_tree(nullptr)
{
    setMinimumSize(400, 300);
}

void TreeWidget::setTree(const BinaryTree* tree) {
    m_tree = tree;
    update();
}

void TreeWidget::layoutNodes(const Node* node, int depth, int& counter,
                             QHash<const Node*, QPoint>& grid)
{
    if (node == nullptr)
        return;
    layoutNodes(node->left, depth + 1, counter, grid);
    grid[node] = QPoint(counter++, depth);
    layoutNodes(node->right, depth + 1, counter, grid);
}

void TreeWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);
    p.setPen(Qt::black);

    if (m_tree == nullptr || m_tree->isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, "Дерево пусто. Загрузите его из файла.");
        return;
    }

    QHash<const Node*, QPoint> grid;
    int counter = 0;
    layoutNodes(m_tree->root(), 0, counter, grid);

    const int levels = m_tree->height();
    const int R = 20;
    const int margin = R + 10;

    double stepX = counter > 1 ? double(width()  - 2 * margin) / (counter - 1) : 0;
    double stepY = levels  > 1 ? double(height() - 2 * margin) / (levels  - 1) : 0;

    auto toPixel = [&](const QPoint& g) {
        double x = counter > 1 ? margin + g.x() * stepX : width()  / 2.0;
        double y = levels  > 1 ? margin + g.y() * stepY : height() / 2.0;
        return QPointF(x, y);
    };

    //линии
    for (auto it = grid.constBegin(); it != grid.constEnd(); ++it) {
        const Node* node = it.key();
        QPointF from = toPixel(it.value());
        if (node->left)
            p.drawLine(from, toPixel(grid[node->left]));
        if (node->right)
            p.drawLine(from, toPixel(grid[node->right]));
    }

    //кружки с числами
    p.setBrush(Qt::white);
    for (auto it = grid.constBegin(); it != grid.constEnd(); ++it) {
        QPointF c = toPixel(it.value());
        p.drawEllipse(c, R, R);
        p.drawText(QRectF(c.x() - R, c.y() - R, 2 * R, 2 * R),
                   Qt::AlignCenter, QString::number(it.key()->value));
    }
}
