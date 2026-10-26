# Arvores Balanceadas - Amazon Engineering Challenge

**Autor:** Joao Victor  
**Disciplina:** IMD0039 - Estruturas de Dados Basicas II  
**Instituicao:** IMD/UFRN  
**Trabalho:** Unidade II - Amazon Engineering Challenge

## Requisitos

- g++ com suporte a C++17 (GCC 7+ ou Clang 5+)
- make

Em Fedora:

```bash
sudo dnf install gcc-c++ make
```

## Como compilar e executar

```bash
# Compilar tudo
make

# Executar o programa interativo
make run

# Rodar testes
make test

# Rodar benchmark
make bench

# Rodar demo roteirizado
make demo

# Limpar binarios e resultados
make clean
```

O binario Linux x86-64 pre-compilado esta em `bin/arvores-balanceadas`.

## Funcionalidades

### Programa interativo (`make run`)

Ao iniciar, o usuario escolhe entre AVL ou Rubro-Negra. Em seguida, um menu oferece:

1. Inserir identificador
2. Remover identificador
3. Buscar identificador
4. Percurso em ordem crescente
5. Imprimir estrutura da arvore
6. Inserir varios (separados por espaco ou virgula)
7. Carregar N identificadores aleatorios
8. Verificar propriedades de balanceamento
9. Estatisticas (altura, rotacoes, recoloracoes)
10. Trocar de estrutura (comeca com arvore vazia)
0. Sair

Exemplo de uso com insercao multipla e impressao:

```
Opcao: 6
Identificadores: 10 20 30 40 50 25
Inseridos: 6. Repetidos ignorados: 0.

Opcao: 5
30 [fb=0]
+--E 20 [fb=0]
|  +--E 10 [fb=0]
|  +--D 25 [fb=0]
+--D 40 [fb=-1]
   +--D 50 [fb=0]
```

### Testes (`make test`)

Executa 182 testes automatizados cobrindo:

- Arvore vazia (busca, remocao, percurso, altura, tamanho)
- Arvore com um elemento
- Insercao de valores repetidos
- Insercao em quatro ordens: crescente, decrescente, aleatoria, quase ordenada
- Buscas de existentes e inexistentes
- Remocao de folha, no com um filho, no com dois filhos
- Remover a raiz repetidamente ate esvaziar
- 5000 operacoes aleatorias comparadas com std::set
- Limite de altura com 20.000 chaves
- Casos conhecidos de AVL (LL, RR, LR, RL)
- Casos conhecidos de Rubro-Negra
- Validadores detectando erros injetados
- BST degenerada
- Comparacao AVL vs Rubro-Negra

Resultado salvo em `results/test-output.txt`.

### Benchmark (`make bench`)

Insere n = 1.000, 10.000 e 100.000 chaves em quatro ordens (crescente, decrescente, quase ordenada e aleatoria) nas tres estruturas (BST, AVL, Rubro-Negra), mede altura e profundidade media, e remove as primeiras n/2 chaves de random(n, 7). Verifica automaticamente os resultados contra a tabela de valores esperados.

Resultados salvos em `results/benchmark.md` e `results/benchmark.json`.

### Demo (`make demo`)

Executa um roteiro comentado de 7 passos mostrando ambas as arvores lado a lado. Saida em `results/demo-output.txt`.

## Tabela de decisoes de implementacao

| Decisao | Escolha | Justificativa |
|---------|---------|---------------|
| Valor repetido no insert | Retorna false, arvore inalterada | Mantem consistencia; BST de busca nao aceita duplicatas |
| Busca de inexistente | Retorna false | Nao ha dado para retornar; false e o valor sentinela natural |
| Remocao de inexistente | Retorna false, arvore inalterada | Operacao segura; sem efeito colateral |
| Arvore vazia | height()=0, size=0, inorder()={} | Definicao da especificacao |
| Sucessor em ordem (AVL) | Menor da subarvore direita | CLRS; preserva invariantes BST com rebalanceamento no caminho de volta |
| Altura em niveis | Vazio=0, folha=1, raiz=max(h_esq,h_dir)+1 | Convencao especificada; facil de armazenar em cada no |
| Contagem de rotacoes | Cada LEFT-ROTATE ou RIGHT-ROTATE elementar conta 1 | Rotacao dupla = 2; alinhado com CLRS |
| Contagem de recoloracoes (RB) | paint() conta somente quando a cor muda; ignora NIL; y.color=z.color nao conta | Especificacao explicita; evita contar atribuicoes no-op |
| validate() AVL | BST, alturas armazenadas, \|fb\|<=1, tamanho | Cobre todas as invariantes da AVL |
| validate() RB | BST, raiz preta, NIL preto, sem duplo vermelho, altura negra uniforme, ponteiros de pai, tamanho | Cobre as 5 propriedades CLRS |

