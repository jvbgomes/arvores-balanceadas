// Suíte de testes para AVL e Rubro-Negra
// Mini executor próprio sem dependências externas
// Imprime ✔ / ✖ por teste e retorna código != 0 se houver falhas
#include "../src/avl.hpp"
#include "../src/rbtree.hpp"
#include "../src/bst.hpp"
#include "../src/datasets.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

// ---------- mini framework de testes ----------

static int total_testes = 0;
static int aprovados    = 0;
static int falhas_count = 0;

static void check(bool cond, const std::string& nome, std::ostream& out = std::cout) {
    total_testes++;
    if (cond) {
        out << "  ✔ " << nome << "\n";
        aprovados++;
    } else {
        out << "  ✖ " << nome << "\n";
        falhas_count++;
    }
}

static void secao(const std::string& titulo, std::ostream& out = std::cout) {
    out << "\n[" << titulo << "]\n";
}

// ---------- helpers ----------

// Verifica validade e registra falha se inválida
template<typename T>
static void assertValid(T& tree, const std::string& ctx, std::ostream& out) {
    auto [ok, erros] = tree.validate();
    std::string msg = ctx + " válida";
    check(ok, msg, out);
    if (!ok) {
        for (auto& e : erros) out << "    erro: " << e << "\n";
    }
}

static double log2_(double x) { return std::log(x) / std::log(2.0); }

// ---------- testes em par (AVL + RB) ----------

template<typename T>
static void testarArvoreVazia(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Árvore vazia", out);
    check(!tree.search(0),          nome + ": busca em vazia retorna false",     out);
    check(!tree.remove(0),          nome + ": remoção em vazia retorna false",   out);
    check(tree.inorder().empty(),   nome + ": percurso vazio",                   out);
    check(tree.height() == 0,       nome + ": altura vazia = 0",                 out);
    check(tree.size()   == 0,       nome + ": tamanho vazio = 0",                out);
    // printTree deve retornar "(árvore vazia)"
    std::ostringstream oss;
    tree.printTree(oss);
    check(oss.str().find("(árvore vazia)") != std::string::npos,
          nome + ": impressão de vazia mostra mensagem correta", out);
    assertValid(tree, nome + " vazia", out);
}

template<typename T>
static void testarUmElemento(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Um elemento", out);
    check(tree.insert(42),          nome + ": insere 42",              out);
    check(tree.search(42),          nome + ": busca 42 encontra",      out);
    check(!tree.search(1),          nome + ": busca 1 não encontra",   out);
    check(tree.size()   == 1,       nome + ": tamanho = 1",            out);
    check(tree.height() == 1,       nome + ": altura = 1",             out);
    assertValid(tree, nome + " 1 elem", out);
    auto ord = tree.inorder();
    check(ord.size() == 1 && ord[0] == 42, nome + ": percurso = [42]", out);
    check(tree.remove(42),          nome + ": remove 42",              out);
    check(!tree.remove(42),         nome + ": remove 42 de novo = false", out);
    check(tree.size()   == 0,       nome + ": tamanho = 0 depois",     out);
    assertValid(tree, nome + " após remoção", out);
}

template<typename T>
static void testarRepetidos(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Valores repetidos", out);
    tree.insert(10);
    tree.insert(20);
    check(!tree.insert(10), nome + ": inserir 10 de novo retorna false", out);
    check(!tree.insert(20), nome + ": inserir 20 de novo retorna false", out);
    check(tree.size() == 2, nome + ": tamanho permanece 2",              out);
    auto ord = tree.inorder();
    check(ord.size() == 2 && ord[0] == 10 && ord[1] == 20,
          nome + ": conteúdo inalterado após duplicatas", out);
    assertValid(tree, nome + " após duplicatas", out);
}

template<typename T>
static void testarInsercaoOrdens(T& tree, const std::string& nome,
                                  const std::vector<int>& dados, int n,
                                  std::ostream& out) {
    for (int k : dados) tree.insert(k);
    auto ord = tree.inorder();
    bool emOrdem = true;
    if ((int)ord.size() != n) emOrdem = false;
    for (int i = 0; i < (int)ord.size(); i++)
        if (ord[i] != i + 1) { emOrdem = false; break; }
    check(emOrdem, nome + ": percurso em ordem 1.." + std::to_string(n), out);
    int h = tree.height();
    double limite = 2.0 * log2_((double)n + 1) + 1.0;
    check(h <= (int)std::ceil(limite),
          nome + ": altura " + std::to_string(h) + " <= 2*log2(" + std::to_string(n) + "+1)+1", out);
    assertValid(tree, nome + " após inserção completa", out);
}

