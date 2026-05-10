#pragma once

#include "CLI11.hpp"

namespace AAP {
    /**
     * Creates the shdr subcommand which is used for serializing shaders
     *
     * @param app A reference to the CLI app
     * */
    void CLIShaderSetup(CLI::App& app);
} // namespace AAP
