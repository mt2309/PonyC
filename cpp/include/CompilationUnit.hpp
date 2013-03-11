#ifndef ponyC_CompilationUnit_hpp
#define ponyC_CompilationUnit_hpp

#include "common.hpp"


class CompilationUnit {

private:
        
    int stage;
    
public:
    std::vector<FullAST*> fullASTList;

    const std::string directoryName;
    
    std::vector<AST*> astList;
    
    CompilationUnit(std::string name, int _stage): stage(_stage), directoryName(name) {}
    
    void buildUnit();
};

#endif