// main.cpp

#include <iostream>
#include <string>
#include <vector>

#include "include/token.h"
#include "include/ast.h"
#include "include/plan.h"
#include "include/table.h"
#include "include/catalog.h"

#include "src/lexer/lexer.cpp"
#include "src/parser/parser.cpp"
#include "src/semantic/semantic_analyzer.cpp"
#include "src/catalog/catalog.cpp"
#include "src/planner/query_planner.cpp"
#include "src/optimizer/optimizer.cpp"
#include "src/executor/executor.cpp"


int main() {

    while (true) {
        std::string query;
        std::cout << "sql-engine> ";
        std::getline(std::cin, query);

        try {
            // Stage 1 — Lex
            Lexer lexer(query);
            std::vector<Token> tokens = lexer.tokenize();

            // Stage 2 — Parse
            Parser parser(tokens);
            ASTNode* root = parser.parse();

            // extract table name from AST
            std::string tableName;
            ASTNode* current = root;
            while (current != nullptr) {
                if (FromNode* f = dynamic_cast<FromNode*>(current)) {
                    tableName = f->table;
                    break;
                }
                current = current->child;
            }

            if (tableName.empty()) {
                throw std::runtime_error("No table found in query");
            }

            // Stage 3 — build catalog dynamically from CSV headers
            Catalog catalog;
            TableSchema schema;
            schema.table_name = tableName;

            CSVReader reader("data/" + tableName + ".csv");
            for (const std::string& col : reader.readHeaders()) {
                schema.columns.push_back({col, "STRING"});
            }
            catalog.tables.push_back(schema);

            // Stage 3 — Semantic Analysis
            SemanticAnalyzer analyzer(root, catalog);
            analyzer.analyze();

            // Stage 4 — Query Planning
            QueryPlanner planner(root);
            std::vector<PlanNode> plan = planner.plan();

            // Stage 5 — Optimization
            Optimizer optimizer(plan);
            std::vector<PlanNode> optimizedPlan = optimizer.optimize();

            // Stage 6 — Execution
            Executor executor(optimizedPlan);
            Table result = executor.execute();
            executor.print(result);

        } catch (std::exception& e) {
            std::cout << "[ERROR] " << e.what() << "\n";
        }
    }

    return 0;
}