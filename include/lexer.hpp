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


typedef enum {
    // primitives
    TK_STRING,
    TK_INT,
    TK_FLOAT,
    TK_ID,
    TK_TYPEID,
    
    // symbols
    TK_LBRACE,
    TK_RBRACE,
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_COMMA,
    TK_RESULTS,
    
    TK_CALL,
    TK_PACKAGE,
    TK_OFTYPE,
    TK_PARTIAL,
    TK_ASSIGN,
    TK_BANG,
    
    TK_PLUS,
    TK_MINUS,
    TK_MULTIPLY,
    TK_DIVIDE,
    TK_MOD,
    
    TK_LSHIFT,
    TK_RSHIFT,
    
    TK_LT,
    TK_LE,
    TK_GE,
    TK_GT,
    
    TK_EQ,
    TK_NOTEQ,
    TK_STEQ,
    TK_NSTEQ,
    
    TK_OR,
    TK_AND,
    TK_XOR,
    
    TK_UNIQ,
    TK_READONLY,
    TK_RECEIVER,
    
    // keywords
    TK_USE,
    TK_DECLARE,
    TK_TYPE,
    TK_LAMBDA,
    TK_TRAIT,
    TK_OBJECT,
    TK_ACTOR,
    TK_IS,
    TK_VAR,
    TK_DELEGATE,
    TK_NEW,
    TK_AMBIENT,
    TK_FUNCTION,
    TK_MESSAGE,
    TK_THROWS,
    TK_THROW,
    TK_RETURN,
    TK_BREAK,
    TK_CONTINUE,
    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_IN,
    TK_WHILE,
    TK_DO,
    TK_MATCH,
    TK_CASE,
    TK_AS,
    TK_CATCH,
    TK_ALWAYS,
    TK_THIS,
    TK_TRUE,
    TK_FALSE,
    
    // abstract
    TK_MODULE,
    TK_DECLAREMAP,
    TK_MAP,
    TK_TYPEBODY,
    TK_TYPECLASS,
    TK_FORMALARGS,
    TK_FIELD,
    TK_ARG,
    TK_ARGS,
    TK_BLOCK,
    TK_CASEVAR,
    TK_LIST,
    
    TK_EOF,
    
    null
} tok_type;


typedef struct Token {

    tok_type id;
    unsigned int line;
    unsigned int line_pos;
    
    union {
        std::string* string;
        double flt;
        unsigned int integer;
    };
    
} Token;

void token_free(Token*);


class Lexer {
private:
    std::string* m;
    unsigned int ptr;
    unsigned int len;
    
    unsigned int line;
    unsigned int line_pos;
    
    std::string* buffer;
    
    std::vector<error_t>* error_list;
    
    Lexer() { }
    
public:
    Lexer(std::string*,std::vector<error_t>*);
    Token* next();
    void adv(unsigned int);
    char look();
    std::string* copy();
    void string_terminate();
    void append(char);
    bool appendn(unsigned int);
    Token* token_new();
    void lexer_newline();
    void nested_comment();
    void line_comment();
    Token* lexer_slash();
    Token* lexer_string();
    Token* lexer_float(Token*, unsigned int);
    Token* lexer_id();
    std::vector<error_t>* lexer_errors();
    
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
};


#endif
