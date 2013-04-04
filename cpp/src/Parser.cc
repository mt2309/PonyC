// Copyright 2013 <Michael Thorpe>
//
//  parser.cpp
//  ponyC
//
//  Created by Michael Thorpe on 14/11/2012.
//
//

#include "Parser.h"

#include <assert.h>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"

TokenType Parser::current() {
    return this->t->id;
}

AST* Parser::ast_new(TokenType id) {
    AST* a = new AST(nullptr,
                     new Token(this->file_name,
                               this->t->line,
                               this->t->linePos,
                               id));
    return a;
}

AST* Parser::ast_token() {
    AST* a = new AST(nullptr, this->t);
    this->t = this->lexer.next();
    return a;
}

AST* Parser::ast_expect(TokenType id) {
    if (this->current() == id)
        return this->ast_token();

    this->push_error("Expected " + tokenToString.at(id) + ", got " +
                     tokenToString.at(this->current()));

    return nullptr;
}

void Parser::push_error(std::string err) {
    this->error_list->push_back(Error(this->t->fileName,
                                     this->t->line,
                                     this->t->linePos, err));
}

bool Parser::accept(TokenType id, AST* ast , size_t slot) {
    if (this->current() != id) { return false; }

    if ((ast != nullptr) && (slot != SIZE_MAX)) {
        assert(slot < AST_SLOTS);
        AST* child = this->ast_token();
        ast->children.at(slot) = child;
    } else {
        delete this->t;
        this->t = this->lexer.next();
    }

    return true;
}

bool Parser::expect(TokenType id , AST* ast, size_t slot) {
    if (this->accept(id, ast, slot)) {
        return true;
    }

    this->push_error("Expected " + tokenToString.at(id) + ", got " +
                     tokenToString.at(this->current()));

    return false;
}

void Parser::rule(rule_t f, AST* ast, size_t slot) {
    assert(ast != nullptr);
    assert(slot >= 0);
    assert(slot < AST_SLOTS);

    AST* child = (this->*f)();
    ast->children.at(slot) = child;
}

void Parser::rulelist(rule_t rule, TokenType id, AST* ast, size_t slot) {
    assert(ast != nullptr);
    assert(slot != SIZE_MAX);
    assert(slot < AST_SLOTS);

    AST* last = nullptr;

    while (true) {
        AST* child = (this->*rule)();

        if (child == nullptr)
            return;

        if (last != nullptr)
            last->sibling = child;
        else
            ast->children.at(slot) = child;

        if (!this->accept(id, ast, SIZE_MAX))
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

void Parser::rulealtlist(const std::vector<alt_t> alt_vec,
                         AST* ast, size_t slot) {
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
            ast->children.at(slot) = child;
        }

        last = child;
    }
}

AST* Parser::mode_receiver() {
    AST* ast = this->ast_expect(TokenType::TK_MODE);

    this->rule(&Parser::expr, ast, 0);
    this->accept(TokenType::TK_RBRACKET, ast, 1);

    return ast;
}

AST* Parser::mode() {
    // ('!' | '@' | '~' | '[:'expr ']')?

    static const std::vector<alt_t> alt = {
        { TokenType::TK_BANG,  &Parser::ast_token },
        { TokenType::TK_UNIQ,  &Parser::ast_token },
        { TokenType::TK_MUT,   &Parser::ast_token },
        { TokenType::TK_MODE,  &Parser::mode_receiver }
    };

    return rulealt(alt);
}

AST* Parser::atom() {
    // THIS | TRUE | FALSE | INT | STRING | ID | typeclass
    static std::vector<alt_t> alt = {
        { TokenType::TK_THIS, &Parser::ast_token },
        { TokenType::TK_TRUE, &Parser::ast_token },
        { TokenType::TK_FALSE, &Parser::ast_token },
        { TokenType::TK_INT, &Parser::ast_token },
        { TokenType::TK_FLOAT, &Parser::ast_token },
        { TokenType::TK_STRING, &Parser::ast_token },
        { TokenType::TK_ID, &Parser::ast_token },
        { TokenType::TK_TYPEID, &Parser::ast_token }};

    AST* ast = this->rulealt(alt);

    if (ast == nullptr) {
        this->push_error("Expected atom");
    }

    return ast;
}

