#pragma once
#include "UniquePtr.hpp"
#include <string>
#include <climits>
#include <utility>
#include <stdexcept>
using namespace std;

template<class T>
struct TreeNode {
    T value;
    int height = 1;
    UniquePtr<TreeNode> left;
    UniquePtr<TreeNode> right;
    TreeNode* next = nullptr;
    TreeNode(const T& value) : value(value) {}
};

template<class T>
class Tree {
    using Node = TreeNode<T>;

    UniquePtr<Node> root;
    Node* firstThread = nullptr;
    void saveNode(const UniquePtr<Node>& node, string& text, const string& format) const;
    UniquePtr<Node> loadNode(const string& text, size_t& pos, const string& format, int depth);
    bool isOrdered(const UniquePtr<Node>& node, const T* lower, const T* upper) const;
    bool checkBranchBalance(UniquePtr<Node>& node);
    void threadNodes(UniquePtr<Node>& node, const string& order, Node*& previous);
    T threadNodesWay(int way);
    const UniquePtr<Node>& findNode(const T& value) const;
    UniquePtr<Node> copyBranch(const UniquePtr<Node>& node) const;
    bool insertNode(UniquePtr<Node>& node, const T& value);
    bool eraseNode(UniquePtr<Node>& node, const T& value);
    UniquePtr<Node> takeMinimum(UniquePtr<Node>& node);
    int nodeHeight(const UniquePtr<Node>& node) const;
    void updateHeight(UniquePtr<Node>& node);
    int balanceFactor(const UniquePtr<Node>& node) const;
    void rotateLeft(UniquePtr<Node>& node);
    void rotateRight(UniquePtr<Node>& node);
    void balance(UniquePtr<Node>& node);
    void collectShifted(const UniquePtr<Node>& node, int shift, Tree& result) const;
    long long sumBranch(const UniquePtr<Node>& node) const;
    void collectFrom(const UniquePtr<Node>& node, const T& minimum, Tree& result) const;

    template<class Action>
    void visitInOrder(const UniquePtr<Node>& node, Action& action) const;

public:
    Tree() = default;
    Tree(Tree&& other);
    Tree& operator=(Tree&& other);
    T operator[](int way);
    bool insert(const T& value);
    bool contains(const T& value) const;
    const T* findValue(const T& value) const;
    bool erase(const T& value);
    int height() const;
    string diagram() const;
    Tree subtree(const T& value) const;
    bool containsSubtree(const Tree& example) const;
    bool containsThreadedSubtree(const string& keys) const;
    bool thread(const string& order = "KLR");
    string save(const string& format = "KLR") const;
    bool load(const string& text, const string& format = "KLR");
    bool loadPairs(const string& text);
    T forEachThreadedWay(int way) const;
    template<class Action>
    void forEachThreaded(Action action) const;

    template<class Action>
    void forEach(Action action) const;
    struct MapOverflow {};
    Tree map(int shift) const;
    template<class Transform>
    Tree map(Transform transform) const;
    Tree where(const T& minimum) const;
    template<class Predicate>
    Tree where(Predicate predicate) const;
    long long reduce() const;
    template<class Result, class Combine>
    Result reduce(Result result, Combine combine) const;
};

template<class T>
bool Tree<T>::insertNode(UniquePtr<Node>& node,const T& value) {
    if (!node) {
        node.Reset(new Node(value));
        return true;
    }
    bool inserted;
    if (value < node->value) inserted = insertNode(node->left, value);
    else if (node->value < value) inserted = insertNode(node->right, value);
    else return false;
    if (inserted) balance(node);
    return inserted;
}

template<class T>
bool Tree<T>::insert(const T& value) {
    if (!insertNode(root, value)) return false;
    firstThread = nullptr;
    return true;
}

