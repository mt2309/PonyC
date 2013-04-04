#ifndef CPP_INCLUDE_COMMON_H_
#define CPP_INCLUDE_COMMON_H_

// Copyright 2013 <Michael Thorpe>

#define AST_SLOTS 7
#define FILE_EXTENSION ".pony"

#include <type_traits>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <ostream>

enum class TokenType : unsigned int {
    // primitives
    TK_STRING   = 0,
    TK_INT      = 1,
    TK_FLOAT    = 2,
    TK_ID       = 3,
    TK_TYPEID   = 4,

    // symbols
    TK_LBRACE   = 5,
    TK_RBRACE   = 6,
    TK_LPAREN   = 7,
    TK_RPAREN   = 8,
    TK_LBRACKET = 9,
    TK_RBRACKET = 10,
    TK_COMMA    = 11,
    TK_RESULTS  = 12,

    TK_CALL     = 13,
    TK_PACKAGE  = 14,
    TK_OFTYPE   = 15,
    TK_PARTIAL  = 16,
    TK_ASSIGN   = 17,
    TK_BANG     = 18,

    TK_PLUS     = 19,
    TK_MINUS    = 20,
    TK_MULTIPLY = 21,
    TK_DIVIDE   = 22,
    TK_MOD      = 23,

    TK_LSHIFT   = 24,
    TK_RSHIFT   = 25,

    TK_LT       = 26,
    TK_LE       = 27,
    TK_GE       = 28,
    TK_GT       = 29,

    TK_EQ       = 30,
    TK_NOTEQ    = 31,
    TK_STEQ     = 32,
    TK_NSTEQ    = 33,

    TK_OR       = 34,
    TK_AND      = 35,
    TK_XOR      = 36,

    TK_UNIQ     = 37,
    TK_MUT      = 38,
    TK_MODE     = 39,

    // keywords
    TK_USE      = 40,
    TK_DECLARE  = 41,
    TK_TYPE     = 42,
    TK_LAMBDA   = 43,
    TK_TRAIT    = 44,
    TK_OBJECT   = 45,
    TK_ACTOR    = 46,
    TK_IS       = 47,
    TK_VAR      = 48,
    TK_DELEGATE = 49,
    TK_NEW      = 50,
    TK_AMBIENT  = 51,
    TK_FUNCTION = 52,
    TK_MESSAGE  = 53,
    TK_THROWS   = 54,
    TK_THROW    = 55,
    TK_RETURN   = 56,
    TK_BREAK    = 57,
    TK_CONTINUE = 58,
    TK_IF       = 59,
    TK_ELSE     = 60,
    TK_FOR      = 61,
    TK_IN       = 62,
    TK_WHILE    = 63,
    TK_DO       = 64,
    TK_MATCH    = 65,
    TK_CASE     = 66,
    TK_AS       = 67,
    TK_CATCH    = 68,
    TK_ALWAYS   = 69,
    TK_THIS     = 70,
    TK_TRUE     = 71,
    TK_FALSE    = 72,

    // abstract
    TK_MODULE   = 73,
    TK_DECLAREMAP = 74,
    TK_MAP      = 75,
    TK_TYPEBODY = 76,
    TK_TYPECLASS = 77,
    TK_FORMALARGS = 78,
    TK_FIELD    = 79,
    TK_ARG      = 80,
    TK_ARGS     = 81,
    TK_BLOCK    = 82,
    TK_CASEVAR  = 83,
    TK_LIST     = 84,

    TK_SCOLON   = 85,

    TK_EOF      = 86
};

