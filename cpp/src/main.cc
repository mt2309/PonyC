// Copyright 2013 <Michael Thorpe>

#include <tuple>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconditional-uninitialized"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wextra-semi"
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "Loader.h"

namespace po = boost::program_options;

int main(int argc, char** argv) {
    std::string stages = std::string("(Stage 1: parser\n")
                       + "Stage 2: typer\n"
                       + "Stage 3: code-gen\n)";

    po::options_description options("Allowed Options");
    options.add_options()
        ("help", "print usage message")
        ("version", "Version")
        ("stage", po::value<int>(),
         (std::string("what stage of the compiler to run ")
                      + "to (mostly used for debugging)\n" + stages).c_str())
        ("input", po::value< std::vector<std::string>>(),
                "List of files/directories to compile")
        ("output", po::value<std::string>(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << options << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (vm.count("version")) {
        std::cout << "1.0.0 - C++ barebones" << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (!vm.count("input")) {
        std::cerr << "Error: no input file specified" << std::endl;
        exit(EXIT_FAILURE);
    }

    int stage = INT_MAX;

    if (vm.count("stage"))
        stage = vm["stage"].as<int>();

    // Discussion needed on multi-file compilation
    std::string input = vm["input"].as<std::vector<std::string>>().front();

    auto unit = Loader::Load(input, stage);

    if (unit == nullptr) {
        std::cerr << "Starting directory: "
                  << input << " was not found!" << std::endl;
    }

    unit->buildUnit();

    return EXIT_SUCCESS;
}
