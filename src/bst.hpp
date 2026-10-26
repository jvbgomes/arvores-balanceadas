// Árvore Binária de Busca simples (sem balanceamento)
// Todas as operações são iterativas para suportar árvores degeneradas
// Usada apenas como referência no benchmark
#pragma once
#include <vector>
#include <stack>
#include <queue>
#include <utility>
#include <algorithm>

struct BSTNode {
    int key;
    BSTNode* left;
    BSTNode* right;
    explicit BSTNode(int k) : key(k), left(nullptr), right(nullptr) {}
};

class BST {
public:
    BSTNode* root;
    int _size;

    BST() : root(nullptr), _size(0) {}

    // Destrutor iterativo (evita stack overflow em árvores degeneradas)
    ~BST() {
        std::queue<BSTNode*> q;
        if (root) q.push(root);
        while (!q.empty()) {
            BSTNode* n = q.front(); q.pop();
            if (n->left)  q.push(n->left);
            if (n->right) q.push(n->right);
            delete n;
        }
    }

    bool insert(int key) {
        if (!root) {
            root = new BSTNode(key);
            _size++;
            return true;
        }
        BSTNode* curr = root;
        while (true) {
            if (key == curr->key) return false;
            if (key < curr->key) {
                if (!curr->left) { curr->left = new BSTNode(key); _size++; return true; }
                curr = curr->left;
            } else {
                if (!curr->right) { curr->right = new BSTNode(key); _size++; return true; }
                curr = curr->right;
            }
        }
    }

    bool search(int key) const {
        BSTNode* curr = root;
        while (curr) {
            if (key == curr->key) return true;
            curr = (key < curr->key) ? curr->left : curr->right;
        }
        return false;
    }

    // Altura iterativa com pilha
    int height() const {
        if (!root) return 0;
        std::stack<std::pair<BSTNode*, int>> s;
        s.push({root, 1});
        int maxH = 0;
        while (!s.empty()) {
            auto [node, h] = s.top(); s.pop();
            if (h > maxH) maxH = h;
            if (node->left)  s.push({node->left,  h + 1});
            if (node->right) s.push({node->right, h + 1});
        }
        return maxH;
    }

    // Profundidade média com raiz em profundidade 1
    double averageDepth() const {
        if (!root) return 0.0;
        std::stack<std::pair<BSTNode*, int>> s;
        s.push({root, 1});
        long long soma = 0;
        int cont = 0;
        while (!s.empty()) {
            auto [node, d] = s.top(); s.pop();
            soma += d;
            cont++;
            if (node->left)  s.push({node->left,  d + 1});
            if (node->right) s.push({node->right, d + 1});
        }
        return (double)soma / cont;
    }

    // Percurso em ordem iterativo (evita stack overflow)
    std::vector<int> inorder() const {
        std::vector<int> result;
        std::stack<BSTNode*> s;
        BSTNode* curr = root;
        while (curr || !s.empty()) {
            while (curr) { s.push(curr); curr = curr->left; }
            curr = s.top(); s.pop();
            result.push_back(curr->key);
            curr = curr->right;
        }
        return result;
    }

    int size() const { return _size; }
};
