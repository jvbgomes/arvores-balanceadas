// Gerador pseudoaleatório mulberry32 e funções de conjuntos de dados
// Especificação exata conforme enunciado do trabalho
#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

// Gerador pseudoaleatório mulberry32 com semente de 32 bits
struct Mulberry32 {
    uint32_t a;

    explicit Mulberry32(uint32_t semente) : a(semente) {}

    // Retorna double no intervalo [0, 1)
    double next() {
        a += 0x6d2b79f5u;
        uint32_t t = a;
        t = (t ^ (t >> 15)) * (t | 1u);
        t ^= t + ((t ^ (t >> 7)) * (t | 61u));
        return (double)(t ^ (t >> 14)) / 4294967296.0;
    }
};

// Retorna vetor 1..n
inline std::vector<int> ascending(int n) {
    std::vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = i + 1;
    return v;
}

// Retorna vetor n..1
inline std::vector<int> descending(int n) {
    std::vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = n - i;
    return v;
}

// Embaralhamento de Fisher-Yates usando mulberry32
inline void shuffle(std::vector<int>& arr, Mulberry32& rng) {
    int n = (int)arr.size();
    for (int i = n - 1; i >= 1; i--) {
        int j = (int)std::floor(rng.next() * (i + 1));
        std::swap(arr[i], arr[j]);
    }
}

// Embaralhamento de 1..n com semente dada
inline std::vector<int> randomDataset(int n, uint32_t semente) {
    auto v = ascending(n);
    Mulberry32 rng(semente);
    shuffle(v, rng);
    return v;
}

// Quase ordenada: ascending + floor(n*0.05) trocas aleatórias
inline std::vector<int> nearlySorted(int n, uint32_t semente) {
    auto v = ascending(n);
    Mulberry32 rng(semente);
    int trocas = (int)std::floor((double)n * 0.05);
    for (int k = 0; k < trocas; k++) {
        int i = (int)std::floor(rng.next() * n);
        int j = (int)std::floor(rng.next() * n);
        std::swap(v[i], v[j]);
    }
    return v;
}
