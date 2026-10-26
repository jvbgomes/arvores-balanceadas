// Demo roteirizado: mesmas operações em AVL e Rubro-Negra
// Grava saída em results/demo-output.txt e imprime no terminal
#include "avl.hpp"
#include "rbtree.hpp"
#include "datasets.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

// Stream que duplica a saída para terminal e arquivo
class DualStream : public std::ostream {
    class DualBuf : public std::streambuf {
    public:
        std::ostream& a; std::ostream& b;
        DualBuf(std::ostream& a, std::ostream& b) : a(a), b(b) {}
        int overflow(int c) override {
            if (c != EOF) { a.put((char)c); b.put((char)c); }
            return c;
        }
    };
public:
    DualBuf buf;
    DualStream(std::ostream& a, std::ostream& b) : std::ostream(&buf), buf(a, b) {}
};

static void separa(std::ostream& out, const std::string& titulo) {
    out << "\n" << std::string(60, '=') << "\n";
    out << "  " << titulo << "\n";
    out << std::string(60, '=') << "\n";
}

static void statsAVL(AVLTree& t, std::ostream& out) {
    out << "  AVL | h=" << t.height() << " rot=" << t.rotations_
        << " LL=" << t.cases_LL << " RR=" << t.cases_RR
        << " LR=" << t.cases_LR << " RL=" << t.cases_RL << "\n";
}

static void statsRB(RBTree& t, std::ostream& out) {
    out << "  RB  | h=" << t.height() << " rot=" << t.rotations_
        << " rec=" << t.recolorings_ << "\n";
}

static void imprimir(AVLTree& avl, RBTree& rb, std::ostream& out) {
    out << "\n--- AVL ---\n"; avl.printTree(out);
    out << "\n--- RB  ---\n"; rb.printTree(out);
}

int main() {
    std::ofstream arq("results/demo-output.txt");
    DualStream out(std::cout, arq);

    out << "=== Demo Roteirizado: AVL + Rubro-Negra ===\n";

    // ------ Passo 1: inserir 10, 20, 30 ------
    separa(out, "Passo 1: inserir 10, 20, 30");
    AVLTree avl1; RBTree rb1;
    for (int v : {10, 20, 30}) { avl1.insert(v); rb1.insert(v); }
    imprimir(avl1, rb1, out);
    statsAVL(avl1, out); statsRB(rb1, out);

    // ------ Passo 2: inserir 30, 10, 20 (rotação dupla) ------
    separa(out, "Passo 2: inserir 30, 10, 20 (rotação dupla)");
    AVLTree avl2; RBTree rb2;
    for (int v : {30, 10, 20}) { avl2.insert(v); rb2.insert(v); }
    imprimir(avl2, rb2, out);
    statsAVL(avl2, out); statsRB(rb2, out);

    // ------ Passo 3: RB com 10, 5, 15 e depois 1 (só recolore) ------
    separa(out, "Passo 3: RB - inserir 10, 5, 15 e depois 1 (caso 1: só recolore)");
    {
        RBTree rb;
        for (int v : {10, 5, 15}) rb.insert(v);
        out << "Após 10, 5, 15: h=" << rb.height() << " rot=" << rb.rotations_
            << " rec=" << rb.recolorings_ << "\n";
        rb.resetStats();
        rb.insert(1);
        out << "Após inserir 1: rot=" << rb.rotations_ << " rec=" << rb.recolorings_ << "\n";
        out << "Cores: 10=" << rb.colorOf(10) << " 5=" << rb.colorOf(5)
            << " 15=" << rb.colorOf(15) << " 1=" << rb.colorOf(1) << "\n";
        rb.printTree(out);
    }

    // ------ Passo 4: inserir 1..15 em ordem crescente ------
    separa(out, "Passo 4: inserir 1..15 em ordem crescente");
    AVLTree avl4; RBTree rb4;
    for (int i = 1; i <= 15; i++) { avl4.insert(i); rb4.insert(i); }
    imprimir(avl4, rb4, out);
    statsAVL(avl4, out); statsRB(rb4, out);
    out << "  RB nós vermelhos: ";
    for (int i = 1; i <= 15; i++)
        if (rb4.colorOf(i) == "vermelho") out << i << " ";
    out << "\n";

    // ------ Passo 5: busca de 7 e de 99 ------
    separa(out, "Passo 5: busca de 7 e de 99 na árvore 1..15");
    out << "AVL busca 7:  " << (avl4.search(7)  ? "encontrado" : "não encontrado") << "\n";
    out << "AVL busca 99: " << (avl4.search(99) ? "encontrado" : "não encontrado") << "\n";
    out << "RB  busca 7:  " << rb4.colorOf(7)   << "\n";
    out << "RB  busca 99: " << rb4.colorOf(99)  << "\n";

    // ------ Passo 6: valor repetido e remoção de inexistente ------
    separa(out, "Passo 6: inserção repetida e remoção de inexistente");
    out << "AVL inserir 7 (repetido): " << (avl4.insert(7) ? "inserido" : "ignorado") << "\n";
    out << "AVL remover 99 (inexist): " << (avl4.remove(99) ? "removido" : "não encontrado") << "\n";
    out << "RB  inserir 7 (repetido): " << (rb4.insert(7) ? "inserido" : "ignorado") << "\n";
    out << "RB  remover 99 (inexist): " << (rb4.remove(99) ? "removido" : "não encontrado") << "\n";

    // ------ Passo 7: remoções de folha, 1 filho, 2 filhos e raiz ------
    separa(out, "Passo 7: remoções em {50,30,70,20,40,60,80,10,65}");
    AVLTree avl7; RBTree rb7;
    for (int v : {50, 30, 70, 20, 40, 60, 80, 10, 65}) {
        avl7.insert(v); rb7.insert(v);
    }
    out << "Árvore inicial:\n";
    imprimir(avl7, rb7, out);

    out << "\nRemovendo 10 (folha):\n";
    avl7.remove(10); rb7.remove(10);
    imprimir(avl7, rb7, out);
    statsAVL(avl7, out); statsRB(rb7, out);

    out << "\nRemovendo 30 (1 filho potencial):\n";
    avl7.remove(30); rb7.remove(30);
    imprimir(avl7, rb7, out);
    statsAVL(avl7, out); statsRB(rb7, out);

    out << "\nRemovendo 70 (2 filhos):\n";
    avl7.remove(70); rb7.remove(70);
    imprimir(avl7, rb7, out);
    statsAVL(avl7, out); statsRB(rb7, out);

    out << "\nRemovendo a raiz " << (avl7.root ? std::to_string(avl7.root->key) : "?") << ":\n";
    if (avl7.root) { int r = avl7.root->key; avl7.remove(r); rb7.remove(r); }
    imprimir(avl7, rb7, out);
    statsAVL(avl7, out); statsRB(rb7, out);

    out << "\nDemo concluído. Saída gravada em results/demo-output.txt\n";
    return 0;
}
