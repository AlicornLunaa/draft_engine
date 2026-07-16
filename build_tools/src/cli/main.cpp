#include "commands.hpp"

#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <validate|pack|export> ...\n", argv[0]);
        return 1;
    }

    std::string command = argv[1];
    if (command == "validate") return run_validate(argc, argv);
    if (command == "pack") return run_pack(argc, argv);
    if (command == "export") return run_export(argc, argv);

    std::fprintf(stderr, "Unknown command \"%s\", expected \"validate\", \"pack\", or \"export\"\n", command.c_str());
    return 1;
}
