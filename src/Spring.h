#pragma once
#include "Particle.h"
#include <glm/glm.hpp>

// Tipos de mola 
enum class TipoMola { STRUCTURAL, SHEAR, BEND };

class Mola {
public:
    Particula* p1;
    Particula* p2;
    float comprimentoRepouso; 
    float ks;                 // rigidez da mola (constante elástica de Hooke)
    float kd;                 // amortecimento (dissipa energia, evita oscilar para sempre)
    TipoMola tipo;

    Mola(Particula* a, Particula* b, float rigidez, float amortecimento, TipoMola t)
        : p1(a), p2(b), ks(rigidez), kd(amortecimento), tipo(t)
    {
        comprimentoRepouso = glm::length(p1->posicao - p2->posicao);
    }
    void aplicarForca() {
        glm::vec3 delta = p1->posicao - p2->posicao;
        float comprimentoAtual = glm::length(delta);

        if (comprimentoAtual < 1e-6f) return; 

        glm::vec3 direcao = delta / comprimentoAtual; 

        float deformacao     = comprimentoAtual - comprimentoRepouso;
        float velocRelativa  = glm::dot(p1->velocidade - p2->velocidade, direcao);
        float magnitudeForca = -(ks * deformacao + kd * velocRelativa);

        glm::vec3 forca = magnitudeForca * direcao;

        p1->adicionarForca(forca);   // ação
        p2->adicionarForca(-forca);  // reação 
    }
};