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

static std::string extractName(AST* ast) {
    if (ast->t->id == TokenType::TK_TYPEID) {
        return ast->t->string;
    }
    
    std::string res;
    
    for (auto children: ast->children) {
        if (children == nullptr)
            continue;
        res = extractName(children);
        if (res.compare("") != 0) {
            return res;
        }
    }
    
    return "";
}

static void extractMixins(AST* ast, std::vector<std::string> &mixins) {
    assert(   ast->t->id == TokenType::TK_OBJECT
           || ast->t->id == TokenType::TK_TRAIT
           || ast->t->id == TokenType::TK_ACTOR
           || ast->t->id == TokenType::TK_DECLARE);
    
    // Why 2, because why not
    if (ast->children.at(2) != nullptr) {
        for (auto child: ast->children.at(2)->children) {
            if (child != nullptr) {
                // Heh, hacky
                mixins.push_back(child->children.at(0)->t->string);
            }
        }
    }
    
}

Type* TypeChecker::newType(AST* ast, Kind k, std::set<ClassContents*> contents) {
    auto mixins = std::vector<std::string>();
    auto name = extractName(ast);
    
    extractMixins(ast, mixins);
    return new Type(extractName(ast),k,ast,mixins,contents);
}

static void getArgsList(AST* ast, std::vector<Variable*> &inputs) {
    AST* current = ast;
    
    while (current != nullptr) {
        assert(current->t->id == TokenType::TK_ARGS);
        
        AST* a = current->children.at(0);
        
        if (a != nullptr)
            inputs.push_back(new Variable(a->children.at(0)->t->string, a->children.at(1)->t->string));
        
        current = current->sibling;
    }
}

static ClassContents* newVarContent(AST* ast) {
    Variable* v = new Variable(ast->children.at(0)->t->string, ast->children.at(0)->t->string);
    ClassContents* c = new ClassContents(ast, v);
    return c;
}

static ClassContents* newDelegateContent(AST* ast) {
    ClassContents* c = new ClassContents(ast, new Delegate);
    return c;
}

static ClassContents* newConstructorContents(AST* ast) {
    ClassContents* c = new ClassContents(ast, new C_New);
    return c;
}

static ClassContents* newAmbientContents(AST* ast) {
    ClassContents* c = new ClassContents(ast, new Ambient);
    return c;
}

static ClassContents* newFunctionContent(AST* ast) {
    auto inputs = std::vector<Variable*>();
    auto outputs = std::vector<Variable*>();
    getArgsList(ast->children.at(3), inputs); getArgsList(ast->children.at(4), outputs);
    Function* f = new Function(inputs, outputs);
    ClassContents* c = new ClassContents(ast, f);
    return c;
}

static ClassContents* newMessageContent(AST* ast) {
    ClassContents* c = new ClassContents(ast, new Message);
    return c;
}

static std::set<ClassContents*> collectFunctions(AST* ast) {
    auto contents = std::set<ClassContents*>();
    
    AST* node = ast;
    
    while (node != nullptr) {
        switch (node->t->id) {
            case TokenType::TK_VAR:
                debug("var declaration");
                contents.insert(newVarContent(node));
                break;
            case TokenType::TK_DELEGATE:
                debug("delegate");
                contents.insert(newDelegateContent(node));
                break;
            case TokenType::TK_NEW:
                debug("constructor");
                contents.insert(newConstructorContents(node));
                break;
            case TokenType::TK_AMBIENT:
                debug("ambient");
                contents.insert(newAmbientContents(node));
                break;
            case TokenType::TK_FUNCTION:
                debug("function");
                contents.insert(newFunctionContent(node));
                break;
            case TokenType::TK_MESSAGE:
                debug("message");
                contents.insert(newMessageContent(node));
                break;
            default:
                break;
        }
        
        node = node->sibling;
    }
    
    return contents;
}

void TypeChecker::recurseSingleTopAST(AST* ast,
                                std::set<Type*> &typeList,
                                std::set<CompilationUnit*> &imports) {

    if (ast == nullptr)
        return;
    
    switch (ast->t->id) {
        case TokenType::TK_OBJECT:
            typeList.insert(newType(ast, Kind::TYPE_OBJECT,collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_TRAIT:
            typeList.insert(newType(ast, Kind::TYPE_TRAIT,collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_ACTOR:
            typeList.insert(newType(ast, Kind::TYPE_ACTOR,collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_DECLARE:
            // Are declarations types? (yes - mappings from one type to another)
            typeList.insert(newType(ast, Kind::TYPE_DECLARE,collectFunctions(ast)));
            break;
        case TokenType::TK_USE:
        {            
            std::string importName = ast->children.at(1)->t->string;
            auto package = Loader::Load(this->unit->directoryName, importName);
            package->buildUnit();

            // For now don't both with the type-id
            
            imports.insert(package);
            break;
        }
        case TokenType::TK_MODULE:
            break;
        default:
            // Not a top level declaration
            return;
    }
    
    for (auto children: ast->children) {
        recurseSingleTopAST(children, typeList, imports);
    }
    
    recurseSingleTopAST(ast->sibling, typeList, imports);
}

bool TypeChecker::checkMixin(std::string mixin, FullAST* ast) {
    
    // Look up type in AST List
    for (auto f : this->fullASTList) {
        for (auto type : f->topLevelDecls) {
            if (type->name.compare(mixin) == 0) {
                return true;
            }
        }
    }
    
    // Then look it up in the imports
    for (auto compilationUnit : ast->imports) {
        for (auto f : compilationUnit->fullASTList) {
            for (auto type : f->topLevelDecls) {
                if (type->name.compare(mixin) == 0) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void TypeChecker::checkMixins() {
    for (auto fullAST : this->fullASTList) {
        for (auto top : fullAST->topLevelDecls) {
            
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
    
    for (auto ast : this->fullASTList) {
        for (auto type : ast->topLevelDecls) {
            auto name = type->name;
            
            if (typeNames.find(name) == typeNames.end()) {
                typeNames.insert(name);
            }
            else {
                this->errorList.push_back(*error_new(type->ast->t->fileName, type->ast->t->line, type->ast->t->linePos,
                                                     (boost::format("Name clash %1% found multiple times in the current module") % name).str()));
            }
        }        
    }
}

void TypeChecker::topLevelTypes() {
    fullASTList = std::set<FullAST*>();
    
    // All compilation units
    for (auto ast: this->astList) {
        
        std::cout << "Typechecking file: " << ast->t->fileName << std::endl;
        
        // Collections holding topLevel types
        // and the imports.
        auto topLevel = std::set<Type*>();
        auto imports = std::set<CompilationUnit*>();

        recurseSingleTopAST(ast, topLevel, imports);
        
        auto fullAST = new FullAST(ast, imports, topLevel);
        
        fullASTList.insert(fullAST);
    }
    
    this->checkMixins();
    this->checkNameClashes();
}

void TypeChecker::typeCheck() {
    this->topLevelTypes();
    
    if (this->errorList.size() > 0) {
        std::cout << "Errors detected in top level types" << std::endl;
    }
    
    // do more type detection here
    
    for (auto error: this->errorList) {
        std::cout << "Error at " << error.prog_name << "\t" << error.line << ":" << error.line_pos << "\t" << error.message << std::endl;
    }
}
