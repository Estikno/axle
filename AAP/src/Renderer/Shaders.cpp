#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <CLI11.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "../Setups.hpp"

#include "ShaderSource_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace AAP {
    void CLIShaderSetup(CLI::App& app) {
        CLI::App* shader = app.add_subcommand("shdr", "Composes a shader collection binary");

        static std::string shaderName = "";
        shader->add_option("-n,--name", shaderName, "Human-readable identifier for the owning object or material");

        static std::string shaderVersion = "";
        shader->add_option("-v,--version", shaderVersion, "Semantic version of this shader package (e.g. 1.0.3)");

        static std::string outputName = "shader.bin";
        shader->add_option("-o,--output", outputName, "Name of the output file");

        static std::vector<std::string> shaderFiles;
        shader
            ->add_option("-f,--files",
                         shaderFiles,
                         "Main shader files. Introduce a vertex shader and a fragment in this specific order.")
            ->required()
            ->expected(2);

        static std::string geometryFile = "";
        shader->add_option("-g,--geometry", geometryFile, "Add a geometry shader.");

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
            if (!DoesFilesExist(shaderFiles)) {
                std::cout << "Files not valid" << std::endl;
                return;
            }

            flatbuffers::FlatBufferBuilder builder(1024);

            // Shader collection parameters
            flatbuffers::Offset<flatbuffers::String> name = builder.CreateString(shaderName);
            flatbuffers::Offset<flatbuffers::String> version = builder.CreateString(shaderVersion);

            // Vertex shader
            flatbuffers::Offset<flatbuffers::String> vertexName = builder.CreateString(shaderFiles[0]);
            flatbuffers::Offset<flatbuffers::Vector<unsigned char>> vertexSourceContent =
                builder.CreateVector(ReadFile(shaderFiles[0].c_str()));
            flatbuffers::Offset<Axle::ShaderSource> vertexShader =
                Axle::CreateShaderSource(builder, Axle::ShaderStage_Vertex, vertexName, vertexSourceContent);

            // Fragment shader
            flatbuffers::Offset<flatbuffers::String> fragmentName = builder.CreateString(shaderFiles[1]);
            flatbuffers::Offset<flatbuffers::Vector<unsigned char>> fragmentSourceContent =
                builder.CreateVector(ReadFile(shaderFiles[1].c_str()));
            flatbuffers::Offset<Axle::ShaderSource> fragmentShader =
                Axle::CreateShaderSource(builder, Axle::ShaderStage_Fragment, fragmentName, fragmentSourceContent);

            // Geometry shader
            flatbuffers::Offset<flatbuffers::String> geometryName;
            flatbuffers::Offset<flatbuffers::Vector<unsigned char>> geometrySourceContent;
            flatbuffers::Offset<Axle::ShaderSource> geometryShader;
            bool geometryExists = false;

            if (geometryFile != "" && std::filesystem::exists(geometryFile)) {
                geometryExists = true;
                geometryName = builder.CreateString(geometryFile);
                geometrySourceContent = builder.CreateVector(ReadFile(geometryFile.c_str()));
                geometryShader =
                    Axle::CreateShaderSource(builder, Axle::ShaderStage_Geometry, geometryName, geometrySourceContent);
            }

            // TODO: Serialize other shaders (Teselation, ...)

            flatbuffers::Offset<Axle::ShaderPipeline> shaderPipeline = Axle::CreateShaderPipeline(
                builder, vertexShader, 0, 0, (geometryExists ? geometryShader : 0), fragmentShader);

            flatbuffers::Offset<Axle::ShaderCollection> shaderCollection =
                Axle::CreateShaderCollection(builder, name, version, shaderPipeline);

            builder.Finish(shaderCollection);

            u8* buf = builder.GetBufferPointer();
            u32 size = builder.GetSize();

            WriteFile(outputName.c_str(), buf, size);
        });
    }
} // namespace AAP
