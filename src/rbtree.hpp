// Árvore Rubro-Negra seguindo o algoritmo CLRS com nó sentinela NIL preto
// insertFixup e deleteFixup exatamente conforme CLRS 4ª edição
// paint(nó, cor) conta recolorações; y.color = z.color na remoção não conta
#pragma once
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <utility>
#include <climits>
#include <iostream>

enum class Cor { VERMELHO, PRETO };

struct RBNode {
    int   key;
    Cor   color;
    RBNode* left;
    RBNode* right;
    RBNode* parent;

    RBNode(int k, Cor c, RBNode* nil)
        : key(k), color(c), left(nil), right(nil), parent(nil) {}
};

class RBTree {
public:
    RBNode* nil;   // sentinela preta
    RBNode* root;
    int _size;

    // Estatísticas
    int rotations_;
    int recolorings_;

    RBTree() {
        // Cria sentinela NIL preta
        nil = new RBNode(0, Cor::PRETO, nullptr);
        nil->left = nil->right = nil->parent = nil;
        root = nil;
        _size = 0;
        rotations_ = 0;
        recolorings_ = 0;
    }

    // Destrutor iterativo
    ~RBTree() {
        std::queue<RBNode*> q;
        if (root != nil) q.push(root);
        while (!q.empty()) {
            RBNode* n = q.front(); q.pop();
            if (n->left  != nil) q.push(n->left);
            if (n->right != nil) q.push(n->right);
            delete n;
        }
        delete nil;
    }

    // Atribui cor a um nó; conta somente quando a cor muda e o nó não é NIL
    void paint(RBNode* node, Cor cor) {
        if (node == nil) return;
        if (node->color != cor) {
            node->color = cor;
            recolorings_++;
        }
    }

    // --- Interface pública ---

    bool insert(int key) {
        // Busca para verificar duplicata e encontrar posição
        RBNode* y = nil;
        RBNode* x = root;
        while (x != nil) {
            y = x;
            if (key < x->key)       x = x->left;
            else if (key > x->key)  x = x->right;
            else                    return false; // duplicata
        }

        RBNode* z = new RBNode(key, Cor::VERMELHO, nil);
        z->parent = y;

        if (y == nil)
            root = z;
        else if (key < y->key)
            y->left = z;
        else
            y->right = z;

        // z.left, z.right, z.color já inicializados no construtor
        // New node born red – não conta como recoloração
        _size++;
        insertFixup(z);
        return true;
    }

    bool remove(int key) {
        RBNode* z = findNode(key);
        if (z == nil) return false;
        rbDelete(z);
        _size--;
        return true;
    }

    bool search(int key) const {
        return findNode(key) != nil;
    }

    // Altura da árvore (raiz vazia = 0)
    int height() const {
        if (root == nil) return 0;
        std::stack<std::pair<RBNode*, int>> s;
        s.push({root, 1});
        int maxH = 0;
        while (!s.empty()) {
            auto [node, h] = s.top(); s.pop();
            if (h > maxH) maxH = h;
            if (node->left  != nil) s.push({node->left,  h + 1});
            if (node->right != nil) s.push({node->right, h + 1});
        }
        return maxH;
    }

    int size() const { return _size; }

    // Profundidade média com raiz em profundidade 1
    double averageDepth() const {
        if (root == nil) return 0.0;
        std::stack<std::pair<RBNode*, int>> s;
        s.push({root, 1});
        long long soma = 0;
        int cont = 0;
        while (!s.empty()) {
            auto [node, d] = s.top(); s.pop();
            soma += d;
            cont++;
            if (node->left  != nil) s.push({node->left,  d + 1});
            if (node->right != nil) s.push({node->right, d + 1});
        }
        return (double)soma / cont;
    }

