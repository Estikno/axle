#include "axpch.hpp"

#include "glm/gtc/type_ptr.hpp"
#include <glad/gl.h>

#include "TextureManager.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"

#include <stb_image.h>

namespace Axle {
    std::unique_ptr<TextureManager> TextureManager::s_Instance = nullptr;

    void TextureManager::Init() {
        s_Instance = std::make_unique<TextureManager>();
        AX_CORE_INFO(LogChannel::Renderer, "Texture manager initialized");
    }

    void TextureManager::Shutdown() {
        s_Instance->Clear();

        AX_CORE_INFO(LogChannel::Renderer, "Texture manager deleted");
    }


    u32 TextureManager::CreateTexture(i32 width, i32 height, TextureFormat internalFormat) {
        u32 id;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id, 1, TextureFormatToOpenGL(internalFormat), width, height);

        m_NoFileTextures.insert(id);
        return id;
    }

    u32 TextureManager::CreateTexture(const std::string& path, TextureFormat internalFormat, bool flipVertically) {
        // Check if it has already been loaded
        auto find = m_PathsToIDs.find(path);
        if (find != m_PathsToIDs.end())
            return find->second;

        // Load data
        Expected<ResourceManager::ManagedFileHandle> exp = ResourceManager::GetInstance().Load(path);

        AX_ENSURE(exp.IsValid(), LogChannel::Renderer, "Couldn't load texture: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        ResourceManager::ManagedFileHandle tmpHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(tmpHandle).Unwrap();

        // Inerpret loaded data
        stbi_set_flip_vertically_on_load(flipVertically);
        i32 width, height, nrChannels;
        u8* data = stbi_load_from_memory(reinterpret_cast<const u8*>(readGuard.Data()),
                                         static_cast<i32>(readGuard.Size()),
                                         &width,
                                         &height,
                                         &nrChannels,
                                         0);
        stbi_set_flip_vertically_on_load(false);

        AX_ENSURE(data != nullptr, LogChannel::Renderer, "Error interpreting image of file: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        // Opengl
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3) {
            format = GL_RGB8;
        } else if (nrChannels == 4) {
            format = GL_RGBA8;
        }

        u32 id;
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        // glTextureStorage2D(id, 1 + floor(log2(fmax(width, height))), format, width, height);
        glTextureStorage2D(id, 1, format, width, height);
        glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        m_PathsToIDs[path] = id;
        m_IDToPaths[id] = path;
        m_IDToHandle[id] = std::move(tmpHandle);

        return id;
    }

    void TextureManager::SetWrapping(u32 ID, TextureWrapMode s, TextureWrapMode t) {
        glTextureParameteri(ID, GL_TEXTURE_WRAP_S, TextureWrapModeToOpenGL(s));
        glTextureParameteri(ID, GL_TEXTURE_WRAP_T, TextureWrapModeToOpenGL(t));
    }
    void TextureManager::SetFiltering(u32 ID, TextureFilteringMode min, TextureFilteringMode mag) {
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, TextureFilterToOpenGL(min));
        glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, TextureFilterToOpenGL(mag));
    }
    void TextureManager::GenerateMipmaps(u32 ID) {
        glGenerateTextureMipmap(ID);
    }

    void TextureManager::SetBorderColor(u32 ID, const glm::vec4& color) {
        glTextureParameterfv(ID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    }

    void TextureManager::Bind(u32 ID, u32 textureUnit) {
        glBindTextureUnit(textureUnit, ID);
    }

    void TextureManager::Clear() {
        for (auto it = m_NoFileTextures.begin(); it != m_NoFileTextures.end(); it++) {
            u32 id = *it;
            glDeleteTextures(1, &id);
        }
        for (auto it = m_IDToHandle.begin(); it != m_IDToHandle.end(); it++) {
            u32 id = it->first;
            glDeleteTextures(1, &id);
        }
    }
} // namespace Axle
