// Copyright 2013 <Michael Thorpe>
//
//  lexer.hpp
//  ponyC
//
//  Created by Michael Thorpe on 12/11/2012.
//
//

#ifndef CPP_INCLUDE_LEXER_H_
#define CPP_INCLUDE_LEXER_H_

#include <vector>
#include <string>
#include "Error.h"
#include "Common.h"


typedef struct symbol_t {
    const std::string symbol;
    const TokenType id;
} symbol_t;


class Lexer {
    private:
        static const std::vector<const symbol_t> symbols2;
        static const std::vector<const symbol_t> symbols1;
        static const std::vector<const symbol_t> keywords;

        std::string fileName;
        std::string m;
        size_t ptr;
        size_t len;

        size_t line;
        size_t line_pos;

        std::string buffer;

        std::vector<Error> error_list;

    public:
        Lexer(std::string, std::string, std::vector<Error>);

        Token* next();
        void adv(size_t);
        char look();
        std::string copy();
        void string_terminate();
        void append(char c);
        bool appendn(size_t);
        Token* token_new();

    private:
        void step();
        std::string buff_copy();
        void push_error(std::string);
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
        Token* lexer_float(Token*, size_t);
        Token* lexer_id();
        void lexer_newline();
        void nested_comment();
        void line_comment();
};


#endif  // CPP_INCLUDE_LEXER_H_
