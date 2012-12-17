//
//  typer.hpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef ponyC_typer_hpp
#define ponyC_typer_hpp

#include "parser.hpp"

namespace typer {
    
    class typed_ast: AST {
    
        // beep bop stuff happens
        friend std::ostream& operator<<(std::ostream& os, const typed_ast& ast);
    };
    
    typed_ast type(AST* ast);
}

#endif