template<typename T>
static void testarBuscas(T& tree, int n, const std::string& nome, std::ostream& out) {
    secao(nome + " - Buscas", out);
    bool parOk = true;
    for (int i = 2; i <= n; i += 2)
        if (!tree.search(i)) { parOk = false; break; }
    check(parOk, nome + ": todos os pares existem", out);

    bool impOk = true;
    for (int i = n + 1; i <= n + 10; i += 2)
        if (tree.search(i)) { impOk = false; break; }
    check(impOk, nome + ": ímpares além de n não existem", out);

    check(!tree.search(0),           nome + ": 0 não existe",  out);
    check(!tree.search(-5),          nome + ": -5 não existe", out);
    check(!tree.search(n * 100),     nome + ": valor enorme não existe", out);
}

// ---------- testes de remoção (folha, 1 filho, 2 filhos) ----------

template<typename T>
static void testarRemocoes(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Remoções (folha/1 filho/2 filhos) com random(63,11)", out);
    auto dados = randomDataset(63, 11);
    for (int k : dados) tree.insert(k);
    assertValid(tree, nome + " após inserção 63", out);

    // Remove uma folha: busca um nó sem filhos no percurso
    // Vamos simplesmente remover chaves conhecidas e verificar validade
    // Para garantir cobertura, removemos o mínimo (sempre folha esquerda mais profunda),
    // um nó do meio e a raiz potencial

    auto inOrd = tree.inorder();
    int folha    = inOrd.front();  // menor (provavelmente folha)
    int raiz_key = tree.root ? tree.root->key : (inOrd[inOrd.size()/2]);

    check(tree.remove(folha), nome + ": remove folha " + std::to_string(folha), out);
    assertValid(tree, nome + " após remover folha", out);

    // Remove nó com dois filhos (raiz geralmente tem dois filhos)
    bool removeuRaiz = tree.remove(raiz_key);
    check(removeuRaiz || !tree.search(raiz_key),
          nome + ": remove nó com 2 filhos", out);
    assertValid(tree, nome + " após remover 2 filhos", out);

    // Verifica percurso correto
    auto ordemPos = tree.inorder();
    bool crescente = true;
    for (int i = 1; i < (int)ordemPos.size(); i++)
        if (ordemPos[i] <= ordemPos[i-1]) { crescente = false; break; }
    check(crescente, nome + ": percurso em ordem crescente após remoções", out);
}

template<typename T>
static void testarRemoverRaizRepetidamente(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Remover raiz repetidamente até esvaziar", out);
    for (int i = 1; i <= 15; i++) tree.insert(i);
    int n = tree.size();
    bool ok = true;
    while (tree.size() > 0) {
        int r = tree.root ? tree.root->key : -1;
        if (r == -1) { ok = false; break; }
        if (!tree.remove(r)) { ok = false; break; }
        auto [v, _] = tree.validate();
        if (!v) { ok = false; break; }
    }
    check(ok && tree.size() == 0, nome + ": remover raiz " + std::to_string(n) + " vezes esvaziou a árvore", out);
}

template<typename T>
static void testarRemoverTodasRandom500(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Remover todas as chaves de random(500,3) na ordem de random(500,4)", out);
    auto ins  = randomDataset(500, 3);
    auto rem  = randomDataset(500, 4);
    for (int k : ins) tree.insert(k);
    bool ok = true;
    for (int k : rem) {
        tree.remove(k);
        auto [v, _] = tree.validate();
        if (!v) { ok = false; break; }
    }
    check(ok, nome + ": valida a cada remoção de random(500,4)", out);
    check(tree.size() == 0, nome + ": árvore vazia ao final", out);
}

