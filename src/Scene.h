#pragma once
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Cenario {

    inline std::vector<float> construirChao(float y, float tamanho, float cx, float cz)
    {
        float metade = tamanho * 0.5f;
        glm::vec3 normal(0, 1, 0);

        glm::vec3 p0(cx - metade, y, cz - metade);
        glm::vec3 p1(cx + metade, y, cz - metade);
        glm::vec3 p2(cx + metade, y, cz + metade);
        glm::vec3 p3(cx - metade, y, cz + metade);

        auto empurrar = [](std::vector<float>& d, const glm::vec3& p, const glm::vec3& n) {
            d.push_back(p.x); d.push_back(p.y); d.push_back(p.z);
            d.push_back(n.x); d.push_back(n.y); d.push_back(n.z);
        };

        std::vector<float> dados;
        empurrar(dados, p0, normal); empurrar(dados, p1, normal); empurrar(dados, p2, normal);
        empurrar(dados, p0, normal); empurrar(dados, p2, normal); empurrar(dados, p3, normal);
        return dados;
    }
    inline std::vector<float> construirHaste(float x0, float x1, float y, float z,
                                              float raio, int segmentos)
    {
        std::vector<float> dados;

        auto empurrar = [&dados](const glm::vec3& p, const glm::vec3& n) {
            dados.push_back(p.x); dados.push_back(p.y); dados.push_back(p.z);
            dados.push_back(n.x); dados.push_back(n.y); dados.push_back(n.z);
        };

        for (int i = 0; i < segmentos; ++i) {
            float angulo0 = (float)i       / segmentos * glm::two_pi<float>();
            float angulo1 = (float)(i + 1) / segmentos * glm::two_pi<float>();

            glm::vec3 n0(0.0f, std::cos(angulo0), std::sin(angulo0));
            glm::vec3 n1(0.0f, std::cos(angulo1), std::sin(angulo1));
            glm::vec3 a0(x0, y + raio * n0.y, z + raio * n0.z);
            glm::vec3 a1(x0, y + raio * n1.y, z + raio * n1.z);
            glm::vec3 b0(x1, y + raio * n0.y, z + raio * n0.z);
            glm::vec3 b1(x1, y + raio * n1.y, z + raio * n1.z);

            // Triângulo 1: a0, b0, b1
            empurrar(a0, n0); empurrar(b0, n0); empurrar(b1, n1);
            // Triângulo 2: a0, b1, a1
            empurrar(a0, n0); empurrar(b1, n1); empurrar(a1, n1);
        }

        return dados;
    }

} 