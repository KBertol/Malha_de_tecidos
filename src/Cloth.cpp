#include "Cloth.h"

Cloth::Cloth(int w, int h, float spc, float particleMass)
    : width(w), height(h), spacing(spc)
{
    particles.reserve(width * height);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            particles.emplace_back(glm::vec3(x * spacing, -y * spacing, 0.0f), particleMass);
}

void Cloth::createSprings(float ksStruct, float kdStruct, float ksShear, float kdShear, float ksBend, float kdBend)
{
    auto idx = [this](int x, int y) { return y * width + x; };
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x + 1 < width)  springs.emplace_back(&particles[idx(x,y)], &particles[idx(x+1,y)], ksStruct, kdStruct, SpringType::STRUCTURAL);
            if (y + 1 < height) springs.emplace_back(&particles[idx(x,y)], &particles[idx(x,y+1)], ksStruct, kdStruct, SpringType::STRUCTURAL);
            if (x + 1 < width && y + 1 < height) {
                springs.emplace_back(&particles[idx(x,y)],   &particles[idx(x+1,y+1)], ksShear, kdShear, SpringType::SHEAR);
                springs.emplace_back(&particles[idx(x+1,y)], &particles[idx(x,y+1)],   ksShear, kdShear, SpringType::SHEAR);
            }
            if (x + 2 < width)  springs.emplace_back(&particles[idx(x,y)], &particles[idx(x+2,y)], ksBend, kdBend, SpringType::BEND);
            if (y + 2 < height) springs.emplace_back(&particles[idx(x,y)], &particles[idx(x,y+2)], ksBend, kdBend, SpringType::BEND);
        }
    }
}

void Cloth::buildMesh()
{
    indices.clear();
    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            unsigned int tl = y * width + x, tr = tl + 1;
            unsigned int bl = (y+1) * width + x, br = bl + 1;
            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }
}

void Cloth::applyExternalForces(const glm::vec3& gravity, const glm::vec3& wind)
{
    for (auto& p : particles) {
        p.clearForce();
        p.addForce(gravity * p.mass);
        p.addForce(wind);
    }
}

void Cloth::resolveFloorCollision(float floor, float restitution)
{
    for (auto& p : particles) {
        if (p.isFixed) continue;
        if (p.position.y < floor) {
            p.position.y = floor;
            if (p.velocity.y < 0.0f)
                p.velocity.y = -p.velocity.y * restitution;
            p.velocity.x *= 0.92f;
            p.velocity.z *= 0.92f;
        }
    }
}

void Cloth::update(float dt, const glm::vec3& gravity, const glm::vec3& wind, int numSubSteps)
{
    float subDt = dt / static_cast<float>(numSubSteps);
    for (int step = 0; step < numSubSteps; ++step) {
        applyExternalForces(gravity, wind);
        for (auto& s : springs) s.applyForce();
        for (auto& p : particles) p.integrate(subDt);
        if (floorCollisionEnabled) resolveFloorCollision(floorY);
    }
    computeNormals();
}

void Cloth::computeNormals()
{
    for (auto& p : particles) p.normal = glm::vec3(0.0f);
    for (size_t i = 0; i < indices.size(); i += 3) {
        const glm::vec3& a = particles[indices[i]].position;
        const glm::vec3& b = particles[indices[i+1]].position;
        const glm::vec3& c = particles[indices[i+2]].position;
        glm::vec3 n = glm::cross(b - a, c - a);
        particles[indices[i]].normal   += n;
        particles[indices[i+1]].normal += n;
        particles[indices[i+2]].normal += n;
    }
    for (auto& p : particles)
        if (glm::length(p.normal) > 1e-6f) p.normal = glm::normalize(p.normal);
}

void Cloth::setFixed(int x, int y, bool fixed) { at(x, y).isFixed = fixed; }

std::vector<float> Cloth::getVertexData() const
{
    std::vector<float> data;
    data.reserve(particles.size() * 6);
    for (const auto& p : particles) {
        data.push_back(p.position.x); data.push_back(p.position.y); data.push_back(p.position.z);
        data.push_back(p.normal.x);   data.push_back(p.normal.y);   data.push_back(p.normal.z);
    }
    return data;
}
