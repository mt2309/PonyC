//
//  type_checker.cpp
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#include "type_checker.hpp"
#include <iostream>
#include <stdlib.h>

static std::string extractName(AST* ast) {
    if (ast->t->id == TK_TYPEID) {
        return *ast->t->string;
    }
    
    std::string res;
    
    for (auto children: *ast->children) {
        if (children == nullptr)
            continue;
        res = extractName(children);
        if (res.compare("") != 0)
            return res;
    }
    
    return "";
}

static Type* newType(std::string name, std::string type, Kind k, AST* ast) {
    Type* t = (Type*)calloc(1, sizeof(Type));
    t->name = name;
    t->type = type;
    t->kind = k;
    t->ast = ast;
    return t;
}

static void recurseSingleAST(AST* ast, std::vector<Type*>* typeList) {

    if (ast == nullptr)
        return;
    
    switch (ast->t->id) {
        case TK_OBJECT:
            std::cout << "Object" << std::endl;
            typeList->push_back(newType(extractName(ast),"",TYPE_OBJECT, ast));
            break;
        case TK_TRAIT:
            std::cout << "Trait" << std::endl;
            typeList->push_back(newType(extractName(ast), "", TYPE_TRAIT, ast));
            break;
        case TK_ACTOR:
            std::cout << "Actor" << std::endl;
            typeList->push_back(newType(extractName(ast), "", TYPE_ACTOR, ast));
            break;
        case TK_DECLARE:
            std::cout << "Declare" << std::endl;
            typeList->push_back(newType(extractName(ast), "", TYPE_DECLARE, ast));
        default:
            // Not a top level declaration
            std::cout << "No top level" << std::endl;
            break;
    }
        
    recurseSingleAST(ast->sibling, typeList);
}

std::vector<Type*>* TypeChecker::topLevelTypes() {
    auto topLevel = new std::vector<Type*>();
    
    // All modules
    for (auto ast: *this->ast_list) {
        recurseSingleAST(ast,topLevel);
    }
    
    return topLevel;
}
