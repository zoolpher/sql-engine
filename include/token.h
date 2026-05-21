// include/token.h

#pragma once
/*
Without #pragma once, if both lexer.cpp and parser.cpp include token.h — the compiler sees the 
same definitions twice and throws a redefinition error. 💀

#pragma once tells the compiler — "include this file only once, no matter how many times it's 
#included."
*/

#include <string>


enum class TokenType {
    // Keywords
    SELECT,
    FROM,
    WHERE,
    ORDER, BY, GROUP,

    // Identifiers 
    IDENTIFIER,

    // Operators
    GT,
    LT, 
    EQ,       // =
    NEQ,      // !=
    GTE,      // >=
    LTE,      // <=

    // Literals
    INTEGER, FLOAT, STRING,

    // Symbols
    COMMA,
    SEMICOLON,
    LPAREN,
    RPAREN,

    // Special
    END_OF_FILE     // Tells the lexer it's done.
};

struct Token {
    TokenType type; 
    std::string value;
};