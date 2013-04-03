// Copyright 2013 <Michael Thorpe>
//
//  TraitTypeChecker.h
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#ifndef CPP_INCLUDE_TRAITTYPECHECKER_H_
#define CPP_INCLUDE_TRAITTYPECHECKER_H_

#include "TypeChecker.h"

class TraitTypeChecker {
private:
    TypeChecker* tc;

public:
    explicit TraitTypeChecker(TypeChecker* t) : tc(t) {}
    void typeCheck();
    
private:
    void checkMixins();
    bool checkMixin(std::string, Type&, FullAST*);

};

#endif  // CPP_INCLUDE_TRAITTYPECHECKER_H_