    // Percurso em ordem iterativo
    std::vector<int> inorder() const {
        std::vector<int> result;
        std::stack<RBNode*> s;
        RBNode* curr = root;
        while (curr != nil || !s.empty()) {
            while (curr != nil) { s.push(curr); curr = curr->left; }
            curr = s.top(); s.pop();
            result.push_back(curr->key);
            curr = curr->right;
        }
        return result;
    }

    // Cor de um nó: "vermelho", "preto" ou "ausente"
    std::string colorOf(int key) const {
        RBNode* n = findNode(key);
        if (n == nil) return "ausente";
        return (n->color == Cor::VERMELHO) ? "vermelho" : "preto";
    }

    void resetStats() {
        rotations_   = 0;
        recolorings_ = 0;
    }

    // Valida todas as propriedades de árvore rubro-negra
    std::pair<bool, std::vector<std::string>> validate() const {
        std::vector<std::string> erros;

        // 1. NIL é preto
        if (nil->color != Cor::PRETO)
            erros.push_back("Nó NIL não é preto");

        // 2. Raiz é preta
        if (root != nil && root->color != Cor::PRETO)
            erros.push_back("Raiz não é preta");

        // Percorre todos os nós iterativamente
        std::stack<RBNode*> stk;
        RBNode* curr = root;
        int prev_key = INT_MIN;
        int contados = 0;
        bool blackHeightOK = true;
        int refBH = -1;

        // DFS para verificar altura negra (varre até cada folha)
        {
            struct Frame { RBNode* node; int bh; };
            std::stack<Frame> dfs;
            if (root != nil) dfs.push({root, 0});
            while (!dfs.empty()) {
                auto [node, bh] = dfs.top(); dfs.pop();
                int newBH = bh + (node->color == Cor::PRETO ? 1 : 0);

                // 3. Nenhum nó vermelho tem filho vermelho
                if (node->color == Cor::VERMELHO) {
                    if (node->left  != nil && node->left->color  == Cor::VERMELHO)
                        erros.push_back("Nó vermelho " + std::to_string(node->key) + " tem filho esquerdo vermelho");
                    if (node->right != nil && node->right->color == Cor::VERMELHO)
                        erros.push_back("Nó vermelho " + std::to_string(node->key) + " tem filho direito vermelho");
                }

                // 4. Ponteiros de pai
                if (node->left != nil && node->left->parent != node)
                    erros.push_back("Ponteiro de pai incorreto em " + std::to_string(node->left->key));
                if (node->right != nil && node->right->parent != node)
                    erros.push_back("Ponteiro de pai incorreto em " + std::to_string(node->right->key));

                bool ehFolha = (node->left == nil && node->right == nil);
                if (ehFolha) {
                    int pathBH = newBH + 1; // conta o NIL como preto
                    if (refBH == -1) refBH = pathBH;
                    else if (pathBH != refBH && blackHeightOK) {
                        erros.push_back("Altura negra inconsistente: esperada " + std::to_string(refBH) +
                            " encontrada " + std::to_string(pathBH));
                        blackHeightOK = false;
                    }
                } else {
                    if (node->left  != nil) dfs.push({node->left,  newBH});
                    else {
                        // Caminho até NIL esquerdo
                        int pathBH = newBH + 1;
                        if (refBH == -1) refBH = pathBH;
                        else if (pathBH != refBH && blackHeightOK) {
                            erros.push_back("Altura negra inconsistente: esperada " + std::to_string(refBH) +
                                " encontrada " + std::to_string(pathBH));
                            blackHeightOK = false;
                        }
                    }
                    if (node->right != nil) dfs.push({node->right, newBH});
                    else {
                        int pathBH = newBH + 1;
                        if (refBH == -1) refBH = pathBH;
                        else if (pathBH != refBH && blackHeightOK) {
                            erros.push_back("Altura negra inconsistente: esperada " + std::to_string(refBH) +
                                " encontrada " + std::to_string(pathBH));
                            blackHeightOK = false;
                        }
                    }
                }
            }
        }

        // Percurso em ordem para BST e tamanho
        while (curr != nil || !stk.empty()) {
            while (curr != nil) { stk.push(curr); curr = curr->left; }
            curr = stk.top(); stk.pop();

            if (curr->key <= prev_key)
                erros.push_back("Violação da propriedade BST: " +
                    std::to_string(prev_key) + " >= " + std::to_string(curr->key));
            prev_key = curr->key;
            contados++;
            curr = curr->right;
        }

        if (contados != _size)
            erros.push_back("Tamanho incorreto: armazenado=" + std::to_string(_size) +
                " calculado=" + std::to_string(contados));

        return {erros.empty(), erros};
    }