template<class T>
const UniquePtr<TreeNode<T>>& Tree<T>::findNode(const T& value) const {
    const UniquePtr<Node>* position = &root;
    while (*position) {
        const UniquePtr<Node>& node = *position;
        if (value < node->value) position = &node->left;
        else if (node->value < value) position = &node->right;
        else return node;
    }
    return *position;
}

template<class T>
bool Tree<T>::contains(const T& value) const {
    return bool(findNode(value));
}

template<class T>
const T* Tree<T>::findValue(const T& value) const {
    auto& node = findNode(value);
    if (!node) return nullptr;
    return &node->value;
}

template<class T>
UniquePtr<TreeNode<T>> Tree<T>::takeMinimum(UniquePtr<Node>& node) {
    if (node->left) {
        UniquePtr<Node> minimum = takeMinimum(node->left);
        balance(node);
        return minimum;
    }
    UniquePtr<Node> minimum(move(node));
    node = move(minimum->right);
    return minimum;
}

template<class T>
bool Tree<T>::eraseNode(UniquePtr<Node>& node, const T& value) {
    if (!node) return false;
    if (value < node->value) {
        if (!eraseNode(node->left, value)) return false;
    } else if (node->value < value) {
        if (!eraseNode(node->right, value)) return false;
    } else {
        UniquePtr<Node> removed(move(node));
        if (!removed->left) {
            node = move(removed->right);
        } else if (!removed->right) {
            node = move(removed->left);
        } else {
            UniquePtr<Node> replacement = takeMinimum(removed->right);
            replacement->left = move(removed->left);
            replacement->right = move(removed->right);
            node = move(replacement);
        }
    }
    if (node) balance(node);
    return true;
}

template<class T>
bool Tree<T>::erase(const T& value) {
    if (!eraseNode(root, value)) return false;
    firstThread = nullptr;
    return true;
}

template<class T>
int Tree<T>::nodeHeight(const UniquePtr<Node>& node) const {
    if (!node) return 0;
    return node->height;
}

template<class T>
int Tree<T>::height() const {
    return nodeHeight(root);
}

template<class T>
void Tree<T>::updateHeight(UniquePtr<Node>& node) {
    int left = nodeHeight(node->left);
    int right = nodeHeight(node->right);
    node->height = 1 + (left > right ? left : right);
}

template<class T>
int Tree<T>::balanceFactor(const UniquePtr<Node>& node) const {
    return nodeHeight(node->left) - nodeHeight(node->right);
}

template<class T>
void Tree<T>::rotateLeft(UniquePtr<Node>& node) {
    UniquePtr<Node> top(move(node->right));
    node->right = move(top->left);
    updateHeight(node);
    top->left = move(node);
    updateHeight(top);
    node = move(top);
}

template<class T>
void Tree<T>::rotateRight(UniquePtr<Node>& node) {
    UniquePtr<Node> top(move(node->left));
    node->left = move(top->right);
    updateHeight(node);
    top->right = move(node);
    updateHeight(top);
    node = move(top);
}

template<class T>
void Tree<T>::balance(UniquePtr<Node>& node) {
    updateHeight(node);
    int difference = balanceFactor(node);
    if (difference > 1) {
        if (balanceFactor(node->left) < 0) rotateLeft(node->left);
        rotateRight(node);
    } else if (difference < -1) {
        if (balanceFactor(node->right) > 0) rotateRight(node->right);
        rotateLeft(node);
    }
}

template<class T>
template<class Action>
void Tree<T>::visitInOrder(const UniquePtr<Node>& node, Action& action) const {
    if (!node) return;
    visitInOrder(node->left, action);
    action(node->value);
    visitInOrder(node->right, action);
}

template<class T>
template<class Action>
void Tree<T>::forEach(Action action) const {
    visitInOrder(root, action);
}

template<class T>
void Tree<T>::collectShifted(const UniquePtr<Node>& node, int shift, Tree& result) const {
    if (!node) return;
    collectShifted(node->left, shift, result);
    long long value = static_cast<long long>(node->value) + shift;
    if (value < INT_MIN || value > INT_MAX) throw MapOverflow{};
    result.insert(static_cast<int>(value));
    collectShifted(node->right, shift, result);
}

