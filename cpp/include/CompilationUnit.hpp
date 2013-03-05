#ifndef ponyC_CompilationUnit_hpp
#define ponyC_CompilationUnit_hpp

#include "common.hpp"


class CompilationUnit {

private:
    std::vector<FullAST*> fullASTList;
        
    int stage;
    
public:
    
    const std::string directoryName;
    
    std::vector<AST*> astList;
    
    CompilationUnit(std::string name, int _stage): directoryName(name), stage(_stage) {}
    
    void buildUnit();
};

#endif