template<typename T>
static void testarRemoverInexistentes(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - Remoção de inexistentes (1..50 inseridos)", out);
    for (int i = 1; i <= 50; i++) tree.insert(i);
    int sz = tree.size();
    check(!tree.remove(0),   nome + ": remove 0 retorna false",   out);
    check(!tree.remove(51),  nome + ": remove 51 retorna false",  out);
    check(!tree.remove(-1),  nome + ": remove -1 retorna false",  out);
    check(tree.size() == sz, nome + ": tamanho inalterado",       out);
    assertValid(tree, nome + " inexistentes", out);
}

template<typename T>
static void testar5000Ops(T& tree, const std::string& nome, std::ostream& out) {
    secao(nome + " - 5000 operações aleatórias vs std::set", out);
    Mulberry32 rng(2024);
    std::set<int> ref;
    bool ok = true;
    bool validOk = true;
    for (int op = 0; op < 5000; op++) {
        int chave = (int)std::floor(rng.next() * 200);
        double r  = rng.next();
        bool resBool, refBool;
        if (r < 0.45) {
            resBool = tree.insert(chave);
            refBool = ref.insert(chave).second;
        } else if (r < 0.80) {
            resBool = tree.remove(chave);
            refBool = ref.erase(chave) > 0;
        } else {
            resBool = tree.search(chave);
            refBool = ref.count(chave) > 0;
        }
        if (resBool != refBool) { ok = false; }
        auto [v, _] = tree.validate();
        if (!v) { validOk = false; }
    }
    check(ok,      nome + ": retornos coincidem com std::set em 5000 ops", out);
    check(validOk, nome + ": válida a cada operação",                       out);
    // Percurso final
    auto ord = tree.inorder();
    std::vector<int> refv(ref.begin(), ref.end());
    check(ord == refv,            nome + ": percurso final igual ao std::set", out);
    check(tree.size() == (int)ref.size(), nome + ": tamanho igual ao std::set", out);
}

template<typename T>
static void testarLimiteAltura20000(T& tree, const std::string& nome,
                                     bool isRB, std::ostream& out) {
    secao(nome + " - Limite de altura 20000 chaves crescentes", out);
    for (int i = 1; i <= 20000; i++) tree.insert(i);
    int h = tree.height();
    int n = 20000;
    bool ok;
    if (isRB) {
        double lim = 2.0 * log2_((double)n + 1) + 1.0;
        ok = h <= (int)std::ceil(lim);
        check(ok, nome + ": altura " + std::to_string(h) +
              " <= 2*log2(n+1)+1=" + std::to_string((int)std::ceil(lim)), out);
    } else {
        double lim = 1.4405 * log2_((double)n + 2) + 0.6723;
        ok = h <= (int)std::ceil(lim);
        check(ok, nome + ": altura " + std::to_string(h) +
              " <= 1.4405*log2(n+2)+0.6723=" + std::to_string((int)std::ceil(lim)), out);
    }
}

// ---------- testes específicos AVL ----------

static void testarCasosAVL(std::ostream& out) {
    secao("AVL - Casos conhecidos", out);

    // 1, 2, 3 → raiz 2, 1 rotação, caso RR, altura 2
    {
        AVLTree t;
        t.insert(1); t.insert(2); t.insert(3);
        check(t.root && t.root->key == 2, "AVL: 1,2,3 → raiz=2", out);
        check(t.rotations_ == 1,          "AVL: 1,2,3 → 1 rotação", out);
        check(t.cases_RR   == 1,          "AVL: 1,2,3 → caso RR",   out);
        check(t.height()   == 2,          "AVL: 1,2,3 → altura=2",  out);
    }
    // 3, 2, 1 → caso LL
    {
        AVLTree t;
        t.insert(3); t.insert(2); t.insert(1);
        check(t.root && t.root->key == 2, "AVL: 3,2,1 → raiz=2",   out);
        check(t.cases_LL == 1,            "AVL: 3,2,1 → caso LL",   out);
    }
    // 3, 1, 2 → caso LR, 2 rotações
    {
        AVLTree t;
        t.insert(3); t.insert(1); t.insert(2);
        check(t.root && t.root->key == 2, "AVL: 3,1,2 → raiz=2",    out);
        check(t.cases_LR  == 1,           "AVL: 3,1,2 → caso LR",    out);
        check(t.rotations_ == 2,          "AVL: 3,1,2 → 2 rotações", out);
    }
    // 1, 3, 2 → caso RL, 2 rotações
    {
        AVLTree t;
        t.insert(1); t.insert(3); t.insert(2);
        check(t.root && t.root->key == 2, "AVL: 1,3,2 → raiz=2",    out);
        check(t.cases_RL  == 1,           "AVL: 1,3,2 → caso RL",    out);
        check(t.rotations_ == 2,          "AVL: 1,3,2 → 2 rotações", out);
    }
}

