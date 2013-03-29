// Copyright 2013 <Michael Thorpe>
//
//  Typer.h
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef CPP_INCLUDE_TYPER_H_
#define CPP_INCLUDE_TYPER_H_

#include <string>
#include <set>
#include <vector>

#include "Common.h"

typedef class CompilationUnit CompilationUnit;


struct ClassContents {
    std::string name;
    AST* ast;
    
    ClassContents(std::string n, AST* a) : name(n), ast(a) {}
};

inline bool operator<(const ClassContents& a, const ClassContents& b) {
    return a.name < b.name;
}
    
struct Parameter {
    std::string name;
    std::vector<std::string> type;
    Parameter(std::string n, std::vector<std::string> t) : name(n), type(t) {}
};
    
struct Field : ClassContents {
    std::vector<std::string> type;
    Field(std::string n, std::vector<std::string> t, AST* a) : ClassContents(n,a), type(t) {}
};

struct Delegate : ClassContents {
    
    Delegate(std::string n, AST* a) : ClassContents(n,a) {}
};

struct Constructor : ClassContents {
    Constructor(std::string n, AST* a) : ClassContents(n,a) {}
};

struct Ambient : ClassContents {
    Ambient(std::string n, AST* a) : ClassContents(n,a) {}
};

struct Function : ClassContents {
    std::vector<Parameter> arguments;
    std::vector<Parameter> outputs;
    
    Function(std::vector<Parameter> args, std::vector<Parameter> o, std::string n, AST* a) :
    ClassContents(n,a), arguments(args), outputs(o) {}
};

struct Message : ClassContents {
    Message(std::string n, AST* a) : ClassContents(n,a) {}
};


struct Type {
    std::string name;
    
    Kind kind;
    
    AST* ast;
    std::vector<std::string> mixins;
    std::set<ClassContents> contents;
    
    Type(std::string n, Kind k, AST* a, std::vector<std::string> m,
         std::set<ClassContents> c) :  name(n), kind(k), ast(a),
    mixins(m), contents(c) {}
    
    Type(std::string n, Kind k) :   name(n), kind(k), ast(nullptr), mixins(),
    contents() {}
};

inline bool operator<(const Type& a, const Type& b) {
    return a.name < b.name;
}
    
    
struct FullAST {
    AST* ast;
    std::set<CompilationUnit> imports;
    std::set<Type> topLevelDecls;
    
    FullAST(AST* a, std::set<CompilationUnit> i,
            std::set<Type> t) : ast(a), imports(i), topLevelDecls(t) {}
};
    
inline bool operator<(const FullAST& a, const FullAST& b) {
    return a.ast->t->fileName < b.ast->t->fileName;
}


#endif  // CPP_INCLUDE_TYPER_H_