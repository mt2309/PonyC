#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common.hpp"

void token_free(Token* token) {
  if( token == nullptr ) { return; }

  free(token);
}

void ast_free(AST* ast) {
  if (ast == nullptr)
    return;
  if (ast->t != nullptr) { token_free(ast->t); }

  for (size_t i = 0; i < AST_SLOTS; i++) {
    ast_free(ast->children.at(i));
  }

  ast_free(ast->sibling);
}

static std::string read_file(fs::path path) {
    std::ifstream infile(path.string().c_str());
    std::stringstream stream;
    stream << infile.rdbuf();
    
    return std::string(stream.str());
}

void recurse_dir(fs::path p, std::vector<std::tuple<program_name,std::string>>* vec) {

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

std::vector<std::tuple<program_name,std::string>>* get_files_directory(std::string dir) {
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