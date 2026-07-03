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

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

float cameraDistance = 40.0f;
float cameraYaw = -60.0f;
float cameraPitch = 18.0f;
glm::vec3 cameraTarget(0.0f, -4.0f, 0.0f);

bool wireframe = false;
bool windEnabled = true;
float windStrength = 7.0f;

// Posição normalizada do mouse: (-1,-1) canto superior esquerdo,
// (1,1) canto inferior direito, (0,0) centro da janela.
// Atualizada pelo callback do cursor a cada movimento do mouse.
float mouseNormX = 0.0f;
float mouseNormZ = 0.0f;

const float RODY = 6.0f;
const float RODHALFLEN = 8.0f;
const float FLOORY = -9.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Chamado automaticamente pelo GLFW a cada movimento do mouse.
// Converte a posição em pixels para o intervalo [-1, 1] em cada eixo,
// com (0,0) no centro da janela. Esses valores viram a direção do vento.
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouseNormX = (float)(xpos / SCR_WIDTH  * 2.0 - 1.0); // -1=esquerda, 1=direita
    mouseNormZ = (float)(ypos / SCR_HEIGHT * 2.0 - 1.0); // -1=topo,     1=baixo
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float camSpeed = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) cameraYaw   -= camSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraYaw   += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) cameraPitch += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) cameraPitch -= camSpeed;
    if (glfwGetKey(window, GLFW_KEY_W)     == GLFW_PRESS) cameraDistance -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_S)     == GLFW_PRESS) cameraDistance += 0.5f;
    cameraPitch = std::clamp(cameraPitch, -89.0f, 89.0f);
    cameraDistance = std::clamp(cameraDistance, 5.0f, 100.0f);
}

void processToggleKeys(GLFWwindow* window) {
    static bool vPrev = false, fPrev = false;
    bool vNow = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
    if (vNow && !vPrev) wireframe = !wireframe;
    vPrev = vNow;
    bool fNow = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fNow && !fPrev) windEnabled = !windEnabled;
    fPrev = fNow;
}

unsigned int createStaticVAO(const std::vector<float>& vertexData, unsigned int& outVBO) {
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    outVBO = vbo;
    return vao;
}

int main() {
    glfwSetErrorCallback([](int code, const char* description) {
        std::cerr << "Erro GLFW [" << code << "]: " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar o GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Simulacao de Tecido - Varal/Cortina", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // glad substitui o GLEW: carrega os ponteiros de funcao OpenGL
    // via glfwGetProcAddress, sem depender de checagens via GLX que
    // causavam o bug "Unknown error" com GLEW 2.2.0 + Mesa 26.
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o glad" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);

    // Cria o tecido
    Cloth cloth(30, 30, 0.5f, 0.1f);
    // ks menores e mais subpassos = simulação estável sem explodir.
    // Molas muito rígidas (ks alto) com dt grande causam instabilidade
    // numérica no integrador de Euler — o tecido "explode" em 1-2 frames.
    cloth.createSprings(200.0f, 8.0f, 150.0f, 6.0f, 100.0f, 4.0f);
    cloth.buildMesh();

    float clothWidthWorld = (cloth.width - 1) * cloth.spacing;
    for (auto& p : cloth.particles)
        p.position += glm::vec3(-clothWidthWorld * 0.5f, RODY, 0.0f);

    for (int x = 0; x < cloth.width; x += 4)
        cloth.setFixed(x, 0, true);
    cloth.setFixed(cloth.width - 1, 0, true);

    cloth.floorY = FLOORY;
    cloth.floorCollisionEnabled = true;

    Shader sceneShader("shaders/cloth.vert", "shaders/cloth.frag");

    // Buffers do tecido (dinamico)
    unsigned int clothVAO, clothVBO, clothEBO;
    glGenVertexArrays(1, &clothVAO);
    glGenBuffers(1, &clothVBO);
    glGenBuffers(1, &clothEBO);
    glBindVertexArray(clothVAO);
    glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
    glBufferData(GL_ARRAY_BUFFER, cloth.getVertexData().size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cloth.indices.size() * sizeof(unsigned int), cloth.indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Buffers do chao e da haste (estaticos)
    std::vector<float> floorData = Scene::buildFloor(FLOORY, 30.0f, 0.0f, 0.0f);
    unsigned int floorVBO, floorVAO = createStaticVAO(floorData, floorVBO);
    int floorVertexCount = (int)(floorData.size() / 6);

    std::vector<float> rodData = Scene::buildRod(-RODHALFLEN, RODHALFLEN, RODY, 0.0f, 0.15f, 16);
    unsigned int rodVBO, rodVAO = createStaticVAO(rodData, rodVBO);
    int rodVertexCount = (int)(rodData.size() / 6);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = std::min(currentFrame - lastFrame, 0.02f);
        lastFrame = currentFrame;

        processInput(window);
        processToggleKeys(window);

        glm::vec3 gravity(0.0f, -9.8f, 0.0f);
        glm::vec3 wind(0.0f);
        if (windEnabled) {
            // A posição normalizada do mouse [-1, 1] define a direção
            // e intensidade do vento nos eixos X e Z.
            // Mouse no centro = sem vento. Mouse na borda = vento máximo.
            // Um leve vento base oscilante é somado para o tecido nunca
            // ficar completamente parado quando o mouse está no centro.
            float baseWind = 1.5f * std::sin(currentFrame * 0.8f);
            wind = glm::vec3(
                windStrength * mouseNormX + baseWind,
                0.0f,
                windStrength * mouseNormZ
            );
        }

        cloth.update(deltaTime, gravity, wind, 8);

        std::vector<float> vertexData = cloth.getVertexData();
        glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), vertexData.data());

        glClearColor(0.12f, 0.13f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

        sceneShader.use();

        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);
        glm::vec3 cameraOffset(
            cameraDistance * std::cos(pitchRad) * std::cos(yawRad),
            cameraDistance * std::sin(pitchRad),
            cameraDistance * std::cos(pitchRad) * std::sin(yawRad)
        );
        glm::vec3 cameraPos = cameraTarget + cameraOffset;

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        sceneShader.setMat4("model", model);
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setVec3("viewPos", cameraPos);
        sceneShader.setVec3("lightPos", glm::vec3(8.0f, 14.0f, 10.0f));
        sceneShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.97f));

        sceneShader.setVec3("objectColor", glm::vec3(0.35f, 0.35f, 0.38f));
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);

        sceneShader.setVec3("objectColor", glm::vec3(0.55f, 0.55f, 0.58f));
        glBindVertexArray(rodVAO);
        glDrawArrays(GL_TRIANGLES, 0, rodVertexCount);

        sceneShader.setVec3("objectColor", glm::vec3(0.75f, 0.25f, 0.3f));
        glBindVertexArray(clothVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)cloth.indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &clothVAO); glDeleteBuffers(1, &clothVBO); glDeleteBuffers(1, &clothEBO);
    glDeleteVertexArrays(1, &floorVAO); glDeleteBuffers(1, &floorVBO);
    glDeleteVertexArrays(1, &rodVAO);   glDeleteBuffers(1, &rodVBO);
    glfwTerminate();
    return 0;
}