## Descricao dos testes

Os 182 testes estao agrupados por categoria:

- **Arvore vazia (14 testes):** verifica comportamento inicial em ambas as arvores
- **Um elemento (22 testes):** insercao, busca, remocao e validade
- **Repetidos (10 testes):** insert retorna false e nada muda
- **Insercao em 4 ordens (32 testes):** percurso, altura e validade para cada ordem/arvore
- **Buscas (10 testes):** pares existentes, impares inexistentes, bordas (0, -5, enorme)
- **Remocoes (12 testes):** folha, 1 filho, 2 filhos com random(63,11)
- **Remover raiz (2 testes):** esvazia inserindo 1..15 e removendo sempre a raiz
- **random(500) (4 testes):** insere random(500,3) e remove na ordem random(500,4)
- **Inexistentes (10 testes):** remove 0, 51, -1 de arvore com 1..50
- **5000 ops (8 testes):** compara com std::set usando mulberry32(2024)
- **Altura 20000 (2 testes):** limites teoricos AVL e RB
- **Casos AVL (12 testes):** LL, RR, LR, RL com contagem exata
- **Casos RB (13 testes):** 1,2,3; caso 1; 3,1,2; colorOf
- **Validadores (3 testes):** deteccao de corrupcao manual
- **BST degenerada (2 testes):** altura n e comparacao com AVL
- **Comparacao (6 testes):** mesmos retornos, percurso e tamanho
- **Impressao (12 testes):** saida de aceitacao para ambas as arvores
- **Remocao de 20 (6 testes):** altura e estrutura apos remover 20
- **1..15 crescente (9 testes):** altura, rotacoes, recoloracoes, cores especificas

## Tabela de resultados com 100.000 chaves

| Conjunto | BST h | BST prof | AVL h ins | AVL rot | AVL h rem | AVL rot pos | RB h ins | RB rot | RB rec | RB h rem | RB rot pos | RB rec pos |
|----------|-------|----------|-----------|---------|-----------|-------------|----------|--------|--------|----------|------------|------------|
| Crescente | 100000 | 50000.50 | 17 | 99983 | 17 | 8420 | 31 | 99969 | 499846 | 18 | 9375 | 113807 |
| Decrescente | 100000 | 50000.50 | 17 | 99983 | 17 | 8273 | 31 | 99969 | 499846 | 18 | 9133 | 113513 |
| Quase ordenada | 3319 | 182.79 | 20 | 120065 | 19 | 16368 | 30 | 125816 | 438708 | 20 | 15566 | 109520 |
| Aleatoria | 40 | 20.61 | 20 | 69742 | 19 | 18848 | 20 | 58191 | 231855 | 19 | 18120 | 77444 |

## Referencias

- CORMEN, T. H.; LEISERSON, C. E.; RIVEST, R. L.; STEIN, C. *Introduction to Algorithms*. 4. ed. MIT Press, 2022.
- ADELSON-VELSKY, G. M.; LANDIS, E. M. An algorithm for the organization of information. *Soviet Mathematics Doklady*, v. 3, p. 1259-1263, 1962.
- BAYER, R. Symmetric binary B-Trees: data structure and maintenance algorithms. *Acta Informatica*, v. 1, n. 4, p. 290-306, 1972.
- GUIBAS, L. J.; SEDGEWICK, R. A dichromatic framework for balanced trees. *19th Annual Symposium on Foundations of Computer Science*, p. 8-21, 1978.
