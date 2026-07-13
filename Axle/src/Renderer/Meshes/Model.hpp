#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Mesh.hpp"
#include "../Shaders/ShaderProgram.hpp"

namespace Axle {
    class Model {
    public:
        Model() = default;
        Model(const std::string& path);

        void Draw(ShaderProgram& program);

    private:
        struct InternalMethods;

        std::vector<Mesh> m_Meshes;
        ResourceManager::ManagedFileHandle m_Handle;
    };
} // namespace Axle
