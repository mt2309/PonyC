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

#define AST_SLOTS 7

typedef struct AST {
    Token* t;
    struct AST* sibling;
    AST* children[AST_SLOTS];
} AST;

typedef class Parser Parser;

typedef AST* (Parser::*rule_t)();

typedef struct alt_t{
    tok_type id;
    rule_t f;
} alt_t;


class Parser {
    
private:
    Lexer::Lexer* lexer;
    Token* t;
    AST* ast;
    std::vector<error_t>* error_list;
    std::string* file;
    
public:

    Parser(std::string* file) {
        this->file = file;
        this->error_list = new std::vector<error_t>();
        this->lexer = new Lexer(file,this->error_list);
    };
    
    
    AST* parse();
    void push_error(std::string err);
    
    tok_type current();
    bool accept(tok_type, AST*,int);
    bool expect(tok_type, AST*,int);
    void rule(rule_t,AST*,int);
    void rulelist(rule_t,tok_type,AST*,int);
    AST* tokenrule();
    AST* annotation();
    
private:
    
    AST* ast_new(tok_type id);
    AST* ast_token();
    AST* ast_expect(tok_type id);
    AST* rulealt(const std::vector<alt_t> alt_vec);
    void rulealtlist(const std::vector<alt_t> alt_vec, AST* ast, int slot);
    
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
};

#endif
