#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int programID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setFloat(const std::string& name, float value) const;
private:
    unsigned int compile(const char* source, unsigned int type);
    std::string readFile(const char* path);
};
