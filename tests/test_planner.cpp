// tests/test_planner.cpp

#include <iostream>
#include <vector>
#include <string>
#include "../include/token.h"
#include "../include/ast.h"
#include "../include/catalog.h"
#include "../include/plan.h"
#include "../src/lexer/lexer.cpp"
#include "../src/parser/parser.cpp"
#include "../src/planner/query_planner.cpp"

// ─────────────────────────────────────────
// DEBUG HELPERS
// ─────────────────────────────────────────

std::string planTypeName(PlanType type) {
    switch (type) {
        case PlanType::SCAN:    return "SCAN";
        case PlanType::FILTER:  return "FILTER";
        case PlanType::SORT:    return "SORT";
        case PlanType::PROJECT: return "PROJECT";
        default:                return "UNKNOWN";
    }
}

void printPlan(const std::vector<PlanNode>& plan) {
    std::cout << "[DEBUG] Execution Plan (" << plan.size() << " nodes):\n";
    for (int i = 0; i < plan.size(); i++) {
        const PlanNode& node = plan[i];
        std::cout << "  Step " << i + 1 << ": " << planTypeName(node.type);

        if (node.type == PlanType::SCAN) {
            std::cout << " | table: " << (node.table.empty() ? "(empty)" : node.table);
        }

        else if (node.type == PlanType::FILTER) {
            std::cout << " | condition: "
                      << (node.condition.left.empty()  ? "(empty)" : node.condition.left)  << " "
                      << (node.condition.op.empty()    ? "(empty)" : node.condition.op)    << " "
                      << (node.condition.right.empty() ? "(empty)" : node.condition.right);
        }

        else if (node.type == PlanType::SORT) {
            std::cout << " | column: " << (node.column.empty() ? "(empty)" : node.column);
        }

        else if (node.type == PlanType::PROJECT) {
            std::cout << " | columns: ";
            if (node.columns.empty()) {
                std::cout << "(empty)";
            } else {
                for (const auto& col : node.columns) std::cout << col << " ";
            }
        }

        std::cout << "\n";
    }
}

void printSeparator(const std::string& label) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << label << "\n";
    std::cout << "========================================\n";
}

void runTest(const std::string& label, const std::string& query) {
    printSeparator(label);
    std::cout << "QUERY: " << query << "\n\n";

    try {
        // Lex
        Lexer lexer(query);
        std::vector<Token> tokens = lexer.tokenize();

        // Parse
        Parser parser(tokens);
        ASTNode* root = parser.parse();

        // Plan
        QueryPlanner planner(root);
        std::vector<PlanNode> plan = planner.plan();

        printPlan(plan);

        // Validate execution order
        std::cout << "\n[DEBUG] Order check:\n";
        bool scan_seen    = false;
        bool filter_seen  = false;
        bool sort_seen    = false;
        bool project_seen = false;
        bool order_ok     = true;

        for (const PlanNode& node : plan) {
            if (node.type == PlanType::SCAN) {
                scan_seen = true;
            }
            if (node.type == PlanType::FILTER) {
                if (!scan_seen) { order_ok = false; std::cout << "  [WARN] FILTER appears before SCAN\n"; }
                filter_seen = true;
            }
            if (node.type == PlanType::SORT) {
                if (!scan_seen) { order_ok = false; std::cout << "  [WARN] SORT appears before SCAN\n"; }
                sort_seen = true;
            }
            if (node.type == PlanType::PROJECT) {
                if (!scan_seen)   { order_ok = false; std::cout << "  [WARN] PROJECT appears before SCAN\n"; }
                project_seen = true;
            }
        }

        if (order_ok) std::cout << "  [PASS] Execution order is correct\n";

    } catch (const std::exception& e) {
        std::cout << "[ERROR] " << e.what() << "\n";
    } catch (...) {
        std::cout << "[ERROR] Unknown exception\n";
    }
}

// ─────────────────────────────────────────
// TESTS
// ─────────────────────────────────────────

int main() {

    // ── NORMAL CASES ──

    runTest(
        "Basic SELECT FROM",
        "SELECT name FROM users"
        // expected plan: SCAN → PROJECT
    );

    runTest(
        "SELECT with WHERE",
        "SELECT name FROM users WHERE age >= 18"
        // expected plan: SCAN → FILTER → PROJECT
    );

    runTest(
        "SELECT multiple columns with WHERE",
        "SELECT name, age, city FROM users WHERE age > 20"
        // expected plan: SCAN → FILTER → PROJECT
        // PROJECT should contain all 3 columns
    );

    runTest(
        "SELECT with ORDER BY",
        "SELECT name FROM users ORDER BY name"
        // expected plan: SCAN → SORT → PROJECT
    );

    runTest(
        "SELECT with WHERE and ORDER BY",
        "SELECT name FROM users WHERE age > 20 ORDER BY name"
        // expected plan: SCAN → FILTER → SORT → PROJECT
    );

    // ── ORDER VALIDATION ──

    runTest(
        "ORDER CHECK: SCAN must come first",
        "SELECT name FROM users WHERE age > 20"
        // FILTER must appear after SCAN, not before
        // planner reverses the plan_nodes vector — verify this works correctly
    );

    // ── BUG: empty WhereNode always produces FILTER node ──

    runTest(
        "BUG: no WHERE — empty FILTER node in plan",
        "SELECT name FROM users"
        // parser always attaches WhereNode even with no WHERE clause
        // planner will emit a FILTER node with empty condition
        // expected plan should be: SCAN → PROJECT
        // actual plan will be:     SCAN → FILTER(empty) → PROJECT
        // FILTER with empty condition is meaningless and dangerous in executor
    );

    // ── BUG: GROUP BY not handled ──

    runTest(
        "BUG: GROUP BY missing from planner",
        "SELECT department FROM employees GROUP BY department"
        // GroupByNode exists in AST
        // but planner has no case for PlanType::GROUP
        // GroupByNode will be silently skipped
        // plan will only contain: SCAN → PROJECT
        // GROUP BY is completely lost
    );

    // ── BUG: reverse() correctness ──

    runTest(
        "REVERSE CHECK: plan order after std::reverse",
        "SELECT name FROM users WHERE age > 20 ORDER BY name"
        // AST traversal order: SELECT → WHERE → ORDER → FROM
        // after push_back:     PROJECT, FILTER, SORT, SCAN
        // after reverse():     SCAN, FILTER, SORT, PROJECT  ← correct
        // verify this is actually what comes out
    );

    return 0;
}