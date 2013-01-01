//
//  parser.cpp
//  ponyC
//
//  Created by Michael Thorpe on 14/11/2012.
//
//

#include "parser.hpp"
#include "error.hpp"
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#include <boost/format.hpp>
#pragma GCC diagnostic pop

tok_type Parser::current() {
    return this->t->id;
}

AST* Parser::ast_new(tok_type id) {
    AST* a = (AST*)calloc(1, sizeof(AST));
    a->t = (Token*)calloc(1, sizeof(Token));
    a->t->id = id;
    a->t->line = this->t->line;
    a->t->line_pos = this->t->line_pos;
    a->children = new std::vector<AST*>(AST_SLOTS);
    a->types = new std::map<std::string, Type*>();
    return a;
}

AST* Parser::ast_token() {
    AST* a = (AST*)calloc(1, sizeof(AST));
    a->t = this->t;
    this->t = this->lexer->next();
    a->children = new std::vector<AST*>(AST_SLOTS);
    return a;
}

AST* Parser::ast_expect(tok_type id) {
    
    if (this->current() == id)
        return this->ast_token();
    
    this->push_error((boost::format("Expected %1%, got %2%") % id % this->current()).str());
    return nullptr;
}


void Parser::push_error(std::string err) {
    std::cout << "Got an error: " << err << std::endl;
    this->error_list->push_back(*error_new(this->t->fileName, this->t->line, this->t->line_pos, err));
}

bool Parser::accept(tok_type id, AST* ast , int slot) {
        
    if (this->current() != id) { return false; }
    
    if ((ast != nullptr) && (slot >= 0)) {
        assert(slot < AST_SLOTS);
        AST* child = this->ast_token();
        ast->children->at(slot) = child;
    } else {
        token_free(this->t);
        this->t = this->lexer->next();
    }
    
    return true;
}

bool Parser::expect(tok_type id , AST* ast ,int slot) {
    
    
    if (this->accept(id, ast, slot)) {
        return true;
    }
        
    this->push_error((boost::format("Expected %1%, got %2%") % id % this->current()).str());
    return false;
    
}

void Parser::rule(rule_t f, AST* ast, int slot) {
    assert(ast != nullptr);
    assert(slot >= 0);
    assert(slot < AST_SLOTS);
    
    AST* child = (this->*f)();
    ast->children->at(slot) = child;
}

void Parser::rulelist(rule_t rule, tok_type id, AST* ast, int slot) {
    assert(ast != nullptr);
    assert(slot >= 0);
    assert(slot < AST_SLOTS);
    
    AST* last = nullptr;
    
    while (true) {
        AST* child = (this->*rule)();
        
        if (child == nullptr)
            return;
        
        if (last != nullptr)
            last->sibling = child;
        else
            ast->children->at(slot) = child;
        
        if (!this->accept(id, ast, -1))
            return;
        
        last = child;
    }
}

AST* Parser::rulealt(const std::vector<alt_t> alt_vec) {
    
    for (auto alt : alt_vec) {
        if (this->current() == alt.id) {
            return (this->*(alt.f))();
        }
    }
    
    return nullptr;
}

void Parser::rulealtlist(const std::vector<alt_t> alt_vec, AST* ast, int slot) {
    assert(ast != nullptr);
    assert(slot >= 0);
    assert(slot < AST_SLOTS);
    
    AST* last = nullptr;
    
    while (true) {
        AST* child = this->rulealt(alt_vec);
        if (child == nullptr) {
            return;
        }
        
        if (last != nullptr) {
            last->sibling = child;
        } else {
            ast->children->at(slot) = child;
        }
        
        last = child;
    }
}

AST* Parser::mode_receiver() {
    AST* ast = this->ast_expect(TK_MODE);
    
    this->rule(&Parser::expr, ast, 0);
    this->accept(TK_RBRACKET, ast, 1);
    
    return ast;
}

