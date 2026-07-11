// #include <glad/gl.h>
// #include "Model.hpp"
//
// #include "Mesh.hpp"
// #include "assimp/Importer.hpp"
// #include "assimp/material.h"
// #include "assimp/mesh.h"
// #include "assimp/postprocess.h"
// #include "assimp/scene.h"
// #include "assimp/types.h"
// #include "Core/Error/Panic.hpp"
// #include "Core/Logger/Log.hpp"
//
// #include <stb_image.h>
//
// static u32 TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
//
// void Model::Draw(ShaderHelper& shader) {
//     for (u32 i = 0; i < m_Meshes.size(); ++i) {
//         m_Meshes[i].Draw(shader);
//     }
// }
//
// void Model::LoadModel(const std::string& path) {
//     Assimp::Importer importer;
//     const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
//
//     AX_ENSURE(scene != nullptr && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode != nullptr,
//               LogChannel::Client,
//               "Couldn't load model {0}. Error: {1}",
//               path,
//               importer.GetErrorString());
//
//     m_Directory = path.substr(0, path.find_last_of('/'));
//     ProcessNode(scene->mRootNode, scene);
// }
//
// void Model::ProcessNode(aiNode* node, const aiScene* scene) {
//     // process all mesh nodes
//     for (u32 i = 0; i < node->mNumMeshes; ++i) {
//         aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//         m_Meshes.push_back(ProcessMesh(mesh, scene));
//     }
//
//     // Do the same for its children
//     for (u32 i = 0; i < node->mNumChildren; ++i) {
//         ProcessNode(node->mChildren[i], scene);
//     }
// }
//
//
// Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
//     std::vector<Vertex> vertices;
//     std::vector<u32> indices;
//     std::vector<Texture> textures;
//
//     for (u32 i = 0; i < mesh->mNumVertices; ++i) {
//         Vertex vertex;
//
//         vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
//         vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
//
//         if (mesh->mTextureCoords[0] != nullptr) {
//             vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][1].x, mesh->mTextureCoords[1][i].y);
//         } else
//             vertex.TexCoords = glm::vec2(0.0f, 0.0f);
//
//         vertices.push_back(vertex);
//     }
//
//     for (u32 i = 0; i < mesh->mNumFaces; ++i) {
//         aiFace face = mesh->mFaces[i];
//         for (u32 j = 0; j < face.mNumIndices; ++j) {
//             indices.push_back(face.mIndices[j]);
//         }
//     }
//
//     if (mesh->mMaterialIndex >= 0) {
//         aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//         std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_difuse");
//         textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//         std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR,
//         "texture_specular"); textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//     }
//
//     return Mesh(vertices, indices, textures);
// }
//
// std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
//     std::vector<Texture> textures;
//
//     for (u32 i = 0; i < mat->GetTextureCount(type); ++i) {
//         aiString str;
//         mat->GetTexture(type, i, &str);
//         bool skip = false;
//
//         for (u32 j = 0; j < m_TexturesLoaded.size(); ++j) {
//             if (std::strcmp(m_TexturesLoaded[j].path.data(), str.C_Str()) == 0) {
//                 textures.push_back(m_TexturesLoaded[j]);
//                 skip = true;
//                 break;
//             }
//         }
//
//         if (!skip) {
//             Texture texture;
//             texture.id = TextureFromFile(str.C_Str(), m_Directory);
//             texture.type = typeName;
//             texture.path = str.C_Str();
//             textures.push_back(texture);
//             m_TexturesLoaded.push_back(texture);
//         }
//     }
//
//     return textures;
// }
//
// static u32 TextureFromFile(const char* path, const std::string& directory, bool gamma) {
//     std::string filename = std::string(path);
//     filename = directory + '/' + filename;
//
//     u32 textureID;
//     glGenTextures(1, &textureID);
//
//     i32 width, height, nrComponents;
//     u8* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//     AX_ENSURE(data != nullptr, LogChannel::Client, "Couldn't load image {0}", filename);
//
//     GLenum format;
//     if (nrComponents == 1)
//         format = GL_RED;
//     else if (nrComponents == 3)
//         format = GL_RGB;
//     else if (nrComponents == 4)
//         format = GL_RGBA;
//
//     glBindTexture(GL_TEXTURE_2D, textureID);
//     glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//     glGenerateMipmap(GL_TEXTURE_2D);
//
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//     stbi_image_free(data);
//     return textureID;
// }
