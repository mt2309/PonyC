//
//  error.hpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef ponyC_error_hpp
#define ponyC_error_hpp

#include <string>

typedef struct error_t {
    std::string prog_name;
    size_t line;
    size_t line_pos;
    std::string message;
} error_t;

const error_t* error_new(std::string, size_t, size_t, std::string);

#endif
