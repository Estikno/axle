#include "axpch.hpp"

#include "Model.hpp"
#include "Renderer/Meshes/Mesh.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Textures/TextureManager.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/types.h"

#include <tracy/Tracy.hpp>

namespace Axle {
    struct Model::InternalMethods {
        static void ProcessNode(aiNode* node, const aiScene* scene, Model* model);
        static Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, Model* model);
        static std::vector<std::pair<u32, TextureType>>
        LoadMaterialTextures(aiMaterial* mat, aiTextureType aiType, TextureType type, const std::string& directory);
    };

    Model::Model(const std::string& path) {
        ZoneScopedN("Create model");

        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
            AX_CORE_ERROR(LogChannel::Renderer,
                          "Couldn't import model from file: {0}. Error: {1}",
                          path,
                          import.GetErrorString());
            return;
        }
        m_Directory = path.substr(0, path.find_last_of('/'));

        AX_CORE_INFO(LogChannel::Renderer,
                     "Loading model with: meshes={0} materials={1} nodes={2}",
                     scene->mNumMeshes,
                     scene->mNumMaterials,
                     scene->mRootNode->mNumChildren);

        InternalMethods::ProcessNode(scene->mRootNode, scene, this);
    }

    void Model::Draw(u32 program) {
        ZoneScopedN("Draw model");

        for (u32 i = 0; i < m_Meshes.size(); ++i) {
            m_Meshes[i].Draw(program);
        }
    }

    void Model::InternalMethods::ProcessNode(aiNode* node, const aiScene* scene, Model* model) {
        ZoneScopedN("Process model node");

        // Process node's meshes
        for (u32 i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model->m_Meshes.push_back(ProcessMesh(mesh, scene, model));
        }

        // Recursion
        for (u32 i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    Mesh Model::InternalMethods::ProcessMesh(aiMesh* mesh, const aiScene* scene, Model* model) {
        ZoneScopedN("Process mesh model");

        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::vector<std::pair<u32, TextureType>> textures;

        // Vertex data
        for (u32 i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->mTextureCoords[0] != nullptr)
                vertex.textureCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.textureCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // Indices
        for (u32 i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        // Material
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<std::pair<u32, TextureType>> diffuseMaps =
            LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse, model->m_Directory);
        textures.insert(
            textures.end(), std::make_move_iterator(diffuseMaps.begin()), std::make_move_iterator(diffuseMaps.end()));

        std::vector<std::pair<u32, TextureType>> specularMaps =
            LoadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular, model->m_Directory);
        textures.insert(
            textures.end(), std::make_move_iterator(specularMaps.begin()), std::make_move_iterator(specularMaps.end()));

        return Mesh(vertices, indices, std::move(textures));
    }

    std::vector<std::pair<u32, TextureType>>
    Model::InternalMethods::LoadMaterialTextures(aiMaterial* mat,
                                                 aiTextureType aiType,
                                                 TextureType type,
                                                 const std::string& directory) {
        ZoneScopedN("Load material textures");

        std::vector<std::pair<u32, TextureType>> textures;

        for (u32 i = 0; i < mat->GetTextureCount(aiType); ++i) {
            aiString str;
            mat->GetTexture(aiType, i, &str);

            std::string filename = directory + "/" + std::string(str.C_Str());

            u32 textId = TextureManager::CreateTexture(filename, -1);
            TextureManager::SetWrapping(textId, TextureWrapMode::Repeat, TextureWrapMode::Repeat);
            TextureManager::SetFiltering(textId, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
            textures.push_back(std::make_pair(textId, type));
        }

        return textures;
    }
} // namespace Axle
