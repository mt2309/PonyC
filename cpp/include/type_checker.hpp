//
//  type_checker.h
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#ifndef __ponyC__type_checker__
#define __ponyC__type_checker__

#include <vector>
#include "CompilationUnit.hpp"
#include "common.hpp"
#include "error.hpp"


class TypeChecker {
    const CompilationUnit* unit;
    std::vector<AST*> astList;
    std::vector<FullAST*> fullASTList;
    std::vector<const error_t> errorList;

public:
    TypeChecker(CompilationUnit* _unit) : unit(_unit), astList(_unit->astList), errorList() {}

    void typeCheck();

private:
    // First pass
    void topLevelTypes();
    void checkMixins();
    void recurseSingleTopAST(AST* ast, std::vector<Type*> typeList, std::vector<CompilationUnit*> imports);
    bool checkMixin(std::string mixin, FullAST* ast);
    void checkNameClashes();
};

#endif /* defined(__ponyC__type_checker__) */
