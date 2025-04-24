#include <Axle.hpp>

class Sandbox : public Axle::Application {
  public:
    Sandbox() {
    }
    ~Sandbox() {
    }
};

Axle::Application *Axle::CreateApplication() {
    return new Sandbox();
}
