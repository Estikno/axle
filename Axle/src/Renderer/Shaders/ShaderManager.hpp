#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Error/Result.hpp"

namespace Axle {
    class Shader;

    /**
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class ShaderManager {
    public:
        static void Init();
        static void Shutdown();

        static Result<Ref<Shader>> IsCached(const std::string& filename);
        static void CacheShader(const std::string& filename, const Ref<Shader>& shader);

    private:
        static std::unique_ptr<ShaderManager> s_Instance;

        std::unordered_map<std::string, WeakRef<Shader>> m_Shaders;
    };
} // namespace Axle