    // Impressão da árvore no estilo prefixo
    void printTree(std::ostream& out = std::cout) const {
        if (root == nil) { out << "(árvore vazia)\n"; return; }
        if (_size > 80) {
            out << "A árvore tem " << _size << " elementos, muito grande para imprimir. Mostrando só as métricas.\n";
            return;
        }
        auto label = [](RBNode* n) -> std::string {
            return std::to_string(n->key) + (n->color == Cor::VERMELHO ? " (V)" : " (P)");
        };

        out << label(root) << "\n";

        struct Frame {
            RBNode*     node;
            std::string prefix;
            char        dir;
            bool        isLast;
        };
        std::stack<Frame> stk;

        auto pushChildren = [&](RBNode* node, const std::string& prefix) {
            bool temEsq = node->left  != nil;
            bool temDir = node->right != nil;
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
            out << prefix << conn << dir << " " << label(node) << "\n";
            std::string childPrefix = prefix + (isLast ? "   " : "│  ");
            pushChildren(node, childPrefix);
        }
        out << "Legenda: (V) vermelho, (P) preto, E = esquerda, D = direita\n";
    }

private:
    // Busca nó por chave (retorna nil se não encontrado)
    RBNode* findNode(int key) const {
        RBNode* x = root;
        while (x != nil) {
            if (key == x->key) return x;
            x = (key < x->key) ? x->left : x->right;
        }
        return nil;
    }

    // Mínimo de uma subárvore
    RBNode* minimum(RBNode* x) const {
        while (x->left != nil) x = x->left;
        return x;
    }

    // Rotação à esquerda em torno de x
    void rotateLeft(RBNode* x) {
        rotations_++;
        RBNode* y  = x->right;
        x->right   = y->left;
        if (y->left != nil) y->left->parent = x;
        y->parent  = x->parent;
        if (x->parent == nil)      root = y;
        else if (x == x->parent->left) x->parent->left  = y;
        else                           x->parent->right = y;
        y->left    = x;
        x->parent  = y;
    }

    // Rotação à direita em torno de y
    void rotateRight(RBNode* y) {
        rotations_++;
        RBNode* x  = y->left;
        y->left    = x->right;
        if (x->right != nil) x->right->parent = y;
        x->parent  = y->parent;
        if (y->parent == nil)       root = x;
        else if (y == y->parent->right) y->parent->right = x;
        else                            y->parent->left  = x;
        x->right   = y;
        y->parent  = x;
    }

