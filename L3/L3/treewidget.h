#pragma once
#include <QHash>
#include <QPoint>
#include <QWidget>

#include "BinaryTree.h"

class TreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget* parent = nullptr);
    void setTree(const BinaryTree* tree);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const BinaryTree* m_tree;

    static void layoutNodes(const Node* node, int depth, int& counter,
                            QHash<const Node*, QPoint>& grid);
};
