// Copyright 2013 <Michael Thorpe>

#ifndef CPP_INCLUDE_COMPILATIONUNIT_H_
#define CPP_INCLUDE_COMPILATIONUNIT_H_

#include <vector>
#include <string>

#include "Common.h"

typedef std::string program_name;

class CompilationUnit {
    private:
        int stage;

    public:
        std::vector<FullAST*> fullASTList;

        const std::string directoryName;

        std::vector<AST*> astList;

        void buildUnit();
        CompilationUnit(std::string name, int _stage):
            stage(_stage), directoryName(name) {}
};

#endif  // CPP_INCLUDE_COMPILATIONUNIT_H_
