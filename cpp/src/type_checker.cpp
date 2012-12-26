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
#include <assert.h>

#define debug(x)    (std::cout << x << std::endl)

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

static std::vector<std::string>* extractMixins(AST* ast) {
    assert(ast->t->id == TK_OBJECT
           || ast->t->id == TK_TRAIT
           || ast->t->id == TK_ACTOR
           || ast->t->id == TK_DECLARE);
    auto mixins = new std::vector<std::string>();
    
    if (ast->children->at(2) != nullptr) {
        for (auto child: *ast->children->at(2)->children) {
            if (child != nullptr) {
                mixins->push_back(*child->children->at(0)->t->string);
            }
        }
    }
    
    
    return mixins;
}

static Type* newType(std::string name, std::string type, Kind k, AST* ast) {
    Type* t = (Type*)calloc(1, sizeof(Type));
    t->name = name;
    t->type = type;
    t->kind = k;
    t->ast = ast;
    t->mixins = extractMixins(ast);
    return t;
}

static void recurseSingleTopAST(AST* ast, std::vector<Type*>* typeList) {

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
            break;
    }
    
    for (auto children: *ast->children) {
        recurseSingleTopAST(children, typeList);
    }
    
    recurseSingleTopAST(ast->sibling, typeList);
}

std::vector<Type*>* TypeChecker::topLevelTypes() {
    auto topLevel = new std::vector<Type*>();
    
    // All modules
    for (auto ast: *this->ast_list) {
        recurseSingleTopAST(ast,topLevel);
    }
    
    return topLevel;
}

void TypeChecker::typeCheck() {
    auto topLevelTypes = this->topLevelTypes();
}
