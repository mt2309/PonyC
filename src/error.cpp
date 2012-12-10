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

const error_t* error_new(unsigned int line, unsigned int pos, std::string str) {
    error_t* err = (error_t*)calloc(1,sizeof(error_t));
    err->line = line;
    err->line_pos = pos;
    err->message = str;    
    return err;
}