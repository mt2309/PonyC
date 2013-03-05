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
    std::vector<AST*> ast_list;
    std::vector<const error_t> error_list;

public:
    TypeChecker(CompilationUnit* _unit) : unit(_unit), ast_list(_unit->astList) {}

    void typeCheck();

private:
    // First pass
    void topLevelTypes();
    void recurseSingleTopAST(AST* ast, std::vector<Type*> typeList, std::vector<Import*> imports);


};

#endif /* defined(__ponyC__type_checker__) */
