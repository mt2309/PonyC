// Copyright 2013 <Michael Thorpe>
//
//  type_checker.h
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#ifndef CPP_INCLUDE_TYPECHECKER_H_
#define CPP_INCLUDE_TYPECHECKER_H_

#include <vector>
#include <set>
#include <string>

#include "Typer.h"

#include "CompilationUnit.h"
#include "Common.h"
#include "Error.h"

class TypeChecker {
private:
    const CompilationUnit unit;
    std::vector<AST*> astList;
    std::set<FullAST*> fullASTList;
    std::vector<const Error> errorList;
    std::set<std::string> typeNames;

public:
    explicit TypeChecker(CompilationUnit _unit) :
        unit(_unit), astList(_unit.astList), errorList(), typeNames() {}
    void typeCheck();

private:
    void topLevelTypes();
    void checkMixins();
    void recurseSingleTopAST(AST* const ast, std::set<Type> &t, std::set<CompilationUnit> &i);
    bool checkMixin(std::string mixin, FullAST* ast);
    void checkNameClashes();
    Type* newType(AST* const ast, Kind k, std::set<ClassContents> contents);
    Mode getMode(AST* const ast);
    ClassContents* newVarContent(AST* const ast);
    ClassContents* newFunctionContent(AST* const ast);
    void getTypeList(AST* const ast, std::vector<std::string> &types);
    void getArgsList(AST* const ast, std::vector<Parameter> &types);
    std::set<ClassContents> collectFunctions(AST* const ast);
};

#endif  // CPP_INCLUDE_TYPECHECKER_H_
