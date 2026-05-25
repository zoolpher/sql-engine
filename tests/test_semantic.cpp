// tests/test_semantic.cpp

#include <iostream>
#include <vector>
#include <string>
#include "../include/token.h"
#include "../include/ast.h"
#include "../include/catalog.h"
#include "../src/lexer/lexer.cpp"
#include "../src/parser/parser.cpp"
#include "../src/semantic/semantic_analyzer.cpp"

// ─────────────────────────────────────────
// CATALOG SETUP
// ─────────────────────────────────────────

// implement getTable — required by Catalog struct
TableSchema* Catalog::getTable(std::string name) {
    for (TableSchema& t : tables) {
        if (t.table_name == name) return &t;
    }
    return nullptr;
}

Catalog buildCatalog() {
    Catalog catalog;

    // Table: users (id INT, name STRING, age INT, city STRING)
    TableSchema users;
    users.table_name = "users";
    users.columns = {
        {"id",   "INT"},
        {"name", "STRING"},
        {"age",  "INT"},
        {"city", "STRING"}
    };

    // Table: employees (id INT, department STRING, salary INT)
    TableSchema employees;
    employees.table_name = "employees";
    employees.columns = {
        {"id",         "INT"},
        {"department", "STRING"},
        {"salary",     "INT"}
    };

    catalog.tables.push_back(users);
    catalog.tables.push_back(employees);
    return catalog;
}

// ─────────────────────────────────────────
// DEBUG HELPERS
// ─────────────────────────────────────────

void printSeparator(const std::string& label) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << label << "\n";
    std::cout << "========================================\n";
}

void runTest(const std::string& label, const std::string& query, Catalog& catalog) {
    printSeparator(label);
    std::cout << "QUERY: " << query << "\n\n";

    try {
        // Lex
        Lexer lexer(query);
        std::vector<Token> tokens = lexer.tokenize();

        // Parse
        Parser parser(tokens);
        ASTNode* root = parser.parse();

        // Analyze
        SemanticAnalyzer analyzer(root, catalog);
        analyzer.analyze();

        std::cout << "[PASS] No errors — query is semantically valid\n";

    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << e.what() << "\n";
    } catch (...) {
        std::cout << "[FAIL] Unknown exception caught\n";
    }
}

// ─────────────────────────────────────────
// TESTS
// ─────────────────────────────────────────

int main() {
    Catalog catalog = buildCatalog();

    // ── VALID QUERIES — should all PASS ──

    runTest(
        "Valid: SELECT single column",
        "SELECT name FROM users",
        catalog
    );

    runTest(
        "Valid: SELECT multiple columns",
        "SELECT name, age, city FROM users",
        catalog
    );

    runTest(
        "Valid: SELECT with WHERE",
        "SELECT name FROM users WHERE age >= 18",
        catalog
    );

    runTest(
        "Valid: SELECT from employees table",
        "SELECT department FROM employees WHERE salary > 50000",
        catalog
    );

    // ── INVALID FROM — table does not exist ──

    runTest(
        "FAIL: table does not exist",
        "SELECT name FROM customers",
        catalog
        // 'customers' is not in catalog
        // should throw: table 'customers' does not exist
    );

    runTest(
        "FAIL: table name wrong case",
        "SELECT name FROM Users",
        catalog
        // 'Users' != 'users' — case sensitive match
        // should throw: table 'Users' does not exist
    );

    // ── INVALID SELECT — column does not exist ──

    runTest(
        "FAIL: column does not exist",
        "SELECT email FROM users",
        catalog
        // 'email' is not in users table
        // should throw: column 'email' does not exist
    );

    runTest(
        "FAIL: one valid column one invalid column",
        "SELECT name, email FROM users",
        catalog
        // 'name' is valid, 'email' is not
        // should throw on 'email'
    );

    runTest(
        "FAIL: column exists in different table",
        "SELECT salary FROM users",
        catalog
        // 'salary' exists in employees, not in users
        // should throw: column 'salary' does not exist in table 'users'
    );

    // ── INVALID WHERE — condition column does not exist ──

    runTest(
        "FAIL: WHERE column does not exist",
        "SELECT name FROM users WHERE email = 'test@test.com'",
        catalog
        // 'email' not in users
        // should throw: column 'email' does not exist
    );

    runTest(
        "FAIL: WHERE column from wrong table",
        "SELECT name FROM users WHERE salary > 50000",
        catalog
        // 'salary' exists in employees, not users
        // should throw: column 'salary' does not exist in table 'users'
    );

    // ── BUG: empty WhereNode always present ──

    runTest(
        "BUG: no WHERE clause — empty WhereNode still validated",
        "SELECT name FROM users",
        catalog
        // parser always attaches WhereNode even with no WHERE
        // condition.left = "" — an empty string
        // Pass 3 will try to validate "" as a column name
        // will throw: column '' does not exist — even though query is valid
    );

    return 0;
}