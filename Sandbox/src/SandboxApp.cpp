#include <glad/gl.h>
#include <Axle.hpp>
#include "Core/Logger/Log.hpp"

#include <stb_image.h>

using namespace Axle;

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
        // Load shader files
        ResourceManager::ManagedFileHandle VertexShaderHandle =
            ResourceManager::GetInstance().Load("Sandbox/src/Shaders/default.vert").Unwrap();
        ResourceManager::ManagedFileHandle FragmentShaderHandle =
            ResourceManager::GetInstance().Load("Sandbox/src/Shaders/default.frag").Unwrap();

        ResourceManager::ReadGuard rVertex =
            std::move(ResourceManager::GetInstance().DataConst(VertexShaderHandle).Unwrap());
        const char* rVertexData = rVertex.Data();
        GLint vertexSize = (GLint) rVertex.Size();
        ResourceManager::ReadGuard rFrag =
            std::move(ResourceManager::GetInstance().DataConst(FragmentShaderHandle).Unwrap());
        const char* rFragData = rFrag.Data();
        GLint fragSize = (GLint) rFrag.Size();

        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &rVertexData, &vertexSize);
        glCompileShader(vertexShader);

        i32 success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            AX_PANIC(LogChannel::Client, "ERROR::SHADER::VERTEX::COMPILATION_FAILED: {0}", infoLog);
        }

        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &rFragData, &fragSize);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            AX_PANIC(LogChannel::Client, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: {0}", infoLog);
        }

        // Create the shader program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            AX_PANIC(LogChannel::Client, "ERROR::SHADER::PROGRAM::LINK_FAILED: {0}", infoLog);
        }

        glUseProgram(program);
        // We no longer need the shader objects since we linked with the program
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Load texture
        int width, height, nrChannels;
        unsigned char* data = stbi_load("assets/tests/container.jpg", &width, &height, &nrChannels, 0);

        // Assign texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }

        // Free the image memory
        stbi_image_free(data);


        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Store the actual vertices
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Optimization for not using so many useless veritces
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Tell what the data is and how it should be read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    void OnDettachRender() override {}
    void OnRender(f64 deltaTime) override {
        glUseProgram(program);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void OnEvent(Event& event) override {}

private:
    GLuint program, VAO, texture;
    float vertices[32] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    unsigned int indices[6] = {
        // note that we start from 0!
        0,
        1,
        3, // first triangle
        1,
        2,
        3 // second triangle
    };
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
