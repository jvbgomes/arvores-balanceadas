// Programa principal: menu interativo em português
// Suporta árvore AVL e árvore Rubro-Negra
#include "avl.hpp"
#include "rbtree.hpp"
#include "datasets.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

// Lê um inteiro da linha; retorna false se a entrada não for um inteiro válido
static bool lerInteiro(int& val) {
    std::string linha;
    if (!std::getline(std::cin, linha)) return false;
    if (linha.empty()) return false;
    std::istringstream ss(linha);
    ss >> val;
    return !ss.fail() && ss.eof();
}

// Imprime o cabeçalho do menu de operações
static void imprimirMenuOp(bool isAVL, int tamanho, int altura) {
    std::cout << "\n";
    if (isAVL)
        std::cout << "[AVL | " << tamanho << " elemento(s) | altura " << altura << "]\n";
    else
        std::cout << "[Rubro-Negra | " << tamanho << " elemento(s) | altura " << altura << "]\n";

    std::cout << " 1) Inserir identificador\n";
    std::cout << " 2) Remover identificador\n";
    std::cout << " 3) Buscar identificador\n";
    std::cout << " 4) Percurso em ordem (crescente)\n";
    std::cout << " 5) Imprimir estrutura da árvore\n";
    std::cout << " 6) Inserir vários (separados por espaço ou vírgula)\n";
    std::cout << " 7) Carregar N identificadores aleatórios\n";
    std::cout << " 8) Verificar propriedades de balanceamento\n";
    std::cout << " 9) Estatísticas (altura, rotações, recolorações)\n";
    std::cout << " 10) Trocar de estrutura (começa com árvore vazia)\n";
    std::cout << " 0) Sair\n";
    std::cout << "Opção: ";
    std::cout.flush();
}

// Analisa uma string de inteiros separados por espaço ou vírgula
// Retorna true se todos os tokens forem inteiros válidos, false se houver inválido ou vazio
static bool parsearInteiros(const std::string& entrada, std::vector<int>& vals) {
    vals.clear();
    if (entrada.empty()) return false;
    // Substitui vírgulas por espaços
    std::string s = entrada;
    for (char& c : s) if (c == ',') c = ' ';
    std::istringstream ss(s);
    std::string token;
    while (ss >> token) {
        // Verifica que o token é um inteiro puro
        bool neg = (token[0] == '-');
        std::string digits = neg ? token.substr(1) : token;
        if (digits.empty()) return false;
        for (char c : digits) if (!std::isdigit(c)) return false;
        try {
            vals.push_back(std::stoi(token));
        } catch (...) { return false; }
    }
    return !vals.empty();
}

