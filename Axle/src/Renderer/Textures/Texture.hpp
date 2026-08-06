#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Resource/ResourceManager.hpp"

namespace Axle {
    enum class TextureWrapMode { Repeat = 0, MirroredRepeat, ClampToEdge, ClampToBorder };
    enum class TextureFilteringMode {
        Nearest = 0,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear
    };
    enum class TextureFormat {
        RGB = 0,
        RGBA,
        RGB8,
        RGBA8,
        RGB16F,
        RGBA16F,
        RGB32F,
        RGBA32F,
        R8,
        RG8,
        SRGB8,
        SRGB8Alpha8,
        Depth24Stencil8,
        Depth32F
    };
    enum class TextureType { Diffuse = 0, Specular, Unknown };

    struct FaceExtract {
        u32 target;
        i32 col, row;
    };

    // Order matches GL_TEXTURE_CUBE_MAP_POSITIVE_X..NEGATIVE_Z (+X,-X,+Y,-Y,+Z,-Z)
    constexpr FaceExtract kFaces[6] = {
        {0x8515, 2, 1},
        {0x8516, 0, 1},
        {0x8517, 1, 0},
        {0x8518, 1, 2},
        {0x8519, 1, 1},
        {0x851A, 3, 1},
    };

    u32 TextureWrapModeToOpenGL(TextureWrapMode mode);
    u32 TextureFilterToOpenGL(TextureFilteringMode mode);
    u32 TextureFormatToOpenGL(TextureFormat format);

    void
    ExtractFaceFromHorizontalCross(const u8* data, i32 width, i32 nrChannels, i32 col, i32 row, i32 faceWidth, u8* dst);

    inline u32 CalculateMipmaps(u32 width, u32 height) {
        return floor(log2(std::max(width, height)));
    }

    class Texture : public RefCounted {
    public:
        virtual ~Texture() = default;

        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
        virtual TextureType GetType() const = 0;
        virtual u32 GetID() const = 0;

        virtual void Bind(u32 textureUnit) const = 0;
    };

    class Texture2D : public Texture {
    public:
        /// This constructor does nothing
        Texture2D() = default;

        /**
         * Create a texture but does not assign any data to it. Usefull for texture rendering...
         *
         * @param width Width of the texture
         * @param height Height of the texture
         * @param internalFormat What format does the texture have internally
         * @param mipmaps Indicates the number of mipmaps to have. Leave at -1 to automatically calculate.
         * @param type The type of the texture
         * */
        Texture2D(u32 width,
                  u32 height,
                  TextureFormat internalFormat,
                  i32 mipmaps = 0,
                  TextureType type = TextureType::Unknown);

        /**
         * Creates a texture. This constructor does not check cached textures nor does it caches, so even though
         * the texture you are looking may have already been loaded, this will do the process all
         * over again. It's recommended to instead use the static Create method to have that functionality.
         *
         * @param filename File containing the texture
         * @param mipmaps Indicates the number of mipmaps to have. Leave at -1 to automatically calculate.
         * @param flipVertically wether or not to flip the image vertically
         * @param type The type of the texture
         * */
        Texture2D(const std::string& path,
                  i32 mipmaps = 0,
                  bool flipVertically = true,
                  TextureType type = TextureType::Unknown);

        /**
         * Creates a texture. Unlike the base constructor this method supports caching and it's the recommended
         * way of creating a texture.
         *
         * @param filename File containing the texture
         * @param mipmaps Indicates the number of mipmaps to have. Leave at -1 to automatically calculate.
         * @param type The type of the texture
         * @param checkCached Indicates wether or not to check for cached programs with the same filename. If set to
         * false this method behaves like the constructor version.
         *
         * @returns A counted reference to the texture
         * */
        static Ref<Texture2D> Create(const std::string& filename,
                                     i32 mipmaps = 0,
                                     TextureType type = TextureType::Unknown,
                                     bool checkCached = true);

        virtual ~Texture2D() override;

        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        inline virtual u32 GetWidth() const override {
            return m_Width;
        }

        inline virtual u32 GetHeight() const override {
            return m_Height;
        }

        inline virtual TextureType GetType() const override {
            return m_Type;
        }

        inline virtual u32 GetID() const override {
            return m_ID;
        }

        virtual void Bind(u32 textureUnit) const override;

    private:
        /**
         * Deallocates all used memory
         * */
        void Reset();

        u32 m_ID = 0;
        u32 m_Width = 0, m_Height = 0;
        ResourceManager::ManagedFileHandle m_Handle;

        TextureType m_Type = TextureType::Unknown;
    };

    class TextureCubemap : public Texture {
    public:
        /// This constructor does nothing
        TextureCubemap() = default;

        /**
         * Creates a texture cubemap. This constructor does not check cached textures nor does it caches, so even though
         * the texture cubemap you are looking may have already been loaded and processed, this will do the process all
         * over again. It's recommended to instead use the static Create method to have that functionality.
         *
         * @param filename File containing the texture cubemap
         * @param flipVertically wether or not to flip the image vertically
         * */
        TextureCubemap(const std::string& path, bool flipVertically = false);

        /**
         * Creates a texture cubemap. Unlike the base constructor this method supports caching and it's the recommended
         * way of creating a texture.
         *
         * @param filename File containing the texture
         * @param checkCached Indicates wether or not to check for cached programs with the same filename. If set to
         * false this method behaves like the constructor version.
         *
         * @returns A counted reference to the texture
         * */
        static Ref<TextureCubemap> Create(const std::string& filename, bool checkCached = true);

        virtual ~TextureCubemap() override;

        TextureCubemap(TextureCubemap&& other) noexcept;
        TextureCubemap& operator=(TextureCubemap&& other) noexcept;

        TextureCubemap(const TextureCubemap&) = delete;
        TextureCubemap& operator=(const TextureCubemap&) = delete;

        inline virtual u32 GetWidth() const override {
            return m_Width;
        }

        inline virtual u32 GetHeight() const override {
            return m_Height;
        }

        inline virtual TextureType GetType() const override {
            return m_Type;
        }

        inline virtual u32 GetID() const override {
            return m_ID;
        }

        virtual void Bind(u32 textureUnit) const override;

    private:
        /**
         * Deallocates all used memory
         * */
        void Reset();

        u32 m_ID = 0;
        u32 m_Width = 0, m_Height = 0;
        ResourceManager::ManagedFileHandle m_Handle;

        TextureType m_Type = TextureType::Unknown;
    };
} // namespace Axle