AST* Parser::mode() {
    // ('!' | '@' | '~' | '[:'expr ']')?
        
    static const std::vector<alt_t> alt = {
        { TK_BANG,  &Parser::ast_token },
        { TK_UNIQ,  &Parser::ast_token },
        { TK_MUT,   &Parser::ast_token },
        { TK_MODE,  &Parser::mode_receiver }
    };

    return rulealt(alt);
}

AST* Parser::atom() {
    
    // THIS | TRUE | FALSE | INT | STRING | ID | typeclass
    static std::vector<alt_t> alt =
    {
        { TK_THIS, &Parser::ast_token },
        { TK_TRUE, &Parser::ast_token },
        { TK_FALSE, &Parser::ast_token },
        { TK_INT, &Parser::ast_token },
        { TK_FLOAT, &Parser::ast_token },
        { TK_STRING, &Parser::ast_token },
        { TK_ID, &Parser::ast_token },
        { TK_TYPEID, &Parser::ast_token }
    };
    
    AST* ast = this->rulealt(alt);
    
    if (ast == nullptr) {
        this->push_error("Expected atom");
    }
    
    return ast;
}

AST* Parser::command() {
    
    AST* ast;
    
    if (this->accept(TK_LPAREN, nullptr, -1)) {
        ast = this->expr();
        this->expect(TK_RPAREN, nullptr, -1);
    } else if (this->current() == TK_LBRACKET) {
        ast = this->ast_token();
        ast->t->id = TK_LIST;
        this->rulelist(&Parser::arg, TK_COMMA, ast, 0);
        this->expect(TK_RBRACKET, ast, -1);
    } else {
        ast = this->atom();
    }
    
    if (ast != nullptr) {
        while (true) {
            switch (this->current()) {
                case TK_CALL: {
                    AST* a = this->ast_token();
                    a->children->at(0) = ast;
                    ast = a;
                    
                    this->accept(TK_ID, ast, 1);
                    this->rule(&Parser::formalargs, ast, 2);
                    this->rule(&Parser::args, ast, 3);
                }
                    break;
                    
                case TK_LPAREN: {
                    AST* a = this->ast_token();
                    a->children->at(0) = ast;
                    ast = a;
                    
                    this->rule(&Parser::args, ast, 2);
                }
                    break;
                    
                default:
                    return ast;
            }
        }
    }
    
    return ast;
}

AST* Parser::unop() {
    AST* ast = this->ast_token();
    this->rule(&Parser::unary,ast,0);
    return ast;
}

AST* Parser::unary() {
    
    static std::vector<alt_t> alt =
    {
        { TK_PARTIAL, &Parser::unop },
        { TK_MINUS, &Parser::unop },
        { TK_BANG, &Parser::unop },
        { TK_LAMBDA, &Parser::lambda }
    };
    
    AST* ast = this->rulealt(alt);
    
    if (ast == nullptr) {
        ast = this->command();
        
        if (ast == nullptr)
            this->push_error("Expected unary");
    }
    
    return ast;
}

AST* Parser::expr() {
    AST* ast = this->unary();
    
    if (ast != nullptr) {
        while (true) {
            switch (this->current()) {
                case TK_PLUS:
                case TK_MINUS:
                case TK_MULTIPLY:
                case TK_DIVIDE:
                case TK_MOD:
                case TK_LSHIFT:
                case TK_RSHIFT:
                case TK_LT:
                case TK_LE:
                case TK_GE:
                case TK_GT:
                case TK_EQ:
                case TK_NOTEQ:
                case TK_STEQ:
                case TK_NSTEQ:
                case TK_OR:
                case TK_AND:
                case TK_XOR:
                    {
                        AST* binop = this->ast_token();
                        binop->children->at(0) = ast;
                        ast = binop;

                        this->rule(&Parser::unary, ast, 1);
                    }
                    break;
                    
                default:
                    return ast;
            }
        }
    }
    
    return nullptr;
}

