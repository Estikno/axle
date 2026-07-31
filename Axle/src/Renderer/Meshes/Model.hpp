#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Mesh.hpp"

namespace Axle {
    class Model {
    public:
        Model() = default;
        Model(const std::string& path);

        void Draw(const Ref<Shader>& shader);

    private:
        struct InternalMethods;

        std::vector<Mesh> m_Meshes;
        ResourceManager::ManagedFileHandle m_Handle;
        std::string m_Directory;
    };
} // namespace Axle
