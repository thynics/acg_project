#pragma once
#include <glm/glm.hpp>
#include <array>

// ✅ Plane 必须先定义
struct Plane {
    glm::vec3 normal;
    float d;

    bool isInside(const glm::vec3& point) const {
        return glm::dot(normal, point) + d >= 0.0f;
    }
};

// ✅ AABB（可选）
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

// ✅ Frustum 使用 std::array<Plane, 6> 没问题
class Frustum {
public:
    std::array<Plane, 6> planes;

    void ExtractFromMatrix(const glm::mat4& clipMatrix);
    bool Intersects(const AABB& box) const;
};
