#ifndef ponyC_CompilationUnit_hpp
#define ponyC_CompilationUnit_hpp

#include "common.hpp"

typedef std::string program_name;

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