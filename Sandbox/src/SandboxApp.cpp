#include <glad/gl.h>
#include <AxleApp.hpp>

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/vector3.h>

#include "Renderer/Shaders/ShaderProgram.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Application.hpp"
#include "Core/Core.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Events/Event.hpp"
#include "Renderer/Camera/Camera.hpp"

using namespace Axle;

struct PerFrameData {
    glm::mat4 mvp;
};

class LearnLayer : public Axle::Layer {
public:
    LearnLayer()
        : Layer("Learn") {}
    ~LearnLayer() override = default;

    void OnAttach() override {}
    void OnUpdate(f64 fixedDeltaTime) override {}
    void OnDettach() override {
        AX_INFO("Learn layer detached");
        Layer::OnDettach();
    }

    void OnAttachRender() override {
        glfwSetInputMode(Application::GetInstance().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        positionerDebug = CameraPositionerDebug(glm::vec3(0.0f, 0.0f, -10.0f), 90.0f, 0.0f);
        camera = Camera(positionerDebug);

        // Shaders
        Shader vertexShader("Sandbox/src/Shaders/default.bin", ShaderType::Vertex);
        Shader fragmentShader("Sandbox/src/Shaders/default.bin", ShaderType::Fragment);
        Shader geometryShader("Sandbox/src/Shaders/default.bin", ShaderType::Geometry);
        program = ShaderProgram(vertexShader, fragmentShader, geometryShader);
        program.Use();

        // PerFrameData uniform
        const GLsizeiptr kUniformBufferSize = sizeof(PerFrameData);

        glCreateBuffers(1, &perFrameBuffer);
        glNamedBufferStorage(perFrameBuffer, kUniformBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameBuffer, 0, kUniformBufferSize);

        // Load rubber_duck mesh
        const aiScene* scene =
            aiImportFile("assets/tests/CookBookAssets/rubber_duck/scene.gltf", aiProcess_Triangulate);
        AX_ENSURE(scene != nullptr && scene->HasMeshes(), LogChannel::Client, "Couldn't import the rubber_duck mesh");

        struct VertexData {
            glm::vec3 pos;
            glm::vec2 tc;
        };

        const aiMesh* mesh = scene->mMeshes[0];
        std::vector<VertexData> vertices;

        for (u32 i = 0; i != mesh->mNumVertices; ++i) {
            const aiVector3D v = mesh->mVertices[i];
            const aiVector3D t = mesh->mTextureCoords[0][i];
            vertices.push_back({.pos = glm::vec3(v.x, v.z, v.y), .tc = glm::vec2(t.x, t.y)});
        }

        for (u32 i = 0; i != mesh->mNumFaces; ++i) {
            for (u32 j = 0; j != 3; ++j) {
                indices.push_back(mesh->mFaces[i].mIndices[j]);
            }
        }

        aiReleaseImport(scene);

        // Upload data to OpenGL
        const size_t kSizeIndices = sizeof(u32) * indices.size();
        const size_t kSizeVertices = sizeof(VertexData) * vertices.size();

        glCreateBuffers(1, &dataIndices);
        glNamedBufferStorage(dataIndices, kSizeIndices, indices.data(), 0);
        glCreateBuffers(1, &dataVertices);
        glNamedBufferStorage(dataVertices, kSizeVertices, vertices.data(), 0);

        // Make the VAO read the indices from dataIndices
        glCreateVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glVertexArrayElementBuffer(VAO, dataIndices);

        // Bind the vertices data to a shader storage buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataVertices);

        // Texture
        int w, h, comp;
        const u8* img =
            stbi_load("assets/tests/CookBookAssets/rubber_duck/textures/Duck_baseColor.png", &w, &h, &comp, 3);
        AX_ENSURE(img != nullptr, LogChannel::Client, "Texture not loaded");

        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureStorage2D(texture, 1, GL_RGB8, w, h);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(texture, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img);
        glBindTextures(0, 1, &texture);

        stbi_image_free((void*) img);
    }

    void OnDettachRender() override {
        glDeleteBuffers(1, &dataIndices);
        glDeleteBuffers(1, &dataVertices);
        glDeleteBuffers(1, &perFrameBuffer);
        glDeleteVertexArrays(1, &VAO);
    }

    void OnRender(f64 deltaTime) override {
        const glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -1.5f)),
                                        (float) glfwGetTime(),
                                        glm::vec3(0.0f, 1.0f, 0.0f));

        positionerDebug.Update(deltaTime);

        const glm::mat4 p = glm::perspective(glm::radians(positionerDebug.GetFOV()), width / height, 0.1f, 1000.0f);
        const PerFrameData perFrameData = {.mvp = p * positionerDebug.GetViewMatrix() * m};

        glNamedBufferSubData(perFrameBuffer, 0, sizeof(PerFrameData), &perFrameData);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned>(indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
        width = (f32) event.GetWidth();
        height = (f32) event.GetHeight();
        return false;
    }

    bool OnMouseScroll(MouseScrollEvent& event) {
        positionerDebug.ProcessMouseScroll(event.GetYOffset());
        return false;
    }

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
        dispatcher.Dispatch<MouseScrollEvent>(AX_BIND_EVENT_FN(OnMouseScroll));
    }

private:
    ShaderProgram program;
    std::vector<u32> indices;

    Camera camera;
    CameraPositionerDebug positionerDebug;

    GLuint dataIndices, dataVertices, VAO, texture, perFrameBuffer;

    f32 width = 1280.0f, height = 720.0f;
};

class Sandbox : public Axle::Application {
public:
    Sandbox() {
        PushLayer(new LearnLayer());
    }
    ~Sandbox() {}
};

Axle::Application* Axle::CreateApplication() {
    return new Sandbox();
}
