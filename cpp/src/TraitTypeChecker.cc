// Copyright 2013 <Michael Thorpe>
//
//  type_checker.cpp
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#include "TraitTypeChecker.h"

#include <assert.h>
#include <iostream>

#define debug(x)    (std::cout << x << std::endl)

bool TraitTypeChecker::checkMixin(std::string mixin, Type& t, FullAST ast) {
    // Look up type in AST List
    for (auto &f : this->tc->fullASTList) {
        for (auto &type : f.topLevelDecls) {
            if (type.name.compare(mixin) == 0) {
                t.fullyQualifiedMixins.push_back(type);
                debug(t.fullyQualifiedMixins.size());
                return true;
            }
        }
    }

    // Then look it up in the imports
    for (auto &compilationUnit : ast.imports) {
        for (auto &f : compilationUnit.fullASTList) {
            for (auto &type : f.topLevelDecls) {
                if (type.name.compare(mixin) == 0) {
                    t.fullyQualifiedMixins.push_back(type);
                    return true;
                }
            }
        }
    }
    debug("not found");
    return false;
}

void TraitTypeChecker::checkMixins() {
    printf("\n");
    for (auto &fullAST : this->tc->fullASTList) {
        std::cout << "Traitchecking file: " << fullAST.ast->t->fileName << std::endl;
        for (auto top : fullAST.topLevelDecls) {
            // For every mixin check its existence
            for (auto mixin : top.mixins) {
                if (!this->checkMixin(mixin, top, fullAST)) {
                    this->tc->errorList.push_back(Error(fullAST.ast->t->fileName,
                                                    top.ast->t->line,
                                                    top.ast->t->linePos,
                                                    ("Mixin " +
                                                    mixin +
                                                     " not found in scope")));
                }
            }
            assert(top.fullyQualifiedMixins.size() == top.mixins.size());
        }
    }
}

void TraitTypeChecker::typeCheck() {
    
    this->checkMixins();

    if (this->tc->errorList.size() > 0) {
        std::cout << "Errors detected in trait type checking" << std::endl;
    }

    for (auto error : this->tc->errorList) {
        std::cout << "Error at " << error.prog_name << "\t"
            << error.line << ":" << error.line_pos << "\t"
            << error.message << std::endl;
    }
}
