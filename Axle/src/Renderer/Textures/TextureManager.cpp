#include "axpch.hpp"

#include <glad/gl.h>

#include "TextureManager.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Textures/Texture.hpp"

#include <stb_image.h>
#include "glm/gtc/type_ptr.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

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

    u32 TextureManager::CreateTextureImpl(i32 width, i32 height, i32 mipmaps, TextureFormat internalFormat) {
        ZoneScopedN("Create Texture Without source");
        u32 id;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id,
                           (mipmaps < 0) ? 1 + CalculateMipmaps(width, height) : 1 + mipmaps,
                           TextureFormatToOpenGL(internalFormat),
                           width,
                           height);

        m_NoFileTextures.insert(id);
        return id;
    }

    u32 TextureManager::CreateTextureImpl(const std::string& path, i32 mipmaps, bool flipVertically) {
        ZoneScopedN("Create Texture with source");

        // Check if it has already been loaded
        auto find = m_PathsToIDs.find(path);
        if (find != m_PathsToIDs.end())
            return find->second;

        // Load data
        i32 width, height, nrChannels;
        std::pair<u8*, ResourceManager::ManagedFileHandle> data =
            LoadTextureFromFile(path, width, height, nrChannels, flipVertically);

        // Detect format automatically
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrChannels == 1) {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        } else if (nrChannels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        } else if (nrChannels == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }

        // OpenGL stuff
        u32 id;
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(
            id, (mipmaps < 0) ? 1 + CalculateMipmaps(width, height) : 1 + mipmaps, internalFormat, width, height);
        glTextureSubImage2D(id, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data.first);

        stbi_image_free(data.first);

        // Update internal data
        m_PathsToIDs[path] = id;
        m_IDToPaths[id] = path;
        m_IDToHandle[id] = std::move(data.second);

        return id;
    }

    u32 TextureManager::CreateTextureImpl(const std::string& path,
                                          i32 mipmaps,
                                          TextureFormat internalFormat,
                                          bool flipVertically) {
        ZoneScopedN("Create Texture with source");

        // Check if it has already been loaded
        auto find = m_PathsToIDs.find(path);
        if (find != m_PathsToIDs.end())
            return find->second;

        // Load data
        i32 width, height, nrChannels;
        std::pair<u8*, ResourceManager::ManagedFileHandle> data =
            LoadTextureFromFile(path, width, height, nrChannels, flipVertically);

        // Detect format automatically
        GLenum dataFormat;
        if (nrChannels == 1) {
            dataFormat = GL_RED;
        } else if (nrChannels == 3) {
            dataFormat = GL_RGB;
        } else if (nrChannels == 4) {
            dataFormat = GL_RGBA;
        }

        // OpenGL stuff
        u32 id;
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id,
                           (mipmaps < 0) ? 1 + CalculateMipmaps(width, height) : 1 + mipmaps,
                           TextureFormatToOpenGL(internalFormat),
                           width,
                           height);
        glTextureSubImage2D(id, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data.first);

        stbi_image_free(data.first);

        // Update internal data
        m_PathsToIDs[path] = id;
        m_IDToPaths[id] = path;
        m_IDToHandle[id] = std::move(data.second);

        return id;
    }

    u32 TextureManager::CreateTextureImpl(const std::string& path,
                                          i32 mipmaps,
                                          TextureFormat internalFormat,
                                          TextureFormat dataFormat,
                                          bool flipVertically) {
        ZoneScopedN("Create Texture with source");

        // Check if it has already been loaded
        auto find = m_PathsToIDs.find(path);
        if (find != m_PathsToIDs.end())
            return find->second;

        // Load data
        i32 width, height, nrChannels;
        std::pair<u8*, ResourceManager::ManagedFileHandle> data =
            LoadTextureFromFile(path, width, height, nrChannels, flipVertically);

        // OpenGL stuff
        u32 id;
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glTextureStorage2D(id,
                           (mipmaps < 0) ? 1 + CalculateMipmaps(width, height) : 1 + mipmaps,
                           TextureFormatToOpenGL(internalFormat),
                           width,
                           height);
        glTextureSubImage2D(
            id, 0, 0, 0, width, height, TextureFormatToOpenGL(dataFormat), GL_UNSIGNED_BYTE, data.first);

        stbi_image_free(data.first);

        // Update internal data
        m_PathsToIDs[path] = id;
        m_IDToPaths[id] = path;
        m_IDToHandle[id] = std::move(data.second);

        return id;
    }

    void TextureManager::SetWrappingImpl(u32 ID, TextureWrapMode s, TextureWrapMode t) {
        TracyGpuZone("Set Texture Wrapping");
        glTextureParameteri(ID, GL_TEXTURE_WRAP_S, TextureWrapModeToOpenGL(s));
        glTextureParameteri(ID, GL_TEXTURE_WRAP_T, TextureWrapModeToOpenGL(t));
    }

    void TextureManager::SetFilteringImpl(u32 ID, TextureFilteringMode min, TextureFilteringMode mag) {
        TracyGpuZone("Set Texture Filtering");
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, TextureFilterToOpenGL(min));
        glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, TextureFilterToOpenGL(mag));
    }

    void TextureManager::GenerateMipmapsImpl(u32 ID) {
        TracyGpuZone("Generate Mipmaps");
        glGenerateTextureMipmap(ID);
    }

    void TextureManager::SetBorderColorImpl(u32 ID, const glm::vec4& color) {
        TracyGpuZone("Set Texture Border Color");
        glTextureParameterfv(ID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    }

    void TextureManager::BindImpl(u32 ID, u32 textureUnit) {
        TracyGpuZone("Bind Texture");
        glBindTextureUnit(textureUnit, ID);
    }

    void TextureManager::ClearImpl() {
        ZoneScopedN("Clear all textures");
        TracyGpuZone("Clear all textures");

        for (auto it = m_NoFileTextures.begin(); it != m_NoFileTextures.end(); it++) {
            u32 id = *it;
            glDeleteTextures(1, &id);
        }
        for (auto it = m_IDToHandle.begin(); it != m_IDToHandle.end(); it++) {
            u32 id = it->first;
            glDeleteTextures(1, &id);
        }

        m_PathsToIDs.clear();
        m_IDToPaths.clear();
        m_IDToHandle.clear();
        m_NoFileTextures.clear();
    }

    std::pair<u8*, ResourceManager::ManagedFileHandle> TextureManager::LoadTextureFromFile(const std::string& path,
                                                                                           i32& width,
                                                                                           i32& height,
                                                                                           i32& nrChannels,
                                                                                           bool flipVertically) {
        ZoneScopedN("Load texture from file");
        // Load data
        Result<ResourceManager::ManagedFileHandle> exp = ResourceManager::Load(path);

        AX_ENSURE(exp.IsOk(), LogChannel::Renderer, "Couldn't load texture: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        ResourceManager::ManagedFileHandle tmpHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::DataConst(tmpHandle).Unwrap();

        // Inerpret loaded data
        stbi_set_flip_vertically_on_load(flipVertically);
        u8* data = stbi_load_from_memory(reinterpret_cast<const u8*>(readGuard.Data()),
                                         static_cast<i32>(readGuard.Size()),
                                         &width,
                                         &height,
                                         &nrChannels,
                                         0);
        stbi_set_flip_vertically_on_load(false);

        AX_ENSURE(data != nullptr, LogChannel::Renderer, "Error interpreting image of file: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        return std::make_pair(data, std::move(tmpHandle));
    }
} // namespace Axle