template<class T>
Tree<T> Tree<T>::map(int shift) const {
    Tree<T> result;
    collectShifted(root, shift, result);
    return result;
}

template<class T>
template<class Transform>
Tree<T> Tree<T>::map(Transform transform) const {
    Tree<T> result;
    forEach([&](const T& value) {
        result.insert(transform(value));
    });
    return result;
}

template<class T>
void Tree<T>::collectFrom(const UniquePtr<Node>& node, const T& minimum, Tree& result) const {
    if (!node) return;
    collectFrom(node->left, minimum, result);
    if (!(node->value < minimum)) result.insert(node->value);
    collectFrom(node->right, minimum, result);
}

template<class T>
Tree<T> Tree<T>::where(const T& minimum) const {
    Tree<T> result;
    collectFrom(root, minimum, result);
    return result;
}

template<class T>
template<class Predicate>
Tree<T> Tree<T>::where(Predicate predicate) const {
    Tree<T> result;
    forEach([&](const T& value) {
        if (predicate(value)) result.insert(value);
    });
    return result;
}

template<class T>
long long Tree<T>::sumBranch(const UniquePtr<Node>& node) const {
    if (!node) return 0;
    return sumBranch(node->left) + node->value + sumBranch(node->right);
}

template<class T>
long long Tree<T>::reduce() const {
    return sumBranch(root);
}

template<class T>
template<class Result, class Combine>
Result Tree<T>::reduce(Result result, Combine combine) const {
    forEach([&](const T& value) {
        result = combine(value, result);
    });
    return result;
}

template<class T>
UniquePtr<TreeNode<T>> Tree<T>::copyBranch(const UniquePtr<Node>& node) const {
    if (!node) return UniquePtr<Node>();
    UniquePtr<Node> copy(new Node(node->value));
    copy->left = copyBranch(node->left);
    copy->right = copyBranch(node->right);
    copy->height = node->height;
    return copy;
}

template<class T>
Tree<T> Tree<T>::subtree(const T& value) const {
    Tree<T> result;
    result.root = copyBranch(findNode(value));
    return result;
}

template<class T>
bool Tree<T>::containsSubtree(const Tree& example) const {
    if (!example.root) return true;

    Tree branch = subtree(example.root->value);
    if (!branch.root) return false;
    Tree pattern = example.subtree(example.root->value);

    branch.thread();
    pattern.thread();
    const Node* first = branch.firstThread;
    const Node* second = pattern.firstThread;
    while (first && second) {
        if (first->value < second->value || second->value < first->value) return false;
        if (bool(first->left) != bool(second->left)) return false;
        if (bool(first->right) != bool(second->right)) return false;
        first = first->next;
        second = second->next;
    }
    return !first && !second;
}

template<class T>
Tree<T>::Tree(Tree&& other)
    : root(move(other.root)), firstThread(other.firstThread) {
    other.firstThread = nullptr;
}

template<class T>
Tree<T>& Tree<T>::operator=(Tree&& other) {
    if (this != &other) {
        root = move(other.root);
        firstThread = other.firstThread;
        other.firstThread = nullptr;
    }
    return *this;
}

template<class T>
T Tree<T>::operator[](int way) {
    return threadNodesWay(way);

}

template<class T>
bool Tree<T>::thread(const string& order) {
    if (order.size() != 3 || order.find('K') == string::npos ||
        order.find('L') == string::npos || order.find('R') == string::npos)
        return false;

    firstThread = nullptr;
    Node* previous = nullptr;
    threadNodes(root, order, previous);
    return true;
}

