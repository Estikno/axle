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
        // Create the vertext shader and compile it at run-time
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // Create the fragment shader and compile it at run-time
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // Create a program object to be able to use the shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // We dont need the shaders anymore, they are already linked into the program
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create a vertex array object and a vertex buffer object to pass information to the GPU
        glGenVertexArrays(1, VAOs);
        glGenBuffers(1, VBOs);
        glGenBuffers(1, &EBO);

        // We tell OpenGL to use these VAO and VBO on the subsequent calls
        glBindVertexArray(VAOs[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // We transfer the vertex data to the GPU
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // We define how to interpret the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*) 0);
        glEnableVertexAttribArray(0);

        // We reset the VAO and VBO bindings to their defaults to prevent accidental modification
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Important: Do NOT unbind the EBO while a VAO is active as the bound EBO is stored in the VAO.
        // Unbinding it will mean that OpenGL will think we dont want to use the EBO.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void OnDettachRender() override {
        // We clean up the used resources
        glDeleteVertexArrays(1, VAOs);
        glDeleteBuffers(1, VBOs);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
    }
    void OnRender(f64 DeltaTime) override {
        // We specify OpenGL to use our program and our VAO
        glUseProgram(shaderProgram);
        glBindVertexArray(VAOs[0]);

        // We draw the triangle
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
    }

private:
    f32 vertices[18] = {
        0.0f,
        0.5f,
        0.0f,

        -0.5f * float(sqrt(3.0)) / 2.0f,
        -0.5f * 0.5f,
        0.0f,

        0.5f * float(sqrt(3.0)) / 2.0f,
        -0.5f * 0.5f,
        0.0f,

        0.0f,
        -0.5f,
        0.0f,

        -0.6f,
        0.2f,
        0.0f,

        0.6f,
        0.2f,
        0.0f,
    };
    u32 indices[9] = {1, 3, 4, 3, 5, 2, 0, 4, 5};
    u32 vertexShader, fragmentShader, shaderProgram, EBO;
    u32 VBOs[1], VAOs[1];
    const char* vertexShaderSource = "#version 460 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                     "}\0";
    const char* fragmentShaderSource = "#version 460 core\n"
                                       "out vec4 FragColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                       "}\n\0";
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
