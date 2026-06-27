#include "glm/fwd.hpp"
#include <glad/gl.h>
#include <AxleApp.hpp>

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Helpers/Shader.hpp"
#include "Helpers/Camera.hpp"
#include "Core/Application.hpp"

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
        // Setup shaders
        lightShader = Shader("Sandbox/src/Shaders/default.vert", "Sandbox/src/Shaders/lightFragment.frag");
        shader = Shader("Sandbox/src/Shaders/default.vert", "Sandbox/src/Shaders/default.frag");
        shader.Use();

        shader.SetVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
        shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        // Setup camera
        camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
        glfwSetInputMode(Application::GetInstance().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Store the actual vertices
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Tell what the data is and how it should be read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*) 0);
        glEnableVertexAttribArray(0);

        glm::mat4 model = glm::mat4(1.0f);
        shader.SetMat4("model", model);

        // Light
        lightShader.Use();
        glGenVertexArrays(1, &lightVAO);
        glBindVertexArray(lightVAO);
        // we only need to bind to the VBO, the container's VBO's data already contains the data.
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // set the vertex attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);


        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        lightShader.SetMat4("model", model);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void OnDettachRender() override {
        glDeleteVertexArrays(1, &VAO);
        glDeleteVertexArrays(1, &lightVAO);
        glDeleteBuffers(1, &VBO);
    }

    void OnRender(f64 deltaTime) override {
        // Draw cube
        shader.Use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw light cube
        lightShader.Use();
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Update camera
        camera.ProcessKeyboard(deltaTime);
        camera.ProcessMouseMovement(deltaTime);
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.GetZoom()), width / height, 0.1f, 100.0f);

        shader.Use();
        shader.SetMat4("projection", projection);
        shader.SetMat4("view", camera.GetViewMatrix());

        lightShader.Use();
        lightShader.SetMat4("projection", projection);
        lightShader.SetMat4("view", camera.GetViewMatrix());
    }

    bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
        width = (f32) event.GetWidth();
        height = (f32) event.GetHeight();
        return false;
    }

    bool OnMouseScroll(MouseScrollEvent& event) {
        camera.ProcessMouseScroll(static_cast<f32>(event.GetYOffset()));
        return false;
    }

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
        dispatcher.Dispatch<MouseScrollEvent>(AX_BIND_EVENT_FN(OnMouseScroll));
    }

private:
    Shader shader, lightShader;
    Camera camera;

    f32 width = 1280.0f, height = 720.0f;

    GLuint VAO, lightVAO, VBO;
    float vertices[108] = {-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
                           0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

                           -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
                           0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

                           -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
                           -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

                           0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
                           0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

                           -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
                           0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

                           -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
                           0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};

    glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
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
