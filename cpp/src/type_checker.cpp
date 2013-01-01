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
#include <map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#include <boost/format.hpp>
#pragma GCC diagnostic pop

#define debug(x)    (std::cout << x << std::endl)

static FullAST* ASTnew(std::vector<Import*>* imports, AST* ast, std::vector<Type*>* topLevel) {
    FullAST* astImport = (FullAST*)calloc(1, sizeof(FullAST));
    astImport->imports = imports;
    astImport->ast = ast;
    astImport->topLevelDecls = topLevel;
    
    return astImport;
}

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

static Type* newType(AST* ast, std::string type, Kind k) {
    Type* t = (Type*)calloc(1, sizeof(Type));
    
    t->ast = ast;
    t->name = extractName(ast);
    t->type = type;
    t->kind = k;
    t->mixins = extractMixins(ast);
    return t;
}

static void recurseSingleTopAST(AST* ast,
                                std::vector<Type*>* typeList,
                                std::vector<Import*>* imports) {

    if (ast == nullptr)
        return;
    
    switch (ast->t->id) {
        case TK_OBJECT:
            typeList->push_back(newType(ast,"",TYPE_OBJECT));
            break;
        case TK_TRAIT:
            typeList->push_back(newType(ast,"", TYPE_TRAIT));
            break;
        case TK_ACTOR:
            typeList->push_back(newType(ast, "", TYPE_ACTOR));
            break;
        case TK_DECLARE:
            typeList->push_back(newType(ast, "", TYPE_DECLARE));
        case TK_USE:
        {
            Import* import = (Import*)calloc(1, sizeof(Import));
            import->importName = ast->children->at(1)->t->string;
            if (ast->children->at(0) != nullptr && ast->children->at(0)->t->id == TK_TYPEID) {
                import->importedAs = ast->children->at(0)->t->string;
            } else {
                import->importedAs = nullptr;
            }
            imports->push_back(import);
            break;
        }
        default:
            // Not a top level declaration
            break;
    }
    
    for (auto children: *ast->children) {
        recurseSingleTopAST(children, typeList, imports);
    }
    
    recurseSingleTopAST(ast->sibling, typeList, imports);
}

void TypeChecker::topLevelTypes() {
    auto fullASTs = new std::vector<FullAST*>();
    // mapping of type names to asts
    // Pretty sure this means one global namespace
    // Right now this will be broken, since we should consider the case of:
    // use List = 
    auto modules = new std::map<std::string, Type*>();
    
    // All compilation units
    for (auto ast: *this->ast_list) {
        auto topLevel = new std::vector<Type*>();
        auto imports = new std::vector<Import*>();

        recurseSingleTopAST(ast, topLevel, imports);
        
        for (auto type: *topLevel) {
            modules->insert(std::pair<std::string, Type*>(type->name,type));
        }
        
        auto fullAST = ASTnew(imports, ast, topLevel);
        fullASTs->push_back(fullAST);
    }
    
    
    // Verify each import maps to an AST node.
    for (auto ast : *fullASTs) {
        // Check imports
        for (auto import : *ast->imports) {
            debug(*import->importName);
            if (modules->at(*import->importName) == nullptr)
                this->error_list->push_back(*error_new(ast->ast->t->fileName, 1, 1,
                                                       (boost::format("Import %1% not found") % import).str()));
        }
    }
    
    //Check mixins are all valid
    for (auto ast: *fullASTs) {
        for (auto type: *ast->topLevelDecls) {
            // For each trait mixed in, check for presence in global types.
            for (auto mixin : *type->mixins) {
                bool found = false;
                // O(n^2)!
                for (auto t: *ast->imports) {
                    if (mixin.compare(*t->importName) == 0) {
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
                            *error_new(type->ast->t->fileName,
                                       type->ast->t->line,
                                       type->ast->t->line_pos,
                                       (boost::format("Trait %1% not found") % mixin).str()));
                }
            }
        }
    }
}

void TypeChecker::typeCheck() {
    this->topLevelTypes();
    
    if (this->error_list->size() > 0) {
        std::cout << "Errors detected in top level" << std::endl;
    }
    
    // do more type detection here
    
    for (auto error: *this->error_list) {
        std::cout << "Error at " << error.line << ":" << error.line_pos << "\t" << error.message << std::endl;
    }
}
