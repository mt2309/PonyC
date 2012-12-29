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
    unsigned int line;
    unsigned int line_pos;
    std::string message;
} error_t;

const error_t* error_new(std::string, unsigned int, unsigned int, std::string);

#endif
