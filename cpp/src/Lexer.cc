//
//  lexer.cpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#include "Lexer.h"
#include "Error.h"

#include <assert.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"

const std::vector<const symbol_t> Lexer::symbols2 = {
    { "->", TokenType::TK_RESULTS },
    { "::", TokenType::TK_PACKAGE },

    { "<<", TokenType::TK_LSHIFT },
    { ">>", TokenType::TK_RSHIFT },

    { "==", TokenType::TK_EQ },
    { "!=", TokenType::TK_NOTEQ },
    { "#=", TokenType::TK_STEQ },
    { "~=", TokenType::TK_NSTEQ },

    { "[:", TokenType::TK_MODE },

    { "<=", TokenType::TK_LE },
    { ">=", TokenType::TK_GE },
};

const std::vector<const symbol_t> Lexer::symbols1 = {
    { "{", TokenType::TK_LBRACE },
    { "}", TokenType::TK_RBRACE },
    { "(", TokenType::TK_LPAREN },
    { ")", TokenType::TK_RPAREN },
    { "[", TokenType::TK_LBRACKET },
    { "]", TokenType::TK_RBRACKET },
    { ",", TokenType::TK_COMMA },

    { ".", TokenType::TK_CALL },
    { ":", TokenType::TK_OFTYPE },
    { "\\", TokenType::TK_PARTIAL },
    { "=", TokenType::TK_ASSIGN },
    { "!", TokenType::TK_BANG },

    { "+", TokenType::TK_PLUS },
    { "-", TokenType::TK_MINUS },
    { "*", TokenType::TK_MULTIPLY },
    { "/", TokenType::TK_DIVIDE },
    { "%", TokenType::TK_MOD },

    { "<", TokenType::TK_LT },
    { ">", TokenType::TK_GT },

    { "|", TokenType::TK_OR },
    { "&", TokenType::TK_AND },
    { "^", TokenType::TK_XOR },

    { "@", TokenType::TK_UNIQ },
    { "~", TokenType::TK_MUT },

    { ";", TokenType::TK_SCOLON }
};

const std::vector<const symbol_t> Lexer::keywords = {
    { "use", TokenType::TK_USE },
    { "declare", TokenType::TK_DECLARE },
    { "type", TokenType::TK_TYPE },
    { "lambda", TokenType::TK_LAMBDA },
    { "trait", TokenType::TK_TRAIT },
    { "object", TokenType::TK_OBJECT },
    { "actor", TokenType::TK_ACTOR },
    { "is", TokenType::TK_IS },
    { "var", TokenType::TK_VAR },
    { "delegate", TokenType::TK_DELEGATE },
    { "new", TokenType::TK_NEW },
    { "ambient", TokenType::TK_AMBIENT },
    { "function", TokenType::TK_FUNCTION },
    { "message", TokenType::TK_MESSAGE },
    { "throws", TokenType::TK_THROWS },
    { "throw", TokenType::TK_THROW },
    { "return", TokenType::TK_RETURN },
    { "break", TokenType::TK_BREAK },
    { "continue", TokenType::TK_CONTINUE },
    { "if", TokenType::TK_IF },
    { "else", TokenType::TK_ELSE },
    { "for", TokenType::TK_FOR },
    { "in", TokenType::TK_IN },
    { "while", TokenType::TK_WHILE },
    { "do", TokenType::TK_DO },
    { "match", TokenType::TK_MATCH },
    { "case", TokenType::TK_CASE },
    { "as", TokenType::TK_AS },
    { "catch", TokenType::TK_CATCH },
    { "always", TokenType::TK_ALWAYS },
    { "this", TokenType::TK_THIS },
    { "true", TokenType::TK_TRUE },
    { "false", TokenType::TK_FALSE },
};

void Lexer::push_error(std::string err) {
    this->error_list.push_back(Error(this->fileName, this->line, this->line_pos, err));
}

static bool isSymbol(char c) {
    return ((c >= '!') && (c <= '.'))
    || ((c >= ':') && (c <= '@'))
    || ((c >= '[') && (c <= '^'))
    || ((c >= '{') && (c <= '~'));
}

void Lexer::adv(size_t count) {
    assert(this->len >= count);

    this->ptr += count;
    this->len -= count;
    this->line_pos += count;
}