template<class T>
void Tree<T>::threadNodes(UniquePtr<Node>& node, const string& order, Node*& previous) {
    if (!node) return;
    for (char step : order) {
        if (step == 'L') {
            threadNodes(node->left, order, previous);
        } else if (step == 'R') {
            threadNodes(node->right, order, previous);
        } else {
            if (previous) previous->next = &*node;
            else firstThread = &*node;
            node->next = nullptr;
            previous = &*node;
        }
    }
}

template<class T>
T Tree<T>::threadNodesWay(int way) {
   Tree<T> find;
   find.root=copyBranch(root);
   find.thread("LKR");
   return find.forEachThreadedWay(way);
}

template<class T>
T Tree<T>::forEachThreadedWay(int way) const {
    if(way<0) throw std::out_of_range("Отр. знач");
    const Node* node = firstThread;
    while (node && way>0) {
        node = node->next;
        way--;
    }
    if(!node) throw std::out_of_range("Нев. индексв");
    return node->value;
}

template<class T>
template<class Action>
void Tree<T>::forEachThreaded(Action action) const {
    const Node* node = firstThread;
    while (node) {
        action(node->value);
        node = node->next;
    }
}

struct TreeReadError {};

inline string treeValueText(int value) {
    return to_string(value);
}

inline const string& treeValueText(const string& value) {
    return value;
}

inline void readTreeValue(const string& text, int& value) {
    size_t end = 0;
    try {
        value = stoi(text, &end);
    } catch (...) {
        throw TreeReadError{};
    }
    if (end != text.size()) throw TreeReadError{};
}

inline void readTreeValue(const string& text, string& value) {
    value = text;
}

inline string readTreeText(const string& text, size_t& pos) {
    size_t length = 0;
    if (pos == text.size() || text[pos] < '0' || text[pos] > '9')
        throw TreeReadError{};
    while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9') {
        size_t digit = text[pos] - '0';
        if (digit > text.size() || length > (text.size() - digit) / 10)
            throw TreeReadError{};
        length = length * 10 + digit;
        ++pos;
    }
    if (pos == text.size() || text[pos] != ':') throw TreeReadError{};
    ++pos;
    if (length > text.size() - pos) throw TreeReadError{};
    string value = text.substr(pos, length);
    pos += length;
    return value;
}

inline string normalizeTreeFormat(const string& format) {
    string result;
    for (size_t i = 0; i < format.size(); ++i) {
        if (format.compare(i, sizeof("К") - 1, "К") == 0) {
            result += 'K';
            i += sizeof("К") - 2;
        } else if (format.compare(i, sizeof("Л") - 1, "Л") == 0) {
            result += 'L';
            i += sizeof("Л") - 2;
        } else if (format.compare(i, sizeof("П") - 1, "П") == 0) {
            result += 'R';
            i += sizeof("П") - 2;
        } else {
            result += format[i];
        }
    }
    for (char field : string("KLR")) {
        size_t first = result.find(field);
        if (first == string::npos || result.find(field, first + 1) != string::npos)
            return "";
    }
    return result;
}

template<class T>
void Tree<T>::saveNode(const UniquePtr<Node>& node, string& text, const string& format) const {
    if (!node) {
        text += '#';
        return;
    }
    if (format != "KLR") text += '+';
    for (char field : format) {
        if (field == 'K') {
            string value = treeValueText(node->value);
            text += to_string(value.size()) + ':' + value;
        } else if (field == 'L') {
            saveNode(node->left, text, format);
        } else if (field == 'R') {
            saveNode(node->right, text, format);
        } else {
            text += field;
        }
    }
}

template<class T>
string Tree<T>::save(const string& format) const {
    string normalized = normalizeTreeFormat(format);
    if (normalized.empty()) return "";
    string text;
    saveNode(root, text, normalized);
    return text;
}

