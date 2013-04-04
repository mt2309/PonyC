// Copyright 2013 <Michael Thorpe>
//
//  TypeChecker.h
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef CPP_INCLUDE_TYPECHECKER_H_
#define CPP_INCLUDE_TYPECHECKER_H_

#include <vector>
#include <set>

#include "Common.h"
#include "Typer.h"
#include "CompilationUnit.h"
#include "Error.h"

class TypeChecker {
public:
    const CompilationUnit unit;
    std::vector<AST*> astList;
    std::set<FullAST> fullASTList;
    std::vector<const Error> errorList;
    std::set<std::string> typeNames;
    
    explicit TypeChecker(CompilationUnit _unit) :
        unit(_unit), astList(_unit.astList), errorList(), typeNames() {}
};


#endif  // CPP_INCLUDE_TYPECHECKER_H_