AST* Parser::arg() {
    
    AST* ast = this->ast_new(TK_ARG);
    
    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::oftype, ast, 1);
    
    if (this->accept(TK_ASSIGN, ast, -1)) {
        this->rule(&Parser::expr, ast, 2);
    }
    
    return ast;
}

AST* Parser::args() {
    AST* ast = this->ast_new(TK_ARGS);
    
    this->expect(TK_LPAREN, ast, -1);
    
    if (!this->accept(TK_RPAREN, ast, -1)) {
        this->rulelist(&Parser::arg, TK_COMMA, ast, 0);
        this->expect(TK_RPAREN, ast, -1);
    }
    
    return ast;
}

AST* Parser::conditional() {
    AST* ast = this->ast_expect(TK_IF);
    
    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::block, ast, 1);
    
    if (this->accept(TK_ELSE, ast, -1)) {
        if (this->current() == TK_IF)
            this->rule(&Parser::conditional, ast, 2);
        else
            this->rule(&Parser::block, ast, 2);
    }
    
    return ast;
}

AST* Parser::forvar() {
    
    AST* ast = this->ast_new(TK_VAR);
    
    this->expect(TK_ID, ast, 0);
    this->rule(&Parser::oftype, ast, 1);
    
    return ast;
}

AST* Parser::forloop() {
    AST* ast = this->ast_expect(TK_FOR);
    
    this->rulelist(&Parser::forvar, TK_COMMA, ast, 0 );
    this->expect(TK_IN, ast, -1 );
    this->rule(&Parser::expr, ast, 1 );
    this->rule(&Parser::block, ast, 2 );
    
    return ast;
}

AST* Parser::whileloop() {
    AST* ast = this->ast_expect(TK_WHILE);
    
    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::block, ast, 1);
    
    return ast;
}

AST* Parser::doloop() {
    
    AST* ast = this->ast_expect(TK_DO);
    
    this->rule(&Parser::block, ast, 1 );
    this->expect(TK_WHILE, ast, -1 );
    this->rule(&Parser::expr, ast, 0 );
    
    return ast;
}

AST* Parser::casevar() {
    AST* ast = this->ast_new(TK_CASEVAR);
    
    if (this->accept(TK_AS, ast, -1)) {
        this->rule(&Parser::forvar, ast, 0);
    } else {
        this->rule(&Parser::expr, ast, 1);
        
        if (this->accept(TK_AS, ast, -1))
            this->rule(&Parser::forvar, ast, 0);
    }
    
    return ast;
}

AST* Parser::caseblock() {
    AST* ast = this->ast_expect(TK_CASE);
    
    if (    (this->current() != TK_IF)
        &&  (this->current() != TK_LBRACE))
    {
        
        this->rulelist(&Parser::casevar, TK_COMMA, ast, 0);
    }
    
    if (this->accept(TK_IF, ast, -1)) {
        this->rule(&Parser::expr, ast, 1);
    }
    
    this->rule(&Parser::block, ast, 2);
    
    return ast;
}

AST* Parser::match() {
    
    static std::vector<alt_t> alt = {
        { TK_CASE, &Parser::caseblock }
    };
    
    AST* ast = this->ast_expect(TK_MATCH);
    
    this->rulelist(&Parser::expr, TK_COMMA, ast, 0 );
    this->expect(TK_LBRACE, ast, -1 );
    this->rulealtlist(alt, ast, 1 );
    this->expect(TK_RBRACE, ast, -1 );
    
    return ast;
}

AST* Parser::catchblock() {
    AST* ast = this->ast_expect( TK_CATCH );
    
    this->rule( &Parser::block, ast, 0 );
    
    return ast;
}

AST* Parser::always() {
    AST* ast = this->ast_expect(TK_ALWAYS);
    
    this->rule( &Parser::block, ast, 0 );
    
    return ast;
}

