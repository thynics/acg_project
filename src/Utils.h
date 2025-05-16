#pragma once
#include <glm/glm.hpp>
#include <array>

struct Plane {
    glm::vec3 normal;
    float d;

    bool isInside(const glm::vec3& point) const {
        return glm::dot(normal, point) + d >= 0.0f;
    }
};

class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(FLT_MAX), max(-FLT_MAX) {}
    void expand(const glm::vec3& p) {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
};

class Frustum {
public:
    std::array<Plane, 6> planes;

    void ExtractFromMatrix(const glm::mat4& clipMatrix);
    bool Intersects(const AABB& box) const;
};

void Frustum::ExtractFromMatrix(const glm::mat4& m) {
    // 左
    planes[0].normal = glm::vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]);
    planes[0].d      = m[3][3] + m[3][0];
    // 右
    planes[1].normal = glm::vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]);
    planes[1].d      = m[3][3] - m[3][0];
    // 下
    planes[2].normal = glm::vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]);
    planes[2].d      = m[3][3] + m[3][1];
    // 上
    planes[3].normal = glm::vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]);
    planes[3].d      = m[3][3] - m[3][1];
    // 近
    planes[4].normal = glm::vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]);
    planes[4].d      = m[3][3] + m[3][2];
    // 远
    planes[5].normal = glm::vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]);
    planes[5].d      = m[3][3] - m[3][2];

    // 归一化六个平面
    for (auto& plane : planes) {
        float len = glm::length(plane.normal);
        plane.normal /= len;
        plane.d /= len;
    }
}

bool Frustum::Intersects(const AABB& box) const {
    for (const auto& plane : planes) {
        // 对每个面做 1 次 AABB 检查
        glm::vec3 p;
        p.x = (plane.normal.x >= 0) ? box.max.x : box.min.x;
        p.y = (plane.normal.y >= 0) ? box.max.y : box.min.y;
        p.z = (plane.normal.z >= 0) ? box.max.z : box.min.z;

        if (!plane.isInside(p)) {
            return false; // 被裁掉
        }
    }
    return true;
}