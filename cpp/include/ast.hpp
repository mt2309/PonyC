#ifndef ponyC_ast_hpp
#define ponyC_ast_hpp

#define AST_SLOTS 7

typedef struct AST {
  Token* t;
  struct AST* sibling;
  AST* children[AST_SLOTS];
} AST;

#endif
