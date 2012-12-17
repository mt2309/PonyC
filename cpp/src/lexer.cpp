//
//  lexer.cpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#include "lexer.hpp"
#include "error.hpp"

#include <assert.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <boost/format.hpp>

static const std::vector<symbol_t> symbols2 = {
    { "->", TK_RESULTS },
    { "::", TK_PACKAGE },
    
    { "<<", TK_LSHIFT },
    { ">>", TK_RSHIFT },
    
    { "==", TK_EQ },
    { "!=", TK_NOTEQ },
    { "#=", TK_STEQ },
    { "~=", TK_NSTEQ },
    
    { "[:", TK_MODE },
    
    { "<=", TK_LE },
    { ">=", TK_GE },
};

static const std::vector<symbol_t> symbols1 = {
    { "{", TK_LBRACE },
    { "}", TK_RBRACE },
    { "(", TK_LPAREN },
    { ")", TK_RPAREN },
    { "[", TK_LBRACKET },
    { "]", TK_RBRACKET },
    { ",", TK_COMMA },
    
    { ".", TK_CALL },
    { ":", TK_OFTYPE },
    { "\\", TK_PARTIAL },
    { "=", TK_ASSIGN },
    { "!", TK_BANG },
    
    { "+", TK_PLUS },
    { "-", TK_MINUS },
    { "*", TK_MULTIPLY },
    { "/", TK_DIVIDE },
    { "%", TK_MOD },
    
    { "<", TK_LT },
    { ">", TK_GT },
    
    { "|", TK_OR },
    { "&", TK_AND },
    { "^", TK_XOR },
    
    { "@", TK_UNIQ },
    { "~", TK_MUT },
    
    { ";", TK_SCOLON }
};

static const std::vector<symbol_t> keywords = {
    { "use", TK_USE },
    { "declare", TK_DECLARE },
    { "type", TK_TYPE },
    { "lambda", TK_LAMBDA },
    { "trait", TK_TRAIT },
    { "object", TK_OBJECT },
    { "actor", TK_ACTOR },
    { "is", TK_IS },
    { "var", TK_VAR },
    { "delegate", TK_DELEGATE },
    { "new", TK_NEW },
    { "ambient", TK_AMBIENT },
    { "function", TK_FUNCTION },
    { "message", TK_MESSAGE },
    { "throws", TK_THROWS },
    { "throw", TK_THROW },
    { "return", TK_RETURN },
    { "break", TK_BREAK },
    { "continue", TK_CONTINUE },
    { "if", TK_IF },
    { "else", TK_ELSE },
    { "for", TK_FOR },
    { "in", TK_IN },
    { "while", TK_WHILE },
    { "do", TK_DO },
    { "match", TK_MATCH },
    { "case", TK_CASE },
    { "as", TK_AS },
    { "catch", TK_CATCH },
    { "always", TK_ALWAYS },
    { "this", TK_THIS },
    { "true", TK_TRUE },
    { "false", TK_FALSE },
};

void Lexer::push_error(std::string err) {
    std::cout << "Got an error: " << err << std::endl;
    this->error_list->push_back(*error_new(this->line, this->line_pos, err));
}

static bool isSymbol(char c) {
    return ((c >= '!') && (c <= '.'))
    || ((c >= ':') && (c <= '@'))
    || ((c >= '[') && (c <= '^'))
    || ((c >= '{') && (c <= '~'));
}

void Lexer::adv(unsigned int count) {
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
    std::cout << "Looking at: " << this->m->at(this->ptr) << std::endl;
    return this->m->at(this->ptr);
}

std::string* Lexer::buff_copy() {
    if (this->buffer->size() == 0)
        return nullptr;
    std::string* ret = new std::string(*this->buffer);
    delete this->buffer;
    this->buffer = new std::string("");
    return ret;
}

void Lexer::string_terminate() {
    this->push_error("Unterminated string");
    this->ptr += this->len;
    this->len = 0;
    delete this->buffer;
    this->buffer = new std::string("");
}

void Lexer::append(char c) {
    std::cout << "Appending " << c << std::endl;
    this->buffer->append(1, c);
}

