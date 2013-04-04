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

enum class Mode {
    READONLY,
    MUTABLE,
    IMMUTABLE,
    UNIQUE
};


struct ClassContents {
    std::string name;
    bool abstract;
    const AST* ast;
    
    ClassContents(std::string n, bool abs, const AST* a) : name(n), abstract(abs), ast(a) {}
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
    Field(std::string n, bool abs, std::vector<std::string> t, const AST* a) : ClassContents(n,abs,a), type(t) {}
};

struct Delegate : ClassContents {
    
    Delegate(std::string n, const AST* a) : ClassContents(n,false,a) {}
};

struct Constructor : ClassContents {
    Constructor(std::string n, bool abs, const AST* a) : ClassContents(n,abs,a) {}
};

struct Ambient : ClassContents {
    Ambient(std::string n, bool abs, const AST* a) : ClassContents(n,abs,a) {}
};

struct Function : ClassContents {
    Mode mode;
    std::vector<Parameter> arguments;
    std::vector<Parameter> outputs;
    
    Function(Mode m, bool abs, std::vector<Parameter> args, std::vector<Parameter> o, std::string n, const AST* a) :
        ClassContents(n,abs,a), mode(m), arguments(args), outputs(o) {}
};

struct Message : ClassContents {
    Message(std::string n, bool abs, const AST* a) : ClassContents(n,abs,a) {}
};
    
struct Type {
    
    std::string name;
    Kind kind;
    
    AST* ast;
    std::vector<std::string> mixins;
    std::vector<Type> fullyQualifiedMixins;
    std::set<ClassContents> contents;
    
    Type(std::string n, Kind k, AST* a, std::vector<std::string> m,
         std::set<ClassContents> c) :  name(n), kind(k), ast(a),
    mixins(m), fullyQualifiedMixins(), contents(c) {}
    
    Type(std::string n, Kind k, AST* a) : name(n), kind(k), ast(a) {}
    
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
