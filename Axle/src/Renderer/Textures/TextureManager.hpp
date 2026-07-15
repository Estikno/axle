#pragma once

#include "axpch.hpp"

#include "Core/Resource/ResourceManager.hpp"
#include "Texture.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    /**
     * Manages the textures of the renderer
     *
     * ALL the functionality of this class is NOT THREAD SAFE and must only be called from the renderer thread. Efforst
     * are being made to pararelize the loadings but it's currently not in effect
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
         * Returns a reference to the current instance. This is only valid after initializing the manager.
         *
         * @returns A reference to the manager instance
         * */
        inline static TextureManager& GetInstance() {
            return *s_Instance;
        }

        /**
         * Creates a texture with the given width, height and internalFormat.
         * This method does not populate the data, it simply reserves the needed space. Usefull for texture rendering.
         *
         * @param width The width of the texture
         * @param width The height of the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal format of the texture, this has to be sized
         *
         * @returns The id returned by OpenGL
         * */
        u32 CreateTexture(i32 width, i32 height, i32 mipmaps, TextureFormat internalFormat);

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
        u32 CreateTexture(const std::string& path, i32 mipmaps, bool flipVertically = true);

        /**
         * Creates a texture and populates the data from the given file path. This method automatically detects and
         * sets the data format of the texture.
         *
         * @param path The path to the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal OpenGL format of the texture
         * @param flipVertically Do you want the texture data to be flipped vertically when imported
         *
         * @returns The id returned by OpenGL
         * */
        u32
        CreateTexture(const std::string& path, i32 mipmaps, TextureFormat internalFormat, bool flipVertically = true);

        /**
         * Creates a texture and populates the data from the given file path. This method does not automatically set
         * neither the internal nor the data format, they have to be defined by the caller.
         *
         * @param path The path to the texture
         * @þaram mipmaps Desired number of mipmaps. Set to -1 to be calculated automatically.
         * @param internalFormat The internal OpenGL format of the texture
         * @param dataFormat The data format of the texture
         * @param flipVertically Do you want the texture data to be flipped vertically when imported
         *
         * @returns The id returned by OpenGL
         * */
        u32 CreateTexture(const std::string& path,
                          i32 mipmaps,
                          TextureFormat internalFormat,
                          TextureFormat dataFormat,
                          bool flipVertically = true);

        /**
         * Sets the wrapping mode for the s and t coordinate
         *
         * @param ID Texture id
         * @param s Texture wrapping mode for the s coordinate
         * @param t Texture wrapping mode for the t coordinate
         * */
        void SetWrapping(u32 ID, TextureWrapMode s, TextureWrapMode t);

        /**
         * Sets the texture filtering mode
         *
         * @param ID Texture id
         * @param min Texture filtering mode when minifying
         * @param mag Texture filtering mode when magnifying
         * */
        void SetFiltering(u32 ID, TextureFilteringMode min, TextureFilteringMode mag);

        /**
         * Generates mipmaps for the wanted texture. The number of minmaps is set when creating the texture.
         *
         * @param ID Texture id
         * */
        void GenerateMipmaps(u32 ID);

        /**
         * Sets a border color. Usefull if for example you choose clamp to border as a wrapping mode.
         *
         * @param ID Texture id
         * @param color The desired color for the border (RGBA, 0.0-1.0)
         * */
        void SetBorderColor(u32 ID, const glm::vec4& color);

        /**
         * Binds a texture to the desired texture unit.
         *
         * @param ID Texture id
         * @param textureUnit Texture unit to bind to
         * */
        void Bind(u32 ID, u32 textureUnit);

        /**
         * Deallocates all texture related memory. Created textures won't be usable anymore.
         * */
        void Clear();

    private:
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
        inline u32 CalculateMipmaps(i32 width, i32 height) {
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
