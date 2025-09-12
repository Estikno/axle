#include <Axle.hpp>

class LearnLayer : public Axle::Layer {
public:
    LearnLayer()
        : Layer("Learn") {}
    ~LearnLayer() override {
        OnDetach();
    }

    void OnAttach() override {}
    void OnDetach() override {
        AX_INFO("Learn layer detached");
    }
    void OnUpdate() override {}
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