    // Corrige violações após inserção (CLRS RB-INSERT-FIXUP)
    void insertFixup(RBNode* z) {
        while (z->parent->color == Cor::VERMELHO) {
            if (z->parent == z->parent->parent->left) {
                // Pai é filho esquerdo do avô
                RBNode* y = z->parent->parent->right; // tio
                if (y->color == Cor::VERMELHO) {
                    // Caso 1: tio vermelho → recolore e sobe
                    paint(z->parent,         Cor::PRETO);
                    paint(y,                 Cor::PRETO);
                    paint(z->parent->parent, Cor::VERMELHO);
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Caso 2: z é filho direito → rotação esquerda
                        z = z->parent;
                        rotateLeft(z);
                    }
                    // Caso 3: z é filho esquerdo → recolore e rotaciona
                    paint(z->parent,         Cor::PRETO);
                    paint(z->parent->parent, Cor::VERMELHO);
                    rotateRight(z->parent->parent);
                }
            } else {
                // Simétrico: pai é filho direito do avô
                RBNode* y = z->parent->parent->left; // tio
                if (y->color == Cor::VERMELHO) {
                    // Caso 1 simétrico
                    paint(z->parent,         Cor::PRETO);
                    paint(y,                 Cor::PRETO);
                    paint(z->parent->parent, Cor::VERMELHO);
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        // Caso 2 simétrico
                        z = z->parent;
                        rotateRight(z);
                    }
                    // Caso 3 simétrico
                    paint(z->parent,         Cor::PRETO);
                    paint(z->parent->parent, Cor::VERMELHO);
                    rotateLeft(z->parent->parent);
                }
            }
        }
        // Garante que a raiz é sempre preta
        paint(root, Cor::PRETO);
    }

    // Substitui subárvore u por subárvore v (CLRS RB-TRANSPLANT)
    void transplant(RBNode* u, RBNode* v) {
        if (u->parent == nil)         root = v;
        else if (u == u->parent->left) u->parent->left  = v;
        else                           u->parent->right = v;
        v->parent = u->parent;
    }

    // Remoção completa (CLRS RB-DELETE)
    void rbDelete(RBNode* z) {
        RBNode* y = z;
        Cor yOrigColor = y->color;
        RBNode* x;

        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            // Dois filhos: usa o sucessor em ordem
            y = minimum(z->right);
            yOrigColor = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color; // Atribuição direta; não conta como recoloração
        }
        delete z;

        if (yOrigColor == Cor::PRETO)
            deleteFixup(x);
    }

    // Corrige violações após remoção (CLRS RB-DELETE-FIXUP)
    void deleteFixup(RBNode* x) {
        while (x != root && x->color == Cor::PRETO) {
            if (x == x->parent->left) {
                RBNode* w = x->parent->right; // irmão
                if (w->color == Cor::VERMELHO) {
                    // Caso 1: irmão vermelho
                    paint(w,          Cor::PRETO);
                    paint(x->parent,  Cor::VERMELHO);
                    rotateLeft(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Cor::PRETO && w->right->color == Cor::PRETO) {
                    // Caso 2: ambos os filhos do irmão são pretos
                    paint(w, Cor::VERMELHO);
                    x = x->parent;
                } else {
                    if (w->right->color == Cor::PRETO) {
                        // Caso 3: filho direito do irmão é preto
                        paint(w->left, Cor::PRETO);
                        paint(w,       Cor::VERMELHO);
                        rotateRight(w);
                        w = x->parent->right;
                    }
                    // Caso 4: filho direito do irmão é vermelho
                    paint(w,          x->parent->color);
                    paint(x->parent,  Cor::PRETO);
                    paint(w->right,   Cor::PRETO);
                    rotateLeft(x->parent);
                    x = root;
                }
            } else {
                // Simétrico: x é filho direito
                RBNode* w = x->parent->left;
                if (w->color == Cor::VERMELHO) {
                    // Caso 1 simétrico
                    paint(w,          Cor::PRETO);
                    paint(x->parent,  Cor::VERMELHO);
                    rotateRight(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == Cor::PRETO && w->left->color == Cor::PRETO) {
                    // Caso 2 simétrico
                    paint(w, Cor::VERMELHO);
                    x = x->parent;
                } else {
                    if (w->left->color == Cor::PRETO) {
                        // Caso 3 simétrico
                        paint(w->right, Cor::PRETO);
                        paint(w,        Cor::VERMELHO);
                        rotateLeft(w);
                        w = x->parent->left;
                    }
                    // Caso 4 simétrico
                    paint(w,          x->parent->color);
                    paint(x->parent,  Cor::PRETO);
                    paint(w->left,    Cor::PRETO);
                    rotateRight(x->parent);
                    x = root;
                }
            }
        }
        paint(x, Cor::PRETO);
    }
};
