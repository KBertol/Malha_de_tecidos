#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Cloth.h"
#include "Shader.h"
#include "Scene.h"

#include <iostream>
#include <algorithm>
#include <cmath>

// ---------- Configurações da janela ----------
const unsigned int LARGURA_TELA = 1024;
const unsigned int ALTURA_TELA  = 768;

// ---------- Câmera orbital ----------
float distanciaCamera = 40.0f;
float yawCamera       = -60.0f;  // graus, rotação horizontal
float pitchCamera     = 18.0f;   // graus, rotação vertical
glm::vec3 alvoCamera(0.0f, -4.0f, 0.0f); // ponto que a câmera olha

// ---------- Estado da simulação ----------
bool modoArame    = false; // V = alterna wireframe (mostra a malha de triângulos)
bool ventoAtivo   = true;  // F = liga/desliga o vento
float forcaVento  = 7.0f;

// Controla a direção do vento
float mousePosX = 0.0f; 
float mousePosZ = 0.0f; 

// ---------- Geometria da cena ----------
const float ALTURA_HASTE  = 6.0f;   
const float METADE_HASTE  = 8.0f;   
const float ALTURA_CHAO   = -9.0f; 

void callback_redimensionar(GLFWwindow* janela, int largura, int altura) {
    glViewport(0, 0, largura, altura);
}

void callback_mouse(GLFWwindow* janela, double posX, double posY) {
    mousePosX = (float)(posX / LARGURA_TELA * 2.0 - 1.0);
    mousePosZ = (float)(posY / ALTURA_TELA  * 2.0 - 1.0);
}

