#pragma once 


#include <string>

struct Config {
    int nx = 256;
    int ny = 256;
    int steps = 500;

    double dt = 1e-6;
    double kappa = 0.01;

    std::string output = "final_field.dat";
};

Config parse_args(int argc, char** argv);
void print_config(const Config& cfg);