#include "Shader.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::lerArquivo(const char* caminho)
{
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir arquivo de shader: " << caminho << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << arquivo.rdbuf();
    return ss.str();
}

unsigned int Shader::compilar(const char* codigo, unsigned int tipo)
{
    unsigned int shader = glCreateShader(tipo);
    glShaderSource(shader, 1, &codigo, nullptr);
    glCompileShader(shader);

    int sucesso;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &sucesso);
    if (!sucesso) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Erro ao compilar shader: " << log << std::endl;
    }
    return shader;
}

Shader::Shader(const char* caminhoVertex, const char* caminhoFragment)
{
    std::string codigoVertex   = lerArquivo(caminhoVertex);
    std::string codigoFragment = lerArquivo(caminhoFragment);

    unsigned int vertex   = compilar(codigoVertex.c_str(),   GL_VERTEX_SHADER);
    unsigned int fragment = compilar(codigoFragment.c_str(), GL_FRAGMENT_SHADER);

    idPrograma = glCreateProgram();
    glAttachShader(idPrograma, vertex);
    glAttachShader(idPrograma, fragment);
    glLinkProgram(idPrograma);

    int sucesso;
    glGetProgramiv(idPrograma, GL_LINK_STATUS, &sucesso);
    if (!sucesso) {
        char log[512];
        glGetProgramInfoLog(idPrograma, 512, nullptr, log);
        std::cerr << "Erro ao linkar programa de shader: " << log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::usar() const
{
    glUseProgram(idPrograma);
}

void Shader::enviarMat4(const std::string& nome, const glm::mat4& mat) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(idPrograma, nome.c_str()),
        1, GL_FALSE, glm::value_ptr(mat)
    );
}

void Shader::enviarVec3(const std::string& nome, const glm::vec3& vec) const
{
    glUniform3fv(
        glGetUniformLocation(idPrograma, nome.c_str()),
        1, glm::value_ptr(vec)
    );
}

void Shader::enviarFloat(const std::string& nome, float valor) const
{
    glUniform1f(glGetUniformLocation(idPrograma, nome.c_str()), valor);
}