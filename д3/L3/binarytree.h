#pragma once
#include <QString>
#include <QVector>

// Узел бинарного дерева
struct Node {
    int   value;   // значение, хранящееся в узле
    Node* left;    // указатель на левого потомка (nullptr, если его нет)
    Node* right;   // указатель на правого потомка (nullptr, если его нет)

    explicit Node(int v) : value(v), left(nullptr), right(nullptr) {}
};

// Класс "Бинарное дерево"
class BinaryTree {
public:
    BinaryTree();
    ~BinaryTree();                                   // деструктор: освобождает память

    // Копировать дерево нам не нужно, поэтому запрещаем (иначе два объекта
    // удалят одну и ту же память)
    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;

    bool loadFromFile(const QString& fileName);      // чтение дерева из файла (послойный формат)
    void clear();                                    // удалить все узлы
    bool isEmpty() const;                            // дерево пустое?

    QVector<int> preorder() const;                   // префиксный обход УЛП
    int          height() const;                     // высота дерева

    const Node*  root() const { return m_root; }     // нужен виджету для рисования

private:
    Node* m_root;                                    // указатель на корень

    // Рекурсивные вспомогательные функции (static - им не нужен объект this)
    static void destroy(Node* node);
    static void preorderRec(const Node* node, QVector<int>& out);
    static int  heightRec(const Node* node);
};
