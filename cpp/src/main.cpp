#include <iostream>
#include <fstream>
#include <tuple>

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

#include "Loader.hpp"

namespace po = boost::program_options;

using namespace std;


int main(int argc, char** argv) {
    string stages =
    R"(Stage 1: parser
    Stage 2: typer
    Stage 3: code-gen)";

    po::options_description options("Allowed Options");
    options.add_options()
        ("help", "print usage message")
        ("version", "Version")
        ("stage", po::value<int>(), (string("what stage of the compiler to run to (mostly used for debugging)\n") + stages).c_str())
        ("input", po::value< vector<string>>(), "List of files/directories to compile")
        ("output", po::value<string>(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << options << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (vm.count("version")) {
        cout << "1.0.0 - C++ barebones" << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (!vm.count("input")) {
        cerr << "Error: no input file specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    int stage = INT_MAX;
    
    if (vm.count("stage"))
        stage = vm["stage"].as<int>();
    
    // Discussion needed on multi-file compilation
    string input = vm["input"].as<vector<string>>().front();
    
    auto unit = Loader::Load(input, stage);
    
    if (unit == nullptr) {
        cerr << "Starting directory: " << input << " was not found!" << endl;
    }
    
    unit->buildUnit();
    
    return EXIT_SUCCESS;
}
