#pragma once
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Scene {
    inline std::vector<float> buildFloor(float y, float size, float cx, float cz)
    {
        float h = size * 0.5f;
        glm::vec3 n(0,1,0);
        glm::vec3 p0(cx-h,y,cz-h), p1(cx+h,y,cz-h), p2(cx+h,y,cz+h), p3(cx-h,y,cz+h);
        auto push = [](std::vector<float>& d, const glm::vec3& p, const glm::vec3& n){
            d.push_back(p.x); d.push_back(p.y); d.push_back(p.z);
            d.push_back(n.x); d.push_back(n.y); d.push_back(n.z);
        };
        std::vector<float> data;
        push(data,p0,n); push(data,p1,n); push(data,p2,n);
        push(data,p0,n); push(data,p2,n); push(data,p3,n);
        return data;
    }

    inline std::vector<float> buildRod(float x0, float x1, float y, float z, float radius, int segments)
    {
        std::vector<float> data;
        auto push = [&data](const glm::vec3& p, const glm::vec3& n){
            data.push_back(p.x); data.push_back(p.y); data.push_back(p.z);
            data.push_back(n.x); data.push_back(n.y); data.push_back(n.z);
        };
        for (int i = 0; i < segments; ++i) {
            float t0 = (float)i / segments * glm::two_pi<float>();
            float t1 = (float)(i+1) / segments * glm::two_pi<float>();
            glm::vec3 n0(0, std::cos(t0), std::sin(t0));
            glm::vec3 n1(0, std::cos(t1), std::sin(t1));
            glm::vec3 a0(x0, y+radius*n0.y, z+radius*n0.z);
            glm::vec3 a1(x0, y+radius*n1.y, z+radius*n1.z);
            glm::vec3 b0(x1, y+radius*n0.y, z+radius*n0.z);
            glm::vec3 b1(x1, y+radius*n1.y, z+radius*n1.z);
            push(a0,n0); push(b0,n0); push(b1,n1);
            push(a0,n0); push(b1,n1); push(a1,n1);
        }
        return data;
    }
}