AST* Parser::lvalue() {
    AST* ast;
    
    if( this->current() == TK_VAR)
    {
        ast = this->ast_token();
        
        this->expect(TK_ID, ast, 0 );
        
        this->rule(&Parser::oftype, ast, 1 );
    } else {
        ast = this->command();
    }
    
    return ast;
}

AST* Parser::assignment() {
        
    AST* ast = this->ast_new(TK_ASSIGN);
    
    this->rulelist( &Parser::lvalue, TK_COMMA, ast, 0 );
    
    if( this->accept(TK_ASSIGN, ast, -1 ))
        this->rulelist(&Parser::expr, TK_COMMA, ast, 1 );
    
    return ast;
}

AST* Parser::block() {
    
    static std::vector<alt_t> alt =
    {
        { TK_LBRACE,    &Parser::block },
        { TK_IF,        &Parser::conditional },
        { TK_FOR,       &Parser::forloop },
        { TK_WHILE,     &Parser::whileloop },
        { TK_DO,        &Parser::doloop },
        { TK_MATCH,     &Parser::match },
        { TK_RETURN,    &Parser::ast_token },
        { TK_BREAK,     &Parser::ast_token },
        { TK_CONTINUE,  &Parser::ast_token },
        { TK_THROW,     &Parser::ast_token },
        
        { TK_VAR,       &Parser::assignment },
        { TK_THIS,      &Parser::assignment },
        { TK_TRUE,      &Parser::assignment },
        { TK_FALSE,     &Parser::assignment },
        { TK_INT,       &Parser::assignment },
        { TK_FLOAT,     &Parser::assignment },
        { TK_STRING,    &Parser::assignment },
        { TK_ID,        &Parser::assignment },
        { TK_TYPEID,    &Parser::assignment }
        
    };

    AST* ast = this->ast_new( TK_BLOCK );
    this->expect(TK_LBRACE, ast, -1 );
    this->rulealtlist(alt, ast, 0 );
    
    if(this->current() == TK_CATCH)
        this->rule(&Parser::catchblock, ast, 1 );
    
    if (this->current() == TK_ALWAYS)
        this->rule(&Parser::always, ast, 2);
    
    this->expect(TK_RBRACE, ast, -1 );
    return ast;
}

AST* Parser::formalargs() {
    AST* ast = this->ast_new( TK_FORMALARGS );
        
    if( this->accept(TK_LBRACKET, ast, -1 ) )
    {
        this->rulelist( &Parser::arg, TK_COMMA, ast, 0 );
        this->expect(TK_RBRACKET, ast, -1 );
    }
    
    return ast;
}

AST* Parser::typelambda() {
    AST* ast = this->ast_expect( TK_LAMBDA );
    
    this->rule(&Parser::mode, ast, 0 );
    this->rule(&Parser::args, ast, 1 );
    
    if( this->accept(TK_RESULTS, ast, -1 ) )
    {
        this->rule( &Parser::args, ast, 2 );
    }
    
    this->accept(TK_THROWS, ast, 3 );
    
    return ast;
}

AST* Parser::lambda() {
    AST* ast = this->typelambda();
    
    if( this->accept(TK_IS, ast, -1 ) )
        this->rule(&Parser::block, ast, 4 );
    
    return ast;
}

AST* Parser::typeclass() {
    AST* ast = this->ast_new( TK_TYPECLASS );
    
    this->expect(TK_TYPEID, ast, 0 );
    
    if(this->accept(TK_PACKAGE, ast, -1 )) {
        this->expect(TK_TYPEID, ast, 1 );
    }
    
    this->rule(&Parser::mode, ast, 2 );
    this->rule(&Parser::formalargs, ast, 3 );
    return ast;
}

AST* Parser::partialtype() {

    AST* ast = this->ast_expect(TK_PARTIAL);
    this->rule(&Parser::typeclass, ast, 0 );
    return ast;
}

