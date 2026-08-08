#include "axpch.hpp"

#include <glad/gl.h>
#include <cstring>

#include "Texture.hpp"
#include "Renderer/GLDebug.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Error/Result.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Textures/TextureManager.hpp"

#include <stb_image.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    u32 TextureWrapModeToOpenGL(TextureWrapMode mode) {
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

    u32 TextureFilterToOpenGL(TextureFilteringMode mode) {
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

    u32 TextureFormatToOpenGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB:
                return GL_RGB;
            case TextureFormat::RGBA:
                return GL_RGBA;
            case TextureFormat::RGB8:
                return GL_RGB8;
            case TextureFormat::RGBA8:
                return GL_RGBA8;
            case TextureFormat::RGB16F:
                return GL_RGB16F;
            case TextureFormat::RGBA16F:
                return GL_RGBA16F;
            case TextureFormat::RGB32F:
                return GL_RGB32F;
            case TextureFormat::RGBA32F:
                return GL_RGBA32F;
            case TextureFormat::R8:
                return GL_R8;
            case TextureFormat::RG8:
                return GL_RG8;
            case TextureFormat::SRGB8:
                return GL_SRGB8;
            case TextureFormat::SRGB8Alpha8:
                return GL_SRGB8_ALPHA8;
            case TextureFormat::Depth24Stencil8:
                return GL_DEPTH24_STENCIL8;
            case TextureFormat::Depth32F:
                return GL_DEPTH_COMPONENT32F;
        }
    }

    void ExtractFaceFromHorizontalCross(const u8* data,
                                        i32 width,
                                        i32 nrChannels,
                                        i32 col,
                                        i32 row,
                                        i32 faceWidth,
                                        u8* dst) {
        /*
              .  [+Y] .   .
            [-X] [+Z] [+X] [-Z]
              .  [-Y] .   .
        */
        for (u32 y = 0; y < faceWidth; ++y) {
            const u8* srcRow = data + ((row * faceWidth + y) * width + col * faceWidth) * nrChannels;
            u8* dstRow = dst + y * faceWidth * nrChannels;
            std::memcpy(dstRow, srcRow, faceWidth * nrChannels);
        }
    }

    // --------------
    // Texture 2D
    // --------------

    Texture2D::Texture2D(u32 width, u32 height, TextureFormat internalFormat, i32 mipmaps, TextureType type)
        : m_Width(width),
          m_Height(height),
          m_Type(type) {
        TracyGpuZone("Create texture");

        AX_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &m_ID));
        AX_GL_CALL(glTextureStorage2D(m_ID,
                                      (mipmaps < 0) ? 1 + CalculateMipmaps(m_Width, m_Height) : 1 + mipmaps,
                                      TextureFormatToOpenGL(internalFormat),
                                      m_Width,
                                      m_Height));

        // Set default parameters
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }

    Texture2D::Texture2D(const std::string& path, i32 mipmaps, bool flipVertically, TextureType type)
        : m_Type(type) {
        ZoneScopedN("Create texture with source");

        Result<ResourceManager::ManagedFileHandle> res = ResourceManager::Load(path);

        AX_ENSURE(res.IsOk(), LogChannel::Renderer, "Couldn't load texture: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        m_Handle = res.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::DataConst(m_Handle).Unwrap();

        i32 width, height, nrChannels;
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

        m_Width = static_cast<u32>(width);
        m_Height = static_cast<u32>(height);

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
        } else {
            AX_PANIC(LogChannel::Renderer, "Image format not supported");
        }

        TracyGpuZone("Create texture");

        // OpenGL stuff
        AX_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &m_ID));
        AX_GL_CALL(glTextureStorage2D(m_ID,
                                      (mipmaps < 0) ? 1 + CalculateMipmaps(m_Width, m_Height) : 1 + mipmaps,
                                      internalFormat,
                                      m_Width,
                                      m_Height));

        // Set default parameters
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        AX_GL_CALL(glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        AX_GL_CALL(glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data));
        AX_GL_CALL(glGenerateTextureMipmap(m_ID));

        stbi_image_free(data);
    }

    Ref<Texture2D> Texture2D::Create(const std::string& filename, i32 mipmaps, TextureType type, bool checkCached) {
        if (checkCached) {
            Result<Ref<Texture2D>> res = TextureManager::IsCached2D(filename);
            if (res.IsOk())
                return res.Unwrap();
        }

        Ref<Texture2D> tex = Ref<Texture2D>::Create(filename, mipmaps, true, type); // strong count now 1, safely

        if (checkCached)
            TextureManager::Cache2D(filename, tex); // takes a WeakRef from an already-owned Ref

        return tex;
    }

    Texture2D::~Texture2D() {
        Reset();
    }

    Texture2D::Texture2D(Texture2D&& other) noexcept
        : m_ID(other.m_ID),
          m_Width(other.m_Width),
          m_Height(other.m_Height),
          m_Handle(std::move(other.m_Handle)) {
        other.m_ID = 0;
    }

    Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Handle = std::move(other.m_Handle);

            other.m_ID = 0;
        }
        return *this;
    }

    void Texture2D::Bind(u32 textureUnit) const {
        AX_GL_CALL(glBindTextureUnit(textureUnit, m_ID));
    }

    void Texture2D::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteTextures(1, &m_ID));
        }
    }

    // --------------
    // Texture Cubemap
    // --------------

    TextureCubemap::TextureCubemap(const std::string& path, bool flipVertically) {
        ZoneScopedN("Create cubemap texture");

        // Load data
        Result<ResourceManager::ManagedFileHandle> res = ResourceManager::Load(path);

        AX_ENSURE(res.IsOk(), LogChannel::Renderer, "Couldn't load texture: {0}", path);
        // TODO: Default to an ugly texture if it couldn't load it

        m_Handle = res.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::DataConst(m_Handle).Unwrap();

        i32 width, height, nrChannels;
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

        m_Width = static_cast<u32>(width);
        m_Height = static_cast<u32>(height);

        AX_ASSERT(width / 4 == height / 3, LogChannel::Renderer, "Texture is not valid as a cubemap texture");
        const i32 faceWidth = width / 4;

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
        } else {
            AX_PANIC(LogChannel::Renderer, "Image format not supported");
        }

        // OpenGL sutuff
        TracyGpuZone("Create texture cubemap");

        AX_GL_CALL(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID));
        AX_GL_CALL(glTextureStorage2D(m_ID, 1, internalFormat, faceWidth, faceWidth));

        // Store space for one face of the cubemap
        std::vector<u8> faceBuf(faceWidth * faceWidth * nrChannels);
        for (const FaceExtract& f : kFaces) {
            ExtractFaceFromHorizontalCross(data, width, nrChannels, f.col, f.row, faceWidth, faceBuf.data());

            const i32 faceIndex = f.target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;

            AX_GL_CALL(glTextureSubImage3D(
                m_ID, 0, 0, 0, faceIndex, faceWidth, faceWidth, 1, dataFormat, GL_UNSIGNED_BYTE, faceBuf.data()));
        }

        // Default options for test purposes
        glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);
    }

    Ref<TextureCubemap> TextureCubemap::Create(const std::string& filename, bool checkCached) {
        if (checkCached) {
            Result<Ref<TextureCubemap>> res = TextureManager::IsCachedCubemap(filename);
            if (res.IsOk())
                return res.Unwrap();
        }

        Ref<TextureCubemap> tex = Ref<TextureCubemap>::Create(filename); // strong count now 1, safely

        if (checkCached)
            TextureManager::CacheCubemap(filename, tex); // takes a WeakRef from an already-owned Ref

        return tex;
    }

    TextureCubemap::~TextureCubemap() {
        Reset();
    }

    TextureCubemap::TextureCubemap(TextureCubemap&& other) noexcept
        : m_ID(other.m_ID),
          m_Width(other.m_Width),
          m_Height(other.m_Height),
          m_Handle(std::move(other.m_Handle)) {
        other.m_ID = 0;
    }

    TextureCubemap& TextureCubemap::operator=(TextureCubemap&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Handle = std::move(other.m_Handle);

            other.m_ID = 0;
        }
        return *this;
    }

    void TextureCubemap::Bind(u32 textureUnit) const {
        AX_GL_CALL(glBindTextureUnit(textureUnit, m_ID));
    }

    void TextureCubemap::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteTextures(1, &m_ID));
        }
    }
} // namespace Axle
