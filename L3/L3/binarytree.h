#pragma once
#include <QString>
#include <QVector>

// Узел бинарного дерева
struct Node {
    int   value;
    Node* left;
    Node* right;

    explicit Node(int v) : value(v), left(nullptr), right(nullptr) {}
};

// Класс "Бинарное дерево"
class BinaryTree {
public:
    BinaryTree();
    ~BinaryTree();
//запрет копирования
    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;

    bool loadFromFile(const QString& fileName);      // чтение дерева
    void clear();                                    // удалить все узлы
    bool isEmpty() const;                            // дерево пустое

    QVector<int> preorder() const;                   // префиксный обход УЛП
    int          height() const;                     // высота дерева

    const Node*  root() const { return m_root; }

private:
    Node* m_root;                                    // указатель на корень

    // Рекурсивные вспомогательные функции
    static void destroy(Node* node);
    static void preorderRec(const Node* node, QVector<int>& out);
    static int  heightRec(const Node* node);
};
