#pragma once
#include <glm/glm.hpp>

class Particula {
public:
    glm::vec3 posicao;    
    glm::vec3 velocidade; 
    glm::vec3 forca;      
    glm::vec3 normal;     
    float massa;
    bool estaFixa;        

    Particula(const glm::vec3& pos, float m = 1.0f)
        : posicao(pos), velocidade(0.0f), forca(0.0f),
          normal(0.0f, 0.0f, 1.0f), massa(m), estaFixa(false) {}

    void zerarForca() { forca = glm::vec3(0.0f); }
    void adicionarForca(const glm::vec3& f) { forca += f; }

    // Integração de Euler semi-implícito:
    //   v(t+dt) = v(t) + a(t)*dt     onde a = F/m (2ª Lei de Newton)
    //   x(t+dt) = x(t) + v(t+dt)*dt  usa o v já atualizado (mais estável)
    void integrar(float dt) {
        if (estaFixa) return;
        glm::vec3 aceleracao = forca / massa;
        velocidade += aceleracao * dt;
        posicao    += velocidade * dt;
    }
};