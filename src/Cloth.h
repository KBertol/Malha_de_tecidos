#pragma once
#include "Particle.h"
#include "Spring.h"
#include <vector>
#include <glm/glm.hpp>

class Cloth {
public:
    int width, height;
    float spacing;
    bool floorCollisionEnabled = true;
    float floorY = -12.0f;

    std::vector<Particle> particles;
    std::vector<Spring> springs;
    std::vector<unsigned int> indices;

    Cloth(int w, int h, float spacing, float particleMass);
    Particle& at(int x, int y) { return particles[y * width + x]; }
    void createSprings(float ksStruct, float kdStruct, float ksShear, float kdShear, float ksBend, float kdBend);
    void buildMesh();
    void applyExternalForces(const glm::vec3& gravity, const glm::vec3& wind);
    void update(float dt, const glm::vec3& gravity, const glm::vec3& wind, int numSubSteps);
    void computeNormals();
    void resolveFloorCollision(float floorY, float restitution = 0.3f);
    void setFixed(int x, int y, bool fixed);
    std::vector<float> getVertexData() const;
};
