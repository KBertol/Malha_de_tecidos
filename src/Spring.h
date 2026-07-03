#pragma once
#include "Particle.h"
#include <glm/glm.hpp>

enum class SpringType { STRUCTURAL, SHEAR, BEND };

class Spring {
public:
    Particle* p1; Particle* p2;
    float restLength, ks, kd;
    SpringType type;

    Spring(Particle* a, Particle* b, float stiffness, float damping, SpringType t)
        : p1(a), p2(b), ks(stiffness), kd(damping), type(t)
    { restLength = glm::length(p1->position - p2->position); }

    void applyForce() {
        glm::vec3 delta = p1->position - p2->position;
        float currentLength = glm::length(delta);
        if (currentLength < 1e-6f) return;
        glm::vec3 dir = delta / currentLength;
        float stretch = currentLength - restLength;
        float dampingTerm = glm::dot(p1->velocity - p2->velocity, dir);
        float forceMagnitude = -(ks * stretch + kd * dampingTerm);
        glm::vec3 force = forceMagnitude * dir;
        p1->addForce(force);
        p2->addForce(-force);
    }
};
