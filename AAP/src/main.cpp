#include <iostream>
#include <string>
#include <vector>
#include <CLI11.hpp>
#include <filesystem>

#include "Types.hpp"
#include "Utils.hpp"

#include "ShaderCollection_generated.h"
#include "flatbuffers/flatbuffers.h"

using namespace AAP;

int main(int argc, char** argv) {
    CLI::App app("Axle's Asset Pipeline", "AAP");
    argv = app.ensure_utf8(argv);
    app.require_subcommand(1);

    // SHADER COMPILATION ------------------
    CLI::App* shader = app.add_subcommand("shdr", "Composes a shader collection binary");

    std::string materialName;
    shader->add_option("-n,--name", materialName, "Human-readable identifier for the owning object or material")
        ->expected(1);

    std::string shaderVersion;
    shader->add_option("-v,--version", "Semantic version of this shader package (e.g. 1.0.3)");

    std::string target_api;
    shader->add_option(
        "-t,--target", target_api, "Target graphics API / hardware capability level this set was compiled for");

    bool MeshPipeline = false;
    shader->add_flag(
        "-m,--mesh_pipeline", MeshPipeline, "Uses a Mesh and Fragment shader only (and optionally a task one)");

    std::vector<std::string> shader_files;
    shader
        ->add_option(
            "-f,--files",
            shader_files,
            "Shader files. If Mesh Pipeline enabled introduce the mesh shader, fragment shader. If it's disabled introduce a vertex shader and a fragment one in this order.")
        ->required()
        ->expected(2);

    std::vector<std::string> tesselation_files;
    shader
        ->add_option("--tess,--tessellation",
                     tesselation_files,
                     "(Optional) Tessellation shader files. First Tessellation control then Tessellation evaluation")
        ->expected(2);

    std::string geometry_file;
    shader->add_option("--geo,--geometry", geometry_file, "(Optional) Geometry shader file.");

    shader->callback([&]() {
        // Check compulsory shaders
        if (!DoesFilesExist(shader_files)) {
            std::cout << "Files not valid" << std::endl;
            return;
        }

        flatbuffers::FlatBufferBuilder builder(1024);

        if (shader->get_option("-n")->count() > 0) {
            builder.CreateString(materialName);
        }
    });

    // -----------------------------------

    CLI11_PARSE(app, argc, argv);
    return 0;
}