static const std::map<TokenType, std::string> tokenToString = {
    {TokenType::TK_STRING        ,"TK_STRING"},
    {TokenType::TK_INT           ,"TK_INT"},
    {TokenType::TK_FLOAT         ,"TK_FLOAT"},
    {TokenType::TK_ID            ,"TK_ID"},
    {TokenType::TK_TYPEID        ,"TK_TYPEID"},
    {TokenType::TK_LBRACE        ,"TK_LBRACE"},
    {TokenType::TK_RBRACE        ,"TK_RBRACE"},
    {TokenType::TK_LPAREN        ,"TK_LPAREN"},
    {TokenType::TK_RPAREN        ,"TK_RPAREN"},
    {TokenType::TK_LBRACKET      ,"TK_LBRACKET"},
    {TokenType::TK_RBRACKET      ,"TK_RBRACKET"},
    {TokenType::TK_COMMA         ,"TK_COMMA"},
    {TokenType::TK_RESULTS       ,"TK_RESULTS"},
    {TokenType::TK_CALL          ,"TK_CALL"},
    {TokenType::TK_PACKAGE       ,"TK_PACKAGE"},
    {TokenType::TK_OFTYPE        ,"TK_OFTYPE"},
    {TokenType::TK_PARTIAL       ,"TK_PARTIAL"},
    {TokenType::TK_ASSIGN        ,"TK_ASSIGN"},
    {TokenType::TK_BANG          ,"TK_BANG"},
    {TokenType::TK_PLUS          ,"TK_PLUS"},
    {TokenType::TK_MINUS         ,"TK_MINUS"},
    {TokenType::TK_MULTIPLY      ,"TK_MULTIPLY"},
    {TokenType::TK_DIVIDE        ,"TK_DIVIDE"},
    {TokenType::TK_MOD           ,"TK_MOD"},
    {TokenType::TK_LSHIFT        ,"TK_LSHIFT"},
    {TokenType::TK_RSHIFT        ,"TK_RSHIFT"},
    {TokenType::TK_LT            ,"TK_LT"},
    {TokenType::TK_LE            ,"TK_LE"},
    {TokenType::TK_GE            ,"TK_GE"},
    {TokenType::TK_GT            ,"TK_GT"},
    {TokenType::TK_EQ            ,"TK_EQ"},
    {TokenType::TK_NOTEQ         ,"TK_NOTEQ"},
    {TokenType::TK_STEQ          ,"TK_STEQ"},
    {TokenType::TK_NSTEQ         ,"TK_NSTEQ"},
    {TokenType::TK_OR            ,"TK_OR"},
    {TokenType::TK_AND           ,"TK_AND"},
    {TokenType::TK_XOR           ,"TK_XOR"},
    {TokenType::TK_UNIQ          ,"TK_UNIQ"},
    {TokenType::TK_MUT           ,"TK_MUT"},
    {TokenType::TK_MODE          ,"TK_MODE"},
    {TokenType::TK_USE           ,"TK_USE"},
    {TokenType::TK_DECLARE       ,"TK_DECLARE"},
    {TokenType::TK_TYPE          ,"TK_TYPE"},
    {TokenType::TK_LAMBDA        ,"TK_LAMBDA"},
    {TokenType::TK_TRAIT         ,"TK_TRAIT"},
    {TokenType::TK_OBJECT        ,"TK_OBJECT"},
    {TokenType::TK_ACTOR         ,"TK_ACTOR"},
    {TokenType::TK_IS            ,"TK_IS"},
    {TokenType::TK_VAR           ,"TK_VAR"},
    {TokenType::TK_DELEGATE      ,"TK_DELEGATE"},
    {TokenType::TK_NEW           ,"TK_NEW"},
    {TokenType::TK_AMBIENT       ,"TK_AMBIENT"},
    {TokenType::TK_FUNCTION      ,"TK_FUNCTION"},
    {TokenType::TK_MESSAGE       ,"TK_MESSAGE"},
    {TokenType::TK_THROWS        ,"TK_THROWS"},
    {TokenType::TK_THROW         ,"TK_THROW"},
    {TokenType::TK_RETURN        ,"TK_RETURN"},
    {TokenType::TK_BREAK         ,"TK_BREAK"},
    {TokenType::TK_CONTINUE      ,"TK_CONTINUE"},
    {TokenType::TK_IF            ,"TK_IF"},
    {TokenType::TK_ELSE          ,"TK_ELSE"},
    {TokenType::TK_FOR           ,"TK_FOR"},
    {TokenType::TK_IN            ,"TK_IN"},
    {TokenType::TK_WHILE         ,"TK_WHILE"},
    {TokenType::TK_DO            ,"TK_DO"},
    {TokenType::TK_MATCH         ,"TK_MATCH"},
    {TokenType::TK_CASE          ,"TK_CASE"},
    {TokenType::TK_AS            ,"TK_AS"},
    {TokenType::TK_CATCH         ,"TK_CATCH"},
    {TokenType::TK_ALWAYS        ,"TK_ALWAYS"},
    {TokenType::TK_THIS          ,"TK_THIS"},
    {TokenType::TK_TRUE          ,"TK_TRUE"},
    {TokenType::TK_FALSE         ,"TK_FALSE"},
    {TokenType::TK_MODULE        ,"TK_MODULE"},
    {TokenType::TK_DECLAREMAP    ,"TK_DECLAREMAP"},
    {TokenType::TK_MAP           ,"TK_MAP"},
    {TokenType::TK_TYPEBODY      ,"TK_TYPEBODY"},
    {TokenType::TK_TYPECLASS     ,"TK_TYPECLASS"},
    {TokenType::TK_FORMALARGS    ,"TK_FORMALARGS"},
    {TokenType::TK_FIELD         ,"TK_FIELD"},
    {TokenType::TK_ARG           ,"TK_ARG"},
    {TokenType::TK_ARGS          ,"TK_ARGS"},
    {TokenType::TK_BLOCK         ,"TK_BLOCK"},
    {TokenType::TK_CASEVAR       ,"TK_CASEVAR"},
    {TokenType::TK_LIST          ,"TK_LIST"},
    {TokenType::TK_SCOLON        ,"TK_SCOLON"},
    {TokenType::TK_EOF           ,"TK_EOF"}
};

#define tokTypeInt(id) (static_cast<std::underlying_type<TokenType>::type>(id))

enum class Kind {
    TYPE_TRAIT,
    TYPE_ACTOR,
    TYPE_OBJECT,
    TYPE_PRIMITIVE,
    TYPE_IMPORT,
    TYPE_DECLARE
};

struct Token {
public:
    const std::string fileName;
    const size_t line;
    const size_t linePos;

    union {
        std::string string;
        double flt;
        size_t integer;
    };

    TokenType id;

    Token(std::string file, size_t l, size_t lp) :  fileName(file),
                                                    line(l), linePos(lp) {}

    Token(std::string file, size_t l, size_t lp, TokenType t) : fileName(file),
                                                                line(l),
                                                                linePos(lp),
                                                                id(t) {}
    ~Token() {}
};

struct AST {
    Token* const t;
    AST* sibling;
    std::vector<AST*> children;
    AST(AST* s, Token* _t) : t(_t), sibling(s), children(AST_SLOTS) {}
};

#endif  // CPP_INCLUDE_COMMON_H_