void Lexer::step() {
    this->adv(1);
}

char Lexer::look() {
    assert(this->len > 0);
    return this->m.at(this->ptr);
}

std::string Lexer::buff_copy() {
    if (this->buffer.size() == 0)
        return nullptr;
    std::string ret = std::string(this->buffer);
    this->buffer = std::string("");
    return std::string(ret);
}

void Lexer::string_terminate() {
    this->push_error("Unterminated string");
    this->ptr += this->len;
    this->len = 0;
    this->buffer = std::string("");
}

void Lexer::append(char c) {
    this->buffer.append(1, c);
}

bool Lexer::appendn(size_t length) {

    size_t prev_pos = this->ptr;
    uint32_t c = 0;

    if (this->len < length) {
        this->string_terminate();
        return false;
    }

    this->adv(length);

    for (size_t i = prev_pos; i < (len+prev_pos); i++) {
        c <<= 4;

        if( (this->m.at(i) >= '0') && (this->m.at(i) <= '9') ) {
            c += this->m.at(i) - '0';
        } else if( (this->m.at(i) >= 'a') && (this->m.at(i) <= 'f') ) {
            c += this->m.at(i) - 'a';
        } else if( (this->m.at(i) >= 'A') && (this->m.at(i) <= 'F') ) {
            c += this->m.at(i) - 'A';
        } else {
            this->push_error("Escape sequence contains non-hexadecimal value " + std::to_string(c));
            return false;
        }
    }

    if( c <= 0x7F ) {
        this->append(c & 0x7F );
    } else if(c <= 0x7FF ) {
        this->append(0xC0 | (c >> 6) );
        this->append(0x80 | (c & 0x3F) );
    } else if(c <= 0xFFFF ) {
        this->append(0xE0 | (c >> 12) );
        this->append(0x80 | ((c >> 6) & 0x3F) );
        this->append(0x80 | (c & 0x3F) );
    } else if(c <= 0x10FFFF ) {
        this->append(0xF0 | (c >> 18) );
        this->append(0x80 | ((c >> 12) & 0x3F) );
        this->append(0x80 | ((c >> 6) & 0x3F) );
        this->append(0x80 | (c & 0x3F) );
    } else {
        this->push_error("Escape sequence exceeds unicode range (0x10FFFF)");
        return false;
    }

    return true;
}

Token* Lexer::token_new() {
    Token* tok = new Token(this->fileName, this->line, this->line_pos);

    return tok;
}

void Lexer::lexer_newline() {
    this->line++;
    this->line_pos = 0;
}

void Lexer::nested_comment() {
    size_t depth = 1;

    while (depth > 0) {

        if (this->len <= 1) {
            this->push_error("Nested comment doesn't terminate");
            this->ptr += this->len;
            this->len = 0;
            return;
        }

        if (this->look() == '*')
        {
            this->step();
            if (this->look() == '/')
            {
                depth--;
            }

        }
        else if (this->look() == '/') {
            this->step();

            if (this->look() == '*')
            {
                depth++;
            }

        }
        else if (this->look() == '\n') {
            this->lexer_newline();
        }

        this->step();
    }
}

void Lexer::line_comment() {
    while ((this->len > 0) && (this->look() != '\n')) {
        this->step();
    }
}

Token* Lexer::lexer_slash() {
    this->step();

    if ( this->len > 0) {
        if (this->look() == '*') {
            this->step();
            this->nested_comment();
            return nullptr;
        } else if (this->look() == '/') {
            this->step();
            this->line_comment();
            return nullptr;
        }
    }

    Token* t = this->token_new();
    t->id = TokenType::TK_DIVIDE;

    return t;
}

