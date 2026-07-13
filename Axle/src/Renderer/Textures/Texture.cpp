#include "axpch.hpp"

#include <glad/gl.h>
#include <stb_image.h>
#include "glm/gtc/type_ptr.hpp"

#include "Texture.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Other/CustomTypes/Expected.hpp"

namespace Axle {
    static u32 TextureWrapModeToOpenGL(TextureWrapMode mode) {
        switch (mode) {
            case TextureWrapMode::Repeat:
                return GL_REPEAT;
            case TextureWrapMode::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case TextureWrapMode::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case TextureWrapMode::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
        }
    }

    static u32 TextureFilterToOpenGL(TextureFilteringMode mode) {
        switch (mode) {
            case TextureFilteringMode::Nearest:
                return GL_NEAREST;
            case TextureFilteringMode::Linear:
                return GL_LINEAR;
            case TextureFilteringMode::NearestMipmapNearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case TextureFilteringMode::LinearMipmapNearest:
                return GL_LINEAR_MIPMAP_NEAREST;
            case Axle::TextureFilteringMode::NearestMipmapLinear:
                return GL_NEAREST_MIPMAP_LINEAR;
            case TextureFilteringMode::LinearMipmapLinear:
                return GL_LINEAR_MIPMAP_LINEAR;
        }
    }

    static u32 TextureFormatToOpenGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB:
                return GL_RGB8;
            case TextureFormat::RGBA:
                return GL_RGBA8;
        }
        // TODO: Add all remaining formats
    }

    Texture::Texture(i32 width, i32 height, TextureFormat internalFormat, TextureFormat dataFormat, TextureType type)
        : m_Width(width),
          m_Height(height),
          m_InternalFormat(internalFormat),
          m_DataFormat(dataFormat),
          m_Type(type) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
        glTextureStorage2D(m_ID, 1, TextureFormatToOpenGL(m_InternalFormat), m_Width, m_Height);
    }

    Texture::Texture(const std::string& file, TextureType type, bool flipVertically)
        : m_Type(type) {
        Expected<ResourceManager::ManagedFileHandle> exp = ResourceManager::GetInstance().Load(file);

        // Load data
        AX_ENSURE(exp.IsValid(), LogChannel::Renderer, "Couldn't load texture: {0}", file);
        // TODO: Default to an ugly texture if it couldn't load it

        m_FileHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(m_FileHandle).Unwrap();

        // Intepret loaded data
        stbi_set_flip_vertically_on_load(flipVertically);
        u8* data = stbi_load_from_memory(reinterpret_cast<const u8*>(readGuard.Data()),
                                         static_cast<i32>(readGuard.Size()),
                                         &m_Width,
                                         &m_Height,
                                         &m_NrChannels,
                                         0);
        stbi_set_flip_vertically_on_load(false);

        AX_ENSURE(data != nullptr, LogChannel::Renderer, "Error interpreting image of file: {0}", file);
        // TODO: Default to an ugly texture if it couldn't load it

        GLenum format;
        if (m_NrChannels == 1)
            format = GL_RED;
        else if (m_NrChannels == 3) {
            format = GL_RGB8;
            m_InternalFormat = TextureFormat::RGB;
            m_DataFormat = TextureFormat::RGB;
        } else if (m_NrChannels == 4) {
            format = GL_RGBA8;
            m_InternalFormat = TextureFormat::RGBA;
            m_DataFormat = TextureFormat::RGBA;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
        glTextureStorage2D(m_ID, 1, format, m_Width, m_Height);
        glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    Texture::Texture(Texture&& other) noexcept
        : m_ID(other.m_ID),
          m_Width(other.m_Width),
          m_Height(other.m_Height),
          m_NrChannels(other.m_NrChannels),
          m_FileHandle(std::move(other.m_FileHandle)),
          m_InternalFormat(other.m_InternalFormat),
          m_DataFormat(other.m_DataFormat) {
        other.m_ID = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            if (m_ID != 0)
                glDeleteTextures(1, &m_ID);
            m_ID = other.m_ID;
            other.m_ID = 0;

            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_NrChannels = other.m_NrChannels;
            m_FileHandle = other.m_FileHandle;
            m_InternalFormat = other.m_InternalFormat;
            m_DataFormat = other.m_DataFormat;
        }
        return *this;
    }

    Texture::~Texture() {
        if (m_ID != 0)
            glDeleteTextures(1, &m_ID);
    }

    void Texture::SetSource(const std::string& file, bool flipVertically) {
        Expected<ResourceManager::ManagedFileHandle> exp = ResourceManager::GetInstance().Load(file);

        // Load data
        AX_ENSURE(exp.IsValid(), LogChannel::Renderer, "Couldn't load texture: {0}", file);
        // TODO: Default to an ugly texture if it couldn't load it

        m_FileHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(m_FileHandle).Unwrap();

        // Intepret loaded data
        stbi_set_flip_vertically_on_load(flipVertically);
        u8* data = stbi_load_from_memory(reinterpret_cast<const u8*>(readGuard.Data()),
                                         static_cast<i32>(readGuard.Size()),
                                         &m_Width,
                                         &m_Height,
                                         &m_NrChannels,
                                         0);
        stbi_set_flip_vertically_on_load(false);

        AX_ENSURE(data != nullptr, LogChannel::Renderer, "Error interpreting image of file: {0}", file);
        // TODO: Default to an ugly texture if it couldn't load it

        glTextureSubImage2D(
            m_ID, 0, 0, 0, m_Width, m_Height, TextureFormatToOpenGL(m_DataFormat), GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    void Texture::SetSource(ResourceManager::ManagedFileHandle& handle, bool flipVertically) {
        m_FileHandle = handle;
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(m_FileHandle).Unwrap();

        // Intepret loaded data
        stbi_set_flip_vertically_on_load(flipVertically);
        u8* data = stbi_load_from_memory(reinterpret_cast<const u8*>(readGuard.Data()),
                                         static_cast<i32>(readGuard.Size()),
                                         &m_Width,
                                         &m_Height,
                                         &m_NrChannels,
                                         0);
        stbi_set_flip_vertically_on_load(false);

        AX_ENSURE(data != nullptr, LogChannel::Renderer, "Error interpreting image");
        // TODO: Default to an ugly texture if it couldn't load it

        glTextureSubImage2D(
            m_ID, 0, 0, 0, m_Width, m_Height, TextureFormatToOpenGL(m_DataFormat), GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    void Texture::SetWrapping(TextureWrapMode s, TextureWrapMode t) {
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, TextureWrapModeToOpenGL(s));
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, TextureWrapModeToOpenGL(t));
    }

    void Texture::SetFiltering(TextureFilteringMode min, TextureFilteringMode mag) {
        glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, TextureFilterToOpenGL(min));
        glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, TextureFilterToOpenGL(mag));
    }

    void Texture::GenerateMipmaps() {
        glGenerateTextureMipmap(m_ID);
    }

    void Texture::SetBorderColor(const glm::vec4& color) {
        glTextureParameterfv(m_ID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    }

    void Texture::Bind(u32 textureUnit) {
        glBindTextureUnit(textureUnit, m_ID);
    }
} // namespace Axle
