//
//  error.cpp
//  ponyC
//
//  Created by Michael Thorpe on 14/11/2012.
//
//

#include "error.hpp"
#include <stdlib.h>
#include <string>

const error_t* error_new(std::string prog_name, size_t line, size_t pos, std::string str) {
    error_t* err = new error_t;
    err->prog_name = prog_name;
    err->line = line;
    err->line_pos = pos;
    err->message = str;    
    return err;
}
