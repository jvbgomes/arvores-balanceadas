// Benchmark: compara BST, AVL e Rubro-Negra em diferentes conjuntos
// Gera results/benchmark.md, results/benchmark.json e imprime tabela no terminal
// Verifica automaticamente contra a tabela de valores esperados
#include "../src/avl.hpp"
#include "../src/rbtree.hpp"
#include "../src/bst.hpp"
#include "../src/datasets.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <vector>
#include <string>
#include <cmath>

// ---------- estrutura de resultado ----------

struct ResultRow {
    int    n;
    std::string dataset;
    // BST
    int    bst_h;
    double bst_prof;
    // AVL após inserção
    int    avl_h_ins;
    double avl_prof_ins;
    int    avl_rot_ins;
    // AVL após remoção
    int    avl_h_rem;
    int    avl_rot_rem;
    // RB após inserção
    int    rb_h_ins;
    double rb_prof_ins;
    int    rb_rot_ins;
    int    rb_rec_ins;
    // RB após remoção
    int    rb_h_rem;
    int    rb_rot_rem;
    int    rb_rec_rem;
};

// ---------- valores esperados ----------

struct Esperado {
    int n; std::string ds;
    int bst_h; double bst_prof;
    int avl_h_ins; double avl_prof_ins; int avl_rot_ins;
    int avl_h_rem; int avl_rot_rem;
    int rb_h_ins; double rb_prof_ins; int rb_rot_ins; int rb_rec_ins;
    int rb_h_rem; int rb_rot_rem; int rb_rec_rem;
};

static const std::vector<Esperado> ESPERADOS = {
    {1000,"crescente",   1000,500.50, 10,8.99,990,  10,75,  17,9.41,983,4909,   10,101,1118},
    {1000,"decrescente", 1000,500.50, 10,8.99,990,  10,95,  17,9.41,983,4909,   10,110,1130},
    {1000,"quase ord",   161, 42.99,  12,9.15,1108, 11,145, 17,9.63,1182,4334,  11,165,1086},
    {1000,"aleatoria",   24,  11.72,  12,9.22,668,  11,190, 12,9.25,581,2319,   12,176,773},
    {10000,"crescente",  10000,5000.50,14,12.36,9986, 14,868, 24,12.89,9976,49877, 14,980,11323},
    {10000,"decrescente",10000,5000.50,14,12.36,9986, 14,836, 24,12.89,9976,49877, 14,929,11355},
    {10000,"quase ord",  657, 127.82, 15,12.55,12109,15,1577,23,12.96,12516,43774,15,1576,10881},
    {10000,"aleatoria",  31,  16.47,  16,12.60,6930, 15,1900,16,12.62,5841,23125, 15,1774,7671},
    {100000,"crescente", 100000,50000.50,17,15.69,99983,17,8420,31,16.10,99969,499846,18,9375,113807},
    {100000,"decrescente",100000,50000.50,17,15.69,99983,17,8273,31,16.10,99969,499846,18,9133,113513},
    {100000,"quase ord", 3319,182.79,  20,15.98,120065,19,16368,30,16.53,125816,438708,20,15566,109520},
    {100000,"aleatoria", 40,  20.61,  20,15.92,69742, 19,18848,20,15.96,58191,231855,19,18120,77444},
};

// Compara doubles formatados com 2 casas decimais (como a tabela do enunciado)
static bool dblEq(double a, double b) {
    std::ostringstream oa, ob;
    oa << std::fixed << std::setprecision(2) << a;
    ob << std::fixed << std::setprecision(2) << b;
    return oa.str() == ob.str();
}

// ---------- função principal de benchmark ----------

