#include "Cloth.h"

Tecido::Tecido(int larg, int alt, float spc, float massaParticula)
    : largura(larg), altura(alt), espacamento(spc)
{
    particulas.reserve(largura * altura);

    for (int y = 0; y < altura; ++y)
        for (int x = 0; x < largura; ++x)
            particulas.emplace_back(
                glm::vec3(x * espacamento, -y * espacamento, 0.0f),
                massaParticula
            );
}

void Tecido::criarMolas(float ksEstrutural, float kdEstrutural,
                         float ksShear,      float kdShear,
                         float ksBend,       float kdBend)
{
    auto indice = [this](int x, int y) { return y * largura + x; };

    for (int y = 0; y < altura; ++y) {
        for (int x = 0; x < largura; ++x) {

            // Molas ESTRUTURAIS: ligam vizinhos diretos (horizontal e vertical)
            if (x + 1 < largura)
                molas.emplace_back(&particulas[indice(x,y)], &particulas[indice(x+1,y)],
                                    ksEstrutural, kdEstrutural, TipoMola::STRUCTURAL);
            if (y + 1 < altura)
                molas.emplace_back(&particulas[indice(x,y)], &particulas[indice(x,y+1)],
                                    ksEstrutural, kdEstrutural, TipoMola::STRUCTURAL);

            // Molas SHEAR: ligam as diagonais de cada quad
            if (x + 1 < largura && y + 1 < altura) {
                molas.emplace_back(&particulas[indice(x,y)],   &particulas[indice(x+1,y+1)],
                                    ksShear, kdShear, TipoMola::SHEAR);
                molas.emplace_back(&particulas[indice(x+1,y)], &particulas[indice(x,y+1)],
                                    ksShear, kdShear, TipoMola::SHEAR);
            }

            // Molas BEND: ligam partículas a 2 posições de distância
            if (x + 2 < largura)
                molas.emplace_back(&particulas[indice(x,y)], &particulas[indice(x+2,y)],
                                    ksBend, kdBend, TipoMola::BEND);
            if (y + 2 < altura)
                molas.emplace_back(&particulas[indice(x,y)], &particulas[indice(x,y+2)],
                                    ksBend, kdBend, TipoMola::BEND);
        }
    }
}

void Tecido::construirMalha()
{
    indices.clear();

    // Para cada quad da grade, cria 2 triângulos.
    for (int y = 0; y < altura - 1; ++y) {
        for (int x = 0; x < largura - 1; ++x) {
            unsigned int topoEsq   = y * largura + x;
            unsigned int topoDir   = topoEsq + 1;
            unsigned int baixoEsq  = (y + 1) * largura + x;
            unsigned int baixoDir  = baixoEsq + 1;

            indices.push_back(topoEsq);
            indices.push_back(baixoEsq);
            indices.push_back(topoDir);

            indices.push_back(topoDir);
            indices.push_back(baixoEsq);
            indices.push_back(baixoDir);
        }
    }
}

void Tecido::aplicarForcasExternas(const glm::vec3& gravidade, const glm::vec3& vento)
{
    for (auto& p : particulas) {
        p.zerarForca();
        p.adicionarForca(gravidade * p.massa); // F = m*g (2ª Lei de Newton)
        p.adicionarForca(vento);              
    }
}

void Tecido::resolverColisaoChao(float chao, float restituicao)
{
    for (auto& p : particulas) {
        if (p.estaFixa) continue;

        if (p.posicao.y < chao) {
            p.posicao.y = chao;

            if (p.velocidade.y < 0.0f)
                p.velocidade.y = -p.velocidade.y * restituicao;

            p.velocidade.x *= 0.92f;
            p.velocidade.z *= 0.92f;
        }
    }
}

void Tecido::atualizar(float dt, const glm::vec3& gravidade, const glm::vec3& vento, int numSubPassos)
{

    float subDt = dt / static_cast<float>(numSubPassos);

    for (int passo = 0; passo < numSubPassos; ++passo) {
        aplicarForcasExternas(gravidade, vento);

        for (auto& m : molas)
            m.aplicarForca();

        for (auto& p : particulas)
            p.integrar(subDt);

        if (colisaoChaoAtiva)
            resolverColisaoChao(alturaChao);
    }

    calcularNormais();
}

void Tecido::calcularNormais()
{
    for (auto& p : particulas)
        p.normal = glm::vec3(0.0f);

    for (size_t i = 0; i < indices.size(); i += 3) {
        const glm::vec3& a = particulas[indices[i]].posicao;
        const glm::vec3& b = particulas[indices[i+1]].posicao;
        const glm::vec3& c = particulas[indices[i+2]].posicao;

        glm::vec3 normalFace = glm::cross(b - a, c - a);

        particulas[indices[i]].normal   += normalFace;
        particulas[indices[i+1]].normal += normalFace;
        particulas[indices[i+2]].normal += normalFace;
    }

    for (auto& p : particulas)
        if (glm::length(p.normal) > 1e-6f)
            p.normal = glm::normalize(p.normal);
}

void Tecido::fixarParticula(int x, int y, bool fixa)
{
    em(x, y).estaFixa = fixa;
}

std::vector<float> Tecido::obterDadosVertices() const
{
    std::vector<float> dados;
    dados.reserve(particulas.size() * 6);

    for (const auto& p : particulas) {
        dados.push_back(p.posicao.x); dados.push_back(p.posicao.y); dados.push_back(p.posicao.z);
        dados.push_back(p.normal.x);  dados.push_back(p.normal.y);  dados.push_back(p.normal.z);
    }

    return dados;
}