AST* Parser::typeelement() {
    static std::vector<alt_t> alt =
    {
        { TK_PARTIAL, &Parser::partialtype },
        { TK_TYPEID, &Parser::typeclass },
        { TK_LAMBDA, &Parser::typelambda }
    };
    
    AST* ast = this->rulealt(alt);
    
    if( ast == NULL ) {
        this->push_error("Expected partial type, type or lambda");
    }
    return ast;
}

AST* Parser::oftype() {
    AST* ast = this->ast_new(TK_OFTYPE);
    
    if(this->accept(TK_OFTYPE, ast, -1 ))
        rulelist(&Parser::typeelement, TK_OR, ast, 0 );
    
    return ast;
}

AST* Parser::field() {
    AST* ast = this->ast_new( TK_FIELD );
    
    this->expect(TK_VAR, ast, -1 );
    this->expect(TK_ID, ast, 0 );
    this->rule( &Parser::oftype, ast, 1 );
    
    if(this->accept(TK_ASSIGN, ast, -1 ))
        this->rule( &Parser::expr, ast, 2 );
    
    return ast;
}

AST* Parser::delegate() {
    AST* ast = this->ast_expect( TK_DELEGATE );
    
    this->expect(TK_ID, ast, 0 );
    this->rule(&Parser::oftype, ast, 1 );
    return ast;
}

AST* Parser::constructor() {
    AST* ast = this->ast_expect(TK_NEW);
    
    this->rule(&Parser::mode, ast, 0);
    this->accept(TK_ID, ast, 1 );
    this->rule(&Parser::formalargs, ast, 2 );
    this->rule(&Parser::args, ast, 3 );
    this->accept(TK_THROWS, ast, 4 );
    
    if(this->current() == TK_LBRACE)
        this->rule(&Parser::block, ast, 5 );
    
    return ast;
}

AST* Parser::ambient() {
    
    AST* ast = this->ast_expect(TK_AMBIENT);
    
    this->accept(TK_ID, ast, 0 );
    this->rule(&Parser::formalargs, ast, 1 );
    this->rule(&Parser::args, ast, 2 );
    this->accept(TK_THROWS, ast, 3 );
    
    if(this->current() == TK_LBRACE)
        this->rule(&Parser::block, ast, 4 );
    
    return ast;
}

AST* Parser::function() {
    AST* ast = this->ast_expect(TK_FUNCTION);
    
    this->rule(&Parser::mode, ast, 0 );
    this->accept(TK_ID, ast, 1 );
    this->rule(&Parser::formalargs, ast, 2 );
    this->rule(&Parser::args, ast, 3 );
    
    if( this->accept(TK_RESULTS, ast, -1 ) )
        this->rule(&Parser::args, ast, 4 );
    
    this->accept(TK_THROWS, ast, 5 );
    
    if( this->current() == TK_LBRACE )
        this->rule(&Parser::block, ast, 6 );
    
    return ast;
}

AST* Parser::message() {
    AST* ast = this->ast_expect(TK_MESSAGE);
    
    this->rule(&Parser::mode, ast, 0);
    this->accept(TK_ID, ast, 1);
    this->rule(&Parser::formalargs, ast, 2);
    this->rule(&Parser::args, ast, 3);
    
    if( this->current() == TK_LBRACE )
        this->rule(&Parser::block, ast, 4);
    
    return ast;
}

AST* Parser::typebody() {
    static std::vector<alt_t> alt = {
        { TK_VAR,       &Parser::field },
        { TK_DELEGATE,  &Parser::delegate },
        { TK_NEW,       &Parser::constructor },
        { TK_AMBIENT,   &Parser::ambient },
        { TK_FUNCTION,  &Parser::function },
        { TK_MESSAGE,   &Parser::message }
    };
    
    AST* ast = this->ast_new( TK_TYPEBODY );
    
    this->expect(TK_LBRACE, ast, -1 );
    this->rulealtlist(alt, ast, 0 );
    this->expect(TK_RBRACE, ast, -1 );
    
    return ast;
}

