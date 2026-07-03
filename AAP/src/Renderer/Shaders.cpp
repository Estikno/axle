#include "flatbuffers/buffer.h"
#include "flatbuffers/string.h"
#include "flatbuffers/vector.h"
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <CLI11.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "../Setups.hpp"

#include "ShadeSource_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace AAP {
    void CLIShaderSetup(CLI::App& app) {
        CLI::App* shader = app.add_subcommand("shdr", "Composes a shader collection binary");

        static std::string shaderName = "";
        shader->add_option("-n,--name", shaderName, "Human-readable identifier for the owning object or material");

        static std::string shaderVersion = "";
        shader->add_option("-v,--version", shaderVersion, "Semantic version of this shader package (e.g. 1.0.3)");

        static std::vector<std::string> shader_files;
        shader
            ->add_option(
                "-f,--files", shader_files, "Shader files. Introduce a vertex shader and a fragment one in this order.")
            ->required()
            ->expected(2);

        // std::vector<std::string> tesselation_files;
        // shader
        //     ->add_option(
        //         "--tess,--tessellation",
        //         tesselation_files,
        //         "(Optional) Tessellation shader files. First Tessellation control then Tessellation evaluation")
        //     ->expected(2);

        // std::string geometry_file;
        // shader->add_option("--geo,--geometry", geometry_file, "(Optional) Geometry shader file.");

        shader->callback([&]() {
            // Check compulsory shaders
            if (!DoesFilesExist(shader_files)) {
                std::cout << "Files not valid" << std::endl;
                return;
            }

            flatbuffers::FlatBufferBuilder builder(1024);

            // Shader collection parameters
            flatbuffers::Offset<flatbuffers::String> name = builder.CreateString(shaderName);
            flatbuffers::Offset<flatbuffers::String> version = builder.CreateString(shaderVersion);

            // Vertex shader
            flatbuffers::Offset<flatbuffers::String> vertexName = builder.CreateString(shader_files[0]);
            flatbuffers::Offset<flatbuffers::Vector<unsigned char>> vertexSourceContent =
                builder.CreateVector(ReadFile(shader_files[0].c_str()));
            flatbuffers::Offset<Axle::ShaderSource> vertexShader =
                Axle::CreateShaderSource(builder, Axle::ShaderStage_Vertex, vertexName, vertexSourceContent);

            // Fragment shader
            flatbuffers::Offset<flatbuffers::String> fragmentName = builder.CreateString(shader_files[1]);
            flatbuffers::Offset<flatbuffers::Vector<unsigned char>> fragmentSourceContent =
                builder.CreateVector(ReadFile(shader_files[1].c_str()));
            flatbuffers::Offset<Axle::ShaderSource> fragmentShader =
                Axle::CreateShaderSource(builder, Axle::ShaderStage_Fragment, fragmentName, fragmentSourceContent);

            // TODO: Serialize other shaders (Teselation, ...)

            flatbuffers::Offset<Axle::ShaderPipeline> shaderPipeline =
                Axle::CreateShaderPipeline(builder, vertexShader, 0, 0, 0, fragmentShader);

            flatbuffers::Offset<Axle::ShaderCollection> shaderCollection =
                Axle::CreateShaderCollection(builder, name, version, shaderPipeline);

            builder.Finish(shaderCollection);

            u8* buf = builder.GetBufferPointer();
            u32 size = builder.GetSize();

            WriteFile("test.bin", buf, size);
        });
    }
} // namespace AAP
