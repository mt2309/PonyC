// Copyright 2013 <Michael Thorpe>

#ifndef CPP_INCLUDE_LOADER_H_
#define CPP_INCLUDE_LOADER_H_

#include <string>

#include "CompilationUnit.h"

// Currently no assertion that imports are a DAG
class Loader {
  public:
    static CompilationUnit* Load(std::string, std::string);
    static CompilationUnit* Load(std::string, int stage);
};

#endif  // CPP_INCLUDE_LOADER_H_