AST* Parser::is() {
    AST* ast = this->ast_new( TK_IS );
    
    if(this->accept(TK_IS, ast, -1 ))
        this->rulelist(&Parser::typeclass, TK_COMMA, ast, 0 );
    
    return ast;
}

AST* Parser::trait() {
    AST* ast = this->ast_expect(TK_TRAIT);
    
    this->expect(TK_TYPEID, ast, 0 );
    this->rule(&Parser::formalargs, ast, 1 );
    this->rule(&Parser::is, ast, 2 );
    this->rule(&Parser::typebody, ast, 3 );
    
    return ast;
}

AST* Parser::object() {
    AST* ast = this->ast_expect(TK_OBJECT);
    
    this->expect(TK_TYPEID, ast, 0 );
    this->rule(&Parser::formalargs, ast, 1 );
    this->rule(&Parser::is, ast, 2 );
    this->rule(&Parser::typebody, ast, 3 );
    
    return ast;
}

AST* Parser::actor() {
    AST* ast = this->ast_expect(TK_ACTOR);
    
    this->expect(TK_TYPEID, ast, 0 );
    this->rule(&Parser::formalargs, ast, 1 );
    this->rule(&Parser::is, ast, 2 );
    this->rule(&Parser::typebody, ast, 3 );
    
    return ast;
}

AST* Parser::type() {
    AST* ast = this->ast_expect(TK_TYPE);
    
    this->expect(TK_TYPEID, ast, 0 );
    this->rule(&Parser::oftype, ast, 1 );
    this->rule(&Parser::is, ast, 2 );
    
    return ast;
}

AST* Parser::map() {
    AST* ast = this->ast_new(TK_MAP);
    
    this->expect(TK_ID, ast, 0 );
    this->expect(TK_ASSIGN, ast, -1 );
    this->expect(TK_ID, ast, 1 );
    
    return ast;
}

AST* Parser::declaremap() {
    AST* ast = this->ast_new(TK_DECLAREMAP);
    
    if(this->accept(TK_LBRACE, ast, -1 )) {
        this->rulelist(&Parser::map, TK_COMMA, ast, 0 );
        this->expect(TK_RBRACE, ast, -1 );
    }
    
    return ast;
}

AST* Parser::declare() {
    AST* ast = this->ast_expect(TK_DECLARE);
    
    this->rule(&Parser::typeclass, ast, 0 );
    this->rule(&Parser::is, ast, 1 );
    this->rule(&Parser::declaremap, ast, 2 );
    
    return ast;
}

AST* Parser::use() {
    AST* ast = this->ast_expect(TK_USE);
    
    if( this->accept(TK_TYPEID, ast, 0 ) )
        this->expect(TK_ASSIGN, ast, -1 );
    
    this->expect(TK_TYPEID, ast, 1 );
    return ast;
}

AST* Parser::module() {
    
    static std::vector<alt_t> alt =
    {
        { TK_USE,           &Parser::use },
        { TK_DECLARE,       &Parser::declare },
        { TK_TYPE,          &Parser::type },
        { TK_TRAIT,         &Parser::trait },
        { TK_OBJECT,        &Parser::object },
        { TK_ACTOR,         &Parser::actor },
    };
    
    AST* ast = this->ast_new(TK_MODULE);
    
    this->rulealtlist(alt, ast, 0 );
    
    this->expect(TK_EOF, ast, -1);
    
    return ast;
}

AST* Parser::parse() {
    
    this->t = this->lexer->next();
    
    this->m_ast = this->module();
    
    if (this->error_list->size() != 0) {
        std::cout << "Parse errors detected:" << std::endl;
        for(error_t err : *this->error_list) {
            std::cout << boost::format("Error at %1%:%2%: %3%")
                % err.line
                % err.line_pos
                % err.message << std::endl;
        }
    }
    return this->m_ast;
}
