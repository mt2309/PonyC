// Copyright 2013 <Michael Thorpe>
//
//  Primitives.h
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef CPP_INCLUDE_PRIMITIVES_H_
#define CPP_INCLUDE_PRIMITIVES_H_

#include <set>

#include "Common.h"

const std::set<Type> primitives = {
    Type("Int",Kind::TYPE_PRIMITIVE),
    Type("UInt",Kind::TYPE_PRIMITIVE),
    Type("Int32",Kind::TYPE_PRIMITIVE),
    Type("String",Kind::TYPE_PRIMITIVE)
};

#endif  // CPP_INCLUDE_PRIMTIVES_H_