// ---------- testes específicos RB ----------

static void testarCasosRB(std::ostream& out) {
    secao("RB - Casos conhecidos", out);

    // 1, 2, 3 → raiz 2 preta, 1 e 3 vermelhos, 1 rotação
    {
        RBTree t;
        t.insert(1); t.insert(2); t.insert(3);
        check(t.root && t.root->key == 2,              "RB: 1,2,3 → raiz=2",      out);
        check(t.root->color == Cor::PRETO,             "RB: 1,2,3 → raiz preta",  out);
        check(t.root->left  && t.root->left->color  == Cor::VERMELHO, "RB: 1,2,3 → 1 vermelho", out);
        check(t.root->right && t.root->right->color == Cor::VERMELHO, "RB: 1,2,3 → 3 vermelho", out);
        check(t.rotations_ == 1,                       "RB: 1,2,3 → 1 rotação",   out);
    }
    // 10, 5, 15 e depois 1 → zero rotações, recolore, 10/5/15 pretos, 1 vermelho
    {
        RBTree t;
        t.insert(10); t.insert(5); t.insert(15);
        t.resetStats();
        t.insert(1);
        check(t.rotations_ == 0,                   "RB: após inserir 1 em {10,5,15} → 0 rotações", out);
        check(t.colorOf(10) == "preto",            "RB: 10 preto",  out);
        check(t.colorOf(5)  == "preto",            "RB: 5 preto",   out);
        check(t.colorOf(15) == "preto",            "RB: 15 preto",  out);
        check(t.colorOf(1)  == "vermelho",         "RB: 1 vermelho", out);
    }
    // 3, 1, 2 → raiz 2, 2 rotações
    {
        RBTree t;
        t.insert(3); t.insert(1); t.insert(2);
        check(t.root && t.root->key == 2, "RB: 3,1,2 → raiz=2",    out);
        check(t.rotations_ == 2,          "RB: 3,1,2 → 2 rotações", out);
    }
    // colorOf de chave inexistente
    {
        RBTree t;
        t.insert(5);
        check(t.colorOf(99) == "ausente", "RB: colorOf inexistente = ausente", out);
    }
}

// ---------- validadores detectam erros ----------

static void testarValidadoresErros(std::ostream& out) {
    secao("Validadores detectam erros injetados", out);

    // AVL: trocar chave e somar 3 à altura da raiz
    {
        AVLTree t;
        for (int i = 1; i <= 10; i++) t.insert(i);
        if (t.root && t.root->left) {
            t.root->left->key += 1000; // quebra ordem BST
        }
        int altOrig = t.root->height;
        t.root->height += 3;
        auto [ok, erros] = t.validate();
        check(!ok, "AVL: detecta chave corrompida e altura errada", out);
        // Restaura para não corromper destrutor
        if (t.root && t.root->left) t.root->left->key -= 1000;
        t.root->height = altOrig;
    }
    // RB: pintar raiz de vermelho → deve detectar
    {
        RBTree t;
        for (int i = 1; i <= 7; i++) t.insert(i);
        t.root->color = Cor::VERMELHO; // viola propriedade RB
        auto [ok1, _1] = t.validate();
        check(!ok1, "RB: detecta raiz vermelha", out);
        t.root->color = Cor::PRETO;

        // Pintar filho esquerdo da raiz e seu filho esquerdo de vermelho
        if (t.root->left != t.nil && t.root->left->left != t.nil) {
            t.root->left->color       = Cor::VERMELHO;
            t.root->left->left->color = Cor::VERMELHO;
            auto [ok2, _2] = t.validate();
            check(!ok2, "RB: detecta duplo vermelho", out);
            t.root->left->color       = Cor::PRETO;
            t.root->left->left->color = Cor::PRETO;
        }
    }
}

