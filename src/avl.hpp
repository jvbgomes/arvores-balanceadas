// Árvore AVL com algoritmo de balanceamento escrito do zero
// Altura armazenada em cada nó; rebalanceamento na volta da recursão
// Remoção usa cópia da chave do sucessor em ordem
#pragma once
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <utility>
#include <algorithm>
#include <climits>
#include <iostream>

struct AVLNode {
    int key;
    int height;    // altura deste nó (folha=1, vazio=0)
    AVLNode* left;
    AVLNode* right;
    explicit AVLNode(int k) : key(k), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
public:
    AVLNode* root;
    int _size;

    // Estatísticas
    int rotations_;
    int cases_LL, cases_RR, cases_LR, cases_RL;

    AVLTree() : root(nullptr), _size(0),
        rotations_(0), cases_LL(0), cases_RR(0), cases_LR(0), cases_RL(0) {}

    // Destrutor iterativo (BFS)
    ~AVLTree() {
        std::queue<AVLNode*> q;
        if (root) q.push(root);
        while (!q.empty()) {
            AVLNode* n = q.front(); q.pop();
            if (n->left)  q.push(n->left);
            if (n->right) q.push(n->right);
            delete n;
        }
    }

    // --- Interface pública ---

    bool insert(int key) {
        bool inserted = false;
        root = insertRec(root, key, inserted);
        if (inserted) _size++;
        return inserted;
    }

    bool remove(int key) {
        bool removed = false;
        root = removeRec(root, key, removed);
        if (removed) _size--;
        return removed;
    }

    bool search(int key) const {
        AVLNode* curr = root;
        while (curr) {
            if (key == curr->key) return true;
            curr = (key < curr->key) ? curr->left : curr->right;
        }
        return false;
    }

    // Altura da árvore (raiz vazia = 0)
    int height() const { return nodeHeight(root); }

    int size() const { return _size; }