// Loop de operações para uma árvore AVL
static bool menuAVL(AVLTree& avl) {
    while (true) {
        imprimirMenuOp(true, avl._size, avl.height());
        std::string linhaOp;
        if (!std::getline(std::cin, linhaOp)) {
            std::cout << "\nEncerrando.\n";
            return true; // EOF → encerra o programa
        }
        int op = -1;
        {
            std::istringstream ss(linhaOp);
            if (!(ss >> op) || !ss.eof()) op = -1;
        }

        if (op == 0) return false; // voltar à escolha de estrutura → na verdade "sair"
        // op==0 sai do programa; op==10 troca estrutura

        if (op == 1) {
            std::cout << "Identificador a inserir: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else if (avl.insert(v)) {
                std::cout << "Inserido: " << v << "\n";
            } else {
                std::cout << "O identificador " << v << " já existe. Inserção ignorada (a árvore não aceita repetidos).\n";
            }
        } else if (op == 2) {
            std::cout << "Identificador a remover: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else if (avl.remove(v)) {
                std::cout << "Removido: " << v << "\n";
            } else {
                std::cout << "O identificador " << v << " não existe. Nada foi removido.\n";
            }
        } else if (op == 3) {
            std::cout << "Identificador a buscar: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else if (avl.search(v)) {
                std::cout << "Encontrado: " << v << "\n";
            } else {
                std::cout << "Não encontrado: " << v << "\n";
            }
        } else if (op == 4) {
            auto seq = avl.inorder();
            if (seq.empty()) {
                std::cout << "(árvore vazia)\n";
            } else {
                for (int i = 0; i < (int)seq.size(); i++) {
                    if (i > 0) std::cout << " ";
                    std::cout << seq[i];
                }
                std::cout << "\n";
            }
        } else if (op == 5) {
            avl.printTree();
        } else if (op == 6) {
            std::cout << "Identificadores: ";
            std::string linha;
            if (!std::getline(std::cin, linha)) {
                std::cout << "Entrada inválida: informe apenas números inteiros.\n";
                continue;
            }
            std::vector<int> vals;
            if (!parsearInteiros(linha, vals)) {
                std::cout << "Entrada inválida: informe apenas números inteiros.\n";
            } else {
                int inseridos = 0, repetidos = 0;
                for (int v : vals) {
                    if (avl.insert(v)) inseridos++;
                    else               repetidos++;
                }
                std::cout << "Inseridos: " << inseridos << ". Repetidos ignorados: " << repetidos << ".\n";
            }
        } else if (op == 7) {
            std::cout << "Quantos identificadores (1 a 100000)? ";
            int n;
            if (!lerInteiro(n) || n < 1 || n > 100000) {
                std::cout << "Entrada inválida: informe um inteiro de 1 a 100000.\n";
            } else {
                auto agora = std::chrono::steady_clock::now().time_since_epoch().count();
                uint32_t semente = (uint32_t)(agora & 0xFFFFFFFF);
                auto dados = randomDataset(n, semente);
                int k = 0;
                for (int v : dados) if (avl.insert(v)) k++;
                std::cout << "Inseridos " << k << " identificadores embaralhados (1 a " << n << ").\n";
            }
        } else if (op == 8) {
            auto [ok, erros] = avl.validate();
            if (ok) {
                std::cout << "Todas as propriedades de balanceamento estão preservadas.\n";
            } else {
                std::cout << "Violações encontradas:\n";
                for (auto& e : erros) std::cout << " - " << e << "\n";
            }
        } else if (op == 9) {
            std::cout << "Elementos: "  << avl._size    << "\n";
            std::cout << "Altura (em níveis): " << avl.height() << "\n";
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Profundidade média dos nós: " << avl.averageDepth() << "\n";
            std::cout << "Rotações realizadas: "  << avl.rotations_  << "\n";
            std::cout << "Casos AVL: LL=" << avl.cases_LL
                      << " RR=" << avl.cases_RR
                      << " LR=" << avl.cases_LR
                      << " RL=" << avl.cases_RL << "\n";
        } else if (op == 10) {
            return false; // troca de estrutura
        } else {
            std::cout << "Opção inválida.\n";
        }
    }
}

