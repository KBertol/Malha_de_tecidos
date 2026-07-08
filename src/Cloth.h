#pragma once
#include "Particle.h"
#include "Spring.h"
#include <vector>
#include <glm/glm.hpp>

class Tecido {
public:
    int largura, altura;  // número de partículas em cada dimensão da grade
    float espacamento;    // distância entre partículas vizinhas no repouso

    bool colisaoChaoAtiva = true;
    float alturaChao = -12.0f;

    std::vector<Particula> particulas;
    std::vector<Mola> molas;
    std::vector<unsigned int> indices; 

    Tecido(int largura, int altura, float espacamento, float massaParticula);

    Particula& em(int x, int y) { return particulas[y * largura + x]; }

    void criarMolas(float ksEstrutural, float kdEstrutural,
                    float ksShear,      float kdShear,
                    float ksBend,       float kdBend);

    void construirMalha();
    void aplicarForcasExternas(const glm::vec3& gravidade, const glm::vec3& vento);
    void atualizar(float dt, const glm::vec3& gravidade, const glm::vec3& vento, int numSubPassos);
    void calcularNormais();
    void resolverColisaoChao(float alturaChao, float restituicao = 0.3f);
    void fixarParticula(int x, int y, bool fixa);
    std::vector<float> obterDadosVertices() const;
};