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
#include "CompilationUnit.h"
#include "Common.h"
#include "Error.h"

enum class Content {
    CN_VAR,
    CN_DELEGATE,
    CN_NEW,
    CN_AMBIENT,
    CN_FUNCTION,
    CN_MESSAGE
};

struct Variable {
    std::string name;
    std::vector<std::string> type;
    Variable(std::string n, std::vector<std::string> t) : name(n), type(t) {}
};

struct Delegate {
    std::string ID;
};

struct C_New {
};

struct Ambient {
};

struct Function {
    std::vector<Variable*> arguments;
    std::vector<Variable*> outputs;

    Function(std::vector<Variable*> a, std::vector<Variable*> o) :
        arguments(a), outputs(o) {}
};

struct Message {
};

struct ClassContents {
    AST* ast;
    Content type;

    // hacky subtyping
    union {
        Variable* variable;
        Delegate* delegate;
        C_New* c_new;
        Ambient* ambient;
        Function* function;
        Message* message;
    };

    ClassContents(AST* a, Variable* v)  :
        ast(a), type(Content::CN_VAR), variable(v) {}

    ClassContents(AST* a, Delegate* d)  :
        ast(a), type(Content::CN_DELEGATE), delegate(d) {}

    ClassContents(AST* a, C_New* c)     :
        ast(a), type(Content::CN_NEW), c_new(c) {}

    ClassContents(AST* a, Ambient* am)  :
        ast(a), type(Content::CN_AMBIENT), ambient(am) {}

    ClassContents(AST* a, Function* f)  :
        ast(a), type(Content::CN_FUNCTION), function(f) {}

    ClassContents(AST* a, Message* m)   :
        ast(a), type(Content::CN_MESSAGE), message(m) {}
};

class TypeChecker {
    private:
        const CompilationUnit* unit;
        std::vector<AST*> astList;
        std::set<FullAST*> fullASTList;
        std::vector<const Error> errorList;
        std::set<std::string> typeNames;

    public:
        explicit TypeChecker(CompilationUnit* _unit) :
            unit(_unit), astList(_unit->astList), errorList(), typeNames() {}
        void typeCheck();

    private:
        void topLevelTypes();
        void checkMixins();
        void recurseSingleTopAST(AST* ast, std::set<Type*> &typeList,
                                 std::set<CompilationUnit*> &imports);
        bool checkMixin(std::string mixin, FullAST* ast);
        void checkNameClashes();
        Type* newType(AST* ast, Kind k, std::set<ClassContents*> contents);
};

#endif  // CPP_INCLUDE_TYPECHECKER_H_
