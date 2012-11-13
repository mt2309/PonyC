//
//  lexer.hpp
//  ponyC
//
//  Created by Michael Thorpe on 12/11/2012.
//
//

#ifndef ponyC_lexer_hpp
#define ponyC_lexer_hpp

#include <vector>

namespace lexer {
    
    enum tok_type {
        USE,
        DECLARE,
        DECLAREMAP,
        TYPEDEF,
        TYPENAME,
        TYPE,
        PARTIAL,
        LITERAL,
        FORMALARGS,
        ARGS,
        MUTABLE
    };
        
    struct token {
        std::string tok_name;
        tok_type type;
    };

        
    // returns a vector of pony tokens, given a single pony file
    std::vector<token> lex(std::string);

}



#endif
