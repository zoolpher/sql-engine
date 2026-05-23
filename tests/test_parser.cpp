// tests/test_parser.cpp

#include <iostream>
#include <vector>
#include <string>
#include "../include/token.h"
#include "../include/ast.h"
#include "../src/lexer/lexer.cpp"
#include "../src/parser/parser.cpp"

// ─────────────────────────────────────────
// DEBUG HELPERS
// ─────────────────────────────────────────

void printSeparator(const std::string& label) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << label << "\n";
    std::cout << "========================================\n";
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "[DEBUG] Token stream:\n";
    for (const Token& t : tokens) {
        std::cout << "  [" << t.value << "]\n";
    }
}

void printAST(ASTNode* node, int depth = 0) {
    if (!node) {
        std::cout << std::string(depth * 2, ' ') << "(null)\n";
        return;
    }

    // Try SelectNode
    if (SelectNode* s = dynamic_cast<SelectNode*>(node)) {
        std::cout << std::string(depth * 2, ' ') << "SelectNode\n";
        std::cout << std::string(depth * 2, ' ') << "  columns: ";
        if (s->columns.empty()) {
            std::cout << "(none)";
        } else {
            for (const auto& col : s->columns) std::cout << col << " ";
        }
        std::cout << "\n";
        printAST(s->child, depth + 1);
        return;
    }

    // Try WhereNode
    if (WhereNode* w = dynamic_cast<WhereNode*>(node)) {
        std::cout << std::string(depth * 2, ' ') << "WhereNode\n";
        std::cout << std::string(depth * 2, ' ') << "  condition: "
                  << w->condition.left << " "
                  << w->condition.op << " "
                  << w->condition.right << "\n";
        printAST(w->child, depth + 1);
        return;
    }

    // Try FromNode
    if (FromNode* f = dynamic_cast<FromNode*>(node)) {
        std::cout << std::string(depth * 2, ' ') << "FromNode\n";
        std::cout << std::string(depth * 2, ' ') << "  table: " << f->table << "\n";
        printAST(f->child, depth + 1);
        return;
    }

    // Try OrderByNode
    if (OrderByNode* o = dynamic_cast<OrderByNode*>(node)) {
        std::cout << std::string(depth * 2, ' ') << "OrderByNode\n";
        std::cout << std::string(depth * 2, ' ') << "  column: " << o->column << "\n";
        printAST(o->child, depth + 1);
        return;
    }

    // Try GroupByNode
    if (GroupByNode* g = dynamic_cast<GroupByNode*>(node)) {
        std::cout << std::string(depth * 2, ' ') << "GroupByNode\n";
        std::cout << std::string(depth * 2, ' ') << "  column: " << g->column << "\n";
        printAST(g->child, depth + 1);
        return;
    }

    std::cout << std::string(depth * 2, ' ') << "UnknownNode\n";
}

void runTest(const std::string& label, const std::string& query) {
    printSeparator(label);
    std::cout << "QUERY: " << query << "\n\n";

    // Lex
    Lexer lexer(query);
    std::vector<Token> tokens = lexer.tokenize();
    printTokens(tokens);

    // Parse
    std::cout << "\n[DEBUG] AST:\n";
    try {
        Parser parser(tokens);
        ASTNode* root = parser.parse();
        printAST(root);
    } catch (const std::exception& e) {
        std::cout << "[ERROR] Exception caught: " << e.what() << "\n";
    } catch (...) {
        std::cout << "[ERROR] Unknown exception caught\n";
    }
}

// ─────────────────────────────────────────
// TESTS
// ─────────────────────────────────────────

int main() {

    // ── NORMAL CASES ──

    runTest(
        "Basic SELECT",
        "SELECT name FROM users"
    );

    runTest(
        "SELECT multiple columns",
        "SELECT name, age, salary FROM employees"
    );

    runTest(
        "SELECT with WHERE (integer)",
        "SELECT name FROM users WHERE age >= 18"
    );

    runTest(
        "SELECT with WHERE (string literal)",
        "SELECT name FROM users WHERE city = 'Delhi'"
    );

    runTest(
        "SELECT with ORDER BY",
        "SELECT name FROM users ORDER BY name"
    );

    runTest(
        "SELECT with GROUP BY",
        "SELECT department FROM employees GROUP BY department"
    );

    runTest(
        "SELECT with WHERE and ORDER BY",
        "SELECT name FROM users WHERE age > 20 ORDER BY name"
    );

    runTest(
        "SELECT with WHERE and GROUP BY",
        "SELECT department FROM employees WHERE salary > 50000 GROUP BY department"
    );

    // ── BUG / EDGE CASES ──

    runTest(
        "BUG: No WHERE clause — does WhereNode stay empty?",
        "SELECT name FROM users"
        // WhereNode is pre-allocated but never filled
        // condition.left, op, right will all be empty strings
        // child chain may be broken
    );

    runTest(
        "BUG: SELECT * (asterisk) — not handled",
        "SELECT * FROM users"
        // isalpha('*') is false, so it won't be caught as IDENTIFIER
        // columns vector will be empty, silent failure
    );

    runTest(
        "BUG: Missing FROM — parser loops forever or crashes",
        "SELECT name WHERE age > 20"
        // inner while loop: while(peek().type != FROM)
        // will consume WHERE, then consume tokens past it
        // likely crashes or infinite loops
    );

    runTest(
        "BUG: WHERE with float value",
        "SELECT salary FROM employees WHERE salary > 50000.50"
        // condition.right will be "50000.50" as a string — okay
        // but TokenType is FLOAT not INTEGER
        // consume().value still works, but worth verifying
    );

    runTest(
        "BUG: ASTNode child not initialized — dangling pointer?",
        "SELECT name FROM users ORDER BY name"
        // ASTNode struct has child pointer but no constructor
        // child is uninitialized garbage unless explicitly set
        // printAST traversal may crash or read garbage
    );

    runTest(
        "BUG: ORDER BY and GROUP BY both present — last one wins",
        "SELECT name FROM users ORDER BY name GROUP BY name"
        // both set fromNode->child
        // second one silently overwrites the first
        // ORDER BY result is lost
    );

    runTest(
        "BUG: Unterminated string literal in WHERE",
        "SELECT name FROM users WHERE city = 'Delhi"
        // lexer silently emits STRING token "Delhi"
        // parser gets it, condition.right = "Delhi"
        // no error raised anywhere
    );

    return 0;
}