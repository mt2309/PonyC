#include "common.hpp"

void token_free(Token* token) {
  if( token == nullptr ) { return; }

  free(token);
}

void ast_free(AST* ast) {
  if (ast == nullptr)
    return;
  if (ast->t != nullptr) { token_free(ast->t); }

  for (size_t i = 0; i < AST_SLOTS; i++) {
    ast_free(ast->children.at(i));
  }

  ast_free(ast->sibling);
}

