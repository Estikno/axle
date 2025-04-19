#pragma once

#ifdef AX_PLATFORM_WINDOWS

extern Axle::Application* Axle::CreateApplication();

int main(int argc, char** argv) {
	auto app = Axle::CreateApplication();
	app->Run();
	delete app;
}
#endif // AX_PLATFORM_WINDOWS
