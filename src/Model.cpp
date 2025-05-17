#include "Model.h"
#include <tiny_obj_loader.h>
#include <iostream>
#include <unordered_map>
#include <array>
#include <limits> // for std::numeric_limits

Model::Model(const std::string &path)
{
    loadModel(path);
}

void Model::Draw(const Shader &shader, const glm::mat4& model) const
{
    shader.setMat4("model", model);
    for (const auto &mesh : meshes)
    {
        mesh.Draw(shader);
    }
}

void Model::loadModel(const std::string &path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = path.substr(0, path.find_last_of('/') + 1);
    directory = baseDir;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), baseDir.c_str(), true);

    if (!warn.empty())
        std::cout << "tinyobjloader warning: " << warn << std::endl;
    if (!err.empty())
        std::cerr << "tinyobjloader error: " << err << std::endl;
    if (!ret)
    {
        std::cerr << "Failed to load obj: " << path << std::endl;
        return;
    }

    for (const auto &shape : shapes)
    {
        meshes.push_back(processShape(attrib, shape, materials));
    }

    minBound = glm::vec3( std::numeric_limits<float>::max() );
    maxBound = glm::vec3( std::numeric_limits<float>::lowest() );
    for (const auto &mesh : meshes)
    {
        for (const auto &vertex : mesh.vertices)
        {
            minBound = glm::min(minBound, vertex.Position);
            maxBound = glm::max(maxBound, vertex.Position);
        }
    }
}

Mesh Model::processShape(const tinyobj::attrib_t &attrib,
                         const tinyobj::shape_t &shape,
                         const std::vector<tinyobj::material_t> &materials)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (size_t i = 0; i < shape.mesh.indices.size(); i++)
    {
        tinyobj::index_t idx = shape.mesh.indices[i];

        Vertex vertex{};
        vertex.Position = glm::vec3(
            attrib.vertices[3 * idx.vertex_index + 0],
            attrib.vertices[3 * idx.vertex_index + 1],
            attrib.vertices[3 * idx.vertex_index + 2]);

        if (idx.normal_index >= 0)
        {
            vertex.Normal = glm::vec3(
                attrib.normals[3 * idx.normal_index + 0],
                attrib.normals[3 * idx.normal_index + 1],
                attrib.normals[3 * idx.normal_index + 2]);
        }
        else
        {
            vertex.Normal = glm::vec3(0.0f);
        }

        if (idx.texcoord_index >= 0)
        {
            vertex.TexCoords = glm::vec2(
                attrib.texcoords[2 * idx.texcoord_index + 0],
                attrib.texcoords[2 * idx.texcoord_index + 1]);
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f);
        }

        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(indices.size()));
    }

    glm::vec3 diffuseColor(1.0f); // default white
    std::string diffuseMapPath;
    int matID = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];
    if (matID >= 0 && matID < materials.size())
    {
        const auto &mat = materials[matID];
        diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

        if (!mat.diffuse_texname.empty())
        {
            diffuseMapPath = directory + mat.diffuse_texname; // e.g., "textures/sponza_column_b_diff.tga"
        }
    }

    return Mesh(vertices, indices, diffuseColor, diffuseMapPath);
}


std::pair<glm::vec3, glm::vec3> Model::CalculateWorldAABB(const glm::mat4 &modelMatrix) const
{
    // 假设模型空间的 AABB 是：
    // minBound = glm::vec3(minX, minY, minZ);
    // maxBound = glm::vec3(maxX, maxY, maxZ);

    glm::vec3 minBound = this->minBound; // 成员变量
    glm::vec3 maxBound = this->maxBound;

    // 生成模型空间下 AABB 的 8 个角点
    std::vector<glm::vec3> corners = {
        glm::vec3(minBound.x, minBound.y, minBound.z),
        glm::vec3(maxBound.x, minBound.y, minBound.z),
        glm::vec3(minBound.x, maxBound.y, minBound.z),
        glm::vec3(maxBound.x, maxBound.y, minBound.z),
        glm::vec3(minBound.x, minBound.y, maxBound.z),
        glm::vec3(maxBound.x, minBound.y, maxBound.z),
        glm::vec3(minBound.x, maxBound.y, maxBound.z),
        glm::vec3(maxBound.x, maxBound.y, maxBound.z),
    };

    glm::vec3 worldMin( std::numeric_limits<float>::max() );
    glm::vec3 worldMax( std::numeric_limits<float>::lowest() );

    for (const auto &corner : corners)
    {
        glm::vec4 transformed = modelMatrix * glm::vec4(corner, 1.0f);
        glm::vec3 pos = glm::vec3(transformed);
        worldMin = glm::min(worldMin, pos);
        worldMax = glm::max(worldMax, pos);
    }

    return {worldMin, worldMax};
}

