// test_lexer.cpp
#include <iostream>
#include <vector>
#include "../include/token.h"

#include "../src/lexer/lexer.cpp"

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::SELECT:     return "SELECT";
        case TokenType::FROM:       return "FROM";
        case TokenType::WHERE:      return "WHERE";
        case TokenType::ORDER:      return "ORDER";
        case TokenType::BY:         return "BY";
        case TokenType::GROUP:      return "GROUP";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::GT:         return "GT";
        case TokenType::LT:         return "LT";
        case TokenType::EQ:         return "EQ";
        case TokenType::NEQ:        return "NEQ";
        case TokenType::GTE:        return "GTE";
        case TokenType::LTE:        return "LTE";
        case TokenType::INTEGER:    return "INTEGER";
        case TokenType::FLOAT:      return "FLOAT";
        case TokenType::STRING:     return "STRING";
        case TokenType::COMMA:      return "COMMA";
        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::END_OF_FILE: return "EOF";
        default:                    return "UNKNOWN";
    }
}

void runTest(const std::string& label, const std::string& query) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << label << "\n";
    std::cout << "QUERY: " << query << "\n";
    std::cout << "----------------------------------------\n";

    Lexer lexer(query);
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token& t : tokens) {
        std::cout << "[" << tokenTypeName(t.type) << "] => \"" << t.value << "\"\n";
    }
}

int main() {

    // --- NORMAL QUERIES ---

    runTest(
        "Basic SELECT",
        "SELECT name FROM users;"
    );

    runTest(
        "SELECT with WHERE and comparison",
        "SELECT name FROM users WHERE age >= 18;"
    );

    runTest(
        "Multiple columns + float literal",
        "SELECT id, salary FROM employees WHERE salary > 50000.50;"
    );

    runTest(
        "String literal (correct)",
        "SELECT name FROM users WHERE city = 'Delhi';"
    );

    runTest(
        "Not-equal operator",
        "SELECT name FROM users WHERE status != 'inactive';"
    );

    // --- BUG DEMONSTRATION ---

    runTest(
        "BUG: Unterminated string literal",
        "SELECT name FROM users WHERE city = 'Delhi"   // no closing quote
    );

    return 0;
}