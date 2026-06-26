#include "glm/ext/matrix_float4x4.hpp"
#include <glad/gl.h>
#include <Axle.hpp>
#include "Core/Logger/Log.hpp"

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

        // Load textures
        int width1, height1, nrChannels1;
        unsigned char* data1 = stbi_load("assets/tests/container.jpg", &width1, &height1, &nrChannels1, 0);

        stbi_set_flip_vertically_on_load(true);

        int width2, height2, nrChannels2;
        unsigned char* data2 = stbi_load("assets/tests/awesomeface.png", &width1, &height1, &nrChannels1, 0);

        // Assign texture
        glGenTextures(1, &texture1);
        glGenTextures(1, &texture2);

        glBindTexture(GL_TEXTURE_2D, texture1);

        // Texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (data1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }

        glBindTexture(GL_TEXTURE_2D, texture2);

        // Texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (data2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }

        // Free the image memory
        stbi_image_free(data1);
        stbi_image_free(data2);

        // Set texture unit
        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "texture1"), 0);
        glUniform1i(glGetUniformLocation(program, "texture2"), 1);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Store the actual vertices
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Optimization for not using so many useless veritces
        // GLuint EBO;
        // glGenBuffers(1, &EBO);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Tell what the data is and how it should be read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) (3 * sizeof(f32)));
        glEnableVertexAttribArray(1);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0, 0.0, 0.0));

        glm::mat4 view = glm::mat4(1.0f);
        // note that we're translating the scene in the reverse direction of where we want to move
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
    void OnDettachRender() override {}
    void OnRender(f64 deltaTime) override {
        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        for (u32 i = 0; i < 10; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void OnEvent(Event& event) override {}

private:
    GLuint program, VAO, texture1, texture2;
    float vertices[180] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};
    glm::vec3 cubePositions[10] = {glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec3(2.0f, 5.0f, -15.0f),
                                   glm::vec3(-1.5f, -2.2f, -2.5f),
                                   glm::vec3(-3.8f, -2.0f, -12.3f),
                                   glm::vec3(2.4f, -0.4f, -3.5f),
                                   glm::vec3(-1.7f, 3.0f, -7.5f),
                                   glm::vec3(1.3f, -2.0f, -2.5f),
                                   glm::vec3(1.5f, 2.0f, -2.5f),
                                   glm::vec3(1.5f, 0.2f, -1.5f),
                                   glm::vec3(-1.3f, 1.0f, -1.5f)};
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
