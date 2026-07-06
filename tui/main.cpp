#include "commands.h"
#include "logging/logger.h"
#include "utils/utils.h"
#include "constants.h"
#include <filesystem>
#include <iostream>

static void ensure_initialized() {
    if (!std::filesystem::exists(get_config_path(CONFIG_FILE))) {
        init();
    }
}

int main(int argc, char* argv[]) {
    if (argc > 3) {
        std::cerr << "Usage: lmcli [COMMAND] [SUBCOMMAND]\n";
        std::cerr << "See 'lmcli help' for available commands and subcommands.\n";
        return 1;
    }

    const std::string command = argc > 1 ? argv[1] : "start";

    if (command == "start") ensure_initialized();

    if (!logger_init()) {
        std::cerr << "Warning: config not loaded, logging enabled by default.\n";
    }

    if (command == "start") {
        start();
    }
    else if (command == "help") {
        help();
    }
    else if (command == "init") {
        init();
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "See 'lmcli help' for available commands.\n";
        return 1;
    }

    return 0;
}
