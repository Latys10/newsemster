#include "BinaryTree.h"

#include <QFile>
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

// Удаление дерева: сначала потомки, потом сам узел
void BinaryTree::destroy(Node* node) {
    if (node == nullptr)
        return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

// Чтение дерева из файла. Каждая строка файла - один уровень дерева.
// Поддерживаются два формата строки:
//   1) записаны ВСЕ позиции уровня (под "-" тоже стоят "- -"):
//        1
//        - 3
//        - - 9 7
//   2) записаны дети только существующих узлов:
//        1
//        4 3
//        2 - 9 7
bool BinaryTree::loadFromFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;                                      // не смогли открыть файл

    QTextStream in(&file);
    const QStringList lines = in.readAll().split('\n');    // файл по строкам = по уровням
    file.close();

    clear();                                               // выбрасываем старое дерево

    QVector<Node*> prev;        // позиции предыдущего уровня (nullptr = узла нет)
    bool firstLine = true;

    for (const QString& line : lines) {
        const QStringList tokens =
            line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.isEmpty())
            continue;                                      // пустую строку пропускаем

        // 1. Разбираем строку: есть ли узел в позиции и какое у него значение
        QVector<bool> has;
        QVector<int>  val;
        for (const QString& tok : tokens) {
            if (tok == "-") {
                has.append(false);
                val.append(0);
            } else {
                bool ok = false;
                int v = tok.toInt(&ok);
                if (!ok) {                                 // мусор в файле
                    clear();
                    return false;
                }
                has.append(true);
                val.append(v);
            }
        }

        // 2. Первая строка - корень
        if (firstLine) {
            if (tokens.size() != 1 || !has[0]) {
                clear();
                return false;
            }
            m_root = new Node(val[0]);
            prev.clear();
            prev.append(m_root);
            firstLine = false;
            continue;
        }

        // 3. Определяем формат строки
        //    positional: записаны ВСЕ позиции уровня (2 * число позиций выше)
        //    иначе: записаны дети только существующих узлов
        const bool positional = (tokens.size() == 2 * prev.size());

        QVector<Node*> parents;                            // родители для "сжатого" формата
        if (!positional) {
            for (Node* n : prev)
                if (n) parents.append(n);
            if (tokens.size() > 2 * parents.size()) {      // элементов больше, чем может быть
                clear();
                return false;
            }
        }

        // 4. Создаём узлы текущего уровня и привязываем к родителям
        QVector<Node*> cur(tokens.size(), nullptr);
        for (int i = 0; i < tokens.size(); ++i) {
            if (!has[i])
                continue;                                  // "-": узла нет

            Node* parent = positional ? prev[i / 2] : parents[i / 2];
            if (parent == nullptr) {                       // потомок у несуществующего узла
                clear();
                return false;
            }

            Node* child = new Node(val[i]);
            if (i % 2 == 0) parent->left  = child;         // чётная позиция - левый
            else            parent->right = child;         // нечётная - правый
            cur[i] = child;
        }
        prev = cur;                                        // этот уровень станет "предыдущим"
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
    if (node == nullptr)
        return;
    out.append(node->value);          // У
    preorderRec(node->left, out);     // Л
    preorderRec(node->right, out);    // П
}

// ---------- Вариант 1, задание 2: высота дерева ----------
int BinaryTree::height() const {
    return heightRec(m_root);
}

int BinaryTree::heightRec(const Node* node) {
    if (node == nullptr)
        return 0;
    int hl = heightRec(node->left);
    int hr = heightRec(node->right);
    return 1 + qMax(hl, hr);
}