bool Lexer::appendn(size_t len) {
    
    size_t prev_pos = this->ptr;
    uint32_t c = 0;
    
    if (this->len < len) {
        this->string_terminate();
        return false;
    }
    
    this->adv(len);
    
    for (unsigned int i = prev_pos; i < (len+prev_pos); i++) {
        
        std::cout << "Appending " << len << " chars" << std::endl;
        
        c <<= 4;
        
        if( (this->m->at(i) >= '0') && (this->m->at(i) <= '9') ) {
            c += this->m->at(i) - '0';
        } else if( (this->m->at(i) >= 'a') && (this->m->at(i) <= 'f') ) {
            c += this->m->at(i) - 'a';
        } else if( (this->m->at(i) >= 'A') && (this->m->at(i) <= 'F') ) {
            c += this->m->at(i) - 'A';
        } else {
            this->push_error((boost::format("Escape sequence contains non-hexadecimal value %1%") % c).str());
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
    Token* tok = (Token*)calloc(1, sizeof(Token));
    tok->line = this->line;
    tok->line_pos = this->line_pos;
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
    t->id = TK_DIVIDE;
    
    return t;
}

Token* Lexer::lexer_string() {
    
    this->step();
    assert(this->buffer->size() == 0);
    
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
            t->id = TK_STRING;
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
                    this->push_error((boost::format("Invalid escape sequence: %1%") % c).str());
                    break;
            }
        } else {
            this->append(this->look());
            this->step();
        }
    }
    return nullptr;
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
                this->push_error((boost::format(
                                "Invalid digit in real number: %1%") % c).str());
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
                    this->push_error((boost::format("Invalid digit in exponent: %1%") % c).str());
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
    t->id = TK_FLOAT;
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
                this->push_error((boost::format("Invalid digit in hexadecimal number: %1%") % c ).str());
                error = true;
            }
        } else {
            break;
        }
        
        this->step();
    }
    
    if( error ) { return nullptr; }
    
    Token* t = this->token_new();
    t->id = TK_INT;
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
                this->push_error((boost::format("Invalid digit in decimal number: %1%") % c ).str());
                error = true;
            }
        } else {
            break;
        }
        
        this->step();
    }
    
    if( error ) { return nullptr; }
    
    Token* t = this->token_new();
    t->id = TK_INT;
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
                this->push_error((boost::format("Invalid digit in binary number: %1%") % c ).str());
                error = true;
            }
        } else {
            break;
        }
        
        this->step();
    }
    
    if( error ) { return nullptr; }
    
    Token* t = this->token_new();
    t->id = TK_INT;
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
        if (!this->buffer->compare(p.symbol)) {
            t->id = p.id;
            delete this->buffer;
            this->buffer = new std::string("");
            return t;
        }
    }
    
    t->id = TK_ID;
    t->string = this->buff_copy();
    return t;
}

Token* Lexer::type_id() {
    this->read_id();
    
    Token* t = this->token_new();
    t->id = TK_TYPEID;
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
    
    this->push_error((boost::format("Unknown symbol: %1%") % sym[0]).str());
    return nullptr;
}

Token* Lexer::next() {
    Token* t = nullptr;
    
    while((t == nullptr) && (this->len > 0))
    {
        char c = this->look();
        
        std::cout << "Character next = " << c << std::endl;
        
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
                this->push_error((boost::format("Unrecognized character: %1%") % c ).str());
                this->step();
            }
        }
    }
    
    if(t == nullptr) {
        
        std::cout << "t came out null" << std::endl;
        
        t = this->token_new();
        t->id = TK_EOF;
    }
    
    return t;
}

void token_free(Token* token) {
    if( token == nullptr ) { return; }
    
    switch( token->id )
    {
        case TK_STRING:
        case TK_ID:
        case TK_TYPEID:
            if( token->string != NULL ) { delete token->string; }
            break;
            
        default: {}
    }
    
    free(token);
}

Lexer::Lexer(std::string* input, std::vector<error_t>* list) {
    this->m = input;
    this->len = input->size();
    this->line = 1;
    this->line_pos = 1;
    this->ptr = 0;
    this->error_list = list;
    this->buffer = new std::string("");
};
