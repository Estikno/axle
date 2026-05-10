#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <CLI11.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "Setups.hpp"

#include "ShaderCollection_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace AAP {
    void CLIShaderSetup(CLI::App& app) {
        CLI::App* shader = app.add_subcommand("shdr", "Composes a shader collection binary");

        static std::string materialName = "";
        shader->add_option("-n,--name", materialName, "Human-readable identifier for the owning object or material");

        static std::string shaderVersion;
        shader->add_option("-v,--version", shaderVersion, "Semantic version of this shader package (e.g. 1.0.3)");

        static std::string target_api;
        shader->add_option(
            "-t,--target", target_api, "Target graphics API / hardware capability level this set was compiled for");

        static bool isMeshPipeline = false;
        shader->add_flag(
            "-m,--mesh_pipeline", isMeshPipeline, "Uses a Mesh and Fragment shader only (and optionally a task one)");

        static std::vector<std::string> shader_files;
        shader
            ->add_option(
                "-f,--files",
                shader_files,
                "Shader files. If Mesh Pipeline enabled introduce the mesh shader and then fragment shader. If it's disabled introduce a vertex shader and a fragment one in this order.")
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

            // Shader collection fields
            flatbuffers::Offset<flatbuffers::String> name = builder.CreateString(materialName);
            flatbuffers::Offset<flatbuffers::String> version = builder.CreateString(shaderVersion);
            flatbuffers::Offset<flatbuffers::String> api = builder.CreateString(target_api);

            // Serialize the Fragment shader
            flatbuffers::Offset<flatbuffers::String> frag_name = builder.CreateString(shader_files[1]);
            std::vector<u8> frag_bytes = ReadFile(shader_files[1].c_str());
            flatbuffers::Offset<flatbuffers::Vector<u8>> frag_bytecode = builder.CreateVector(frag_bytes);
            flatbuffers::Offset<Axle::Shader> FragShader =
                Axle::CreateShader(builder,
                                   Axle::ShaderStage::ShaderStage_Fragment,
                                   frag_name,
                                   0,
                                   Axle::ShaderFormat::ShaderFormat_GLSL_Source,
                                   frag_bytecode,
                                   0);

            // Final Table
            flatbuffers::Offset<Axle::ShaderCollection> ShaderCollection;

            if (isMeshPipeline) {
                // Serialize the Mesh shader
                flatbuffers::Offset<flatbuffers::String> mesh_name = builder.CreateString(shader_files[0]);
                std::vector<u8> mesh_bytes = ReadFile(shader_files[0].c_str());
                flatbuffers::Offset<flatbuffers::Vector<u8>> mesh_bytecode = builder.CreateVector(mesh_bytes);
                flatbuffers::Offset<Axle::Shader> MeshShader =
                    Axle::CreateShader(builder,
                                       Axle::ShaderStage::ShaderStage_Mesh,
                                       mesh_name,
                                       0,
                                       Axle::ShaderFormat::ShaderFormat_GLSL_Source,
                                       mesh_bytecode,
                                       0);

                flatbuffers::Offset<Axle::MeshPipeline> MeshPipeline =
                    Axle::CreateMeshPipeline(builder, 0, MeshShader, FragShader);

                ShaderCollection = Axle::CreateShaderCollection(
                    builder, name, version, api, Axle::Pipeline_MeshPipeline, MeshPipeline.Union());
            } else {
                // Serialize the Vertex Shader
                flatbuffers::Offset<flatbuffers::String> vertx_name = builder.CreateString(shader_files[0]);
                std::vector<u8> vertx_bytes = ReadFile(shader_files[0].c_str());
                flatbuffers::Offset<flatbuffers::Vector<u8>> vertx_bytecode = builder.CreateVector(vertx_bytes);
                flatbuffers::Offset<Axle::Shader> VertxShader =
                    Axle::CreateShader(builder,
                                       Axle::ShaderStage::ShaderStage_Vertex,
                                       vertx_name,
                                       0,
                                       Axle::ShaderFormat::ShaderFormat_GLSL_Source,
                                       vertx_bytecode,
                                       0);

                flatbuffers::Offset<Axle::ClassicPipeline> ClassPipeline =
                    Axle::CreateClassicPipeline(builder, VertxShader, 0, 0, 0, FragShader);

                ShaderCollection = Axle::CreateShaderCollection(
                    builder, name, version, api, Axle::Pipeline_ClassicPipeline, ClassPipeline.Union());
            }

            builder.Finish(ShaderCollection);

            u8* buf = builder.GetBufferPointer();
            u32 size = builder.GetSize();

            WriteFile("test.shdrbin", buf, size);
        });
    }
} // namespace AAP