// ---------- BST degenerada ----------

static void testarBSTDegenerada(std::ostream& out) {
    secao("BST - Degeneração e comparação com AVL", out);
    BST bst;
    for (int i = 1; i <= 2000; i++) bst.insert(i);
    check(bst.height() == 2000, "BST: altura 2000 com 1..2000", out);

    AVLTree avl;
    for (int i = 1; i <= 2000; i++) avl.insert(i);
    check(bst.height() > 100 * avl.height(),
          "BST: altura > 100x AVL (" + std::to_string(bst.height()) +
          " vs " + std::to_string(avl.height()) + ")", out);
}

// ---------- comparação AVL vs RB com random(800,77) ----------

static void testarComparacao(std::ostream& out) {
    secao("Comparação AVL vs RB com random(800,77) e remoções", out);
    auto ins = randomDataset(800, 77);
    auto rem = randomDataset(800, 78);

    AVLTree avl;
    RBTree  rb;

    // Inserções
    std::vector<bool> retAVL, retRB;
    for (int k : ins) {
        retAVL.push_back(avl.insert(k));
        retRB.push_back(rb.insert(k));
    }
    check(retAVL == retRB, "Comparação: mesmos retornos de insert", out);

    // Remoções das primeiras 400 de random(800,78)
    retAVL.clear(); retRB.clear();
    for (int i = 0; i < 400; i++) {
        retAVL.push_back(avl.remove(rem[i]));
        retRB.push_back(rb.remove(rem[i]));
    }
    check(retAVL == retRB, "Comparação: mesmos retornos de remove", out);
    check(avl.size() == rb.size(), "Comparação: mesmo tamanho final", out);
    check(avl.inorder() == rb.inorder(), "Comparação: mesmo percurso final", out);
    assertValid(avl, "AVL final comparação", out);
    assertValid(rb,  "RB final comparação",  out);
}

// ---------- testes de inserção em 4 ordens ----------

static void testarInsercoes4Ordens(std::ostream& out) {
    int n = 1000;

    // Crescente
    {
        AVLTree avl; RBTree rb;
        auto d = ascending(n);
        secao("AVL + RB - Inserção crescente n=1000", out);
        // Valida a cada inserção
        bool validOk = true;
        for (int k : d) {
            avl.insert(k);
            auto [v, _] = avl.validate();
            if (!v) { validOk = false; break; }
        }
        check(validOk, "AVL crescente: válida a cada inserção", out);
        testarInsercaoOrdens(avl, "AVL crescente", d, n, out);

        validOk = true;
        for (int k : d) {
            rb.insert(k);
            auto [v, _] = rb.validate();
            if (!v) { validOk = false; break; }
        }
        check(validOk, "RB crescente: válida a cada inserção", out);
        testarInsercaoOrdens(rb, "RB crescente", d, n, out);
    }
    // Decrescente
    {
        AVLTree avl; RBTree rb;
        auto d = descending(n);
        secao("AVL + RB - Inserção decrescente n=1000", out);
        for (int k : d) avl.insert(k);
        testarInsercaoOrdens(avl, "AVL decrescente", d, n, out);
        for (int k : d) rb.insert(k);
        testarInsercaoOrdens(rb, "RB decrescente", d, n, out);
    }
    // random(1000, 42)
    {
        AVLTree avl; RBTree rb;
        auto d = randomDataset(n, 42);
        secao("AVL + RB - Inserção random(1000,42)", out);
        for (int k : d) avl.insert(k);
        testarInsercaoOrdens(avl, "AVL rand42", d, n, out);
        for (int k : d) rb.insert(k);
        testarInsercaoOrdens(rb, "RB rand42", d, n, out);
    }
    // quase ordenada(1000, 42)
    {
        AVLTree avl; RBTree rb;
        auto d = nearlySorted(n, 42);
        secao("AVL + RB - Inserção quase ordenada(1000,42)", out);
        for (int k : d) avl.insert(k);
        testarInsercaoOrdens(avl, "AVL quasi42", d, n, out);
        for (int k : d) rb.insert(k);
        testarInsercaoOrdens(rb, "RB quasi42", d, n, out);
    }
}

