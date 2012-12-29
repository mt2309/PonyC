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
#include <string>
#include "error.hpp"
#include "common.hpp"


typedef struct symbol_t {
    const std::string symbol;
    tok_type id;
} symbol_t;


class Lexer {
private:
    std::string fileName;
    std::string* m;
    size_t ptr;
    size_t len;

    size_t line;
    size_t line_pos;

    std::string* buffer;

    std::vector<error_t>* error_list;

public:
    Lexer(std::string,std::string*,std::vector<error_t>*);
    Token* next();
    void adv(unsigned int);
    char look();
    std::string* copy();
    void string_terminate();
    void append(char);
    bool appendn(size_t);
    Token* token_new();

private:
    void step();
    std::string* buff_copy();
    void push_error(std::string err);
    Token* real(size_t v);
    Token* hexadecimal();
    Token* decimal();
    Token* binary();
    Token* number();
    void read_id();
    Token* identifier();
    Token* type_id();
    Token* symbol();
    Token* lexer_slash();
    Token* lexer_string();
    Token* lexer_float(Token*, unsigned int);
    Token* lexer_id();
    void lexer_newline();
    void nested_comment();
    void line_comment();
};


#endif
