#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int idPrograma; 

    Shader(const char* caminhoVertex, const char* caminhoFragment);
    void usar() const;
    void enviarMat4(const std::string& nome, const glm::mat4& mat) const;
    void enviarVec3(const std::string& nome, const glm::vec3& vec) const;
    void enviarFloat(const std::string& nome, float valor) const;

private:
    std::string lerArquivo(const char* caminho);
    unsigned int compilar(const char* codigo, unsigned int tipo);
};