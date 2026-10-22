#pragma once
#include <QHash>
#include <QPoint>
#include <QWidget>

#include "BinaryTree.h"

// Виджет, который умеет рисовать бинарное дерево
class TreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget* parent = nullptr);
    void setTree(const BinaryTree* tree);      // какое дерево рисовать

protected:
    void paintEvent(QPaintEvent* event) override;   // Qt вызывает сам, когда нужно перерисовать

private:
    const BinaryTree* m_tree;

    // Расставляет узлы по сетке: x - номер при инфиксном обходе, y - глубина
    static void layoutNodes(const Node* node, int depth, int& counter,
                            QHash<const Node*, QPoint>& grid);
};