void processarEntrada(GLFWwindow* janela) {
    if (glfwGetKey(janela, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(janela, true);

    float velocidadeCamera = 1.0f;
    if (glfwGetKey(janela, GLFW_KEY_LEFT)  == GLFW_PRESS) yawCamera   -= velocidadeCamera;
    if (glfwGetKey(janela, GLFW_KEY_RIGHT) == GLFW_PRESS) yawCamera   += velocidadeCamera;
    if (glfwGetKey(janela, GLFW_KEY_UP)    == GLFW_PRESS) pitchCamera += velocidadeCamera;
    if (glfwGetKey(janela, GLFW_KEY_DOWN)  == GLFW_PRESS) pitchCamera -= velocidadeCamera;
    if (glfwGetKey(janela, GLFW_KEY_W)     == GLFW_PRESS) distanciaCamera -= 0.5f;
    if (glfwGetKey(janela, GLFW_KEY_S)     == GLFW_PRESS) distanciaCamera += 0.5f;

    pitchCamera     = std::clamp(pitchCamera,     -89.0f, 89.0f);
    distanciaCamera = std::clamp(distanciaCamera,   5.0f, 100.0f);
}

void processarAlternancia(GLFWwindow* janela) {
    static bool vAnterior = false, fAnterior = false;

    bool vAgora = glfwGetKey(janela, GLFW_KEY_V) == GLFW_PRESS;
    if (vAgora && !vAnterior) modoArame = !modoArame;
    vAnterior = vAgora;

    bool fAgora = glfwGetKey(janela, GLFW_KEY_F) == GLFW_PRESS;
    if (fAgora && !fAnterior) ventoAtivo = !ventoAtivo;
    fAnterior = fAgora;
}

// Cria um VAO + VBO para geometria estática (chão e haste).
unsigned int criarVAOEstatico(const std::vector<float>& vertices, unsigned int& vboSaida) {
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    vboSaida = vbo;
    return vao;
}

int main() {
    glfwSetErrorCallback([](int codigo, const char* descricao) {
        std::cerr << "Erro GLFW [" << codigo << "]: " << descricao << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar o GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* janela = glfwCreateWindow(LARGURA_TELA, ALTURA_TELA,
        "Simulacao de Tecido - Varal/Cortina", nullptr, nullptr);
    if (!janela) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(janela);
    glfwSetFramebufferSizeCallback(janela, callback_redimensionar);
    glfwSetCursorPosCallback(janela, callback_mouse);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o glad" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST); 

    // ---------- Cria o tecido ----------
    Tecido tecido(30, 30, 0.5f, 0.1f);
    tecido.criarMolas(200.0f, 8.0f,   // estrutural: ks, kd
                      150.0f, 6.0f,   // shear: ks, kd
                      100.0f, 4.0f);  // bend: ks, kd
    tecido.construirMalha();

    float larguraMundo = (tecido.largura - 1) * tecido.espacamento;
    for (auto& p : tecido.particulas)
        p.posicao += glm::vec3(-larguraMundo * 0.5f, ALTURA_HASTE, 0.0f);

    for (int x = 0; x < tecido.largura; x += 4)
        tecido.fixarParticula(x, 0, true);
    tecido.fixarParticula(tecido.largura - 1, 0, true); // garante a última coluna

    tecido.alturaChao = ALTURA_CHAO;
    tecido.colisaoChaoAtiva = true;

    // ---------- Shader (compartilhado por tecido, chão e haste) ----------
    Shader shaderCena("shaders/cloth.vert", "shaders/cloth.frag");

    // ---------- Buffers do tecido (dinâmico — atualizado todo frame) ----------
    unsigned int vaoTecido, vboTecido, eboTecido;
    glGenVertexArrays(1, &vaoTecido);
    glGenBuffers(1, &vboTecido);
    glGenBuffers(1, &eboTecido);

    glBindVertexArray(vaoTecido);
    glBindBuffer(GL_ARRAY_BUFFER, vboTecido);
    glBufferData(GL_ARRAY_BUFFER,
                 tecido.obterDadosVertices().size() * sizeof(float),
                 nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboTecido);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 tecido.indices.size() * sizeof(unsigned int),
                 tecido.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ---------- Buffers do chão e da haste (estáticos) ----------
    std::vector<float> verticesChao = Cenario::construirChao(ALTURA_CHAO, 30.0f, 0.0f, 0.0f);
    unsigned int vboChao, vaoChao = criarVAOEstatico(verticesChao, vboChao);
    int qtdVerticesChao = (int)(verticesChao.size() / 6);

    std::vector<float> verticesHaste = Cenario::construirHaste(
        -METADE_HASTE, METADE_HASTE, ALTURA_HASTE, 0.0f, 0.15f, 16);
    unsigned int vboHaste, vaoHaste = criarVAOEstatico(verticesHaste, vboHaste);
    int qtdVerticesHaste = (int)(verticesHaste.size() / 6);

    // ---------- Loop principal ----------
    float frameAnterior = 0.0f;

    while (!glfwWindowShouldClose(janela)) {
        float frameAtual = static_cast<float>(glfwGetTime());
        float deltaTempo = std::min(frameAtual - frameAnterior, 0.02f); // limita passo máximo
        frameAnterior = frameAtual;

        processarEntrada(janela);
        processarAlternancia(janela);

        // --- Atualiza a física ---
        glm::vec3 gravidade(0.0f, -9.8f, 0.0f);
        glm::vec3 vento(0.0f);
        if (ventoAtivo) {

            float ventoBase = 1.5f * std::sin(frameAtual * 0.8f);
            vento = glm::vec3(
                forcaVento * mousePosX + ventoBase,
                0.0f,
                forcaVento * mousePosZ
            );
        }

        tecido.atualizar(deltaTempo, gravidade, vento, 8 /* subpassos */);

        // --- Atualiza o VBO do tecido com as novas posições/normais ---
        std::vector<float> dadosVertices = tecido.obterDadosVertices();
        glBindBuffer(GL_ARRAY_BUFFER, vboTecido);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        dadosVertices.size() * sizeof(float),
                        dadosVertices.data());

        // --- Renderização ---
        glClearColor(0.12f, 0.13f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, modoArame ? GL_LINE : GL_FILL);

        shaderCena.usar();

        float yawRad   = glm::radians(yawCamera);
        float pitchRad = glm::radians(pitchCamera);
        glm::vec3 posicaoCamera = alvoCamera + glm::vec3(
            distanciaCamera * std::cos(pitchRad) * std::cos(yawRad),
            distanciaCamera * std::sin(pitchRad),
            distanciaCamera * std::cos(pitchRad) * std::sin(yawRad)
        );

        glm::mat4 matrizVisao = glm::lookAt(posicaoCamera, alvoCamera, glm::vec3(0, 1, 0));
        glm::mat4 matrizProjecao = glm::perspective(
            glm::radians(45.0f),
            (float)LARGURA_TELA / (float)ALTURA_TELA,
            0.1f, 100.0f
        );
        glm::mat4 matrizModelo = glm::mat4(1.0f); // identidade (sem transformação extra)

        shaderCena.enviarMat4("model",      matrizModelo);
        shaderCena.enviarMat4("view",       matrizVisao);
        shaderCena.enviarMat4("projection", matrizProjecao);
        shaderCena.enviarVec3("viewPos",    posicaoCamera);
        shaderCena.enviarVec3("lightPos",   glm::vec3(8.0f, 14.0f, 10.0f));
        shaderCena.enviarVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.97f));

        // Desenha o chão (cinza)
        shaderCena.enviarVec3("objectColor", glm::vec3(0.35f, 0.35f, 0.38f));
        glBindVertexArray(vaoChao);
        glDrawArrays(GL_TRIANGLES, 0, qtdVerticesChao);

        // Desenha a haste (cinza claro)
        shaderCena.enviarVec3("objectColor", glm::vec3(0.55f, 0.55f, 0.58f));
        glBindVertexArray(vaoHaste);
        glDrawArrays(GL_TRIANGLES, 0, qtdVerticesHaste);

        // Desenha o tecido (vermelho)
        shaderCena.enviarVec3("objectColor", glm::vec3(0.75f, 0.25f, 0.3f));
        glBindVertexArray(vaoTecido);
        glDrawElements(GL_TRIANGLES, (GLsizei)tecido.indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(janela); // troca os buffers (double buffering)
        glfwPollEvents();        // processa eventos de teclado/mouse
    }
    glDeleteVertexArrays(1, &vaoTecido); glDeleteBuffers(1, &vboTecido); glDeleteBuffers(1, &eboTecido);
    glDeleteVertexArrays(1, &vaoChao);   glDeleteBuffers(1, &vboChao);
    glDeleteVertexArrays(1, &vaoHaste);  glDeleteBuffers(1, &vboHaste);

    glfwTerminate();
    return 0;
}