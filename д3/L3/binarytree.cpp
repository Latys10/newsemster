#include "BinaryTree.h"

#include <QFile>
#include <QQueue>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

BinaryTree::BinaryTree() : m_root(nullptr) {}

BinaryTree::~BinaryTree() {
    clear();
}

bool BinaryTree::isEmpty() const {
    return m_root == nullptr;
}

void BinaryTree::clear() {
    destroy(m_root);
    m_root = nullptr;
}

// Удаление дерева: сначала удаляем потомков, потом сам узел (ЛПУ-обход)
void BinaryTree::destroy(Node* node) {
    if (node == nullptr)          // база рекурсии
        return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

bool BinaryTree::loadFromFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;                                   // не смогли открыть файл

    QTextStream in(&file);
    // Читаем весь текст и режем по пробелам/переводам строк на "слова" (токены)
    QStringList tokens = in.readAll().split(QRegularExpression("\\s+"),
                                            Qt::SkipEmptyParts);
    file.close();

    clear();                                            // выбрасываем старое дерево
    if (tokens.isEmpty())
        return true;                                    // пустой файл = пустое дерево

    bool ok = false;
    int rootValue = tokens[0].toInt(&ok);
    if (!ok)
        return false;                                   // первый токен - не число

    m_root = new Node(rootValue);

    QQueue<Node*> queue;                                // очередь узлов, у которых ещё
    queue.enqueue(m_root);                              // не назначены потомки
    int pos = 1;                                        // индекс следующего токена

    while (!queue.isEmpty() && pos < tokens.size()) {
        Node* parent = queue.dequeue();                 // берём родителя

        for (int side = 0; side < 2; ++side) {          // side 0 - левый, 1 - правый
            if (pos >= tokens.size())
                break;                                  // токены кончились

            QString tok = tokens[pos++];
            if (tok == "-")
                continue;                               // потомка нет

            int value = tok.toInt(&ok);
            if (!ok) {                                  // мусор в файле
                clear();
                return false;
            }

            Node* child = new Node(value);
            if (side == 0) parent->left  = child;
            else           parent->right = child;

            queue.enqueue(child);                       // у ребёнка потомки будут позже
        }
    }
    return true;
}

// ---------- Вариант 1, задание 1: префиксный обход УЛП ----------
QVector<int> BinaryTree::preorder() const {
    QVector<int> result;
    preorderRec(m_root, result);
    return result;
}

void BinaryTree::preorderRec(const Node* node, QVector<int>& out) {
    if (node == nullptr)                // пустое поддерево - нечего обходить
        return;
    out.append(node->value);            // У - узел
    preorderRec(node->left, out);       // Л - левое поддерево
    preorderRec(node->right, out);      // П - правое поддерево
}

// ---------- Вариант 1, задание 2: высота дерева ----------
int BinaryTree::height() const {
    return heightRec(m_root);
}

int BinaryTree::heightRec(const Node* node) {
    if (node == nullptr)                // высота пустого дерева = 0
        return 0;
    int hl = heightRec(node->left);     // высота левого поддерева
    int hr = heightRec(node->right);    // высота правого поддерева
    return 1 + qMax(hl, hr);            // +1 за сам узел
}
