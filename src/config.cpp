#include "config.hpp"

#include <iostream>
#include <stdexcept>

Config parse_args(int argc, char** argv)
{
    Config cfg;

    for (int i = 1; i < argc; ++i) {

        std::string arg = argv[i];

        auto require_value = [&](const std::string& name)
        {
            if (i + 1 >= argc) {
                throw std::runtime_error(
                    "Missing value after " + name);
            }

            return std::string(argv[++i]);
        };

        if (arg == "--nx") {

            cfg.nx = std::stoi(require_value(arg));

        } else if (arg == "--ny") {

            cfg.ny = std::stoi(require_value(arg));

        } else if (arg == "--steps") {

            cfg.steps = std::stoi(require_value(arg));

        } else if (arg == "--dt") {

            cfg.dt = std::stod(require_value(arg));

        } else if (arg == "--kappa") {

            cfg.kappa = std::stod(require_value(arg));

        } else if (arg == "--output") {

            cfg.output = require_value(arg);

        } else {

            throw std::runtime_error(
                "Unknown argument: " + arg);
        }
    }

    return cfg;
}

void print_config(const Config& cfg)
{
    std::cout << "====================================\n";
    std::cout << "miniClimate-HPC\n";
    std::cout << "====================================\n";

    std::cout << "Grid      : "
              << cfg.nx << " x "
              << cfg.ny << "\n";

    std::cout << "Steps     : "
              << cfg.steps << "\n";

    std::cout << "dt        : "
              << cfg.dt << "\n";

    std::cout << "kappa     : "
              << cfg.kappa << "\n";

    std::cout << "Output    : "
              << cfg.output << "\n";

    std::cout << "====================================\n";
}