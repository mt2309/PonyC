//
//  parser.hpp
//  ponyC
//
//  Created by Michael Thorpe on 13/11/2012.
//
//

#ifndef ponyC_parser_hpp
#define ponyC_parser_hpp

#include <string>
#include <vector>

#include "lexer.hpp"
#include "error.hpp"
#include "common.hpp"

typedef class Parser Parser;

typedef AST* (Parser::*rule_t)();

typedef struct alt_t{
    TokenType id;
    rule_t f;
} alt_t;


class Parser {

private:
    Lexer::Lexer* lexer;
    Token* t;
    AST* m_ast;
    std::string program_text;
    std::string file_name;

public:

    Parser(std::string _file_name, std::string file) : program_text(file), file_name(_file_name), error_list() {
        this->lexer = new Lexer(_file_name,file,this->error_list);
    }

    std::vector<error_t> error_list;

    AST* parse();
    void push_error(std::string err);

    TokenType current();
    bool accept(TokenType, AST*,size_t);
    bool expect(TokenType, AST*,size_t);
    void rule(rule_t,AST*,size_t);
    void rulelist(rule_t,TokenType,AST*,size_t);
    AST* tokenrule();
    AST* mode();

private:

    AST* ast_new(TokenType);
    AST* ast_token();
    AST* ast_expect(TokenType);
    AST* rulealt(const std::vector<alt_t>);
    void rulealtlist(const std::vector<alt_t>, AST*, size_t);

    AST* lambda();
    AST* typeclass();
    AST* unary();
    AST* expr();
    AST* arg();
    AST* args();
    AST* atom();
    AST* command();
    AST* unop();
    AST* block();
    AST* oftype();
    AST* conditional();
    AST* forvar();
    AST* forloop();
    AST* whileloop();
    AST* doloop();
    AST* casevar();
    AST* caseblock();
    AST* match();
    AST* catchblock();
    AST* always();
    AST* lvalue();
    AST* assignment();
    AST* formalargs();
    AST* typelambda();
    AST* partialtype();
    AST* typeelement();
    AST* field();
    AST* delegate();
    AST* constructor();
    AST* ambient();
    AST* function();
    AST* message();
    AST* typebody();
    AST* is();
    AST* trait();
    AST* object();
    AST* actor();
    AST* type();
    AST* map();
    AST* declaremap();
    AST* declare();
    AST* use();
    AST* module();
    AST* mode_receiver();
};

#endif
