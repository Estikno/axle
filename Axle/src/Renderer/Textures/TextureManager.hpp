#pragma once

#include "axpch.hpp"

#include "Core/Resource/ResourceManager.hpp"
#include "Texture.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    /**
     * Manages the textures of the renderer
     *
     * ALL the functionality of this class is NOT THREAD SAFE and must only be called from the renderer thread. Loadings
     * are planned to be pararelized but it's currently not in effect
     * */
    class TextureManager {
    public:
        // Constructor and destructor do nothing as everything is constroled via Init/Shutdown
        TextureManager() {}
        ~TextureManager() {}

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        /**
         * Initializes the system
         * */
        static void Init();
        /**
         * Properly deletes and deallocates all resources used
         * */
        static void Shutdown();

        /**
         * Creates a texture with the given width, height and internalFormat.
         * This method does not populate the data, it simply reserves the needed space. Usefull for texture rendering.
         *
         * @param width The width of the texture
         * @param width The height of the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal OpenGL format of the texture, this has to be sized
         *
         * @returns The id returned by OpenGL
         * */
        inline static u32 CreateTexture(i32 width, i32 height, i32 mipmaps, TextureFormat internalFormat) {
            return s_Instance->CreateTextureImpl(width, height, mipmaps, internalFormat);
        }

        /**
         * Creates a texture and populates the data from the given file path. This method automatically detects and
         * sets both the internal and data format of the texture.
         *
         * @param path The path to the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param flipVertically Do you want the texture data to be flipped vertically when imported
         *
         * @returns The id returned by OpenGL
         * */
        inline static u32 CreateTexture(const std::string& path, i32 mipmaps, bool flipVertically = true) {
            return s_Instance->CreateTextureImpl(path, mipmaps, flipVertically);
        }

        /**
         * Creates a texture and populates the data from the given file path. This method automatically detects and
         * sets the data format of the texture.
         *
         * @param path The path to the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal OpenGL format of the texture, this has to be sized
         * @param flipVertically Do you want the texture data to be flipped vertically when imported
         *
         * @returns The id returned by OpenGL
         * */
        inline static u32
        CreateTexture(const std::string& path, i32 mipmaps, TextureFormat internalFormat, bool flipVertically = true) {
            return s_Instance->CreateTextureImpl(path, mipmaps, internalFormat, flipVertically);
        }

        /**
         * Creates a texture and populates the data from the given file path. This method does not automatically set
         * neither the internal nor the data format, they have to be defined by the caller.
         *
         * @param path The path to the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal OpenGL format of the texture, this has to be sized
         * @param dataFormat The data format of the texture
         * @param flipVertically Do you want the texture data to be flipped vertically when imported
         *
         * @returns The id returned by OpenGL
         * */
        inline static u32 CreateTexture(const std::string& path,
                                        i32 mipmaps,
                                        TextureFormat internalFormat,
                                        TextureFormat dataFormat,
                                        bool flipVertically = true) {
            return s_Instance->CreateTextureImpl(path, mipmaps, internalFormat, dataFormat, flipVertically);
        }

        /**
         * Sets the wrapping mode for the s and t coordinate
         *
         * @param ID Texture id
         * @param s Texture wrapping mode for the s coordinate
         * @param t Texture wrapping mode for the t coordinate
         * */
        inline static void SetWrapping(u32 ID, TextureWrapMode s, TextureWrapMode t) {
            s_Instance->SetWrappingImpl(ID, s, t);
        }

        /**
         * Sets the texture filtering mode
         *
         * @param ID Texture id
         * @param min Texture filtering mode when minifying
         * @param mag Texture filtering mode when magnifying
         * */
        inline static void SetFiltering(u32 ID, TextureFilteringMode min, TextureFilteringMode mag) {
            s_Instance->SetFilteringImpl(ID, min, mag);
        }

        /**
         * Generates mipmaps for the wanted texture. The number of minmaps is set when creating the texture.
         *
         * @param ID Texture id
         * */
        inline static void GenerateMipmaps(u32 ID) {
            s_Instance->GenerateMipmapsImpl(ID);
        }

        /**
         * Sets a border color. Usefull if for example you choose clamp to border as a wrapping mode.
         *
         * @param ID Texture id
         * @param color The desired color for the border (RGBA, 0.0-1.0)
         * */
        inline static void SetBorderColor(u32 ID, const glm::vec4& color) {
            s_Instance->SetBorderColorImpl(ID, color);
        }

        /**
         * Binds a texture to the desired texture unit.
         *
         * @param ID Texture id
         * @param textureUnit Texture unit to bind to
         * */
        inline static void Bind(u32 ID, u32 textureUnit) {
            s_Instance->BindImpl(ID, textureUnit);
        }

        /**
         * Deallocates all texture related memory. Created textures won't be usable anymore.
         * */
        inline static void Clear() {
            s_Instance->ClearImpl();
        }

    private:
        // Static methods implementations
        u32 CreateTextureImpl(i32 width, i32 height, i32 mipmaps, TextureFormat internalFormat);
        u32 CreateTextureImpl(const std::string& path, i32 mipmaps, bool flipVertically);
        u32 CreateTextureImpl(const std::string& path, i32 mipmaps, TextureFormat internalFormat, bool flipVertically);
        u32 CreateTextureImpl(const std::string& path,
                              i32 mipmaps,
                              TextureFormat internalFormat,
                              TextureFormat dataFormat,
                              bool flipVertically);
        void SetWrappingImpl(u32 ID, TextureWrapMode s, TextureWrapMode t);
        void SetFilteringImpl(u32 ID, TextureFilteringMode min, TextureFilteringMode mag);
        void GenerateMipmapsImpl(u32 ID);
        void SetBorderColorImpl(u32 ID, const glm::vec4& color);
        void BindImpl(u32 ID, u32 textureUnit);
        void ClearImpl();

        /**
         * Handles the creation of the internal file handle and returns a pointer to the data interpreted by
         * stb_image.
         *
         * @param path The path to the texture
         * @param width A reference to the width to be modified by stb_image
         * @param height A reference to the height to be modified by stb_image
         * @param nrChannels A reference to the number of channels to be modified by stb_image
         * @param flipVertically Sets if the texture is loaded flipping the vertical axis
         *
         * @returns A pair containing first a pointer to the data interpreted by stb_image that needs to be deleted by
         * the caller and as the second element the file handle attached to the texture file
         * */
        std::pair<u8*, ResourceManager::ManagedFileHandle>
        LoadTextureFromFile(const std::string& path, i32& width, i32& height, i32& nrChannels, bool flipVertically);

        /**
         * Calculates how many mipmaps a texture needs based on the width and height.
         *
         * @param width Width of the texture
         * @param height Height of the texture
         *
         * @returns The necessary mipmaps for the texture
         * */
        inline static u32 CalculateMipmaps(i32 width, i32 height) {
            return floor(log2(std::max(width, height)));
        }

        static std::unique_ptr<TextureManager> s_Instance;

        std::unordered_map<std::string, u32> m_PathsToIDs;
        std::unordered_map<u32, std::string> m_IDToPaths;
        std::unordered_map<u32, ResourceManager::ManagedFileHandle> m_IDToHandle;

        // Textures with no file attachment
        std::unordered_set<u32> m_NoFileTextures;
    };
} // namespace Axle
