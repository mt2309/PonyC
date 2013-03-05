#include <iostream>
#include <tuple>
#include "CompilationUnit.hpp"
#include "type_checker.hpp"
#include "parser.hpp"

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