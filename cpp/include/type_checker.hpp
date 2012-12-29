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
#include "ast.hpp"
#include "error.hpp"

class TypeChecker {
    std::vector<AST*>* ast_list;
    std::vector<const error_t>* error_list;
    
public:
    TypeChecker(std::vector<AST*>* ast) {
        this->ast_list = ast;
        this->error_list = new std::vector<const error_t>();
    };
    
    void typeCheck();
    
private:
    // First pass
    void topLevelTypes();
    
};

#endif /* defined(__ponyC__type_checker__) */
