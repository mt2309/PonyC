// Copyright 2013 <Michael Thorpe>
//
//  type_checker.h
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#ifndef CPP_INCLUDE_TOPTYPECHECKER_H_
#define CPP_INCLUDE_TOPTYPECHECKER_H_

#include <vector>
#include <set>
#include <string>

#include "Typer.h"
#include "TypeChecker.h"

#include "CompilationUnit.h"
#include "Common.h"
#include "Error.h"

class TopTypeChecker {
private:
    TypeChecker* tc;

public:
    explicit TopTypeChecker(TypeChecker* t) : tc(t) {}
    void typeCheck();

private:
    void topLevelTypes();
    void recurseSingleTopAST(AST* const ast, std::set<Type> &t, std::set<CompilationUnit> &i);
    void checkNameClashes();
    Type* newType(AST* const ast, Kind k, std::set<ClassContents> contents);
    Mode getMode(AST* const ast);
    ClassContents* newVarContent(AST* const ast);
    ClassContents* newFunctionContent(AST* const ast);
    void getTypeList(AST* const ast, std::vector<std::string> &types);
    void getArgsList(AST* const ast, std::vector<Parameter> &types);
    std::set<ClassContents> collectFunctions(AST* const ast, Kind k);
};

#endif  // CPP_INCLUDE_TOPTYPECHECKER_H_