// Loop de operações para uma árvore Rubro-Negra
static bool menuRB(RBTree& rb) {
    while (true) {
        imprimirMenuOp(false, rb._size, rb.height());
        std::string linhaOp;
        if (!std::getline(std::cin, linhaOp)) {
            std::cout << "\nEncerrando.\n";
            return true;
        }
        int op = -1;
        {
            std::istringstream ss(linhaOp);
            if (!(ss >> op) || !ss.eof()) op = -1;
        }

        if (op == 0) return true; // sair

        if (op == 1) {
            std::cout << "Identificador a inserir: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else if (rb.insert(v)) {
                std::cout << "Inserido: " << v << "\n";
            } else {
                std::cout << "O identificador " << v << " já existe. Inserção ignorada (a árvore não aceita repetidos).\n";
            }
        } else if (op == 2) {
            std::cout << "Identificador a remover: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else if (rb.remove(v)) {
                std::cout << "Removido: " << v << "\n";
            } else {
                std::cout << "O identificador " << v << " não existe. Nada foi removido.\n";
            }
        } else if (op == 3) {
            std::cout << "Identificador a buscar: ";
            int v;
            if (!lerInteiro(v)) {
                std::cout << "Entrada inválida: informe um número inteiro.\n";
            } else {
                std::string cor = rb.colorOf(v);
                if (cor == "ausente") {
                    std::cout << "Não encontrado: " << v << "\n";
                } else {
                    std::cout << "Encontrado: " << v << " (nó " << cor << ")\n";
                }
            }
        } else if (op == 4) {
            auto seq = rb.inorder();
            if (seq.empty()) {
                std::cout << "(árvore vazia)\n";
            } else {
                for (int i = 0; i < (int)seq.size(); i++) {
                    if (i > 0) std::cout << " ";
                    std::cout << seq[i];
                }
                std::cout << "\n";
            }
        } else if (op == 5) {
            rb.printTree();
        } else if (op == 6) {
            std::cout << "Identificadores: ";
            std::string linha;
            if (!std::getline(std::cin, linha)) {
                std::cout << "Entrada inválida: informe apenas números inteiros.\n";
                continue;
            }
            std::vector<int> vals;
            if (!parsearInteiros(linha, vals)) {
                std::cout << "Entrada inválida: informe apenas números inteiros.\n";
            } else {
                int inseridos = 0, repetidos = 0;
                for (int v : vals) {
                    if (rb.insert(v)) inseridos++;
                    else               repetidos++;
                }
                std::cout << "Inseridos: " << inseridos << ". Repetidos ignorados: " << repetidos << ".\n";
            }
        } else if (op == 7) {
            std::cout << "Quantos identificadores (1 a 100000)? ";
            int n;
            if (!lerInteiro(n) || n < 1 || n > 100000) {
                std::cout << "Entrada inválida: informe um inteiro de 1 a 100000.\n";
            } else {
                auto agora = std::chrono::steady_clock::now().time_since_epoch().count();
                uint32_t semente = (uint32_t)(agora & 0xFFFFFFFF);
                auto dados = randomDataset(n, semente);
                int k = 0;
                for (int v : dados) if (rb.insert(v)) k++;
                std::cout << "Inseridos " << k << " identificadores embaralhados (1 a " << n << ").\n";
            }
        } else if (op == 8) {
            auto [ok, erros] = rb.validate();
            if (ok) {
                std::cout << "Todas as propriedades de balanceamento estão preservadas.\n";
            } else {
                std::cout << "Violações encontradas:\n";
                for (auto& e : erros) std::cout << " - " << e << "\n";
            }
        } else if (op == 9) {
            std::cout << "Elementos: "  << rb._size    << "\n";
            std::cout << "Altura (em níveis): " << rb.height() << "\n";
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Profundidade média dos nós: " << rb.averageDepth() << "\n";
            std::cout << "Rotações realizadas: "  << rb.rotations_  << "\n";
            std::cout << "Recolorações realizadas: " << rb.recolorings_ << "\n";
        } else if (op == 10) {
            return false; // troca de estrutura
        } else {
            std::cout << "Opção inválida.\n";
        }
    }
}

int main() {
    std::cout << "=== Logística da Black Friday: árvores balanceadas ===\n";

    while (true) {
        std::cout << "Escolha a estrutura de dados:\n";
        std::cout << " 1) Árvore AVL\n";
        std::cout << " 2) Árvore Rubro-Negra\n";
        std::cout << " 0) Sair\n";
        std::cout << "Opção: ";
        std::cout.flush();

        std::string linha;
        if (!std::getline(std::cin, linha)) {
            std::cout << "\nEncerrando.\n";
            return 0;
        }

        int escolha = -1;
        {
            std::istringstream ss(linha);
            if (!(ss >> escolha) || !ss.eof()) escolha = -1;
        }

        if (escolha == 0) {
            std::cout << "Encerrando.\n";
            return 0;
        } else if (escolha == 1) {
            AVLTree avl;
            bool sair = menuAVL(avl);
            if (sair) return 0;
            // troca de estrutura: volta ao início do while
        } else if (escolha == 2) {
            RBTree rb;
            bool sair = menuRB(rb);
            if (sair) return 0;
        } else {
            std::cout << "Opção inválida. Digite 1, 2 ou 0.\n";
        }
    }
}