// ---------- testes de saída de aceitação de impressão ----------

static void testarImpressao(std::ostream& out) {
    secao("Impressão – saída de aceitação (10 20 30 40 50 25)", out);

    // AVL
    {
        AVLTree avl;
        for (int v : {10, 20, 30, 40, 50, 25}) avl.insert(v);
        std::ostringstream oss;
        avl.printTree(oss);
        std::string s = oss.str();
        check(s.find("30 [fb=0]") != std::string::npos,      "AVL print: raiz 30 fb=0", out);
        check(s.find("├─E 20 [fb=0]") != std::string::npos,  "AVL print: ├─E 20", out);
        check(s.find("│  ├─E 10 [fb=0]") != std::string::npos,"AVL print: │  ├─E 10", out);
        check(s.find("│  └─D 25 [fb=0]") != std::string::npos,"AVL print: │  └─D 25", out);
        check(s.find("└─D 40 [fb=-1]") != std::string::npos,  "AVL print: └─D 40 fb=-1", out);
        check(s.find("   └─D 50 [fb=0]") != std::string::npos,"AVL print: └─D 50", out);
    }

    // RB
    {
        RBTree rb;
        for (int v : {10, 20, 30, 40, 50, 25}) rb.insert(v);
        std::ostringstream oss;
        rb.printTree(oss);
        std::string s = oss.str();
        check(s.find("20 (P)") != std::string::npos,          "RB print: raiz 20 P", out);
        check(s.find("├─E 10 (P)") != std::string::npos,      "RB print: ├─E 10 P", out);
        check(s.find("└─D 40 (V)") != std::string::npos,      "RB print: └─D 40 V", out);
        check(s.find("   ├─E 30 (P)") != std::string::npos,   "RB print: ├─E 30 P", out);
        check(s.find("   │  └─E 25 (V)") != std::string::npos,"RB print: │  └─E 25 V", out);
        check(s.find("   └─D 50 (P)") != std::string::npos,   "RB print: └─D 50 P", out);
    }
}

// ---------- remoção de 20 após inserção de 10 20 30 40 50 25 ----------

static void testarRemocao20(std::ostream& out) {
    secao("Remoção de 20 → altura 3 em ambas as árvores", out);
    // AVL
    {
        AVLTree avl;
        for (int v : {10, 20, 30, 40, 50, 25}) avl.insert(v);
        avl.remove(20);
        check(avl.height() == 3, "AVL: altura=3 após remover 20", out);
        check(avl.root && avl.root->key == 30, "AVL: raiz=30 após remover 20", out);
        auto* l = avl.root->left;
        auto* r = avl.root->right;
        check(l && l->key == 25 && l->left && l->left->key == 10,
              "AVL: esq=25 com filho esq=10", out);
        check(r && r->key == 40 && r->right && r->right->key == 50,
              "AVL: dir=40 com filho dir=50", out);
    }
    // RB
    {
        RBTree rb;
        for (int v : {10, 20, 30, 40, 50, 25}) rb.insert(v);
        rb.remove(20);
        check(rb.height() == 3, "RB: altura=3 após remover 20", out);
        assertValid(rb, "RB após remover 20", out);
    }
}

// ---------- 1..15 em ordem crescente ----------

static void testarInsercao1a15(std::ostream& out) {
    secao("Inserção 1..15 crescente (saída de aceitação)", out);
    {
        AVLTree avl;
        for (int i = 1; i <= 15; i++) avl.insert(i);
        check(avl.height() == 4,  "AVL 1..15: altura=4",    out);
        check(avl.rotations_ == 11, "AVL 1..15: 11 rotações", out);
    }
    {
        RBTree rb;
        for (int i = 1; i <= 15; i++) rb.insert(i);
        check(rb.height() == 6,       "RB 1..15: altura=6",        out);
        check(rb.rotations_   == 9,   "RB 1..15: 9 rotações",      out);
        check(rb.recolorings_ == 45,  "RB 1..15: 45 recolorações", out);
        // Nós 8, 12, 13, 15 são vermelhos
        check(rb.colorOf(8)  == "vermelho", "RB 1..15: 8 vermelho",  out);
        check(rb.colorOf(12) == "vermelho", "RB 1..15: 12 vermelho", out);
        check(rb.colorOf(13) == "vermelho", "RB 1..15: 13 vermelho", out);
        check(rb.colorOf(15) == "vermelho", "RB 1..15: 15 vermelho", out);
    }
}