Token* Lexer::lexer_string() {

    this->step();
    assert(this->buffer.size() == 0);

    while (true) {
        if (this->len == 0)
        {
            this->string_terminate();
            return nullptr;
        }
        else if (this->look() == '\"')
        {
            this->step();
            Token* t = this->token_new();
            t->id = TokenType::TK_STRING;
            t->string = this->buff_copy();
            return t;
        }
        else if (this->look() == '\\')
        {
            if (this->len < 2)
            {
                this->string_terminate();
                return nullptr;
            }

            this->step();
            char c = this->look();
            this->step();

            switch (c) {
                case 'a':
                    this->append(0x07);
                    break;

                case 'b':
                    this->append(0x08);
                    break;

                case 'f':
                    this->append(0x0C);
                    break;

                case 'n':
                    this->append(0x0A);
                    break;

                case 'r':
                    this->append(0x0D);
                    break;

                case 't':
                    this->append(0x09);
                    break;

                case 'v':
                    this->append(0x0B);
                    break;

                case '\"':
                    this->append(0x22);
                    break;

                case '\\':
                    this->append(0x5C);
                    break;

                case '0':
                    this->append(0x00);
                    break;

                case 'x':
                    this->appendn(2);
                    break;

                case 'u':
                    this->appendn(4);
                    break;

                case 'U':
                    this->appendn(6);
                    break;

                default:
                    this->push_error("Invalid escape sequence: " + std::to_string(c));
                    break;
            }
        } else {
            this->append(this->look());
            this->step();
        }
    }
}

Token* Lexer::real(size_t v) {
    double d = v;
    size_t digits = -1;
    int e = 0;
    bool error = false;
    char c;

    if (this->look() == '.') {
        this->step();
        digits = 0;
    }

    while (this->len > 0) {
        c = this->look();

        if ((c >= '0') || (c == '9'))
        {
            d = (d * 10) + (c - '0');
            digits++;
            e--;
        }
        else if ((c=='e') || (c=='E'))
        {
            break;
        }
        else if (c == '_') {
            //skip
        }
        else if (isalpha(c)) {
            if (!error) {
                this->push_error("Invalid digit in real number: " + std::to_string(c));
                error = true;
            }
        } else {
            break;
        }

        this->step();
    }

    if (digits == 0) {
        this->push_error("Real number has no digits following '.'");
        error = true;
    }

    if((this->len > 0) && ((this->look() == 'e') || (this->look() == 'E'))) {
        this->step();
        digits = 0;

        if (this->len == 0) {
            this->push_error("Real number doesn't terminate");
            return nullptr;
        }

        c = this->look();
        bool neg = false;
        int n = 0;

        if ((c == '+') || (c == '-')) {
            this->step();
            neg = (c == '-');

            if (this->len == 0) {
                this->push_error("Real number doesn't terminate");
                return nullptr;
            }
        }

        while (this->len > 0) {
            c = this->look();

            if ((c >= '0') && (c <= '9')) {
                n = (n * 10) + (c - '0');
                digits++;
            } else if( c == '_' ) {
                // skip
            } else if( isalpha( c ) ) {
                if (!error) {
                    this->push_error("Invalid digit in exponent: " + std::to_string(c));
                    error = true;
                }
            } else {
                break;
            }

            this->step();
        }

        if (neg)
        {
            e -= n;
        }
        else
        {
            e += n;
        }

        if (digits == 0) {
            this->push_error("Exponent has no digits");
            error = true;
        }
    }

    if (error) {return nullptr;}

    Token* t = this->token_new();
    t->id = TokenType::TK_FLOAT;
    t->flt = d * pow(10.0, e);
    return t;
}

Token* Lexer::hexadecimal() {
    size_t v = 0;
    bool error = false;
    char c;

    while(this->len > 0 )
    {
        c = this->look();

        if((c >= '0') && (c <= '9'))
        {
            v = (v * 16) + (c - '0');
        } else if( (c >= 'a') && (c <= 'z') ) {
            v = (v * 16) + (c - 'a');
        } else if( (c >= 'A') && (c <= 'Z') ) {
            v = (v * 16) + (c - 'A');
        } else if( c == '_' ) {
            // skip
        } else if( isalpha( c ) ) {
            if( !error ) {
                this->push_error("Invalid digit in hexadecimal number: " + std::to_string(c));
                error = true;
            }
        } else {
            break;
        }

        this->step();
    }

    if( error ) { return nullptr; }

    Token* t = this->token_new();
    t->id = TokenType::TK_INT;
    t->integer = v;
    return t;
}

