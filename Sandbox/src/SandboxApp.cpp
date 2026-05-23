#include <glad/gl.h>
#include <Axle.hpp>

using namespace Axle;

class LearnLayer : public Axle::Layer {
public:
    LearnLayer()
        : Layer("Learn") {}
    ~LearnLayer() override = default;

    void OnAttach() override {}
    void OnUpdate(f64 FixedDeltaTime) override {}
    void OnDettach() override {
        AX_INFO("Learn layer detached");
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
            AX_PANIC("ERROR::SHADER::VERTEX::COMPILATION_FAILED: {0}", infoLog);
        }

        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &rFragData, &fragSize);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            AX_PANIC("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: {0}", infoLog);
        }

        // Create the shader program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            AX_PANIC("ERROR::SHADER::PROGRAM::LINK_FAILED: {0}", infoLog);
        }

        glUseProgram(program);
        // We no longer need the shader objects since we linked with the program
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*) 0);
        glEnableVertexAttribArray(0);
    }
    void OnDettachRender() override {}
    void OnRender(f64 DeltaTime) override {
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

private:
    f32 vertices[9] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
    GLuint program, VAO;
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