int main() {
    std::vector<ResultRow> resultados;

    std::vector<int> ns = {1000, 10000, 100000};
    // Para BST com n=100000 em ordem crescente/decrescente é muito lento (O(n²))
    // A especificação diz "faça uma única execução dela"
    // Nós medimos apenas para os casos de referência

    for (int n : ns) {
        std::vector<std::pair<std::string,std::vector<int>>> datasets;
        datasets.push_back({"crescente",   ascending(n)});
        datasets.push_back({"decrescente", descending(n)});
        datasets.push_back({"quase ord",   nearlySorted(n, 42)});
        datasets.push_back({"aleatoria",   randomDataset(n, 42)});

        // Chaves de remoção: primeiras n/2 de random(n, 7)
        auto remKeys = randomDataset(n, 7);

        for (auto& [dsNome, dados] : datasets) {
            ResultRow r;
            r.n       = n;
            r.dataset = dsNome;

            // --- BST ---
            // Para n=100000 com dados ordenados: O(n²), mas fazemos mesmo assim
            // (é a "única execução" mencionada na especificação)
            {
                BST bst;
                for (int k : dados) bst.insert(k);
                r.bst_h    = bst.height();
                r.bst_prof = bst.averageDepth();
            }

            // --- AVL ---
            {
                AVLTree avl;
                for (int k : dados) avl.insert(k);
                auto [ok, _] = avl.validate();
                (void)ok;
                r.avl_h_ins    = avl.height();
                r.avl_prof_ins = avl.averageDepth();
                r.avl_rot_ins  = avl.rotations_;

                avl.resetStats();
                int remCount = 0;
                for (int i = 0; i < n / 2; i++) {
                    avl.remove(remKeys[i]);
                    remCount++;
                }
                (void)remCount;
                avl.validate();
                r.avl_h_rem   = avl.height();
                r.avl_rot_rem = avl.rotations_;
            }

            // --- RB ---
            {
                RBTree rb;
                for (int k : dados) rb.insert(k);
                rb.validate();
                r.rb_h_ins    = rb.height();
                r.rb_prof_ins = rb.averageDepth();
                r.rb_rot_ins  = rb.rotations_;
                r.rb_rec_ins  = rb.recolorings_;

                rb.resetStats();
                for (int i = 0; i < n / 2; i++) {
                    rb.remove(remKeys[i]);
                }
                rb.validate();
                r.rb_h_rem    = rb.height();
                r.rb_rot_rem  = rb.rotations_;
                r.rb_rec_rem  = rb.recolorings_;
            }

            resultados.push_back(r);
            std::cout << "  [n=" << n << " " << dsNome << "] BST h=" << r.bst_h
                      << " AVL h=" << r.avl_h_ins << " rot=" << r.avl_rot_ins
                      << " RB h=" << r.rb_h_ins << " rot=" << r.rb_rot_ins
                      << " rec=" << r.rb_rec_ins << "\n";
        }
    }

    // ---------- verificação automática contra esperados ----------
    std::cout << "\n=== Verificação contra tabela de valores esperados ===\n";
    std::vector<std::string> divergencias;
    int divergCount = 0;

    for (auto& r : resultados) {
        // Encontra esperado correspondente
        const Esperado* esp = nullptr;
        for (auto& e : ESPERADOS)
            if (e.n == r.n && e.ds == r.dataset) { esp = &e; break; }
        if (!esp) continue;

        auto check = [&](const std::string& campo, auto medido, auto esperado) {
            bool ok;
            if constexpr (std::is_same_v<decltype(medido), double>)
                ok = dblEq(medido, (double)esperado);
            else
                ok = (medido == esperado);
            if (!ok) {
                std::string msg = "n=" + std::to_string(r.n) + " " + r.dataset +
                    ": " + campo + " medido=" + std::to_string(medido) +
                    " esperado=" + std::to_string(esperado);
                divergencias.push_back(msg);
                divergCount++;
            }
        };

        check("BST_h",        r.bst_h,       esp->bst_h);
        check("BST_prof",     r.bst_prof,    esp->bst_prof);
        check("AVL_h_ins",    r.avl_h_ins,   esp->avl_h_ins);
        check("AVL_prof_ins", r.avl_prof_ins,esp->avl_prof_ins);
        check("AVL_rot_ins",  r.avl_rot_ins, esp->avl_rot_ins);
        check("AVL_h_rem",    r.avl_h_rem,   esp->avl_h_rem);
        check("AVL_rot_rem",  r.avl_rot_rem, esp->avl_rot_rem);
        check("RB_h_ins",     r.rb_h_ins,    esp->rb_h_ins);
        check("RB_prof_ins",  r.rb_prof_ins, esp->rb_prof_ins);
        check("RB_rot_ins",   r.rb_rot_ins,  esp->rb_rot_ins);
        check("RB_rec_ins",   r.rb_rec_ins,  esp->rb_rec_ins);
        check("RB_h_rem",     r.rb_h_rem,    esp->rb_h_rem);
        check("RB_rot_rem",   r.rb_rot_rem,  esp->rb_rot_rem);
        check("RB_rec_rem",   r.rb_rec_rem,  esp->rb_rec_rem);
    }

    if (divergencias.empty()) {
        std::cout << "✔ Todos os valores coincidem com a tabela esperada.\n";
    } else {
        std::cout << "✖ " << divergCount << " divergência(s) encontrada(s):\n";
        for (auto& d : divergencias) std::cout << "  - " << d << "\n";
    }

    // ---------- gera benchmark.md ----------
    {
        std::ofstream md("results/benchmark.md");
        md << "# Benchmark de Árvores Balanceadas\n\n";
        md << "Autor: João Victor — IMD/UFRN  \n";
        md << "Semente de remoção: random(n, 7), primeiros n/2 elementos\n\n";

        md << "## Resultados\n\n";
        md << "| n | Conjunto | BST h | BST prof | AVL h | AVL prof | AVL rot | "
              "AVL h pós | AVL rot pós | RB h | RB prof | RB rot | RB rec | "
              "RB h pós | RB rot pós | RB rec pós |\n";
        md << "|---|---------|------|---------|------|---------|---------|"
              "---------|------------|------|---------|--------|--------|"
              "---------|-----------|----------|\n";
        for (auto& r : resultados) {
            md << "| " << r.n << " | " << r.dataset
               << " | " << r.bst_h << " | " << std::fixed << std::setprecision(2) << r.bst_prof
               << " | " << r.avl_h_ins << " | " << r.avl_prof_ins << " | " << r.avl_rot_ins
               << " | " << r.avl_h_rem << " | " << r.avl_rot_rem
               << " | " << r.rb_h_ins << " | " << r.rb_prof_ins << " | " << r.rb_rot_ins
               << " | " << r.rb_rec_ins
               << " | " << r.rb_h_rem << " | " << r.rb_rot_rem << " | " << r.rb_rec_rem
               << " |\n";
        }

        if (!divergencias.empty()) {
            md << "\n## Divergências em relação à tabela esperada\n\n";
            for (auto& d : divergencias) md << "- " << d << "\n";
        } else {
            md << "\n## Verificação\n\nTodos os valores coincidem com a tabela esperada.\n";
        }
    }

    // ---------- gera benchmark.json ----------
    {
        std::ofstream js("results/benchmark.json");
        js << "[\n";
        for (int i = 0; i < (int)resultados.size(); i++) {
            auto& r = resultados[i];
            js << "  {\n";
            js << "    \"n\": "       << r.n       << ",\n";
            js << "    \"dataset\": \"" << r.dataset << "\",\n";
            js << "    \"bst\": { \"h\": " << r.bst_h << ", \"prof\": "
               << std::fixed << std::setprecision(2) << r.bst_prof << " },\n";
            js << "    \"avl_ins\": { \"h\": " << r.avl_h_ins << ", \"prof\": "
               << r.avl_prof_ins << ", \"rot\": " << r.avl_rot_ins << " },\n";
            js << "    \"avl_rem\": { \"h\": " << r.avl_h_rem << ", \"rot\": "
               << r.avl_rot_rem << " },\n";
            js << "    \"rb_ins\": { \"h\": " << r.rb_h_ins << ", \"prof\": "
               << r.rb_prof_ins << ", \"rot\": " << r.rb_rot_ins
               << ", \"rec\": " << r.rb_rec_ins << " },\n";
            js << "    \"rb_rem\": { \"h\": " << r.rb_h_rem << ", \"rot\": "
               << r.rb_rot_rem << ", \"rec\": " << r.rb_rec_rem << " }\n";
            js << "  }" << (i + 1 < (int)resultados.size() ? "," : "") << "\n";
        }
        js << "]\n";
    }

    std::cout << "\nResultados salvos em results/benchmark.md e results/benchmark.json\n";
    return divergencias.empty() ? 0 : 1;
}
