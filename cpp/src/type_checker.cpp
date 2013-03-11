//
//  type_checker.cpp
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#include "type_checker.hpp"
#include "CompilationUnit.hpp"
#include "Loader.hpp"
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <set>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#include <boost/format.hpp>
#pragma GCC diagnostic pop

#define debug(x)    (std::cout << x << std::endl)

static FullAST* ASTnew(std::vector<CompilationUnit*>* imports, AST* ast, std::vector<Type*>* topLevel) {
    FullAST* astImport = new FullAST;
    astImport->imports = imports;
    astImport->ast = ast;
    astImport->topLevelDecls = topLevel;
    
    return astImport;
}

static std::string* extractName(AST* ast) {
    if (ast->t->id == TK_TYPEID) {
        return ast->t->string;
    }
    
    std::string* res;
    
    for (auto children: ast->children) {
        if (children == nullptr)
            continue;
        res = extractName(children);
        if (res->compare("") != 0) {
            return res;
        }
    }
    
    return nullptr;
}

static std::vector<std::string> extractMixins(AST* ast) {
    assert(ast->t->id == TK_OBJECT
           || ast->t->id == TK_TRAIT
           || ast->t->id == TK_ACTOR
           || ast->t->id == TK_DECLARE);
    auto mixins = std::vector<std::string>();
    
    // Why 2, because why not
    if (ast->children.at(2) != nullptr) {
        for (auto child: ast->children.at(2)->children) {
            if (child != nullptr) {
                // Heh, hacky
                mixins.push_back(*child->children.at(0)->t->string);
            }
        }
    }
    
    return mixins;
}

static Type* newType(AST* ast, Kind k, std::vector<ClassContents*> contents) {
    Type* t = new Type;
    
    t->ast = ast;
    t->name = extractName(ast);
    t->kind = k;
    t->mixins = extractMixins(ast);
    t->contents = contents;
    return t;
}

static ClassContents* newContents(AST* ast) {
    ClassContents* c = (ClassContents*)calloc(1, sizeof(ClassContents));
    c->ast = ast;
    return c;
}

static std::vector<ClassContents*> collectFunctions(AST* ast) {
    auto contents = std::vector<ClassContents*>();
    
    AST* node = ast;
    
    while (node != nullptr) {
        switch (node->t->id) {
            case TK_VAR:
                debug("var declaration");
                contents.push_back(newContents(node));
                break;
            case TK_DELEGATE:
                debug("delegate");
                break;
            case TK_NEW:
                debug("constructor");
                break;
            case TK_AMBIENT:
                debug("ambient");
                break;
            case TK_FUNCTION:
                debug("function");
                break;
            case TK_MESSAGE:
                debug("message");
                break;
            default:
                break;
        }
        
        node = node->sibling;
    }
    
    return contents;
}

void TypeChecker::recurseSingleTopAST(AST* ast,
                                std::vector<Type*> typeList,
                                std::vector<CompilationUnit*> imports) {

    if (ast == nullptr)
        return;
    
    switch (ast->t->id) {
        case TK_OBJECT:
            typeList.push_back(newType(ast,TYPE_OBJECT,collectFunctions(ast->children.at(3)->children.at(0))));
            return;
        case TK_TRAIT:
            typeList.push_back(newType(ast,TYPE_TRAIT,collectFunctions(ast->children.at(3)->children.at(0))));
            return;
        case TK_ACTOR:
            typeList.push_back(newType(ast, TYPE_ACTOR,collectFunctions(ast->children.at(3)->children.at(0))));
            return;
        case TK_DECLARE:
            // Are declarations types? (yes - mappings from one type to another)
            typeList.push_back(newType(ast, TYPE_DECLARE,collectFunctions(ast)));
            return;
        case TK_USE:
        {            
            std::string importName = *ast->children.at(1)->t->string;
            auto package = Loader::Load(this->unit->directoryName, importName);
            package->buildUnit();

            // For now don't both with the type-id
            
            imports.push_back(package);
            return;
        }
        default:
            // Not a top level declaration
            break;
    }
    
    for (auto children: ast->children) {
        recurseSingleTopAST(children, typeList, imports);
    }
    
    recurseSingleTopAST(ast->sibling, typeList, imports);
}

bool TypeChecker::checkMixin(std::string mixin, FullAST* ast) {
    // Look up type in AST List
    for (auto f : fullASTList) {
        for (auto type : *f->topLevelDecls) {
            if (type->name->compare(mixin) == 0) {
                return true;
            }
        }
    }
    
    // Then look it up in the imports
    for (auto compilationUnit : *ast->imports) {
        for (auto f : compilationUnit->fullASTList) {
            for (auto type : *f->topLevelDecls) {
                if (type->name->compare(mixin) == 0) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void TypeChecker::checkMixins() {
    for (auto fullAST : fullASTList) {
        for (auto top : *fullAST->topLevelDecls) {
            
            // For every mixin check its existence
            for (auto mixin : top->mixins) {
                if (!this->checkMixin(mixin, fullAST)) {
                    this->errorList.push_back(*error_new(fullAST->ast->t->fileName, top->ast->t->line, top->ast->t->linePos,
                                                        (boost::format("Mixin %1% not found in current path") % mixin).str()));
                }
            }
        }
    }
}

void TypeChecker::checkNameClashes() {
    std::set<std::string> names;
    
    for (auto ast : this->fullASTList) {
        for (auto type : *ast->topLevelDecls) {
            auto name = type->name;
            debug(name);
            
            if (names.find(*name) == names.end()) {
                names.insert(*name);
                debug(name);
            }
            else {
                this->errorList.push_back(*error_new(type->ast->t->fileName, type->ast->t->line, type->ast->t->linePos,
                                                     (boost::format("Name clash %1% not found in current path") % name).str()));
            }
        }
    }
}

void TypeChecker::topLevelTypes() {
    fullASTList = std::vector<FullAST*>();
    
    // All compilation units
    for (auto ast: this->astList) {
        
        std::cout << "Typechecking file: " << ast->t->fileName << std::endl;
        
        // Collections holding topLevel types
        // and the imports.
        auto topLevel = new std::vector<Type*>;
        auto imports = new std::vector<CompilationUnit*>;

        recurseSingleTopAST(ast, *topLevel, *imports);
        
        auto fullAST = ASTnew(imports, ast, topLevel);
        
        fullASTList.push_back(fullAST);
    }
    
    this->checkMixins();
    this->checkNameClashes();
}

void TypeChecker::typeCheck() {
    this->topLevelTypes();
    
    if (this->errorList.size() > 0) {
        std::cout << "Errors detected in top level" << std::endl;
    }
    
    // do more type detection here
    
    for (auto error: this->errorList) {
        std::cout << "Error at " << error.line << ":" << error.line_pos << "\t" << error.message << std::endl;
    }
}
