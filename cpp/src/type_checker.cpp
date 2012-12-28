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
#include <boost/format.hpp>

#define debug(x)    (std::cout << x << std::endl)

typedef struct FullAST {
    AST* ast;
    std::vector<std::string>* imports;
    std::vector<Type*>* topLevelDecls;
} FullAST;

static std::string extractName(AST* ast) {
    if (ast->t->id == TK_TYPEID) {
        return *ast->t->string;
    }
    
    std::string res;
    
    for (auto children: *ast->children) {
        if (children == nullptr)
            continue;
        res = extractName(children);
        if (res.compare("") != 0) {
            return res;
        }
    }
    
    return "";
}

static void extractImports(AST* ast,std::vector<std::string>* imports) {
    if (ast == nullptr)
        return;
    
    if (ast->t->id == TK_USE) {
        // detect imports of kind:
        // use FullyQualifedName
        // TODO:
        // use Type = FullyQualifiedName
        imports->push_back(*ast->children->at(1)->t->string);
    }
    
    for (auto children: *ast->children) {
        extractImports(children, imports);
    }
    
    extractImports(ast->sibling, imports);

}

static std::vector<std::string>* extractMixins(AST* ast) {
    assert(ast->t->id == TK_OBJECT
           || ast->t->id == TK_TRAIT
           || ast->t->id == TK_ACTOR
           || ast->t->id == TK_DECLARE);
    auto mixins = new std::vector<std::string>();
    
    // Why 2, because why not
    if (ast->children->at(2) != nullptr) {
        for (auto child: *ast->children->at(2)->children) {
            if (child != nullptr) {
                // Heh, hacky
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
            typeList->push_back(newType(extractName(ast),"",TYPE_OBJECT, ast));
            break;
        case TK_TRAIT:
            typeList->push_back(newType(extractName(ast), "", TYPE_TRAIT, ast));
            break;
        case TK_ACTOR:
            typeList->push_back(newType(extractName(ast), "", TYPE_ACTOR, ast));
            break;
        case TK_DECLARE:
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
    auto ASTimports = new std::vector<FullAST*>();
    // All modules
    for (auto ast: *this->ast_list) {
        auto topLevel = new std::vector<Type*>();
        recurseSingleTopAST(ast,topLevel);
        
        auto import = new std::vector<std::string>();
        extractImports(ast, import);
        
        auto astImport = (FullAST*)calloc(1, sizeof(FullAST));
        astImport->imports = import;
        astImport->ast = ast;
        astImport->topLevelDecls = topLevel;
        
        ASTimports->push_back(astImport);
    }
    
    //Check mixins are all valid
    for (auto ast: *ASTimports) {
        for (auto type: *ast->topLevelDecls) {
            // For each trait mixed in, check for presence in global types.
            for (auto mixin : *type->mixins) {
                bool found = false;
                // O(n^2)!
                for (auto t: *ast->imports) {
                    debug(t);
                    if (mixin.compare(t) == 0) {
                        debug("Found\timport");
                        found = true;
                    }
                }
                for (auto t: *ast->topLevelDecls) {
                    if ((mixin.compare(t->name) == 0) && (t->kind == TYPE_TRAIT)) {
                        found = true;
                    }
                }
                if (!found) {
                    this->error_list->push_back(
                            *error_new(type->ast->t->line,
                            type->ast->t->line_pos,
                            (boost::format("Trait %1% not found") % mixin).str()));
                }
            }
        }
    }
}

void TypeChecker::typeCheck() {
    auto topLevelTypes = this->topLevelTypes();
    
    if (this->error_list->size() > 0) {
        std::cout << "Errors detected in top level" << std::endl;
    }
    
    // do more type detection here
    
    for (auto error: *this->error_list) {
        std::cout << "Error at " << error.line << ":" << error.line_pos << "\t" << error.message << std::endl;
    }
}