template<class T>
UniquePtr<TreeNode<T>> Tree<T>::loadNode(const string& text, size_t& pos, const string& format, int depth) {
    if (pos == text.size()) throw TreeReadError{};
    if (text[pos] == '#') {
        ++pos;
        return UniquePtr<Node>();
    }
    if (depth > 128) throw TreeReadError{};
    if (format != "KLR" && text[pos++] != '+') throw TreeReadError{};

    T value{};
    UniquePtr<Node> left;
    UniquePtr<Node> right;
    for (char field : format) {
        if (field == 'K') {
            readTreeValue(readTreeText(text, pos), value);
        } else if (field == 'L') {
            left = loadNode(text, pos, format, depth + 1);
        } else if (field == 'R') {
            right = loadNode(text, pos, format, depth + 1);
        } else {
            if (pos == text.size() || text[pos++] != field) throw TreeReadError{};
        }
    }
    UniquePtr<Node> node(new Node(value));
    node->left = move(left);
    node->right = move(right);
    updateHeight(node);
    int difference = balanceFactor(node);
    if (difference < -1 || difference > 1) throw TreeReadError{};
    return node;
}

template<class T>
bool Tree<T>::isOrdered(const UniquePtr<Node>& node, const T* lower, const T* upper) const {
    if (!node) return true;
    if (lower && !(*lower < node->value)) return false;
    if (upper && !(node->value < *upper)) return false;
    return isOrdered(node->left, lower, &node->value) &&
           isOrdered(node->right, &node->value, upper);
}

template<class T>
bool Tree<T>::load(const string& text, const string& format) {
    string normalized = normalizeTreeFormat(format);
    if (normalized.empty()) return false;
    size_t pos = 0;
    try {
        UniquePtr<Node> newRoot = loadNode(text, pos, normalized, 1);
        if (pos != text.size() || !isOrdered(newRoot, nullptr, nullptr)) return false;
        root = move(newRoot);
        firstThread = nullptr;
        return true;
    } catch (const TreeReadError&) {
        return false;
    }
}

inline bool isPairSpace(char symbol) {
    return symbol == ' ' || symbol == '\t' || symbol == '\r' || symbol == '\n';
}

inline void skipPairSpaces(const string& text, size_t& pos) {
    while (pos < text.size() && isPairSpace(text[pos]))
        ++pos;
}

inline string readPairToken(const string& text, size_t& pos) {
    skipPairSpaces(text, pos);
    if (pos == text.size()) throw TreeReadError{};
    string value;
    if (text[pos] == '"') {
        ++pos;
        while (pos < text.size() && text[pos] != '"') {
            if (text[pos] == '\\') ++pos;
            if (pos == text.size()) throw TreeReadError{};
            value += text[pos++];
        }
        if (pos == text.size()) throw TreeReadError{};
        ++pos;
        if (pos < text.size() && !isPairSpace(text[pos]))
            throw TreeReadError{};
    } else {
        while (pos < text.size() && !isPairSpace(text[pos]))
            value += text[pos++];
    }
    return value;
}

template<class T>
bool Tree<T>::containsThreadedSubtree(const string& keys) const {
    size_t pos = 0;
    skipPairSpaces(keys, pos);
    if (pos == keys.size()) return true;

    try {
        T value{};
        readTreeValue(readPairToken(keys, pos), value);
        Tree branch = subtree(value);
        branch.thread();

        const Node* node = branch.firstThread;
        while (node) {
            if (node->value < value || value < node->value) return false;
            node = node->next;
            skipPairSpaces(keys, pos);
            if (pos == keys.size()) return node == nullptr;
            if (node) readTreeValue(readPairToken(keys, pos), value);
        }
    } catch (const TreeReadError&) {
        return false;
    }
    return false;
}

template<class T>
bool Tree<T>::checkBranchBalance(UniquePtr<Node>& node) {
    if (!node) return true;
    if (!checkBranchBalance(node->left)) return false;
    if (!checkBranchBalance(node->right)) return false;
    updateHeight(node);
    int difference = balanceFactor(node);
    return difference >= -1 && difference <= 1;
}

