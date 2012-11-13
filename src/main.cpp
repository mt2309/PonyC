#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "lexer.hpp"
#include "parser.hpp"
#include "typer.hpp"
#include "code_gen.hpp"

namespace po = boost::program_options;

using namespace std;

string stages = 
R"(Stage 1: lexer
Stage 2: parser
Stage 3: typer
Stage 4: code-gen)";

int main(int argc, char** argv) {

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

    std::cout << "Hello, world!" << std::endl;

    if (vm.count("help")) {
        cout << options << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (vm.count("version")) {
        cout << "1.0.0 - C++ barebones" << endl;
        exit(EXIT_SUCCESS);
    }
    

    // For now, we'll handle the problem of compiling a single pony file.
    
    string input_file;
    
    if (vm.count("input")) {
        input_file = vm["input"].as<vector<string>>().front();
    }
    else {
        cerr << "Error: no input file specified" << endl;
        exit(EXIT_FAILURE);
    }
    
    string program_text = read_file(input_file);

    // If there's a stage given, dump it to stdin
    // Perhaps output to a .lexer/.parser/.typer/.code_gen or similar file instead?
    if (vm.count("stage")) {
        switch(vm["stage"].as<int>()) {
            case 1:
                cout << lexer::lex();
                break;
            case 2:
                cout << parser(lexer());
                break;
            case 3:
                cout << typer(parser(lexer));
                break;
            case 4:
                cout << code_gen(typer(parser(lexer)));
                break;
            default:
                cerr << "Use of undefined stage" << endl;
                exit(EXIT_FAILURE);
        }
    }
    else {
        // Else compile the code! Lets do this!
        ofstream outfile;
        if (vm.count("output")) {
            outfile.open(vm["output"].as<string>());
        }
        else {
            // TODO:
            // This is not what we want - we'll be outputting llvm IR here
            // a.out is what gcc/clang use for binary files
            outfile.open("a.out");
        }
    }
    
    return EXIT_SUCCESS;
}

string read_file(string file_name) {
    ifstream infile(file_name);
    stringstream stream;
    stream << infile.rdbuf();
    
    return stream.str();
}