Token* Lexer::decimal() {

    size_t v = 0;
    bool error = false;
    char c;

    while(this->len > 0)
    {
        c = this->look();

        if( (c >= '0') && (c <= '9') )
        {
            v = (v * 10) + (c - '0');
        } else if( (c == '.') || (c == 'e') || (c == 'E') ) {
            return this->real(v);
        } else if( c == '_' ) {
            // skip
        } else if( isalnum( c ) ) {
            if( !error )
            {
                this->push_error("Invalid digit in decimal number: " + std::to_string(c));
                error = true;
            }
        } else {
            break;
        }

        this->step();
    }

    if( error ) { return nullptr; }

    Token* t = this->token_new();
    t->id = TokenType::TK_INT;
    t->integer = v;
    return t;
}

Token* Lexer::binary() {

    size_t v = 0;
    bool error = false;
    char c;

    while( this->len > 0 )
    {
        c = this->look();

        if( (c >= '0') && (c <= '1') )
        {
            v = (v * 2) + (c - '0');
        } else if( c == '_' ) {
            // skip
        } else if( isalnum( c ) ) {
            if( !error )
            {
                this->push_error("Invalid digit in binary number: " + std::to_string(c));
                error = true;
            }
        } else {
            break;
        }

        this->step();
    }

    if( error ) { return nullptr; }

    Token* t = this->token_new();
    t->id = TokenType::TK_INT;
    t->integer = v;
    return t;
}

Token* Lexer::number() {

    if( this->look() == '0' )
    {
        this->step();

        if( this->len > 0 )
        {
            char c = this->look();

            switch( c )
            {
                case 'x': return this->hexadecimal();
                case 'b': return this->binary();
                default: return this->decimal();
            }
        }
    }
    return this->decimal();
}

void Lexer::read_id() {
    char c;

    while( this->len > 0 )
    {
        c = this->look();

        if((c == '_') || isalnum( c ))
        {
            this->append(c);
            this->step();
        } else {
            break;
        }
    }
}

Token* Lexer::identifier() {

    Token* t = this->token_new();

    this->read_id();

    for ( auto p : keywords) {
        if (!this->buffer.compare(p.symbol)) {
            t->id = p.id;
            this->buffer = std::string("");
            return t;
        }
    }

    t->id = TokenType::TK_ID;
    t->string = this->buff_copy();
    return t;
}

Token* Lexer::type_id() {
    this->read_id();

    Token* t = this->token_new();
    t->id = TokenType::TK_TYPEID;
    t->string = this->buff_copy();

    return t;
}

Token* Lexer::symbol() {

    Token* t;
    char sym[2];

    sym[0] = this->look();
    this->step();

    if( this->len > 1 ) {
        sym[1] = this->look();

        if(isSymbol(sym[1]))
        {
            for( auto p : symbols2)
            {
                if((sym[0] == p.symbol[0]) && (sym[1] == p.symbol[1]))
                {
                    this->step();
                    t = this->token_new();
                    t->id = p.id;
                    return t;
                }
            }
        }
    }

    for( auto p : symbols1)
    {
        if( sym[0] == p.symbol[0])
        {
            t = this->token_new();
            t->id = p.id;
            return t;
        }
    }

    this->push_error("Unknown symbol: " + std::to_string(sym[0]));
    return nullptr;
}

Token* Lexer::next() {
    Token* t = nullptr;

    while((t == nullptr) && (this->len > 0))
    {
        char c = this->look();

        switch( c )
        {
        case '\n':
            this->lexer_newline();
            this->step();
            break;

        case '\r':
        case '\t':
        case ' ':
            this->step();
            break;

        case '/':
                t = this->lexer_slash();
            break;

        case '\"':
            t = this->lexer_string();
            break;

        default:
            if( isdigit( c ) )
            {
                t = this->number();
            }
            else if( islower( c ) || (c == '_') )
            {
                t = this->identifier();
            }
            else if( isupper( c ) )
            {
                t = this->type_id();
            }
            else if( isSymbol( c ) )
            {
                t = this->symbol();
            }
            else
            {
                this->push_error("Unrecognized character: " + std::to_string(c));
                this->step();
            }
        }
    }

    if(t == nullptr) {

        t = this->token_new();
        t->id = TokenType::TK_EOF;
    }

    return t;
}

Lexer::Lexer(std::string _fileName, std::string input, std::vector<Error> list) {
    this->fileName = _fileName;
    this->m = input;
    this->len = input.size();
    this->line = 1;
    this->line_pos = 1;
    this->ptr = 0;
    this->error_list = list;
    this->buffer = std::string("");
}

#pragma GCC diagnostic pop

