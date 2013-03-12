#include <iostream>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>

#include "CompilationUnit.hpp"
#include "type_checker.hpp"
#include "parser.hpp"

static std::string read_file(fs::path path) {
    std::ifstream infile(path.string().c_str());
    std::stringstream stream;
    stream << infile.rdbuf();
    
    return std::string(stream.str());
}

static void recurse_dir(fs::path p, std::vector<std::tuple<program_name,std::string>>* vec) {
    
    if (!fs::exists(p)) {
        std::cout << "Directory " << p.root_path() << " not found" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    fs::directory_iterator end_it;
    
    for(fs::directory_iterator itr(p); itr != end_it; itr++) {
        if (itr->path().extension() == FILE_EXTENSION) {
            vec->push_back(make_tuple(itr->path().string(),
                                      read_file(itr->path())));
        }
    }
}

static std::vector<std::tuple<program_name,std::string>>* get_files_directory(std::string dir) {
    fs::path p(dir);
    auto vec = new std::vector<std::tuple<program_name,std::string>>();
    
    if (!fs::is_directory(p) && p.extension() == FILE_EXTENSION) {
        vec->push_back(make_tuple(p.string(), read_file(p)));
    }
    else {
        recurse_dir(p, vec);
    }
    
    return vec;
}

static std::vector<FullAST*> previouslyParsedUnits;

void CompilationUnit::buildUnit() {
    auto programText = get_files_directory(directoryName);
    
    for(auto & prog : *programText) {
        
        Parser* p = new Parser(std::get<0>(prog),std::get<1>(prog));
        AST* ast;
        std::cout << "Parsing file: " << std::get<0>(prog) << std::endl;
        ast = p->parse();
        if (p->error_list.size() > 0) {
            std::cout << "Errors detected, continuing parsing remainder" << std::endl;
            continue;
        }
        
        astList.push_back(ast);
        delete p;
    }
    
    if (stage == 1)
        exit(EXIT_SUCCESS);
    
    //Type check!
    auto typeChecker = new TypeChecker(this);
    typeChecker->typeCheck();
}