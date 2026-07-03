#pragma once
#include <glm/glm.hpp>

class Particle {
public:
    glm::vec3 position, velocity, force, normal;
    float mass;
    bool isFixed;

    Particle(const glm::vec3& pos, float m = 1.0f)
        : position(pos), velocity(0.0f), force(0.0f),
          normal(0.0f, 0.0f, 1.0f), mass(m), isFixed(false) {}

    void clearForce() { force = glm::vec3(0.0f); }
    void addForce(const glm::vec3& f) { force += f; }

    void integrate(float dt) {
        if (isFixed) return;
        glm::vec3 acceleration = force / mass;
        velocity += acceleration * dt;
        position += velocity * dt;
    }
};
