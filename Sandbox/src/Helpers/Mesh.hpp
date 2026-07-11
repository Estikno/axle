// #pragma once
//
// #include <Axle.hpp>
//
// #include <glm/glm.hpp>
// #include "ShaderHelper.hpp"
//
// using namespace Axle;
//
// struct Vertex {
//     glm::vec3 Position;
//     glm::vec3 Normal;
//     glm::vec2 TexCoords;
// };
//
// struct Texture {
//     u32 id;
//     std::string type;
//     std::string path;
// };
//
// class Mesh {
// public:
//     std::vector<Vertex> p_Vertices;
//     std::vector<u32> p_Indices;
//     std::vector<Texture> p_Textures;
//
//     Mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const std::vector<Texture>& textures)
//         : p_Vertices(vertices),
//           p_Indices(indices),
//           p_Textures(textures) {
//         SetupMesh();
//     }
//     void Draw(ShaderHelper& shader);
//
// private:
//     void SetupMesh();
//
//     u32 m_VAO = 0, m_VBO = 0, m_EBO = 0;
// };