// ---------- programa principal ----------

int main() {
    std::ofstream arqOut("results/test-output.txt");
    // Usa um ostream que escreve nos dois lugares
    class DualStream : public std::ostream {
    public:
        std::ostream& a;
        std::ostream& b;
        class DualBuf : public std::streambuf {
        public:
            std::ostream& a; std::ostream& b;
            DualBuf(std::ostream& a, std::ostream& b) : a(a), b(b) {}
            int overflow(int c) override {
                if (c != EOF) { a.put((char)c); b.put((char)c); }
                return c;
            }
        } buf;
        DualStream(std::ostream& a, std::ostream& b) : std::ostream(&buf), a(a), b(b), buf(a, b) {}
    } dual(std::cout, arqOut);

    dual << "=== Suite de Testes: AVL + Rubro-Negra ===\n";

    // --- Árvore vazia ---
    { AVLTree t; testarArvoreVazia(t, "AVL", dual); }
    { RBTree  t; testarArvoreVazia(t, "RB",  dual); }

    // --- Um elemento ---
    { AVLTree t; testarUmElemento(t, "AVL", dual); }
    { RBTree  t; testarUmElemento(t, "RB",  dual); }

    // --- Valores repetidos ---
    { AVLTree t; testarRepetidos(t, "AVL", dual); }
    { RBTree  t; testarRepetidos(t, "RB",  dual); }

    // --- Inserção em 4 ordens ---
    testarInsercoes4Ordens(dual);

    // --- Buscas ---
    {
        AVLTree avl; RBTree rb;
        auto d = ascending(1000);
        for (int k : d) { avl.insert(k); rb.insert(k); }
        testarBuscas(avl, 1000, "AVL", dual);
        testarBuscas(rb,  1000, "RB",  dual);
    }

    // --- Remoções (folha / 1 filho / 2 filhos) ---
    { AVLTree t; testarRemocoes(t, "AVL", dual); }
    { RBTree  t; testarRemocoes(t, "RB",  dual); }

    // --- Remover raiz repetidamente ---
    { AVLTree t; testarRemoverRaizRepetidamente(t, "AVL", dual); }
    { RBTree  t; testarRemoverRaizRepetidamente(t, "RB",  dual); }

    // --- Remover todas as chaves de random(500,3) ---
    { AVLTree t; testarRemoverTodasRandom500(t, "AVL", dual); }
    { RBTree  t; testarRemoverTodasRandom500(t, "RB",  dual); }

    // --- Remoção de inexistentes ---
    { AVLTree t; testarRemoverInexistentes(t, "AVL", dual); }
    { RBTree  t; testarRemoverInexistentes(t, "RB",  dual); }

    // --- 5000 operações aleatórias ---
    { AVLTree t; testar5000Ops(t, "AVL", dual); }
    { RBTree  t; testar5000Ops(t, "RB",  dual); }

    // --- Limite de altura 20000 ---
    { AVLTree t; testarLimiteAltura20000(t, "AVL", false, dual); }
    { RBTree  t; testarLimiteAltura20000(t, "RB",  true,  dual); }

    // --- Casos conhecidos AVL ---
    testarCasosAVL(dual);

    // --- Casos conhecidos RB ---
    testarCasosRB(dual);

    // --- Validadores detectam erros ---
    testarValidadoresErros(dual);

    // --- BST degenerada ---
    testarBSTDegenerada(dual);

    // --- Comparação AVL vs RB ---
    testarComparacao(dual);

    // --- Impressão (saída de aceitação) ---
    testarImpressao(dual);

    // --- Remoção de 20 ---
    testarRemocao20(dual);

    // --- 1..15 crescente ---
    testarInsercao1a15(dual);

    // --- Resumo ---
    dual << "\n=== Resumo ===\n";
    dual << "Total: " << total_testes << " | Aprovados: " << aprovados
         << " | Falhas: " << falhas_count << "\n";

    return (falhas_count > 0) ? 1 : 0;
}