    // Profundidade média com raiz em profundidade 1
    double averageDepth() const {
        if (!root) return 0.0;
        std::stack<std::pair<AVLNode*, int>> s;
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

    // Percurso em ordem iterativo
    std::vector<int> inorder() const {
        std::vector<int> result;
        std::stack<AVLNode*> s;
        AVLNode* curr = root;
        while (curr || !s.empty()) {
            while (curr) { s.push(curr); curr = curr->left; }
            curr = s.top(); s.pop();
            result.push_back(curr->key);
            curr = curr->right;
        }
        return result;
    }

    // Zera estatísticas de rotações/casos
    void resetStats() {
        rotations_ = 0;
        cases_LL = cases_RR = cases_LR = cases_RL = 0;
    }

    // Valida BST, alturas armazenadas, fatores de balanceamento e tamanho
    std::pair<bool, std::vector<std::string>> validate() const {
        std::vector<std::string> erros;

        // Percurso em ordem para verificar BST e propriedades de cada nó
        struct Frame { AVLNode* node; };
        std::stack<AVLNode*> s;
        AVLNode* curr = root;
        int prev_key = INT_MIN;
        int contados = 0;

        while (curr || !s.empty()) {
            while (curr) { s.push(curr); curr = curr->left; }
            curr = s.top(); s.pop();

            // Propriedade BST
            if (curr->key <= prev_key)
                erros.push_back("Violação da propriedade BST: " + std::to_string(prev_key) + " >= " + std::to_string(curr->key));
            prev_key = curr->key;

            // Altura armazenada correta
            int calc = 1 + std::max(nodeHeight(curr->left), nodeHeight(curr->right));
            if (curr->height != calc)
                erros.push_back("Altura incorreta no nó " + std::to_string(curr->key) +
                    ": armazenada=" + std::to_string(curr->height) + " calculada=" + std::to_string(calc));

            // Fator de balanceamento
            int bf = nodeHeight(curr->left) - nodeHeight(curr->right);
            if (bf < -1 || bf > 1)
                erros.push_back("Fator de balanceamento inválido no nó " + std::to_string(curr->key) +
                    ": fb=" + std::to_string(bf));

            contados++;
            curr = curr->right;
        }

        if (contados != _size)
            erros.push_back("Tamanho incorreto: armazenado=" + std::to_string(_size) +
                " calculado=" + std::to_string(contados));

        return {erros.empty(), erros};
    }

    // Impressão da árvore no estilo prefixo
    void printTree(std::ostream& out = std::cout) const { // NOLINT
        if (!root) { out << "(árvore vazia)\n"; return; }
        if (_size > 80) {
            out << "A árvore tem " << _size << " elementos, muito grande para imprimir. Mostrando só as métricas.\n";
            return;
        }
        out << root->key << " [fb=" << (nodeHeight(root->left) - nodeHeight(root->right)) << "]\n";

        struct Frame {
            AVLNode* node;
            std::string prefix;
            char dir;
            bool isLast;
        };
        std::stack<Frame> stk;

        auto pushChildren = [&](AVLNode* node, const std::string& prefix) {
            bool temEsq = node->left  != nullptr;
            bool temDir = node->right != nullptr;
            if (temEsq && temDir) {
                stk.push({node->right, prefix, 'D', true});
                stk.push({node->left,  prefix, 'E', false});
            } else if (temEsq) {
                stk.push({node->left,  prefix, 'E', true});
            } else if (temDir) {
                stk.push({node->right, prefix, 'D', true});
            }
        };

        pushChildren(root, "");

        while (!stk.empty()) {
            auto [node, prefix, dir, isLast] = stk.top(); stk.pop();
            std::string conn = isLast ? "└─" : "├─";
            int bf = nodeHeight(node->left) - nodeHeight(node->right);
            out << prefix << conn << dir << " " << node->key << " [fb=" << bf << "]\n";
            std::string childPrefix = prefix + (isLast ? "   " : "│  ");
            pushChildren(node, childPrefix);
        }
        out << "Legenda: fb = fator de balanceamento, E = esquerda, D = direita\n";
    }

private:
    // Altura de um nó (nullptr = 0)
    static int nodeHeight(AVLNode* n) { return n ? n->height : 0; }

    // Atualiza altura com base nos filhos
    static void updateHeight(AVLNode* n) {
        n->height = 1 + std::max(nodeHeight(n->left), nodeHeight(n->right));
    }

    // Fator de balanceamento: h(esq) - h(dir)
    static int bf(AVLNode* n) {
        return n ? nodeHeight(n->left) - nodeHeight(n->right) : 0;
    }

    // Rotação à direita: devolve nova raiz da subárvore
    AVLNode* rotateRight(AVLNode* y) {
        rotations_++;
        AVLNode* x  = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left  = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    // Rotação à esquerda: devolve nova raiz da subárvore
    AVLNode* rotateLeft(AVLNode* x) {
        rotations_++;
        AVLNode* y  = x->right;
        AVLNode* T2 = y->left;
        y->left  = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Rebalanceia um nó (chamado na volta da recursão)
    // Atualiza a altura e aplica rotações conforme especificação
    AVLNode* balance(AVLNode* node) {
        updateHeight(node);
        int b = bf(node);

        if (b > 1) {
            // Subárvore esquerda mais alta
            if (bf(node->left) < 0) {
                // Caso LR: filho esquerdo pende para direita
                cases_LR++;
                node->left = rotateLeft(node->left);
            } else {
                // Caso LL
                cases_LL++;
            }
            return rotateRight(node);
        }

        if (b < -1) {
            // Subárvore direita mais alta
            if (bf(node->right) > 0) {
                // Caso RL: filho direito pende para esquerda
                cases_RL++;
                node->right = rotateRight(node->right);
            } else {
                // Caso RR
                cases_RR++;
            }
            return rotateLeft(node);
        }

        return node;
    }

    // Mínimo de uma subárvore (iterativo)
    static AVLNode* findMin(AVLNode* node) {
        while (node->left) node = node->left;
        return node;
    }

    // Inserção recursiva; sets inserted=true se chave nova
    AVLNode* insertRec(AVLNode* node, int key, bool& inserted) {
        if (!node) { inserted = true; return new AVLNode(key); }
        if (key < node->key)
            node->left  = insertRec(node->left,  key, inserted);
        else if (key > node->key)
            node->right = insertRec(node->right, key, inserted);
        else
            return node; // chave duplicada, não insere

        return balance(node);
    }

    // Remoção recursiva; sets removed=true se chave encontrada
    // Nó com dois filhos: copia chave do sucessor em ordem e remove o sucessor
    AVLNode* removeRec(AVLNode* node, int key, bool& removed) {
        if (!node) return nullptr;

        if (key < node->key) {
            node->left  = removeRec(node->left,  key, removed);
        } else if (key > node->key) {
            node->right = removeRec(node->right, key, removed);
        } else {
            removed = true;
            if (!node->left || !node->right) {
                // Nó com zero ou um filho
                AVLNode* filho = node->left ? node->left : node->right;
                delete node;
                return filho;
            } else {
                // Nó com dois filhos: substitui pela chave do sucessor em ordem
                AVLNode* suc = findMin(node->right);
                node->key    = suc->key;
                // Remove o sucessor da subárvore direita
                bool dummy = false;
                node->right  = removeRec(node->right, suc->key, dummy);
            }
        }

        return balance(node);
    }
};
