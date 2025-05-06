#include "Model.h"
#include <tiny_obj_loader.h>
#include <iostream>
#include <unordered_map>
#include <limits> // for std::numeric_limits

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::Draw(const Shader& shader) const {
    for (const auto& mesh : meshes) {
        mesh.Draw(shader);
    }
}

void Model::loadModel(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = path.substr(0, path.find_last_of('/') + 1);
    directory = baseDir;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), baseDir.c_str(), true);

    if (!warn.empty()) std::cout << "tinyobjloader warning: " << warn << std::endl;
    if (!err.empty()) std::cerr << "tinyobjloader error: " << err << std::endl;
    if (!ret) {
        std::cerr << "Failed to load obj: " << path << std::endl;
        return;
    }

    for (const auto& shape : shapes) {
        meshes.push_back(processShape(attrib, shape, materials));
    }
}

Mesh Model::processShape(const tinyobj::attrib_t& attrib,
                         const tinyobj::shape_t& shape,
                         const std::vector<tinyobj::material_t>& materials) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
        tinyobj::index_t idx = shape.mesh.indices[i];

        Vertex vertex{};
        vertex.Position = glm::vec3(
            attrib.vertices[3 * idx.vertex_index + 0],
            attrib.vertices[3 * idx.vertex_index + 1],
            attrib.vertices[3 * idx.vertex_index + 2]
        );

        if (idx.normal_index >= 0) {
            vertex.Normal = glm::vec3(
                attrib.normals[3 * idx.normal_index + 0],
                attrib.normals[3 * idx.normal_index + 1],
                attrib.normals[3 * idx.normal_index + 2]
            );
        } else {
            vertex.Normal = glm::vec3(0.0f);
        }

        if (idx.texcoord_index >= 0) {
            vertex.TexCoords = glm::vec2(
                attrib.texcoords[2 * idx.texcoord_index + 0],
                attrib.texcoords[2 * idx.texcoord_index + 1]
            );
        } else {
            vertex.TexCoords = glm::vec2(0.0f);
        }

        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(indices.size()));
    }

    glm::vec3 diffuseColor(1.0f); // default white
    int matID = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];
    if (matID >= 0 && matID < materials.size()) {
        const auto& mat = materials[matID];
        diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
    }

    return Mesh(vertices, indices, diffuseColor);
}




std::pair<glm::vec3, glm::vec3> Model::CalculateWorldAABB(const glm::mat4& modelMatrix) const {
    glm::vec3 minBound(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());

    glm::vec3 maxBound(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest());

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            // 将顶点从模型空间转换到世界空间
            glm::vec4 worldPos = modelMatrix * glm::vec4(vertex.Position, 1.0f);
            glm::vec3 pos = glm::vec3(worldPos);

            minBound = glm::min(minBound, pos);
            maxBound = glm::max(maxBound, pos);
        }
    }

    return { minBound, maxBound }; // World-space AABB
}
