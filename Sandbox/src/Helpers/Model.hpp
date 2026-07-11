// #pragma once
//
// #include "assimp/material.h"
// #include <Axle.hpp>
//
// #include "ShaderHelper.hpp"
// #include "Mesh.hpp"
// #include "assimp/mesh.h"
// #include "assimp/scene.h"
//
// using namespace Axle;
//
// class Model {
// public:
//     Model(const char* path);
//
//     void Draw(ShaderHelper& shader);
//
// private:
//     void LoadModel(const std::string& path);
//     void ProcessNode(aiNode* node, const aiScene* scene);
//     Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
//     std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
//
//     std::vector<Mesh> m_Meshes;
//     std::string m_Directory;
//     std::vector<Texture> m_TexturesLoaded;
// };