AST* Parser::command() {
    AST* ast;

    if (this->accept(TokenType::TK_LPAREN, nullptr, SIZE_MAX)) {
        ast = this->expr();
        this->expect(TokenType::TK_RPAREN, nullptr, SIZE_MAX);
    } else if (this->current() == TokenType::TK_LBRACKET) {
        ast = this->ast_token();
        ast->t->id = TokenType::TK_LIST;
        this->rulelist(&Parser::arg, TokenType::TK_COMMA, ast, 0);
        this->expect(TokenType::TK_RBRACKET, ast, SIZE_MAX);
    } else {
        ast = this->atom();
    }

    if (ast != nullptr) {
        while (true) {
            switch (this->current()) {
                case TokenType::TK_CALL: {
                    AST* a = this->ast_token();
                    a->children.at(0) = ast;
                    ast = a;

                    this->accept(TokenType::TK_ID, ast, 1);
                    this->rule(&Parser::formalargs, ast, 2);
                    this->rule(&Parser::args, ast, 3);
                }
                    break;

                case TokenType::TK_LPAREN: {
                    AST* a = this->ast_token();
                    a->children.at(0) = ast;
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
    this->rule(&Parser::unary, ast, 0);
    return ast;
}

AST* Parser::unary() {
    static std::vector<alt_t> alt = {
        { TokenType::TK_PARTIAL, &Parser::unop },
        { TokenType::TK_MINUS, &Parser::unop },
        { TokenType::TK_BANG, &Parser::unop },
        { TokenType::TK_LAMBDA, &Parser::lambda }};

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
                case TokenType::TK_PLUS:
                case TokenType::TK_MINUS:
                case TokenType::TK_MULTIPLY:
                case TokenType::TK_DIVIDE:
                case TokenType::TK_MOD:
                case TokenType::TK_LSHIFT:
                case TokenType::TK_RSHIFT:
                case TokenType::TK_LT:
                case TokenType::TK_LE:
                case TokenType::TK_GE:
                case TokenType::TK_GT:
                case TokenType::TK_EQ:
                case TokenType::TK_NOTEQ:
                case TokenType::TK_STEQ:
                case TokenType::TK_NSTEQ:
                case TokenType::TK_OR:
                case TokenType::TK_AND:
                case TokenType::TK_XOR:
                    {
                        AST* binop = this->ast_token();
                        binop->children.at(0) = ast;
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
    AST* ast = this->ast_new(TokenType::TK_ARG);

    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::oftype, ast, 1);

    if (this->accept(TokenType::TK_ASSIGN, ast, SIZE_MAX)) {
        this->rule(&Parser::expr, ast, 2);
    }

    return ast;
}

AST* Parser::args() {
    AST* ast = this->ast_new(TokenType::TK_ARGS);

    this->expect(TokenType::TK_LPAREN, ast, SIZE_MAX);

    if (!this->accept(TokenType::TK_RPAREN, ast, SIZE_MAX)) {
        this->rulelist(&Parser::arg, TokenType::TK_COMMA, ast, 0);
        this->expect(TokenType::TK_RPAREN, ast, SIZE_MAX);
    }

    return ast;
}

AST* Parser::conditional() {
    AST* ast = this->ast_expect(TokenType::TK_IF);

    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::block, ast, 1);

    if (this->accept(TokenType::TK_ELSE, ast, SIZE_MAX)) {
        if (this->current() == TokenType::TK_IF)
            this->rule(&Parser::conditional, ast, 2);
        else
            this->rule(&Parser::block, ast, 2);
    }

    return ast;
}

AST* Parser::forvar() {
    AST* ast = this->ast_new(TokenType::TK_VAR);

    this->expect(TokenType::TK_ID, ast, 0);
    this->rule(&Parser::oftype, ast, 1);

    return ast;
}

AST* Parser::forloop() {
    AST* ast = this->ast_expect(TokenType::TK_FOR);

    this->rulelist(&Parser::forvar, TokenType::TK_COMMA, ast, 0);
    this->expect(TokenType::TK_IN, ast, SIZE_MAX);
    this->rule(&Parser::expr, ast, 1);
    this->rule(&Parser::block, ast, 2);

    return ast;
}

AST* Parser::whileloop() {
    AST* ast = this->ast_expect(TokenType::TK_WHILE);

    this->rule(&Parser::expr, ast, 0);
    this->rule(&Parser::block, ast, 1);

    return ast;
}

AST* Parser::doloop() {
    AST* ast = this->ast_expect(TokenType::TK_DO);

    this->rule(&Parser::block, ast, 1);
    this->expect(TokenType::TK_WHILE, ast, SIZE_MAX);
    this->rule(&Parser::expr, ast, 0);

    return ast;
}

AST* Parser::casevar() {
    AST* ast = this->ast_new(TokenType::TK_CASEVAR);

    if (this->accept(TokenType::TK_AS, ast, SIZE_MAX)) {
        this->rule(&Parser::forvar, ast, 0);
    } else {
        this->rule(&Parser::expr, ast, 1);

        if (this->accept(TokenType::TK_AS, ast, SIZE_MAX))
            this->rule(&Parser::forvar, ast, 0);
    }

    return ast;
}

AST* Parser::caseblock() {
    AST* ast = this->ast_expect(TokenType::TK_CASE);

    if ((this->current() != TokenType::TK_IF)
        && (this->current() != TokenType::TK_LBRACE)) {
        this->rulelist(&Parser::casevar, TokenType::TK_COMMA, ast, 0);
    }

    if (this->accept(TokenType::TK_IF, ast, SIZE_MAX)) {
        this->rule(&Parser::expr, ast, 1);
    }

    this->rule(&Parser::block, ast, 2);

    return ast;
}

AST* Parser::match() {
    static std::vector<alt_t> alt = {
        { TokenType::TK_CASE, &Parser::caseblock }
    };

    AST* ast = this->ast_expect(TokenType::TK_MATCH);

    this->rulelist(&Parser::expr, TokenType::TK_COMMA, ast, 0);
    this->expect(TokenType::TK_LBRACE, ast, SIZE_MAX);
    this->rulealtlist(alt, ast, 1);
    this->expect(TokenType::TK_RBRACE, ast, SIZE_MAX);

    return ast;
}

AST* Parser::catchblock() {
    AST* ast = this->ast_expect(TokenType::TK_CATCH);

    this->rule(&Parser::block, ast, 0);

    return ast;
}

AST* Parser::always() {
    AST* ast = this->ast_expect(TokenType::TK_ALWAYS);

    this->rule(&Parser::block, ast, 0);

    return ast;
}

AST* Parser::lvalue() {
    AST* ast;

    if (this->current() == TokenType::TK_VAR)
    {
        ast = this->ast_token();

        this->expect(TokenType::TK_ID, ast, 0);

        this->rule(&Parser::oftype, ast, 1);
    } else {
        ast = this->command();
    }

    return ast;
}

AST* Parser::assignment() {
    AST* ast = this->ast_new(TokenType::TK_ASSIGN);

    this->rulelist(&Parser::lvalue, TokenType::TK_COMMA, ast, 0);

    if (this->accept(TokenType::TK_ASSIGN, ast, SIZE_MAX))
        this->rulelist(&Parser::expr, TokenType::TK_COMMA, ast, 1);

    return ast;
}

AST* Parser::block() {
    static std::vector<alt_t> alt =
    {
        { TokenType::TK_LBRACE,    &Parser::block },
        { TokenType::TK_IF,        &Parser::conditional },
        { TokenType::TK_FOR,       &Parser::forloop },
        { TokenType::TK_WHILE,     &Parser::whileloop },
        { TokenType::TK_DO,        &Parser::doloop },
        { TokenType::TK_MATCH,     &Parser::match },
        { TokenType::TK_RETURN,    &Parser::ast_token },
        { TokenType::TK_BREAK,     &Parser::ast_token },
        { TokenType::TK_CONTINUE,  &Parser::ast_token },
        { TokenType::TK_THROW,     &Parser::ast_token },

        { TokenType::TK_VAR,       &Parser::assignment },
        { TokenType::TK_THIS,      &Parser::assignment },
        { TokenType::TK_TRUE,      &Parser::assignment },
        { TokenType::TK_FALSE,     &Parser::assignment },
        { TokenType::TK_INT,       &Parser::assignment },
        { TokenType::TK_FLOAT,     &Parser::assignment },
        { TokenType::TK_STRING,    &Parser::assignment },
        { TokenType::TK_ID,        &Parser::assignment },
        { TokenType::TK_TYPEID,    &Parser::assignment }

    };

    AST* ast = this->ast_new(TokenType::TK_BLOCK);
    this->expect(TokenType::TK_LBRACE, ast, SIZE_MAX);
    this->rulealtlist(alt, ast, 0);

    if (this->current() == TokenType::TK_CATCH)
        this->rule(&Parser::catchblock, ast, 1);

    if (this->current() == TokenType::TK_ALWAYS)
        this->rule(&Parser::always, ast, 2);

    this->expect(TokenType::TK_RBRACE, ast, SIZE_MAX);
    return ast;
}

AST* Parser::formalargs() {
    AST* ast = this->ast_new(TokenType::TK_FORMALARGS);

    if (this->accept(TokenType::TK_LBRACKET, ast, SIZE_MAX))
    {
        this->rulelist(&Parser::arg, TokenType::TK_COMMA, ast, 0);
        this->expect(TokenType::TK_RBRACKET, ast, SIZE_MAX);
    }

    return ast;
}

AST* Parser::typelambda() {
    AST* ast = this->ast_expect(TokenType::TK_LAMBDA);

    this->rule(&Parser::mode, ast, 0);
    this->rule(&Parser::args, ast, 1);

    if (this->accept(TokenType::TK_RESULTS, ast, SIZE_MAX))
    {
        this->rule(&Parser::args, ast, 2);
    }

    this->accept(TokenType::TK_THROWS, ast, 3);

    return ast;
}

AST* Parser::lambda() {
    AST* ast = this->typelambda();

    if (this->accept(TokenType::TK_IS, ast, SIZE_MAX))
        this->rule(&Parser::block, ast, 4);

    return ast;
}

AST* Parser::typeclass() {
    AST* ast = this->ast_new(TokenType::TK_TYPECLASS);

    this->expect(TokenType::TK_TYPEID, ast, 0);

    if (this->accept(TokenType::TK_PACKAGE, ast, SIZE_MAX)) {
        this->expect(TokenType::TK_TYPEID, ast, 1);
    }

    this->rule(&Parser::mode, ast, 2);
    this->rule(&Parser::formalargs, ast, 3);
    return ast;
}

AST* Parser::partialtype() {

    AST* ast = this->ast_expect(TokenType::TK_PARTIAL);
    this->rule(&Parser::typeclass, ast, 0);
    return ast;
}

AST* Parser::typeelement() {
    static std::vector<alt_t> alt =
    {
        { TokenType::TK_PARTIAL, &Parser::partialtype },
        { TokenType::TK_TYPEID, &Parser::typeclass },
        { TokenType::TK_LAMBDA, &Parser::typelambda }
    };

    AST* ast = this->rulealt(alt);

    if (ast == NULL) {
        this->push_error("Expected partial type, type or lambda");
    }

    return ast;
}

AST* Parser::oftype() {
    AST* ast = this->ast_new(TokenType::TK_OFTYPE);

    if (this->accept(TokenType::TK_OFTYPE, ast, SIZE_MAX))
        rulelist(&Parser::typeelement, TokenType::TK_OR, ast, 0);

    return ast;
}

AST* Parser::field() {
    AST* ast = this->ast_new(TokenType::TK_FIELD);

    this->expect(TokenType::TK_VAR, ast, SIZE_MAX);
    this->expect(TokenType::TK_ID, ast, 0);
    this->rule(&Parser::oftype, ast, 1);

    if (this->accept(TokenType::TK_ASSIGN, ast, SIZE_MAX))
        this->rule(&Parser::expr, ast, 2);
    
    this->expect(TokenType::TK_SCOLON, ast, SIZE_MAX);

    return ast;
}

AST* Parser::delegate() {
    AST* ast = this->ast_expect(TokenType::TK_DELEGATE);

    this->expect(TokenType::TK_ID, ast, 0);
    this->rule(&Parser::oftype, ast, 1);
    return ast;
}

AST* Parser::constructor() {
    AST* ast = this->ast_expect(TokenType::TK_NEW);

    this->rule(&Parser::mode, ast, 0);
    this->accept(TokenType::TK_ID, ast, 1);
    this->rule(&Parser::formalargs, ast, 2);
    this->rule(&Parser::args, ast, 3);
    this->accept(TokenType::TK_THROWS, ast, 4);

    if (this->current() == TokenType::TK_LBRACE)
        this->rule(&Parser::block, ast, 5);
    else
        this->expect(TokenType::TK_SCOLON, ast, SIZE_MAX);

    return ast;
}

AST* Parser::ambient() {
    AST* ast = this->ast_expect(TokenType::TK_AMBIENT);

    this->accept(TokenType::TK_ID, ast, 0);
    this->rule(&Parser::formalargs, ast, 1);
    this->rule(&Parser::args, ast, 2);
    this->accept(TokenType::TK_THROWS, ast, 3);

    if (this->current() == TokenType::TK_LBRACE)
        this->rule(&Parser::block, ast, 4);
    else
        this->expect(TokenType::TK_SCOLON, ast, SIZE_MAX);


    return ast;
}

AST* Parser::function() {
    AST* ast = this->ast_expect(TokenType::TK_FUNCTION);

    this->rule(&Parser::mode, ast, 0);
    this->accept(TokenType::TK_ID, ast, 1);
    this->rule(&Parser::formalargs, ast, 2);
    this->rule(&Parser::args, ast, 3);

    if (this->accept(TokenType::TK_RESULTS, ast, SIZE_MAX))
        this->rule(&Parser::args, ast, 4);

    this->accept(TokenType::TK_THROWS, ast, 5);

    if (this->current() == TokenType::TK_LBRACE)
        this->rule(&Parser::block, ast, 6);
    else
        this->expect(TokenType::TK_SCOLON, ast, SIZE_MAX);

    return ast;
}

AST* Parser::message() {
    AST* ast = this->ast_expect(TokenType::TK_MESSAGE);

    this->rule(&Parser::mode, ast, 0);
    this->accept(TokenType::TK_ID, ast, 1);
    this->rule(&Parser::formalargs, ast, 2);
    this->rule(&Parser::args, ast, 3);

    if (this->current() == TokenType::TK_LBRACE)
        this->rule(&Parser::block, ast, 4);
    else
        this->expect(TokenType::TK_SCOLON, ast, SIZE_MAX);

    return ast;
}

AST* Parser::typebody() {
    static std::vector<alt_t> alt = {
        { TokenType::TK_VAR,       &Parser::field },
        { TokenType::TK_DELEGATE,  &Parser::delegate },
        { TokenType::TK_NEW,       &Parser::constructor },
        { TokenType::TK_AMBIENT,   &Parser::ambient },
        { TokenType::TK_FUNCTION,  &Parser::function },
        { TokenType::TK_MESSAGE,   &Parser::message }
    };

    AST* ast = this->ast_new(TokenType::TK_TYPEBODY);

    this->expect(TokenType::TK_LBRACE, ast, SIZE_MAX);
    this->rulealtlist(alt, ast, 0);
    this->expect(TokenType::TK_RBRACE, ast, SIZE_MAX);

    return ast;
}

AST* Parser::is() {
    AST* ast = this->ast_new(TokenType::TK_IS);

    if (this->accept(TokenType::TK_IS, ast, SIZE_MAX))
        this->rulelist(&Parser::typeclass, TokenType::TK_COMMA, ast, 0);

    return ast;
}

AST* Parser::trait() {
    AST* ast = this->ast_expect(TokenType::TK_TRAIT);

    this->expect(TokenType::TK_TYPEID, ast, 0);
    this->rule(&Parser::formalargs, ast, 1);
    this->rule(&Parser::is, ast, 2);
    this->rule(&Parser::typebody, ast, 3);

    return ast;
}

AST* Parser::object() {
    AST* ast = this->ast_expect(TokenType::TK_OBJECT);

    this->expect(TokenType::TK_TYPEID, ast, 0);
    this->rule(&Parser::formalargs, ast, 1);
    this->rule(&Parser::is, ast, 2);
    this->rule(&Parser::typebody, ast, 3);

    return ast;
}

AST* Parser::actor() {
    AST* ast = this->ast_expect(TokenType::TK_ACTOR);

    this->expect(TokenType::TK_TYPEID, ast, 0);
    this->rule(&Parser::formalargs, ast, 1);
    this->rule(&Parser::is, ast, 2);
    this->rule(&Parser::typebody, ast, 3);

    return ast;
}

AST* Parser::type() {
    AST* ast = this->ast_expect(TokenType::TK_TYPE);

    this->expect(TokenType::TK_TYPEID, ast, 0);
    this->rule(&Parser::oftype, ast, 1);
    this->rule(&Parser::is, ast, 2);

    return ast;
}

AST* Parser::map() {
    AST* ast = this->ast_new(TokenType::TK_MAP);

    this->expect(TokenType::TK_ID, ast, 0);
    this->expect(TokenType::TK_ASSIGN, ast, SIZE_MAX);
    this->expect(TokenType::TK_ID, ast, 1);

    return ast;
}

AST* Parser::declaremap() {
    AST* ast = this->ast_new(TokenType::TK_DECLAREMAP);

    if (this->accept(TokenType::TK_LBRACE, ast, SIZE_MAX)) {
        this->rulelist(&Parser::map, TokenType::TK_COMMA, ast, 0);
        this->expect(TokenType::TK_RBRACE, ast, SIZE_MAX);
    }

    return ast;
}

AST* Parser::declare() {
    AST* ast = this->ast_expect(TokenType::TK_DECLARE);

    this->rule(&Parser::typeclass, ast, 0);
    this->rule(&Parser::is, ast, 1);
    this->rule(&Parser::declaremap, ast, 2);

    return ast;
}

AST* Parser::use() {
    AST* ast = this->ast_expect(TokenType::TK_USE);

    if (this->accept(TokenType::TK_TYPEID, ast, 0))
        this->expect(TokenType::TK_ASSIGN, ast, SIZE_MAX);

    this->expect(TokenType::TK_STRING, ast, 1);
    return ast;
}

AST* Parser::module() {
    static std::vector<alt_t> alt = {
        { TokenType::TK_USE,           &Parser::use },
        { TokenType::TK_DECLARE,       &Parser::declare },
        { TokenType::TK_TYPE,          &Parser::type },
        { TokenType::TK_TRAIT,         &Parser::trait },
        { TokenType::TK_OBJECT,        &Parser::object },
        { TokenType::TK_ACTOR,         &Parser::actor }};

    AST* ast = this->ast_new(TokenType::TK_MODULE);

    this->rulealtlist(alt, ast, 0);

    this->expect(TokenType::TK_EOF, ast, SIZE_MAX);

    return ast;
}

AST* Parser::parse() {
    this->t = this->lexer.next();

    this->m_ast = this->module();

    if (this->error_list->size() != 0) {
        std::cout << "Parse errors detected:" << std::endl;
        for (auto err : *this->error_list) {
            std::cout << "Error at " <<
                err.line << ":" << err.line_pos << ":" << err.message
                << std::endl;
        }
    }
    return this->m_ast;
}

#pragma GCC diagnostic pop
