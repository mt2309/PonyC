// Copyright 2013 <Michael Thorpe>
//
//  error.hpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef CPP_INCLUDE_ERROR_H_
#define CPP_INCLUDE_ERROR_H_

#include <string>

struct Error {
    std::string prog_name;
    size_t line;
    size_t line_pos;
    std::string message;

    Error(std::string n, size_t l, size_t pos, std::string str) :
    prog_name(n), line(l), line_pos(pos), message(str) {}
};

#endif  // CPP_INCLUDE_ERROR_H_
