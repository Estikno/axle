#include "assimp/types.h"
#include "axpch.hpp"

#include "Model.hpp"
#include "Renderer/Meshes/Mesh.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/material.h"
#include "assimp/postprocess.h"

namespace Axle {
    struct Model::InternalMethods {
        static void ProcessNode(aiNode* node, const aiScene* scene, Model* model);
        static Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        static std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType aiType, TextureType type);
    };

    Model::Model(const std::string& path) {
        Expected<ResourceManager::ManagedFileHandle> exp = ResourceManager::GetInstance().Load(path);

        if (!exp.IsValid()) {
            AX_CORE_ERROR(LogChannel::Renderer, "Couldn't load model from file: {0}");
            return;
        }

        m_Handle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(m_Handle).Unwrap();

        Assimp::Importer import;
        const aiScene* scene =
            import.ReadFileFromMemory(readGuard.Data(), readGuard.Size(), aiProcess_Triangulate | aiProcess_FlipUVs);

        if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
            AX_CORE_ERROR(LogChannel::Renderer,
                          "Couldn't import model from file: {0}. Error: {1}",
                          path,
                          import.GetErrorString());
            return;
        }

        InternalMethods::ProcessNode(scene->mRootNode, scene, this);
    }

    void Model::Draw(ShaderProgram& program) {
        for (u32 i = 0; i < m_Meshes.size(); ++i) {
            m_Meshes[i].Draw(program);
        }
    }

    void Model::InternalMethods::ProcessNode(aiNode* node, const aiScene* scene, Model* model) {
        // Process node's meshes
        for (u32 i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model->m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        // Recursion
        for (u32 i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    Mesh Model::InternalMethods::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::vector<Texture> textures;

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

        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps =
            LoadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture>
    Model::InternalMethods::LoadMaterialTextures(aiMaterial* mat, aiTextureType aiType, TextureType type) {
        std::vector<Texture> textures;

        for (u32 i = 0; i < mat->GetTextureCount(aiType); ++i) {
            aiString str;
            mat->GetTexture(aiType, i, &str);

            Texture text(std::string(str.C_Str()), TextureType::Diffuse);
            text.GenerateMipmaps();
            text.SetWrapping(TextureWrapMode::Repeat, TextureWrapMode::Repeat);
            text.SetFiltering(TextureFilteringMode::LinearMipmapLinear, TextureFilteringMode::Linear);

            textures.push_back(text);
        }

        return textures;
    }
} // namespace Axle
