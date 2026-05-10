#include <CLI11.hpp>

#include "Setups.hpp"

using namespace AAP;

int main(int argc, char** argv) {
    CLI::App app("Axle's Asset Pipeline", "AAP");
    argv = app.ensure_utf8(argv);
    app.require_subcommand(1);

    // SHADER COMPILATION ------------------
    CLIShaderSetup(app);
    // -----------------------------------

    CLI11_PARSE(app, argc, argv);
    return 0;
}