template<class T>
bool Tree<T>::loadPairs(const string& text) {
    Tree<T> result;
    size_t pos = 0;
    skipPairSpaces(text, pos);
    try {
        while (pos < text.size()) {
            if (text[pos] == '#') return false;
            T value{};
            readTreeValue(readPairToken(text, pos), value);
            skipPairSpaces(text, pos);
            if (pos == text.size()) return false;
            bool isRoot = text[pos] == '#';
            string parentText = readPairToken(text, pos);
            if (isRoot) {
                if (parentText != "#" || result.root) return false;
                result.root.Reset(new Node(value));
            } else {
                T parentValue{};
                readTreeValue(parentText, parentValue);
                Node* parent = nullptr;
                if (result.root) parent = &*result.root;
                int depth = 1;
                while (parent) {
                    if (parentValue < parent->value) parent = parent->left ? &*parent->left : nullptr;
                    else if (parent->value < parentValue) parent = parent->right ? &*parent->right : nullptr;
                    else break;
                    ++depth;
                }
                if (!parent || depth >= 128) return false;
                if (value < parent->value) {
                    if (parent->left) return false;
                    parent->left.Reset(new Node(value));
                } else if (parent->value < value) {
                    if (parent->right) return false;
                    parent->right.Reset(new Node(value));
                } else {
                    return false;
                }
            }
            skipPairSpaces(text, pos);
        }
    } catch (const TreeReadError&) {
        return false;
    }
    if (!result.isOrdered(result.root, nullptr, nullptr)) return false;
    if (!result.checkBranchBalance(result.root)) return false;
    root = move(result.root);
    firstThread = nullptr;
    return true;
}

template<class T>
string Tree<T>::diagram() const {
    if (!root) return "";

    int levels = height();
    if (levels > 4) levels = 4;
    const Node* nodes[16]{};
    string labels[16];
    nodes[1] = &*root;

    size_t cellWidth = 4;
    for (int i = 1; i < (1 << levels); ++i) {
        if (!nodes[i]) continue;
        labels[i] = treeValueText(nodes[i]->value);
        if (labels[i].empty()) labels[i] = "\"\"";
        if (labels[i].size() + 2 > cellWidth) cellWidth = labels[i].size() + 2;
        if (i < 8) {
            nodes[2 * i] = nodes[i]->left ? &*nodes[i]->left : nullptr;
            nodes[2 * i + 1] = nodes[i]->right ? &*nodes[i]->right : nullptr;
        }
    }
    while (levels > 1 && cellWidth * (1 << (levels - 1)) > 80)
        --levels;

    const size_t width = cellWidth * (1 << (levels - 1));
    string result;
    for (int level = 0; level < levels; ++level) {
        int count = 1 << level;
        size_t blockWidth = width / count;
        string values(width, ' ');
        string branches(width, ' ');

        for (int i = 0; i < count; ++i) {
            int index = count + i;
            if (!nodes[index]) continue;
            size_t start = i * blockWidth;
            size_t center = start + blockWidth / 2;
            const string& label = labels[index];
            values.replace(center - label.size() / 2, label.size(), label);

            if (level + 1 == levels) continue;
            size_t left = center;
            size_t right = center;
            if (nodes[2 * index]) left = start + blockWidth / 4;
            if (nodes[2 * index + 1]) right = start + blockWidth / 2 + blockWidth / 4;
            if (left != right) {
                branches.replace(left, right - left + 1, right - left + 1, '-');
                branches[left] = '+';
                branches[center] = '+';
                branches[right] = '+';
            }
        }
        values.resize(values.find_last_not_of(' ') + 1);
        result += values + '\n';
        size_t lastBranch = branches.find_last_not_of(' ');
        if (lastBranch != string::npos) result += branches.substr(0, lastBranch + 1) + '\n';
    }
    if (levels < height()) result += "Показано уровней: " + to_string(levels) + '\n';
    return result;
}
