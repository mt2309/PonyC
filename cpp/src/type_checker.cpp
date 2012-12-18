//
//  type_checker.cpp
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#include "type_checker.hpp"

std::vector<Type*>* TypeChecker::topLevelTypes() {
    auto topLevel = new std::vector<Type*>();
    
    for (auto ast: *this->ast_list) {
        
    }
    
    return topLevel;
}
